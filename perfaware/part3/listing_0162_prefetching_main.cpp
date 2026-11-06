/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 162
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

extern "C" void PeriodicRead(u64 OuterLoopCount, u8 *Data, u64 InnterLoopCount);
extern "C" void PeriodicPrefetchedRead(u64 OuterLoopCount, u8 *Data, u64 InnterLoopCount);
#pragma comment (lib, "listing_0161_prefetching")

typedef void ASMFunction(u64 OuterLoopCount, u8 *Data, u64 InnterLoopCount);
struct test_function
{
    char const *Name;
    ASMFunction *Func;
};
test_function TestFunctions[] =
{
    {"PeriodicRead", PeriodicRead},
    {"PeriodicPrefetchedRead", PeriodicPrefetchedRead},
};

int main(void)
{
    InitializeOSPlatform();
    
    repetition_tester Testers[32][ArrayCount(TestFunctions)] = {};
    u64 InnerLoopCounts[ArrayCount(Testers)] = {};
    
    buffer Buffer = AllocateBuffer(1*1024*1024*1024);
    if(IsValid(Buffer))
    {
#define OuterLoopCount (1024*1024)
        u64 CacheLineSize = 64;
        u64 TestSize = OuterLoopCount*CacheLineSize;
        
        // NOTE(casey): Initialize the buffer to a random jump pattern
        u64 CacheLineCount = Buffer.Count / CacheLineSize;
        u64 JumpOffset = 0;
        for(u64 OuterLoopIndex = 0; OuterLoopIndex < OuterLoopCount; ++OuterLoopIndex)
        {
            // NOTE(casey): Make sure we don't get a collision in the random offsets
            u64 NextOffset = 0;
            u64 *NextPointer = 0;

            u64 RandomValue = 0;
            ReadOSRandomBytes(sizeof(RandomValue), &RandomValue);
			b32 Found = false;
            for(u64 SearchIndex = 0; SearchIndex < CacheLineCount; ++SearchIndex)
            {
				NextOffset = (RandomValue + SearchIndex) % CacheLineCount;
                NextPointer = (u64 *)(Buffer.Data + NextOffset*CacheLineSize);
                if(*NextPointer == 0)
                {
					Found = true;
                    break;
                }
            }
			
			if(!Found)
			{
				fprintf(stderr, "ERROR: Unable to create single continuous pointer chain.\n");
			}
            
            // NOTE(casey): Write the next pointer and some "data" to the cache line
            u64 *JumpData = (u64 *)(Buffer.Data + JumpOffset*CacheLineSize);
            JumpData[0] = (u64)(NextPointer);
            JumpData[1] = OuterLoopIndex;
            
            JumpOffset = NextOffset;
        }
        
        for(u64 InnerLoopIndex = 0; InnerLoopIndex < ArrayCount(Testers); ++InnerLoopIndex)
        {
            u64 InnerLoopCount = 4*(InnerLoopIndex + 1);
			if(InnerLoopIndex >= 16)
			{
				InnerLoopCount = (64*(InnerLoopIndex - 14));
			}
			
            InnerLoopCounts[InnerLoopIndex] = InnerLoopCount;
            
            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
                repetition_tester *Tester = &Testers[InnerLoopIndex][FuncIndex];
                
                test_function TestFunc = TestFunctions[FuncIndex];
                
                printf("\n--- %s (%llu inner loop iterations)---\n", TestFunc.Name, InnerLoopCount);
                NewTestWave(Tester, TestSize, GetCPUTimerFreq());
                
                while(IsTesting(Tester))
                {
                    BeginTime(Tester);
                    TestFunc.Func(OuterLoopCount, Buffer.Data, InnerLoopCount);
                    EndTime(Tester);
                    CountBytes(Tester, TestSize);
                }
            }
        }
        
        printf("InnerLoopCount");
        for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
        {
            printf(",%s", TestFunctions[FuncIndex].Name);
        }
        printf("\n");
        
        for(u64 InnerLoopIndex = 0; InnerLoopIndex < ArrayCount(Testers); ++InnerLoopIndex)
        {
            printf("%llu", InnerLoopCounts[InnerLoopIndex]);
            for(u32 FuncIndex = 0; FuncIndex < ArrayCount(TestFunctions); ++FuncIndex)
            {
                repetition_tester *Tester = &Testers[InnerLoopIndex][FuncIndex];
                
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
