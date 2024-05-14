/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 160
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

extern "C" void StandardWrite(u64 Count, u8 *Data, u64 InnterLoopCount, u8 *Table);
extern "C" void StreamingWrite(u64 Count, u8 *Data, u64 InnterLoopCount, u8 *Table);
#pragma comment (lib, "listing_0159_nontemporal_stores")

typedef void ASMFunction(u64 Count, u8 *Data, u64 InnterLoopCount, u8 *Table);
struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"StandardWrite", StandardWrite},
    {"StreamingWrite", StreamingWrite},
};

int main(void)
{
    InitializeOSPlatform();
    
    buffer Source = AllocateBuffer(1024*1024);
    buffer Dest = AllocateBuffer(1*1024*1024*1024);
    if(IsValid(Source) && IsValid(Dest))
    {
        // NOTE(casey): Initialize the source to a repeating pattern we can check later
        for(u64 ByteIndex = 0; ByteIndex < Source.Count; ++ByteIndex)
        {
            Source.Data[ByteIndex] = (u8)ByteIndex;
        }
        
        u64 InnerLoopSize = 256;
        for(u64 SourceCount = InnerLoopSize; SourceCount <= Source.Count; SourceCount*=2)
        {
            u64 InnerLoopCount = SourceCount/InnerLoopSize;
            u64 OuterLoopCount = (Dest.Count/(InnerLoopSize*InnerLoopCount));
            
            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
                // NOTE(casey): Clear the dest so we can verify that the copy works correctly
                for(u64 ByteIndex = 0; ByteIndex < Dest.Count; ++ByteIndex)
                {
                    Dest.Data[ByteIndex] = 0;
                }
                
                repetition_tester Tester = {};
                test_function TestFunc = TestFunctions[FuncIndex];
                
                printf("\n--- %s (%llu byte source)---\n", TestFunc.Name, SourceCount);
                NewTestWave(&Tester, Dest.Count, GetCPUTimerFreq());
                
                while(IsTesting(&Tester))
                {
                    BeginTime(&Tester);
                    TestFunc.Func(OuterLoopCount, Source.Data, InnerLoopCount, Dest.Data);
                    EndTime(&Tester);
                    CountBytes(&Tester, Dest.Count);
                }

                for(u64 ByteIndex = 0; ByteIndex < Dest.Count; ++ByteIndex)
                {
                    if(Dest.Data[ByteIndex] != Source.Data[ByteIndex % SourceCount])
                    {
                        fprintf(stderr, "ERROR: Destination written incorrectly.");
                        break;
                    }
                }
            }
        }
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&Source);
    FreeBuffer(&Dest);
    
    return 0;
}
