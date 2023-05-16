/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static segmented_access SegmentFromRegister(segmented_access Memory, u16 RegisterContents)
{
    assert(Memory.SegmentBase == 0);
    assert(Memory.SegmentOffset == 0);
    segmented_access Result = Memory;
    Result.SegmentBase = RegisterContents;
    return Result;
}

static u8 *GetRegisterPtr(register_state_8086 *Registers, u32 Index)
{
    u8 *Result = Registers->u8[Index % ArrayCount(Registers->u16)];
    return Result;
}

static u8 *GetRegisterPtr(register_state_8086 *Registers, register_access Access)
{
    assert(Access.Offset <= 1);
    assert((Access.Offset + Access.Count) <= 2);
    
    u8 *Result = GetRegisterPtr(Registers, Access.Index) + Access.Offset;
    return Result;
}

static u16 GetRegisterValue(register_state_8086 *Registers, register_access Access)
{
    assert((Access.Count >= 1) && (Access.Count <= 2));
    
    u8 *Reg8 = GetRegisterPtr(Registers, Access);
    u16 *Reg16 = (u16 *)Reg8;
    u16 Result = (Access.Count == 2) ? *Reg16 : *Reg8;
    return Result;
}

static u16 GetRegisterValueU16(register_state_8086 *Registers, u32 Index)
{
    register_access Access = {};
    Access.Count = 2;
    Access.Index = Index;
    
    u16 Result = GetRegisterValue(Registers, Access);
    return Result;
}

static void WriteU8(segmented_access Memory, u16 Offset, u8 Value)
{
    *AccessMemory(Memory, Offset) = Value;
}

static u8 ReadU8(segmented_access Memory, u16 Offset)
{
    u8 Result = *AccessMemory(Memory, Offset);
    return Result;
}

static void WriteU16(segmented_access Memory, u16 Offset, u16 Value)
{
    WriteU8(Memory, Offset + 0, (Value & 0xff));
    WriteU8(Memory, Offset + 1, ((Value >> 8) & 0xff));
}

static u16 ReadU16(segmented_access Memory, u16 Offset)
{
    u16 Result = (u16)ReadU8(Memory, Offset) | ((u16)ReadU8(Memory, Offset + 1) << 8);
    return Result;
}

static void WriteN(segmented_access Memory, u16 Offset, u16 Value, u32 Count)
{
    if(Count == 1)
    {
        WriteU8(Memory, Offset, Value & 0xff);
    }
    else
    {
        WriteU16(Memory, Offset, Value);
    }
}

static void Push(segmented_access Memory, register_state_8086 *Registers, u16 Value)
{
    segmented_access StackSegment = SegmentFromRegister(Memory, Registers->ss);
    
    Registers->sp -= 2;
    WriteU16(StackSegment, Registers->sp, Value);
}

static u16 Pop(segmented_access Memory, register_state_8086 *Registers)
{
    segmented_access StackSegment = SegmentFromRegister(Memory, Registers->ss);

    u16 Result = ReadU16(StackSegment, Registers->sp);
    Registers->sp += 2;
    
    return Result;
}

static void PushFlags(segmented_access Memory, register_state_8086 *Registers)
{
    Push(Memory, Registers, Registers->flags & FLAG_MASK_8086);
}

static void PopFlags(segmented_access Memory, register_state_8086 *Registers)
{
    Registers->flags &= FLAG_MASK_8086;
    Registers->flags |= Pop(Memory, Registers);
}

static u16 SignBitFor(u32 WWidth)
{
    u16 Result = (WWidth == 1) ? (1 << 7) : (1 << 15);
    return Result;
}

static u16 WidthMaskFor(u32 WWidth)
{
    u16 Result = (WWidth == 1) ? 0xff : 0xffff;
    return Result;
}

static u16 ParityFlagOf(u16 x)
{
    // NOTE(casey): Normally you would use a population count instruction for this operation,
    // but the only way to do that in vanilla C++ is to use the std:: library which I don't
    // think is a good idea in general for a variety of reasons. So the parity is computed
    // manually here using Hacker's Delight Second Edition page 96 (I have kept the variable
    // names the same as what appears there):
    
    u16 y = x ^ (x >> 1);
    y = y ^ (y >> 2);
    y = y ^ (y >> 4);
    
    // NOTE(casey): It may seem odd to do only the bottom 8 bits,
    // but for some reason, that is how the 8086 PF is specified.
    // Apparently it was only for backwards compatibility, so it
    // never looks at the high 8 bits.
    
    assert(Flag_PF == (1 << 2));
    return ((~y & 0x1) << 2);
}

