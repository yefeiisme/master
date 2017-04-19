#include "string.h"
#include "IRingBuffer.h"
#include "RingBuffer.h"

#ifndef NULL
#define	NULL	0
#endif

#define RB_SPACE	(sizeof(void*)*2)

CRingBuffer::CRingBuffer()
{
	m_pBuffer	= NULL;
	m_pRead		= NULL;
	m_pWrite	= NULL;
	m_pTemp		= NULL;
	m_pNextRead	= NULL;
	m_uBufLen	= 0;
	m_uMaxPack	= 0;
}

CRingBuffer::~CRingBuffer()
{
	delete []m_pBuffer;
	m_pBuffer	= NULL;
	m_pRead		= NULL;
	m_pWrite	= NULL;
	m_pTemp		= NULL;
	m_pNextRead	= NULL;
	m_uBufLen	= 0;
	m_uMaxPack	= 0;
}

bool CRingBuffer::Initialize(const unsigned int uBufLen, const unsigned int uMaxPack)
{
	if (uBufLen <= (2 * uMaxPack + RB_SPACE))
		return false;

	m_pBuffer	= new char[uBufLen];
	if (NULL == m_pBuffer)
		return false;

	memset(m_pBuffer, 0, uBufLen);

	m_uMaxPack	= uMaxPack;
	m_uBufLen	= uBufLen - (m_uMaxPack + RB_SPACE);	//隔开8字节
	m_pRead		= m_pBuffer;
	m_pWrite	= m_pBuffer;
	m_pTemp		= m_pBuffer + (m_uBufLen + RB_SPACE);
	m_pNextRead	= m_pBuffer;

	return true;
}

void CRingBuffer::Clear()
{
	m_pBuffer	= NULL;
	m_pRead		= NULL;
	m_pWrite	= NULL;
	m_pTemp		= NULL;
	m_pNextRead	= NULL;
	m_uBufLen	= 0;
	m_uMaxPack	= 0;
}

bool CRingBuffer::Reinit()
{
	if (NULL == m_pBuffer)
		return false;
	
	m_pRead		= m_pBuffer;
	m_pWrite	= m_pBuffer;
	m_pNextRead	= m_pBuffer;

	return true;
}
/*
 *	写入数据,返回右指针
 */
char* CRingBuffer::PutData(char *pRead, char *pWrite, char *pData, const unsigned int uDataSize)
{
	//右边空间
	unsigned int uRightMargin = m_uBufLen - (pWrite - m_pBuffer);

	if (pWrite >= pRead && uDataSize >= uRightMargin)
	{
		/*
		 *	没有环绕,但是右边空间不够,即将环绕
		 */
		memcpy(pWrite, pData, uRightMargin);
		pWrite = m_pBuffer;
		memcpy(pWrite, pData + uRightMargin, uDataSize - uRightMargin);
		pWrite += uDataSize - uRightMargin;
		
		if (pWrite > m_pBuffer + m_uBufLen)
			return NULL;

		return pWrite;
	}

	//环绕了,或者没有环绕，但是右边空间够用
	memcpy(pWrite, pData, uDataSize);
	pWrite += uDataSize;

	if (pWrite > m_pBuffer + m_uBufLen)
		return NULL;

	return pWrite;
}

unsigned CRingBuffer::GetDataLen(char *pRead, char *pWrite)
{
	//左指针右边空间
	unsigned int uRightMargin = m_uBufLen - (pRead - m_pBuffer);

	if (pWrite < pRead && sizeof(unsigned int) > uRightMargin)
	{
		/*
		 *	环绕了，但是数据长度不够读取
		 */
		unsigned int	uDataLen = 0;
		char			*ptr = (char*)&uDataLen;

		memcpy(ptr, pRead, uRightMargin);
		memcpy(ptr + uRightMargin, m_pBuffer, sizeof(unsigned int) - uRightMargin);

		return uDataLen;
	}
	
	return *((unsigned int*)pRead);
}

