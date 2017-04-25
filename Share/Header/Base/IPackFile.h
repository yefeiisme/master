#ifndef __I_PACK_FILE_H__
#define __I_PACK_FILE_H__

class IPakFile
{
public:
	virtual bool			Open(const char *pszFileName) = 0;
	virtual void			Close() = 0;
	virtual unsigned int	Read(void *pBuffer, unsigned int uSize) = 0;
	virtual unsigned int	Size() = 0;
	virtual void			Release() = 0;
};

IPakFile			*CreatePakFile();

#endif
