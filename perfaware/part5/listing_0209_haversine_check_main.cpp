/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 209
   ======================================================================== */

#include "listing_0208_combined_haversine_test.cpp"

inline f64 ArcsineCoreFromSquared(f64 X2)
{
    f64 X = _mm_cvtsd_f64(_mm_sqrt_sd(_mm_setzero_pd(), _mm_set_sd(X2)));
    
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

inline f64 SineCoreWithPrefix(f64 A, f64 B, f64 C)
{
    f64 X = fma(A, B, C);
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

static f64 SimplifiedHaversineI(haversine_setup Setup)
{
    u64 PairCount = Setup.PairCount;
    haversine_pair *Pairs = Setup.Pairs;
    
    f64 Sum = 0;
    
    f64 SumCoef = (2.0*QUESTIONABLE_EARTH_RADIUS) / (f64)PairCount;
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        
        f64 lat1 = Pair.Y0;
        f64 lat2 = Pair.Y1;
        f64 lon1 = Pair.X0;
        f64 lon2 = Pair.X1;

        f64 RadC = 0.01745329251994329577;
        f64 HalfRadC = RadC/2.0;
        f64 HalfPi = Pi64/2.0;
        f64 Deg180 = 180.0;
        
        f64 SLC1 = (lat1 < 0) ? RadC : -RadC;
        f64 SLC2 = (lat2 < 0) ? RadC : -RadC;
        
        f64 DLat = fabs(lat2 - lat1);
        f64 DLon = fabs(lon2 - lon1);
        f64 SLC0 = (DLat < Deg180) ? HalfRadC : -HalfRadC;
        f64 SLC3 = (DLon < Deg180) ? HalfRadC : -HalfRadC;
        f64 ALC0 = (DLat < Deg180) ? 0 : Pi64;
        f64 ALC3 = (DLon < Deg180) ? 0 : Pi64;

        f64 S1 = SineCoreWithPrefix(SLC1, lat1, HalfPi);
        f64 S2 = SineCoreWithPrefix(SLC2, lat2, HalfPi);
        f64 S0 = SineCoreWithPrefix(SLC0, DLat, ALC0);
        f64 S3 = SineCoreWithPrefix(SLC3, DLon, ALC3);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 R = ArcsineCoreFromSquared(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static test_function TestFunctions[] =
{
    {"ReferenceHaversine", ReferenceSumHaversine},
    {"SimplifiedHaversineI", SimplifiedHaversineI},
};

int main(int ArgCount, char **Args)
{
    CombinedHaversineTest(ArgCount, Args, ArrayCount(TestFunctions), TestFunctions);
    return 0;
}

