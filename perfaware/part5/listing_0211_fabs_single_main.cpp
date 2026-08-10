/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 211
   ======================================================================== */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

typedef int32_t b32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

union binary_f64
{
    f64 F;
    u64 U;
};

static f64 FAbsConditional(f64 Value)
{
    f64 Result = Value;
    if(Result < 0)
    {
        Result = -Result;
    }
    
    return Result;
}

static f64 FAbsBinary(f64 Value)
{
    binary_f64 SignBit;
    SignBit.F = -0.0;
    
    binary_f64 InvSignBit;
    InvSignBit.U = ~SignBit.U;
    
    binary_f64 Result;
    Result.F = Value;
    
    Result.U = Result.U & InvSignBit.U;
    
    return Result.F;
}

int main(void)
{
    f64 RandomFloats[] =
    {
        -0.32, 349.99, 12.2890, -490.0,
        -8589.91983, 39102901123901.0, 0.000000238, -0.000000000000028483,
    };
    
    for(u32 FloatIndex = 0; FloatIndex < ArrayCount(RandomFloats); ++FloatIndex)
    {
        f64 Input = RandomFloats[FloatIndex];

        f64 CLib = fabs(Input);
        f64 Cond = FAbsConditional(Input);
        f64 Binary = FAbsBinary(Input);
        
        b32 Match = ((CLib == Cond) && (Cond == Binary));
        printf("%s: %f -> %f\n", Match ? "Match" : "ERROR", Input, Binary);
    }
    
    return 0;
}
