/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 131
   ======================================================================== */

static void WriteToAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Tester, Params, &DestBuffer);
        
        BeginTime(Tester);
        for(u64 Index = 0; Index < DestBuffer.Count; ++Index)
        {
            DestBuffer.Data[Index] = (u8)Index;
        }
        EndTime(Tester);
        
        CountBytes(Tester, DestBuffer.Count);
        
        HandleDeallocation(Params, &DestBuffer);
    }
}

extern "C" void MOVAllBytesASM(u64 Count, u8 *Data);
extern "C" void NOPAllBytesASM(u64 Count);
extern "C" void CMPAllBytesASM(u64 Count);
extern "C" void DECAllBytesASM(u64 Count);
#pragma comment (lib, "listing_0132_nop_loop")

static void MOVAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Tester, Params, &DestBuffer);
        
        BeginTime(Tester);
        MOVAllBytesASM(DestBuffer.Count, DestBuffer.Data);
        EndTime(Tester);
        
        CountBytes(Tester, DestBuffer.Count);
        
        HandleDeallocation(Params, &DestBuffer);
    }
}

static void NOPAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Tester, Params, &DestBuffer);
        
        BeginTime(Tester);
        NOPAllBytesASM(DestBuffer.Count);
        EndTime(Tester);
        
        CountBytes(Tester, DestBuffer.Count);
        
        HandleDeallocation(Params, &DestBuffer);
    }
}

static void CMPAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Tester, Params, &DestBuffer);
        
        BeginTime(Tester);
        CMPAllBytesASM(DestBuffer.Count);
        EndTime(Tester);
        
        CountBytes(Tester, DestBuffer.Count);
        
        HandleDeallocation(Params, &DestBuffer);
    }
}

static void DECAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Tester, Params, &DestBuffer);
        
        BeginTime(Tester);
        DECAllBytesASM(DestBuffer.Count);
        EndTime(Tester);
        
        CountBytes(Tester, DestBuffer.Count);
        
        HandleDeallocation(Params, &DestBuffer);
    }
}
