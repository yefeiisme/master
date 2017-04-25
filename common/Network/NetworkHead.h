#ifndef __NETWORK_HEAD_H_
#define __NETWORK_HEAD_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <errno.h>

#if defined(WIN32) || defined(WIN64)
#include <Winsock2.h>
#include <Ws2tcpip.h>

#define socklen_t	int
#else
#include <ctype.h>
#include <pthread.h>
#include <fcntl.h>
#include <algorithm>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#define SOCKET int
#define INVALID_SOCKET -1

#define closesocket close
#endif

#ifndef __linux
#define MSG_NOSIGNAL	0
#endif

#ifndef NULL
#define	NULL	0
#endif

#ifndef SAFE_DELETE
#define SAFE_DELETE(Address)				{ if( NULL != (Address) )	delete(Address);	Address = NULL; }
#endif
#ifndef SAFE_DELETE_ARR
#define SAFE_DELETE_ARR(Address)			{ if( NULL != (Address) )	delete[](Address);	Address = NULL; }
#endif

#endif
