#ifndef __MEMCLASS_H
#define __MEMCLASS_H

class CMemClass
{
private:
	void			*m_lpMemPtr;
	unsigned int	m_lpMemLen;
public:
	CMemClass();
	~CMemClass();
	void			*Alloc(unsigned int uiSize);
	void			Free();
	void			Zero();
	void			Fill(unsigned char byFill);
	void			Fill(unsigned short wFill);
	void			Fill(unsigned int dwFill);
	void			*GetMemPtr() { return m_lpMemPtr; };
	unsigned int	GetMemLen() { return m_lpMemLen; };
};

#endif
