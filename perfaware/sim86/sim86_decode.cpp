/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

instruction_format InstructionFormats[] =
{
#include "sim86_instruction_table.inl"
};

static disasm_context DefaultDisAsmContext(void)
{
    disasm_context Result = {};
    
    Result.DefaultSegment = Register_ds;
    
    return Result;
}

static instruction_operand GetRegOperand(u32 IntelRegIndex, b32 Wide)
{
    // NOTE(casey): This maps Intel's REG and RM field encodings for registers to our encoding for registers.
    register_access RegTable[][2] =
    {
        {{Register_a, 0, 1}, {Register_a, 0, 2}},
        {{Register_c, 0, 1}, {Register_c, 0, 2}},
        {{Register_d, 0, 1}, {Register_d, 0, 2}},
        {{Register_b, 0, 1}, {Register_b, 0, 2}},
        {{Register_a, 1, 1}, {Register_sp, 0, 2}},
        {{Register_c, 1, 1}, {Register_bp, 0, 2}},
        {{Register_d, 1, 1}, {Register_si, 0, 2}},
        {{Register_b, 1, 1}, {Register_di, 0, 2}},
    };
    
    instruction_operand Result = {};
    Result.Type = Operand_Register;
    Result.Register = RegTable[IntelRegIndex & 0x7][(Wide != 0)];
    
    return Result;
}

// NOTE(casey): ParseDataValue is not a real function, it's basically just a macro that is used in
// TryParse. It should never be called otherwise, but that is not something you can do in C++.
// In other languages it would be a "local function".
static u32 ParseDataValue(memory *Memory, segmented_access *Access, b32 Exists, b32 Wide, b32 SignExtended)
{
    u32 Result = {};
    
    if(Exists)
    {
        if(Wide)
        {
            u8 D0 = ReadMemory(Memory, GetAbsoluteAddressOf(*Access, 0));
            u8 D1 = ReadMemory(Memory, GetAbsoluteAddressOf(*Access, 1));
            Result = (D1 << 8) | D0;
            Access->SegmentOffset += 2;
        }
        else
        {
            Result = ReadMemory(Memory, GetAbsoluteAddressOf(*Access));
            if(SignExtended)
            {
                Result = (s32)*(s8 *)&Result;
            }
            Access->SegmentOffset += 1;
        }
    }
    
    return Result;
}

