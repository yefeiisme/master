#ifndef __NET_LINK_H_
#define __NET_LINK_H_

#include "INetwork.h"
#include "../NetworkHead.h"
#define MAX_IP_LEN	128

enum E_NET_LINK_STATE
{
	NET_LINK_STATE_DISCONNNECT = -1,
	NET_LINK_STATE_CONNECT,
	NET_LINK_STATE_WAI_CONNECT,
};

class CTcpConnection : public ITcpConnection
{
public:
	SOCKET						m_nSock;
	unsigned int				m_uConnID;
private:
	E_NET_LINK_STATE			m_eStatus;

	/**********************���ͻ�����**********************/
	char						*m_pSendBuf;
	char						*m_pTempSendBuf;
	char						*m_pFlush;
	char						*m_pSend;
	unsigned int				m_uSendBufLen;
	unsigned int				m_uTempSendBufLen;
	/**********************���ͻ�����**********************/
	
	/**********************���ջ�����**********************/
	char						*m_pRecvBuf;
	char						*m_pTempRecvBuf;
	char						*m_pPack;
	char						*m_pRecv;
	char						*m_pUnreleased;
	unsigned int				m_uRecvBufLen;
	unsigned int				m_uTempRecvBufLen;
	/**********************���ջ�����**********************/

	bool						m_bSocketConnected;		// ���������Ƿ�����״̬
	bool						m_bLogicConnected;		// �ⲿ�߼��Ƿ�����״̬
	bool						m_bConnectSuccess;		// �첽�����Ƿ����
	bool						m_bIPV6;				// �Ƿ���IPV6�����ַ
public:
	CTcpConnection();
	~CTcpConnection();

	inline bool					IsSocketConnected()
	{
		return m_bSocketConnected;
	}

	inline bool					IsLogicConnected()
	{
		return m_bLogicConnected;
	}

	inline bool					IsConnectSuccess()
	{
		return m_bConnectSuccess;
	}

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
	inline void					ReInit(const int nSocket, const bool bIPV6 = false)
	{
		m_pUnreleased	= m_pRecv = m_pPack = m_pRecvBuf;
		m_pFlush		= m_pSend = m_pSendBuf;
		m_nSock			= nSocket;
		m_bIPV6			= bIPV6;
	}

	const char					*GetIP();

	int							RecvData();
	int							SendData();
	bool						PutPack(const void *pPack, unsigned int uPackLen);
	const void					*GetPack(unsigned int &uPackLen);

	inline void					ShutDown()
	{
		m_bLogicConnected	= false;
	}

	void						Disconnect();
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
