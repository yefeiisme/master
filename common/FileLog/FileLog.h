#ifndef _CFS__FILELOGS___H____
#define _CFS__FILELOGS___H____

#ifdef WIN32
#include <Windows.h>
#else
#include <pthread.h>
#endif

#define LOG_ROOT_DIRECTORY	128
#define LOG_DIRECTORY_LEN	256
#define LOG_FILE_NAME_LEN	256
#define LOG_FILE_CONTENT	1024*1024		// 文件写入的最大值
#define MAXSIZE_CURTIME		64

class IFileLog;

class CFileLog : public IFileLog
{
private:
	FILE			*m_pFile;

	time_t			m_nTimeNow;			// 当前时间
	tm				m_tagTimeNow;		// 当前时间

	tm				m_tagDirTime;		// 记录上次打开目录的时间
	time_t			m_nOpenTime;		// 记录文件创建的时间.当需要新创建文件时，与此变量进行对比.如果一样，则不创建新文件，直接对当前文件进行写入；如果不一样，则创建新文件，并对此变量重新赋值，并在新文件中写入
	tm				m_tagOpenTime;		// 文件被打开的时间
	int				m_nWriteSize;		// 文件写入数据的大小

	char			m_strRootDir[LOG_ROOT_DIRECTORY];
	char			m_strLogDir[LOG_DIRECTORY_LEN];
private:
	void			WriteLogInner(const char *pcLogMsg, unsigned char bKeepOpen = 0);
	void			OpenDirectory();
	bool			OpenLogFile();
	bool			IsOverHour();		// 是否过了当前的小时
public:
	CFileLog();
	virtual ~CFileLog();

	static CFileLog	&Singleton();

	bool			Initialize(char strDirectoryName[]);
	void			WriteLog(const char *pcLogMsg, ...);
};

class Mutex
{
private:
#ifdef WIN32
	CRITICAL_SECTION	m_mutex;
#else
	pthread_mutex_t		m_mutex;
#endif
	int count;
public:
#ifdef WIN32
	Mutex(int mutex_type);
#else
	Mutex(int mutex_type);
#endif
	~Mutex();

	int		lock();
	int		unlock();
};

#endif
