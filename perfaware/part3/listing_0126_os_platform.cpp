/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 126
   ======================================================================== */

static u64 EstimateCPUTimerFreq(void);

#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

#pragma comment (lib, "advapi32.lib")

struct os_platform
{
    b32 Initialized;
    u64 LargePageSize; // NOTE(casey): This will be 0 when large pages are not supported (which is most of the time!)
    HANDLE ProcessHandle;
    u64 CPUTimerFreq;
};
static os_platform GlobalOSPlatform;

static u64 GetOSTimerFreq(void)
{
	LARGE_INTEGER Freq;
	QueryPerformanceFrequency(&Freq);
	return Freq.QuadPart;
}

static u64 ReadOSTimer(void)
{
	LARGE_INTEGER Value;
	QueryPerformanceCounter(&Value);
	return Value.QuadPart;
}

static u64 ReadOSPageFaultCount(void)
{
    PROCESS_MEMORY_COUNTERS_EX MemoryCounters = {};
    MemoryCounters.cb = sizeof(MemoryCounters);
    GetProcessMemoryInfo(GlobalOSPlatform.ProcessHandle, (PROCESS_MEMORY_COUNTERS *)&MemoryCounters, sizeof(MemoryCounters));
    
    u64 Result = MemoryCounters.PageFaultCount;
    return Result;
}

static u64 TryToEnableLargePages(void)
{
    u64 Result = 0;
    
    HANDLE TokenHandle;
    if(OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &TokenHandle))
    {
        TOKEN_PRIVILEGES Privs = {};
        Privs.PrivilegeCount = 1;
        Privs.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        if(LookupPrivilegeValue(0, SE_LOCK_MEMORY_NAME, &Privs.Privileges[0].Luid))
        {
            AdjustTokenPrivileges(TokenHandle, FALSE, &Privs, 0, 0, 0);
            if(GetLastError() == ERROR_SUCCESS)
            {
                Result = GetLargePageMinimum();
            }
        }
        
        CloseHandle(TokenHandle);
    }
    
    return Result;
}

static void InitializeOSPlatform(void)
{
    if(!GlobalOSPlatform.Initialized)
    {
        GlobalOSPlatform.Initialized = true;
        GlobalOSPlatform.LargePageSize = TryToEnableLargePages();
        GlobalOSPlatform.ProcessHandle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
        GlobalOSPlatform.CPUTimerFreq = EstimateCPUTimerFreq();
    }
}

#else

#include <x86intrin.h>
#include <sys/time.h>

struct os_platform
{
    b32 Initialized;
    u64 CPUTimerFreq;
};
static os_platform GlobalOSPlatform;

static u64 GetOSTimerFreq(void)
{
	return 1000000;
}

static u64 ReadOSTimer(void)
{
	// NOTE(casey): The "struct" keyword is not necessary here when compiling in C++,
	// but just in case anyone is using this file from C, I include it.
	struct timeval Value;
	gettimeofday(&Value, 0);
	
	u64 Result = GetOSTimerFreq()*(u64)Value.tv_sec + (u64)Value.tv_usec;
	return Result;
}

static u64 ReadOSPageFaultCount(void)
{
    // NOTE(casey): The course materials are not tested on MacOS/Linux.
    // This code was contributed to the public github. It may or may not work
    // for your system.
    
    struct rusage Usage = {};
    getrusage(RUSAGE_SELF, &Usage);
    
    // ru_minflt  the number of page faults serviced without any I/O activity.
    // ru_majflt  the number of page faults serviced that required I/O activity.
    u64 Result = Usage.ru_minflt + Usage.ru_majflt;
    
    return Result;
}

static void InitializeOSPlatform(void)
{
    if(!GlobalOSPlatform.Initialized)
    {
        GlobalOSPlatform.Initialized = true;
        GlobalOSPlatform.CPUTimerFreq = EstimateCPUTimerFreq();
    }
}

#endif

/* NOTE(casey): These do not need to be "inline", it could just be "static"
   because compilers will inline it anyway. But compilers will warn about
   static functions that aren't used. So "inline" is just the simplest way
   to tell them to stop complaining about that. */
inline u64 ReadCPUTimer(void)
{
	// NOTE(casey): If you were on ARM, you would need to replace __rdtsc
	// with one of their performance counter read instructions, depending
	// on which ones are available on your platform.
	
	return __rdtsc();
}

inline u64 GetCPUTimerFreq(void)
{
    u64 Result = GlobalOSPlatform.CPUTimerFreq;
    return Result;
}

inline u64 GetLargePageSize(void)
{
    u64 Result = GlobalOSPlatform.LargePageSize;
    return Result;
}

static u64 EstimateCPUTimerFreq(void)
{
	u64 MillisecondsToWait = 100;
	u64 OSFreq = GetOSTimerFreq();

	u64 CPUStart = ReadCPUTimer();
	u64 OSStart = ReadOSTimer();
	u64 OSEnd = 0;
	u64 OSElapsed = 0;
	u64 OSWaitTime = OSFreq * MillisecondsToWait / 1000;
	while(OSElapsed < OSWaitTime)
	{
		OSEnd = ReadOSTimer();
		OSElapsed = OSEnd - OSStart;
	}
	
	u64 CPUEnd = ReadCPUTimer();
	u64 CPUElapsed = CPUEnd - CPUStart;
	
	u64 CPUFreq = 0;
	if(OSElapsed)
	{
		CPUFreq = OSFreq * CPUElapsed / OSElapsed;
	}
	
	return CPUFreq;
}
