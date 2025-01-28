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
#include <stdarg.h>
#include <stdlib.h>

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
    CheckHardCodedReference("sin", sin, ArrayCount(RefTableSinX), RefTableSinX);
    CheckHardCodedReference("cos", cos, ArrayCount(RefTableCosX), RefTableCosX);
    CheckHardCodedReference("asin", asin, ArrayCount(RefTableArcSinX), RefTableArcSinX);
    CheckHardCodedReference("sqrt", sqrt, ArrayCount(RefTableSqrtX), RefTableSqrtX);
    
    math_tester Tester = {};
    
    while(PrecisionTest(&Tester, -Pi64, Pi64))
    {
        TestResult(&Tester, sin(Tester.InputValue), SinCE(Tester.InputValue), "SinCE");
    }
    
    while(PrecisionTest(&Tester, -Pi64/2, Pi64/2))
    {
        TestResult(&Tester, cos(Tester.InputValue), CosCE(Tester.InputValue), "CosCE");
    }
    
    while(PrecisionTest(&Tester, 0, 1))
    {
        TestResult(&Tester, asin(Tester.InputValue), ASinCE(Tester.InputValue), "ASinCE");
    }
    
    while(PrecisionTest(&Tester, 0, 1))
    {
        TestResult(&Tester, sqrt(Tester.InputValue), SqrtCE(Tester.InputValue), "SqrtCE");
    }
    
    PrintResults(&Tester);
    
    return 0;
}