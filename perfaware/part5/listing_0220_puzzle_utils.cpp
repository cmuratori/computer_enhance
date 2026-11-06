/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 220
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#if _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

typedef uint8_t u8;
typedef uint32_t u32;
typedef float f32;
typedef double f64;

union m256
{
    __m256 P;
    __m256d D;
    __m256i I;
};

#define SIMD_BYTE_COUNT 32

inline void Category(char const *Description)
{
    printf("\n");
    printf("========================================================================\n");
    printf("%s\n", Description);
    printf("========================================================================\n");
}

inline void Section(char const *Mnemonic, char const *Description)
{
    printf("\n");
    printf("%s: %s\n", Mnemonic, Description);
}

inline void PrintSpacing(u32 CharCount)
{
    if(CharCount == SIMD_BYTE_COUNT/2)
    {
        printf(" ");
    }
}

inline void PrintInner(u32 CharCount, char C)
{
    PrintSpacing(CharCount);
    
    if(C == 0)
    {
        C = '0';
    }
    
    printf("%c", C);
}

inline void Print(char const *Label, char const *Values)
{
    printf("%s: ", Label);
    
    for(u32 Index = 0; Index < SIMD_BYTE_COUNT; ++Index)
    {
        PrintInner(Index, Values[Index]);
    }
    
    printf("  ");
    
    for(u32 RIndex = 0; RIndex < SIMD_BYTE_COUNT; ++RIndex)
    {
        PrintInner(RIndex, Values[(SIMD_BYTE_COUNT - 1) - RIndex]);
    }
    
    printf("\n");
}

inline void Print(char const *Label, __m256 P)
{
    char Values[SIMD_BYTE_COUNT];
    
    _mm256_storeu_ps((f32 *)Values, P);
    Print(Label, Values);
}

inline void Print(char const *Label, __m256i P)
{
    char Values[SIMD_BYTE_COUNT];
    
    _mm256_storeu_si256((__m256i *)Values, P);
    Print(Label, Values);
}

inline void Print(char const *Label, __m256d P)
{
    char Values[SIMD_BYTE_COUNT];
    
    _mm256_storeu_pd((f64 *)Values, P);
    Print(Label, Values);
}

inline u32 MatchError(u32 CharCount, u32 Index, char const *Target, char const *Compare)
{
    u32 ErrorCount = 0;
    
    PrintSpacing(CharCount);
    
    char T = Target[Index];
    if(T == '0')
    {
        T = 0;
    }
    
    char C = ' ';
    if(Compare[Index] != T)
    {
        C = '_';
        ++ErrorCount;
    }
    
    printf("%c", C);
    
    return ErrorCount;
}

inline void Match(char const *Label, m256 Value, char const *Target)
{
    char Compare[SIMD_BYTE_COUNT];
    _mm256_storeu_pd((f64 *)Compare, Value.D);
    
    u32 ErrorCount = 0;
    
    printf("   ");
    
    for(u32 Index = 0; Index < SIMD_BYTE_COUNT; ++Index)
    {
        ErrorCount += MatchError(Index, Index, Target, Compare);
    }
    
    printf("  ");
    
    for(u32 RIndex = 0; RIndex < SIMD_BYTE_COUNT; ++RIndex)
    {
        MatchError(RIndex, (SIMD_BYTE_COUNT - 1) - RIndex, Target, Compare);
    }
    
    printf("\n");
    
    Print("T", Target);
    Print("C", Compare);
               
    printf("%s: ", Label);
    if(ErrorCount)
    {
        printf("FAILED with %u errors :(\n\n", ErrorCount);
    }
    else
    {
        printf("Passed!\n");
    }
               
    printf("\n");
}

inline m256 M256From(__m256 Value)
{
    m256 Result = {};
    Result.P = Value;
    return Result;
}

inline m256 M256From(__m256d Value)
{
    m256 Result = {};
    Result.D = Value;
    return Result;
}

inline m256 M256From(__m256i Value)
{
    m256 Result = {};
    Result.I = Value;
    return Result;
}

inline __m256 SolutionGoesHere(m256 = {}, m256 = {})
{
    __m256 Result = _mm256_loadu_ps((f32 *)"????????" "????????" "????????" "????????");
    return Result;
}

#define PRINT_SHUFFLE(Intrinsic, Shuffle, ShuffleParams, ...) \
Print(#ShuffleParams, Intrinsic(__VA_ARGS__, Shuffle ShuffleParams))

#define PRINT_PACK(Intrinsic, ...) Print(#Intrinsic, Intrinsic(__VA_ARGS__))
