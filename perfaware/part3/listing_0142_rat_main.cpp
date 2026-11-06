/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 142
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

typedef void ASMFunction();

extern "C" void RATAdd();
extern "C" void RATMovAdd();
#pragma comment (lib, "listing_0141_rat")

struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"RATAdd", RATAdd},
    {"RATMovAdd", RATMovAdd},
};

int main(void)
{
    InitializeOSPlatform();
    
    u64 LoopCount = 1000000000;
    
    repetition_tester Testers[ArrayCount(TestFunctions)] = {};

    for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
    {
        repetition_tester *Tester = &Testers[FuncIndex];
        test_function TestFunc = TestFunctions[FuncIndex];
        
        printf("\n--- %s ---\n", TestFunc.Name);
        NewTestWave(Tester, LoopCount, GetCPUTimerFreq());
        
        while(IsTesting(Tester))
        {
            BeginTime(Tester);
            TestFunc.Func();
            EndTime(Tester);
            CountBytes(Tester, LoopCount);
        }
    }
    
    return 0;
}
