/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 214
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

struct simd_cmp
{
    u32 Value;
    
    b32 Unordered;
    b32 Signaling;
    
    char const *Operator;
    char const *Mnemonic;
};

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

static simd_cmp CMPImmediates[32] =
{
#define SIMD_CMP(Operator, Unordered, Signaling, Value, Mnemonic) \
    {Value, Unordered, Signaling, #Operator, #Mnemonic},
#include "listing_0213_simd_compare.inl"
};

struct predicate_result
{
    __m256d Res[ArrayCount(CMPImmediates)];
    __m256d A, B;
};

static predicate_result CompareAllWays(__m256d A, __m256d B)
{
    predicate_result Result =
    {
        {
            _mm256_cmp_pd(A, B, 0),
            _mm256_cmp_pd(A, B, 1),
            _mm256_cmp_pd(A, B, 2),
            _mm256_cmp_pd(A, B, 3),
            _mm256_cmp_pd(A, B, 4),
            _mm256_cmp_pd(A, B, 5),
            _mm256_cmp_pd(A, B, 6),
            _mm256_cmp_pd(A, B, 7),
            _mm256_cmp_pd(A, B, 8),
            _mm256_cmp_pd(A, B, 9),
            _mm256_cmp_pd(A, B, 10),
            _mm256_cmp_pd(A, B, 11),
            _mm256_cmp_pd(A, B, 12),
            _mm256_cmp_pd(A, B, 13),
            _mm256_cmp_pd(A, B, 14),
            _mm256_cmp_pd(A, B, 15),
            _mm256_cmp_pd(A, B, 16),
            _mm256_cmp_pd(A, B, 17),
            _mm256_cmp_pd(A, B, 18),
            _mm256_cmp_pd(A, B, 19),
            _mm256_cmp_pd(A, B, 20),
            _mm256_cmp_pd(A, B, 21),
            _mm256_cmp_pd(A, B, 22),
            _mm256_cmp_pd(A, B, 23),
            _mm256_cmp_pd(A, B, 24),
            _mm256_cmp_pd(A, B, 25),
            _mm256_cmp_pd(A, B, 26),
            _mm256_cmp_pd(A, B, 27),
            _mm256_cmp_pd(A, B, 28),
            _mm256_cmp_pd(A, B, 29),
            _mm256_cmp_pd(A, B, 30),
            _mm256_cmp_pd(A, B, 31),
        },
        
        A, B,
    };
    
    return Result;
}

static predicate_result InfTest()
{
    double NumeratorA[] = {1.0, 1.0, 1.0, 1.0};
    double NumeratorB[] = {2.0, 2.0, 2.0, 2.0};
    
    double DenominatorA[] = {1.0, 0.0, 1.0, 0.0};
    double DenominatorB[] = {1.0, 1.0, 0.0, 0.0};
    
    __m256d NuA = _mm256_loadu_pd(NumeratorA);
    __m256d NuB = _mm256_loadu_pd(NumeratorB);
    
    __m256d DeA = _mm256_loadu_pd(DenominatorA);
    __m256d DeB = _mm256_loadu_pd(DenominatorB);
    
    __m256d InfA = _mm256_div_pd(NuA, DeA);
    __m256d InfB = _mm256_div_pd(NuB, DeB);
    
    predicate_result Result = CompareAllWays(InfA, InfB);
    return Result;
}

static predicate_result NaNTest(b32 Signaling)
{
    double NumeratorA[] = {1.0, 0.0, 1.0, 0.0};
    double NumeratorB[] = {2.0, 2.0, 0.0, 0.0};
    
    double DenominatorA[] = {1.0, 0.0, 1.0, 0.0};
    double DenominatorB[] = {1.0, 1.0, 0.0, 0.0};
    
    __m256d NuA = _mm256_loadu_pd(NumeratorA);
    __m256d NuB = _mm256_loadu_pd(NumeratorB);
    
    __m256d DeA = _mm256_loadu_pd(DenominatorA);
    __m256d DeB = _mm256_loadu_pd(DenominatorB);
    
    __m256d NaNA = _mm256_div_pd(NuA, DeA);
    __m256d NaNB = _mm256_div_pd(NuB, DeB);
    
    if(Signaling)
    {
        __m256d SignBit = _mm256_set1_pd(-0.0);
        NaNA = _mm256_andnot_pd(SignBit, NaNA);
        NaNB = _mm256_andnot_pd(SignBit, NaNB);
    }
    
    predicate_result Result = CompareAllWays(NaNA, NaNB);
    return Result;
}

static char const *ToF(u64 Val)
{
    char const *Result = Val ? "T" : "-";
    return Result;
}

static void Print(char const *Description, predicate_result P)
{
    f64 A[4];
    f64 B[4];
    
    _mm256_storeu_pd(A, P.A);
    _mm256_storeu_pd(B, P.B);
    
    printf("%s\n", Description);
    printf("  %9f %9f %9f %9f\n", A[0], A[1], A[2], A[3]);
    printf("  %9f %9f %9f %9f\n", B[0], B[1], B[2], B[3]);
    for(u32 PIndex = 0; PIndex < ArrayCount(P.Res); ++PIndex)
    {
        simd_cmp CMP = CMPImmediates[PIndex];
        u64 Res[4] = {};
        _mm256_storeu_pd((f64 *)Res, P.Res[CMP.Value]);
        printf("  %8s  %s %s  %s %s %s %s\n",
               CMP.Operator,
               CMP.Unordered ? "U" : "-",
               CMP.Signaling ? "S" : "-",
               ToF(Res[0]), ToF(Res[1]), ToF(Res[2]), ToF(Res[3]));
    }
}

int main(void)
{
    predicate_result InfRes = InfTest();
    Print("Inf Compares", InfRes);
    
    predicate_result QNaNRes = NaNTest(false);
    Print("QNaN Compares", QNaNRes);
    
    predicate_result SNaNRes = NaNTest(true);
    Print("SNaN Compares", SNaNRes);
    
    return 0;
}