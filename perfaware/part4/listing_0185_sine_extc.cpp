/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 185
   ======================================================================== */

#if 1

inline f64 OddPowerPolynomialC(u32 CCount, f64 *C, f64 X)
{
    f64 X2 = X*X;
    
    f64 R = C[--CCount];
    while(CCount)
    {
        R = fma(R, X2, C[--CCount]);
    }
    R *= X;
   
    return R;
}

#else

// NOTE(casey): Optional intrinsic version, for reference

inline f64 OddPowerPolynomialC(u32 CCount, f64 *C, f64 XInit)
{
    __m128d X = _mm_set_sd(XInit);
    __m128d X2 = _mm_mul_sd(X, X);
    
    __m128d R = _mm_set_sd(C[--CCount]);
    while(CCount)
    {
        R = _mm_fmadd_sd(R, X2, _mm_set_sd(C[--CCount]));
    }
    R = _mm_mul_sd(R, X);
   
    f64 Result = _mm_cvtsd_f64(R);

    return Result;
}

#endif

inline f64 SineCore_Radians_MFTWP(f64 X)
{
    f64 X2 = X*X;
    
    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    
    f64 R = 0x1.883c1c5deffbep-49;
    R = fma(R, X2, -0x1.ae43dc9bf8ba7p-41);
    R = fma(R, X2, 0x1.6123ce513b09fp-33);
    R = fma(R, X2, -0x1.ae6454d960ac4p-26);
    R = fma(R, X2, 0x1.71de3a52aab96p-19);
    R = fma(R, X2, -0x1.a01a01a014eb6p-13);
    R = fma(R, X2, 0x1.11111111110c9p-7);
    R = fma(R, X2, -0x1.5555555555555p-3);
    R = fma(R, X2, 0x1p0);
    R *= X;
    
    return R;
}
