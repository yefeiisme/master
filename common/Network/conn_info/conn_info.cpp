#include "conn_info.h"
#include "IFileLog.h"

CTcpConnection::CTcpConnection()
{
	m_nSock				= INVALID_SOCKET;
	m_uConnID			= 0;

	m_eStatus			= NET_LINK_STATE_DISCONNNECT;

	m_pSendBuf			= NULL;
	m_pTempSendBuf		= NULL;
	m_pFlush			= NULL;
	m_pSend				= NULL;
	m_uSendBufLen		= 0;
	m_uTempSendBufLen	= 0;

	m_pRecvBuf			= NULL;
	m_pTempRecvBuf		= NULL;
	m_pPack				= NULL;
	m_pRecv				= NULL;
	m_pUnreleased		= NULL;
	m_uRecvBufLen		= 0;
	m_uTempRecvBufLen	= 0;

	m_bSocketConnected	= false;
	m_bLogicConnected	= false;
	m_bConnectSuccess	= false;
	m_bIPV6				= false;
}

CTcpConnection::~CTcpConnection()
{
	SAFE_DELETE_ARR(m_pSendBuf);
	SAFE_DELETE_ARR(m_pRecvBuf);
	SAFE_DELETE_ARR(m_pTempRecvBuf);
	SAFE_DELETE_ARR(m_pTempSendBuf);
}

bool CTcpConnection::Initialize(unsigned int uRecvBufferLen, unsigned int uSendBufferLen, unsigned int uTempRecvBufLen, unsigned int uTempSendBufLen)
{
	if (0 == uRecvBufferLen || 0 == uSendBufferLen)
		return false;

	m_uRecvBufLen	= uRecvBufferLen;
	m_uSendBufLen	= uSendBufferLen;

	m_pRecvBuf		= new char[m_uRecvBufLen];
	if (!m_pRecvBuf)
		return false;

	memset(m_pRecvBuf, 0, sizeof(char)*m_uRecvBufLen);

	m_pSendBuf		= new char[m_uSendBufLen];
	if (!m_pSendBuf)
		return false;

	memset(m_pSendBuf, 0, sizeof(char)*m_uSendBufLen);

	// init temp packet buffer, will used to hold a packet which will be 
	// returned to caller, when the pack wrapped in the buffer
	m_uTempRecvBufLen	= uTempRecvBufLen;
	m_pTempRecvBuf		= new char[m_uTempRecvBufLen];
	if (!m_pTempRecvBuf)
		return false;

	memset(m_pTempRecvBuf, 0, sizeof(char)*m_uTempRecvBufLen);

	m_uTempSendBufLen	= uTempSendBufLen;
	m_pTempSendBuf		= new char[m_uTempSendBufLen];
	if (!m_pTempSendBuf)
		return false;

	memset(m_pTempSendBuf, 0, sizeof(char)*m_uTempSendBufLen);

	return true;
}

const char *CTcpConnection::GetIP()
{
	if (m_bIPV6)
	{
		// 后面再加吧，只有服务器会用，现在主攻ios版，所以不用管这个
		// ...
		return NULL;
	}
	else
	{
		sockaddr_in	tagClientAddr;

		socklen_t nAddrLen = sizeof(tagClientAddr);

		if (getpeername(m_nSock, (sockaddr*)&tagClientAddr, &nAddrLen))
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("%s:%d, CNetLink::reinit Client[%4u] getpeername Error[%d]\n", __FILE__, __LINE__, m_uConnID, WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("%s:%d, CNetLink::reinit Client[%4u] getpeername Error[%d]\n", __FILE__, __LINE__, m_uConnID, errno);
#elif defined(__APPLE__)
#endif
			return false;
		}

		char	*strIP	= inet_ntoa(tagClientAddr.sin_addr);
		if (NULL == strIP)
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("%s:%d, CNetLink::reinit Client[%4u] inet_ntoa Error[%d]\n", __FILE__, __LINE__, m_uConnID, WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("%s:%d, CNetLink::reinit Client[%4u] inet_ntoa Error[%d]\n", __FILE__, __LINE__, m_uConnID, errno);
#elif defined(__APPLE__)
#endif
			return false;
		}

		return strIP;
	}
}

