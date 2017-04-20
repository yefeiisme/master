#ifndef __I_FILE_LOG_H_
#define __I_FILE_LOG_H_

class IFileLog
{
public:
	virtual bool	Initialize(char strDirectoryName[]) = 0;
	virtual void	WriteLog(const char *pcLogMsg, ...) = 0;
};

extern IFileLog	*g_pFileLog;

#endif
