#ifndef KDebug_H
#define KDebug_H

#include <stdio.h>


void g_DebugLog(LPSTR Fmt, ...);
void g_MessageBox(LPSTR lpMsg, ...);
void g_AssertFailed(char* FileName, int LineNum);

#ifndef SAFE_FREE
	#define SAFE_FREE(a)	if (a) {g_MemFree(a); (a)=NULL;}
#endif
#ifndef SAFE_RELEASE
	#define SAFE_RELEASE(a)	if (a) {(a)->Release(); (a)=NULL;}
#endif

#ifdef _DEBUG
	#define KASSERT(x)	if (!(x)) g_AssertFailed(__FILE__, __LINE__)
#else
	#define KASSERT(x)	NULL
#endif

#endif
