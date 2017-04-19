#ifndef __RING_BUFFER_H_
#define __RING_BUFFER_H_

class IRingBuffer;

class CRingBuffer : public IRingBuffer
{
private:
	char			*m_pBuffer;
	char			*m_pRead;
	char			*m_pWrite;
	char			*m_pTemp;
	char			*m_pNextRead;

	unsigned int	m_uBufLen;
	unsigned int	m_uMaxPack;
private:
	/*
	 *	写入数据,返回右指针
	 */
	char			*PutData(char *pRead, char *pWrite, char *pData, const unsigned int uDataSize);
	unsigned int	GetDataLen(char *pRead, char *pWrite);
	void			Clear();
public:
	CRingBuffer();
	~CRingBuffer();

	bool			Initialize(const unsigned int uBufLen, const unsigned int uMaxPack);
	bool			Reinit();
	bool			SndPack(const void *pData, const unsigned int uDataSize);
	const void		*RcvPack(unsigned int &uDataSize);
	void			Release();
};

#endif
