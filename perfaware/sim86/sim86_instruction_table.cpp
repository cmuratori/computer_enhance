/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static instruction_encoding InstructionTable8086[] =
{
#include "sim86_instruction_table.inl"
};

char const *OpcodeMnemonics[] =
{
    "",

#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

static instruction_table Get8086InstructionTable()
{
    instruction_table Result = {};
    
    Result.EncodingCount = ArrayCount(InstructionTable8086);
    Result.Encodings = InstructionTable8086;
    
    return Result;
}

static char const *GetMnemonic(operation_type Op)
{
    char const *Result = OpcodeMnemonics[Op];
    return Result;
}
