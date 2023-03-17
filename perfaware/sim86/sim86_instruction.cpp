/* ========================================================================
   $File: work/tools/sim86/sim86_instruction.cpp $
   $Date: 2023/03/17 00:49:19 UTC $
   $Revision: 1 $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright by Molly Rocket, Inc., All Rights Reserved. $
   ======================================================================== */

static register_access RegisterAccess(register_index Index, u32 Offset, u32 Count)
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
    
    return Result;
}

static instruction_operand EffectiveAddressOperand(register_index Segment,
                                                   register_access Term0, register_access Term1, s32 Displacement)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Memory;
    Result.Address.Segment = Segment;
    Result.Address.Terms[0].Register = Term0;
    Result.Address.Terms[1].Register = Term1;
    Result.Address.Displacement = Displacement;
    
    return Result;
}

static instruction_operand RegisterOperand(register_index Index, u32 Count)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Register;
    Result.Register.Index = Index;
    Result.Register.Offset = 0;
    Result.Register.Count = Count;
    
    return Result;
}

static instruction_operand ImmediateOperand(u32 Value)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Immediate;
    Result.Immediate.Value = Value;
    
    return Result;
}

static instruction_operand ImmediateOperandRelative(u32 Value)
{
    instruction_operand Result = {};
    
    Result.Type = Operand_Immediate;
    Result.Immediate.Value = Value;
    Result.Immediate.Relative = true;
    
    return Result;
}