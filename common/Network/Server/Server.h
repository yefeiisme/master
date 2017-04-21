#ifndef _I_SERVER_H_
#define _I_SERVER_H_

#include <list>
#include "../NetworkHead.h"

using namespace std;

class CServerNetwork : public IServerNetwork
{
private:
	CALLBACK_SERVER_EVENT	m_pfnConnectCallBack;
	void					*m_pFunParam;

	CTcpConnection			*m_pListenLink;

	CTcpConnection			*m_pTcpConnection;		// ���е���������
	CTcpConnection			**m_pFreeConn;			// ��ǰ�������״̬��CNetLink��������

	unsigned int			m_uMaxConnCount;
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink������������list��iterator�÷�

	unsigned int			m_uClientRecvBuffSize;
	unsigned int			m_uClientSendBuffSize;

#ifdef __linux
	int						m_nepfd;
#elif defined(WIN32) || defined(WIN64)
	fd_set					m_ReadSet;
	fd_set					m_ErrorSet;
#elif defined(__APPLE__)
#endif

	// ��ʱ�������list�������ΪTList
	// ...
	list<CTcpConnection*>	m_listActiveConn;
	list<CTcpConnection*>	m_listCloseWaitConn;

	bool					m_bRunning;
private:
	inline CTcpConnection	*GetNewConnection()
	{
		return m_uFreeConnIndex >= m_uMaxConnCount ? NULL : m_pFreeConn[m_uFreeConnIndex++];
	}

	inline void				AddAvailableConnection(CTcpConnection *pConnection)
	{
		if (m_uFreeConnIndex)
		{
			m_pFreeConn[--m_uFreeConnIndex]	= pConnection;
		}
	}

	int						SetNoBlocking(CTcpConnection *pTcpConnection);
	void					AcceptClient(const SOCKET nNewSocket);

	void					RemoveConnection(CTcpConnection *pTcpConnection);
	void					CloseConnection(CTcpConnection *pTcpConnection);
	void					ReadAction();
	void					WriteAction();
	void					CloseAction();

	void					ThreadFunc();
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
