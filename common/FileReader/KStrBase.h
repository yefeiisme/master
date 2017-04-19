#ifndef KStrBase_H
#define KStrBase_H

int		g_StrLen(LPCSTR lpStr);
LPSTR	g_StrEnd(LPCSTR lpStr);
void	g_StrCpy(LPSTR lpDest, LPCSTR lpSrc);
void	g_StrCpyLen(LPSTR lpDest, LPCSTR lpSrc, const unsigned int uMaxLen);
void	g_StrCat(LPSTR lpDest, LPCSTR lpSrc);
void	g_StrCatLen(LPSTR lpDest, LPCSTR lpSrc, int nMaxLen);
BOOL	g_StrCmp(LPCSTR lpDest, LPCSTR lpSrc);
BOOL	g_StrCmpLen(LPCSTR lpDest, LPCSTR lpSrc, int nMaxLen);
void	g_StrUpper(LPSTR lpDest);
void	g_StrLower(LPSTR lpDest);
void	g_StrRep(LPSTR lpDest, LPSTR lpSrc, LPSTR lpRep);

#endif
