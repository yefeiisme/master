#ifndef __I_RING_BUFFER_H_
#define __I_RING_BUFFER_H_

class IRingBuffer
{
public:
	virtual bool		Reinit() = 0;
	virtual bool		SndPack(const void *pData, const unsigned int datasize) = 0;
	virtual const void	*RcvPack(unsigned int &datasize) = 0;
	virtual void		Release() = 0;
};

IRingBuffer	*CreateRingBuffer(const unsigned int uBufLen, const unsigned int uMaxPack);

#endif
