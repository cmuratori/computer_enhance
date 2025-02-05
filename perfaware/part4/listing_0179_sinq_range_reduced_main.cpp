/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 179
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

static f64 SinQHalf(f64 X)
{
    f64 PosX = fabs(X);
    
    f64 PosResult = SinQ(PosX);
    
    f64 Result = (X < 0) ? -PosResult : PosResult;
    
    return Result;
}

static f64 SinQQuarter(f64 X)
{
    f64 HalfPi = Pi64/2;
    f64 PosX = fabs(X);
    f64 QuartX = (PosX > HalfPi) ? (Pi64 - PosX) : PosX;

    f64 QuartX2 = Square(QuartX);
    f64 A = -0.3357488673628103541807525733876701910953780492546723687387637750157263772845455;
    f64 B = 1.164012859946630796034863328523423717191309716948615456152205566227330270901187;
    f64 PosResult = A*QuartX2 + B*QuartX;
    
    f64 Result = (X < 0) ? -PosResult : PosResult;
    
    return Result;
}

static f64 CosQQuarter(f64 X)
{
    f64 Result = SinQQuarter(X + Pi64/2.0);
    return Result;
}

int main(void)
{
    math_tester Tester = {};
    
    while(PrecisionTest(&Tester, -Pi64, Pi64))
    {
        f64 RefOutput = sin(Tester.InputValue);
        TestResult(&Tester, RefOutput, SinQ(Tester.InputValue), "SinQ");
        TestResult(&Tester, RefOutput, SinQHalf(Tester.InputValue), "SinQHalf");
        TestResult(&Tester, RefOutput, SinQQuarter(Tester.InputValue), "SinQQuarter");
    }
    
    while(PrecisionTest(&Tester, -Pi64/2, Pi64/2))
    {
        TestResult(&Tester, cos(Tester.InputValue), CosQQuarter(Tester.InputValue), "CosQQuarter");
    }
    
    PrintResults(&Tester);
    
    return 0;
}