int CTcpConnection::RecvData()
{
	if (!m_bSocketConnected)
		return 0;

	char	*pRecvStart	= m_pRecv;
	char	*pRecvEnd	= m_pUnreleased;
	int		nReadedBytes;
	int		nMaxReadBytes;

	if (pRecvStart >= pRecvEnd)
	{
		// buffer is empty or not wrapped
		// so, fill in the tail, and then, 
		// if the tail full, wrap to the head
		nMaxReadBytes	= m_pRecvBuf + m_uRecvBufLen - pRecvStart;
		nReadedBytes	= recv(m_nSock, pRecvStart, nMaxReadBytes, MSG_NOSIGNAL);

		if (nReadedBytes > 0 && nReadedBytes < nMaxReadBytes)
		{
			m_pRecv = pRecvStart + nReadedBytes;
			return nReadedBytes;
		}
		else if (nReadedBytes == nMaxReadBytes)
		{
			pRecvStart = m_pRecvBuf;
			// not return, fall down to next block to read more data
		}
		else if (nReadedBytes == 0)
		{
			return -1;
		}
		else if (errno != EAGAIN)
		{
#if defined(WIN32) || defined(WIN64)
			int	nError	= WSAGetLastError();
			if (WSAECONNRESET != nError)
#elif defined(__linux)
			int	nError	= errno;
			if (ECONNRESET != nError)
#elif defined(__APPLE__)
#endif
			{
				g_pFileLog->WriteLog("file: %s, line: %d, errno: %d\n", __FILE__, __LINE__, nError);
			}
			return -1;
		}
		else
		{
			return 0;
		}
	}

	nMaxReadBytes	= pRecvEnd - pRecvStart;
	nReadedBytes	= recv(m_nSock, pRecvStart, nMaxReadBytes, MSG_NOSIGNAL);

	if (nReadedBytes > 0 && nReadedBytes < nMaxReadBytes)
	{
		m_pRecv = pRecvStart + nReadedBytes;
		return nReadedBytes;
	}
	else if (nReadedBytes == nMaxReadBytes)
	{
		// Overflow
		g_pFileLog->WriteLog("ERROR on CNetLink::recvdata Client[%4u] recvbuffer overflow\n", m_uConnID);
		return -1;
	}
	else if (nReadedBytes == 0)
	{
		return -1;
	}
	else if (errno != EAGAIN)
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("file: %s, line: %d, errno: %d\n", __FILE__, __LINE__, WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("file: %s, line: %d, errno: %d\n", __FILE__, __LINE__, errno);
#elif defined(__APPLE__)
#endif
		return -1;
	}
	else
	{
		m_pRecv = pRecvStart;
		return 0;
	}
}

