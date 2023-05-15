/* ========================================================================
   $File: work/tools/sim86/sim86_text_table.cpp $
   $Date: 2023/04/28 23:24:55 UTC $
   $Revision: 1 $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

char const *OpcodeMnemonics[] =
{
    "",

#define INST(Mnemonic, ...) #Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
};

static char const *GetMnemonic(operation_type Op)
{
    char const *Result = "";
    if(Op < Op_Count)
    {
        Result = OpcodeMnemonics[Op];
    }
    
    return Result;
}

static char const *GetRegName(register_access Reg)
{
    char const *Names[][3] =
    {
        {"", "", ""},
        {"al", "ah", "ax"},
        {"bl", "bh", "bx"},
        {"cl", "ch", "cx"},
        {"dl", "dh", "dx"},
        {"sp", "sp", "sp"},
        {"bp", "bp", "bp"},
        {"si", "si", "si"},
        {"di", "di", "di"},
        {"es", "es", "es"},
        {"cs", "cs", "cs"},
        {"ss", "ss", "ss"},
        {"ds", "ds", "ds"},
        {"ip", "ip", "ip"},
        {"flags", "flags", "flags"},
        {"", "", ""}
    };
    
    char const *Result = Names[Reg.Index % ArrayCount(Names)][(Reg.Count == 2) ? 2 : Reg.Offset&1];
    return Result;
}
