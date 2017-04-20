#include "../NetworkHead.h"
#include "INetwork.h"
#include "../conn_info/conn_info.h"
#include "Client.h"
#include "IFileLog.h"

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

CClientNetwork::CClientNetwork()
{
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);
	FD_ZERO(&m_ErrorSet);

	m_uMaxLinkCount		= 0;
	m_pLinkList			= NULL;
	m_pFreeLink			= NULL;
	m_nFreeLinkIndex	= 0;

	m_uRecvBufSize		= 0;
	m_uSendBufSize		= 0;

	m_bRunning			= false;
}

CClientNetwork::~CClientNetwork()
{
	for (int nIndex = 0; nIndex < m_uMaxLinkCount; ++nIndex)
	{
		if (!m_pLinkList[nIndex].IsDisconnected())
			m_pLinkList[nIndex].Disconnect();
	}

	SAFE_DELETE_ARR(m_pLinkList);
	m_uMaxLinkCount	= 0;
	SAFE_DELETE_ARR(m_pFreeLink);
	m_nFreeLinkIndex	= 0;

	m_uRecvBufSize		= 0;
	m_uSendBufSize		= 0;

#if defined(WIN32) || defined(WIN64)
	WSACleanup();
#endif
}

bool CClientNetwork::Initialize(
	unsigned int uClientCount,
	unsigned int uSendBuffLen,
	unsigned int uRecvBuffLen,
	unsigned int uTempSendBuffLen,
	unsigned int uTempRecvBuffLen,
	void *lpParm,
	const bool bThread,
	const unsigned int uThreadFrame
	)
{
	m_uMaxLinkCount	= uClientCount;
	m_uRecvBufSize	= uRecvBuffLen;
	m_uSendBufSize	= uSendBuffLen;

#if defined(WIN32) || defined(WIN64)
	WSADATA	wsaData;
	WORD	wVersionRequested	= MAKEWORD(2, 2);
	int		nError				= WSAStartup(wVersionRequested, &wsaData);

	if (nError != 0)
		return false;
#endif

	m_pLinkList = new CTcpConnection[m_uMaxLinkCount];
	if (NULL == m_pLinkList)
		return false;

	m_pFreeLink	= new CTcpConnection*[m_uMaxLinkCount];
	if (NULL == m_pFreeLink)
		return false;

	for (unsigned int uiIndex = 0; uiIndex < m_uMaxLinkCount; ++uiIndex)
	{
		if (!m_pLinkList[uiIndex].Initialize(m_uRecvBufSize, m_uSendBufSize, uTempRecvBuffLen, uTempSendBuffLen))
			return false;

		m_pLinkList[uiIndex].m_uConnID	= uiIndex;

		m_pFreeLink[uiIndex]			= &m_pLinkList[uiIndex];
	}

	m_pFunParam	= lpParm;

	if (bThread)
	{
		// 开启线程，执行DoNetworkAction
		// ...
		m_bRunning	= true;
	}

	return true;
}

void CClientNetwork::Release()
{
	delete this;
}

