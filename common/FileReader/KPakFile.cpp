#include "stdafx.h"
#include "KFilePath.h"
#include "KPakFile.h"

KPakFile::KPakFile()
{
}

KPakFile::~KPakFile()
{
	Close();
}

//---------------------------------------------------------------------------
// 功能:	打开一个文件, 先寻找当前目录下是否有同名的单独文件,
// 参数:	FileName	文件名
// 返回:	TRUE		成功
//			FALSE		失败
//---------------------------------------------------------------------------

#include "string.h"

bool KPakFile::Open(const char* pszFileName)
{
	if (pszFileName == NULL || pszFileName[0] == 0)
		return false;

	bool bOk = false;
	Close();

	char szPackNameAddtion[256];

#ifdef WIN32
	int  len=strlen(pszFileName);
	int  iDestIndex=0;
	
	for (int i=0;i<len;i++)
	{
		if (pszFileName[i]!='/')
		{ 
			if (pszFileName[i]=='\\')
			{
				if (pszFileName[i+1]!='\\')
				{
					szPackNameAddtion[iDestIndex]=pszFileName[i];
					iDestIndex++;
				}//endif
				
			}//endif
			else
			{
				szPackNameAddtion[iDestIndex]=pszFileName[i];
				iDestIndex++;
			}//endelse
		}//endif
		else
		{
			szPackNameAddtion[iDestIndex]='\\';
			iDestIndex++;
		}//end else
		
	}//end for i

	szPackNameAddtion[iDestIndex]=0;
#else
	strcpy(szPackNameAddtion, pszFileName);
#endif

		bOk = (m_File.Open((char*)szPackNameAddtion) != FALSE);
	
	return bOk;
}

//---------------------------------------------------------------------------
// 功能:	从文件中读取数据
// 参数:	pBuffer		缓冲区指针
//			dwSize		要读取的长度
// 返回:	读到的字节长度
//---------------------------------------------------------------------------
unsigned int KPakFile::Read(void* pBuffer, unsigned int uSize)
{
	uSize = m_File.Read(pBuffer, uSize);

	return uSize;
}

//---------------------------------------------------------------------------
// 功能:	返回文件大小
// 返回:	文件的大小 in bytes
//---------------------------------------------------------------------------
unsigned int KPakFile::Size()
{
	unsigned int uSize;

	uSize = m_File.Size();
	return uSize;
}
//---------------------------------------------------------------------------
// 功能:	关闭一个文件
//---------------------------------------------------------------------------
void KPakFile::Close()
{
	m_File.Close();
}

void KPakFile::Release()
{
	Close();
	delete this;
}

IPakFile *CreatePakFile()
{
	KPakFile *pPakFile = new KPakFile;
	return pPakFile;
}
