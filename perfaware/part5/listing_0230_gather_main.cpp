/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 230
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef double f64;
typedef uint32_t u32;
typedef uint64_t u64;

struct quad
{
    __m256d E[4];
};

static quad LoadRows4x4(f64 *Values)
{
    quad Result;
    
    Result.E[0] = _mm256_loadu_pd(Values + 0);
    Result.E[1] = _mm256_loadu_pd(Values + 4);
    Result.E[2] = _mm256_loadu_pd(Values + 8);
    Result.E[3] = _mm256_loadu_pd(Values + 12);
    
    return Result;
}

static quad GatherColumns4x4(f64 *Values)
{
    quad Result;

    __m128i Indexes = _mm_setr_epi32(0, 4, 8, 12);
    
    Result.E[0] = _mm256_i32gather_pd(Values + 0, Indexes, 8);
    Result.E[1] = _mm256_i32gather_pd(Values + 1, Indexes, 8);
    Result.E[2] = _mm256_i32gather_pd(Values + 2, Indexes, 8);
    Result.E[3] = _mm256_i32gather_pd(Values + 3, Indexes, 8);
    
    return Result;
}

static void PrintQuad(char const *Label, quad Q)
{
    printf("%s:\n", Label);
    for(u32 EIndex = 0; EIndex < 4; ++EIndex)
    {
        f64 V[4];
        _mm256_storeu_pd(V, Q.E[EIndex]);
        printf("       %7.1f %7.1f %7.1f %7.1f\n", V[0], V[1], V[2], V[3]);
    }
    
    __m256d SumA = _mm256_add_pd(Q.E[0], Q.E[1]);
    __m256d SumB = _mm256_add_pd(Q.E[2], Q.E[3]);
    __m256d SumAB = _mm256_add_pd(SumA, SumB);
    
    f64 S[4];
    _mm256_storeu_pd(S, SumAB);
    
    printf("  SUM: %7.1f %7.1f %7.1f %7.1f\n", S[0], S[1], S[2], S[3]);
    printf("\n");
}
    
int main(void)
{
    f64 Values[] =
    {
        1, 10, 100, 1000,
        2, 20, 200, 2000,
        3, 30, 300, 3000,
        4, 40, 400, 4000,
    };
    
    quad Rows = LoadRows4x4(Values);
    PrintQuad("Flat load", Rows);
    
    quad GatCols = GatherColumns4x4(Values);
    PrintQuad("Gathered columns", GatCols);
    
    return 0;
}
