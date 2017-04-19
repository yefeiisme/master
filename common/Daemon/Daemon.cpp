#include <stdio.h>
#include <stdlib.h>
#ifdef WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>
#endif
#include "commondefine.h"
#include "Daemon.h"

#ifdef WIN32

bool DaemonInit(OnQuitSignal pfnQuitSignal)
{
	HANDLE      hConsole    = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(hConsole, FOREGROUND_GREEN | FOREGROUND_INTENSITY);

	SetConsoleCtrlHandler(pfnQuitSignal, true);

	return true;
}

#else

bool DaemonInit(OnQuitSignal pfnQuitSignal)
{
	pid_t pid;

	if ((pid = fork()) < 0)
		return false;

	if (pid)
		exit(0);

	for (int nIndex = 0; nIndex < 20; ++nIndex)
		close(nIndex);

	setsid();
	umask(0);

	rlimit  ResLimit;

	ResLimit.rlim_cur = SHRT_MAX;
	ResLimit.rlim_max = SHRT_MAX;

	if (0 != setrlimit(RLIMIT_NOFILE, &ResLimit))
		return false;

	if (0 != getrlimit(RLIMIT_CORE, &ResLimit))
		return false;

	ResLimit.rlim_cur = ResLimit.rlim_max;

	if (0 != setrlimit(RLIMIT_CORE, &ResLimit))
		return false;

	signal(SIGINT, pfnQuitSignal);
	signal(SIGQUIT, pfnQuitSignal);
	signal(SIGTERM, pfnQuitSignal); 

	return true;
}

#endif

void yield(const unsigned int uilTime)
{
#ifdef WIN32
	Sleep(uilTime);
#else
	struct timeval sleeptime;
	sleeptime.tv_sec = 0;
	sleeptime.tv_usec=uilTime*1000;
	select(0, 0, 0, 0, &sleeptime);
#endif
}

uint64 GetMicroTick()
{
#ifdef WIN32
	return GetTickCount();
#else
	timeval now = {0,0};
	gettimeofday(&now, NULL);
	return	(now.tv_sec * 1000 + now.tv_usec / 1000);
#endif
}
