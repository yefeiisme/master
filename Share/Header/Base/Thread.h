#ifndef _I_UTILITIES_H_
#define _I_UTILITIES_H_

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

class CThread
{
private:
	enum enum_status
	{
		new_created,
		running,
		stopped
	};
	enum_status		m_eStatus;
#ifdef WIN32
	HANDLE			m_hThread;
#else
	pthread_t		m_hThread;
	pthread_attr_t	m_ThreadAttr;
#endif
public:
	CThread(int detached);
	virtual ~CThread();
	int				Start();
	//int				Stop(int nKill = 1);
	virtual void	Action() = 0;
	void			Sleep(const unsigned int uiTime);
};

template<class CallObj>
class CThreadObj : public CThread
{
private:
	typedef void (CallObj::*ThreadFun)();
	CallObj*	m_pCObj;
	ThreadFun	m_pThreadFun;
public:
	CThreadObj() : CThread(1), m_pCObj(NULL), m_pThreadFun(NULL)
	{
	}

	~CThreadObj()
	{
	}

	bool		Create(CallObj* pCObj, ThreadFun pTFun)
	{
		m_pCObj			=	pCObj;
		m_pThreadFun	=	pTFun;
		Start();
		return true;
	}

	void		Action()
	{
		(m_pCObj->*m_pThreadFun)();
	}

};

#endif
