#ifndef __I_NETWORK_H_
#define __I_NETWORK_H_

enum enumClientConnectInfo 
{
	enumClientConnectCreate,
	enumClientConnectClose
};

enum enumServerConnectInfo
{
	enumServerConnectCreate,
	enumServerConnectClose
};

typedef void (*CALLBACK_SERVER_EVENT)(void *lpParam, const unsigned int uNetID);
typedef void (*CALLBACK_CLIENT_EVENT)(void *lpParam, const unsigned int uNetID);

class IServerNetwork
{
public:
	virtual void		Release() = 0;
	virtual int			SendData(const unsigned int uClientID, const void *pData, const unsigned int uDataLen) = 0;
	virtual const void	*GetPackFromClient(const unsigned int uClientID, unsigned int &uDataLen) = 0;
	virtual const char	*GetClientIP(const unsigned int uClientID) = 0;
	virtual bool		ShutdownClient(const unsigned int uClientID) = 0;
	virtual void		CloseAcceptor() = 0;
	virtual void		DoNetworkAction() = 0;
};

class IClientNetwork
{
public:
	virtual void		Release() = 0;
	virtual bool		ConnectTo(char *strAddr, const unsigned short usPort) = 0;
	virtual void		Disconnect(const unsigned int uConnID) = 0;
	virtual int			SendPackToServer(const unsigned int uConnID, const void *pData, const unsigned int uLen) = 0;
	virtual const void	*GetPackFromServer(const unsigned int uConnID, unsigned int &uLen) = 0;
	virtual void		DoNetworkAction() = 0;
};


IServerNetwork *CreateServerNetwork(
	unsigned short usPort,// �˿ں�
	void *lpParam,// �ص������Ĳ���
	CALLBACK_SERVER_EVENT pfnConnectCallBack,// ���ӳɹ���Ļص�����
	CALLBACK_SERVER_EVENT pfnDisconnectCallBack,// �Ͽ����Ӻ�Ļص�����
	unsigned int uLinkNum,// ���������
	unsigned int uSendBufferLen,// ÿ�����ӷ��ͻ������Ĵ�С
	unsigned int uRecvBufferLen,// ÿ�����ӽ��ջ������Ĵ�С
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen
	);
IClientNetwork *CreateClientNetwork(
	unsigned int uLinkCount,
	unsigned int uMaxSendBuff,
	unsigned int uMaxReceiveBuff,
	unsigned int uMaxTempSendBuff,
	unsigned int uMaxTempReceiveBuff,
	void *lpParm,
	CALLBACK_CLIENT_EVENT pfnConnected,
	CALLBACK_CLIENT_EVENT pfnDisconnect
	);

#endif
