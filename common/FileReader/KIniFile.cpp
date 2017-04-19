#include "stdafx.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFile.h"
#include "KFilePath.h"
#include "KPakFile.h"
#include "KIniFile.h"
#include <string.h>

//---------------------------------------------------------------------------
typedef struct
{
	DWORD		Id;			// 文件标识 = 0x4b434150 ("PACK")
	DWORD		DataLen;	// 文件原始的长度
	DWORD		PackLen;	// 文件压缩后长度
	DWORD		Method;		// 使用的压缩算法
} TPackHead;
//---------------------------------------------------------------------------
// 函数:	KIniFile
// 功能:	购造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KIniFile::KIniFile()
{
	g_MemZero(&m_Header, sizeof(SECNODE));
}
//---------------------------------------------------------------------------
// 函数:	~KIniFile
// 功能:	析造函数
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
KIniFile::~KIniFile()
{
	Clear();
}
//---------------------------------------------------------------------------
// 函数:	Load
// 功能:	加载一个INI文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
bool KIniFile::Load(const char *pFileName)
{
	KPakFile	File;
	DWORD		dwSize;
	PVOID		Buffer;
	TPackHead*	pHeader;

	// check file name
	if (pFileName[0] == 0)
		return false;

	if (m_Header.pNextNode)
		Clear();

	if (!File.Open((char*)pFileName))
	{
		g_DebugLog("Can't open ini file : %s", pFileName);
		return false;
	}

	dwSize = File.Size();

	Buffer = m_MemStack.Push(dwSize + 4);

	File.Read(Buffer, dwSize);

	pHeader = (TPackHead*)Buffer;
	if (pHeader->Id == 0x4b434150) // "PACK"
	{
		File.Close();
		return LoadPack(pFileName);
	}

	CreateIniLink(Buffer, dwSize);

	return true;
}
//---------------------------------------------------------------------------
// 函数:	LoadPack
// 功能:	打开一个压缩的INI文件
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
bool KIniFile::LoadPack(const char *pFileName)
{
	KPakFile	File;
	PVOID		PackBuf;
	PVOID		DataBuf;

	TPackHead	Header;

	// check file name
	if (pFileName[0] == 0)
		return false;

	// close open file
	if (m_Header.pNextNode)
		Clear();

	// open the pack ini file
	if (!File.Open((char*)pFileName))
		return false;

	// read pack file header
	File.Read(&Header, sizeof(Header));

	// if not a pack file read directly
	if (Header.Id != 0x4b434150) // "PACK"
		return false;

	// allocate buffer for pack data
	PackBuf = m_MemStack.Push(Header.PackLen);

	// allocate bufer for real data
	DataBuf = m_MemStack.Push(Header.DataLen + 2);

	// read pack data for file
	File.Read(PackBuf, Header.PackLen);

	// check data length

	// create ini link in memory
	CreateIniLink(DataBuf, Header.DataLen);

	return true;
}
//---------------------------------------------------------------------------
// 函数:	Clear
// 功能:	清除INI文件的内容
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::Clear()
{
	ReleaseIniLink();
}
//---------------------------------------------------------------------------
// 函数:	ReadLine
// 功能:	读取INI文件的一行
// 参数:	Buffer	缓存
//			Szie	长度
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------
bool KIniFile::ReadLine(char *pBuffer, const long lSize)
{
	if (m_Offset >= lSize)
		return false;

	int count = 0;
	while (pBuffer[m_Offset] != 0x0D && pBuffer[m_Offset] != 0x0A)
	{
		m_Offset++; count++;
		if (m_Offset >= lSize)
			break;
	}
		
	if (count == 0)
	{	
		if (pBuffer[m_Offset] == 0x0D && pBuffer[m_Offset + 1] == 0x0A)
		{
			m_Offset += 2;			
		}
		else
		{
			m_Offset += 1;
		}
		
		return false;
	}
	
	if (pBuffer[m_Offset] == 0x0D && pBuffer[m_Offset + 1] == 0x0A)
	{
		m_Offset += 2;
		pBuffer[m_Offset-2] = 0;
	}
	else
	{
		m_Offset += 1;
		pBuffer[m_Offset-1] = 0;
	}
	return true;
}

