/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 121
   ======================================================================== */

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea. */
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int32_t s32;

typedef int32_t b32;

typedef float f32;
typedef double f64;

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))

#include "listing_0068_buffer.cpp"

#define KERNEL_MEMORY_DLL_PATH L"kernelbase.dll"

//
// NOTE(casey): The following definitions are required if you do not have an up-to-date version of the Windows SDK
//
#ifndef MEM_REPLACE_PLACEHOLDER

#define MEM_REPLACE_PLACEHOLDER 0x4000
#define MEM_RESERVE_PLACEHOLDER 0x40000
#define MEM_PRESERVE_PLACEHOLDER 0x2
#define MemExtendedParameterAddressRequirements 1

struct MEM_EXTENDED_PARAMETER;

#endif

typedef PVOID virtual_alloc_2(HANDLE, PVOID, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER *, ULONG);
typedef PVOID map_view_of_file_3(HANDLE, HANDLE, PVOID, ULONG64, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER *, ULONG);

struct circular_buffer
{
    buffer Base;
    
    HANDLE FileMapping;
    u32 RepCount;
};

static b32 IsValid(circular_buffer Buffer)
{
    b32 Result = (Buffer.Base.Data != 0);
    return Result;
}

static u64 RoundToPow2Size(u64 MinimumSize, u64 Pow2Size)
{
    u64 Result = (MinimumSize + Pow2Size - 1) & ~(Pow2Size - 1);
    return Result;
}

static void UnmapCircularBuffer(u8 *Base, u64 Size, u32 RepCount)
{
    for(u32 RepIndex = 0; RepIndex < RepCount; ++RepIndex)
    {
        UnmapViewOfFile(Base + RepIndex*Size);
    }
}

static void DeallocateCircularBuffer(circular_buffer *Buffer)
{
    if(Buffer)
    {
        if(Buffer->FileMapping != INVALID_HANDLE_VALUE)
        {
            UnmapCircularBuffer(Buffer->Base.Data, Buffer->Base.Count, Buffer->RepCount);
            CloseHandle(Buffer->FileMapping);
        }
        
        *Buffer = {};
    }
}

static circular_buffer AllocateCircularBuffer(u64 MinimumSize, u32 RepCount)
{
    circular_buffer Result = {};
    
    // NOTE(casey): Allocation size has to be aligned to the allocation granularity otherwise the back-to-back buffer mapping might not work.
    SYSTEM_INFO Info;
    GetSystemInfo(&Info);
    u64 DataSize = RoundToPow2Size(MinimumSize, Info.dwAllocationGranularity);
    u64 TotalRepeatedSize = RepCount * DataSize;
    
    Result.FileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, 0, PAGE_READWRITE,
                                           (DWORD)(DataSize >> 32), (DWORD)(DataSize & 0xffffffff), 0);
    Result.RepCount = RepCount;
    
    if(Result.FileMapping != INVALID_HANDLE_VALUE)
    {
        // NOTE(casey): If you were worried about overhead for calling this function, this setup can be
        // cached. However, it's unlikely to matter because this is a heavyweight allocation to begin with,
        // so it will never be particularly fast to set up either way.
        HMODULE Kernel = LoadLibraryW(KERNEL_MEMORY_DLL_PATH);
        virtual_alloc_2 *VirtualAlloc2 = (virtual_alloc_2 *)GetProcAddress(Kernel, "VirtualAlloc2");
        map_view_of_file_3 *MapViewOfFile3 = (map_view_of_file_3 *)GetProcAddress(Kernel, "MapViewOfFile3");

        if(VirtualAlloc2 && MapViewOfFile3)
        {
            // NOTE(casey): On up-to-date versions of Windows, we can allocate ring buffers directly without
            // needing to hunt for addresses by using "placeholders".
            
            u8 *BasePtr = (u8 *)VirtualAlloc2(0, 0, TotalRepeatedSize, MEM_RESERVE|MEM_RESERVE_PLACEHOLDER, PAGE_NOACCESS, 0, 0);
            
            b32 Mapped = true;
            for(u32 RepIndex = 0; RepIndex < RepCount; ++RepIndex)
            {
                VirtualFree(BasePtr + RepIndex*DataSize, DataSize, MEM_RELEASE|MEM_PRESERVE_PLACEHOLDER);
                if(!MapViewOfFile3(Result.FileMapping, 0, BasePtr + RepIndex*DataSize,
                                   0, DataSize, MEM_REPLACE_PLACEHOLDER, PAGE_READWRITE, 0, 0))
                {
                    Mapped = false;
                }
            }
            
            if(Mapped)
            {
                Result.Base.Data = BasePtr;
                Result.Base.Count = DataSize;
            }
        }
        else
        {
            // NOTE(casey): On older versions of Windows, we can still allocate ring buffers, but we have
            // to try multiple times because we cannot guarantee that someone won't use the address range
            // in between the allocation test and the mappings.
            
            for(u32 AttemptIndex = 0; AttemptIndex < 100; ++AttemptIndex)
            {
                u8 *BasePtr = (u8 *)VirtualAlloc(0, TotalRepeatedSize, MEM_RESERVE, PAGE_NOACCESS);
                if(BasePtr)
                {
                    VirtualFree(BasePtr, 0, MEM_RELEASE);
                
                    b32 Mapped = true;
                    for(u32 RepIndex = 0; RepIndex < RepCount; ++RepIndex)
                    {
                        if(!MapViewOfFileEx(Result.FileMapping, FILE_MAP_ALL_ACCESS, 0, 0, DataSize, BasePtr + RepIndex*DataSize))
                        {
                            Mapped = false;
                            break;
                        }
                    }
                    
                    if(Mapped)
                    {
                        Result.Base.Data = BasePtr;
                        Result.Base.Count = DataSize;
                        break;
                    }
                    else
                    {
                        UnmapCircularBuffer(BasePtr, DataSize, RepCount);
                    }
                }
            }
        }
    }
    
    if(!Result.Base.Data)
    {
        DeallocateCircularBuffer(&Result);
    }
    
    return Result;
}

int main(void)
{
    printf("Circular buffer test:\n");
    
    circular_buffer Circular = AllocateCircularBuffer(64*4096, 3);
    if(IsValid(Circular))
    {
        s32 Size = (s32)Circular.Base.Count;
        u8 *Data = Circular.Base.Data + Size;
        Data[0] = 123;
        
        printf("  [%8d]: %u\n", -Size, Data[-Size]);
        printf("  [%8d]: %u\n", 0, Data[0]);
        printf("  [%8d]: %u\n", Size, Data[Size]);
        
        DeallocateCircularBuffer(&Circular);
    }
    else
    {
        printf("  FAILED\n");
    }
    
    // NOTE(casey): Since we do not use these functions in this particular build, we reference their pointers
    // here to prevent the compiler from complaining about "unused functions".
    (void)&IsInBounds;
    (void)&AreEqual;
    (void)&AllocateBuffer;
    (void)&FreeBuffer;
    
    return 0;
}
