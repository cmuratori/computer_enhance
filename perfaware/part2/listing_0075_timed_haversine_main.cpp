/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 75
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <sys/stat.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t b32;

typedef float f32;
typedef double f64;

struct haversine_pair
{
    f64 X0, Y0;
    f64 X1, Y1;
};

#include "listing_0074_platform_metrics.cpp"
#include "listing_0065_haversine_formula.cpp"
#include "listing_0068_buffer.cpp"
#include "listing_0069_lookup_json_parser.cpp"

static buffer ReadEntireFile(char *FileName)
{
    buffer Result = {};
        
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
#if _WIN32
        struct __stat64 Stat;
        _stat64(FileName, &Stat);
#else
        struct stat Stat;
        stat(FileName, &Stat);
#endif
        
        Result = AllocateBuffer(Stat.st_size);
        if(Result.Data)
        {
            if(fread(Result.Data, Result.Count, 1, File) != 1)
            {
                fprintf(stderr, "ERROR: Unable to read \"%s\".\n", FileName);
                FreeBuffer(&Result);
            }
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open \"%s\".\n", FileName);
    }
    
    return Result;
}

static f64 SumHaversineDistances(u64 PairCount, haversine_pair *Pairs)
{
    f64 Sum = 0;
    
    f64 SumCoef = 1 / (f64)PairCount;
    for(u64 PairIndex = 0; PairIndex < PairCount; ++PairIndex)
    {
        haversine_pair Pair = Pairs[PairIndex];
        f64 EarthRadius = 6372.8;
        f64 Dist = ReferenceHaversine(Pair.X0, Pair.Y0, Pair.X1, Pair.Y1, EarthRadius);
        Sum += SumCoef*Dist;
    }
    
    return Sum;
}

static void PrintTimeElapsed(char const *Label, u64 TotalTSCElapsed, u64 Begin, u64 End)
{
    u64 Elapsed = End - Begin;
    f64 Percent = 100.0 * ((f64)Elapsed / (f64)TotalTSCElapsed);
    printf("  %s: %llu (%.2f%%)\n", Label, Elapsed, Percent);
}

int main(int ArgCount, char **Args)
{
    u64 Prof_Begin = 0;
    u64 Prof_Read = 0;
    u64 Prof_MiscSetup = 0;
    u64 Prof_Parse = 0;
    u64 Prof_Sum = 0;
    u64 Prof_MiscOutput = 0;
    u64 Prof_End = 0;
    
    Prof_Begin = ReadCPUTimer();
    
    int Result = 1;
    
    if((ArgCount == 2) || (ArgCount == 3))
    {
        Prof_Read = ReadCPUTimer();
        buffer InputJSON = ReadEntireFile(Args[1]);
        Prof_MiscSetup = ReadCPUTimer();
        
        u32 MinimumJSONPairEncoding = 6*4;
        u64 MaxPairCount = InputJSON.Count / MinimumJSONPairEncoding;
        if(MaxPairCount)
        {
            buffer ParsedValues = AllocateBuffer(MaxPairCount * sizeof(haversine_pair));
            if(ParsedValues.Count)
            {
                haversine_pair *Pairs = (haversine_pair *)ParsedValues.Data;
                
                Prof_Parse = ReadCPUTimer();
                u64 PairCount = ParseHaversinePairs(InputJSON, MaxPairCount, Pairs);
                Prof_Sum = ReadCPUTimer();
                f64 Sum = SumHaversineDistances(PairCount, Pairs);
                Prof_MiscOutput = ReadCPUTimer();
                
                Result = 0;
                
                fprintf(stdout, "Input size: %llu\n", InputJSON.Count);
                fprintf(stdout, "Pair count: %llu\n", PairCount);
                fprintf(stdout, "Haversine sum: %.16f\n", Sum);
                
                if(ArgCount == 3)
                {
                    buffer AnswersF64 = ReadEntireFile(Args[2]);
                    if(AnswersF64.Count >= sizeof(f64))
                    {
                        f64 *AnswerValues = (f64 *)AnswersF64.Data;
                        
                        fprintf(stdout, "\nValidation:\n");
                        
                        u64 RefAnswerCount = (AnswersF64.Count - sizeof(f64)) / sizeof(f64);
                        if(PairCount != RefAnswerCount)
                        {
                            fprintf(stdout, "FAILED - pair count doesn't match %llu.\n", RefAnswerCount);
                        }
                        
                        f64 RefSum = AnswerValues[RefAnswerCount];
                        fprintf(stdout, "Reference sum: %.16f\n", RefSum);
                        fprintf(stdout, "Difference: %.16f\n", Sum - RefSum);
                        
                        fprintf(stdout, "\n");
                    }
                }
            }
            
            FreeBuffer(&ParsedValues);
        }
        else
        {
            fprintf(stderr, "ERROR: Malformed input JSON\n");
        }
        
        FreeBuffer(&InputJSON);
    }
    else
    {
        fprintf(stderr, "Usage: %s [haversine_input.json]\n", Args[0]);
        fprintf(stderr, "       %s [haversine_input.json] [answers.f64]\n", Args[0]);
    }
    
    Prof_End = ReadCPUTimer();
    
    if(Result == 0)
    {
        u64 TotalCPUElapsed = Prof_End - Prof_Begin;
        
        u64 CPUFreq = EstimateCPUTimerFreq();
        if(CPUFreq)
        {
            printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (f64)TotalCPUElapsed / (f64)CPUFreq, CPUFreq);
        }
        
        PrintTimeElapsed("Startup", TotalCPUElapsed, Prof_Begin, Prof_Read);
        PrintTimeElapsed("Read", TotalCPUElapsed, Prof_Read, Prof_MiscSetup);
        PrintTimeElapsed("MiscSetup", TotalCPUElapsed, Prof_MiscSetup, Prof_Parse);
        PrintTimeElapsed("Parse", TotalCPUElapsed, Prof_Parse, Prof_Sum);
        PrintTimeElapsed("Sum", TotalCPUElapsed, Prof_Sum, Prof_MiscOutput);
        PrintTimeElapsed("MiscOutput", TotalCPUElapsed, Prof_MiscOutput, Prof_End);
    }
    
    return Result;
}
