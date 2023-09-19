/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 117
   ======================================================================== */

struct decomposed_virtual_address
{
    u16 PML4Index;
    u16 DirectoryPtrIndex;
    u16 DirectoryIndex;
    u16 TableIndex;
    u32 Offset;
};

static void Print(decomposed_virtual_address Address)
{
    printf("|%3u|%3u|%3u|%3u|%10u|",
           Address.PML4Index, Address.DirectoryPtrIndex,
           Address.DirectoryIndex, Address.TableIndex,
           Address.Offset);
}

static void PrintAsLine(char const *Label, decomposed_virtual_address Address)
{
    printf("%s", Label);
    Print(Address);
    printf("\n");
}

static decomposed_virtual_address DecomposePointer4K(void *Ptr)
{
    decomposed_virtual_address Result = {};
    
    u64 Address = (u64)Ptr;
    Result.PML4Index = ((Address >> 39) & 0x1ff);
    Result.DirectoryPtrIndex = ((Address >> 30) & 0x1ff);
    Result.DirectoryIndex = ((Address >> 21) & 0x1ff);
    Result.TableIndex = ((Address >> 12) & 0x1ff);
    Result.Offset = ((Address >> 0) & 0xfff);
    
    return Result;
}

static decomposed_virtual_address DecomposePointer2MB(void *Ptr)
{
    decomposed_virtual_address Result = {};
    
    u64 Address = (u64)Ptr;
    Result.PML4Index = ((Address >> 39) & 0x1ff);
    Result.DirectoryPtrIndex = ((Address >> 30) & 0x1ff);
    Result.DirectoryIndex = ((Address >> 21) & 0x1ff);
    Result.Offset = ((Address >> 0) & 0x1fffff);
    
    return Result;
}

static decomposed_virtual_address DecomposePointer1GB(void *Ptr)
{
    decomposed_virtual_address Result = {};
    
    u64 Address = (u64)Ptr;
    Result.PML4Index = ((Address >> 39) & 0x1ff);
    Result.DirectoryPtrIndex = ((Address >> 30) & 0x1ff);
    Result.Offset = ((Address >> 0) & 0x3fffffff);
    
    return Result;
}
