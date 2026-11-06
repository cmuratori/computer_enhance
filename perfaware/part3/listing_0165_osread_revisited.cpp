/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 165
   ======================================================================== */

#include <windows.h>

typedef void file_process_func(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch);

#define MIN_MEMORY_PAGE_SIZE 4096
static void AllocateAndTouch(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch)
{
    (void)FileName;
    (void)TotalFileSize;
    (void)Scratch;
    
    buffer Buffer = AllocateBuffer(BufferSize);
    if(IsValid(Buffer))
    {
        u64 TouchCount = (Buffer.Count + MIN_MEMORY_PAGE_SIZE - 1)/MIN_MEMORY_PAGE_SIZE;
        for(u64 TouchIndex = 0; TouchIndex < TouchCount; ++TouchIndex)
        {
            Buffer.Data[MIN_MEMORY_PAGE_SIZE*TouchIndex] = 0;
        }

        CountBytes(Tester, TotalFileSize);
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&Buffer);
}

static void AllocateAndCopy(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch)
{
    (void)FileName;

    buffer Buffer = AllocateBuffer(BufferSize);
    if(IsValid(Buffer))
    {
        u8 *Source = Scratch.Data;
        u64 SizeRemaining = TotalFileSize;
        while(SizeRemaining)
        {
            u64 ReadSize = Buffer.Count;
            if(ReadSize > SizeRemaining)
            {
                ReadSize = SizeRemaining;
            }
            
#if 0
            // NOTE(casey): On x64, this will do a direct rep movsb, which is often the fastest way to move memory - so
            // it's useful to use as a spot-check to make sure memcpy isn't introducing too much overhead.
            __movsb(Buffer.Data, Source, (u32)ReadSize);
#else
            memcpy(Buffer.Data, Source, (u32)ReadSize);
#endif
            
            CountBytes(Tester, ReadSize);
            
            SizeRemaining -= ReadSize;
            Source += ReadSize;
        }
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&Buffer);
}

static void OpenAllocateAndRead(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch)
{
    (void)Scratch;

    HANDLE File = CreateFileA(FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
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
            
            DWORD BytesRead = 0;
            BOOL Result = ReadFile(File, Buffer.Data, (u32)ReadSize, &BytesRead, 0);
            
            if(Result && (BytesRead == ReadSize))
            {
                CountBytes(Tester, ReadSize);
            }
            else
            {
                Error(Tester, "ReadFile failed");
            }
            
            SizeRemaining -= ReadSize;
        }
    }
    else
    {
        Error(Tester, "Couldn't acquire resources");
    }
    
    FreeBuffer(&Buffer);
    CloseHandle(File);
}

static void OpenAllocateAndFRead(repetition_tester *Tester, char const *FileName, u64 TotalFileSize, u64 BufferSize, buffer Scratch)
{
    (void)Scratch;
    
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
}
