/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

typedef u32 register_index;

// NOTE(casey): To make it easier to compile with languages which do not
// have auto-typedef'ing (like C, etc.), all types are manually typedef'd here.
typedef struct register_access register_access;
typedef struct effective_address_term effective_address_term;
typedef struct effective_address_expression effective_address_expression;
typedef struct immediate immediate;
typedef struct instruction_operand instruction_operand;
typedef struct instruction instruction;

typedef enum operation_type : u32
{
    Op_None,

#define INST(Mnemonic, ...) Op_##Mnemonic,
#define INSTALT(...)
#include "sim86_instruction_table.inl"
    
    Op_Count,
} operation_type;

enum instruction_flag
{
    Inst_Lock = 0x1,
    Inst_Rep = 0x2,
    Inst_Segment = 0x4,
    Inst_Wide = 0x8,
    Inst_Far = 0x10,
    Inst_RepNE = 0x20, // NOTE(casey): For user convenience, this will be set _in addition to_ Inst_Rep for REPNE/REPNZ
};

struct register_access
{
    register_index Index;
    u32 Offset;
    u32 Count;
};

struct effective_address_term
{
    register_access Register;
    s32 Scale;
};

enum effective_address_flag
{
    Address_ExplicitSegment = 0x1,
};
struct effective_address_expression
{
    effective_address_term Terms[2];
    u32 ExplicitSegment;
    s32 Displacement;
    u32 Flags;
};

enum immediate_flag
{
    Immediate_RelativeJumpDisplacement = 0x1,
};
struct immediate
{
    s32 Value;
    u32 Flags;
};

typedef enum operand_type : u32
{
    Operand_None,
    Operand_Register,
    Operand_Memory,
    Operand_Immediate,
} operand_type;
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
    
    register_index SegmentOverride;
};