//---------------------------------------------------------------------------
// 函数:	CreateIniLink
// 功能:	创建Ini链表
// 参数:	pBuffer		缓存
//			nBufLen		长度
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::CreateIniLink(void *pBuffer, const long lBufLen)
{
	LPSTR lpBuffer = (LPSTR)pBuffer;
	LPSTR lpString = NULL;
	LPSTR lpValue  = NULL;
	char  szSection[32] = "[MAIN]";

	m_Offset = 0;
	while (m_Offset < lBufLen)
	{
		lpString = &lpBuffer[m_Offset];
		if (!ReadLine(lpBuffer, lBufLen))
			continue;

		if (*lpString == ';')
		{
			continue;
		}
		
		if (*lpString == '#')
		{
			continue;
		}

		if (*lpString == '[')
		{
			g_StrCpyLen(szSection, lpString, sizeof(szSection));
			continue;
		}

		lpValue = SplitKeyValue(lpString);
		SetKeyValue(szSection, lpString, lpValue);
	}
}
//---------------------------------------------------------------------------
// 函数:	ReleaseIniLink()
// 功能:	释放Ini链表
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void KIniFile::ReleaseIniLink()
{
	SECNODE* pThisSec = &m_Header;
	SECNODE* pNextSec = pThisSec->pNextNode;
	KEYNODE* pThisKey = NULL;
	KEYNODE* pNextKey = NULL;

	while (pNextSec != NULL)
	{
		pThisSec = pNextSec->pNextNode;
		pThisKey = &pNextSec->pKeyNode;
		pNextKey = pThisKey->pNextNode;
		while (pNextKey != NULL)
		{
			pThisKey = pNextKey->pNextNode;
			m_MemStack.Free(pNextKey->pKey);
			m_MemStack.Free(pNextKey->pValue);
			m_MemStack.Free(pNextKey);
			pNextKey = pThisKey;
		}
		m_MemStack.Free(pNextSec->pSection);
		m_MemStack.Free(pNextSec);
		pNextSec = pThisSec;
	}
	m_Header.pNextNode = NULL;

	m_MemStack.FreeAllChunks();
}
//---------------------------------------------------------------------------
// 函数:	SplitKeyValue
// 功能:	分割Key和Value
// 参数:	pString		Key=Value
// 返回:	指向Value
//---------------------------------------------------------------------------
char *KIniFile::SplitKeyValue(char *pString)
{
	char *pValue = pString;
	while (*pValue)
	{
		if (*pValue == '=')
			break;
		pValue++;
	}
	*pValue = 0;
	return pValue + 1;
}
//---------------------------------------------------------------------------
// 函数:	String2Id
// 功能:	字符串转成32 bits ID
// 参数:	pString		字符串
// 返回:	32 bits ID
//---------------------------------------------------------------------------
DWORD KIniFile::String2Id(const char *pString)
{
	DWORD Id = 0;
	for (int i=0; pString[i]; i++)
	{
		Id = (Id + (i+1) * pString[i]) % 0x8000000b * 0xffffffef;
	}
	return Id ^ 0x12345678;
}

