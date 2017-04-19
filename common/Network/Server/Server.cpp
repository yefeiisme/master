#ifndef WIN32
#include <sys/epoll.h>
#endif
#include "../conn_info/conn_info.h"
#include "INetwork.h"
#include "Server.h"
#include "IFileLog.h"

#define MAX_EP 500
#define MAX_EP_WAIT (MAX_EP/100)

bool CServerNetwork::AcceptClient(const SOCKET nNewSocket)
{
	if (m_uFreeLinkIndex >= m_uMaxClientCount)
	{
		closesocket(nNewSocket);
		return false;
	}

	CNetworkConection	*pNewLink	= m_pFreeLink[m_uFreeLinkIndex];

	if (!pNewLink->ReInit(nNewSocket))
	{
		closesocket(nNewSocket);
		return false;
	}

	++m_uFreeLinkIndex;

	pNewLink->Connected();

	if (-1 == SetNoBlocking(pNewLink))
	{
		DelClient(pNewLink->m_uConnID);

#ifdef WIN32
		g_pFileLog->WriteLog("ioctlsocket() failed with error %d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("ERROR on rtsig the sock; errno=%d\n", errno);
#endif

		return false;
	}

	m_pfnConnectCallBack(m_pFunParam, pNewLink->m_uConnID);

	return true;
}

#ifdef WIN32
void CServerNetwork::ReadAction()
{
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);

	FD_SET(m_pListenLink->m_nSock, &m_ReadSet);
	timeval		timeout		= {0, 0};

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

	for (int nIndex = 0; nIndex < m_uMaxClientCount; ++nIndex)
	{
		if (m_pLinkList[nIndex].IsDisconnected())
			continue;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_ErrorSet);

		FD_SET(m_pLinkList[nIndex].m_nSock, &m_ReadSet);
		FD_SET(m_pLinkList[nIndex].m_nSock, &m_ErrorSet);

		if (select(0, &m_ReadSet, NULL, &m_ErrorSet, &timeout) <= 0)
			continue;

		if (FD_ISSET(m_pLinkList[nIndex].m_nSock, &m_ReadSet))
		{
			if (m_pLinkList[nIndex].RecvData() == -1)
			{
				DelClient(m_pLinkList[nIndex].m_uConnID);
			}
		}
		else if(FD_ISSET(m_pLinkList[nIndex].m_nSock, &m_ErrorSet))
		{
			DelClient(m_pLinkList[nIndex].m_uConnID);
		}
	}
}
#else
void CServerNetwork::ReadAction()
{
	epoll_event wv[MAX_EP_WAIT] = {0};

	int nRetCount = epoll_wait(m_nepfd, wv, MAX_EP_WAIT, 0);

	if (nRetCount < 0)
		return;

	for (int nLoopCount = 0; nLoopCount < nRetCount; ++nLoopCount)
	{
		CNetworkConection	*pNetLink	= (CNetworkConection*)wv[nLoopCount].data.ptr;
		if (pNetLink->m_nSock == m_pListenLink->m_nSock)
		{
			sockaddr_in	client_addr;
			socklen_t	length = sizeof(client_addr);
			SOCKET		nNewSocket = INVALID_SOCKET;

			while((nNewSocket = accept(m_pListenLink->m_nSock, (sockaddr*)&client_addr, &length)) > 0)
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
		else if(wv[nLoopCount].events & EPOLLERR)
		{
			// error
			DelClient(pNetLink->m_uConnID);
		}
	}
}
#endif	

void CServerNetwork::WriteAction()
{
	for (int nIndex = 0; nIndex < m_uMaxClientCount; ++nIndex)
	{
		if (m_pLinkList[nIndex].FlushData() == -1)
		{
			DelClient( nIndex );
		}
	}
}

int CServerNetwork::SetNoBlocking(CNetworkConection *pNetLink)
{
	if (NULL == pNetLink)
		return -1;

#ifdef WIN32
	unsigned long ulNonBlock = 1;
	return ioctlsocket(pNetLink->m_nSock, FIONBIO, &ulNonBlock);
#else
	int nFlags;
	if ((nFlags = fcntl(pNetLink->m_nSock, F_GETFL, 0)) < 0 || fcntl(pNetLink->m_nSock, F_SETFL, nFlags | O_NONBLOCK ) < 0)
		return -1;

	epoll_event ev	= {0};

	ev.data.ptr		= pNetLink;
	ev.events		= EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP | EPOLLERR ;

	return epoll_ctl(m_nepfd, EPOLL_CTL_ADD, pNetLink->m_nSock, &ev);
#endif
}

bool CServerNetwork::DelClient(const unsigned int nClientID)
{
	if (nClientID >= m_uMaxClientCount)
		return false;

	if (!m_pLinkList[nClientID].IsConnect())
		return false;

#ifdef __linux
	int ret = epoll_ctl(m_nepfd, EPOLL_CTL_DEL, m_pLinkList[nClientID].m_nSock, NULL);
#endif
	m_pLinkList[nClientID].Disconnect();

	m_pFreeLink[--m_uFreeLinkIndex]	= &m_pLinkList[nClientID];

	m_pfnDisconnectCallBack(m_pFunParam, nClientID);

	return true;
}

CServerNetwork::CServerNetwork()
{
	m_uFreeLinkIndex		= 0;
	m_pFreeLink				= NULL;
	m_pfnConnectCallBack	= NULL;
	m_pfnDisconnectCallBack	= NULL;
	m_uMaxClientCount		= 0;
	m_uClientRecvBuffSize	= 0;
	m_uClientSendBuffSize	= 0;
#ifdef __linux
	m_nepfd = 0;
#else
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);
#endif
}

