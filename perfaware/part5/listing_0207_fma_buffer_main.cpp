/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 207
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
#include "listing_0203_csv_repetition_tester.cpp"
#include "listing_0200_dead_code_macros.cpp"

extern "C" void FMADepChainSpacedASM(u32 ChainCount, u32 ChainLength);
#pragma comment (lib, "listing_0206_fma_buffer.lib")

inline void FMADepChain8(u32 ChainCount, u32 ChainLength)
{
    for(u32 ChainIndex = 0; ChainIndex < ChainCount; ChainIndex += 8)
    {
        f64 X2 = 1.001;
        f64 M = 0.01;
        f64 R0 = 1.01;
        f64 R1 = R0;
        f64 R2 = R0;
        f64 R3 = R0;
        f64 R4 = R0;
        f64 R5 = R0;
        f64 R6 = R0;
        f64 R7 = R0;
        
        PretendToWrite(X2);
        PretendToWrite(M);
        PretendToWrite(R0);
        PretendToWrite(R1);
        PretendToWrite(R2);
        PretendToWrite(R3);
        PretendToWrite(R4);
        PretendToWrite(R5);
        PretendToWrite(R6);
        PretendToWrite(R7);
        
        for(u32 LengthIndex = 0; LengthIndex < ChainLength; LengthIndex += 1)
        {
            R0 = fma(R0, X2, M);
            R1 = fma(R1, X2, M);
            R2 = fma(R2, X2, M);
            R3 = fma(R3, X2, M);
            R4 = fma(R4, X2, M);
            R5 = fma(R5, X2, M);
            R6 = fma(R6, X2, M);
            R7 = fma(R7, X2, M);
        }
        
        PretendToRead(R0);
        PretendToRead(R1);
        PretendToRead(R2);
        PretendToRead(R3);
        PretendToRead(R4);
        PretendToRead(R5);
        PretendToRead(R6);
        PretendToRead(R7);
    }
}

inline void FMADepChainSpaced(u32 ChainCount, u32 ChainLength)
{
#define SPACING_COUNT 16
    f64 InitialValueBuffer[SPACING_COUNT] = {};
    f64 TempValueBuffer[SPACING_COUNT] = {};
    
    for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
    {
        InitialValueBuffer[PretendIndex] = 1.01;
    }
    
    for(u32 ChainIndex = 0; ChainIndex < ChainCount; ChainIndex += SPACING_COUNT)
    {
        f64 X2 = 1.001;
        f64 M = 0.01;
        
        PretendToWrite(X2);
        PretendToWrite(M);
        
        f64 *Source = InitialValueBuffer;
        f64 *Dest = TempValueBuffer;
        
        for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
        {
            PretendToWrite(Source[PretendIndex]);
        }
         
        for(u32 LengthIndex = 0; LengthIndex < ChainLength; LengthIndex += 8)
        {
            for(u32 SpacingIndex = 0; SpacingIndex < SPACING_COUNT; ++SpacingIndex)
            {
                f64 R0 = Source[SpacingIndex];
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                R0 = fma(R0, X2, M);
                Dest[SpacingIndex] = R0;
            }
            
            Source = Dest;
        }
        
        for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
        {
            PretendToRead(Dest[PretendIndex]);
        }
    }
}

inline void FMADepChainSpacedMM(u32 ChainCount, u32 ChainLength)
{
#define SPACING_COUNT 16
    __m256d InitialValueBuffer[SPACING_COUNT] = {};
    __m256d TempValueBuffer[SPACING_COUNT] = {};
    
    for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
    {
        InitialValueBuffer[PretendIndex] = _mm256_set1_pd(1.01);
    }
    
    for(u32 ChainIndex = 0; ChainIndex < ChainCount; ChainIndex += SPACING_COUNT)
    {
        __m256d X2 = _mm256_set1_pd(1.001);
        __m256d M = _mm256_set1_pd(0.01);
        
        PretendToWrite(X2);
        PretendToWrite(M);
        
        __m256d *Source = InitialValueBuffer;
        __m256d *Dest = TempValueBuffer;
        
        for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
        {
            PretendToWrite(Source[PretendIndex]);
        }
         
        for(u32 LengthIndex = 0; LengthIndex < ChainLength; LengthIndex += 8)
        {
            for(u32 SpacingIndex = 0; SpacingIndex < SPACING_COUNT; ++SpacingIndex)
            {
                __m256d R0 = Source[SpacingIndex];
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                R0 = _mm256_fmadd_pd(R0, X2, M);
                Dest[SpacingIndex] = R0;
            }
            
            Source = Dest;
        }
        
        for(u32 PretendIndex = 0; PretendIndex < SPACING_COUNT; ++PretendIndex)
        {
            PretendToRead(Dest[PretendIndex]);
        }
    }
}

typedef void fma_function_type(u32 ChainCount, u32 ChainLength);

struct fma_function
{
    char const *Name;
    fma_function_type *Func;
};
static fma_function FMAFunctions[] =
{
    {"FMADepChain8", FMADepChain8},
    {"FMADepChainSpacedASM", FMADepChainSpacedASM},
    {"FMADepChainSpaced", FMADepChainSpaced},
    {"FMADepChainSpacedMM", FMADepChainSpacedMM},
};

int main(void)
{
    InitializeOSPlatform();
    
    repetition_test_series TestSeries = AllocateTestSeries(ArrayCount(FMAFunctions), 1024);
    if(IsValid(TestSeries))
    {
        SetRowLabelLabel(&TestSeries, "ChainLength");
        for(u32 ChainLength = 8; ChainLength <= 256; ChainLength += 8)
        {
            u32 RepCount = 1024*1024;
            u32 ChainCount = RepCount / ChainLength;
            
            // NOTE(casey): Round down to an even 64 so any test construction would
            // not have to handle a residual
            ChainCount &= ~0x3f;

            // NOTE(casey): Remultiply here just to make sure we count
            // our statistics fairly for divides with remainders
            RepCount = ChainCount * ChainLength;
            
            SetRowLabel(&TestSeries, "%u", ChainLength);
            for(u32 FMAFunctionIndex = 0; FMAFunctionIndex < ArrayCount(FMAFunctions); ++FMAFunctionIndex)
            {
                fma_function Function = FMAFunctions[FMAFunctionIndex];
                
                SetColumnLabel(&TestSeries, "%s", Function.Name);
                
                repetition_tester Tester = {};
                NewTestWave(&TestSeries, &Tester, RepValue_OpCount, RepCount, GetCPUTimerFreq());
                
                while(IsTesting(&TestSeries, &Tester))
                {
                    BeginTime(&Tester);
                    Function.Func(ChainCount, ChainLength);
                    CountOps(&Tester, RepCount);
                    EndTime(&Tester);
                }
            }
        }
        
        PrintCSVForValue(&TestSeries, StatValue_GOpPerSecond, stdout);
    }
    
    FreeTestSeries(&TestSeries);
    
    (void)&ReadEntireFile;
    (void)&SetRowLabelLabel;
    (void)&SetRowLabel;
    
    return 0;
}
