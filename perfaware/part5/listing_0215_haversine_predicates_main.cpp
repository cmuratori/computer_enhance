/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 215
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

struct scalar_predicates
{
    b32 LatLess180;
    b32 LonLess180;
    b32 NeedsTransform;
};

static scalar_predicates ComputeScalarPredicates(f64 AbsDLat, f64 AbsDLon, f64 a)
{
    f64 Deg180 = 180.0;
    f64 Half = 0.5;
    
    scalar_predicates Result = {};
    Result.LatLess180 = (AbsDLat < Deg180);
    Result.LonLess180 = (AbsDLon < Deg180);
    Result.NeedsTransform = (Half < a);
    
    return Result;
}

struct avx_predicates
{
    __m256d LatLess180;
    __m256d LonLess180;
    __m256d NeedsTransform;
};

static avx_predicates ComputeAVXPredicates(__m256d AbsDLat, __m256d AbsDLon, __m256d a)
{
    __m256d Deg180 = _mm256_set1_pd(180.0);
    __m256d Half = _mm256_set1_pd(0.5);
    
    avx_predicates Result = {};
    Result.LatLess180 = _mm256_cmp_pd(AbsDLat, Deg180, _CMP_LT_OS);
    Result.LonLess180 = _mm256_cmp_pd(AbsDLon, Deg180, _CMP_LT_OS);
    Result.NeedsTransform = _mm256_cmp_pd(Half, a, _CMP_LT_OS);
    
    return Result;
}

static void PrintPred(char const *Label, f64 InputValue, b32 ScalarAnswer, u64 AVXAnswer)
{
    printf("%16s: %9.4f %s ", Label, InputValue, ScalarAnswer ? "T" : "f");
    if(AVXAnswer == -1)
    {
        printf("T ");
    }
    else if(AVXAnswer == 0)
    {
        printf("f ");
    }
    else
    {
        printf("ERROR ");
    }

    printf("%016llx\n", AVXAnswer);
}

int main(void)
{
    int const Width = 4;
    
    f64 AbsDLat[Width] = {134.34, 212.34, 305.3, 20.6};
    f64 AbsDLon[Width] = {160.43, 179.49, 180.0, 200.4};
    f64 a[Width] = {0.1232, 0.890, 0.49, 0.5};
    
    avx_predicates AVXPred = ComputeAVXPredicates(_mm256_loadu_pd(AbsDLat), _mm256_loadu_pd(AbsDLon), _mm256_loadu_pd(a));
    
    u64 AVXLatLess180[Width];
    _mm256_storeu_pd((f64 *)AVXLatLess180, AVXPred.LatLess180);
    
    u64 AVXLonLess180[Width];
    _mm256_storeu_pd((f64 *)AVXLonLess180, AVXPred.LonLess180);
    
    u64 AVXNeedsTransform[Width];
    _mm256_storeu_pd((f64 *)AVXNeedsTransform, AVXPred.NeedsTransform);
    
    for(u32 SI = 0; SI < Width; ++SI)
    {
        scalar_predicates ScalarPred = ComputeScalarPredicates(AbsDLat[SI], AbsDLon[SI], a[SI]);
    
        PrintPred("LatLess180", AbsDLat[SI], ScalarPred.LatLess180, AVXLatLess180[SI]);
        PrintPred("LonLess180", AbsDLon[SI], ScalarPred.LonLess180, AVXLonLess180[SI]);
        PrintPred("NeedsTransform", a[SI], ScalarPred.NeedsTransform, AVXNeedsTransform[SI]);
    }
    
    return 0;
}
