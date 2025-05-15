/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 193
   ======================================================================== */

#define _CRT_SECURE_NO_WARNINGS
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <intrin.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0169_os_platform.cpp"
#include "listing_0175_math_check.cpp"
#include "listing_0172_reference_haversine.cpp"
#include "listing_0190_math_replacement.cpp"
#include "listing_0192_haversine_replacement.cpp"

int main(int ArgCount, char **Args)
{
    if(ArgCount == 3)
    {
        haversine_setup Setup = SetUpHaversine(Args[1], Args[2]);
        if(IsValid(Setup))
        {
            math_tester Tester = {};

            while(PrecisionTest(&Tester, 0, (f64)Setup.PairCount - 1, (u32)Setup.PairCount))
            {
                haversine_pair Pair = Setup.Pairs[Tester.StepIndex];
                f64 RefOutput = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, QUESTIONABLE_EARTH_RADIUS);
                f64 TestOutput = ReplacementHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, QUESTIONABLE_EARTH_RADIUS);
                TestResult(&Tester, RefOutput, TestOutput, "ReplacementHaversine");
            }
            
            PrintResults(&Tester);

            f64 RefSum = ReferenceSumHaversine(Setup);
            f64 TestSum = ReplacementSumHaversine(Setup);
            
            fprintf(stdout, "\n");
            fprintf(stdout, " RefSum: %+.24f (%+.24f)\n", RefSum, RefSum - Setup.SumAnswer);
            fprintf(stdout, "TestSum: %+.24f (%+.24f)\n", TestSum, TestSum - Setup.SumAnswer);
        }
        
        FreeHaversine(&Setup);
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine JSON file] [haversine answer file]\n", Args[0]);
    }
    
    (void)&ReadOSPageFaultCount;
    (void)&InitializeOSPlatform;
    (void)&ReferenceSumHaversine;
    (void)&ReferenceVerifyHaversine;
    
    return 0;
}
