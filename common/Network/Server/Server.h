#ifndef _I_SERVER_H_
#define _I_SERVER_H_

#include "../NetworkHead.h"

class CServerNetwork : public IServerNetwork
{
private:
#ifdef __linux
	int						m_nepfd;
#else
	fd_set					m_ReadSet;
	fd_set					m_ErrorSet;
#endif
	CALLBACK_SERVER_EVENT	m_pfnConnectCallBack;
	CALLBACK_SERVER_EVENT	m_pfnDisconnectCallBack;
	void					*m_pFunParam;

	CNetworkConection		*m_pListenLink;

	unsigned int			m_uMaxClientCount;
	CNetworkConection		*m_pLinkList;			// ���е���������
	CNetworkConection		**m_pFreeLink;			// ��ǰ�������״̬��CNetLink��������
	unsigned int			m_uFreeLinkIndex;		// m_pFreeLink������������list��iterator�÷�

	unsigned int			m_uClientRecvBuffSize;
	unsigned int			m_uClientSendBuffSize;
private:
	int						SetNoBlocking(CNetworkConection *pNetLink);
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
										CALLBACK_SERVER_EVENT pfnDisconnectCallBack,
										unsigned int uLinkNum,
										unsigned int uSendBufferLen,
										unsigned int uRecvBufferLen,
										unsigned int uTempSendBufferLen,
										unsigned int uTempRecvBufferLen
										);
	void					Release();
	int						SendData(const unsigned int uConnID, const void *pData, const unsigned int uLength);
	const void				*GetPackFromClient(const unsigned int uConnID, unsigned int &uLength);
	inline const char		*GetClientIP(const unsigned int uConnID)
	{
		if (uConnID >= m_uMaxClientCount)
			return NULL;

		return m_pLinkList[uConnID].GetIP();
	}
	bool					ShutdownClient(const unsigned int uConnID);
	void					CloseAcceptor();
	void					DoNetworkAction();
};

#endif
