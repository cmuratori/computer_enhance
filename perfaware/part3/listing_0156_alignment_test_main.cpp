/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 156
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

struct test
{
    repetition_tester Tester;
    
    u64 Alignment;
    u64 RegionSize;
};

int main(void)
{
    InitializeOSPlatform();

    u64 MaxSize = 1024ull*1024*1024;
    u32 MaxAlignment = 256;
    
    u64 Alignments[] = {0, 1, 2, 3, 15, 16, 17, 31, 32, 33, 63, 64, 65};
    u64 RegionSizes[] = {8*1024, 128*1024, 4*1024*1024, MaxSize};
    
    test Tests[ArrayCount(Alignments)][ArrayCount(RegionSizes)] = {};
    u64 InnerLoopSize = 256;
    
    for(u32 AlignmentIndex = 0; AlignmentIndex < ArrayCount(Alignments); ++AlignmentIndex)
    {
        for(u32 RegionIndex = 0; RegionIndex < ArrayCount(RegionSizes); ++RegionIndex)
        {
            test *Test = &Tests[AlignmentIndex][RegionIndex];
            Test->Alignment = Alignments[AlignmentIndex];
            Test->RegionSize = RegionSizes[RegionIndex];
        }
    }
    
    buffer Buffer = AllocateBuffer(MaxSize + MaxAlignment);
    if(IsValid(Buffer))
    {
        // NOTE(casey): Because OSes may not map allocated pages until they are written to, we write garbage
        // to the entire buffer to force it to be mapped.
        for(u64 ByteIndex = 0; ByteIndex < Buffer.Count; ++ByteIndex)
        {
            Buffer.Data[ByteIndex] = (u8)ByteIndex;
        }
        
        for(u32 AlignmentIndex = 0; AlignmentIndex < ArrayCount(Alignments); ++AlignmentIndex)
        {
            for(u32 RegionIndex = 0; RegionIndex < ArrayCount(RegionSizes); ++RegionIndex)
            {
                test *Test = &Tests[AlignmentIndex][RegionIndex];
                repetition_tester *Tester = &Test->Tester;
                
                u64 RegionSize = Test->RegionSize;
                u64 Alignment = Test->Alignment;
                
                u64 OuterLoopCount = MaxSize/RegionSize;
                u64 InnerLoopCount = RegionSize/InnerLoopSize;
                u64 TotalSize = OuterLoopCount*RegionSize;
                
                printf("\n--- Read32x8, %llu outer x %llu inner x %llu bytes = %llu bytes in %lluk chunks, alignment %llu ---\n",
                       OuterLoopCount, InnerLoopCount, InnerLoopSize, TotalSize, RegionSize/1024, Alignment);
                
                NewTestWave(Tester, TotalSize, GetCPUTimerFreq());
                
                while(IsTesting(Tester))
                {
                    BeginTime(Tester);
                    DoubleLoopRead_32x8(OuterLoopCount, Buffer.Data + Alignment, InnerLoopCount);
                    EndTime(Tester);
                    CountBytes(Tester, TotalSize);
                }
            }
        }
        
        printf("Alignment");
        for(u32 RegionIndex = 0; RegionIndex < ArrayCount(RegionSizes); ++RegionIndex)
        {
            test *Test = &Tests[0][RegionIndex];
            printf(",%llu bytes", Test->RegionSize);
        }
        printf("\n");
        
        for(u32 AlignmentIndex = 0; AlignmentIndex < ArrayCount(Alignments); ++AlignmentIndex)
        {
            printf("%llu", Tests[AlignmentIndex][0].Alignment);
            for(u32 RegionIndex = 0; RegionIndex < ArrayCount(RegionSizes); ++RegionIndex)
            {
                test *Test = &Tests[AlignmentIndex][RegionIndex];
                repetition_tester *Tester = &Test->Tester;
                
                repetition_value Value = Tester->Results.Min;
                f64 Seconds = SecondsFromCPUTime((f64)Value.E[RepValue_CPUTimer], Tester->CPUTimerFreq);
                f64 Gigabyte = (1024.0f * 1024.0f * 1024.0f);
                f64 Bandwidth = Value.E[RepValue_ByteCount] / (Gigabyte * Seconds);
                
                printf(",%f", Bandwidth);
            }
            printf("\n");
        }
    }
    else
    {
        fprintf(stderr, "Unable to allocate memory buffer for testing");
    }
    
    FreeBuffer(&Buffer);
    
    return 0;
}
