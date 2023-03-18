/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#include "sim86.h"

/* NOTE(casey): _CRT_SECURE_NO_WARNINGS is here because otherwise we cannot
   call fopen(). If we replace fopen() with fopen_s() to avoid the warning,
   then the code doesn't compile on Linux anymore, since fopen_s() does not
   exist there.
   
   What exactly the CRT maintainers were thinking when they made this choice,
   I have no idea.
*/
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "sim86_instruction.h"
#include "sim86_instruction_table.h"
#include "sim86_memory.h"
#include "sim86_text.h"
#include "sim86_decode.h"

#include "sim86_instruction.cpp"
#include "sim86_instruction_table.cpp"
#include "sim86_memory.cpp"
#include "sim86_text.cpp"
#include "sim86_decode.cpp"

static u32 LoadMemoryFromFile(char *FileName, segmented_access SegMem, u32 AtOffset)
{
    u32 Result = 0;
    
    // NOTE(casey): Because we are simulating a machine, we only attempt to load as
    // much of a file as will fit into that machine's memory. 
    // Any additional bytes are discarded.
    u32 BaseAddress = GetAbsoluteAddressOf(SegMem, AtOffset);
    u32 HighAddress = GetHighestAddress(SegMem);
    u32 MaxBytes = (HighAddress - BaseAddress) + 1;
    
    FILE *File = fopen(FileName, "rb");
    if(File)
    {
        Result = fread(SegMem.Memory + BaseAddress, 1, MaxBytes, File);
        fclose(File);
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to open %s.\n", FileName);
    }
    
    return Result;
}

static segmented_access AllocateMemoryPow2(u32 SizePow2)
{
    static u8 FailedAllocationByte;
    
    u8 *Memory = (u8 *)malloc(1 << SizePow2);
    if(!Memory)
    {
        SizePow2 = 0;
        Memory = &FailedAllocationByte;
    }
    
    segmented_access Result = FixedMemoryPow2(SizePow2, Memory);
    return Result;
}

static void DisAsm8086(u32 DisAsmByteCount, segmented_access DisAsmStart)
{
    segmented_access At = DisAsmStart;
    
    instruction_table Table = Get8086InstructionTable();
    
    u32 Count = DisAsmByteCount;
    while(Count)
    {
        instruction Instruction = DecodeInstruction(Table, At);
        if(Instruction.Op)
        {
            if(Count >= Instruction.Size)
            {
                At = MoveBaseBy(At, Instruction.Size);
                Count -= Instruction.Size;
            }
            else
            {
                fprintf(stderr, "ERROR: Instruction extends outside disassembly region\n");
                break;
            }
            
            PrintInstruction(Instruction, stdout);
            printf("\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }
}

int main(int ArgCount, char **Args)
{
    segmented_access MainMemory = AllocateMemoryPow2(20);
    if(IsValid(MainMemory))
    {
        if(ArgCount > 1)
        {
            for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
            {
                char *FileName = Args[ArgIndex];
                u32 BytesRead = LoadMemoryFromFile(FileName, MainMemory, 0);
                
                printf("; %s disassembly:\n", FileName);
                printf("bits 16\n");
                DisAsm8086(BytesRead, MainMemory);
            }
        }
        else
        {
            fprintf(stderr, "USAGE: %s [8086 machine code file] ...\n", Args[0]);
        }
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to allow main memory for 8086.\n");
    }
    
    return 0;
}
