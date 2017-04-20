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
	CTcpConnection			*m_pLinkList;
	CTcpConnection			**m_pFreeLink;			// ��ǰ�������״̬��CNetLink��������
	int						m_nFreeLinkIndex;		// m_pFreeLink������������list��iterator�÷�

	unsigned int			m_uRecvBufSize;
	unsigned int			m_uSendBufSize;

	void					*m_pFunParam;

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
	void					ProcessConnectedLink(CTcpConnection *pNetLink);
	void					ProcessWaitConnectLink(CTcpConnection *pNetLink);
	void					CloseNetLink(CTcpConnection *pNetLink);
};

#endif
