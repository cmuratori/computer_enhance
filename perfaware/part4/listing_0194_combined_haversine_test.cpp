/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 194
   ======================================================================== */

#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <intrin.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0175_math_check.cpp"
#include "listing_0125_buffer.cpp"
#include "listing_0169_os_platform.cpp"
#include "listing_0164_csv_repetition_tester.cpp"
#include "listing_0172_reference_haversine.cpp"
#include "listing_0188_arcsine_extc.cpp"
#include "listing_0190_math_replacement.cpp"
#include "listing_0192_haversine_replacement.cpp"

struct test_function
{
    char const *Name;
    haversine_compute_func *Compute;
};

inline void CombinedHaversineTest(int ArgCount, char **Args, u32 TestFunctionCount, test_function *TestFunctions)
{
    InitializeOSPlatform();

    if(ArgCount == 3)
    {
        haversine_setup Setup = SetUpHaversine(Args[1], Args[2]);
        repetition_test_series TestSeries = AllocateTestSeries(TestFunctionCount, 1);
        if(IsValid(Setup) && IsValid(TestSeries))
        {
            f64 ReferenceSum = Setup.SumAnswer;
            
            SetRowLabelLabel(&TestSeries, "Test");
            SetRowLabel(&TestSeries, "Haversine");
            for(u32 TestFunctionIndex = 0; TestFunctionIndex < TestFunctionCount; ++TestFunctionIndex)
            {
                test_function Function = TestFunctions[TestFunctionIndex];
                
                SetColumnLabel(&TestSeries, "%s", Function.Name);
                
                repetition_tester Tester = {};
                NewTestWave(&TestSeries, &Tester, Setup.ParsedByteCount, GetCPUTimerFreq());
                
                u64 SumErrorCount = {};
                f64 TestSum = {};

                while(IsTesting(&TestSeries, &Tester))
                {
                    BeginTime(&Tester);
                    TestSum = Function.Compute(Setup);
                    CountBytes(&Tester, Setup.ParsedByteCount);
                    EndTime(&Tester);
                    
                    SumErrorCount += !ApproxAreEqual(TestSum, ReferenceSum);
                }
                
                printf("             ________________                  ________________\n");
                fprintf(stdout, "Sum: %+32.24f (%+32.24f)\n", TestSum, TestSum - ReferenceSum);
                fprintf(stdout, "\n");
                
                if(SumErrorCount)
                {
                    fprintf(stderr, "WARNING: %llu sum mismatches\n", SumErrorCount);
                }
            }
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
    
    (void)&ReferenceVerifyHaversine;
}
