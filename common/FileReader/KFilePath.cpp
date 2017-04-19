#include "stdafx.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"


#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <string.h>
//---------------------------------------------------------------------------
#ifdef WIN32
static char szRootPath[MAXPATH] = "C:";		// 启动路径
static char szCurrPath[MAXPATH] = "\\";		// 当前路径
#else
static char szRootPath[MAXPATH] = "/";		// 启动路径
static char szCurrPath[MAXPATH] = "/";		// 当前路径
#endif

int RemoveTwoPointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	//KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\..\\";
#else
	LPCTSTR lpszOld = "/../";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 3;
		while(lpszTarget > szPath)
		{
			lpszTarget--;
			if ((*lpszTarget) == '\\' ||(*lpszTarget) == '/')
				break;
		}
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveTwoPointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveOnePointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\.\\";
#else
	LPCTSTR lpszOld = "/./";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 2;
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveOnePointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveAllPointPath(LPTSTR szPath, int nLength)
{
	return RemoveOnePointPath(szPath, RemoveTwoPointPath(szPath, nLength));
}

//---------------------------------------------------------------------------
// 函数:	SetRootPath
// 功能:	设置程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
void g_SetRootPath(LPSTR lpPathName)
{
	if (lpPathName)
	{
		g_DebugLog("set path %s\n", lpPathName);
		g_StrCpy(szRootPath, lpPathName);
	}
	else
	{
		memset( szRootPath, 0, sizeof( szRootPath ) );

		getcwd( szRootPath, MAXPATH );
	}
//]] daniel

	// 去掉路径末尾的 '\'
	int len = g_StrLen(szRootPath);
	g_DebugLog("set path %s(%d)\n", szRootPath, len);
	if (szRootPath[len - 1] == '\\' || szRootPath[len - 1] == '/')
	{
		szRootPath[len - 1] = 0;
	}
	g_DebugLog("RootPath = %s", szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	GetRootPath
// 功能:	取得程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
void g_GetRootPath(LPSTR lpPathName)
{
	g_StrCpy(lpPathName, szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	GetFullPath
// 功能:	取得文件的全路径名
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
void g_GetFullPath(LPSTR lpPathName, LPSTR lpFileName)
{
	// 文件带有全路径
	if (lpFileName[1] == ':')
	{
		g_StrCpy(lpPathName, lpFileName);
		return;
	}

	// 文件带有部分路径
	if (lpFileName[0] == '\\' || lpFileName[0] == '/')
	{
		g_StrCpy(lpPathName, szRootPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
	
	// 当前路径为全路径
#ifdef WIN32
	if (szCurrPath[1] == ':')
	{
		g_StrCpy(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
#endif
	// 当前路径为部分路径
	g_StrCpy(lpPathName, szRootPath);
        if(szCurrPath[0] != '\\' && szCurrPath[0] != '/') {
#ifdef WIN32
	g_StrCat(lpPathName, "\\");
#else
	g_StrCat(lpPathName, "/");
#endif
      }
	g_StrCat(lpPathName, szCurrPath);

	if (lpFileName[0] == '.' && (lpFileName[1] == '\\'||lpFileName[1] == '/') )
		g_StrCat(lpPathName, lpFileName + 2);
	else
		g_StrCat(lpPathName, lpFileName);
}
//---------------------------------------------------------------------------
// 函数:	g_UnitePathAndName
// 功能:	一个路径和一个文件名，合并到lpGet中形成一个完整的路径文件名
// 参数:	lpPath 传入路径名 lpFile 传入文件名 lpGet 获得的最终完整文件名
// 返回:	void
// 注意：   这里没有考虑字符串的长度，使用的时候要保证字符串的长度足够
//---------------------------------------------------------------------------
void	g_UnitePathAndName(char *lpPath, char *lpFile, char *lpGet)
{
	if (!lpPath || !lpFile || !lpGet)
		return;
	strcpy(lpGet, lpPath);
	int	nSize = strlen(lpGet);
	if (lpGet[nSize] - 1 != '\\')
	{
		lpGet[nSize] = '\\';
		lpGet[nSize + 1] = 0;
	}
	if (lpFile[0] != '\\')
	{
		strcat(lpGet, lpFile);
	}
	else
	{
		strcat(lpGet, &lpFile[1]);
	}
}
//---------------------------------------------------------------------------
// 函数:	File Name to 32bit Id
// 功能:	文件名转换成 Hash 32bit ID
// 参数:	lpFileName	文件名
// 返回:	FileName Hash 32bit ID
// 
// 注意:	游戏世界和主网关交互数据所用的哈希查找索引也是用
//			的这个函数，所以请修改这个函数时也对应修改主网管
//			中相对应的那个函数。这个函数存在于Common.lib工程的Utils.h
//			中，函数声明为 DWORD HashStr2ID( const char * const pStr );
//---------------------------------------------------------------------------
DWORD g_FileName2Id(LPSTR lpFileName)
{
	DWORD Id = 0;
	char c = 0;
	for (int i = 0; lpFileName[i]; i++)
	{
		c = lpFileName[i];
#ifndef WIN32
		//for linux path looking up
		if ('/' == c)
			c = '\\';
#endif
		Id = (Id + (i + 1) * c) % 0x8000000b * 0xffffffef;
	}
	return (Id ^ 0x12345678);
}