ITcpConnection *CClientNetwork::ConnectTo(char* strAddress, const unsigned short usPort)
{
	if (m_nFreeLinkIndex >= m_uMaxLinkCount)
		return NULL;

	addrinfo tagAddInfo;
	memset(&tagAddInfo,0,sizeof(tagAddInfo));
	tagAddInfo.ai_family	= AF_UNSPEC;
	tagAddInfo.ai_socktype	= SOCK_STREAM;
	tagAddInfo.ai_protocol	= IPPROTO_TCP;

	addrinfo	*pNetAddr	= NULL;
	addrinfo	*pRes		= NULL;
	char		strPort[8];
	memset(strPort, 0, sizeof(strPort));
	snprintf(strPort, sizeof(strPort), "%hu", usPort);

	int retVal	= getaddrinfo(strAddress, strPort, &tagAddInfo, &pNetAddr);
	if (0 != retVal)
	{
		g_pFileLog->WriteLog("[%s][%d] getaddrinfo For IP Type Error\n", __FILE__, __LINE__);
		return NULL;
	}

	for (pRes = pNetAddr; pRes != NULL; pRes = pRes->ai_next)
	{
		if (AF_INET6 == pRes->ai_addr->sa_family)
		{
			char strBuffer[128] = {};
			sockaddr_in	*pAddr = (struct sockaddr_in*)pRes->ai_addr;
			inet_ntop(AF_INET6, &((reinterpret_cast<struct sockaddr_in6*>(pAddr))->sin6_addr), strBuffer, sizeof(strBuffer));

			int nNewSock = socket(AF_INET6, SOCK_STREAM, 0);
			if (nNewSock < 0)
			{
				g_pFileLog->WriteLog("socket failed,sock = %d\n", nNewSock);
				return NULL;
			}

			sockaddr_in6 tagAddrIn;
			memset(&tagAddrIn, 0, sizeof(tagAddrIn));
			tagAddrIn.sin6_family	= AF_INET6;
			tagAddrIn.sin6_port		= htons(usPort);
			if (inet_pton(AF_INET6, strBuffer, &tagAddrIn.sin6_addr) < 0)
			{
				g_pFileLog->WriteLog("inet_pton Error\n");
				return NULL;
			}

			int nRet = connect(nNewSock, (sockaddr*)&tagAddrIn, sizeof(tagAddrIn));
			if (0 == nRet)
			{
				CTcpConnection	*pNewLink = m_pFreeLink[m_nFreeLinkIndex];

				++m_nFreeLinkIndex;

				pNewLink->ReInit(nNewSock, true);

				pNewLink->Connected();

				return pNewLink;
			}
#if defined(WIN32) || defined(WIN64)
			else if (WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux)
			else if (EINPROGRESS == errno)
#elif defined(__APPLE__)
#endif
			{
				CTcpConnection	*pNewLink = m_pFreeLink[m_nFreeLinkIndex];

				++m_nFreeLinkIndex;

				pNewLink->ReInit(nNewSock, true);

				pNewLink->WaitConnect();

				return pNewLink;
			}
			else
			{
				closesocket(nNewSock);
				return NULL;
			}
		}
		else if (AF_INET == pRes->ai_addr->sa_family)
		{
			int	nNewSock = socket(AF_INET, SOCK_STREAM, 0);
			if (nNewSock < 0)
			{
				g_pFileLog->WriteLog("socket failed,sock = %d\n", nNewSock);
				return NULL;
			}

			// make it nonblock
			//#ifdef WIN32
			//	ULONG NonBlock = 1;   
			//	if (ioctlsocket(nNewSock, FIONBIO, &NonBlock) == SOCKET_ERROR)   
			//	{   
			//		printf("ioctlsocket() failed with error %d\n", WSAGetLastError());   
			//		closesocket(nNewSock);
			//		return false;   
			//	}
			//#else
			//	int nFlags = fcntl(nNewSock, F_GETFL, 0);
			//	if (nFlags < 0 || fcntl(nNewSock, F_SETFL, nFlags | O_NONBLOCK | O_ASYNC ) < 0)
			//	{
			//		closesocket(nNewSock);
			//		return false;
			//	}
			//#endif

			sockaddr_in	tagAddrIn;

			memset(&tagAddrIn, 0, sizeof(tagAddrIn));
			tagAddrIn.sin_family		= AF_INET;
			tagAddrIn.sin_port			= htons(usPort);
			tagAddrIn.sin_addr.s_addr	= inet_addr(strAddress);

			int nRet = connect(nNewSock, (sockaddr*)&tagAddrIn, sizeof(tagAddrIn));
			if (0 == nRet)
			{
				CTcpConnection	*pNewLink = m_pFreeLink[m_nFreeLinkIndex];

				++m_nFreeLinkIndex;

				pNewLink->ReInit(nNewSock);

				pNewLink->Connected();

				return pNewLink;
			}
#if defined(WIN32) || defined(WIN64)
			else if (WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux)
			else if (EINPROGRESS == errno)
#elif defined(__APPLE__)
#endif
			{
				CTcpConnection	*pNewLink = m_pFreeLink[m_nFreeLinkIndex];

				++m_nFreeLinkIndex;

				pNewLink->ReInit(nNewSock);

				pNewLink->WaitConnect();

				return pNewLink;
			}
			else
			{
				closesocket(nNewSock);
				return NULL;
			}
		}
	}

	return NULL;
}

void CClientNetwork::DoNetworkAction()
{
	for (unsigned int uiIndex = 0; uiIndex < m_uMaxLinkCount; ++uiIndex)
	{
		if (m_pLinkList[uiIndex].IsConnect())
		{
			ProcessConnectedLink(&m_pLinkList[uiIndex]);
		}
		else if (m_pLinkList[uiIndex].IsWaitConnect())
		{
			ProcessWaitConnectLink(&m_pLinkList[uiIndex]);
		}
	}
}

