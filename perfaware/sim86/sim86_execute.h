/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

enum flags_register_bit
{
    Flag_CF = (1 <<  0), // NOTE(casey): Carry
    Flag_PF = (1 <<  2), // NOTE(casey): Parity
    Flag_AF = (1 <<  4), // NOTE(casey): Aux carry
    Flag_ZF = (1 <<  6), // NOTE(casey): Zero
    Flag_SF = (1 <<  7), // NOTE(casey): Sign
    Flag_TF = (1 <<  8), // NOTE(casey): Trap
    Flag_IF = (1 <<  9), // NOTE(casey): Interrupt
    Flag_DF = (1 << 10), // NOTE(casey): Direction
    Flag_OF = (1 << 11), // NOTE(casey): Overflow
};

#define FLAG_MASK_8086 (Flag_CF | Flag_PF | Flag_AF | Flag_ZF | Flag_SF | Flag_TF | Flag_IF | Flag_DF | Flag_OF)

// NOTE(casey): These are the flags that were in the 8080 (necessary to know for some instructions):
#define FLAG_MASK_OLD_8080 (Flag_CF | Flag_PF | Flag_AF | Flag_ZF | Flag_SF)

union register_state_8086
{
#define REG_16(i) union {struct{u8 i##l; u8 i##h;}; u16 i##x;}
    
    struct 
    {
        u16 Zero;
        
        REG_16(a);
        REG_16(b);
        REG_16(c);
        REG_16(d);
        u16 sp;
        u16 bp;
        u16 si;
        u16 di;
        u16 es;
        u16 cs;
        u16 ss;
        u16 ds;
        u16 ip;
        u16 flags;
    };
    
    u8 u8[Register_count][2];
    u16 u16[Register_count];
    
#undef REG_16
};
#define FLAGS_REGISTER_8086 14
static_assert((sizeof(register_state_8086) / sizeof(u16)) == Register_count, "Mismatched register sizes");

struct exec_result
{
    u32 ShiftCount;
    u32 RepCount;
    b32 BranchTaken;
    b32 AddressIsUnaligned;
    b32 Unimplemented;
};

struct operand_access
{
    segmented_access Op;
    u32 Val;
    b32 AddressIsUnaligned;
};

static exec_result ExecInstruction(segmented_access Memory, register_state_8086 *Registers, instruction Instruction);
