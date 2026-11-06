/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 212
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef int32_t b32;
typedef uint32_t u32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

static __m256d FAbs4x(__m256d Value)
{
    __m256d SignBit = _mm256_set1_pd(-0.0);
    __m256d Result = _mm256_andnot_pd(SignBit, Value);
    return Result;
}

int main(void)
{
    for(u32 Case = 0; Case < 0xf; ++Case)
    {
        f64 Input[4] =
        {
            (Case & 1) ? 1.1 : -1.1,
            (Case & 2) ? 2.2 : -2.2,
            (Case & 4) ? 3.3 : -3.3,
            (Case & 8) ? 4.4 : -4.4,
        };
        
        f64 CLib[4];
        for(u32 VI = 0; VI < ArrayCount(Input); ++VI)
        {
            CLib[VI] = fabs(Input[VI]);
        }
        
        __m256d BinarySIMD = FAbs4x(_mm256_loadu_pd(Input));
        
        f64 Binary[4];
        _mm256_storeu_pd(Binary, BinarySIMD);
        
        for(u32 VI = 0; VI < ArrayCount(Input); ++VI)
        {
            b32 Match = (Binary[VI] == CLib[VI]);
            printf("%s: %f -> %f\n", Match ? "Match" : "ERROR", Input[VI], Binary[VI]);
        }
    }
    
    return 0;
}