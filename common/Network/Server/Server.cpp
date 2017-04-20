#if defined(__linux)
#include <sys/epoll.h>
#elif defined(__APPLE__)
#endif
#include "../conn_info/conn_info.h"
#include "INetwork.h"
#include "Server.h"
#include "IFileLog.h"

#if defined(__linux)
#define MAX_EP 500
#define MAX_EP_WAIT (MAX_EP/100)
#endif

CServerNetwork::CServerNetwork()
{
#if defined(__linux)
	m_nepfd					= 0;
#elif defined(WIN32) || defined(WIN64)
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);
#elif defined(__APPLE__)
#endif

	m_pfnConnectCallBack	= NULL;
	m_pFunParam				= NULL;

	m_pListenLink			= NULL;

	m_uMaxConnCount			= 0;
	m_pTcpConnection		= NULL;
	m_pFreeConn				= NULL;
	m_uFreeConnIndex		= 0;

	m_uClientRecvBuffSize	= 0;
	m_uClientSendBuffSize	= 0;

	m_uSleepFrame			= 0;
	m_bRunning				= false;
}

CServerNetwork::~CServerNetwork()
{
	m_pListenLink->Disconnect();

	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		m_pTcpConnection[nIndex].Disconnect();
	}

	SAFE_DELETE(m_pListenLink);
	SAFE_DELETE_ARR(m_pTcpConnection);

#if defined(__linux)
	closesocket(m_nepfd);
#elif defined(WIN32) || (WIN64)
	WSACleanup();
#elif defined(__APPLE__)
#endif
}

int CServerNetwork::SetNoBlocking(CTcpConnection *pNetLink)
{
	if (NULL == pNetLink)
		return -1;

#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	return ioctlsocket(pNetLink->m_nSock, FIONBIO, &ulNonBlock);
#elif defined(__linux)
	int nFlags;
	if ((nFlags = fcntl(pNetLink->m_nSock, F_GETFL, 0)) < 0 || fcntl(pNetLink->m_nSock, F_SETFL, nFlags | O_NONBLOCK) < 0)
		return -1;

	epoll_event ev	={ 0 };

	ev.data.ptr	= pNetLink;
	ev.events	= EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP | EPOLLERR;

	return epoll_ctl(m_nepfd, EPOLL_CTL_ADD, pNetLink->m_nSock, &ev);
#elif defined(__APPLE__)
#endif
}

bool CServerNetwork::AcceptClient(const SOCKET nNewSocket)
{
	if (m_uFreeConnIndex >= m_uMaxConnCount)
	{
		closesocket(nNewSocket);
		return false;
	}

	CTcpConnection	*pNewLink	= m_pFreeConn[m_uFreeConnIndex];

	++m_uFreeConnIndex;

	pNewLink->ReInit(nNewSocket);

	pNewLink->Connected();

	if (-1 == SetNoBlocking(pNewLink))
	{
		DelClient(pNewLink->m_uConnID);

#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("ioctlsocket() failed with error %d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("ERROR on rtsig the sock; errno=%d\n", errno);
#elif defined(__APPLE__)
#endif

		return false;
	}

	m_pfnConnectCallBack(m_pFunParam, pNewLink);

	return true;
}

bool CServerNetwork::DelClient(const unsigned int nClientID)
{
	if (nClientID >= m_uMaxConnCount)
		return false;

	if (!m_pTcpConnection[nClientID].IsConnect())
		return false;

#if defined(__linux)
	int ret = epoll_ctl(m_nepfd, EPOLL_CTL_DEL, m_pTcpConnection[nClientID].m_nSock, NULL);
#elif defined(__APPLE__)
#endif
	m_pTcpConnection[nClientID].Disconnect();

	m_pFreeConn[--m_uFreeConnIndex]	= &m_pTcpConnection[nClientID];

	//m_pfnDisconnectCallBack(m_pFunParam, nClientID);

	return true;
}

