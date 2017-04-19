#ifndef KFilePath_H
#define KFilePath_H
//---------------------------------------------------------------------------
#ifndef MAXPATH
#define MAXPATH   260
#define MAXDIR    256
#define MAXFILE   256
#define MAXEXT    256
#define MAXDRIVE    3
#endif

#include "stdafx.h"

void	g_SetRootPath(LPSTR lpPathName = NULL);
void	g_GetRootPath(LPSTR lpPathName);
void	g_GetFullPath(LPSTR lpPathName, LPSTR lpFileName);
// 一个路径和一个文件名，合并到lpGet中形成一个完整的路径文件名
void	g_UnitePathAndName(char *lpPath, char *lpFile, char *lpGet);

DWORD	g_FileName2Id(LPSTR lpFileName);

#endif
