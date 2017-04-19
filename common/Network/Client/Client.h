#ifndef _ICLIENT_H_
#define _ICLIENT_H_

class CClientNetwork;
class IClientNetwork;

class CClientNetwork : public IClientNetwork
{
private:
	fd_set					m_ReadSet;
	fd_set					m_WriteSet;
	fd_set					m_ErrorSet;

	unsigned int			m_uMaxLinkCount;
	CNetworkConection		*m_pLinkList;
	CNetworkConection		**m_pFreeLink;			// 当前处理空闲状态的CNetLink索引数组
	int						m_nFreeLinkIndex;		// m_pFreeLink的索引，类似list的iterator用法

	unsigned int			m_uRecvBufSize;
	unsigned int			m_uSendBufSize;

	CALLBACK_CLIENT_EVENT	m_pConnectedFun;
	CALLBACK_CLIENT_EVENT	m_pDisconnectFun;
	void					*m_pFunParam;
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
										CALLBACK_CLIENT_EVENT pfnConnected,
										CALLBACK_CLIENT_EVENT pfnDisconnect
										);
	void					Release();
	bool					ConnectTo(char *strAddr, const unsigned short usPort);
	void					Disconnect(const unsigned int uConnID);
	int						SendPackToServer(const unsigned int uConnID, const void *pData, const unsigned int uLength);
	const void				*GetPackFromServer(const unsigned int uConnID, unsigned int &uLength);
	void					DoNetworkAction();
private:
	void					ProcessConnectedLink(CNetworkConection *pNetLink);
	void					ProcessWaitConnectLink(CNetworkConection *pNetLink);
	void					CloseNetLink(CNetworkConection *pNetLink);
};

#endif
