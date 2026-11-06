/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 140
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
#include "listing_0137_os_platform.cpp"
#include "listing_0109_pagefault_repetition_tester.cpp"

typedef void ASMFunction(u64 Count, u8 *Data);

extern "C" void NOPAligned64(u64 Count, u8 *Data);
extern "C" void NOPAligned1(u64 Count, u8 *Data);
extern "C" void NOPAligned15(u64 Count, u8 *Data);
extern "C" void NOPAligned31(u64 Count, u8 *Data);
extern "C" void NOPAligned63(u64 Count, u8 *Data);
#pragma comment (lib, "listing_0139_jump_alignment")

struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"NOPAligned64", NOPAligned64},
    {"NOPAligned1", NOPAligned1},
    {"NOPAligned15", NOPAligned15},
    {"NOPAligned31", NOPAligned31},
    {"NOPAligned63", NOPAligned63},
};

int main(void)
{
    InitializeOSPlatform();
    
    buffer Buffer = AllocateBuffer(1*1024*1024*1024);
    if(IsValid(Buffer))
    {
        repetition_tester Testers[ArrayCount(TestFunctions)] = {};
        for(;;)
        {
            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
                repetition_tester *Tester = &Testers[FuncIndex];
                test_function TestFunc = TestFunctions[FuncIndex];
                
                printf("\n--- %s ---\n", TestFunc.Name);
                NewTestWave(Tester, Buffer.Count, GetCPUTimerFreq());
                
                while(IsTesting(Tester))
                {
                    BeginTime(Tester);
                    TestFunc.Func(Buffer.Count, Buffer.Data);
                    EndTime(Tester);
                    CountBytes(Tester, Buffer.Count);
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&Buffer);
    
    return 0;
}
