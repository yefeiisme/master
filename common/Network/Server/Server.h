#ifndef _I_SERVER_H_
#define _I_SERVER_H_

#include "../NetworkHead.h"

class CServerNetwork : public IServerNetwork
{
private:
#ifdef __linux
	int						m_nepfd;
#elif defined(WIN32) || defined(WIN64)
	fd_set					m_ReadSet;
	fd_set					m_ErrorSet;
#elif defined(__APPLE__)
#endif
	CALLBACK_SERVER_EVENT	m_pfnConnectCallBack;
	void					*m_pFunParam;

	CTcpConnection			*m_pListenLink;

	unsigned int			m_uMaxConnCount;
	CTcpConnection			*m_pTcpConnection;		// ���е���������
	CTcpConnection			**m_pFreeConn;			// ��ǰ�������״̬��CNetLink��������
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink������������list��iterator�÷�

	unsigned int			m_uClientRecvBuffSize;
	unsigned int			m_uClientSendBuffSize;

	unsigned int			m_uSleepFrame;
	bool					m_bRunning;
private:
	int						SetNoBlocking(CTcpConnection *pNetLink);
	bool					AcceptClient(const SOCKET nNewSocket);

	bool					DelClient(const unsigned int nNetworkIndex);
	void					ReadAction();
	void					WriteAction();
public:
	CServerNetwork();
	~CServerNetwork();

	bool					Initialize(
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
										);
	void					Stop();
	void					Release();
	void					CloseAcceptor();
	void					DoNetworkAction();
};

#endif
