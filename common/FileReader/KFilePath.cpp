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
static char szRootPath[MAXPATH] = "C:";		// ����·��
static char szCurrPath[MAXPATH] = "\\";		// ��ǰ·��
#else
static char szRootPath[MAXPATH] = "/";		// ����·��
static char szCurrPath[MAXPATH] = "/";		// ��ǰ·��
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
// ����:	SetRootPath
// ����:	���ó���ĸ�·��
// ����:	lpPathName	·����
// ����:	void
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

	// ȥ��·��ĩβ�� '\'
	int len = g_StrLen(szRootPath);
	g_DebugLog("set path %s(%d)\n", szRootPath, len);
	if (szRootPath[len - 1] == '\\' || szRootPath[len - 1] == '/')
	{
		szRootPath[len - 1] = 0;
	}
	g_DebugLog("RootPath = %s", szRootPath);
}
//---------------------------------------------------------------------------
// ����:	GetRootPath
// ����:	ȡ�ó���ĸ�·��
// ����:	lpPathName	·����
// ����:	void
//---------------------------------------------------------------------------
void g_GetRootPath(LPSTR lpPathName)
{
	g_StrCpy(lpPathName, szRootPath);
}
//---------------------------------------------------------------------------
// ����:	GetFullPath
// ����:	ȡ���ļ���ȫ·����
// ����:	lpPathName	·����
//			lpFileName	�ļ���
// ����:	void
//---------------------------------------------------------------------------
void g_GetFullPath(LPSTR lpPathName, LPSTR lpFileName)
{
	// �ļ�����ȫ·��
	if (lpFileName[1] == ':')
	{
		g_StrCpy(lpPathName, lpFileName);
		return;
	}

	// �ļ����в���·��
	if (lpFileName[0] == '\\' || lpFileName[0] == '/')
	{
		g_StrCpy(lpPathName, szRootPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
	
	// ��ǰ·��Ϊȫ·��
#ifdef WIN32
	if (szCurrPath[1] == ':')
	{
		g_StrCpy(lpPathName, szCurrPath);
		g_StrCat(lpPathName, lpFileName);
		return;
	}
#endif
	// ��ǰ·��Ϊ����·��
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
// ����:	g_UnitePathAndName
// ����:	һ��·����һ���ļ������ϲ���lpGet���γ�һ��������·���ļ���
// ����:	lpPath ����·���� lpFile �����ļ��� lpGet ��õ����������ļ���
// ����:	void
// ע�⣺   ����û�п����ַ����ĳ��ȣ�ʹ�õ�ʱ��Ҫ��֤�ַ����ĳ����㹻
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
// ����:	File Name to 32bit Id
// ����:	�ļ���ת���� Hash 32bit ID
// ����:	lpFileName	�ļ���
// ����:	FileName Hash 32bit ID
// 
// ע��:	��Ϸ����������ؽ����������õĹ�ϣ��������Ҳ����
//			������������������޸��������ʱҲ��Ӧ�޸�������
//			�����Ӧ���Ǹ��������������������Common.lib���̵�Utils.h
//			�У���������Ϊ DWORD HashStr2ID( const char * const pStr );
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
