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

	m_uMaxConnCount		= 0;
	m_pConnList			= NULL;
	m_pFreeConn			= NULL;
	m_uFreeConnIndex	= 0;

	m_uRecvBufSize		= 0;
	m_uSendBufSize		= 0;

	m_listActiveConn.clear();
	m_listWaitConnectedConn.clear();
	m_listCloseWaitConn.clear();

	m_bRunning			= false;
}

CClientNetwork::~CClientNetwork()
{
	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		m_pConnList[nIndex].ShutDown();
		m_pConnList[nIndex].Disconnect();
	}

	SAFE_DELETE_ARR(m_pConnList);
	m_uMaxConnCount	= 0;
	SAFE_DELETE_ARR(m_pFreeConn);
	m_uFreeConnIndex	= 0;

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
	m_uMaxConnCount	= uClientCount;
	m_uRecvBufSize	= uRecvBuffLen;
	m_uSendBufSize	= uSendBuffLen;

#if defined(WIN32) || defined(WIN64)
	WSADATA	wsaData;
	WORD	wVersionRequested	= MAKEWORD(2, 2);
	int		nError				= WSAStartup(wVersionRequested, &wsaData);

	if (nError != 0)
		return false;
#endif

	m_pConnList = new CTcpConnection[m_uMaxConnCount];
	if (NULL == m_pConnList)
		return false;

	m_pFreeConn	= new CTcpConnection*[m_uMaxConnCount];
	if (NULL == m_pFreeConn)
		return false;

	for (unsigned int uiIndex = 0; uiIndex < m_uMaxConnCount; ++uiIndex)
	{
		if (!m_pConnList[uiIndex].Initialize(m_uRecvBufSize, m_uSendBufSize, uTempRecvBuffLen, uTempSendBuffLen))
			return false;

		m_pConnList[uiIndex].m_uConnID	= uiIndex;

		m_pFreeConn[uiIndex]			= &m_pConnList[uiIndex];
	}

	m_pFunParam	= lpParm;

	if (bThread)
	{
		m_bRunning	= true;
		// 开启线程，执行DoNetworkAction
		// ...
	}

	return true;
}

void CClientNetwork::Release()
{
	delete this;
}

ITcpConnection *CClientNetwork::ConnectTo(char* strAddress, const unsigned short usPort)
{
	if (m_uFreeConnIndex >= m_uMaxConnCount)
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
				CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

				++m_uFreeConnIndex;

				pNewLink->ReInit(nNewSock, true);

				pNewLink->AllConnected();

				m_listActiveConn.push_back(pNewLink);

				return pNewLink;
			}
#if defined(WIN32) || defined(WIN64)
			else if (WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux)
			else if (EINPROGRESS == errno)
#elif defined(__APPLE__)
#endif
			{
				CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

				++m_uFreeConnIndex;

				pNewLink->ReInit(nNewSock, true);

				pNewLink->TcpConnected();

				m_listWaitConnectedConn.push_back(pNewLink);

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
				CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

				++m_uFreeConnIndex;

				pNewLink->ReInit(nNewSock);

				pNewLink->AllConnected();

				m_listActiveConn.push_back(pNewLink);

				return pNewLink;
			}
#if defined(WIN32) || defined(WIN64)
			else if (WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux)
			else if (EINPROGRESS == errno)
#elif defined(__APPLE__)
#endif
			{
				CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

				++m_uFreeConnIndex;

				pNewLink->ReInit(nNewSock);

				pNewLink->TcpConnected();

				m_listWaitConnectedConn.push_back(pNewLink);

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
	ProcessConnectedConnection();

	ProcessWaitConnectConnection();

	ProcessWaitCloseConnection();
}

int CClientNetwork::SetNoBlocking(CTcpConnection *pTcpConnection)
{
#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	return (ioctlsocket(pTcpConnection->m_nSock, FIONBIO, &ulNonBlock) == SOCKET_ERROR);
#elif defined(__linux)
	int nFlags = fcntl(pTcpConnection->m_nSock, F_GETFL, 0);
	if (nFlags < 0 || fcntl(pTcpConnection->m_nSock, F_SETFL, nFlags | O_NONBLOCK | O_ASYNC) < 0)
		return -1;

	return 0;
	//epoll_event ev	={ 0 };

	//ev.data.ptr	= pTcpConnection;
	//ev.events	= EPOLLIN | EPOLLET | EPOLLPRI | EPOLLHUP | EPOLLERR;

	//return epoll_ctl(m_nepfd, EPOLL_CTL_ADD, pTcpConnection->m_nSock, &ev);
#elif defined(__APPLE__)
#endif
}

void CClientNetwork::RemoveConnection(CTcpConnection *pTcpConnection)
{
//#if defined(__linux)
//	int ret = epoll_ctl(m_nepfd, EPOLL_CTL_DEL, pTcpConnection->m_nSock, NULL);
//#elif defined(__APPLE__)
//#endif
	pTcpConnection->Disconnect();
}

