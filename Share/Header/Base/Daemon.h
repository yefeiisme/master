#ifndef __DAEMON_H_
#define __DAEMON_H_

#include "commondefine.h"

#ifdef WIN32
#include "windows.h"
typedef BOOL (WINAPI *OnQuitSignal)(DWORD CtrlType);
#else
typedef void (*OnQuitSignal)(int nSignal);
#endif

bool	DaemonInit(OnQuitSignal pfnQuitSignal);
void	yield(const unsigned int ulTime);
uint64	GetMicroTick();

#endif
