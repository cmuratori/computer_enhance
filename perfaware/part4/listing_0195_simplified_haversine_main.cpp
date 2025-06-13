/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 195
   ======================================================================== */

#include "listing_0194_combined_haversine_test.cpp"
#include "listing_0185_sine_extc.cpp"

static f64 ExpandedHaversine(haversine_setup Setup)
{
    // NOTE(casey): Basic expansion of the replacement haversine
    
    u64 PairCount = Setup.PairCount;
    haversine_pair *Pairs = Setup.Pairs;
    
    f64 Sum = 0;
    
    f64 SumCoef = 1 / (f64)PairCount;
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        f64 EarthRadius = QUESTIONABLE_EARTH_RADIUS;
        
        f64 X0 = Pair.X0;
        f64 Y0 = Pair.Y0;
        f64 X1 = Pair.X1;
        f64 Y1 = Pair.Y1;
        
        f64 lat1 = Y0;
        f64 lat2 = Y1;
        f64 lon1 = X0;
        f64 lon2 = X1;
        
        f64 RadC = 0.01745329251994329577;
        f64 dLat = RadC * (lat2 - lat1); // RadiansFromDegrees(lat2 - lat1)
        f64 dLon = RadC * (lon2 - lon1); // RadiansFromDegrees(lon2 - lon1)
        lat1 = RadC * (lat1); // RadiansFromDegrees(lat1)
        lat2 = RadC * (lat2); // RadiansFromDegrees(lat2)
        
        f64 HalfPi = Pi64/2.0;
        f64 S0 = SinCE(dLat/2.0);
        f64 S1 = SinCE(lat1 + HalfPi); // cos(lat1);
        f64 S2 = SinCE(lat2 + HalfPi); // cos(lat2);
        f64 S3 = SinCE(dLon/2);
        
        f64 a = S0*S0 + S1*S2*S3*S3; // Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
        f64 SqrtA = SqrtCE(a);
        
        // NOTE(casey): ASinCE(SqrtA)
        b32 NeedsTransform = (SqrtA > 0.7071067811865475244);
        f64 RangeA = NeedsTransform ? SqrtCE(1.0 - SqrtA*SqrtA) : SqrtA;
        f64 R = ArcsineCore_MFTWP(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        f64 Dist = 2.0*EarthRadius*RangeR;
    
        Sum += SumCoef*Dist;
    }
    
    return Sum;
}