CServerNetwork::~CServerNetwork()
{
	m_pListenLink->Disconnect();

	for (int nIndex = 0; nIndex < m_uMaxClientCount; ++nIndex)
	{
		m_pLinkList[nIndex].Disconnect();
	}

	SAFE_DELETE(m_pListenLink);
	SAFE_DELETE_ARR(m_pLinkList);

#ifdef __linux
	closesocket(m_nepfd);
#endif
#ifdef WIN32
	WSACleanup();
#endif
}

bool CServerNetwork::Initialize(
	unsigned short usPort,
	void *lpParam,
	CALLBACK_SERVER_EVENT pfnConnectCallBack,
	CALLBACK_SERVER_EVENT pfnDisconnectCallBack,
	unsigned int uLinkNum,
	unsigned int uSendBufferLen,
	unsigned int uRecvBufferLen,
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen
	)
{
#ifdef WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	
	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		return false;
	}
#endif

	m_uMaxClientCount		= uLinkNum;
	m_uClientRecvBuffSize	= uRecvBufferLen;
	m_uClientSendBuffSize	= uSendBufferLen;
	m_pfnConnectCallBack	= pfnConnectCallBack;
	m_pfnDisconnectCallBack	= pfnDisconnectCallBack;
	m_pFunParam				= lpParam;

	if (0 == m_uMaxClientCount)
		return false;

	if (0 == m_uClientRecvBuffSize)
		return false;

	if (0 == m_uClientSendBuffSize)
		return false;

	if (NULL == lpParam)
		return false;

	if (NULL == pfnConnectCallBack)
		return false;

	if (NULL == pfnDisconnectCallBack)
		return false;

	if (NULL == lpParam)
		return false;

	m_pListenLink	= new CNetworkConection;
	if (NULL == m_pListenLink)
		return false;

	m_pLinkList		= new CNetworkConection[m_uMaxClientCount];
	if (NULL == m_pLinkList)
		return false;

	m_pFreeLink		= new CNetworkConection*[m_uMaxClientCount];
	if (!m_pFreeLink)
		return false;

	for (int nIndex = 0; nIndex < m_uMaxClientCount; ++nIndex)
	{
		if (!m_pLinkList[nIndex].Initialize(m_uClientRecvBuffSize, m_uClientSendBuffSize, uTempSendBufferLen, uTempRecvBufferLen))
			return false;

		m_pLinkList[nIndex].m_uConnID = nIndex;

		m_pFreeLink[nIndex]	= &m_pLinkList[nIndex];
	}

#ifdef __linux
	m_nepfd = epoll_create(MAX_EP);
	if (-1 == m_nepfd)
	{
		g_pFileLog->WriteLog( "epoll_create failed!\n" );
		return false;
	}
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
#ifdef WIN32
	setsockopt(m_pListenLink->m_nSock, SOL_SOCKET, SO_REUSEADDR, (const char*)&nOnFlag, sizeof(nOnFlag));
#else
	setsockopt(m_pListenLink->m_nSock, SOL_SOCKET, SO_REUSEADDR, (const void*)&nOnFlag, sizeof(nOnFlag));
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
#ifdef WIN32
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", WSAGetLastError());   
#elif defined(__linux)
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", errno);
#endif
		return false;
	}

	return true;
}

void CServerNetwork::Release()
{
	delete this;
}

int CServerNetwork::SendData(const unsigned int uConnID, const void *pData, const unsigned int uLength)
{
	if (uConnID >= m_uMaxClientCount)
		return -1;

	return (-1 == m_pLinkList[uConnID].PutPack(pData, uLength) ? -1 : 0);
}

const void *CServerNetwork::GetPackFromClient(const unsigned int uConnID, unsigned int &uLength)
{
	if (uConnID >= m_uMaxClientCount)
		return NULL;

	return m_pLinkList[uConnID].GetPack(uLength);
}

bool CServerNetwork::ShutdownClient(const unsigned int uConnID)
{
	return DelClient(uConnID);
}

void CServerNetwork::CloseAcceptor()
{
	if (m_pListenLink)
	{
#ifdef __linux
		int ret = epoll_ctl(m_nepfd, EPOLL_CTL_DEL, m_pListenLink->m_nSock, NULL);
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
	CALLBACK_SERVER_EVENT pfnDisconnectCallBack,
	unsigned int uLinkNum,
	unsigned int uSendBufferLen,
	unsigned int uRecvBufferLen,
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen
)
{
	CServerNetwork  *pServer = new CServerNetwork();
	if (NULL == pServer)
		return NULL;

	if (!pServer->Initialize(usPort, lpParam, pfnConnectCallBack, pfnDisconnectCallBack, uLinkNum, uSendBufferLen, uRecvBufferLen, uTempSendBufferLen, uTempRecvBufferLen))
	{
		pServer->Release();
		return NULL;
	}

	return pServer;
}
