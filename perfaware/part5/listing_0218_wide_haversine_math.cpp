/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 218
   ======================================================================== */

inline __m256d FMA4x(__m256d A, __m256d B, f64 C)
{
    __m256d Result = _mm256_fmadd_pd(A, B, _mm256_set1_pd(C));
    return Result;
}

inline __m256d ArcsineCoreFromSquared4x(__m256d X2) // NOTE(casey): 19 multiplies
{
    __m256d X = _mm256_sqrt_pd(X2);
    
    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    __m256d R = _mm256_set1_pd(0x1.dfc53682725cap-1);
    R = FMA4x(R, X2, -0x1.bec6daf74ed61p1);
    R = FMA4x(R, X2, 0x1.8bf4dadaf548cp2);
    R = FMA4x(R, X2, -0x1.b06f523e74f33p2);
    R = FMA4x(R, X2, 0x1.4537ddde2d76dp2);
    R = FMA4x(R, X2, -0x1.6067d334b4792p1);
    R = FMA4x(R, X2, 0x1.1fb54da575b22p0);
    R = FMA4x(R, X2, -0x1.57380bcd2890ep-2);
    R = FMA4x(R, X2, 0x1.69b370aad086ep-4);
    R = FMA4x(R, X2, -0x1.21438ccc95d62p-8);
    R = FMA4x(R, X2, 0x1.b8a33b8e380efp-7);
    R = FMA4x(R, X2, 0x1.c37061f4e5f55p-7);
    R = FMA4x(R, X2, 0x1.1c875d6c5323dp-6);
    R = FMA4x(R, X2, 0x1.6e88ce94d1149p-6);
    R = FMA4x(R, X2, 0x1.f1c73443a02f5p-6);
    R = FMA4x(R, X2, 0x1.6db6db3184756p-5);
    R = FMA4x(R, X2, 0x1.3333333380df2p-4);
    R = FMA4x(R, X2, 0x1.555555555531ep-3);
    R = FMA4x(R, X2, 0x1p0);
    R = _mm256_mul_pd(R, X);
    
    return R;
}

inline __m256d SineCore4x(__m256d X) // NOTE(casey): 10 multiplies
{
    __m256d X2 = _mm256_mul_pd(X, X);
    
    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    __m256d R = _mm256_set1_pd(0x1.883c1c5deffbep-49);
    R = FMA4x(R, X2, -0x1.ae43dc9bf8ba7p-41);
    R = FMA4x(R, X2, 0x1.6123ce513b09fp-33);
    R = FMA4x(R, X2, -0x1.ae6454d960ac4p-26);
    R = FMA4x(R, X2, 0x1.71de3a52aab96p-19);
    R = FMA4x(R, X2, -0x1.a01a01a014eb6p-13);
    R = FMA4x(R, X2, 0x1.11111111110c9p-7);
    R = FMA4x(R, X2, -0x1.5555555555555p-3);
    R = FMA4x(R, X2, 0x1p0);
    R = _mm256_mul_pd(R, X);
    
    return R;
}

inline __m256d SineCoreWithPrefix4x(__m256d A, __m256d B, __m256d C) // NOTE(casey): 11 multiplies
{
    __m256d X = _mm256_fmadd_pd(A, B, C);
    __m256d X2 = _mm256_mul_pd(X, X);
    
    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    __m256d R = _mm256_set1_pd(0x1.883c1c5deffbep-49);
    R = FMA4x(R, X2, -0x1.ae43dc9bf8ba7p-41);
    R = FMA4x(R, X2, 0x1.6123ce513b09fp-33);
    R = FMA4x(R, X2, -0x1.ae6454d960ac4p-26);
    R = FMA4x(R, X2, 0x1.71de3a52aab96p-19);
    R = FMA4x(R, X2, -0x1.a01a01a014eb6p-13);
    R = FMA4x(R, X2, 0x1.11111111110c9p-7);
    R = FMA4x(R, X2, -0x1.5555555555555p-3);
    R = FMA4x(R, X2, 0x1p0);
    R = _mm256_mul_pd(R, X);
    
    return R;
}

inline __m256d FAbs4x(__m256d Value)
{
    __m256d SignBit = _mm256_set1_pd(-0.0);
    __m256d Result = _mm256_andnot_pd(SignBit, Value);
    return Result;
}