static f64 SimplifiedHaversineA(haversine_setup Setup)
{
    // NOTE(casey): Change running sum to an FMA
    
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
        f64 dLat = RadC * (lat2 - lat1); // RadiansFromDegrees(lat2 - lat1)
        f64 dLon = RadC * (lon2 - lon1); // RadiansFromDegrees(lon2 - lon1)
        lat1 = RadC * (lat1); // RadiansFromDegrees(lat1)
        lat2 = RadC * (lat2); // RadiansFromDegrees(lat2)
        
        f64 HalfPi = Pi64/2.0;
        f64 S0 = SinCE(dLat/2.0);
        f64 S1 = SinCE(lat1 + HalfPi); // cos(lat1);
        f64 S2 = SinCE(lat2 + HalfPi); // cos(lat2);
        f64 S3 = SinCE(dLon/2);
        
        f64 a = S0*S0 + S1*S2*S3*S3; // Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
        f64 SqrtA = SqrtCE(a);
        
        // NOTE(casey): ASinCE(SqrtA)
        b32 NeedsTransform = (SqrtA > 0.7071067811865475244);
        f64 RangeA = NeedsTransform ? SqrtCE(1.0 - SqrtA*SqrtA) : SqrtA;
        f64 R = ArcsineCore_MFTWP(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineB(haversine_setup Setup)
{
    // NOTE(casey): Change units conversions to FMAs
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1); // RadiansFromDegrees(lat2 - lat1) / 2
        f64 HalfDLon = HalfRadC * (lon2 - lon1); // RadiansFromDegrees(lon2 - lon1) / 2
        f64 ShiftedLat1 = fma(RadC, lat1, HalfPi); // RadiansFromDegrees(lat1) + HalfPi
        f64 ShiftedLat2 = fma(RadC, lat2, HalfPi); // RadiansFromDegrees(lat2) + HalfPi
        
        f64 S0 = SinCE(HalfDLat);
        f64 S1 = SinCE(ShiftedLat1);
        f64 S2 = SinCE(ShiftedLat2);
        f64 S3 = SinCE(HalfDLon);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3); // S0*S0 + S1*S2*S3*S3
        f64 SqrtA = SqrtCE(a);
        
        // NOTE(casey): ASinCE(SqrtA)
        b32 NeedsTransform = (SqrtA > 0.7071067811865475244);
        f64 RangeA = NeedsTransform ? SqrtCE(1.0 - SqrtA*SqrtA) : SqrtA;
        f64 R = ArcsineCore_MFTWP(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineC(haversine_setup Setup)
{
    // NOTE(casey): Reduce asin(sqrt) from two sqrts to one
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1);
        f64 HalfDLon = HalfRadC * (lon2 - lon1);
        f64 ShiftedLat1 = fma(RadC, lat1, HalfPi);
        f64 ShiftedLat2 = fma(RadC, lat2, HalfPi);
        
        f64 S0 = SinCE(HalfDLat);
        f64 S1 = SinCE(ShiftedLat1);
        f64 S2 = SinCE(ShiftedLat2);
        f64 S3 = SinCE(HalfDLon);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 SqrtRangeA = SqrtCE(RangeA);
        f64 R = ArcsineCore_MFTWP(SqrtRangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineD(haversine_setup Setup)
{
    // NOTE(casey): Remove unnecessary fabs from cosines
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1);
        f64 HalfDLon = HalfRadC * (lon2 - lon1);

        f64 ShiftedLat1 = fma(RadC, lat1, HalfPi);
        f64 ShiftedLat2 = fma(RadC, lat2, HalfPi);
        f64 P1 = ShiftedLat1; // fabs(ShiftedLat1);
        f64 P2 = ShiftedLat2; // fabs(ShiftedLat2);
        f64 I1 = (P1 > HalfPi) ? (Pi64 - P1) : P1;
        f64 I2 = (P2 > HalfPi) ? (Pi64 - P2) : P2;
        
        f64 S1 = SineCore_Radians_MFTWP(I1);
        f64 S2 = SineCore_Radians_MFTWP(I2);
        
        f64 S0 = SinCE(HalfDLat);
        f64 S3 = SinCE(HalfDLon);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 SqrtRangeA = SqrtCE(RangeA);
        f64 R = ArcsineCore_MFTWP(SqrtRangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineE(haversine_setup Setup)
{
    // NOTE(casey): Remove unnecessary substractions from cosines by case-checking before the fma
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1);
        f64 HalfDLon = HalfRadC * (lon2 - lon1);

        f64 SLC1 = (lat1 < 0) ? RadC : -RadC;
        f64 SLC2 = (lat2 < 0) ? RadC : -RadC;
        f64 I1 = fma(SLC1, lat1, HalfPi);
        f64 I2 = fma(SLC2, lat2, HalfPi);
        f64 S1 = SineCore_Radians_MFTWP(I1);
        f64 S2 = SineCore_Radians_MFTWP(I2);
        
        f64 S0 = SinCE(HalfDLat);
        f64 S3 = SinCE(HalfDLon);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 SqrtRangeA = SqrtCE(RangeA);
        f64 R = ArcsineCore_MFTWP(SqrtRangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineF(haversine_setup Setup)
{
    // NOTE(casey): Remove unnecessary post-negations from the sines (since both results end up being squared)
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1);
        f64 HalfDLon = HalfRadC * (lon2 - lon1);

        f64 SLC1 = (lat1 < 0) ? RadC : -RadC;
        f64 SLC2 = (lat2 < 0) ? RadC : -RadC;
        f64 I1 = fma(SLC1, lat1, HalfPi);
        f64 I2 = fma(SLC2, lat2, HalfPi);
        f64 S1 = SineCore_Radians_MFTWP(I1);
        f64 S2 = SineCore_Radians_MFTWP(I2);
        
        f64 P0 = fabs(HalfDLat);
        f64 P3 = fabs(HalfDLon);
        f64 I0 = (P0 > HalfPi) ? (Pi64 - P0) : P0;
        f64 I3 = (P3 > HalfPi) ? (Pi64 - P3) : P3;
        f64 S0 = SineCore_Radians_MFTWP(I0);
        f64 S3 = SineCore_Radians_MFTWP(I3);
        // f64 S0 = (P0 > HalfPi) ? T0 : -T0;
        // f64 S3 = (P3 > HalfPi) ? T3 : -T3;
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 SqrtRangeA = SqrtCE(RangeA);
        f64 R = ArcsineCore_MFTWP(SqrtRangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

inline f64 ArcsineCoreFromSquared(f64 X2)
{
    f64 X = SqrtCE(X2);

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

static f64 SimplifiedHaversineG(haversine_setup Setup)
{
    // NOTE(casey): Remove unnecessary squaring in arcsine
    
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
        
        f64 HalfDLat = HalfRadC * (lat2 - lat1);
        f64 HalfDLon = HalfRadC * (lon2 - lon1);

        f64 SLC1 = (lat1 < 0) ? RadC : -RadC;
        f64 SLC2 = (lat2 < 0) ? RadC : -RadC;
        f64 I1 = fma(SLC1, lat1, HalfPi);
        f64 I2 = fma(SLC2, lat2, HalfPi);
        f64 S1 = SineCore_Radians_MFTWP(I1);
        f64 S2 = SineCore_Radians_MFTWP(I2);
        
        f64 P0 = fabs(HalfDLat);
        f64 P3 = fabs(HalfDLon);
        f64 I0 = (P0 > HalfPi) ? (Pi64 - P0) : P0;
        f64 I3 = (P3 > HalfPi) ? (Pi64 - P3) : P3;
        f64 S0 = SineCore_Radians_MFTWP(I0);
        f64 S3 = SineCore_Radians_MFTWP(I3);
        // f64 S0 = (P0 > HalfPi) ? T0 : -T0;
        // f64 S3 = (P3 > HalfPi) ? T3 : -T3;
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 R = ArcsineCoreFromSquared(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
}

static f64 SimplifiedHaversineH(haversine_setup Setup)
{
    // NOTE(casey): Move sine range adjustment prior to FMAs
    
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
        f64 I1 = fma(SLC1, lat1, HalfPi);
        f64 I2 = fma(SLC2, lat2, HalfPi);
        f64 S1 = SineCore_Radians_MFTWP(I1);
        f64 S2 = SineCore_Radians_MFTWP(I2);
        
        f64 DLat = fabs(lat2 - lat1);
        f64 DLon = fabs(lon2 - lon1);
        f64 SLC0 = (DLat < Deg180) ? HalfRadC : -HalfRadC;
        f64 SLC3 = (DLon < Deg180) ? HalfRadC : -HalfRadC;
        f64 ALC0 = (DLat < Deg180) ? 0 : Pi64;
        f64 ALC3 = (DLon < Deg180) ? 0 : Pi64;
        f64 I0 = fma(SLC0, DLat, ALC0);
        f64 I3 = fma(SLC3, DLon, ALC3);
        f64 S0 = SineCore_Radians_MFTWP(I0);
        f64 S3 = SineCore_Radians_MFTWP(I3);
        
        f64 a = fma(S0, S0, S1*S2*S3*S3);
        
        b32 NeedsTransform = (a > 0.5);
        f64 RangeA = NeedsTransform ? (1.0 - a) : a;
        f64 R = ArcsineCoreFromSquared(RangeA);
        f64 RangeR = NeedsTransform ? (1.57079632679489661923 - R) : R;
        
        Sum = fma(SumCoef, RangeR, Sum);
    }
    
    return Sum;
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
    // NOTE(casey): Unify all sine FMAs
    
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
    {"ReplacementHaversine", ReplacementSumHaversine},
    {"ExpandedHaversine", ExpandedHaversine},
    {"SimplifiedHaversineA", SimplifiedHaversineA},
    {"SimplifiedHaversineB", SimplifiedHaversineB},
    {"SimplifiedHaversineC", SimplifiedHaversineC},
    {"SimplifiedHaversineD", SimplifiedHaversineD},
    {"SimplifiedHaversineE", SimplifiedHaversineE},
    {"SimplifiedHaversineF", SimplifiedHaversineF},
    {"SimplifiedHaversineG", SimplifiedHaversineG},
    {"SimplifiedHaversineH", SimplifiedHaversineH},
    {"SimplifiedHaversineI", SimplifiedHaversineI},
};

int main(int ArgCount, char **Args)
{
    CombinedHaversineTest(ArgCount, Args, ArrayCount(TestFunctions), TestFunctions);
    return 0;
}