//---------------------------------------------------------------------------
// 函数:	SetKeyValue
// 功能:	设置Key的Value
// 参数:	pSection	节名
//			pKey		建名
//			pValue		建值
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
bool KIniFile::SetKeyValue(const char * pSection, const char * pKey, const char *pValue)
{
	int		nLen;
	DWORD	dwID;

	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}

	// search for the matched section
	SECNODE* pThisSecNode = &m_Header;
	SECNODE* pNextSecNode = pThisSecNode->pNextNode;
	dwID = String2Id(szSection);
	while (pNextSecNode != NULL)
	{
		if (dwID == pNextSecNode->dwID)
		{
			break;
		}
		pThisSecNode = pNextSecNode;
		pNextSecNode = pThisSecNode->pNextNode;
	}

	// if no such section found create a new section
	if (pNextSecNode == NULL)
	{
		nLen = g_StrLen(szSection) + 1;
		pNextSecNode = (SECNODE *)m_MemStack.Push(sizeof(SECNODE));
		pNextSecNode->pSection = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextSecNode->pSection, szSection, nLen);
		pNextSecNode->dwID = dwID;
		pNextSecNode->pKeyNode.pNextNode = NULL;
		pNextSecNode->pNextNode = NULL;
		pThisSecNode->pNextNode = pNextSecNode;
	}

	// search for the same key
	KEYNODE* pThisKeyNode = &pNextSecNode->pKeyNode;
	KEYNODE* pNextKeyNode = pThisKeyNode->pNextNode;
	dwID = String2Id(pKey);
	while (pNextKeyNode != NULL)
	{
		if (dwID == pNextKeyNode->dwID)
		{
			break;
		}
		pThisKeyNode = pNextKeyNode;
		pNextKeyNode = pThisKeyNode->pNextNode;
	}

	// if no such key found create a new key
	if (pNextKeyNode == NULL)
	{
		pNextKeyNode = (KEYNODE *)m_MemStack.Push(sizeof(KEYNODE));

		nLen = g_StrLen(pKey) + 1;
		pNextKeyNode->pKey = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pKey, (void*)pKey, nLen);

		nLen = g_StrLen(pValue) + 1;
		pNextKeyNode->pValue = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pValue, (void*)pValue, nLen);

		pNextKeyNode->dwID = dwID;
		pNextKeyNode->pNextNode = NULL;
		pThisKeyNode->pNextNode = pNextKeyNode;
	}
	// replace the old value with new
	else
	{
		m_MemStack.Free(pNextKeyNode->pValue);
		nLen = g_StrLen(pValue) + 1;
		pNextKeyNode->pValue = (char *)m_MemStack.Push(nLen);
		g_MemCopy(pNextKeyNode->pValue, (void*)pValue, nLen);
	}
	return true;
}
//---------------------------------------------------------------------------
// 函数:	GetKeyValue
// 功能:	取得Key的Value
// 参数:	pSection	节名
//			pKey		建名
//			pValue		建值
// 返回:	TRUE－成功 FALSE－失败
//---------------------------------------------------------------------------
bool KIniFile::GetKeyValue(const char * pSection, const char * pKey, char *pValue, const unsigned int uSize)
{
	DWORD	dwID;

	// setup section name
	char szSection[32] = "[";
	if (pSection[0] != '[')
	{
		g_StrCat(szSection, pSection);
		g_StrCat(szSection, "]");
	}
	else
	{
		g_StrCpy(szSection, pSection);
	}

	// search for the matched section
	SECNODE* pSecNode = m_Header.pNextNode;
	dwID = String2Id(szSection);
	while (pSecNode != NULL)
	{
		if (dwID == pSecNode->dwID)
		{
			break;
		}
		pSecNode = pSecNode->pNextNode;
	}

	// if no such section founded
	if (pSecNode == NULL)
	{
		return false;
	}

	// search for the same key
	KEYNODE* pKeyNode = pSecNode->pKeyNode.pNextNode;
	dwID = String2Id(pKey);
	while (pKeyNode != NULL)
	{
		if (dwID == pKeyNode->dwID)
		{
			break;
		}
		pKeyNode = pKeyNode->pNextNode;
	}

	// if no such key found
	if (pKeyNode == NULL)
	{
		return false;
	}

	// copy the value of the key
	g_StrCpyLen(pValue, pKeyNode->pValue, uSize);
	return true;
}
//---------------------------------------------------------------------------
// 函数:	GetString
// 功能:	读取一个字符串
// 参数:	lpSection		节名
//			lpKeyName		建名
//			lpDefault		缺省值
//			lpRString		返回值
//			dwSize			返回字符串的最大长度
// 返回:	void
//---------------------------------------------------------------------------
bool KIniFile::GetString(const char *lpSection, const char *lpKeyName, const char *lpDefault, char *lpRString, const unsigned int uSize)
{
	if (GetKeyValue(lpSection, lpKeyName, lpRString, uSize))
		return true;

	g_StrCpyLen(lpRString, lpDefault, uSize);

	return false;
}
//---------------------------------------------------------------------------
// 函数:	GetInteger
// 功能:	读取一个整数
// 参数:	lpSection		节名
//			lpKeyName		建名
//			nDefault		缺省值
//			pnValue			返回值
// 返回:	void
//---------------------------------------------------------------------------
bool KIniFile::GetInteger(const char *lpSection, const char *lpKeyName, int nDefault, int *pnValue)
{
	char Buffer[32];
	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		*pnValue = atoi(Buffer);
		return true;
	}
	else
	{
		*pnValue = nDefault;
		return false;
	}
}

bool KIniFile::GetFloat(const char *lpSection, const char *lpKeyName, float fDefault, float *pfValue)
{
	char Buffer[32];
	if (GetKeyValue(lpSection, lpKeyName, Buffer, sizeof(Buffer)))
	{
		*pfValue = atof(Buffer);
		return true;
	}
	else
	{
		*pfValue = fDefault;
		return false;
	}
}

void KIniFile::Release()
{
	Clear();
	delete this;
}

IIniFile *OpenIniFile(const char *FileName)
{
	KIniFile	*pFile	= new KIniFile;
	if (!pFile)
		return NULL;

	if (!pFile->Load(FileName))
	{
		pFile->Clear();
		delete pFile;
		return NULL;
	}

	return pFile;
}
