/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#define assert(...)

#include "sim86.h"

#include "sim86_instruction.h"
#include "sim86_instruction_table.h"
#include "sim86_memory.h"
#include "sim86_decode.h"

#include "sim86_instruction.cpp"
#include "sim86_instruction_table.cpp"
#include "sim86_memory.cpp"
#include "sim86_decode.cpp"
#include "sim86_text_table.cpp"

extern "C" u32 Sim86_GetVersion(void)
{
    u32 Result = SIM86_VERSION;
    return Result;
}

extern "C" void Sim86_Decode8086Instruction(u32 SourceSize, u8 *Source, instruction *Dest)
{
    instruction_table Table = Get8086InstructionTable();
    
    // NOTE(casey): The 8086 decoder requires the ability to read up to 15 bytes (the maximum
    // allowable instruction size)
    assert(Table.MaxInstructionByteCount == 15);
    u8 GuardBuffer[16] = {};
    if(SourceSize < Table.MaxInstructionByteCount)
    {
        // NOTE(casey): I replaced the memcpy here with a manual copy to make it easier for
        // people compiling on things like WebAssembly who do not want to use Emscripten.
        for(u32 I = 0; I < SourceSize; ++I)
        {
            GuardBuffer[I] = Source[I];
        }
        
        Source = GuardBuffer;
    }
    
    segmented_access At = FixedMemoryPow2(4, Source);
    *Dest = DecodeInstruction(Table, At);
}

extern "C" char const *Sim86_RegisterNameFromOperand(register_access *RegAccess)
{
    char const *Result = GetRegName(*RegAccess);
    return Result;
}

extern "C" char const *Sim86_MnemonicFromOperationType(operation_type Type)
{
    char const *Result = GetMnemonic(Type);
    return Result;
}

extern "C" void Sim86_Get8086InstructionTable(instruction_table *Dest)
{
    *Dest = Get8086InstructionTable();
}