static instruction TryDecode(disasm_context *Context, instruction_format *Inst, memory *Memory, segmented_access At)
{
    instruction Dest = {};
    u32 HasBits = 0;
    u32 Bits[Bits_Count] = {};
    b32 Valid = true;
    
    u32 StartingAddress = GetAbsoluteAddressOf(At);
    
    u8 BitsPendingCount = 0;
    u8 BitsPending = 0;
    for(u32 BitsIndex = 0; Valid && (BitsIndex < ArrayCount(Inst->Bits)); ++BitsIndex)
    {
        instruction_bits TestBits = Inst->Bits[BitsIndex];
        if((TestBits.Usage == Bits_Literal) && (TestBits.BitCount == 0))
        {
            // NOTE(casey): That's the end of the instruction format.
            break;
        }
        
        u32 ReadBits = TestBits.Value;
        if(TestBits.BitCount != 0)
        {
            if(BitsPendingCount == 0)
            {
                BitsPendingCount = 8;
                BitsPending = ReadMemory(Memory, GetAbsoluteAddressOf(At));
                ++At.SegmentOffset;
            }
            
            // NOTE(casey): If this assert fires, it means we have an error in our table,
            // since there are no 8086 instructions that have bit values straddling a
            // byte boundary.
            assert(TestBits.BitCount <= BitsPendingCount);
            
            BitsPendingCount -= TestBits.BitCount;
            ReadBits = BitsPending;
            ReadBits >>= BitsPendingCount;
            ReadBits &= ~(0xff << TestBits.BitCount);
        }
        
        if(TestBits.Usage == Bits_Literal)
        {
            // NOTE(casey): This is a "required" sequence
            Valid = Valid && (ReadBits == TestBits.Value);
        }
        else
        {
            Bits[TestBits.Usage] |= (ReadBits << TestBits.Shift);
            HasBits |= (1 << TestBits.Usage);
        }
    }
    
    if(Valid)
    {
        u32 Mod = Bits[Bits_MOD];
        u32 RM = Bits[Bits_RM];
        u32 W = Bits[Bits_W];
        b32 S = Bits[Bits_S];
        b32 D = Bits[Bits_D];
        
        b32 HasDirectAddress = ((Mod == 0b00) && (RM == 0b110));
        b32 HasDisplacement = ((Bits[Bits_HasDisp]) || (Mod == 0b10) || (Mod == 0b01) || HasDirectAddress);
        b32 DisplacementIsW = ((Bits[Bits_DispAlwaysW]) || (Mod == 0b10) || HasDirectAddress);
        b32 DataIsW = ((Bits[Bits_WMakesDataW]) && !S && W);
        
        Bits[Bits_Disp] |= ParseDataValue(Memory, &At, HasDisplacement, DisplacementIsW, (!DisplacementIsW));
        Bits[Bits_Data] |= ParseDataValue(Memory, &At, Bits[Bits_HasData], DataIsW, S);
        
        Dest.Op = Inst->Op;
        Dest.Flags = Context->AdditionalFlags;
        Dest.Address = StartingAddress;
        Dest.Size = GetAbsoluteAddressOf(At) - StartingAddress;
        if(W)
        {
            Dest.Flags |= Inst_Wide;
        }
        
        u32 Disp = Bits[Bits_Disp];
        s16 Displacement = (s16)Disp;
        
        instruction_operand *RegOperand = &Dest.Operands[D ? 0 : 1];
        instruction_operand *ModOperand = &Dest.Operands[D ? 1 : 0];
        
        if(HasBits & (1 << Bits_SR))
        {
            RegOperand->Type = Operand_Register;
            RegOperand->Register.Index = (register_index)(Register_es + (Bits[Bits_SR] & 0x3));
            RegOperand->Register.Count = 2;
        }
        
        if(HasBits & (1 << Bits_REG))
        {
            *RegOperand = GetRegOperand(Bits[Bits_REG], W);
        }
        
        if(HasBits & (1 << Bits_MOD))
        {
            if(Mod == 0b11)
            {
                *ModOperand = GetRegOperand(RM, W || (Bits[Bits_RMRegAlwaysW]));
            }
            else
            {
                ModOperand->Type = Operand_Memory;
                ModOperand->Address.Segment = Context->DefaultSegment;
                ModOperand->Address.Displacement = Displacement;
                
                if((Mod == 0b00) && (RM == 0b110))
                {
                    ModOperand->Address.Base = EffectiveAddress_direct;
                }
                else
                {
                    ModOperand->Address.Base = (effective_address_base)(1+RM);
                }
            }
        }
        
        //
        // NOTE(casey): Because there are some strange opcodes that do things like have an immediate as
        // a _destination_ ("out", for example), I define immediates and other "additional operands" to
        // go in "whatever slot was not used by the reg and mod fields".
        //
        
        instruction_operand *LastOperand = &Dest.Operands[0];
        if(LastOperand->Type)
        {
            LastOperand = &Dest.Operands[1];
        }
        
        if(Bits[Bits_RelJMPDisp])
        {
            LastOperand->Type = Operand_RelativeImmediate;
            LastOperand->ImmediateS32 = Displacement + Dest.Size;
        }
        
        if(Bits[Bits_HasData])
        {
            LastOperand->Type = Operand_Immediate;
            LastOperand->ImmediateU32 = Bits[Bits_Data];
        }
        
        if(HasBits & (1 << Bits_V))
        {
            if(Bits[Bits_V])
            {
                LastOperand->Type = Operand_Register;
                LastOperand->Register.Index = Register_c;
                LastOperand->Register.Offset = 0;
                LastOperand->Register.Count = 1;
            }
            else
            {
                LastOperand->Type = Operand_Immediate;
                LastOperand->ImmediateS32 = 1;
            }
        }
    }
    
    return Dest;
}

static instruction DecodeInstruction(disasm_context *Context, memory *Memory, segmented_access *At)
{
    /* TODO(casey): Hmm. It seems like this is a very inefficient way to parse
       instructions, isn't it? For every instruction, we check every entry in the
       table until we find a match. Is this bad design? Or did the person who wrote
       it know what they were doing, and has a plan for how it can be optimized
       later? Only time will tell... :) */
    
    instruction Result = {};
    for(u32 Index = 0; Index < ArrayCount(InstructionFormats); ++Index)
    {
        instruction_format Inst = InstructionFormats[Index];
        Result = TryDecode(Context, &Inst, Memory, *At);
        if(Result.Op)
        {
            At->SegmentOffset += Result.Size;
            break;
        }
    }
    
    return Result;
}

static void UpdateContext(disasm_context *Context, instruction Instruction)
{
    if(Instruction.Op == Op_lock)
    {
        Context->AdditionalFlags |= Inst_Lock;
    }
    else if(Instruction.Op == Op_rep)
    {
        Context->AdditionalFlags |= Inst_Rep;
    }
    else if(Instruction.Op == Op_segment)
    {
        Context->AdditionalFlags |= Inst_Segment;
        Context->DefaultSegment = Instruction.Operands[1].Register.Index;
    }
    else
    {
        Context->AdditionalFlags = 0;
        Context->DefaultSegment = Register_ds;
    }
}
