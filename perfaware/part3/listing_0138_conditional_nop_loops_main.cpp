/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 138
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

extern "C" void ConditionalNOP(u64 Count, u8 *Data);
#pragma comment (lib, "listing_0136_conditional_nop_loops")

struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"ConditionalNOP", ConditionalNOP},
};

enum branch_pattern
{
    BranchPattern_NeverTaken,
    BranchPattern_AlwaysTaken,
    BranchPattern_Every2,
    BranchPattern_Every3,
    BranchPattern_Every4,
    BranchPattern_CRTRandom,
    BranchPattern_OSRandom,
    
    BranchPattern_Count,
};

static char const *FillWithBranchPattern(branch_pattern Pattern, buffer Buffer)
{
    char const *PatternName = "UNKNOWN";
    
    if(Pattern == BranchPattern_OSRandom)
    {
        PatternName = "OSRandom";
        FillWithRandomBytes(Buffer);
    }
    else
    {
        for(u64 Index = 0; Index < Buffer.Count; ++Index)
        {
            u8 Value = 0;
            
            switch(Pattern)
            {
                case BranchPattern_NeverTaken:
                {
                    PatternName = "Never Taken";
                    Value = 0;
                } break;
                
                case BranchPattern_AlwaysTaken:
                {
                    PatternName = "AlwaysTaken";
                    Value = 1;
                } break;
                
                case BranchPattern_Every2:
                {
                    PatternName = "Every 2";
                    Value = ((Index % 2) == 0);
                } break;
                
                case BranchPattern_Every3:
                {
                    PatternName = "Every 3";
                    Value = ((Index % 3) == 0);
                } break;
                
                case BranchPattern_Every4:
                {
                    PatternName = "Every 4";
                    Value = ((Index % 4) == 0);
                } break;
                
                case BranchPattern_CRTRandom:
                {
                    PatternName = "CRTRandom";
                    // NOTE(casey): rand() actually isn't all that random, so, keep in mind
                    // that in the future we will look at better ways to get entropy for testing
                    // purposes!
                    Value = (u8)rand();
                } break;
                
                default:
                {
                    fprintf(stderr, "Unrecognized branch pattern.\n");
                } break;
            }
            
            Buffer.Data[Index] = Value;
        }
    }
    
    return PatternName;
}

int main(void)
{
    InitializeOSPlatform();
    
    buffer Buffer = AllocateBuffer(1*1024*1024*1024);
    if(IsValid(Buffer))
    {
        repetition_tester Testers[BranchPattern_Count][ArrayCount(TestFunctions)] = {};
        for(;;)
        {
            for(u32 Pattern = 0; Pattern < BranchPattern_Count; ++Pattern)
            {
                char const *PatternName = FillWithBranchPattern((branch_pattern)Pattern, Buffer);
                
                for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
                {
                    repetition_tester *Tester = &Testers[Pattern][FuncIndex];
                    test_function TestFunc = TestFunctions[FuncIndex];
                    
                    printf("\n--- %s, %s ---\n", TestFunc.Name, PatternName);
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
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&Buffer);
    
    return 0;
}
