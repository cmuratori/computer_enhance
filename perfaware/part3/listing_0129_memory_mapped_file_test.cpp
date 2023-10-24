/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 129
   ======================================================================== */

static u64 volatile GlobalSumSink;

static void ReadViaMapViewOfFile(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        HANDLE File = CreateFileA(Params->FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        BeginTime(Tester);

        HANDLE Mapping = CreateFileMappingA(File, 0, PAGE_READONLY, 0, 0, 0);
        u8 *Data = (u8 *)MapViewOfFile(Mapping, FILE_MAP_READ, 0, 0, 0);
        if(Data)
        {
            u64 TotalSize = Params->Dest.Count;
            u64 PageSize = 4096;

            u64 TestSum = 0;
            for(u64 ByteIndex = 0; ByteIndex < TotalSize; ByteIndex += PageSize)
            {
                TestSum += Data[ByteIndex];
            }
            
            GlobalSumSink = TestSum; // NOTE(casey): This is an attempt to prevent the compiler from optimizing out the sum
            
            CountBytes(Tester, TotalSize);
        }
        else
        {
            Error(Tester, "Unable to read file");
        }

        EndTime(Tester);

        UnmapViewOfFile(Data);
        CloseHandle(Mapping);
        CloseHandle(File);
    }
}
