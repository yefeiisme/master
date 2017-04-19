#ifndef KTabFile_H
#define KTabFile_H

#include "KList.h"
#include "ITabFile.h"
#include "MemClass.h"

typedef struct tagTabOffset
{
	unsigned long		dwOffset;
	unsigned long		dwLength;
} TABOFFSET;

class KTabFile : public ITabFile
{
private:
	int			m_Width;
	int			m_Height;
	CMemClass	m_Memory;
	CMemClass	m_OffsetTable;
private:
	void		CreateTabOffset();
	bool		GetValue(const int nRow, const int nColumn, char *lpRString, const unsigned long dwSize);
public:
	KTabFile();
	~KTabFile();
	bool		Load(const char *FileName);
	bool		LoadPack(char *FileName);
	int			FindRow(char *szRow);//返回以1为起点的值
	int			FindColumn(char *szColumn);//返回以1为起点的值
	inline int	GetHeight() const
	{
		return m_Height;
	};
	bool		GetString(const int nRow, char *szColumn, char *lpDefault, char *lpRString, const unsigned long dwSize);
	bool		GetString(const int nRow, const int nColumn, char *lpDefault, char *lpRString, const unsigned long dwSize);
	bool		GetInteger(const int nRow, char *szColumn, const int nDefault, int *pnValue);
	bool		GetInteger(const int nRow, const int nColumn, const int nDefault, int *pnValue);
	bool		GetFloat(const int nRow, char *szColumn, const float fDefault, float *pfValue);
	bool		GetFloat(const int nRow, const int nColumn, const float fDefault, float *pfValue);
	void		Clear();
	void		Release();
};

#endif
