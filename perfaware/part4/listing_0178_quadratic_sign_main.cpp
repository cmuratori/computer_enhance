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
    named_math_func SinFuncs[] =
    {
        {"sin",sin},
        {"SinQ",SinQ},
        {"IntrinSinQ",IntrinSinQ},
    };
    
    math_func_array SinFuncArray = {ArrayCount(SinFuncs), SinFuncs};
    
    printf("--- RANGE: [0, pi] ---\n");
    SampleLargestDiff(sin, SinFuncArray, 0, Pi64);
    
    printf("--- RANGE: [-pi, pi] ---\n");
    SampleLargestDiff(sin, SinFuncArray, -Pi64, Pi64);
    
    return 0;
}