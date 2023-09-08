/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 110
   ======================================================================== */

static void WriteToAllBytes(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        buffer DestBuffer = Params->Dest;
        HandleAllocation(Params, &DestBuffer);
        
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
