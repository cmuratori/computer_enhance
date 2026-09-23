/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 219
   ======================================================================== */

#include "listing_0208_combined_haversine_test.cpp"
#include "listing_0218_wide_haversine_math.cpp"

static f64 WideHaversineTest(haversine_setup Setup)
{
    u64 PairCount = Setup.PairCount;
    haversine_pair *Pairs = Setup.Pairs;
    
    __m256d Sum = _mm256_setzero_pd();
    
    __m256d SumCoef = _mm256_set1_pd((2.0*QUESTIONABLE_EARTH_RADIUS) / (f64)PairCount);
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        
        __m256d lat1 = _mm256_set1_pd(Pair.Y0);
        __m256d lat2 = _mm256_set1_pd(Pair.Y1);
        __m256d lon1 = _mm256_set1_pd(Pair.X0);
        __m256d lon2 = _mm256_set1_pd(Pair.X1);
        
        __m256d NegRadC = _mm256_set1_pd(-0.01745329251994329577);
        __m256d NegPi = _mm256_set1_pd(-3.14159265358979323846);
        __m256d HalfRadC = _mm256_set1_pd(0.00872664625997164788);
        __m256d HalfPi = _mm256_set1_pd(1.57079632679489661923);
        __m256d Half = _mm256_set1_pd(0.5);
        __m256d One = _mm256_set1_pd(1.0);
        __m256d Deg180 = _mm256_set1_pd(180.0);
        
        __m256d AbsDLat = FAbs4x(_mm256_sub_pd(lat2, lat1));
        __m256d AbsDLon = FAbs4x(_mm256_sub_pd(lon2, lon1));
        __m256d AbsLat1 = FAbs4x(lat1);
        __m256d AbsLat2 = FAbs4x(lat2);
        
        __m256d ALC0Pred = _mm256_cmp_pd(AbsDLat, Deg180, _CMP_LT_OS);
        __m256d ALC0 = _mm256_andnot_pd(ALC0Pred, NegPi);
        
        __m256d ALC3Pred = _mm256_cmp_pd(AbsDLon, Deg180, _CMP_LT_OS);
        __m256d ALC3 = _mm256_andnot_pd(ALC3Pred, NegPi);
        
        __m256d S1 = SineCoreWithPrefix4x(NegRadC, AbsLat1, HalfPi);
        __m256d S2 = SineCoreWithPrefix4x(NegRadC, AbsLat2, HalfPi);
        __m256d S0 = SineCoreWithPrefix4x(HalfRadC, AbsDLat, ALC0);
        __m256d S3 = SineCoreWithPrefix4x(HalfRadC, AbsDLon, ALC3);
        
        __m256d S12 = _mm256_mul_pd(S1, S2);
        __m256d S33 = _mm256_mul_pd(S3, S3);
        __m256d S1233 = _mm256_mul_pd(S12, S33);
        __m256d a = _mm256_fmadd_pd(S0, S0, S1233);
        
        __m256d NeedsTransform = _mm256_cmp_pd(Half, a, _CMP_LT_OS);
        __m256d RangeA = _mm256_blendv_pd(a, _mm256_sub_pd(One, a), NeedsTransform);
        __m256d R = ArcsineCoreFromSquared4x(RangeA);
        __m256d RangeR = _mm256_blendv_pd(R, _mm256_sub_pd(HalfPi, R), NeedsTransform);
        
        Sum = _mm256_fmadd_pd(SumCoef, RangeR, Sum);
    }
    
    return _mm256_cvtsd_f64(Sum);
}

static test_function TestFunctions[] =
{
    {"ReferenceHaversine", ReferenceSumHaversine},
    {"WideHaversineTest", WideHaversineTest},
};

int main(int ArgCount, char **Args)
{
    CombinedHaversineTest(ArgCount, Args, ArrayCount(TestFunctions), TestFunctions);
    return 0;
}

        
