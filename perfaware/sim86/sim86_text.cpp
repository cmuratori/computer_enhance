/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static void PrintEffectiveAddressExpression(effective_address_expression Address, FILE *Dest)
{
    b32 HadTerms = false;
    
    char const *Separator = "";
    for(u32 Index = 0; Index < ArrayCount(Address.Terms); ++Index)
    {
        effective_address_term Term = Address.Terms[Index];
        register_access Reg = Term.Register;
        
        if(Reg.Index)
        {
            fprintf(Dest, "%s", Separator);
            if(Term.Scale != 1)
            {
                fprintf(Dest, "%d*", Term.Scale);
            }
            fprintf(Dest, "%s", GetRegName(Reg));
            Separator = "+";
            
            HadTerms = true;
        }
    }
    
    if(!HadTerms || (Address.Displacement != 0))
    {
        fprintf(Dest, "%+d", Address.Displacement);
    }
}

static void PrintInstruction(instruction Instruction, FILE *Dest)
{
    u32 Flags = Instruction.Flags;
    u32 W = Flags & Inst_Wide;
    
    if(Flags & Inst_Lock)
    {
        if(Instruction.Op == Op_xchg)
        {
            // NOTE(casey): This is just a stupidity for matching assembler expectations.
            instruction_operand Temp = Instruction.Operands[0];
            Instruction.Operands[0] = Instruction.Operands[1];
            Instruction.Operands[1] = Temp;
        }
        fprintf(Dest, "lock ");
    }
    
    char const *MnemonicSuffix = "";
    if(Flags & Inst_Rep)
    {
        u32 Z = Flags & Inst_RepNE;
        fprintf(Dest, "%s ", Z ? "rep" : "repne");
        MnemonicSuffix = W ? "w" : "b";
    }
    
    fprintf(Dest, "%s%s ", GetMnemonic(Instruction.Op), MnemonicSuffix);
    
    char const *Separator = "";
    for(u32 OperandIndex = 0; OperandIndex < ArrayCount(Instruction.Operands); ++OperandIndex)
    {
        instruction_operand Operand = Instruction.Operands[OperandIndex];
        if(Operand.Type != Operand_None)
        {
            fprintf(Dest, "%s", Separator);
            Separator = ", ";
            
            switch(Operand.Type)
            {
                case Operand_None: {} break;
                
                case Operand_Register:
                {
                    fprintf(Dest, "%s", GetRegName(Operand.Register));
                } break;
                
                case Operand_Memory:
                {
                    effective_address_expression Address = Operand.Address;
                    
                    if(Address.Flags & Address_ExplicitSegment)
                    {
                        fprintf(Dest, "%u:%u", Address.ExplicitSegment, Address.Displacement);
                    }
                    else
                    {
                        if(Flags & Inst_Far)
                        {
                            fprintf(Dest, "far ");
                        }
                        
                        if(Instruction.Operands[0].Type != Operand_Register)
                        {
                            fprintf(Dest, "%s ", W ? "word" : "byte");
                        }
                        
                        if(Flags & Inst_Segment)
                        {
                            fprintf(Dest, "%s:", GetRegName({Instruction.SegmentOverride, 0, 2}));
                        }
                        
                        fprintf(Dest, "[");
                        PrintEffectiveAddressExpression(Address, Dest);
                        fprintf(Dest, "]");
                    }
                } break;
                
                case Operand_Immediate:
                {
                    immediate Immediate = Operand.Immediate;
                    if(Immediate.Flags & Immediate_RelativeJumpDisplacement)
                    {
                        fprintf(Dest, "$%+d", Immediate.Value + Instruction.Size);
                    }
                    else
                    {
                        fprintf(Dest, "%d", Immediate.Value);
                    }
                } break;
            }
        }
    }
}

static void PrintFlags(u32 Value, FILE *Dest)
{
    if(Value & Flag_CF) {fprintf(Dest, "C");}
    if(Value & Flag_PF) {fprintf(Dest, "P");}
    if(Value & Flag_AF) {fprintf(Dest, "A");}
    if(Value & Flag_ZF) {fprintf(Dest, "Z");}
    if(Value & Flag_SF) {fprintf(Dest, "S");}
    if(Value & Flag_TF) {fprintf(Dest, "T");}
    if(Value & Flag_IF) {fprintf(Dest, "I");}
    if(Value & Flag_DF) {fprintf(Dest, "D");}
    if(Value & Flag_OF) {fprintf(Dest, "O");}
}

static void PrintRegisters(register_state_8086 *Registers, FILE *Dest)
{
    for(u32 RegIndex = 0; RegIndex < ArrayCount(Registers->u16); ++RegIndex)
    {
        u16 Value = Registers->u16[RegIndex];
        
        register_access Access = {};
        Access.Index = RegIndex;
        Access.Count = 2;
        char const *Name = GetRegName(Access);
        if(Value && *Name)
        {
            fprintf(Dest, "%8s: ", Name);
            if(RegIndex == FLAGS_REGISTER_8086)
            {
                PrintFlags(Value, Dest);
            }
            else
            {
                fprintf(Dest, "0x%04x (%u)", Value, Value);
            }
            fprintf(Dest, "\n");
        }
    }
}

static void PrintRegisterDifference(register_state_8086 *Old, register_state_8086 *New, FILE *Dest)
{
    for(u32 RegIndex = 0; RegIndex < ArrayCount(Old->u16); ++RegIndex)
    {
        u16 OldVal = Old->u16[RegIndex];
        u16 NewVal = New->u16[RegIndex];
        
        register_access Access = {};
        Access.Index = RegIndex;
        Access.Count = 2;
        char const *Name = GetRegName(Access);
        
        if(OldVal != NewVal)
        {
            fprintf(Dest, "%s:", Name);
            if(RegIndex == FLAGS_REGISTER_8086)
            {
                PrintFlags(OldVal, Dest);
                fprintf(Dest, "->");
                PrintFlags(NewVal, Dest);
            }
            else
            {
                fprintf(Dest, "0x%x->0x%x", OldVal, NewVal);
            }
            fprintf(Dest, " ");
        }
    }
}

static void PrintClockInterval(instruction_clock_interval Clocks, FILE *Dest)
{
    if(Clocks.Min != Clocks.Max)
    {
        fprintf(Dest, "[%u,%u]", Clocks.Min, Clocks.Max);
    }
    else
    {
        fprintf(Dest, "%u", Clocks.Min);
    }
}

static void ExplainTiming(instruction_timing Timing, instruction_clock_interval Clocks, FILE *Dest)
{
    if(Timing.Base.Min != Clocks.Min)
    {
        fprintf(Dest, " (");
        PrintClockInterval(Timing.Base, Dest);
        if(Timing.EAClocks)
        {
            fprintf(Dest, " + %uea", Timing.EAClocks);
        }
        
        u32 Penalty = Clocks.Min - (Timing.Base.Min + Timing.EAClocks);
        if(Penalty)
        {
            fprintf(Dest, " + %up", Penalty);
        }
        
        fprintf(Dest, ")");
    }
}
