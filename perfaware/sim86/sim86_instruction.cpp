/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static b32 OperandIsType(instruction Instruction, u32 Index, operand_type Type)
{
    b32 Result = (Instruction.Operands[Index].Type == Type);
    return Result;
}

static instruction_operand GetOperand(instruction Instruction, u32 Index)
{
    assert(Index < ArrayCount(Instruction.Operands));
    instruction_operand Result = Instruction.Operands[Index];
    return Result;
}

static register_access RegisterAccess(u32 Index, u32 Offset, u32 Count)
{
    register_access Result = {};
    
    Result.Index = Index;
    Result.Offset = Offset;
    Result.Count = Count;
    
    return Result;
}

static instruction_operand IntersegmentAddressOperand(u32 Segment, s32 Displacement)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Memory;
    Result.Address.ExplicitSegment = Segment;
    Result.Address.Displacement = Displacement;
    Result.Address.Flags = Address_ExplicitSegment;
    
    return Result;
}

static instruction_operand EffectiveAddressOperand(register_access Term0, register_access Term1, s32 Displacement)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Memory;
    Result.Address.Terms[0].Register = Term0;
    Result.Address.Terms[0].Scale = 1;
    Result.Address.Terms[1].Register = Term1;
    Result.Address.Terms[1].Scale = 1;
    Result.Address.Displacement = Displacement;
    
    return Result;
}

static instruction_operand RegisterOperand(u32 Index, u32 Count)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Register;
    Result.Register.Index = Index;
    Result.Register.Offset = 0;
    Result.Register.Count = Count;
    
    return Result;
}

static instruction_operand ImmediateOperand(u32 Value, u32 Flags = 0)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Immediate;
    Result.Immediate.Value = Value;
    Result.Immediate.Flags = Flags;
    
    return Result;
}
