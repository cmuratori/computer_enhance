/* ========================================================================

   (C) Copyright 2025 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 198
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <math.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0163_os_platform.cpp"
#include "listing_0164_csv_repetition_tester.cpp"

inline f64 SqrtCE(f64 ScalarX)
{
    __m128d X = _mm_set_sd(ScalarX);
    
    __m128d SqrtX = _mm_sqrt_sd(X, X);
    
    f64 Result = _mm_cvtsd_f64(SqrtX);
    return Result;
}

inline f64 ArcsineCoreFromSquared(f64 X2)
{
    f64 X = SqrtCE(X2);

    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    f64 R = 0x1.dfc53682725cap-1;
    R = fma(R, X2, -0x1.bec6daf74ed61p1);
    R = fma(R, X2, 0x1.8bf4dadaf548cp2);
    R = fma(R, X2, -0x1.b06f523e74f33p2);
    R = fma(R, X2, 0x1.4537ddde2d76dp2);
    R = fma(R, X2, -0x1.6067d334b4792p1);
    R = fma(R, X2, 0x1.1fb54da575b22p0);
    R = fma(R, X2, -0x1.57380bcd2890ep-2);
    R = fma(R, X2, 0x1.69b370aad086ep-4);
    R = fma(R, X2, -0x1.21438ccc95d62p-8);
    R = fma(R, X2, 0x1.b8a33b8e380efp-7);
    R = fma(R, X2, 0x1.c37061f4e5f55p-7);
    R = fma(R, X2, 0x1.1c875d6c5323dp-6);
    R = fma(R, X2, 0x1.6e88ce94d1149p-6);
    R = fma(R, X2, 0x1.f1c73443a02f5p-6);
    R = fma(R, X2, 0x1.6db6db3184756p-5);
    R = fma(R, X2, 0x1.3333333380df2p-4);
    R = fma(R, X2, 0x1.555555555531ep-3);
    R = fma(R, X2, 0x1p0);
    R *= X;
    
    return R;
}

static void RecommendedWay(u32 RepCount)
{
    for(u32 Rep = 0; Rep < RepCount; ++Rep)
    {
        f64 Value = 0.5;
        
#if __clang__
        asm volatile ("" : : "g"(&Value) : "memory");
#endif
        f64 Result = ArcsineCoreFromSquared(Value);
#if __clang__
        asm volatile ("" : : "g"(&Result) : "memory");
#endif
        
        (void)Result;
    }
}

static void OurWay(u32 RepCount)
{
    for(u32 Rep = 0; Rep < RepCount; ++Rep)
    {
        f64 Value = 0.5;
        
#ifdef __clang__
        asm volatile ("" : "=x"(Value));
#endif
        f64 Result = ArcsineCoreFromSquared(Value);
#ifdef __clang__
        asm volatile ("" : : "x"(Result));
#endif
        
        (void)Result;
    }
}

typedef void loop_function(u32 RepCount);

struct test_function
{
    char const *Name;
    loop_function *Func;
};
static test_function TestFunctions[] =
{
    {"RecommendedWay", RecommendedWay},
    {"OurWay", OurWay},
};

int main(void)
{
    InitializeOSPlatform();
    
    repetition_test_series TestSeries = AllocateTestSeries(ArrayCount(TestFunctions), 1024);
    if(IsValid(TestSeries))
    {
        u32 RepCount = 100000;
        
        for(u32 TestFunctionIndex = 0; TestFunctionIndex < ArrayCount(TestFunctions); ++TestFunctionIndex)
        {
            test_function Function = TestFunctions[TestFunctionIndex];
            
            SetColumnLabel(&TestSeries, "%s", Function.Name);
            
            repetition_tester Tester = {};
            NewTestWave(&TestSeries, &Tester, RepCount, GetCPUTimerFreq());
            
            while(IsTesting(&TestSeries, &Tester))
            {
                BeginTime(&Tester);
                Function.Func(RepCount);
                CountBytes(&Tester, RepCount);
                EndTime(&Tester);
            }
        }
        
        PrintCSVForValue(&TestSeries, StatValue_GBPerSecond, stdout);
    }
    
    FreeTestSeries(&TestSeries);
		
    (void)&ReadEntireFile;
    (void)&SetRowLabelLabel;
    (void)&SetRowLabel;
    
    return 0;
}
