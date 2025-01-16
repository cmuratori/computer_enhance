/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 176
   ======================================================================== */

#include <math.h>
#include <stdio.h>
#include <stdint.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0175_math_check.cpp"

/* NOTE(casey): These are our stub functions. We will start filling
   them in with real computation over time.
*/

static f64 SinCE(f64 X)
{
    f64 Result = X;
    return Result;
}

static f64 CosCE(f64 X)
{
    f64 Result = X;
    return Result;
}

static f64 ASinCE(f64 X)
{
    f64 Result = X;
    return Result;
}

static f64 SqrtCE(f64 X)
{
    double Result = X;
    return Result;
}

int main(void)
{
    named_math_func SineFuncs[] = {{"sin",sin}, {"SinCE",SinCE}};
    math_func_array SineFuncArray = {ArrayCount(SineFuncs), SineFuncs};
    
    named_math_func CosineFuncs[] = {{"cos",cos}, {"CosCE",CosCE}};
    math_func_array CosineFuncArray = {ArrayCount(CosineFuncs), CosineFuncs};
    
    named_math_func ArcSineFuncs[] = {{"asin",asin}, {"ASinCE",ASinCE}};
    math_func_array ArcSineFuncArray = {ArrayCount(ArcSineFuncs), ArcSineFuncs};

    named_math_func SquareRootFuncs[] = {{"sqrt",sqrt}, {"SqrtCE",SqrtCE}};
    math_func_array SquareRootFuncArray = {ArrayCount(SquareRootFuncs), SquareRootFuncs};
    
    CheckHardCodedReference("Sine", SineFuncArray, ArrayCount(RefTableSinX), RefTableSinX);
    CheckHardCodedReference("Cosine", CosineFuncArray, ArrayCount(RefTableCosX), RefTableCosX);
    CheckHardCodedReference("ArcSine", ArcSineFuncArray, ArrayCount(RefTableArcSinX), RefTableArcSinX);
    CheckHardCodedReference("SquareRoot", SquareRootFuncArray, ArrayCount(RefTableSqrtX), RefTableSqrtX);
    
    SampleLargestDiff(sin, SineFuncArray, -Pi64, Pi64);
    SampleLargestDiff(cos, CosineFuncArray, -Pi64/2, Pi64/2);
    SampleLargestDiff(asin, ArcSineFuncArray, 0, 1);
    SampleLargestDiff(sqrt, SquareRootFuncArray, 0, 1);
    
    return 0;
}