/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 178
   ======================================================================== */

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

static f64 Square(f64 X)
{
    f64 Result = X*X;
    return Result;
}

static f64 SinQ(f64 X)
{
    f64 X2 = Square(X);
    f64 A = -4.0 / Square(Pi64);
    f64 B = 4.0 / Pi64;
    
    f64 Result = A*X2 + B*X;
    
    return Result;
}

static f64 IntrinSinQ(f64 ScalarX)
{
    /* NOTE(casey): This is a direct translation of SinQ into intrinsics.
       It is not how we will actually organize these sorts of
       computations for performance. We will get to that later, once we
       are benchmarking! */
       
    __m128d X = _mm_set_sd(ScalarX);
    
    __m128d X2 = _mm_mul_sd(X, X);
    __m128d A = _mm_set_sd(-4.0 / Square(Pi64));
    __m128d B = _mm_set_sd(4.0 / Pi64);
    
    __m128d Approx = _mm_add_sd(_mm_mul_sd(A, X2), _mm_mul_sd(B, X));
    
    f64 Result = _mm_cvtsd_f64(Approx);
    return Result;
}

int main(void)
{
    f64 LowerBounds[] = {0, -Pi64};
    for(u32 BoundIndex = 0; BoundIndex < ArrayCount(LowerBounds); ++BoundIndex)
    {
        math_tester Tester = {};
        
        f64 LowerBound = LowerBounds[BoundIndex];
        printf("RANGE: [%+.24f, %+.24f]\n", LowerBound, Pi64);
        
        while(PrecisionTest(&Tester, LowerBound, Pi64))
        {
            f64 RefOutput = sin(Tester.InputValue);
            TestResult(&Tester, RefOutput, SinQ(Tester.InputValue), "SinQ");
            TestResult(&Tester, RefOutput, IntrinSinQ(Tester.InputValue), "IntrinSinQ");
        }
        
        PrintResults(&Tester);
        printf("\n");
    }
    
    return 0;
}