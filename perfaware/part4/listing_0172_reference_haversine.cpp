/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 172
   ======================================================================== */

/* NOTE(casey): The radius of the Earth is set to 6372.8 because that was
   the constant used in the original question on which this code is based.
   The Earth is not a perfect sphere, but it is outside the scope of our
   exploration here to ask what the best value would be for this constant. */
#define QUESTIONABLE_EARTH_RADIUS 6372.8

struct haversine_pair
{
    f64 X0, Y0;
    f64 X1, Y1;
};

struct haversine_setup
{
    buffer JSONBuffer;
    buffer AnswerBuffer;
    buffer ParsedPairsBuffer;
    
    u64 ParsedByteCount;
    
    u64 PairCount;
    haversine_pair *Pairs;
    f64 *Answers;
    
    f64 SumAnswer;
    
    b32 Valid;
};

typedef f64 haversine_compute_func(haversine_setup Setup);
typedef u64 haversine_verify_func(haversine_setup Setup);

#include "listing_0069_lookup_json_parser.cpp"

static f64 Square(f64 A)
{
    f64 Result = (A*A);
    return Result;
}

static b32 ApproxAreEqual(f64 A, f64 B)
{
    /* NOTE(casey): Epsilon can be set to whatever tolerance we decide we will accept. If we make this value larger,
       we have more options for optimization. If we make it smaller, we must more closely follow the sequence
       of floating point operations that produced the original value. At zero, we would have to reproduce the
       sequence _exactly_. */
    f64 Epsilon = 0.00000001f;
    
    f64 Diff = (A - B);
    b32 Result = (Diff > -Epsilon) && (Diff < Epsilon);
    return Result;
}

static f64 RadiansFromDegrees(f64 Degrees)
{
    f64 Result = 0.01745329251994329577 * Degrees;
    return Result;
}

static f64 ReferenceHaversine(f64 X0, f64 Y0, f64 X1, f64 Y1, f64 EarthRadius)
{
    /* NOTE(casey): This is not meant to be a "good" way to calculate the Haversine distance.
       Instead, it attempts to follow, as closely as possible, the formula used in the real-world
       question on which these homework exercises are loosely based.
    */
    
    f64 lat1 = Y0;
    f64 lat2 = Y1;
    f64 lon1 = X0;
    f64 lon2 = X1;
    
    f64 dLat = RadiansFromDegrees(lat2 - lat1);
    f64 dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    f64 a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    f64 c = 2.0*asin(sqrt(a));
    
    f64 Result = EarthRadius * c;
    
    return Result;
}

static f64 ReferenceSumHaversine(haversine_setup Setup)
{
    u64 PairCount = Setup.PairCount;
    haversine_pair *Pairs = Setup.Pairs;
    
    f64 Sum = 0;
    
    f64 SumCoef = 1 / (f64)PairCount;
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        f64 EarthRadius = QUESTIONABLE_EARTH_RADIUS;
        f64 Dist = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
        Sum += SumCoef*Dist;
    }
    
    return Sum;
}

static u64 ReferenceVerifyHaversine(haversine_setup Setup)
{
    u64 PairCount = Setup.PairCount;
    haversine_pair *Pairs = Setup.Pairs;
    f64 *Answers = Setup.Answers;
    
    u64 ErrorCount = 0;
    
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        f64 EarthRadius = QUESTIONABLE_EARTH_RADIUS;
        f64 Dist = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
        if(!ApproxAreEqual(Dist, Answers[PairIndex]))
        {
            ++ErrorCount;
        }
    }
    
    return ErrorCount;
}

static b32 IsValid(haversine_setup Setup)
{
    b32 Result = Setup.Valid;
    return Result;
}

static haversine_setup SetUpHaversine(char *PairsJSONFileName, char *AnswerFileName)
{
    haversine_setup Result = {};
    
    Result.JSONBuffer = ReadEntireFile(PairsJSONFileName);
    Result.AnswerBuffer = ReadEntireFile(AnswerFileName);
    
    u32 MinimumJSONPairEncoding = 16; // NOTE(casey): There should be no way to define a pair in JSON without substantially more characters than this
    u64 MaxPairCount = Result.JSONBuffer.Count / MinimumJSONPairEncoding;
    Result.ParsedPairsBuffer = AllocateBuffer(sizeof(haversine_pair) * MaxPairCount);
    
    if(IsValid(Result.JSONBuffer) && IsValid(Result.AnswerBuffer) && IsValid(Result.ParsedPairsBuffer))
    {
        Result.Pairs = (haversine_pair *)Result.ParsedPairsBuffer.Data;
        
        u64 AnswerCount = Result.AnswerBuffer.Count / sizeof(f64);
        u64 PairCount = ParseHaversinePairs(Result.JSONBuffer, MaxPairCount, Result.Pairs);
        if(AnswerCount == (PairCount + 1))
        {
            Result.PairCount = PairCount;
            Result.Answers = (f64 *)Result.AnswerBuffer.Data;
            Result.SumAnswer = Result.Answers[PairCount];
            
            Result.ParsedByteCount = (sizeof(haversine_pair)*Result.PairCount);
            
            u64 Megabyte = 1024*1024;
            fprintf(stdout, "Source JSON: %llumb\n", Result.JSONBuffer.Count/Megabyte);
            fprintf(stdout, "Parsed: %llumb (%llu pairs)\n", Result.ParsedByteCount/Megabyte, Result.PairCount);
            
            Result.Valid = (Result.PairCount != 0);
        }
        else
        {
            fprintf(stderr, "ERROR: JSON source data has %llu pairs, but answer file has %llu values (should have %llu).\n",
                    PairCount, AnswerCount, PairCount + 1);
        }
    }
    
    return Result;
}

static void FreeHaversine(haversine_setup *Setup)
{
    FreeBuffer(&Setup->JSONBuffer);
    FreeBuffer(&Setup->ParsedPairsBuffer);
    FreeBuffer(&Setup->AnswerBuffer);
    
    *Setup = {};
}
