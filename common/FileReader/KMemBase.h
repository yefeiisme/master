#ifndef KMemBase_H
#define KMemBase_H

void	g_MemInfo(void);
void*	g_MemAlloc(DWORD dwSize);
void	g_MemFree(LPVOID lpMem);
void	g_MemCopy(LPVOID lpDest, LPVOID lpSrc, DWORD dwLen);
void	g_MemCopyMmx(LPVOID lpDest, LPVOID lpSrc, DWORD dwLen);
BOOL	g_MemComp(LPVOID lpDest, LPVOID lpSrc, DWORD dwLen);
void	g_MemFill(LPVOID lpDest, DWORD dwLen, BYTE byFill);
void	g_MemFill(LPVOID lpDest, DWORD dwLen, WORD wFill);
void	g_MemFill(LPVOID lpDest, DWORD dwLen, DWORD dwFill);
void	g_MemZero(LPVOID lpDest, DWORD dwLen);
void	g_MemXore(LPVOID lpDest, DWORD dwLen, DWORD dwXor);

#endif
