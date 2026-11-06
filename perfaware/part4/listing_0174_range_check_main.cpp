/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 174
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#define __STDC_WANT_LIB_EXT1__ 1

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <float.h>
#include <math.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0125_buffer.cpp"
#include "listing_0169_os_platform.cpp"
#include "listing_0172_reference_haversine.cpp"

struct interval
{
    double Min;
    double Max;
};

struct ranges
{
    interval Sin;
    interval Cos;
    interval ASin;
    interval Sqrt;
};
static ranges Ranges;

static void IncludeIn(interval *Interval, double X)
{
    if(Interval->Min > X) {Interval->Min = X;}
    if(Interval->Max < X) {Interval->Max = X;}
}

static double SinRange(double X)
{
    IncludeIn(&Ranges.Sin, X);
    double Result = sin(X);
    return Result;
}

static double CosRange(double X)
{
    IncludeIn(&Ranges.Cos, X);
    double Result = cos(X);
    return Result;
}

static double ASinRange(double X)
{
    IncludeIn(&Ranges.ASin, X);
    double Result = asin(X);
    return Result;
}

static double SqrtRange(double X)
{
    IncludeIn(&Ranges.Sqrt, X);
    double Result = sqrt(X);
    return Result;
}

static double IncludeHaversineRange(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(SinRange(dLat/2.0)) + CosRange(lat1)*CosRange(lat2)*Square(SinRange(dLon/2));
    f64 c = 2.0*ASinRange(SqrtRange(a));
    
    f64 Result = EarthRadius * c;

    return Result;
}

static void PrInterval(char const *Label, interval Interval)
{
    printf("%s: %f,%f\n", Label, Interval.Min, Interval.Max);
}
            
int main(int ArgCount, char **Args)
{
    if(ArgCount == 3)
    {
        haversine_setup Setup = SetUpHaversine(Args[1], Args[2]);
        if(IsValid(Setup))
        {
            interval InvInf = {DBL_MAX, -DBL_MAX};
            Ranges.Sin = InvInf;
            Ranges.Cos = InvInf;
            Ranges.ASin = InvInf;
            Ranges.Sqrt = InvInf;
            
            for(u64 PairIndex = 0; PairIndex < Setup.PairCount; ++PairIndex)
            {
                haversine_pair Pair = Setup.Pairs[PairIndex];
                f64 Check = IncludeHaversineRange(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, QUESTIONABLE_EARTH_RADIUS);
                f64 Ref = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, QUESTIONABLE_EARTH_RADIUS);
                if(Check != Ref)
                {
                    printf("ERROR: Range-check version doesn't match reference version.\n");
                }
            }
            
            PrInterval("sin", Ranges.Sin);
            PrInterval("cos", Ranges.Cos);
            PrInterval("asin", Ranges.ASin);
            PrInterval("sqrt", Ranges.Sqrt);
        }
        FreeHaversine(&Setup);
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine pairs JSON] [haversine answer file]\n", Args[0]);
    }

    (void)&ReadOSPageFaultCount;
    (void)&InitializeOSPlatform;
    (void)&ReferenceSumHaversine;
    (void)&ReferenceVerifyHaversine;
    
    return 0;
}
