/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 227
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <math.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

int main(void)
{
    __m256d Sum = _mm256_setr_pd(1, 10, 100, 1000);
    
    __m256d SumSwap = _mm256_permute4x64_pd(Sum, 2 + (3 << 2));
    __m256d SumHalf = _mm256_add_pd(Sum, SumSwap);
    
    __m256d SumHalfSwap = _mm256_permute_pd(SumHalf, 1);
    __m256d SumFull = _mm256_add_pd(SumHalf, SumHalfSwap);
    
    printf("Full sum: %f\n", _mm256_cvtsd_f64(SumFull));
    
    return 0;
}
