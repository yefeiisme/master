#include "stdafx.h"
#include "KLinkArray.h"

KLinkArray::~KLinkArray()
{
	 if (pNode) 
	 {
		 delete [] pNode;
		 pNode = NULL;
	 }
}

void KLinkArray::Init(int nSize)
{
	if (pNode)
	{
		delete [] pNode;
		pNode = NULL;
		m_nCount = 0;
		m_nSize = 0;
	}

	pNode = new KLinkNode[nSize];
	m_nSize = nSize;
}

void KLinkArray::Remove(int nIdx)
{
	if (nIdx <= 0 || nIdx >= m_nSize)
	{
		//g_DebugLog("[error]Remove invalid node: index %d, size %d", nIdx, m_nSize);
		return;
	}

	if ((pNode[nIdx].nNext == 0 && pNode[0].nPrev != nIdx) || (pNode[nIdx].nPrev == 0 && pNode[0].nNext != nIdx))
	{
		//g_DebugLog("[error]Node:%d Remove twice", nIdx);
		return;
	}

	pNode[pNode[nIdx].nPrev].nNext = pNode[nIdx].nNext;
	pNode[pNode[nIdx].nNext].nPrev = pNode[nIdx].nPrev;
	pNode[nIdx].nNext = 0;
	pNode[nIdx].nPrev = 0;
	m_nCount--;
}

void KLinkArray::Insert(int nIdx)
{
	if (nIdx <= 0 || nIdx >= m_nSize)
	{
		//g_DebugLog("[error]Insert invalid node: index %d, size %d", nIdx, m_nSize);
		return;
	}

	if (pNode[nIdx].nNext != 0 || pNode[nIdx].nPrev != 0)
	{
		//g_DebugLog("[error]Node:%d Insert twice", nIdx);
		return;
	}

	pNode[nIdx].nNext = pNode[0].nNext;
	pNode[pNode[0].nNext].nPrev = nIdx;
	pNode[0].nNext = nIdx;
	pNode[nIdx].nPrev = 0;
	m_nCount++;
}