static void UpdateCommonFlags(register_state_8086 *Registers, u32 MaskedResult, u32 WWidth)
{
    Registers->flags &= ~(Flag_SF | Flag_ZF | Flag_PF);
    Registers->flags |= (MaskedResult & SignBitFor(WWidth)) ? Flag_SF : 0;
    Registers->flags |= (MaskedResult == 0) ? Flag_ZF : 0;
    Registers->flags |= ParityFlagOf(MaskedResult);
}

static void UpdateArithFlags(register_state_8086 *Registers, u32 UnmaskedResult, u32 MaskedResult, u32 WWidth, b32 OF = false, b32 AF = false)
{
    u32 SignBit = SignBitFor(WWidth);
    b32 CF = (UnmaskedResult & (SignBit << 1));
    
    Registers->flags &= ~(Flag_OF | Flag_CF | Flag_AF);
    Registers->flags |= CF ? Flag_CF : 0;
    Registers->flags |= OF ? Flag_OF : 0;
    Registers->flags |= AF ? Flag_AF : 0;

    UpdateCommonFlags(Registers, MaskedResult, WWidth);
}

static void UpdateLogFlags(register_state_8086 *Registers, u16 MaskedResult, u32 WWidth)
{
    Registers->flags &= ~(Flag_OF | Flag_CF | Flag_AF);
    UpdateCommonFlags(Registers, MaskedResult, WWidth);
}

static void WriteLogOpResult(register_state_8086 *Registers, segmented_access Dest, u16 UnmaskedResult, u32 WWidth)
{
    u16 MaskedResult = UnmaskedResult & WidthMaskFor(WWidth);
    UpdateLogFlags(Registers, MaskedResult, WWidth);
    WriteN(Dest, 0, MaskedResult, WWidth);
}

static void WriteArithOpResult(register_state_8086 *Registers, segmented_access Dest, u32 UnmaskedResult, u32 WWidth,
                               b32 OF = false, b32 AF = false)
{
    /* TODO(casey): I didn't like how this came out. Unlike the other writeback functions, AFAICT this one required
       me to pass in OF and AF since they get computed differently. I assume this is because I don't quite "get" how
       the ALU is supposed to work, and actually it is simpler than that if you know the right approach.
    */
    
    u16 MaskedResult = UnmaskedResult & WidthMaskFor(WWidth);
    UpdateArithFlags(Registers, UnmaskedResult, MaskedResult, WWidth, OF, AF);
    
    WriteN(Dest, 0, MaskedResult, WWidth);
}

static void WriteShiftOpResult(register_state_8086 *Registers, segmented_access Dest, u32 PriorValue, u32 UnmaskedResultS1, u32 WWidth)
{
    u32 UnmaskedResult = (UnmaskedResultS1 >> 1);
    u16 MaskedResult = UnmaskedResult & WidthMaskFor(WWidth);
    u32 SignBit = SignBitFor(WWidth);
    
    b32 CF = (UnmaskedResultS1 & (SignBit << 1)) | (UnmaskedResultS1 & 1);
    b32 OF = ((PriorValue & SignBit) != (UnmaskedResult & SignBit));
    
    Registers->flags |= CF ? Flag_CF : 0;
    Registers->flags |= OF ? Flag_OF : 0;
    
    WriteN(Dest, 0, MaskedResult, WWidth);
}

static void ExecInterrupt(segmented_access Memory, register_state_8086 *Registers, u16 InterruptType)
{
    PushFlags(Memory, Registers);
    Push(Memory, Registers, Registers->cs);
    Push(Memory, Registers, Registers->ip);
    
    Registers->flags &= ~(Flag_TF | Flag_IF);
    
    u16 InterruptAddress = 4*InterruptType;
    
    Registers->ip = ReadU16(Memory, InterruptAddress);
    Registers->cs = ReadU16(Memory, InterruptAddress + 2);
}

