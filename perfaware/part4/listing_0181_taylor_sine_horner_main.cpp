/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 181
   ======================================================================== */

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

#include "listing_0175_math_check.cpp"

inline f64 Factorial(u32 X)
{
    f64 Result = (f64)X;
    while(X > 1)
    {
        Result *= --X;
    }
    
    return Result;
}

inline f64 TaylorSineCoefficient(u32 Power)
{
    f64 Sign = (((Power - 1)/2) % 2) ? -1.0 : 1.0;
    f64 Result = (Sign / Factorial(Power));

    return Result;
}

inline f64 TaylorSine(u32 MaxPower, f64 X)
{
    f64 Result = 0;
    
    f64 X2 = X*X;
    f64 XPow = X;
    for(u32 Power = 1; Power <= MaxPower; Power += 2)
    {
        Result += XPow * TaylorSineCoefficient(Power);
        XPow *= X2;
    }
    
    return Result;
}

inline f64 TaylorSineHorner(u32 MaxPower, f64 X)
{
    f64 Result = 0;

    f64 X2 = X*X;
    for(u32 InvPower = 1; InvPower <= MaxPower; InvPower += 2)
    {
        u32 Power = MaxPower - (InvPower - 1);
        Result = Result*X2 + TaylorSineCoefficient(Power);
    }
    Result *= X;
    
    return Result;
}

int main(void)
{
    math_tester Tester = {};
    
    for(u32 Power = 1; Power <= 31; Power += 2)
    {
        while(PrecisionTest(&Tester, 0, Pi64/2))
        {
            f64 RefOutput = sin(Tester.InputValue);
            TestResult(&Tester, RefOutput, TaylorSine(Power, Tester.InputValue), "TaylorSine%u", Power);
            TestResult(&Tester, RefOutput, TaylorSineHorner(Power, Tester.InputValue), "TaylorSineHorner%u", Power);
        }
    }
    
    PrintResults(&Tester);
    
    return 0;
}