bool CRingBuffer::SndPack(const void *pData, const unsigned int uDataSize)
{
	if (NULL == pData || 0 == uDataSize)
		return false;
	
	if (NULL == m_pBuffer)
		return false;
	
	if (uDataSize > m_uMaxPack)
		return false;

	char	*pRead	= m_pRead;
	char	*pWrite	= m_pWrite;

	/*
	 *	判断是否环绕
	 */
	////////////////////////////////////////////////////////
	unsigned int	uContentSize    = pWrite >= pRead/*没有环绕*/ ? (pWrite - pRead) : m_uBufLen - (pRead - pWrite);

	if (uContentSize > m_uBufLen - 1)
		return false;

	unsigned int	uEmptySize      = m_uBufLen - uContentSize - 1;
	////////////////////////////////////////////////////////

	/*
	 *	没空间了
	 */
	if (uDataSize + sizeof(unsigned int) > uEmptySize)
		return false;

	//写入长度
	pWrite = PutData(pRead, pWrite, (char*)&uDataSize, sizeof(unsigned int));

	if (pWrite == NULL)
		return false;

	//写入数据
	pWrite = PutData(pRead, pWrite, (char*)pData, uDataSize);

	if (pWrite == NULL)
		return false;

	//修改指针
	m_pWrite = pWrite;

	return true;
}

const void *CRingBuffer::RcvPack(unsigned int &uDataSize)
{
	uDataSize = 0;

	if (m_pBuffer == NULL)
		return NULL;

	//释放上一次内容
	m_pRead			= m_pNextRead;
	char	*pRead	= m_pRead;
	char	*pWrite	= m_pWrite;

	/*
	 *	判断是否环绕
	 */
	////////////////////////////////////////////////////////
	unsigned int uContentSize	= pWrite >= pRead/*没有环绕*/ ? pWrite - pRead : m_uBufLen - (pRead - pWrite);

	if (uContentSize > m_uBufLen - 1)
		return NULL;
	////////////////////////////////////////////////////////

	if (uContentSize <= sizeof(unsigned int))
		return NULL;

	uDataSize = GetDataLen(pRead, pWrite);

	if (uContentSize < uDataSize || uDataSize > m_uMaxPack)
	{
		/*
		 *	不应该产生的情况
		 */
		uDataSize = 0;
		return NULL;
	}

	/*
	 *	一起拷贝,总长度
	 */
	unsigned int	uReadLen = uDataSize + sizeof(unsigned int);
	//左指针右边空间
	unsigned int	uRightMargin = m_uBufLen - (pRead - m_pBuffer);

	if (pWrite < pRead && uReadLen >= uRightMargin)
	{
		/*
		 *	环绕了，但是数据长度不够读取
		 */
		memcpy(m_pTemp, pRead, uRightMargin);

		memcpy(m_pTemp + uRightMargin, m_pBuffer, uReadLen - uRightMargin);

		if (uReadLen - uRightMargin > m_uBufLen)
		{
			uDataSize = 0;
			return NULL;
		}
		//修改指针
		m_pNextRead = m_pBuffer + (uReadLen - uRightMargin);

		return (m_pTemp + sizeof(unsigned int));
	}
	else
	{
		//修改指针
		if (m_pNextRead + uReadLen > m_pBuffer + m_uBufLen)
		{
			uDataSize = 0;
			return NULL;
		}

		m_pNextRead += uReadLen; 
		return (pRead + sizeof(unsigned int));
	}
}

void CRingBuffer::Release()
{
	delete this;
}

IRingBuffer	*CreateRingBuffer(const unsigned int uBufLen, const unsigned int uMaxPack)
{
	CRingBuffer	*pRingBuffer	= new CRingBuffer();
	if (NULL == pRingBuffer)
		return NULL;

	if (!pRingBuffer->Initialize(uBufLen, uMaxPack))
	{
		delete pRingBuffer;
		return NULL;
	}

	return pRingBuffer;
}
