/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 122
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

struct address_array
{
    void **Addresses;
    u64 Count;
    u64 PageSize;
};

struct tracked_buffer
{
    buffer Base;
    address_array Results;
};

static b32 IsValid(tracked_buffer Buffer)
{
    b32 Result = (Buffer.Base.Data != 0);
    return Result;
}

static void DeallocateTrackedBuffer(tracked_buffer *Buffer)
{
    if(Buffer)
    {
        if(Buffer->Base.Data) VirtualFree(Buffer->Base.Data, 0, MEM_RELEASE);
        if(Buffer->Results.Addresses) VirtualFree(Buffer->Results.Addresses, 0, MEM_RELEASE);
        *Buffer = {};
    }
}

static tracked_buffer AllocateTrackedBuffer(u64 MinimumSize)
{
    tracked_buffer Result = {};
    
    // NOTE(casey): To make sure we have enough space to store all of the changed pages,
	// we have to ensure we allocate as many entries in our changed address table as there
	// are total pages in the requested buffer size.
    SYSTEM_INFO Info;
    GetSystemInfo(&Info);
    u64 PageCount = ((MinimumSize + Info.dwPageSize - 1) / Info.dwPageSize);
    
    Result.Base.Count = MinimumSize;
    Result.Base.Data = (u8 *)VirtualAlloc(0, MinimumSize, MEM_RESERVE|MEM_COMMIT|MEM_WRITE_WATCH, PAGE_READWRITE);

    Result.Results.Count = PageCount;
    Result.Results.Addresses = (void **)VirtualAlloc(0, PageCount * sizeof(void **), MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    if(Result.Base.Data && Result.Results.Addresses)
    {
        // NOTE(casey): Allocation successful
    }
    else
    {
        DeallocateTrackedBuffer(&Result);
    }
    
    return Result;
}

struct address_array GetAndResetWrittenPages(tracked_buffer *Buffer)
{
    address_array Result = {};
    
    DWORD PageSize = 0;
    ULONG_PTR AddressCount = Buffer->Results.Count;
    if(GetWriteWatch(WRITE_WATCH_FLAG_RESET, Buffer->Base.Data, Buffer->Base.Count,
                     Buffer->Results.Addresses, &AddressCount, &PageSize) == 0)
    {
        Result.Addresses = Buffer->Results.Addresses;
        Result.Count = AddressCount;
        Result.PageSize = PageSize;
    }
    
    return Result;
}

static void PrintAddressArray(address_array Written, u8 *BaseAddress)
{
    for(u64 PageIndex = 0; PageIndex < Written.Count; ++PageIndex)
    {
        printf("  %llu: %llu\n", PageIndex, ((u8 *)Written.Addresses[PageIndex] - BaseAddress) / Written.PageSize);
    }
}

int main(void)
{
    printf("\nTracked buffer test:\n");
    
    tracked_buffer Tracked = AllocateTrackedBuffer(256*4096);
    if(IsValid(Tracked))
    {
        Tracked.Base.Data[15*4096] = 1;
        Tracked.Base.Data[25*4096] = 2;
        Tracked.Base.Data[35*4096] = 3;
        Tracked.Base.Data[45*4096] = 4;
        Tracked.Base.Data[55*4096] = 5;
        
        printf("  --- Pass A ---\n");
        address_array Written = GetAndResetWrittenPages(&Tracked);
        PrintAddressArray(Written, Tracked.Base.Data);
        
        Tracked.Base.Data[11*4096] = 1;
        Tracked.Base.Data[11*4096 + 10] = 2;
        Tracked.Base.Data[22*4096 + 291] = 3;
        Tracked.Base.Data[33*4096 + 382] = 4;
        Tracked.Base.Data[44*4096 + 473] = 5;
        Tracked.Base.Data[55*4096 + 948] = 6;
        
        printf("  --- Pass B ---\n");
        Written = GetAndResetWrittenPages(&Tracked);
        PrintAddressArray(Written, Tracked.Base.Data);
        
        DeallocateTrackedBuffer(&Tracked);
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
