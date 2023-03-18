/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

enum instruction_bits_usage : u8
{
    Bits_End, // NOTE(casey): The 0 value, indicating the end of the instruction encoding array
    
    Bits_Literal, // NOTE(casey): These are opcode bits that identify instructions

    // NOTE(casey): These bits correspond directly to the 8086 instruction manual
    Bits_D,
    Bits_S,
    Bits_W,
    Bits_V,
    Bits_Z,
    Bits_MOD,
    Bits_REG,
    Bits_RM,
    Bits_SR,
    Bits_Disp,
    Bits_Data,

    Bits_DispAlwaysW, // NOTE(casey): Tag for instructions where the displacement is always 16 bits
    Bits_WMakesDataW, // NOTE(casey): Tag for instructions where SW=01 makes the data field become 16 bits
    Bits_RMRegAlwaysW, // NOTE(casey): Tag for instructions where the register encoded in RM is always 16-bit width
    Bits_RelJMPDisp, // NOTE(casey): Tag for instructions that require address adjustment to go through NASM properly
    Bits_Far, // NOTE(casey): Tag for instructions that require a "far" keyword in their ASM to select the right opcode
    
    Bits_Count,
};

struct instruction_bits
{
    instruction_bits_usage Usage;
    u8 BitCount;
    u8 Shift;
    u8 Value;
};

struct instruction_encoding
{
    operation_type Op;
    instruction_bits Bits[16];
};

struct instruction_table
{
    instruction_encoding *Encodings;
    u32 EncodingCount;
    u32 MaxInstructionByteCount;
};
