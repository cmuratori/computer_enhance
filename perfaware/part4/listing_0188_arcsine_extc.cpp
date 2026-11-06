/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 188
   ======================================================================== */

inline f64 ArcsineCore_MFTWP(f64 X)
{
    f64 X2 = X*X;
    
    // NOTE(casey): These minimax coefficients were donated by Demetri Spanos
    
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
    
    return R;
}
