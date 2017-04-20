#ifndef __I_NETWORK_H_
#define __I_NETWORK_H_

class ITcpConnection
{
public:
	virtual bool			IsConnectSuccess() = 0;									// ֻ�����첽Clientȥ�ж��Ƿ��Ѿ�������Server��������������õ��ô˺���
	virtual bool			IsConnect() = 0;										// �������߼����ж��Ƿ�������״̬
	virtual const void		*GetPack(unsigned int &uPackLen) = 0;					// ��ȡ���յ����������ݰ�
	virtual bool			PutPack(const void *pPack, unsigned int uPackLen) = 0;	// �����������ݰ�
	virtual void			ShutDown() = 0;											// �Ͽ������������ⲿ���߼��㣩��������ĶϿ�����
	virtual const char		*GetIP() = 0;
};

class IServerNetwork
{
public:
	virtual void			Stop() = 0;				// �˳�ʱ���ã����������߳��˳���
	virtual void			Release() = 0;			// �ͷ��ڴ�ռ�
	virtual void			CloseAcceptor() = 0;	// �رշ���˵�accept����
	virtual void			DoNetworkAction() = 0;	// ����ģ��δ���������̵߳�ʱ����ã�ÿ��ѭ����ʱ����ã�
};

class IClientNetwork
{
public:
	virtual void			Stop() = 0;													// �˳�ʱ���ã����������߳��˳���
	virtual void			Release() = 0;												// �ͷ��ڴ�ռ�
	virtual ITcpConnection	*ConnectTo(char *strAddr, const unsigned short usPort) = 0;	// ���ӷ�����
	virtual void			DoNetworkAction() = 0;										// ����ģ��δ���������̵߳�ʱ����ã�ÿ��ѭ����ʱ����ã�
};

typedef void(*CALLBACK_SERVER_EVENT)(void *lpParam, ITcpConnection *pTcpConnection);

IServerNetwork *CreateServerNetwork(
	const unsigned short usPort,				// �˿ں�
	void *lpParam,								// �ص������Ĳ���
	CALLBACK_SERVER_EVENT pfnConnectCallBack,	// ���ӳɹ���Ļص�����
	const unsigned int uConnectionNum,			// ���������
	const unsigned int uSendBufferLen,			// ÿ�����ӷ��ͻ������Ĵ�С
	const unsigned int uRecvBufferLen,			// ÿ�����ӽ��ջ������Ĵ�С
	const unsigned int uTempSendBufferLen,		// ����Ͱ��Ĵ�С
	const unsigned int uTempRecvBufferLen,		// �����հ��Ĵ�С
	const bool bThread,							// �Ƿ��������߳�
	const unsigned int uSleepFrame				// �̵߳�Sleepʱ��
	);
IClientNetwork *CreateClientNetwork(
	const unsigned int uConnectionNum,			// ���������
	const unsigned int uSendBufferLen,			// ÿ�����ӷ��ͻ������Ĵ�С
	const unsigned int uRecvBufferLen,			// ÿ�����ӽ��ջ������Ĵ�С
	const unsigned int uTempSendBufferLen,		// ����Ͱ��Ĵ�С
	const unsigned int uTempRecvBufferLen,		// �����հ��Ĵ�С
	void *lpParm,								// �ص������Ĳ���
	const bool bThread,							// �Ƿ��������߳�
	const unsigned int uSleepFrame				// �̵߳�Sleepʱ��
	);

#endif
