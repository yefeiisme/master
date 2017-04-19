#ifndef KFile_H
#define KFile_H

#define SEEK_ERROR		0xFFFFFFFF

class KFile
{
private:
	FILE *		m_hFile;	// File Handle
	DWORD		m_dwLen;	// File Size
	DWORD		m_dwPos;	// File Pointer
public:
	KFile();
	~KFile();
	BOOL		Open(LPSTR FileName);
	BOOL		Create(LPSTR FileName);
	BOOL		Append(LPSTR FileName);
	void		Close();
	DWORD		Read(LPVOID lpBuffer, DWORD dwReadBytes);
	DWORD		Write(LPVOID lpBuffer, DWORD dwWriteBytes);
	DWORD		Seek(LONG lDistance, DWORD dwMoveMethod);
	DWORD		Tell();
	DWORD		Size();
};

#ifndef WIN32

char* strlwr(char* string);
char* strupr(char* string);
void xtoa(unsigned long val, char *buf, unsigned radix, int is_neg);
char* itoa(int val, char *buf, int radix);

#endif

#endif
