#ifndef __I_TABFILE_H__
#define __I_TABFILE_H__

class ITabFile
{
public:
	virtual int		GetHeight() const = 0;
	virtual bool	GetString(const int nRow, char *szColumn, char *lpDefault, char *lpRString, const unsigned long dwSize) = 0;
	virtual bool	GetString(const int nRow, const int nColumn, char *lpDefault, char *lpRString, const unsigned long dwSize) = 0;
	virtual bool	GetInteger(const int nRow, char *szColumn, const int nDefault, int *pnValue) = 0;
	virtual bool	GetInteger(const int nRow, const int nColumn, const int nDefault, int *pnValue) = 0;
	virtual bool	GetFloat(const int nRow, char *szColumn, const float fDefault, float *pfValue)	= 0;
	virtual bool	GetFloat(const int nRow, const int nColumn, const float fDefault, float *pfValue) = 0;
	virtual void	Clear() = 0;
	virtual void	Release() = 0;
};

void	g_SetRootPath(char *lpPathName);
ITabFile *OpenTabFile(const char *FileName);

#endif
