/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 190
   ======================================================================== */

/* NOTE(casey): The minimax coefficients used in these routines were donated
   by Demetri Spanos.
*/

inline f64 SinCE(f64 OrigX)
{
    f64 HalfPi = Pi64/2;
    f64 PosX = fabs(OrigX);
    f64 X = (PosX > HalfPi) ? (Pi64 - PosX) : PosX;
    
    f64 X2 = X*X;
    
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
    
    f64 Result = (OrigX < 0) ? -R : R;
    
    return Result;
}

inline f64 CosCE(f64 X)
{
    f64 Result = SinCE(X + Pi64/2.0);
    return Result;
}

inline f64 SqrtCE(f64 ScalarX)
{
    __m128d X = _mm_set_sd(ScalarX);
    
    __m128d SqrtX = _mm_sqrt_sd(X, X);
    
    f64 Result = _mm_cvtsd_f64(SqrtX);
    return Result;
}

inline f64 ASinCE(f64 OrigX)
{
    b32 NeedsTransform = (OrigX > 0.7071067811865475244);
    f64 X = NeedsTransform ? SqrtCE(1.0 - OrigX*OrigX) : OrigX;
    
    f64 X2 = X*X;
    
    f64 R = 0x1.dfc53682725cap-1;
    R = fma(R, X2, -0x1.bec6daf74ed61p1);
    R = fma(R, X2, 0x1.8bf4dadaf548cp2);
    R = fma(R, X2, -0x1.b06f523e74f33p2);
    R = fma(R, X2, 0x1.4537ddde2d76dp2);
    R = fma(R, X2, -0x1.6067d334b4792p1);
    R = fma(R, X2, 0x1.1fb54da575b22p0);
    R = fma(R, X2, -0x1.57380bcd2890ep-2);
    R = fma(R, X2, 0x1.69b370aad086ep-4);
    R = fma(R, X2, -0x1.21438ccc95d62p-8);
    R = fma(R, X2, 0x1.b8a33b8e380efp-7);
    R = fma(R, X2, 0x1.c37061f4e5f55p-7);
    R = fma(R, X2, 0x1.1c875d6c5323dp-6);
    R = fma(R, X2, 0x1.6e88ce94d1149p-6);
    R = fma(R, X2, 0x1.f1c73443a02f5p-6);
    R = fma(R, X2, 0x1.6db6db3184756p-5);
    R = fma(R, X2, 0x1.3333333380df2p-4);
    R = fma(R, X2, 0x1.555555555531ep-3);
    R = fma(R, X2, 0x1p0);
    R *= X;
    
    f64 Result = NeedsTransform ? (1.57079632679489661923 - R) : R;
    return Result;
}
