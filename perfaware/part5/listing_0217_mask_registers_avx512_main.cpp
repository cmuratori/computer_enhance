/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 217
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef int32_t b32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

struct scalar_values
{
    b32 LatLess180;
    b32 LonLess180;
    b32 NeedsTransform;
    
    f64 ALC0;
    f64 ALC3;
    f64 RangeA;
    f64 RangeR;
};

static scalar_values ComputeScalarValues(f64 AbsDLat, f64 AbsDLon, f64 a, f64 R)
{
    f64 Deg180 = 180.0;
    f64 Half = 0.5;
    f64 NegPi = -3.14159265358979323846;
    f64 HalfPi = 1.57079632679489661923;
    
    scalar_values Result = {};
    Result.LatLess180 = (AbsDLat < Deg180);
    Result.LonLess180 = (AbsDLon < Deg180);
    Result.NeedsTransform = (Half < a);
    
    Result.ALC0 = Result.LatLess180 ? 0 : NegPi;
    Result.ALC3 = Result.LonLess180 ? 0 : NegPi;
    Result.RangeA = Result.NeedsTransform ? (1.0 - a) : a;
    Result.RangeR = Result.NeedsTransform ? (HalfPi - R) : R;
    
    return Result;
}

struct avx512_values
{
    __m512d ALC0;
    __m512d ALC3;
    __m512d RangeA;
    __m512d RangeR;
};

static avx512_values ComputeAVX512Values(__m512d AbsDLat, __m512d AbsDLon, __m512d a, __m512d R)
{
    __m512d Deg180 = _mm512_set1_pd(180.0);
    __m512d Half = _mm512_set1_pd(0.5);
    __m512d NegPi = _mm512_set1_pd(-3.14159265358979323846);
    __m512d HalfPi = _mm512_set1_pd(1.57079632679489661923);
    __m512d Zero = _mm512_setzero_pd();
    __m512d One = _mm512_set1_pd(1.0);
    
    avx512_values Result = {};
    
    __mmask8 LatLess180 = _mm512_cmp_pd_mask(AbsDLat, Deg180, _CMP_LT_OS);
    Result.ALC0 = _mm512_mask_mov_pd(NegPi, LatLess180, Zero);
    
    __mmask8 LonLess180 = _mm512_cmp_pd_mask(AbsDLon, Deg180, _CMP_LT_OS);
    Result.ALC3 = _mm512_mask_mov_pd(NegPi, LonLess180, Zero);
    
    __mmask8 NeedsTransform = _mm512_cmp_pd_mask(Half, a, _CMP_LT_OS);
    Result.RangeA = _mm512_mask_sub_pd(a, NeedsTransform, One, a);
    Result.RangeR = _mm512_mask_sub_pd(R, NeedsTransform, HalfPi, R);
    
    return Result;
}

static void PrintValue(char const *Label, f64 ScalarValue, f64 AVXValue)
{
    printf("%16s: %9.4f %9.4f %s\n",
           Label, ScalarValue, AVXValue,
           (ScalarValue == AVXValue) ? "Match" : "ERROR");
}

int main(void)
{
    int const Width = 8;
    
    f64 AbsDLat[Width] = {134.34, 212.34, 305.3, 20.6, 59.99, 290.0, 111.111, 40.809};
    f64 AbsDLon[Width] = {160.43, 179.49, 180.0, 200.4, 288.8, 9.3, 11.7593, 66.40};
    f64 a[Width] = {0.1232, 0.890, 0.49, 0.5, 0.001, 0.9910, 0.78412, 0.6};
    f64 R[Width] = {0.3509, 0.943, 0.7, 0.7071, 0.592, 0.971, 0.885505, 0.7745};
    
    avx512_values AVXPred = ComputeAVX512Values(_mm512_loadu_pd(AbsDLat), _mm512_loadu_pd(AbsDLon),
                                                _mm512_loadu_pd(a), _mm512_loadu_pd(R));
    
    f64 ALC0[Width];
    _mm512_storeu_pd(ALC0, AVXPred.ALC0);

    f64 ALC3[Width];
    _mm512_storeu_pd(ALC3, AVXPred.ALC3);
    
    f64 RangeA[Width];
    _mm512_storeu_pd(RangeA, AVXPred.RangeA);
    
    f64 RangeR[Width];
    _mm512_storeu_pd(RangeR, AVXPred.RangeR);

    for(u32 SI = 0; SI < Width; ++SI)
    {
        scalar_values ScalarPred = ComputeScalarValues(AbsDLat[SI], AbsDLon[SI], a[SI], R[SI]);
        
        PrintValue("ALC0", ScalarPred.ALC0, ALC0[SI]);
        PrintValue("ALC3", ScalarPred.ALC3, ALC3[SI]);
        PrintValue("RangeA", ScalarPred.RangeA, RangeA[SI]);
        PrintValue("RangeR", ScalarPred.RangeR, RangeR[SI]);
        printf("\n");
    }
    
    return 0;
}

