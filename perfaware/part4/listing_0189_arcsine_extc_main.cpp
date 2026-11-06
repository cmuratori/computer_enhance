/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 189
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
#include "listing_0185_sine_extc.cpp"
#include "listing_0187_arcsine_coefficients.inl"
#include "listing_0188_arcsine_extc.cpp"

int main(void)
{
    math_tester Tester = {};
    
    for(u32 CCount = 2; CCount < ArrayCount(ArcsineRadiansC_Taylor); ++CCount)
    {
        while(PrecisionTest(&Tester, 0, 1.0/sqrt(2.0)))
        {
            f64 RefOutput = asin(Tester.InputValue);
            
            if(CCount <= ArrayCount(ArcsineRadiansC_Taylor))
            {
                TestResult(&Tester, RefOutput, OddPowerPolynomialC(CCount, ArcsineRadiansC_Taylor, Tester.InputValue), "Taylor[%u]", CCount);
            }
            
            if(CCount < ArrayCount(ArcsineRadiansC_MFTWP))
            {
                TestResult(&Tester, RefOutput, OddPowerPolynomialC(CCount, ArcsineRadiansC_MFTWP[CCount], Tester.InputValue), "MFTWP[%u]", CCount);
            }
            
            if(CCount == 19)
            {
                TestResult(&Tester, RefOutput, ArcsineCore_MFTWP(Tester.InputValue), "ArcSineCore_MFTWP");
            }
        }
    }
    
    PrintResults(&Tester);
    
    return 0;
}
