/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 167
   ======================================================================== */

typedef u64 file_process_func(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize);

static u64 Sum64s(u64 DataSize, void *Data)
{
    u64 *Source = (u64 *)Data;
    u64 Sum0 = 0;
    u64 Sum1 = 0;
    u64 Sum2 = 0;
    u64 Sum3 = 0;
    u64 SumCount = DataSize / (4*8);
    while(SumCount--)
    {
        Sum0 += Source[0];
        Sum1 += Source[1];
        Sum2 += Source[2];
        Sum3 += Source[3];
        Source += 4;
    }
    
    u64 Result = Sum0 + Sum1 + Sum2 + Sum3;
    return Result;
}

static u64 OpenAllocateAndFRead(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize)
{
    u64 Result = 0;
    
    FILE *File = fopen(FileName, "rb");
    buffer Buffer = AllocateBuffer(BufferSize);
    if(File && IsValid(Buffer))
    {
        u64 SizeRemaining = TotalFileSize;
        while(SizeRemaining)
        {
            u64 ReadSize = Buffer.Count;
            if(ReadSize > SizeRemaining)
            {
                ReadSize = SizeRemaining;
            }
            
            if(fread(Buffer.Data, ReadSize, 1, File) == 1)
            {
                CountBytes(Tester, ReadSize);
            }
            else
            {
                Error(Tester, "fread failed");
            }
            
            SizeRemaining -= ReadSize;
        }
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&Buffer);
    fclose(File);
    
    return Result;
}

static u64 OpenAllocateAndSum(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize)
{
    u64 Result = 0;
    
    FILE *File = fopen(FileName, "rb");
    buffer Buffer = AllocateBuffer(BufferSize);
    if((File != INVALID_HANDLE_VALUE) && IsValid(Buffer))
    {
        u64 SizeRemaining = TotalFileSize;
        while(SizeRemaining)
        {
            u64 ReadSize = Buffer.Count;
            if(ReadSize > SizeRemaining)
            {
                ReadSize = SizeRemaining;
            }
            
            if(fread(Buffer.Data, ReadSize, 1, File) == 1)
            {
                Result += Sum64s(ReadSize, Buffer.Data);
                CountBytes(Tester, ReadSize);
            }
            else
            {
                Error(Tester, "fread failed");
            }
            
            SizeRemaining -= ReadSize;
        }
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&Buffer);
    fclose(File);
    
    return Result;
}

enum overlapped_buffer_state
{
    Buffer_Unused,
    Buffer_ReadCompleted,
};
struct overlapped_buffer
{
    buffer Value;
    volatile u64 ReadSize;
    volatile overlapped_buffer_state State;
};

struct threaded_io
{
    overlapped_buffer Buffers[2];
    u64 TotalFileSize;
    FILE *File;
    b32 ReadError;
};

THREAD_ENTRY_POINT(IOThreadRoutine, Parameter)
{
    threaded_io *ThreadedIO = (threaded_io *)Parameter;
    
    FILE *File = ThreadedIO->File;
    u32 BufferIndex = 0;
    u64 SizeRemaining = ThreadedIO->TotalFileSize;
    while(SizeRemaining)
    {
        overlapped_buffer *Buffer = &ThreadedIO->Buffers[BufferIndex++ & 1];
        u64 ReadSize = Buffer->Value.Count;
        if(ReadSize > SizeRemaining)
        {
            ReadSize = SizeRemaining;
        }
        
        while(Buffer->State != Buffer_Unused) {_mm_pause();}

        EXCESSIVE_FENCE;
        
        if(fread(Buffer->Value.Data, ReadSize, 1, File) != 1)
        {
            ThreadedIO->ReadError = true;
        }
        
        Buffer->ReadSize = ReadSize;
        
        EXCESSIVE_FENCE;
        
        Buffer->State = Buffer_ReadCompleted;
            
        SizeRemaining -= ReadSize;
    }
    
    return 0;
}

static u64 OpenAllocateAndSumOverlapped(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize)
{
    u64 Result = 0;

    threaded_io ThreadedIO = {};
    ThreadedIO.File = fopen(FileName, "rb");
    ThreadedIO.TotalFileSize = TotalFileSize;
    ThreadedIO.Buffers[0].Value = AllocateBuffer(BufferSize);
    ThreadedIO.Buffers[1].Value = AllocateBuffer(BufferSize);
    
    thread_handle IOThread = {};
    if(ThreadedIO.File &&
       IsValid(ThreadedIO.Buffers[0].Value) && 
       IsValid(ThreadedIO.Buffers[1].Value))
    {
        IOThread = CreateAndStartThread(IOThreadRoutine, &ThreadedIO);
    }
    
    if(IsValidThread(IOThread))
    {
        u64 BufferIndex = 0;
        u64 SizeRemaining = TotalFileSize;
        while(SizeRemaining)
        {
            overlapped_buffer *Buffer = &ThreadedIO.Buffers[BufferIndex++ & 1];
            
            while(Buffer->State != Buffer_ReadCompleted) {_mm_pause();}
            
            EXCESSIVE_FENCE;
            
            u64 ReadSize = Buffer->ReadSize;
            Result += Sum64s(ReadSize, Buffer->Value.Data);
            CountBytes(Tester, ReadSize);
            
            EXCESSIVE_FENCE;
            
            Buffer->State = Buffer_Unused;
            
            SizeRemaining -= ReadSize;
        }
        
        if(ThreadedIO.ReadError)
        {
            Error(Tester, "fread failed");
        }    
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&ThreadedIO.Buffers[0].Value);
    FreeBuffer(&ThreadedIO.Buffers[1].Value);
    fclose(ThreadedIO.File);
    
    return Result;
}
