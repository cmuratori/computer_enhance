/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 127
   ======================================================================== */

#include <fcntl.h>
#include <io.h>

enum allocation_type
{
    AllocType_none,
    AllocType_malloc,
    AllocType_VirtualAlloc,
    AllocType_VirtualAllocLargePages,
    
    AllocType_Count,
};

struct read_parameters
{
    allocation_type AllocType;
    buffer Dest;
    char const *FileName;
};

typedef void read_overhead_test_func(repetition_tester *Tester, read_parameters *Params);

static char const *DescribeAllocationType(allocation_type AllocType)
{
    char const *Result;
    switch(AllocType)
    {
        case AllocType_none: {Result = "";} break;
        case AllocType_malloc: {Result = "malloc";} break;
        case AllocType_VirtualAlloc: {Result = "VirtualAlloc";} break;
        case AllocType_VirtualAllocLargePages: {Result = "VirtualAlloc (large)";} break;
        default : {Result = "UNKNOWN";} break;
    }
    
    return Result;
}

static void HandleAllocation(repetition_tester *Tester, read_parameters *Params, buffer *Buffer)
{
    switch(Params->AllocType)
    {
        case AllocType_none:
        {
        } break;
        
        case AllocType_malloc:
        {
            *Buffer = AllocateBuffer(Params->Dest.Count);
        } break;
        
        case AllocType_VirtualAlloc:
        case AllocType_VirtualAllocLargePages:
        {
            SIZE_T AllocSize = Params->Dest.Count;
            DWORD Flags = MEM_COMMIT|MEM_RESERVE;
            if(Params->AllocType == AllocType_VirtualAllocLargePages)
            {
                u64 LargePageSize = GetLargePageSize();
                if(LargePageSize)
                {
                    Flags |= MEM_LARGE_PAGES;
                    
                    // NOTE(casey): VirtualAlloc requires large page allocations to be an even multiple of the large
                    // page size (for some unknown reason). For non-large pages, it just rounds up for you, but for
                    // large pages, it requires you to do it.
                    AllocSize = (AllocSize + LargePageSize - 1) & ~(LargePageSize - 1);
                }
                else
                {
                    Error(Tester, "No large page support");
                }
            }
            
            u8 *AllocData = (u8 *)VirtualAlloc(0, AllocSize, Flags, PAGE_READWRITE);
            if(AllocData)
            {
                Buffer->Count = Params->Dest.Count;
                Buffer->Data = AllocData;
            }
            else
            {
                Error(Tester, "Allocation failed");
            }
        } break;
        
        default:
        {
            fprintf(stderr, "ERROR: Unrecognized allocation type");
        } break;
    }
}

static void HandleDeallocation(read_parameters *Params, buffer *Buffer)
{
    switch(Params->AllocType)
    {
        case AllocType_none:
        {
        } break;
        
        case AllocType_malloc:
        {
            FreeBuffer(Buffer);
        } break;
        
        case AllocType_VirtualAlloc:
        case AllocType_VirtualAllocLargePages:
        {
            VirtualFree(Buffer->Data, 0, MEM_RELEASE);
            *Buffer = {};
        } break;
        
        default:
        {
            fprintf(stderr, "ERROR: Unrecognized allocation type");
        } break;
    }
}

static void ReadViaFRead(repetition_tester *Tester, read_parameters *Params)
{
    while(IsTesting(Tester))
    {
        FILE *File = fopen(Params->FileName, "rb");
        if(File)
        {
            buffer DestBuffer = Params->Dest;
            HandleAllocation(Tester, Params, &DestBuffer);
            
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
            
            HandleDeallocation(Params, &DestBuffer);
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
            HandleAllocation(Tester, Params, &DestBuffer);
        
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
            
            HandleDeallocation(Params, &DestBuffer);
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
            HandleAllocation(Tester, Params, &DestBuffer);
            
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
            
            HandleDeallocation(Params, &DestBuffer);
            CloseHandle(File);
        }
        else
        {
            Error(Tester, "CreateFileA failed");
        }
    }
}
