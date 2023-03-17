/* ========================================================================
   $File: work/tools/sim86/sim86_instruction.h $
   $Date: 2023/03/17 00:50:37 UTC $
   $Revision: 1 $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

enum instruction_flag
{
    Inst_Lock = (1 << 0),
    Inst_Rep = (1 << 1),
    Inst_Segment = (1 << 2),
    Inst_Wide = (1 << 3),
};

enum register_index
{
    Register_none,
    
    Register_a,
    Register_b,
    Register_c,
    Register_d,
    Register_sp,
    Register_bp,
    Register_si,
    Register_di,
    Register_es,
    Register_cs,
    Register_ss,
    Register_ds,
    Register_ip,
    Register_flags,
    
    Register_count,
};

struct register_access
{
    register_index Index;
    u8 Offset;
    u8 Count;
};

// NOTE(casey): effective_address_term is a placeholder in case we someday want to do
// the fancier effective address calculations found in later x86 instruction sets (which
// include things like coefficients on the registers).
struct effective_address_term
{
    register_access Register;
};

struct effective_address_expression
{
    register_index Segment;
    u32 ExplicitSegment;
    
    effective_address_term Terms[2];
    s32 Displacement;
};

struct immediate
{
    s32 Value;
    b32 Relative;
};

enum operand_type
{
    Operand_None,
    Operand_Register,
    Operand_Memory,
    Operand_Immediate,
};

struct instruction_operand
{
    operand_type Type;
    union
    {
        effective_address_expression Address;
        register_access Register;
        immediate Immediate;
    };
};

struct instruction
{
    u32 Address;
    u32 Size;
    
    operation_type Op;
    u32 Flags;
    
    instruction_operand Operands[2];
};

static register_access RegisterAccess(register_index Index, u32 Offset, u32 Count);

static instruction_operand IntersegmentAddressOperand(u32 Segment, s32 Displacement);
static instruction_operand EffectiveAddressOperand(register_index Segment,
                                                   register_access Term0, register_access Term1, s32 Displacement);
static instruction_operand RegisterOperand(register_index Index, u32 Count);
static instruction_operand ImmediateOperand(u32 Value);
static instruction_operand ImmediateOperandRelative(u32 Value);
