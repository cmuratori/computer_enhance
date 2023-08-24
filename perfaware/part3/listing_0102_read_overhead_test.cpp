/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 102
   ======================================================================== */

#include <windows.h>
#include <fcntl.h>
#include <io.h>

struct read_parameters
{
    buffer Dest;
    char const *FileName;
};

typedef void read_overhead_test_func(repetition_tester *Tester, read_parameters *Params);

static void ReadViaFRead(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        FILE *File = fopen(Params->FileName, "rb");
        if(File)
        {
            buffer DestBuffer = Params->Dest;
            
            BeginTime(Tester);
            size_t Result = fread(DestBuffer.Data, DestBuffer.Count, 1, File);
            EndTime(Tester);
            
            if(Result == 1)
            {
                CountBytes(Tester, DestBuffer.Count);
            }
            else
            {
                Error(Tester, "fread failed");
            }
            
            fclose(File);
        }
        else
        {
            Error(Tester, "fopen failed");
        }
    }
}

static void ReadViaRead(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        int File = _open(Params->FileName, _O_BINARY|_O_RDONLY);
        if(File != -1)
        {
            buffer DestBuffer = Params->Dest;
        
            u8 *Dest = DestBuffer.Data;
            u64 SizeRemaining = DestBuffer.Count;
            while(SizeRemaining)
            {
                u32 ReadSize = INT_MAX;
                if((u64)ReadSize > SizeRemaining)
                {
                    ReadSize = (u32)SizeRemaining;
                }

                BeginTime(Tester);
                int Result = _read(File, Dest, ReadSize);
                EndTime(Tester);

                if(Result == (int)ReadSize)
                {
                    CountBytes(Tester, ReadSize);
                }
                else
                {
                    Error(Tester, "_read failed");
                    break;
                }
                
                SizeRemaining -= ReadSize;
                Dest += ReadSize;
            }
            
            _close(File);
        }
        else
        {
            Error(Tester, "_open failed");
        }
    }
}

static void ReadViaReadFile(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        HANDLE File = CreateFileA(Params->FileName, GENERIC_READ, FILE_SHARE_READ|FILE_SHARE_WRITE, 0,
                                  OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if(File != INVALID_HANDLE_VALUE)
        {
            buffer DestBuffer = Params->Dest;
            
            u64 SizeRemaining = Params->Dest.Count;
            u8 *Dest = (u8 *)DestBuffer.Data;
            while(SizeRemaining)
            {
                u32 ReadSize = (u32)-1;
                if((u64)ReadSize > SizeRemaining)
                {
                    ReadSize = (u32)SizeRemaining;
                }
                
                DWORD BytesRead = 0;
                BeginTime(Tester);
                BOOL Result = ReadFile(File, Dest, ReadSize, &BytesRead, 0);
                EndTime(Tester);
                
                if(Result && (BytesRead == ReadSize))
                {
                    CountBytes(Tester, ReadSize);
                }
                else
                {
                    Error(Tester, "ReadFile failed");
                }
                
                SizeRemaining -= ReadSize;
                Dest += ReadSize;
            }
            
            CloseHandle(File);
        }
        else
        {
            Error(Tester, "CreateFileA failed");
        }
    }
}
