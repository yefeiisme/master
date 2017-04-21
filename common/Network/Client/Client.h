#ifndef _ICLIENT_H_
#define _ICLIENT_H_

#include "INetwork.h"
#include <list>
using namespace std;

class CTcpConnection;

class CClientNetwork : public IClientNetwork
{
private:
	fd_set					m_ReadSet;
	fd_set					m_WriteSet;
	fd_set					m_ErrorSet;

	unsigned int			m_uMaxConnCount;
	CTcpConnection			*m_pConnList;
	CTcpConnection			**m_pFreeConn;			// 当前处理空闲状态的CNetLink索引数组
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink的索引，类似list的iterator用法

	unsigned int			m_uRecvBufSize;
	unsigned int			m_uSendBufSize;

	void					*m_pFunParam;

	// 暂时先用这个list，后面改为TList
	// ...
	list<CTcpConnection*>	m_listActiveConn;
	list<CTcpConnection*>	m_listWaitConnectedConn;
	list<CTcpConnection*>	m_listCloseWaitConn;

	bool					m_bRunning;
public:
	CClientNetwork();
	~CClientNetwork();

	bool					Initialize(
										unsigned int uClientCount,
										unsigned int uSendBuffLen,
										unsigned int uRecvBuffLen,
										unsigned int uTempSendBuffLen,
										unsigned int uTempRecvBuffLen,
										void *lpParm,
										const bool bThread,
										const unsigned int uThreadFrame
										);
	inline void				Stop()
	{
		m_bRunning	= false;
	}
	void					Release();
	ITcpConnection			*ConnectTo(char *strAddr, const unsigned short usPort);
	void					DoNetworkAction();
private:
	inline void				AddAvailableConnection(CTcpConnection *pConnection)
	{
		if (m_uFreeConnIndex)
		{
			m_pFreeConn[--m_uFreeConnIndex]	= pConnection;
		}
	}

	int						SetNoBlocking(CTcpConnection *pTcpConnection);
	void					RemoveConnection(CTcpConnection *pTcpConnection);

	void					ProcessConnectedConnection();
	void					ProcessWaitConnectConnection();
	void					ProcessWaitCloseConnection();
	bool					IsConnectSuccess(CTcpConnection *pNetLink);
};

#endif
