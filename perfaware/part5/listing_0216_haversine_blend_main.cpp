/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 216
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

struct avx_values
{
    __m256d ALC0;
    __m256d ALC3;
    __m256d RangeA;
    __m256d RangeR;
};

static __m256d Deg180 = _mm256_set1_pd(180.0);
static __m256d Half = _mm256_set1_pd(0.5);
static __m256d NegPi = _mm256_set1_pd(-3.14159265358979323846);
static __m256d HalfPi = _mm256_set1_pd(1.57079632679489661923);
static __m256d Zero = _mm256_setzero_pd();
static __m256d One = _mm256_set1_pd(1.0);

static __m256d AANOSelect(__m256d FalseCase, __m256d TrueCase, __m256d Selector)
{
    FalseCase = _mm256_andnot_pd(Selector, FalseCase);
    TrueCase = _mm256_and_pd(Selector, TrueCase);
    
    __m256d Result = _mm256_or_pd(FalseCase, TrueCase);
    
    return Result;
}

static avx_values ComputeAANOValues(__m256d AbsDLat, __m256d AbsDLon, __m256d a, __m256d R)
{
    avx_values Result = {};

    __m256d LatLess180 = _mm256_cmp_pd(AbsDLat, Deg180, _CMP_LT_OS);
    __m256d LonLess180 = _mm256_cmp_pd(AbsDLon, Deg180, _CMP_LT_OS);
#if 1
    Result.ALC0 = AANOSelect(NegPi, Zero, LatLess180);
    Result.ALC3 = AANOSelect(NegPi, Zero, LonLess180);
#else
    Result.ALC0 = _mm256_andnot_pd(LatLess180, NegPi);
    Result.ALC3 = _mm256_andnot_pd(LonLess180, NegPi);
#endif
    
    __m256d NeedsTransform = _mm256_cmp_pd(Half, a, _CMP_LT_OS);
    Result.RangeA = AANOSelect(a, _mm256_sub_pd(One, a), NeedsTransform);
    Result.RangeR = AANOSelect(R, _mm256_sub_pd(HalfPi, R), NeedsTransform);
    
    return Result;
}

static avx_values ComputeBlendVValues(__m256d AbsDLat, __m256d AbsDLon, __m256d a, __m256d R)
{
    avx_values Result = {};
    
    __m256d LatLess180 = _mm256_cmp_pd(AbsDLat, Deg180, _CMP_LT_OS);
    __m256d LonLess180 = _mm256_cmp_pd(AbsDLon, Deg180, _CMP_LT_OS);
#if 1
    Result.ALC0 = _mm256_blendv_pd(NegPi, Zero, LatLess180);
    Result.ALC3 = _mm256_blendv_pd(NegPi, Zero, LonLess180);
#else
    Result.ALC0 = _mm256_andnot_pd(LatLess180, NegPi);
    Result.ALC3 = _mm256_andnot_pd(LonLess180, NegPi);
#endif
    
    __m256d NeedsTransform = _mm256_cmp_pd(Half, a, _CMP_LT_OS);
    Result.RangeA = _mm256_blendv_pd(a, _mm256_sub_pd(One, a), NeedsTransform);
    Result.RangeR = _mm256_blendv_pd(R, _mm256_sub_pd(HalfPi, R), NeedsTransform);
    
    return Result;
}

static void PrintValue(char const *Label, f64 ScalarValue, f64 AVXValue)
{
    printf("%16s: %9.4f %9.4f %s\n",
           Label, ScalarValue, AVXValue,
           (ScalarValue == AVXValue) ? "Match" : "ERROR");
}

typedef avx_values avx_function(__m256d AbsDLat, __m256d AbsDLon, __m256d a, __m256d R);
static void TestAVXFunction(char const *Label, avx_function *AVXFunc)
{
    int const Width = 4;
    
    f64 AbsDLat[Width] = {134.34, 212.34, 305.3, 20.6};
    f64 AbsDLon[Width] = {160.43, 179.49, 180.0, 200.4};
    f64 a[Width] = {0.1232, 0.890, 0.49, 0.5};
    f64 R[Width] = {0.3509, 0.943, 0.7, 0.7071};
    
    avx_values AVXPred = AVXFunc(_mm256_loadu_pd(AbsDLat), _mm256_loadu_pd(AbsDLon),
                                 _mm256_loadu_pd(a), _mm256_loadu_pd(R));
    
    f64 ALC0[Width];
    _mm256_storeu_pd(ALC0, AVXPred.ALC0);

    f64 ALC3[Width];
    _mm256_storeu_pd(ALC3, AVXPred.ALC3);
    
    f64 RangeA[Width];
    _mm256_storeu_pd(RangeA, AVXPred.RangeA);
    
    f64 RangeR[Width];
    _mm256_storeu_pd(RangeR, AVXPred.RangeR);

    printf("%s:\n", Label);
    for(u32 SI = 0; SI < Width; ++SI)
    {
        scalar_values ScalarPred = ComputeScalarValues(AbsDLat[SI], AbsDLon[SI], a[SI], R[SI]);
        
        PrintValue("ALC0", ScalarPred.ALC0, ALC0[SI]);
        PrintValue("ALC3", ScalarPred.ALC3, ALC3[SI]);
        PrintValue("RangeA", ScalarPred.RangeA, RangeA[SI]);
        PrintValue("RangeR", ScalarPred.RangeR, RangeR[SI]);
        printf("\n");
    }
}

int main(void)
{
    TestAVXFunction("ComputeAANOValues", ComputeAANOValues);
    TestAVXFunction("ComputeBlendVValues", ComputeBlendVValues);
    
    return 0;
}

