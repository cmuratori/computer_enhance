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
#include <string.h>
#include <assert.h>

#include "sim86_instruction.h"
#include "sim86_instruction_table.h"
#include "sim86_memory.h"
#include "sim86_decode.h"
#include "sim86_execute.h"
#include "sim86_cycles.h"
#include "sim86_text.h"

#include "sim86_instruction.cpp"
#include "sim86_instruction_table.cpp"
#include "sim86_memory.cpp"
#include "sim86_decode.cpp"
#include "sim86_execute.cpp"
#include "sim86_cycles.cpp"
#include "sim86_text_table.cpp"
#include "sim86_text.cpp"

enum sim_flags
{
    SimFlag_StopOnRet = 0x1,
    SimFlag_ShowClocks = 0x2,
    SimFlag_DumpMemory = 0x4,
    SimFlag_ExplainClocks = 0x8,
    SimFlag_NoRegisterDiffs = 0x10,
};

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

static void PrintEstimatedClocks(timing_state State, instruction Instruction, u32 SimFlags,
                                 instruction_clock_interval *Accum)
{
    instruction_timing Timing = EstimateInstructionClocks(State, Instruction);
    instruction_clock_interval Clocks = ExpectedClocksFrom(State, Instruction, Timing);
    Accum->Min += Clocks.Min;
    Accum->Max += Clocks.Max;
    
    if(Accum->Min != Accum->Max)
    {
        fprintf(stdout, "Clocks: +[%u,%u] = [%u,%u]", Clocks.Min, Clocks.Max, Accum->Min, Accum->Max);
    }
    else
    {
        fprintf(stdout, "Clocks: +%u = %u", Clocks.Min, Accum->Min);
    }
    
    if(SimFlags & SimFlag_ExplainClocks)
    {
        ExplainTiming(Timing, Clocks, stdout);
    }
}

static void DisAsm8086(u32 DisAsmByteCount, segmented_access DisAsmStart, u32 SimFlags, timing_state Timing)
{
    segmented_access At = DisAsmStart;
    
    instruction_table Table = Get8086InstructionTable();

    // NOTE(casey): When not simulating, assume branches are taken, since that is what most loop conditionals will do
    // and that is what we would normally be timing.
    Timing.AssumeBranchTaken = true;
    instruction_clock_interval TimeAccum = {};
    
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
            if(SimFlags & SimFlag_ShowClocks)
            {
                printf(" ; ");
                PrintEstimatedClocks(Timing, Instruction, SimFlags, &TimeAccum);
            }
            printf("\n");
        }
        else
        {
            fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
            break;
        }
    }
}

static b32 IsRet(operation_type Op)
{
    b32 Result = ((Op == Op_ret) ||
                  (Op == Op_retf));
    return Result;
}

static void Run8086(u32 OnePastLastByte, segmented_access MainMemory, u32 SimFlags, timing_state Timing)
{
    instruction_table Table = Get8086InstructionTable();
    register_state_8086 Registers = {};
    instruction_clock_interval TimeAccum = {};
    
    for(;;)
    {
        segmented_access At = MainMemory;
        At.Mask = 0xffff;
        At.SegmentBase = Registers.cs;
        At.SegmentOffset = Registers.ip;
        
        if(GetAbsoluteAddressOf(At) < OnePastLastByte)
        {
            instruction Instruction = DecodeInstruction(Table, At);
            if(Instruction.Op)
            {
                register_state_8086 PrevRegisters = Registers;
                
                if((SimFlags & SimFlag_StopOnRet) &&
                   IsRet(Instruction.Op))
                {
                    fprintf(stdout, "STOPONRET: Return encountered at address %u.\n", Instruction.Address);
                    break;
                }
                
                Registers.ip += Instruction.Size;
                exec_result Exec = ExecInstruction(MainMemory, &Registers, Instruction);
                
                if(!Exec.Unimplemented)
                {
                    PrintInstruction(Instruction, stdout);
                    printf(" ; ");
                    if(SimFlags & SimFlag_ShowClocks)
                    {
                        UpdateTimingForExec(&Timing, Exec);
                        PrintEstimatedClocks(Timing, Instruction, SimFlags, &TimeAccum);
                        fprintf(stdout, " | ");
                    }
                    if(!(SimFlags & SimFlag_NoRegisterDiffs))
                    {
                        PrintRegisterDifference(&PrevRegisters, &Registers, stdout);
                    }
                    printf("\n");
                }
                else
                {
                    printf("ERROR: Unimplemented instruction (%s).\n", GetMnemonic(Instruction.Op));
                    break;
                }
            }
            else
            {
                fprintf(stderr, "ERROR: Unrecognized binary in instruction stream.\n");
                break;
            }
        }
        else
        {
            break;
        }
    }
    
    printf("\n");
    printf("Final registers:\n");
    PrintRegisters(&Registers, stdout);
    printf("\n");
}

