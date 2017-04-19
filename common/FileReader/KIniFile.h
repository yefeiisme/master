#ifndef KIniFile_H
#define KIniFile_H

#include "IIniFile.h"
#include "KMemStack.h"

typedef struct tagKeyNode
{
	DWORD			dwID;
	char			*pKey;
	char			*pValue;
	tagKeyNode		*pNextNode;
} KEYNODE;

typedef struct tagSecNode
{
	DWORD			dwID;
	char			*pSection;
	tagKeyNode		pKeyNode;
	tagSecNode		*pNextNode;
} SECNODE;

class KIniFile : public IIniFile
{
private:
	SECNODE		m_Header;
	LONG		m_Offset;
	KMemStack	m_MemStack;
private:
	void		CreateIniLink(void *pBuffer, const long lSize);
	void		ReleaseIniLink();
	DWORD		String2Id(const char *pString);
	bool		ReadLine(char *Buffer, const long lSize);
	char		*SplitKeyValue(char *pString);
	bool		SetKeyValue(const char * pSection, const char * pKey, const char *pValue);
	bool		GetKeyValue(const char * pSection, const char * pKey, char *pValue, const unsigned int uSize);
public:
	KIniFile();
	~KIniFile();
	bool		Load(const char *FileName);
	bool		LoadPack(const char *pFileName);
	void		Clear();
	bool		GetString(const char *lpSection, const char *lpKeyName, const char *lpDefault, char *lpRString, const unsigned int uSize);
	bool		GetInteger(const char *lpSection, const char *lpKeyName, int nDefault, int *pnValue);
	bool		GetFloat(const char *lpSection, const char *lpKeyName, float fDefault, float *pfValue);
	void		Release();
};

#endif
