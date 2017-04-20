#ifndef __COMMON_H_
#define __COMMON_H_

#ifndef MAXPATH
#define MAXPATH   260
#define MAXDIR    256
#define MAXFILE   256
#define MAXEXT    256
#define MAXDRIVE    3
#endif

#include "stdafx.h"

void	g_SetRootPath(char *lpPathName = NULL);
void	g_GetRootPath(char *lpPathName);
void	g_GetFullPath(char *lpPathName, char *lpFileName);

void	g_RandomSeed(UINT nSeed);
UINT	g_Random(UINT nMax);

void	ClearChar( char* szAction, char cOne, char cTwo );
int		FindDelimter( char* szAction, char cOne, char cTwo );
void	ConvLowerCase( char* szAction );
int		CheckAct( char* szAction );
char	*FindCloseComma( char* szAction );
char	*FindAddiOp( char* szAction, int& nAddiOp );
float	qsqrt(float fDistance);

#endif

