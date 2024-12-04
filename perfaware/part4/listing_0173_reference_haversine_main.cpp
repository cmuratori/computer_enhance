/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 173
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
#include "listing_0169_os_platform.cpp"
#include "listing_0164_csv_repetition_tester.cpp"
#include "listing_0172_reference_haversine.cpp"

struct test_function
{
    char const *Name;
    haversine_compute_func *Compute;
    haversine_verify_func *Verify;
};
static test_function TestFunctions[] =
{
    {"ReferenceHaversine", ReferenceSumHaversine, ReferenceVerifyHaversine},
};

int main(int ArgCount, char **Args)
{
    InitializeOSPlatform();
    
    if(ArgCount == 3)
    {
        haversine_setup Setup = SetUpHaversine(Args[1], Args[2]);
        repetition_test_series TestSeries = AllocateTestSeries(ArrayCount(TestFunctions), 1);
        if(IsValid(Setup) && IsValid(TestSeries))
        {
            f64 ReferenceSum = Setup.SumAnswer;
            
            SetRowLabelLabel(&TestSeries, "Test");
            SetRowLabel(&TestSeries, "Haversine");
            for(u32 TestFunctionIndex = 0; TestFunctionIndex < ArrayCount(TestFunctions); ++TestFunctionIndex)
            {
                test_function Function = TestFunctions[TestFunctionIndex];
                
                SetColumnLabel(&TestSeries, "%s", Function.Name);
                
                repetition_tester Tester = {};
                NewTestWave(&TestSeries, &Tester, Setup.ParsedByteCount, GetCPUTimerFreq());
                
                u64 IndividualErrorCount = Function.Verify(Setup);
                u64 SumErrorCount = {};
                
                while(IsTesting(&TestSeries, &Tester))
                {
                    BeginTime(&Tester);
                    f64 Check = Function.Compute(Setup);
                    CountBytes(&Tester, Setup.ParsedByteCount);
                    EndTime(&Tester);

                    SumErrorCount += !ApproxAreEqual(Check, ReferenceSum);
                }
                
                if(SumErrorCount || IndividualErrorCount)
                {
                    fprintf(stderr, "WARNING: %llu haversines mismatched, %llu sum mismatches\n",
                            IndividualErrorCount, SumErrorCount);
                }
            }
            
            PrintCSVForValue(&TestSeries, StatValue_GBPerSecond, stdout);
        }
        else
        {
            fprintf(stderr, "ERROR: Test data size must be non-zero\n");
        }
        
        FreeHaversine(&Setup);
        FreeTestSeries(&TestSeries);
    }
    else
    {
        fprintf(stderr, "Usage: %s [existing filename]\n", Args[0]);
    }
		
    return 0;
}
