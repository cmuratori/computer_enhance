/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 202
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
#include "listing_0200_dead_code_macros.cpp"

static void FMADepChain(u32 ChainCount, u32 ChainLength)
{
    for(u32 ChainIndex = 0; ChainIndex < ChainCount; ChainIndex += 1)
    {
        f64 X2 = 0;
        f64 M = 0;
        f64 R0 = 0;
        
        PretendToWrite(X2);
        PretendToWrite(M);
        PretendToWrite(R0);
        
        for(u32 LengthIndex = 0; LengthIndex < ChainLength; LengthIndex += 8)
        {
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
            R0 = fma(R0, X2, M);
        }
        
        PretendToRead(R0);
    }
}

int main(void)
{
    InitializeOSPlatform();
    
    repetition_test_series TestSeries = AllocateTestSeries(1, 1024);
    if(IsValid(TestSeries))
    {
        SetRowLabelLabel(&TestSeries, "ChainLength");
        for(u32 ChainLength = 8; ChainLength <= 256; ChainLength += 8)
        {
            u32 RepCount = 1024*1024;
            u32 ChainCount = RepCount / ChainLength;
            
            SetRowLabel(&TestSeries, "%u", ChainLength);
            SetColumnLabel(&TestSeries, "FMADepChain");
            
            repetition_tester Tester = {};
            NewTestWave(&TestSeries, &Tester, RepCount, GetCPUTimerFreq());
            
            while(IsTesting(&TestSeries, &Tester))
            {
                BeginTime(&Tester);
                FMADepChain(ChainCount, ChainLength);
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