int main(int ArgCount, char **Args)
{
    b32 Execute = false;
    u32 DumpIndex = 0;
    u32 SimFlags = 0;
    
    timing_state Timing = {};
    
    u32 MainMemPow2 = 20;
    u32 MainMemSize = (1 << MainMemPow2);
    segmented_access MainMemory = AllocateMemoryPow2(MainMemPow2);
    if(IsValid(MainMemory))
    {
        if(ArgCount > 1)
        {
            for(int ArgIndex = 1; ArgIndex < ArgCount; ++ArgIndex)
            {
                char *FileName = Args[ArgIndex];
                
                if(strcmp(FileName, "-exec") == 0)
                {
                    Execute = true;
                }
                else if(strcmp(FileName, "-showclocks") == 0)
                {
                    SimFlags |= SimFlag_ShowClocks;
                }
                else if(strcmp(FileName, "-explainclocks") == 0)
                {
                    SimFlags |= SimFlag_ShowClocks|SimFlag_ExplainClocks;
                }
                else if(strcmp(FileName, "-8088") == 0)
                {
                    Timing.Assume8088 = true;
                }
                else if(strcmp(FileName, "-disasm") == 0)
                {
                    Execute = false;
                }
                else if(strcmp(FileName, "-dump") == 0)
                {
                    SimFlags |= SimFlag_DumpMemory;
                }
                else if(strcmp(FileName, "-stoponret") == 0)
                {
                    SimFlags |= SimFlag_StopOnRet;
                }
                else
                {
                    if(SimFlags & SimFlag_ShowClocks)
                    {
                        fprintf(stdout,
                                "\n"
                                "WARNING: Clocks reported by this utility are strictly from the 8086 manual.\n"
                                "They will be inaccurate, both because the manual clocks are estimates, and because\n"
                                "some of the entries in the manual look highly suspicious and are probably typos.\n"
                                "\n");
                    }
                    
                    u32 BytesRead = LoadMemoryFromFile(FileName, MainMemory, 0);
                    if(Execute)
                    {
                        printf("--- %s execution ---\n", FileName);
                        Run8086(BytesRead, MainMemory, SimFlags, Timing);
                    }
                    else
                    {
                        printf("; %s disassembly:\n", FileName);
                        printf("bits 16\n");
                        DisAsm8086(BytesRead, MainMemory, SimFlags, Timing);
                    }
                    
                    if(SimFlags & SimFlag_DumpMemory)
                    {
                        char DumpFileName[256];
                        sprintf(DumpFileName, "sim86_memory_%u.data", DumpIndex);
                        FILE *DumpFile = fopen(DumpFileName, "wb");
                        if(DumpFile)
                        {
                            fwrite(MainMemory.Memory, MainMemSize, 1, DumpFile);
                            fclose(DumpFile);
                        }
                        
                        ++DumpIndex;
                    }
                }
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
