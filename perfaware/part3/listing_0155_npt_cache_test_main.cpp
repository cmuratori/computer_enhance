/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 155
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

extern "C" void DoubleLoopRead_32x8(u64 Count, u8 *Data, u64 Mask);
#pragma comment (lib, "listing_0154_npt_cache_test")

int main(void)
{
    InitializeOSPlatform();
    
    u64 RegionSizes[64] = {};
    repetition_tester Testers[ArrayCount(RegionSizes)] = {};
    u64 InnerLoopSize = 256;
    
    u64 SizeDelta = 512;
    u64 AccumulatedSize = 4*1024;
    for(u32 TestIndex = 0; TestIndex < ArrayCount(Testers); ++TestIndex)
    {
        RegionSizes[TestIndex] = AccumulatedSize;
        
        // NOTE(casey): Increase the delta every time we hit an even power of two
        // (just so we don't have to do too many tests to cover the typical cache ranges)
        if(((AccumulatedSize - 1) & AccumulatedSize) == 0)
        {
            SizeDelta *= 2;
        }
        AccumulatedSize += SizeDelta;
    }
    
    buffer Buffer = AllocateBuffer(1024ull*1024*1024);
    if(IsValid(Buffer))
    {
        // NOTE(casey): Because OSes may not map allocated pages until they are written to, we write garbage
        // to the entire buffer to force it to be mapped.
        for(u64 ByteIndex = 0; ByteIndex < Buffer.Count; ++ByteIndex)
        {
            Buffer.Data[ByteIndex] = (u8)ByteIndex;
        }
        
        for(u32 TestIndex = 0; TestIndex < ArrayCount(Testers); ++TestIndex)
        {
            repetition_tester *Tester = Testers + TestIndex;
            
            u64 RegionSize = RegionSizes[TestIndex];

            u64 OuterLoopCount = Buffer.Count/RegionSize;
            u64 InnerLoopCount = RegionSize/InnerLoopSize;
            u64 TotalSize = OuterLoopCount*RegionSize;
            
            printf("\n--- Read32x8, %llu outer x %llu inner x %llu bytes = %llu bytes in %lluk chunks ---\n",
                   OuterLoopCount, InnerLoopCount, InnerLoopSize, TotalSize, RegionSize/1024);
            
            NewTestWave(Tester, TotalSize, GetCPUTimerFreq());
            
            while(IsTesting(Tester))
            {
                BeginTime(Tester);
                DoubleLoopRead_32x8(OuterLoopCount, Buffer.Data, InnerLoopCount);
                EndTime(Tester);
                CountBytes(Tester, TotalSize);
            }
        }
        
        printf("Region Size,gb/s\n");
        for(u32 TestIndex = 0; TestIndex < ArrayCount(Testers); ++TestIndex)
        {
            repetition_tester *Tester = Testers + TestIndex;
            
            repetition_value Value = Tester->Results.Min;
            f64 Seconds = SecondsFromCPUTime((f64)Value.E[RepValue_CPUTimer], Tester->CPUTimerFreq);
            f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
            f64 Bandwidth = Value.E[RepValue_ByteCount] / (Gigabyte * Seconds);
                
            printf("%llu,%f\n", RegionSizes[TestIndex], Bandwidth);
        }
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&Buffer);
    
    return 0;
}
