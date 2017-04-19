#ifndef __NET_LINK_H_
#define __NET_LINK_H_

#include "../NetworkHead.h"
#define MAX_IP_LEN	128

enum E_NET_LINK_STATE
{
	NET_LINK_STATE_DISCONNNECT = -1,
	NET_LINK_STATE_CONNECT,
	NET_LINK_STATE_WAI_CONNECT,
};

class CNetworkConection
{
public:
	SOCKET						m_nSock;
	unsigned int				m_uConnID;
private:
	E_NET_LINK_STATE			m_eStatus;

	/**********************发送缓冲区**********************/
	char						*m_pSendBuf;
	char						*m_pTempSendBuf;
	char						*m_pFlush;
	char						*m_pSend;
	unsigned int				m_uSendBufLen;
	unsigned int				m_uTempSendBufLen;
	/**********************发送缓冲区**********************/
	
	/**********************接收缓冲区**********************/
	char						*m_pRecvBuf;
	char						*m_pTempRecvBuf;
	char						*m_pPack;
	char						*m_pRecv;
	char						*m_pUnreleased;
	unsigned int				m_uRecvBufLen;
	unsigned int				m_uTempRecvBufLen;
	/**********************接收缓冲区**********************/

	bool						m_bIPV6;
public:
	CNetworkConection();
	~CNetworkConection();

	inline bool					IsConnect()
	{
		return NET_LINK_STATE_CONNECT == m_eStatus;
	}

	inline bool					IsDisconnected()
	{
		return NET_LINK_STATE_DISCONNNECT == m_eStatus;
	}

	inline bool					IsWaitConnect()
	{
		return NET_LINK_STATE_WAI_CONNECT == m_eStatus;
	}

	bool						Initialize(unsigned int uRecvBufferLen, unsigned int uSendBufferLen, unsigned int uTempRecvBufLen, unsigned int uTempSendBufLen);
	bool						ReInit(const int nSocket, const bool bIPV6 = false);
	const char					*GetIP();

	int							RecvData();
	int							FlushData();
	int							PutPack(const void *pPack, unsigned int uPackLen);
	const void					*GetPack(unsigned int &uPackLen);
	bool						Disconnect();
	inline void					WaitConnect()
	{
		m_eStatus	= NET_LINK_STATE_WAI_CONNECT;
	}
	inline void					Connected()
	{
		m_eStatus	= NET_LINK_STATE_CONNECT;
	}
};

#endif
