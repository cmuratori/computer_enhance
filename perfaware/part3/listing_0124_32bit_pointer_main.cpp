/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 124
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

#define KERNEL_MEMORY_DLL_PATH L"kernelbase.dll"

//
// NOTE(casey): The following definitions are required if you do not have an up-to-date version of the Windows SDK
//
#ifndef MEM_REPLACE_PLACEHOLDER

#define MemExtendedParameterAddressRequirements 1

struct MEM_EXTENDED_PARAMETER
{
    DWORD64 Type;
    union
    {
        DWORD64 ULong64;
        PVOID Pointer;
        SIZE_T Size;
        HANDLE Handle;
        DWORD ULong;
    };
};

struct MEM_ADDRESS_REQUIREMENTS
{
    PVOID LowestStartingAddress;
    PVOID HighestEndingAddress;
    SIZE_T Alignment;
};

#endif

typedef PVOID virtual_alloc_2(HANDLE, PVOID, SIZE_T, ULONG, ULONG, MEM_EXTENDED_PARAMETER *, ULONG);

template<class type> struct p32
{
    operator p32<void>() {p32<void> Result = {Value}; return Result;}
    operator type *() {return (type *)(u64)Value;}
    type *operator->() {return (type *)(u64)Value;}
    u32 Value;
};

template<class type> p32<type> Cast32BP(p32<void> Input)
{
    p32<type> Result = {};
    
    Result.Value = Input.Value;
    
    return Result;
}

static p32<void> Allocate32BPCompatible(u32 MinimumSize)
{
    p32<void> Result = {};
    
    HMODULE Kernel = LoadLibraryW(KERNEL_MEMORY_DLL_PATH);
    virtual_alloc_2 *VirtualAlloc2 = (virtual_alloc_2 *)GetProcAddress(Kernel, "VirtualAlloc2");
    if(VirtualAlloc2)
    {
        MEM_ADDRESS_REQUIREMENTS Addr = {};
        Addr.LowestStartingAddress = 0;
        Addr.HighestEndingAddress = (PVOID)0xffffffffull;
        Addr.Alignment = 0;

        MEM_EXTENDED_PARAMETER Param = {};
        Param.Type = MemExtendedParameterAddressRequirements;
        Param.Pointer = &Addr;
        
        Result.Value = (u32)(u64)VirtualAlloc2(0, 0, MinimumSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE, &Param, 1);
    }
    else
    {
        /* NOTE(casey): I've included this code for reference, just to show that it can work without
           the new APIs. However, it is not code I would ever consider shipping. It just hunts for addresses
           under 4GB to try to allocate, and makes no attempt to intelligently map that space or track what it has
           already used. So it is unnecessarily quite slow, and is very limited in that it has a much
           larger granularity than the Windows' own 64k restriction.
        */
        for(u32 BasePage = 64; !Result.Value && (BasePage < 65536); BasePage += 64)
        {
            Result.Value = (u32)(u64)VirtualAlloc((PVOID)(u64)(BasePage*64*1024), MinimumSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
        }
    }
    
    return Result;
}

static void Deallocate32BCompatible(p32<void> Memory)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

int main(void)
{
    printf("\n32-bit pointer test:\n");
    
    p32<u32> SmallPointer = Cast32BP<u32>(Allocate32BPCompatible(256*1024));
    if(SmallPointer)
    {
        SmallPointer[0] = 123;
        SmallPointer[1] = 256;
        
        u32 *RegularPointer = SmallPointer;
        printf(" Ptr: %llu bytes (%llu bits)\n", sizeof(SmallPointer), 8*sizeof(SmallPointer));
        printf("   %u: %u\n", 0, RegularPointer[0]);
        printf("   %u: %u\n", 1, RegularPointer[1]);
        
        Deallocate32BCompatible(SmallPointer);
    }
    else
    {
        printf("  FAILED\n");
    }
    
    return 0;
}
        