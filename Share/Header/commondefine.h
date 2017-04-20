#ifndef __COMMON_DEFINE_H_
#define __COMMON_DEFINE_H_



typedef unsigned long		DWORD;  //%lu
typedef unsigned char		BYTE;	//%hhu
typedef unsigned short		WORD;	//%hu
typedef unsigned int		UINT;	//%u
typedef void*				LPVOID;
typedef int					BATTLETYPE; //%d
#ifdef WIN32
typedef unsigned __int64	uint64;		//%llu
typedef __int64				int64;		//%lld
#else
typedef unsigned long long	uint64;
typedef long long			int64;
#endif

#ifndef NULL
#define NULL    0
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(Address)				{ if( NULL != (Address) )	delete(Address);	Address = NULL; }
#endif
#ifndef SAFE_DELETE_ARR
#define SAFE_DELETE_ARR(Address)			{ if( NULL != (Address) )	delete[](Address);	Address = NULL; }
#endif

#ifndef STRING_INSURE
#define STRING_INSURE(dest) {dest[sizeof(dest) - 1] = '\0';}
#endif

#ifndef STRING_COPY
#define STRING_COPY(dest, src) {strncpy(dest, src, sizeof(dest)); dest[sizeof(dest) - 1] = '\0';}
#endif

#ifndef STRING_COPY_SIZE
#define STRING_COPY_SIZE(dest, src, size) {strncpy(dest, src, size); dest[size - 1] = '\0';}
#endif

#pragma	pack(push, 1)
typedef struct __SDefinedProtocolHead
{
	BYTE			byProtocol;
}SDefinedProtocolHead;

typedef struct __SUndefinedProtocolHead
{
	BYTE			byProtocol;
	WORD			wSize;
}SUndefinedProtocolHead;

#pragma pack(pop)

#define MAX_TEMP_CHAR_LEN				128




#endif
