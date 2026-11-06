/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 186
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
#include "listing_0184_sine_coefficients.inl"
#include "listing_0185_sine_extc.cpp"

int main(void)
{
    math_tester Tester = {};
    
    for(u32 CCount = 2; CCount < 16; ++CCount)
    {
        while(PrecisionTest(&Tester, 0, Pi64/2))
        {
            f64 RefOutput = sin(Tester.InputValue);
            
            if(CCount <= ArrayCount(SineRadiansC_Taylor))
            {
                TestResult(&Tester, RefOutput, OddPowerPolynomialC(CCount, SineRadiansC_Taylor, Tester.InputValue), "Taylor[%u]", CCount);
            }
            
            if(CCount < ArrayCount(SineRadiansC_MFTWP))
            {
                TestResult(&Tester, RefOutput, OddPowerPolynomialC(CCount, SineRadiansC_MFTWP[CCount], Tester.InputValue), "MFTWP[%u]", CCount);
            }
            
            if(CCount == 9)
            {
                TestResult(&Tester, RefOutput, SineCore_Radians_MFTWP(Tester.InputValue), "SineCore_Radians_MFTWP");
            }
        }
    }
    
    PrintResults(&Tester);
    
    return 0;
}
