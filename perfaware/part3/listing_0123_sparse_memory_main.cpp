/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 123
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

struct sparse_buffer
{
    buffer Base;
};

static b32 IsValid(sparse_buffer Buffer)
{
    b32 Result = (Buffer.Base.Data != 0);
    return Result;
}

static sparse_buffer AllocateSparseBuffer(u64 Size)
{
    sparse_buffer Result = {};
    
    Result.Base.Data = (u8 *)VirtualAlloc(0, Size, MEM_RESERVE, PAGE_NOACCESS);
    if(Result.Base.Data)
    {
        Result.Base.Count = Size;
    }
    
    return Result;
}

static void DeallocateSparseBuffer(sparse_buffer *Buffer)
{
    if(Buffer)
    {
        VirtualFree(Buffer->Base.Data, 0, MEM_RELEASE);
        *Buffer = {};
    }
}

static void EnsureMemoryIsMapped(sparse_buffer *, void *Pointer, u32 Size)
{
    VirtualAlloc(Pointer, Size, MEM_COMMIT, PAGE_READWRITE);
}

int main(void)
{
    printf("\nSparse memory test:\n");

    u64 Gigabyte = 1024*1024*1024;
    sparse_buffer Sparse = AllocateSparseBuffer(256*Gigabyte);
    if(IsValid(Sparse))
    {
        u8 *Write = Sparse.Base.Data;
        
        u64 Offsets[] = {16*Gigabyte, 100*Gigabyte, 200*Gigabyte, 255*Gigabyte};
        
        for(u32 OffsetIndex = 0; OffsetIndex < ArrayCount(Offsets); ++OffsetIndex)
        {
            u64 Offset = Offsets[OffsetIndex];
            EnsureMemoryIsMapped(&Sparse, Write + Offset, sizeof(*Write));
            Write[Offset] = (u8)(100 + OffsetIndex);
        }
        
        for(u32 OffsetIndex = 0; OffsetIndex < ArrayCount(Offsets); ++OffsetIndex)
        {
            u64 Offset = Offsets[OffsetIndex];
            printf("  %u: %u\n", OffsetIndex, Write[Offset]);
        }
    }
    DeallocateSparseBuffer(&Sparse);

    // NOTE(casey): Since we do not use these functions in this particular build, we reference their pointers
    // here to prevent the compiler from complaining about "unused functions".
    (void)&IsInBounds;
    (void)&AreEqual;
    (void)&AllocateBuffer;
    (void)&FreeBuffer;
		
    return 0;
}
