/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 177
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

static f64 SqrtCE(f64 ScalarX)
{
    __m128d X = _mm_set_sd(ScalarX);
    
    __m128d SqrtX = _mm_sqrt_sd(X, X);
    
    f64 Result = _mm_cvtsd_f64(SqrtX);
    return Result;
}

static f64 SqrtFCE(f64 ScalarX)
{
    __m128 X = _mm_set_ss((f32)ScalarX);
    
    __m128 SqrtX = _mm_sqrt_ss(X);
    
    f64 Result = _mm_cvtss_f32(SqrtX);
    return Result;
}

static f64 ApproxSqrtCE(f64 ScalarX)
{
    __m128 X = _mm_set_ss((f32)ScalarX);
    
    __m128 RSqrtX = _mm_rsqrt_ss(X);
    __m128 SqrtX = _mm_rcp_ss(RSqrtX);
    
    f64 Result = _mm_cvtss_f32(SqrtX);
    return Result;
}

int main(void)
{
    named_math_func SquareRootFuncs[] =
    {
        {"sqrt",sqrt},
        {"SqrtCE",SqrtCE},
        {"SqrtFCE",SqrtFCE},
        {"ApproxSqrtCE",ApproxSqrtCE}
    };
    
    math_func_array SquareRootFuncArray = {ArrayCount(SquareRootFuncs), SquareRootFuncs};
    SampleLargestDiff(sqrt, SquareRootFuncArray, 0, 1);
    
    return 0;
}