const void *CTcpConnection::GetPack(unsigned int &uPackLen)
{
	if (!m_bLogicConnected)
		return NULL;
	
	char			*pPackStart	= m_pPack;
	char			*pPackEnd	= m_pRecv;
	unsigned short	usPackLength;
	
	// get data and tail length
	int nDataLen = pPackEnd >= pPackStart ? pPackEnd - pPackStart : m_uRecvBufLen + pPackEnd - pPackStart;
	int nTailLen = m_uRecvBufLen + m_pRecvBuf - pPackStart;

	// verify the length is ok.
	if (nDataLen < sizeof(unsigned short))
	{
		return NULL;
	}
	
	// get packet length, since the data length is larger than 2
	// we can make sure that the following 2 bytes are data
	if (nTailLen > sizeof(unsigned short))
	{
		usPackLength	= *(unsigned short *)pPackStart;
		pPackStart		+= sizeof(unsigned short);
		nTailLen		= nTailLen - sizeof(unsigned short);
	}
	else if (nTailLen == sizeof(unsigned short))
	{
		usPackLength	= *(unsigned short *)pPackStart;
		pPackStart		= m_pRecvBuf;
	}
	else
	{
		usPackLength	= *(unsigned char*)pPackStart;
		usPackLength	+= (*(unsigned char*)m_pRecvBuf) << 8;
		pPackStart		= m_pRecvBuf + 1;
	}
	
	// check if the pack length is correct, if not, close the client
	if (usPackLength <= sizeof(unsigned short))
	{
		m_pPack = m_pRecv;
		return NULL;
	}
	
	// check if there is a whole packet in the buffer
	if (usPackLength > nDataLen)
	{
		// only parts of the packet is received, do nothing, just return;
		return NULL;
	}
	
	// substract the length of header
	usPackLength -= sizeof(unsigned short);
	
	// return the ptr
	char *pRetBuf = NULL;
	if (pPackEnd > pPackStart || usPackLength < nTailLen)
	{
		// Not wrap
		m_pPack			= pPackStart + usPackLength;
		uPackLen		= usPackLength;
		m_pUnreleased	= pPackStart;
		pRetBuf			= pPackStart;
	}
	else if (usPackLength > nTailLen)
	{
		// wrapped
		if (nTailLen > m_uTempRecvBufLen || usPackLength - nTailLen > m_uTempRecvBufLen - nTailLen)
		{
			g_pFileLog->WriteLog("[%s][%d] Client[%4u]:Recv temp buff overflow tail_length = %d tmp_recvbuf_len = %u, pack_length = %u\n", __FILE__, __LINE__, m_uConnID, nTailLen, m_uTempSendBufLen, uPackLen);
			return NULL;
		}
		memcpy(m_pTempRecvBuf, pPackStart, nTailLen);
		memcpy(m_pTempRecvBuf + nTailLen, m_pRecvBuf, usPackLength - nTailLen);
		m_pUnreleased	= pPackStart;
		m_pPack			= m_pRecvBuf + usPackLength - nTailLen;
		uPackLen		= usPackLength;
		pRetBuf			= m_pTempRecvBuf;
	}
	else if (usPackLength == nTailLen)
	{
		// broken at the end of the buffer
		m_pPack			= m_pRecvBuf;
		uPackLen		= usPackLength;
		m_pUnreleased	= pPackStart;
		pRetBuf			= pPackStart;
	}
	else
	{
		return NULL;
	}

	return pRetBuf;
}

bool CTcpConnection::PutPack(const void* pPack, unsigned int uPackLen)
{
	if (!m_bSocketConnected)
		return false;

	char	*pPutStart	= m_pSend;
	char	*pPutEnd	= m_pFlush;
	char	*pData		= (char*)pPack;

	// add 2 to the pack_len, so it's the total length included the pack header
	uPackLen	+= sizeof(unsigned short);

	int		nEmptyLen	= pPutStart >= pPutEnd ? pPutEnd + m_uSendBufLen - pPutStart : nEmptyLen = pPutEnd - pPutStart;

	// if buffer is full, there are must be something wrong,
	// in this case; we just close the conn;
	if (uPackLen >= nEmptyLen)
	{
		g_pFileLog->WriteLog("[%s][%d] CNetLink::putpack Client[%4u]: Send buffer overflow!!!\n", __FILE__, __LINE__, m_uConnID);
		return false;
	}

	int	nTailLen = m_pSendBuf + m_uSendBufLen - pPutStart;
	
	if (pPutEnd > pPutStart || uPackLen < nTailLen)
	{
		// data will not wrap, copy and return
		*(unsigned short*)pPutStart = (unsigned short)(uPackLen);
		memcpy( pPutStart + sizeof(unsigned short), pData, uPackLen - sizeof(unsigned short) );

		m_pSend = pPutStart + uPackLen;
	}
	else if (uPackLen == nTailLen)
	{
		*(unsigned short*)pPutStart = (unsigned short)(uPackLen);
		memcpy( pPutStart + sizeof(unsigned short), pData, uPackLen - sizeof(unsigned short) );

		m_pSend = m_pSendBuf;
	}
	else
	{
		if (m_uTempSendBufLen < uPackLen - sizeof(unsigned short))
		{
			g_pFileLog->WriteLog("[%s][%d] Client[%4u]:Send temp buff overflow tmp_sendbuf_len = %u, pack_len = %u\n", __FILE__, __LINE__, m_uConnID, m_uTempSendBufLen, uPackLen);
			return false;
		}
		memcpy(m_pTempSendBuf, pData, uPackLen - sizeof(unsigned short));

		// data will wrap
		if (nTailLen > sizeof(unsigned short))
		{
			*(unsigned short*)pPutStart = (unsigned short)(uPackLen);
			memcpy(pPutStart + sizeof(unsigned short), m_pTempSendBuf, nTailLen - sizeof(unsigned short));
			memcpy(m_pSendBuf, m_pTempSendBuf + nTailLen - sizeof(unsigned short), uPackLen - nTailLen);
			m_pSend	= m_pSendBuf + uPackLen - nTailLen;
		}
		else if (nTailLen == sizeof(unsigned short))
		{
			*(unsigned short*)pPutStart = (unsigned short)(uPackLen);
			memcpy(m_pSendBuf, m_pTempSendBuf, uPackLen - sizeof(unsigned short));
			m_pSend	= m_pSendBuf + uPackLen - sizeof(unsigned short);
		}
		else
		{
			// tail_len == 1 
			*(unsigned char*)pPutStart		= uPackLen & 0x0FF;
			*(unsigned char*)m_pSendBuf		= (uPackLen >> 8) & 0xFF;
			memcpy(m_pSendBuf + 1, m_pTempSendBuf, uPackLen - sizeof(unsigned short));
			m_pSend	= m_pSendBuf + uPackLen - 1;
		}
	}
	return true;
}


