#include <stdio.h>
#include "IFileLog.h"
#include "string.h"
#include <stdarg.h>
#include <time.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif
#include "FileLog.h"

#define FILESTATUS_CLOSE			0
#define FILESTATUS_OPEN				1

#define MAXSIZE_PATHNAME			512
#define MAXSIZE_MSGBUF				(1024*64)
#define SERVERLOG_SKIP				" -> "

#define MAXSIZE_TIMESTRING			64
#ifdef _MSC_VER
#define snprintf _snprintf
#endif


Mutex		m_gLogMutex(0);
IFileLog	*g_pFileLog	= &CFileLog::Singleton();

CFileLog::CFileLog()
{
	m_pFile			= NULL;

	m_nTimeNow		= 0;
	memset(&m_tagTimeNow, 0, sizeof(m_tagTimeNow));

	memset(&m_tagDirTime, 0, sizeof(m_tagDirTime));
	m_nOpenTime	= 0;
	memset(&m_tagOpenTime, 0, sizeof(m_tagOpenTime));

	m_nWriteSize	= 0;

	memset(m_strRootDir, 0, sizeof(m_strRootDir));
	memset(m_strLogDir, 0, sizeof(m_strLogDir));
}

CFileLog::~CFileLog()
{
	if (m_pFile)
	{
		fclose(m_pFile);
	}
	m_pFile			= NULL;
}

CFileLog &CFileLog::Singleton()
{
	static CFileLog singleton;

	return singleton;
}

void CFileLog::WriteLogInner(const char *pcLogMsg, unsigned char bKeepOpen)
{
	if (pcLogMsg == NULL)
		return;

	m_gLogMutex.lock();

	m_nTimeNow			= time(NULL);
	time_t  tmCurTime	= m_nTimeNow;
	m_tagTimeNow		= *localtime(&tmCurTime);;

	OpenDirectory();

	if (OpenLogFile())
	{
		m_nWriteSize += fprintf(m_pFile, "%02d:%02d:%02d ", m_tagTimeNow.tm_hour, m_tagTimeNow.tm_min, m_tagTimeNow.tm_sec);

		m_nWriteSize += fwrite(pcLogMsg, 1, strlen(pcLogMsg), m_pFile);

		fclose(m_pFile);

		m_pFile	= NULL;
	}

	m_gLogMutex.unlock();
}

bool CFileLog::Initialize(char strDirectoryName[])
{
	strncpy(m_strRootDir, strDirectoryName, sizeof(m_strRootDir));
	m_strRootDir[sizeof(m_strRootDir)-1]	= '\0';

#ifdef WIN32
	CreateDirectory(m_strRootDir, NULL);
#else /* LINUX */
	mkdir(m_strRootDir, 0700);
#endif

	return true;
}

void CFileLog::WriteLog(const char *pcLogMsg, ...)
{
	char szMsgBuf[MAXSIZE_MSGBUF] = {0};
	va_list va;
	va_start(va, pcLogMsg);
	vsnprintf(szMsgBuf, sizeof(szMsgBuf), pcLogMsg, va);
#ifdef WIN32
	printf( szMsgBuf );
#endif
	va_end(va);
	WriteLogInner(szMsgBuf);
}

void CFileLog::OpenDirectory()
{
	if (m_tagDirTime.tm_yday == m_tagTimeNow.tm_yday &&
		m_tagDirTime.tm_mon == m_tagTimeNow.tm_mon &&
		m_tagDirTime.tm_mday == m_tagTimeNow.tm_mday)
	{
		return;
	}

	m_tagDirTime	= m_tagTimeNow;

	snprintf(m_strLogDir, sizeof(m_strLogDir),
		"%s%04d-%02d-%02d",
		m_strRootDir, 
		m_tagDirTime.tm_year+1900,
		m_tagDirTime.tm_mon+1,
		m_tagDirTime.tm_mday
		);
#ifdef WIN32
	CreateDirectory(m_strLogDir, NULL);
#else /* LINUX */
	mkdir(m_strLogDir, 0700);
#endif
}

bool CFileLog::OpenLogFile()
{
	char	szOpenPathName[MAXSIZE_PATHNAME] = {0};

	// 如果是第一次打开文件，或者是已经写了超过1个小时的日志文件，或者是文件写入大小超过LOG_FILE_CONTENT（1M），则换新的文件写入
	if (0 == m_nOpenTime || IsOverHour() || m_nWriteSize > LOG_FILE_CONTENT)
	{
		m_nOpenTime		= m_nTimeNow;
		m_tagOpenTime	= m_tagTimeNow;

		snprintf(szOpenPathName, sizeof(szOpenPathName),
#ifdef WIN32
			"%s\\%02d-%02d-%02d.log",
#else
			"%s/%02d-%02d-%02d.log",
#endif
			m_strLogDir,
			m_tagOpenTime.tm_hour,
			m_tagOpenTime.tm_min,
			m_tagOpenTime.tm_sec
			);

		m_pFile = fopen(szOpenPathName, "a+");
		if (NULL == m_pFile)
			return false;

		fseek(m_pFile,0,SEEK_END);
		m_nWriteSize = ftell(m_pFile);

		return true;
	}

	// 继续打开，以上次打开时间为文件名的文件，进行写入操作
	snprintf(szOpenPathName, sizeof(szOpenPathName),
#ifdef WIN32
		"%s\\%02d-%02d-%02d.log",
#else
		"%s/%02d-%02d-%02d.log",
#endif
		m_strLogDir,
		m_tagOpenTime.tm_hour,
		m_tagOpenTime.tm_min,
		m_tagOpenTime.tm_sec
		);

	m_pFile = fopen(szOpenPathName, "a+");
	if (NULL == m_pFile)
		return false;

	return true;
}

bool CFileLog::IsOverHour()
{
	return (m_tagOpenTime.tm_year != m_tagTimeNow.tm_year ||
			m_tagOpenTime.tm_mon != m_tagTimeNow.tm_mon ||
			m_tagOpenTime.tm_mday != m_tagTimeNow.tm_mday ||
			m_tagOpenTime.tm_hour != m_tagTimeNow.tm_hour);
}

Mutex::Mutex( int mutex_type )
{
#ifdef WIN32
	InitializeCriticalSection(&m_mutex);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	if ( pthread_mutexattr_settype(&attr, mutex_type ))
	{
		// Invalid mutex_type, use default type;
		pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_DEFAULT);
	}
	pthread_mutex_init(&m_mutex, &attr);
#endif
}

Mutex::~Mutex()
{
#ifdef WIN32
	DeleteCriticalSection(&m_mutex);
#else
	pthread_mutex_destroy(&m_mutex);
#endif
}

int Mutex::lock()
{
#ifdef WIN32
	EnterCriticalSection(&m_mutex);
	return 0;
#else
	return pthread_mutex_lock(&m_mutex);
#endif
}

int Mutex::unlock()
{
#ifdef WIN32
	LeaveCriticalSection(&m_mutex);
	return 0;
#else
	return pthread_mutex_unlock(&m_mutex);
#endif
}