void CClientNetwork::ProcessConnectedConnection()
{
	timeval			timeout			= {0, 0};
	CTcpConnection	*pTcpConnection	= NULL;

	for (list<CTcpConnection*>::iterator Iter = m_listActiveConn.begin(); Iter != m_listActiveConn.end();)
	{
		pTcpConnection	= *Iter;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_WriteSet);
		FD_ZERO(&m_ErrorSet);

		FD_SET(pTcpConnection->m_nSock, &m_ReadSet);
		FD_SET(pTcpConnection->m_nSock, &m_WriteSet);
		FD_SET(pTcpConnection->m_nSock, &m_ErrorSet);

		if (select(1024, &m_ReadSet, &m_WriteSet, &m_ErrorSet, &timeout) <= 0)
		{
			++Iter;
			return;
		}

		if (FD_ISSET(pTcpConnection->m_nSock, &m_ErrorSet))
		{
			RemoveConnection(pTcpConnection);
			m_listCloseWaitConn.push_back(pTcpConnection);
			Iter	= m_listActiveConn.erase(Iter);
			continue;
		}

		if (FD_ISSET(pTcpConnection->m_nSock, &m_ReadSet))
		{
			if (pTcpConnection->RecvData() == -1)
			{
				RemoveConnection(pTcpConnection);
				m_listCloseWaitConn.push_back(pTcpConnection);
				Iter	= m_listActiveConn.erase(Iter);
				continue;
			}
		}

		if (FD_ISSET(pTcpConnection->m_nSock, &m_WriteSet))
		{
			if (pTcpConnection->SendData() == -1)
			{
				RemoveConnection(pTcpConnection);
				m_listCloseWaitConn.push_back(pTcpConnection);
				Iter	= m_listActiveConn.erase(Iter);
				continue;
			}
		}

		++Iter;
	}
}

void CClientNetwork::ProcessWaitConnectConnection()
{
	timeval			timeout			= {0, 0};
	CTcpConnection	*pTcpConnection	= NULL;

	for (list<CTcpConnection*>::iterator Iter = m_listWaitConnectedConn.begin(); Iter != m_listWaitConnectedConn.end(); ++Iter)
	{
		pTcpConnection	= *Iter;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_WriteSet);

		FD_SET(pTcpConnection->m_nSock, &m_ReadSet);
		FD_SET(pTcpConnection->m_nSock, &m_WriteSet);

		if (select(0, &m_ReadSet, &m_WriteSet, NULL, &timeout) <= 0)
		{
			++Iter;
			continue;
		}

		if (!FD_ISSET(pTcpConnection->m_nSock, &m_WriteSet) && !FD_ISSET(pTcpConnection->m_nSock, &m_ReadSet))
		{
			++Iter;
			continue;
		}

		int nError = 0;
		socklen_t len = sizeof(nError);
#if defined(WIN32) || defined(WIN64)
		if (getsockopt(pTcpConnection->m_nSock, SOL_SOCKET, SO_ERROR, (char*)&nError, &len) < 0)
#elif defined(__linux)
		if (getsockopt(pTcpConnection->m_nSock, SOL_SOCKET, SO_ERROR, &nError, &len) < 0)
#elif defined(__APPLE__)
#endif
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			continue;
		}

		if (nError != 0)
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("Connnect Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("Connnect Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			continue;
		}

		if (-1 == SetNoBlocking(pTcpConnection))
		{
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			continue;
		}

		pTcpConnection->ConnectSuccess();

		m_listActiveConn.push_back(pTcpConnection);

		Iter	= m_listWaitConnectedConn.erase(Iter);
	}
}

void CClientNetwork::ProcessWaitCloseConnection()
{
	CTcpConnection	*pTcpConnection	= NULL;

	for (list<CTcpConnection*>::iterator Iter = m_listCloseWaitConn.begin(); Iter != m_listCloseWaitConn.end();)
	{
		pTcpConnection	= *Iter;
		if (pTcpConnection->IsLogicConnected())
		{
			++Iter;
			continue;
		}

		AddAvailableConnection(pTcpConnection);

		Iter	= m_listCloseWaitConn.erase(Iter);
	}
}

bool CClientNetwork::IsConnectSuccess(CTcpConnection *pTcpConnection)
{
	timeval	timeout	= {0, 0};

	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);

	FD_SET(pTcpConnection->m_nSock, &m_ReadSet);
	FD_SET(pTcpConnection->m_nSock, &m_WriteSet);

	if (select(0, &m_ReadSet, &m_WriteSet, NULL, &timeout) <= 0)
		return false;

	if (!FD_ISSET(pTcpConnection->m_nSock, &m_WriteSet) && !FD_ISSET(pTcpConnection->m_nSock, &m_ReadSet))
		return false;

	int nError = 0;
	socklen_t len = sizeof(nError);
#if defined(WIN32) || defined(WIN64)
	if (getsockopt(pTcpConnection->m_nSock, SOL_SOCKET, SO_ERROR, (char*)&nError, &len) < 0)
#elif defined(__linux)
	if (getsockopt(pTcpConnection->m_nSock, SOL_SOCKET, SO_ERROR, &nError, &len) < 0)
#elif defined(__APPLE__)
#endif
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		closesocket(pTcpConnection->m_nSock);
		AddAvailableConnection(pTcpConnection);
		return false;
	}

	if (nError != 0)
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("Connnect Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("Connnect Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		closesocket(pTcpConnection->m_nSock);
		AddAvailableConnection(pTcpConnection);
		return false;
	}
#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	if (ioctlsocket(pTcpConnection->m_nSock, FIONBIO, &ulNonBlock) == SOCKET_ERROR)
	{
		g_pFileLog->WriteLog("Set socket async failed! errno=%d\n", WSAGetLastError());
		closesocket(pTcpConnection->m_nSock);
		AddAvailableConnection(pTcpConnection);
		return false;
	}
#elif defined(__linux)
	int nFlags = fcntl(pTcpConnection->m_nSock, F_GETFL, 0);
	if (nFlags < 0 || fcntl(pTcpConnection->m_nSock, F_SETFL, nFlags | O_NONBLOCK | O_ASYNC ) < 0)
	{
		g_pFileLog->WriteLog("Set socket async failed! errno=%d\n", errno);
		closesocket(pTcpConnection->m_nSock);
		AddAvailableConnection(pTcpConnection);
		return false;
	}
#elif defined(__APPLE__)
#endif

	pTcpConnection->ConnectSuccess();

	return true;
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