int CTcpConnection::SendData()
{
	char	*pFlushStart	= m_pFlush;
	char	*pFlushEnd		= m_pSend;
	int		nMaxSendBytes;
	int		nSendedBytes;
	int		nTailLen = 0;

	if (pFlushStart == pFlushEnd)
	{
		// nothing to send
		return 0;
	}

	if (pFlushStart < pFlushEnd)
	{
		nMaxSendBytes	= pFlushEnd - pFlushStart;
		nSendedBytes	= send(m_nSock, pFlushStart, nMaxSendBytes, MSG_NOSIGNAL);

		if (nSendedBytes == nMaxSendBytes)
		{
			m_pFlush = pFlushEnd;
			return 0;
		}
		else if (nSendedBytes >= 0)
		{
			m_pFlush = pFlushStart + nSendedBytes;
			return 0;
		}
	}
	else
	{
		nTailLen		= m_pSendBuf  + m_uSendBufLen - pFlushStart;
		nSendedBytes	= send(m_nSock, pFlushStart, nTailLen, MSG_NOSIGNAL);

		if (nSendedBytes == nTailLen)
		{
			if (pFlushEnd == m_pSendBuf)
			{
				m_pFlush = m_pSendBuf;
				return 0;
			}
			// wrap to the head of buffer
			nMaxSendBytes	= pFlushEnd - m_pSendBuf;
			nSendedBytes	= send(m_nSock, m_pSendBuf, nMaxSendBytes, MSG_NOSIGNAL);

			if (nSendedBytes >= 0)
			{
				m_pFlush = m_pSendBuf + nSendedBytes;
				return 0;
			}
		}
		else if (nSendedBytes >= 0)
		{
			m_pFlush = pFlushStart + nSendedBytes;
			return 0;
		}
	}

	if (errno != EAGAIN)
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("file: %s, line: %d, errno: %d\n", __FILE__, __LINE__, WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("file: %s, line: %d, errno: %d\n", __FILE__, __LINE__, errno);
#elif defined(__APPLE__)
#endif
		return -1;
	}
	else
	{
		// should block, return and wait for next time
		return 0;
	}
	
}

void CTcpConnection::Disconnect()
{
	m_bSocketConnected	= false;
	m_bConnectSuccess	= false;

	if (INVALID_SOCKET != m_nSock)
	{
		closesocket(m_nSock);
		m_nSock		= INVALID_SOCKET;
	}
}
