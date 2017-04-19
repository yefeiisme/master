#include "stdafx.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "MemClass.h"
#include "KMemClass1.h"

CMemClass::CMemClass()
{
	m_lpMemPtr = NULL;
	m_lpMemLen = 0;
}

CMemClass::~CMemClass()
{
	Free();
}
//---------------------------------------------------------------------------
// 函数:	Alloc
// 功能:	分配内存
// 参数:	dwSize		内存块大小
// 返回:	PVOID		内存块指针
//---------------------------------------------------------------------------
void *CMemClass::Alloc(unsigned int uiSize)
{
	// 已经分配的就不要再分配了
	if (m_lpMemLen == uiSize)
		return m_lpMemPtr;

	// 释放已经分配的内存
	if (m_lpMemPtr)
		Free();

	// 分配内存
	m_lpMemPtr = g_MemAlloc(uiSize);
	if (m_lpMemPtr != NULL)
		m_lpMemLen = uiSize;
	return m_lpMemPtr;
}

//---------------------------------------------------------------------------
// 函数:	Free
// 功能:	释放内存
// 参数:	void
// 返回:	void
//---------------------------------------------------------------------------
void CMemClass::Free()
{
	if (m_lpMemPtr)
		g_MemFree(m_lpMemPtr);
	m_lpMemPtr = NULL;
	m_lpMemLen = 0;
}

void *KMemClass1::Alloc(DWORD dwSize)
{
	this->m_lpMemPtr = new BYTE[dwSize];
	this->m_lpMemLen = dwSize;
	return m_lpMemPtr;
}

void KMemClass1::Free()
{
	delete []this->m_lpMemPtr;
}