static void ConditionalJump(exec_result *Result, register_state_8086 *Registers, s8 Displacement, b32 ShouldJump)
{
    u16 NewIP = (Registers->ip + Displacement);
    Registers->ip = ShouldJump ? NewIP : Registers->ip;
    
    Result->BranchTaken = ShouldJump;
}

static segmented_access DetermineSegmentAccess(segmented_access Memory, instruction Instruction, register_state_8086 *Registers,
                                               u16 DefaultSegRegValue)
{
    u16 DefaultSegmentBase = (Instruction.SegmentOverride) ? GetRegisterValueU16(Registers, Instruction.SegmentOverride) : DefaultSegRegValue;
    segmented_access Result = SegmentFromRegister(Memory, DefaultSegmentBase);
    return Result;
}

static operand_access AccessOperand(segmented_access Memory, register_state_8086 *Registers, instruction Instruction, u32 OperandIndex,
                                    u32 *IgnoredBytes)
{
    operand_access Result = {};
    
    assert(OperandIndex < ArrayCount(Instruction.Operands));
    instruction_operand Source = Instruction.Operands[OperandIndex];
    
    Result.Op.Memory = (u8 *)IgnoredBytes;
    
    switch(Source.Type)
    {
        case Operand_None:
        {
        } break;
        
        case Operand_Register:
        {
            assert(Source.Register.Offset <= 1);
            assert((Source.Register.Count >= 1) && (Source.Register.Count <= 2));
            assert((Source.Register.Offset + Source.Register.Count) <= 2);
            
            Result.Op = FixedMemoryPow2(Source.Register.Count - 1, GetRegisterPtr(Registers, Source.Register));
            Result.Val = GetRegisterValue(Registers, Source.Register);
        } break;
        
        case Operand_Memory:
        {
            Result.Op.Mask = 0xffff;
            Result.Op.SegmentOffset = Source.Address.Displacement;
            
            if(Source.Address.Flags & Address_ExplicitSegment)
            {
                Result.Op.SegmentBase = Source.Address.ExplicitSegment;
            }
            else
            {
                u16 SegReg = (Source.Address.Terms[0].Register.Index == Register_bp) ? Registers->ss : Registers->ds;
                
                Result.Op.Memory = Memory.Memory;
                Result.Op.SegmentBase = DetermineSegmentAccess(Memory, Instruction, Registers, SegReg).SegmentBase;
                for(u32 TermIndex = 0; TermIndex < ArrayCount(Source.Address.Terms); ++TermIndex)
                {
                    effective_address_term Term = Source.Address.Terms[TermIndex];
                    Result.Op.SegmentOffset += Term.Scale*(GetRegisterValue(Registers, Term.Register));
                }
            }
            
            Result.AddressIsUnaligned |= (Result.Op.SegmentOffset & 1);
            Result.Val = ReadU16(Result.Op, 0);
        } break;
        
        case Operand_Immediate:
        {
            Result.Val = Source.Immediate.Value;
        } break;
    }
    
    return Result;
}

