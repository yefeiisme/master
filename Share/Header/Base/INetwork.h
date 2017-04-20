#ifndef __I_NETWORK_H_
#define __I_NETWORK_H_

class ITcpConnection
{
public:
	virtual bool			IsConnectSuccess() = 0;									// 只用于异步Client去判断是否已经连接上Server，其它情况都不用调用此函数
	virtual bool			IsConnect() = 0;										// 用于在逻辑层判断是否是连接状态
	virtual const void		*GetPack(unsigned int &uPackLen) = 0;					// 获取已收到的网络数据包
	virtual bool			PutPack(const void *pPack, unsigned int uPackLen) = 0;	// 发送网络数据包
	virtual void			ShutDown() = 0;											// 断开操作：用于外部（逻辑层）主动发起的断开连接
	virtual const char		*GetIP() = 0;
};

class IServerNetwork
{
public:
	virtual void			Stop() = 0;				// 退出时调用（用于网络线程退出）
	virtual void			Release() = 0;			// 释放内存空间
	virtual void			CloseAcceptor() = 0;	// 关闭服务端的accept功能
	virtual void			DoNetworkAction() = 0;	// 网络模块未开启独立线程的时候调用（每次循环的时候调用）
};

class IClientNetwork
{
public:
	virtual void			Stop() = 0;													// 退出时调用（用于网络线程退出）
	virtual void			Release() = 0;												// 释放内存空间
	virtual ITcpConnection	*ConnectTo(char *strAddr, const unsigned short usPort) = 0;	// 连接服务器
	virtual void			DoNetworkAction() = 0;										// 网络模块未开启独立线程的时候调用（每次循环的时候调用）
};

typedef void(*CALLBACK_SERVER_EVENT)(void *lpParam, ITcpConnection *pTcpConnection);

IServerNetwork *CreateServerNetwork(
	const unsigned short usPort,				// 端口号
	void *lpParam,								// 回调函数的参数
	CALLBACK_SERVER_EVENT pfnConnectCallBack,	// 连接成功后的回调函数
	const unsigned int uConnectionNum,			// 最大连接数
	const unsigned int uSendBufferLen,			// 每个连接发送缓冲区的大小
	const unsigned int uRecvBufferLen,			// 每个连接接收缓冲区的大小
	const unsigned int uTempSendBufferLen,		// 最大发送包的大小
	const unsigned int uTempRecvBufferLen,		// 最大接收包的大小
	const bool bThread,							// 是否开启网络线程
	const unsigned int uSleepFrame				// 线程的Sleep时间
	);
IClientNetwork *CreateClientNetwork(
	const unsigned int uConnectionNum,			// 最大连接数
	const unsigned int uSendBufferLen,			// 每个连接发送缓冲区的大小
	const unsigned int uRecvBufferLen,			// 每个连接接收缓冲区的大小
	const unsigned int uTempSendBufferLen,		// 最大发送包的大小
	const unsigned int uTempRecvBufferLen,		// 最大接收包的大小
	void *lpParm,								// 回调函数的参数
	const bool bThread,							// 是否开启网络线程
	const unsigned int uSleepFrame				// 线程的Sleep时间
	);

#endif
