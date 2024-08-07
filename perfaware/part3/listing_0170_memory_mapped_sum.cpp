/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 170
   ======================================================================== */

static u64 MapAndSum(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize)
{
    u64 Result = 0;
    
    memory_mapped_file MappedFile = OpenMemoryMappedFile(FileName);
    if(IsValid(MappedFile))
    {
        u64 AtOffset = 0;
        while(AtOffset < TotalFileSize)
        {
            u64 SizeRemaining = TotalFileSize - AtOffset;
            u64 ReadSize = BufferSize;
            if(ReadSize > SizeRemaining)
            {
                ReadSize = SizeRemaining;
            }
            
            SetMapRegion(&MappedFile, AtOffset, ReadSize);
            if(IsValid(MappedFile.Memory))
            {
                Result += Sum64s(MappedFile.Memory.Count, MappedFile.Memory.Data);
                CountBytes(Tester, ReadSize);
            }
            else
            {
                Error(Tester, "Memory map failed");
            }
            
            AtOffset += ReadSize;
        }
    }
    else
    {
        Error(Tester, "Couldn't open file for mapping");
    }

    CloseMemoryMappedFile(&MappedFile);
    
    return Result;
}

struct threaded_mapped_io
{
    buffer Memory;
    volatile b32 ThreadFinished;
};

THREAD_ENTRY_POINT(MappedIOThreadRoutine, Parameter)
{
    threaded_mapped_io *ThreadedIO = (threaded_mapped_io *)Parameter;
    
    u32 PreventOptimization = 0;
    buffer Memory = ThreadedIO->Memory;
    for(u64 TouchOffset = 0; TouchOffset < Memory.Count; TouchOffset += MIN_OS_PAGE_SIZE)
    {
        PreventOptimization += Memory.Data[TouchOffset];
    }
    
    ThreadedIO->ThreadFinished = true;
    
    return PreventOptimization;
}

static u64 MapAndSumOverlapped(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize)
{
    (void)BufferSize;
    
    u64 Result = 0;

    memory_mapped_file MappedFile = OpenMemoryMappedFile(FileName);
    if(IsValid(MappedFile))
    {
        SetMapRegion(&MappedFile, 0, TotalFileSize);
        if(IsValid(MappedFile.Memory))
        {
            threaded_mapped_io ThreadedIO = {};
            ThreadedIO.Memory = MappedFile.Memory;
            
            thread_handle IOThread = CreateAndStartThread(MappedIOThreadRoutine, &ThreadedIO);
            if(IsValidThread(IOThread))
            {
                Result = Sum64s(MappedFile.Memory.Count, MappedFile.Memory.Data);
                CountBytes(Tester, MappedFile.Memory.Count);
                
                while(!ThreadedIO.ThreadFinished) {CPUWaitLoop();}
            }
            else
            {
                Error(Tester, "Couldn't start touch thread");
            }
        }
        else
        {
            Error(Tester, "Couldn't map file memory");
        }
    }
    else
    {
        Error(Tester, "Couldn't open file for mapping");
    }
    
    CloseMemoryMappedFile(&MappedFile);
    
    return Result;
}
