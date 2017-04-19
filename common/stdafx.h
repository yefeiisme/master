#ifndef __STDAFX_H_
#define __STDAFX_H_

#include <list>
using namespace std;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define LONG long
#define INT int
#define LPINT int *
#define HANDLE unsigned long
#define LPSTR char *
#define LPTSTR char *
#define LPCSTR const char *
#define LPCTSTR const char *
#define DWORD unsigned long
#define LPVOID void *
#define PVOID void *
#define BOOL int
#define TRUE 1
#define FALSE 0
#define BYTE unsigned char
#define WORD unsigned short
#define UINT unsigned int
#define PBYTE unsigned char *
typedef unsigned long long	uint64;
typedef long long			int64;

#define FILE_CURRENT	1
#define FILE_END		2
#define FILE_BEGIN		0

#ifndef NULL
#define	NULL	0
#endif
#ifndef ZeroMemory
#define ZeroMemory(x,y) memset(x, 0, y)
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(Address)				{ if( NULL != (Address) )	delete(Address);	Address = NULL; }
#endif
#ifndef SAFE_DELETE_ARR
#define SAFE_DELETE_ARR(Address)			{ if( NULL != (Address) )	delete[](Address);	Address = NULL; }
#endif

#define MAX_HOST_LEN		16
#define MAX_USER_LEN		32
#define MAX_PASSWORD_LEN	32
#define MAX_DB_NAME_LEN		32


#endif 
