/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 168
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0163_os_platform.cpp"
#include "listing_0164_csv_repetition_tester.cpp"
#include "listing_0167_osread_sum.cpp"

struct test_function
{
    char const *Name;
    file_process_func *Func;
};
static test_function TestFunctions[] =
{
    {"OpenAllocateAndFRead", OpenAllocateAndFRead},
    {"OpenAllocateAndSum", OpenAllocateAndSum},
    {"OpenAllocateAndSumOverlapped", OpenAllocateAndSumOverlapped},
};

int main(int ArgCount, char **Args)
{
    InitializeOSPlatform();
    
    if(ArgCount == 2)
    {
        char *FileName = Args[1];
        
        buffer Buffer = ReadEntireFile(FileName);
        repetition_test_series TestSeries = AllocateTestSeries(ArrayCount(TestFunctions), 1024);
        
        if(IsValid(Buffer) && IsValid(TestSeries))
        {
            u64 FileSize = Buffer.Count;
            u64 ReferenceSum = Sum64s(Buffer.Count, Buffer.Data);
            
            SetRowLabelLabel(&TestSeries, "ReadBufferSize");
            for(u64 ReadBufferSize = 256*1024; ReadBufferSize <= Buffer.Count; ReadBufferSize*=2)
            {
                SetRowLabel(&TestSeries, "%lluk", ReadBufferSize/1024);
                for(u32 TestFunctionIndex = 0; TestFunctionIndex < ArrayCount(TestFunctions); ++TestFunctionIndex)
                {
                    test_function Function = TestFunctions[TestFunctionIndex];
                    
                    SetColumnLabel(&TestSeries, "%s", Function.Name);
                    
                    repetition_tester Tester = {};
                    NewTestWave(&TestSeries, &Tester, FileSize, GetCPUTimerFreq());
                    
					b32 Passed = true;
                    while(IsTesting(&TestSeries, &Tester))
                    {
                        BeginTime(&Tester);
                        u64 Check = Function.Func(&Tester, FileName, FileSize, ReadBufferSize);
						if(Check != ReferenceSum)
						{
							Passed = false;
						}							
                        EndTime(&Tester);
                    }
                    
                    if(!Passed)
                    {
                        fprintf(stderr, "WARNING: Checksum mismatch\n");
                    }
                }
            }

            PrintCSVForValue(&TestSeries, StatValue_GBPerSecond, stdout);
        }
        else
        {
            fprintf(stderr, "ERROR: Test data size must be non-zero\n");
        }
        
        FreeBuffer(&Buffer);
        FreeTestSeries(&TestSeries);
    }
    else
    {
        fprintf(stderr, "Usage: %s [existing filename]\n", Args[0]);
    }
		
    return 0;
}
