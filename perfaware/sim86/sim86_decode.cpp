/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

struct decode_context
{
    u32 DefaultSegment;
    u32 AdditionalFlags;
};

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
static u32 ParseDataValue(segmented_access *Access, b32 Exists, b32 Wide, b32 SignExtended)
{
    u32 Result = {};
    
    if(Exists)
    {
        if(Wide)
        {
            u8 D0 = *AccessMemory(*Access, 0);
            u8 D1 = *AccessMemory(*Access, 1);
            Result = (D1 << 8) | D0;
            Access->SegmentOffset += 2;
        }
        else
        {
            Result = *AccessMemory(*Access);
            if(SignExtended)
            {
                Result = (s32)*(s8 *)&Result;
            }
            Access->SegmentOffset += 1;
        }
    }
    
    return Result;
}

static instruction TryDecode(decode_context *Context, instruction_encoding *Inst, segmented_access At)
{
    instruction Dest = {};
    b32 Has[Bits_Count] = {};
    u32 Bits[Bits_Count] = {};
    b32 Valid = true;
    
    u64 StartingAddress = GetAbsoluteAddressOf(At);
    
    u8 BitsPendingCount = 0;
    u8 BitsPending = 0;
    for(u32 BitsIndex = 0; Valid && (BitsIndex < ArrayCount(Inst->Bits)); ++BitsIndex)
    {
        instruction_bits TestBits = Inst->Bits[BitsIndex];
        if(TestBits.Usage == Bits_End)
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
                BitsPending = *AccessMemory(At);
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
            Has[TestBits.Usage] = true;
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
        Has[Bits_Disp] = ((Has[Bits_Disp]) || (Mod == 0b10) || (Mod == 0b01) || HasDirectAddress);

        b32 DisplacementIsW = ((Bits[Bits_DispAlwaysW]) || (Mod == 0b10) || HasDirectAddress);
        b32 DataIsW = ((Bits[Bits_WMakesDataW]) && !S && W);
        
        Bits[Bits_Disp] |= ParseDataValue(&At, Has[Bits_Disp], DisplacementIsW, (!DisplacementIsW));
        Bits[Bits_Data] |= ParseDataValue(&At, Has[Bits_Data], DataIsW, S);
        
        Dest.Op = Inst->Op;
        Dest.Flags = Context->AdditionalFlags;
        Dest.Address = StartingAddress;
        Dest.Size = GetAbsoluteAddressOf(At) - StartingAddress;
        Dest.SegmentOverride = Context->DefaultSegment;
        
        if(W)
        {
            Dest.Flags |= Inst_Wide;
        }

        if(Bits[Bits_Far])
        {
            Dest.Flags |= Inst_Far;
        }
        
        if(Bits[Bits_Z])
        {
            Dest.Flags |= Inst_RepNE;
        }
        
        u32 Disp = Bits[Bits_Disp];
        s16 Displacement = (s16)Disp;
        
        instruction_operand *RegOperand = &Dest.Operands[D ? 0 : 1];
        instruction_operand *ModOperand = &Dest.Operands[D ? 1 : 0];
        
        if(Has[Bits_SR])
        {
            *RegOperand = RegisterOperand(Register_es + (Bits[Bits_SR] & 0x3), 2);
        }
        
        if(Has[Bits_REG])
        {
            *RegOperand = GetRegOperand(Bits[Bits_REG], W);
        }
        
        if(Has[Bits_MOD])
        {
            if(Mod == 0b11)
            {
                *ModOperand = GetRegOperand(RM, W || (Bits[Bits_RMRegAlwaysW]));
            }
            else
            {
                register_mapping_8086 IntelTerm0[8] = { Register_b,  Register_b, Register_bp, Register_bp, Register_si, Register_di, Register_bp, Register_b};
                register_mapping_8086 IntelTerm1[8] = {Register_si, Register_di, Register_si, Register_di};
                
                u32 I = RM&0x7;
                register_mapping_8086 Term0 = IntelTerm0[I];
                register_mapping_8086 Term1 = IntelTerm1[I];
                if((Mod == 0b00) && (RM == 0b110))
                {
                    Term0 = {};
                    Term1 = {};
                }
                
                *ModOperand = EffectiveAddressOperand(RegisterAccess(Term0, 0, 2), RegisterAccess(Term1, 0, 2), Displacement);
            }
        }
        
        if(Has[Bits_Data] && Has[Bits_Disp] && !Has[Bits_MOD])
        {
            Dest.Operands[0] = IntersegmentAddressOperand(Bits[Bits_Data], Bits[Bits_Disp]);
        }
        else
        {
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
                *LastOperand = ImmediateOperand(Displacement, Immediate_RelativeJumpDisplacement);
            }
            else if(Has[Bits_Data])
            {
                *LastOperand = ImmediateOperand(Bits[Bits_Data]);
            }
            else if(Has[Bits_V])
            {
                if(Bits[Bits_V])
                {
                    *LastOperand = RegisterOperand(Register_c, 1);
                }
                else
                {
                    *LastOperand = ImmediateOperand(1);
                }
            }
        }
    }
    
    return Dest;
}

static instruction DecodeInstruction(instruction_table Table, segmented_access At)
{
    /* TODO(casey): Hmm. It seems like this is a very inefficient way to parse
       instructions, isn't it? For every instruction, we check every entry in the
       table until we find a match. Is this bad design? Or did the person who wrote
       it know what they were doing, and has a plan for how it can be optimized
       later? Only time will tell... :) */
    
    decode_context Context = {};
    instruction Result = {};
    
    u32 StartingAddress = GetAbsoluteAddressOf(At);
    u32 TotalSize = 0;
    while(TotalSize < Table.MaxInstructionByteCount)
    {
        Result = {};
        for(u32 Index = 0; Index < Table.EncodingCount; ++Index)
        {
            instruction_encoding Inst = Table.Encodings[Index];
            Result = TryDecode(&Context, &Inst, At);
            if(Result.Op)
            {
                At.SegmentOffset += Result.Size;
                TotalSize += Result.Size;
                break;
            }
        }
        
        if(Result.Op == Op_lock)
        {
            Context.AdditionalFlags |= Inst_Lock;
        }
        else if(Result.Op == Op_rep)
        {
            Context.AdditionalFlags |= Inst_Rep | (Result.Flags & Inst_RepNE);
        }
        else if(Result.Op == Op_segment)
        {
            Context.AdditionalFlags |= Inst_Segment;
            Context.DefaultSegment = Result.Operands[1].Register.Index;
        }
        else
        {
            break;
        }
    }

    if(TotalSize <= Table.MaxInstructionByteCount)
    {
        Result.Address = StartingAddress;
        Result.Size = TotalSize;
    }
    else
    {
        Result = {};
    }
    
    return Result;
}