#if defined(WIN32) || defined(WIN64)
void CServerNetwork::ReadAction()
{
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);

	FD_SET(m_pListenLink->m_nSock, &m_ReadSet);
	timeval	timeout	= {0, 0};

	if (select(0, &m_ReadSet, NULL, NULL, &timeout) > 0)
	{
		if (FD_ISSET(m_pListenLink->m_nSock, &m_ReadSet))
		{
			sockaddr_in	client_addr;
			socklen_t	length	= sizeof(client_addr);
			SOCKET	nNewSocket	= accept(m_pListenLink->m_nSock, (sockaddr*)&client_addr, &length);
			if (INVALID_SOCKET != nNewSocket)
			{
				AcceptClient(nNewSocket);
			}
		}
	}

	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].IsDisconnected())
			continue;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_ErrorSet);

		FD_SET(m_pTcpConnection[nIndex].m_nSock, &m_ReadSet);
		FD_SET(m_pTcpConnection[nIndex].m_nSock, &m_ErrorSet);

		if (select(0, &m_ReadSet, NULL, &m_ErrorSet, &timeout) <= 0)
			continue;

		if (FD_ISSET(m_pTcpConnection[nIndex].m_nSock, &m_ReadSet))
		{
			if (m_pTcpConnection[nIndex].RecvData() == -1)
			{
				DelClient(m_pTcpConnection[nIndex].m_uConnID);
			}
		}
		else if (FD_ISSET(m_pTcpConnection[nIndex].m_nSock, &m_ErrorSet))
		{
			DelClient(m_pTcpConnection[nIndex].m_uConnID);
		}
	}
}
#elif defined(__linux)
void CServerNetwork::ReadAction()
{
	epoll_event wv[MAX_EP_WAIT] = {0};

	int nRetCount = epoll_wait(m_nepfd, wv, MAX_EP_WAIT, 0);

	if (nRetCount < 0)
		return;

	for (int nLoopCount = 0; nLoopCount < nRetCount; ++nLoopCount)
	{
		CTcpConnection	*pNetLink	= (CTcpConnection*)wv[nLoopCount].data.ptr;
		if (pNetLink->m_nSock == m_pListenLink->m_nSock)
		{
			sockaddr_in	client_addr;
			socklen_t	length = sizeof(client_addr);
			SOCKET		nNewSocket = INVALID_SOCKET;

			while ((nNewSocket = accept(m_pListenLink->m_nSock, (sockaddr*)&client_addr, &length)) > 0)
			{
				AcceptClient(nNewSocket);
			}

			continue;
		}

		if (wv[nLoopCount].events & EPOLLIN)
		{
			if (pNetLink->RecvData() == -1)
			{
				DelClient(pNetLink->m_uConnID);
			}
		}
		else if (wv[nLoopCount].events & EPOLLPRI)
		{
			// data incoming
			//do_read_pack( pNetLink->m_nSock );
		}
		else if (wv[nLoopCount].events & EPOLLHUP)
		{
			DelClient(pNetLink->m_uConnID);
		}
		else if (wv[nLoopCount].events & EPOLLERR)
		{
			// error
			DelClient(pNetLink->m_uConnID);
		}
	}
}
#elif defined(__APPLE__)
#endif	

void CServerNetwork::WriteAction()
{
	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].FlushData() == -1)
		{
			DelClient(nIndex);
		}
	}
}

bool CServerNetwork::Initialize(
	unsigned short usPort,
	void *lpParam,
	CALLBACK_SERVER_EVENT pfnConnectCallBack,
	unsigned int uConnectionNum,
	unsigned int uSendBufferLen,
	unsigned int uRecvBufferLen,
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen,
	const bool bThread,
	const unsigned int uThreadFrame
	)
{
#if defined(WIN32) || defined(WIN64)
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		return false;
	}
