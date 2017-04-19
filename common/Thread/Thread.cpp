#include "Thread.h"
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#ifdef WIN32
static DWORD WINAPI ThreadProc(void* lpParameter)
{
	CThread *pThread = (CThread*)lpParameter;
	pThread->Action();
	return 0;
}
#else
static void* ThreadProc(void* lpParameter)
{
	CThread *pThread = (CThread*)lpParameter;
	pThread->Action();
	return 0;
}
#endif

CThread::CThread(int detached)
{
	m_eStatus = new_created;
#ifdef WIN32
#else
	pthread_attr_init(&m_ThreadAttr);
	if (detached)
	{
		pthread_attr_setdetachstate(&m_ThreadAttr, PTHREAD_CREATE_DETACHED);
	}
#endif
}

CThread::~CThread()
{
#ifdef WIN32
	if (m_eStatus == running)
	{
		TerminateThread(m_hThread, 0);
	}
#else
//	if ( status == running )
//	{
//		pthread_cancel(thread_id);
//	}
#endif
}

int CThread::Start()
{
#ifdef WIN32
	if (m_eStatus == new_created)
	{
		m_hThread = CreateThread(NULL, 0, ThreadProc, this, 0, NULL);
		if (m_hThread)
		{
			m_eStatus = running;
			return 0;
		}
	}
#else
	if (m_eStatus == new_created && !pthread_create(&m_hThread, &m_ThreadAttr, ThreadProc, this))
	{
		m_eStatus = running;
		return 0;
	}
#endif
	return -1;
}

//int CThread::Stop(int nKill)
//{
//#ifdef WIN32
//	if (m_eStatus == running)
//	{
//		if (nKill)
//		{
//			TerminateThread(m_hThread, 0);
//		}
//		m_eStatus = stopped;
//		return 0;
//	}
//#else
//	if (m_eStatus == running)
//	{
//		if (nKill)
//		{
//			pthread_cancel(m_hThread);
//			usleep(100);	// let thread process left work
//		}
//		m_eStatus = stopped;
//		return 0;
//	}
//#endif
//	return -1;
//}

void CThread::Sleep(const unsigned int uiTime)
{
#ifdef WIN32
	::Sleep(uiTime);
#else
	struct timeval sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_usec=uiTime * 1000;
	select(0,0,0,0,&sleeptime);
#endif
}