static exec_result ExecInstruction(segmented_access Memory, register_state_8086 *Registers, instruction Instruction)
{
    exec_result Result = {};
    
    u32 WWidth = (Instruction.Flags & Inst_Wide) ? 2 : 1;
    b32 IsFar = (Instruction.Flags & Inst_Far);
    
    b32 CF = Registers->flags & Flag_CF;
    b32 PF = Registers->flags & Flag_PF;
    b32 AF = Registers->flags & Flag_AF;
    b32 ZF = Registers->flags & Flag_ZF;
    b32 SF = Registers->flags & Flag_SF;
    b32 OF = Registers->flags & Flag_OF;
    b32 IF = Registers->flags & Flag_IF;
    b32 DF = Registers->flags & Flag_DF;
    b32 TF = Registers->flags & Flag_TF;
    
    segmented_access DefaultSegment = DetermineSegmentAccess(Memory, Instruction, Registers, Registers->ds);
    
    u32 IgnoredBytes = 0;
    operand_access OpAccess[ArrayCount(Instruction.Operands)];
    for(u32 OpIndex = 0; OpIndex < ArrayCount(Instruction.Operands); ++OpIndex)
    {
        OpAccess[OpIndex] = AccessOperand(Memory, Registers, Instruction, OpIndex, &IgnoredBytes);
        Result.AddressIsUnaligned |= OpAccess[OpIndex].AddressIsUnaligned;
    }
    
    segmented_access Op0 = OpAccess[0].Op;
    segmented_access Op1 = OpAccess[1].Op;
    
    u32 V0 = OpAccess[0].Val;
    u32 V1 = OpAccess[1].Val;
    
    switch(Instruction.Op)
    {
        case Op_mov:
        {
            WriteN(Op0, 0, V1, WWidth);
        } break;
        
        case Op_push:
        {
            Push(Memory, Registers, V0);
        } break;
        
        case Op_pop:
        {
            WriteN(Op0, 0, Pop(Memory, Registers), 2);
        } break;
        
        case Op_xchg:
        {
            WriteN(Op0, 0, V1, WWidth);
            WriteN(Op1, 0, V0, WWidth);
        } break;
        
        case Op_xlat:
        {
            // TODO(casey): The description of XLAT in the manual doesn't say whether it uses the
            // segment override or not. I assume here that it does, but if it doesn't, this should
            // be changed to use Registers->ds instead of SegmentBase.
            Registers->al = ReadU8(DefaultSegment, Registers->bx + Registers->al);
        } break;
        
        case Op_in:
        case Op_out:
        {
            // NOTE(casey): Since the rest of the system isn't simulated, there's nothing to do here.
            Result.Unimplemented = true;
        } break;
        
        case Op_lea:
        {
            WriteN(Op0, 0, Op1.SegmentOffset, WWidth);
        } break;
        
        case Op_lds:
        {
            WriteU16(Op0, 0, ReadU16(Op1, 2));
            Registers->ds = ReadU16(Op1, 0);
        } break;
        
        case Op_les:
        {
            WriteU16(Op0, 0, ReadU16(Op1, 2));
            Registers->es = ReadU16(Op1, 0);
        } break;
        
        case Op_lahf:
        {
            Registers->ah = (u8)Registers->flags & FLAG_MASK_OLD_8080;
        } break;
        
        case Op_sahf:
        {
            Registers->flags &= FLAG_MASK_OLD_8080;
            Registers->flags |= (Registers->ah & FLAG_MASK_OLD_8080);
        } break;
        
        case Op_pushf:
        {
            PushFlags(Memory, Registers);
        } break;
        
        case Op_popf:
        {
            PopFlags(Memory, Registers);
        } break;
        
        case Op_add:
        {
            u32 SignBit = SignBitFor(WWidth);
            u32 Mask = WidthMaskFor(WWidth);
            u32 R = (V0 & Mask) + (V1 & Mask);
            b32 OF = (~(V0 ^ V1) & (V0 ^ R)) & SignBit;
            b32 AF = ((V0 & 0xf) + (V1 & 0xf)) & 0x10;
            WriteArithOpResult(Registers, Op0, R, WWidth, OF, AF);
        } break;
        
        case Op_adc:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_inc:
        {
            u32 R = V0 + 1;
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_aaa:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_daa:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_sub:
        {
            u32 SignBit = SignBitFor(WWidth);
            u32 WidthMask = WidthMaskFor(WWidth);
            u32 R = (V0 & WidthMask) - (V1 & WidthMask);
            b32 OF = ((V0 ^ V1) & (V0 ^ R)) & SignBit;
            b32 AF = ((V0 & 0xf) - (V1 & 0xf)) & 0x10;
            WriteArithOpResult(Registers, Op0, R, WWidth, OF, AF);
        } break;
        
        case Op_sbb:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_dec:
        {
            u32 R = V0 - 1;
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_neg:
        {
            u32 R = -V0;
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_cmp:
        {
            u32 SignBit = SignBitFor(WWidth);
            u32 WidthMask = WidthMaskFor(WWidth);
            u32 R = (V0 & WidthMask) - (V1 & WidthMask);
            b32 OF = ((V0 ^ V1) & (V0 ^ R)) & SignBit;
            b32 AF = ((V0 & 0xf) - (V1 & 0xf)) & 0x10;
            UpdateArithFlags(Registers, R, R & WidthMaskFor(WWidth), WWidth, OF, AF);
        } break;
        
        case Op_aas:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_das:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_mul:
        {
            u32 R = V0*V1;
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_imul:
        {
            u32 R = (s32)(s16)V0 * (s32)(s16)V1;
            if(WWidth == 1)
            {
                R = (s32)(s8)V0 * (s32)(s8)V1;
            }
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_aam:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_div:
        {
            if(V1 == 0)
            {
                ExecInterrupt(Memory, Registers, 0);
            }
            else
            {
                u32 R = V0/V1;
                WriteArithOpResult(Registers, Op0, R, WWidth);
            }
        } break;
        
        case Op_idiv:
        {
            u32 R = (s32)(s16)V0 / (s32)(s16)V1;
            if(WWidth == 1)
            {
                R = (s32)(s8)V0 / (s32)(s8)V1;
            }
            WriteArithOpResult(Registers, Op0, R, WWidth);
        } break;
        
        case Op_aad:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_cbw:
        {
            Registers->ah = (Registers->al & 0x80) ? 0xff : 0x00;
        } break;
        
        case Op_cwd:
        {
            Registers->dx = (Registers->ax & 0x8000) ? 0xffff : 0x000;
        } break;
        
        case Op_not:
        {
            u32 R = ~V0;
            WriteN(Op0, 0, R, WWidth);
            // NOTE(casey): For whatever reason, NOT is defined as leaving the flags unchanged, unlike the other logical ops
        } break;
        
        case Op_shl:
        {
            u32 R = (V0 << 1) << V1;
            
            WriteShiftOpResult(Registers, Op0, V0, R, WWidth);
            Result.ShiftCount = V1;
        } break;
        
        case Op_shr:
        {
            u32 R = (V0 << 1) >> V1;
            
            WriteShiftOpResult(Registers, Op0, V0, R, WWidth);
            Result.ShiftCount = V1;
        } break;
        
        case Op_sar:
        {
            u32 R = (s32)(s16)(V0 << 1) >> V1;
            if(WWidth == 1)
            {
                R = (s32)(s8)(V0 << 1) >> V1;
            }
            
            WriteShiftOpResult(Registers, Op0, V0, R, WWidth);
            Result.ShiftCount = V1;
        } break;
        
        case Op_rol:
        case Op_ror:
        case Op_rcl:
        case Op_rcr:
        {
            Result.ShiftCount = V1;
            Result.Unimplemented = true;
        } break;
        
        case Op_and:
        {
            WriteLogOpResult(Registers, Op0, V0 & V1, WWidth);
        } break;
        
        case Op_test:
        {
            UpdateLogFlags(Registers, V0 & V1, WWidth);
        } break;
        
        case Op_or:
        {
            WriteLogOpResult(Registers, Op0, V0 | V1, WWidth);
        } break;
        
        case Op_xor:
        {
            WriteLogOpResult(Registers, Op0, V0 ^ V1, WWidth);
        } break;
        
        case Op_movs:
        case Op_cmps:
        case Op_scas:
        case Op_lods:
        case Op_stos:
        {
            Result.RepCount = Registers->cx;
            Result.Unimplemented = true;
        } break;
        
        case Op_call:
        {
            instruction_operand Target = Instruction.Operands[0];
            assert(Target.Type == Operand_Memory);
            
            if(IsFar)
            {
                Push(Memory, Registers, Registers->cs);
                Registers->cs = Target.Address.ExplicitSegment;
            }
            
            Push(Memory, Registers, Registers->ip);

            // TODO(casey): This is not actually complete.
            // It needs the IP to be updated here.
            
            Result.Unimplemented = true;
        }
        
        case Op_jmp:
        {
            Result.Unimplemented = true;
        } break;
        
        case Op_ret:
        case Op_retf:
        {
            Registers->ip = Pop(Memory, Registers);
            if(IsFar)
            {
                Registers->cs = Pop(Memory, Registers);
            }
            
            Registers->sp += V0;
        } break;
        
        case Op_je:
        {
            ConditionalJump(&Result, Registers, V0, ZF == 1);
        } break;
        
        case Op_jl:
        {
            ConditionalJump(&Result, Registers, V0, (SF ^ OF) == 1);
        } break;
        
        case Op_jle:
        {
            ConditionalJump(&Result, Registers, V0, ((SF ^ OF) | ZF) == 1);
        } break;
        
        case Op_jb:
        {
            ConditionalJump(&Result, Registers, V0, CF == 1);
        } break;
        
        case Op_jbe:
        {
            ConditionalJump(&Result, Registers, V0, (CF | ZF) == 1);
        } break;
        
        case Op_jp:
        {
            ConditionalJump(&Result, Registers, V0, PF == 1);
        } break;
        
        case Op_jo:
        {
            ConditionalJump(&Result, Registers, V0, OF == 1);
        } break;
        
        case Op_js:
        {
            ConditionalJump(&Result, Registers, V0, SF == 1);
        } break;
        
        case Op_jne:
        {
            ConditionalJump(&Result, Registers, V0, ZF == 0);
        } break;
        
        case Op_jnl:
        {
            ConditionalJump(&Result, Registers, V0, (SF ^ OF) == 0);
        } break;
        
        case Op_jg:
        {
            ConditionalJump(&Result, Registers, V0, ((SF & OF) | ZF) == 0);
        } break;
        
        case Op_jnb:
        {
            ConditionalJump(&Result, Registers, V0, CF == 0);
        } break;
        
        case Op_ja:
        {
            ConditionalJump(&Result, Registers, V0, (CF | ZF) == 0);
        } break;
        
        case Op_jnp:
        {
            ConditionalJump(&Result, Registers, V0, PF == 0);
        } break;
        
        case Op_jno:
        {
            ConditionalJump(&Result, Registers, V0, OF == 0);
        } break;
        
        case Op_jns:
        {
            ConditionalJump(&Result, Registers, V0, SF == 0);
        } break;
        
        case Op_loop:
        {
            ConditionalJump(&Result, Registers, V0, --Registers->cx != 0);
        } break;
        
        case Op_loopz:
        {
            ConditionalJump(&Result, Registers, V0, (--Registers->cx != 0) && (ZF == 1));
        } break;
        
        case Op_loopnz:
        {
            ConditionalJump(&Result, Registers, V0, (--Registers->cx != 0) && (ZF == 0));
        } break;
        
        case Op_jcxz:
        {
            ConditionalJump(&Result, Registers, V0, Registers->cx != 0);
        } break;
        
        case Op_int:
        {
            ExecInterrupt(Memory, Registers, V0);
        } break;
        
        case Op_int3:
        {
            ExecInterrupt(Memory, Registers, 3);
        } break;
        
        case Op_into:
        {
            ExecInterrupt(Memory, Registers, 4);
        } break;
        
        case Op_iret:
        {
            Registers->ip = Pop(Memory, Registers);
            Registers->cs = Pop(Memory, Registers);
            PopFlags(Memory, Registers);
        } break;
        
        case Op_clc:
        {
            Registers->flags &= ~Flag_CF;
        } break;
        
        case Op_cmc:
        {
            Registers->flags ^= Flag_CF;
        } break;
        
        case Op_stc:
        {
            Registers->flags |= Flag_CF;
        } break;
        
        case Op_cld:
        {
            Registers->flags &= ~Flag_DF;
        } break;
        
        case Op_std:
        {
            Registers->flags |= Flag_DF;
        } break;
        
        case Op_cli:
        {
            Registers->flags &= ~Flag_IF;
        } break;
        
        case Op_sti:
        {
            Registers->flags |= Flag_IF;
        } break;
        
        //
        
        case Op_hlt:
        case Op_wait:
        case Op_esc:
        {
            // NOTE(casey): Not really usable unless the simulator is hooked up to external input
        } break;
        
        case Op_rep:
        case Op_lock:
        case Op_segment:
        {
            // NOTE(casey): These are not actually executed, they just affect subsequent instructions during decode.
        } break;
        
        case Op_None:
        case Op_Count:
        {
            // NOTE(casey): This should never actually happen, but if it does, it just means someone is passing
            // us an uninitialized instruction or something, which is fine.
        } break;
    }
    
    return Result;
}
    