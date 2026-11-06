/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 133
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0126_os_platform.cpp"
#include "listing_0109_pagefault_repetition_tester.cpp"
#include "listing_0127_largepageread_overhead_test.cpp"
#include "listing_0131_front_end_test.cpp"

struct test_function
{
    char const *Name;
    read_overhead_test_func *Func;
};
test_function TestFunctions[] =
{
    {"WriteToAllBytes", WriteToAllBytes},
    {"MOVAllBytes", MOVAllBytes},
    {"NOPAllBytes", NOPAllBytes},
    {"CMPAllBytes", CMPAllBytes},
    {"DECAllBytes", DECAllBytes},
};

int main(int ArgCount, char **Args)
{
    InitializeOSPlatform();
    
    if(ArgCount == 2)
    {
        char *FileName = Args[1];
#if _WIN32
        struct __stat64 Stat;
        _stat64(FileName, &Stat);
#else
        struct stat Stat;
        stat(FileName, &Stat);
#endif
        
        read_parameters Params = {};
        Params.Dest = AllocateBuffer(Stat.st_size);
        Params.FileName = FileName;
    
        if(Params.Dest.Count > 0)
        {
            repetition_tester Testers[ArrayCount(TestFunctions)] = {};
            
            for(;;)
            {
                for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
                {
                    repetition_tester *Tester = &Testers[FuncIndex];
                    test_function TestFunc = TestFunctions[FuncIndex];
                    
                    printf("\n--- %s ---\n", TestFunc.Name);
                    NewTestWave(Tester, Params.Dest.Count, GetCPUTimerFreq());
                    TestFunc.Func(Tester, &Params);
                }
            }
            
            // NOTE(casey): We would normally call this here, but we can't because the compiler will complain about "unreachable code".
            // So instead we just reference the pointer to prevent the compiler complaining about unused function :(
            (void)&FreeBuffer;
        }
        else
        {
            fprintf(stderr, "ERROR: Test data size must be non-zero\n");
        }
    }
    else
    {
        fprintf(stderr, "Usage: %s [existing filename]\n", Args[0]);
    }

    // NOTE(casey): We don't use these functions
    (void)&DescribeAllocationType;
    (void)&ReadViaFRead;
    (void)&ReadViaRead;
    (void)&ReadViaReadFile;

    return 0;
}