void CClientNetwork::ProcessConnectedLink(CTcpConnection *pNetLink)
{
	timeval	timeout	= {0, 0};

	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);
	FD_ZERO(&m_ErrorSet);

	FD_SET(pNetLink->m_nSock, &m_ReadSet);
	FD_SET(pNetLink->m_nSock, &m_WriteSet);
	FD_SET(pNetLink->m_nSock, &m_ErrorSet);

	if (select(1024, &m_ReadSet, &m_WriteSet, &m_ErrorSet, &timeout) <= 0)
		return;

	if (FD_ISSET(pNetLink->m_nSock, &m_ErrorSet))
	{
		CloseNetLink(pNetLink);
	}
	else
	{
		if (FD_ISSET(pNetLink->m_nSock, &m_ReadSet))
		{
			if (pNetLink->RecvData() == -1)
			{
				CloseNetLink(pNetLink);
			}
		}

		if (FD_ISSET(pNetLink->m_nSock, &m_WriteSet))
		{
			if (pNetLink->FlushData() == -1)
			{
				CloseNetLink(pNetLink);
			}
		}
	}
}

void CClientNetwork::ProcessWaitConnectLink(CTcpConnection *pNetLink)
{
	timeval	timeout	= {0, 0};

	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);

	FD_SET(pNetLink->m_nSock, &m_ReadSet);
	FD_SET(pNetLink->m_nSock, &m_WriteSet);

	if (select(0, &m_ReadSet, &m_WriteSet, NULL, &timeout) <= 0)
		return;

	if (!FD_ISSET(pNetLink->m_nSock, &m_WriteSet) && !FD_ISSET(pNetLink->m_nSock, &m_ReadSet))
		return;

	int nError = 0;
	socklen_t len = sizeof(nError);
#if defined(WIN32) || defined(WIN64)
	if (getsockopt(pNetLink->m_nSock, SOL_SOCKET, SO_ERROR, (char*)&nError, &len) < 0)
#elif defined(__linux)
	if (getsockopt(pNetLink->m_nSock, SOL_SOCKET, SO_ERROR, &nError, &len) < 0)
#elif defined(__APPLE__)
#endif
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		closesocket(pNetLink->m_nSock);
		m_pFreeLink[--m_nFreeLinkIndex]	= pNetLink;
		return;
	}

	if (nError != 0)
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("Connnect Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("Connnect Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		closesocket(pNetLink->m_nSock);
		m_pFreeLink[--m_nFreeLinkIndex]	= pNetLink;
		return;
	}
#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	if (ioctlsocket(pNetLink->m_nSock, FIONBIO, &ulNonBlock) == SOCKET_ERROR)
	{
		g_pFileLog->WriteLog("Set socket async failed! errno=%d\n", WSAGetLastError());
		closesocket(pNetLink->m_nSock);
		m_pFreeLink[--m_nFreeLinkIndex]	= pNetLink;
		return;
	}
#elif defined(__linux)
	int nFlags = fcntl(pNetLink->m_nSock, F_GETFL, 0);
	if (nFlags < 0 || fcntl(pNetLink->m_nSock, F_SETFL, nFlags | O_NONBLOCK | O_ASYNC ) < 0)
	{
		g_pFileLog->WriteLog("Set socket async failed! errno=%d\n", errno);
		closesocket(pNetLink->m_nSock);
		m_pFreeLink[--m_nFreeLinkIndex]	= pNetLink;
		return;
	}
#elif defined(__APPLE__)
#endif

	pNetLink->Connected();
}

void CClientNetwork::CloseNetLink(CTcpConnection *pNetLink)
{
	pNetLink->Disconnect();

	m_pFreeLink[--m_nFreeLinkIndex]	= pNetLink;

	//m_pDisconnectFun(m_pFunParam, pNetLink->m_uConnID);
}

IClientNetwork *CreateClientNetwork(
	unsigned int uLinkCount,
	unsigned int uMaxSendBuff,
	unsigned int uMaxReceiveBuff,
	unsigned int uMaxTempSendBuff,
	unsigned int uMaxTempReceiveBuff,
	void *lpParm,
	const bool bThread,
	const unsigned int uSleepFrame
	)
{
	CClientNetwork	*pClient = new CClientNetwork();
	if (NULL == pClient)
		return NULL;

	if (!pClient->Initialize(uLinkCount, uMaxSendBuff, uMaxReceiveBuff, uMaxTempSendBuff, uMaxTempReceiveBuff, lpParm, bThread, uSleepFrame))
	{
		pClient->Release();
		return NULL;
	}

	return pClient;
}