#endif

	m_uMaxConnCount		= uConnectionNum;
	m_uClientRecvBuffSize	= uRecvBufferLen;
	m_uClientSendBuffSize	= uSendBufferLen;
	m_pfnConnectCallBack	= pfnConnectCallBack;
	m_pFunParam				= lpParam;

	if (0 == m_uMaxConnCount)
		return false;

	if (0 == m_uClientRecvBuffSize)
		return false;

	if (0 == m_uClientSendBuffSize)
		return false;

	if (NULL == lpParam)
		return false;

	if (NULL == pfnConnectCallBack)
		return false;

	if (NULL == lpParam)
		return false;

	m_pListenLink	= new CTcpConnection;
	if (NULL == m_pListenLink)
		return false;

	m_pTcpConnection		= new CTcpConnection[m_uMaxConnCount];
	if (NULL == m_pTcpConnection)
		return false;

	m_pFreeConn		= new CTcpConnection*[m_uMaxConnCount];
	if (!m_pFreeConn)
		return false;

	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (!m_pTcpConnection[nIndex].Initialize(m_uClientRecvBuffSize, m_uClientSendBuffSize, uTempSendBufferLen, uTempRecvBufferLen))
			return false;

		m_pTcpConnection[nIndex].m_uConnID = nIndex;

		m_pFreeConn[nIndex]	= &m_pTcpConnection[nIndex];
	}

#if defined(__linux)
	m_nepfd = epoll_create(MAX_EP);
	if (-1 == m_nepfd)
	{
		g_pFileLog->WriteLog( "epoll_create failed!\n" );
		return false;
	}
#elif defined(__APPLE__)
#endif

	struct sockaddr_in tagAddr;
	memset(&tagAddr, 0, sizeof(tagAddr));
	tagAddr.sin_family		= AF_INET;
	tagAddr.sin_port		= htons(usPort);
	tagAddr.sin_addr.s_addr	= INADDR_ANY;

	// start listen
	m_pListenLink->m_nSock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_pListenLink->m_nSock < 0)
	{
		g_pFileLog->WriteLog("create listen socket failed!\n");
		return false;
	}

	int nOnFlag = 1;
#if defined(WIN32) || defined(WIN64)
	setsockopt(m_pListenLink->m_nSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOnFlag, sizeof(nOnFlag));
#elif defined(__linux)
	setsockopt(m_pListenLink->m_nSock, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOnFlag, sizeof(nOnFlag));
#elif defined(__APPLE__)
#endif

	if (bind(m_pListenLink->m_nSock, (const sockaddr *)&tagAddr, sizeof(tagAddr)))
	{
		g_pFileLog->WriteLog("bind listen socket failed!\n");
		return false;
	}

	if (listen(m_pListenLink->m_nSock, 4096))
	{
		g_pFileLog->WriteLog("listen socket failed!\n");
		return false;
	}

	if (-1 == SetNoBlocking(m_pListenLink))
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", WSAGetLastError());   
#elif defined(__linux)
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		return false;
	}

	if (bThread)
	{
		// 开启线程，执行DoNetworkAction
		// ...
		m_bRunning	= true;
	}

	return true;
}

void CServerNetwork::Stop()
{
}

void CServerNetwork::Release()
{
	delete this;
}

void CServerNetwork::CloseAcceptor()
{
	if (m_pListenLink)
	{
#if defined(__linux)
		int ret = epoll_ctl(m_nepfd, EPOLL_CTL_DEL, m_pListenLink->m_nSock, NULL);
#elif defined(__APPLE__)
#endif
		m_pListenLink->Disconnect();
	}
}

void CServerNetwork::DoNetworkAction()
{
	ReadAction();
	WriteAction();
}

IServerNetwork *CreateServerNetwork(
	unsigned short usPort,
	void *lpParam,
	CALLBACK_SERVER_EVENT pfnConnectCallBack,
	unsigned int uConnectionNum,
	unsigned int uSendBufferLen,
	unsigned int uRecvBufferLen,
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen,
	const bool bThread,
	const unsigned int uSleepFrame
)
{
	CServerNetwork  *pServer = new CServerNetwork();
	if (NULL == pServer)
		return NULL;

	if (!pServer->Initialize(usPort, lpParam, pfnConnectCallBack, uConnectionNum, uSendBufferLen, uRecvBufferLen, uTempSendBufferLen, uTempRecvBufferLen, bThread, uSleepFrame))
	{
		pServer->Release();
		return NULL;
	}

	return pServer;
}
