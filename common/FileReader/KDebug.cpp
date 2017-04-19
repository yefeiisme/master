#include "stdafx.h"
#include "KFile.h"
#include "KMemBase.h"
#include "KDebug.h"
#include <string.h>

//---------------------------------------------------------------------------
// 函数:	g_DebugLog
// 功能:	向调试窗口输出调试信息
// 参数:	Fmt		格式化字符串
//			...		输出的字符串
// 返回:	void
//---------------------------------------------------------------------------
void g_DebugLog(LPSTR Fmt, ...)
{
}
//---------------------------------------------------------------------------
// 函数:	g_MessageBox
// 功能:	Display a System Message Box
// 参数:	char* lpMsg, ...
// 返回:	void
//---------------------------------------------------------------------------
void g_MessageBox(LPSTR lpMsg, ...)
{
}
//---------------------------------------------------------------------------
// 函数:	g_AssertFailed
// 功能:	断言失败
// 参数:	FileName	在哪个文件中失败
//			LineNum		在文件中的第几行
// 返回:	void
//---------------------------------------------------------------------------
void g_AssertFailed(LPSTR pFileName, int nLineNum)
{
	char szMsg[256];
	sprintf(szMsg, "Assert failed in %s, line = %i", pFileName, nLineNum);
	g_DebugLog(szMsg);
	g_MessageBox(szMsg);
	exit(1);
}
