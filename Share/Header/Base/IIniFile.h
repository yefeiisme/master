#ifndef __I_INI_FILE_H__
#define __I_INI_FILE_H__

class IIniFile
{
public:
	virtual bool	Load(const char *FileName) = 0;
	virtual bool	LoadPack(const char *FileName) = 0;
	virtual bool	GetString(const char *lpSection, const char *lpKeyName, const char *lpDefault, char *lpRString, const unsigned int uSize) = 0;
	virtual bool	GetInteger(const char *lpSection, const char *lpKeyName, int nDefault, int *pnValue) = 0;
	virtual bool	GetFloat(const char *lpSection, const char *lpKeyName, float fDefault, float *pfValue) = 0;
	virtual void	Release() = 0;
};

IIniFile *OpenIniFile(const char *FileName);

#endif
