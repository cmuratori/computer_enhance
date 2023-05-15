/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static instruction_timing ClockRangeTransfers(u32 MinClocks, u32 MaxClocks, u32 Transfers, u32 EAClocks = 0)
{
    instruction_timing Result = {};
    
    Result.Base.Min = MinClocks;
    Result.Base.Max = MaxClocks;
    Result.Transfers = Transfers;
    Result.EAClocks = EAClocks;
    
    return Result;
}

static instruction_timing ClocksTransfers(u32 Clocks, u32 Transfers, u32 EAClocks = 0)
{
    instruction_timing Result = ClockRangeTransfers(Clocks, Clocks, Transfers, EAClocks);
    return Result;
}

static u32 CalculateEAClocksFrom(instruction Instruction, u32 OperandIndex)
{
    u32 Result = 0;
    
    instruction_operand Operand = GetOperand(Instruction, OperandIndex);
    assert(Operand.Type == Operand_Memory);
    
    effective_address_expression Expr = Operand.Address;
    register_index T0 = Expr.Terms[0].Register.Index;
    register_index T1 = Expr.Terms[1].Register.Index;
    if(T0)
    {
        if(T1)
        {
            if(((T0 == Register_bp) && (T1 == Register_di)) ||
               ((T0 == Register_b) && (T1 == Register_si)))
            {
                Result = 7;
            }
            else
            {
                Result = 8;
            }
        }
        else
        {
            Result = 5;
        }
    }
    else
    {
        Result = 2;
    }
    
    if(Expr.Displacement)
    {
        Result += 4;
    }
    
    if(Instruction.SegmentOverride)
    {
        Result += 2;
    }
    
    return Result;
}
    
static instruction_timing EstimateInstructionClocks(timing_state State, instruction Instruction)
{
    /* TODO(casey): This routine is designed to return the results of the cycles table in the 8086 users manual.
       Based on some of the entries in the table, it is HIGHLY LIKELY that some of the entries are typos.
       Please do not use this as an actual reference for the behavior of an 8086. Without a more accurate
       reference manual, these numbers are VERY suspect. */
    
    instruction_timing Result = {};

    b32 UsedAccumulator = false;
    b32 UsedSegReg = false;

    b32 Register0 = OperandIsType(Instruction, 0, Operand_Register);
    b32 Register1 = OperandIsType(Instruction, 1, Operand_Register);
    b32 Memory0 = OperandIsType(Instruction, 0, Operand_Memory);
    b32 Memory1 = OperandIsType(Instruction, 1, Operand_Memory);
    b32 Immediate0 = OperandIsType(Instruction, 0, Operand_Immediate);
    b32 Immediate1 = OperandIsType(Instruction, 1, Operand_Immediate);
    
    b32 Far = (Instruction.Flags & Inst_Far);
    b32 Wide = (Instruction.Flags & Inst_Wide);
    
    u32 EA = 0;
    if(Memory0) EA = CalculateEAClocksFrom(Instruction, 0);
    if(Memory1) EA = CalculateEAClocksFrom(Instruction, 1);
    
    // NOTE(casey): These have to be passed in
    b32 Taken = State.AssumeBranchTaken;
    u32 Rep = State.AssumeRepCount;
    u32 CL = State.AssumeShiftCount;
    
    switch(Instruction.Op)
    {
        case Op_cbw:
        case Op_clc:
        case Op_cld:
        case Op_cli:
        case Op_cmc:
        case Op_hlt:
        case Op_lock:
        case Op_rep:
        case Op_stc:
        case Op_std:
        case Op_sti:
        case Op_segment:
        {
            Result = ClocksTransfers(2, 0);
        } break;
        
        case Op_aaa:
        case Op_aas:
        case Op_daa:
        case Op_das:
        case Op_lahf:
        case Op_sahf:
        {
            Result = ClocksTransfers(4, 0);
        } break;
        
        case Op_cwd: {Result = ClocksTransfers(5, 0);} break;
        case Op_aad: {Result = ClocksTransfers(60, 0);} break;
        case Op_aam: {Result = ClocksTransfers(83, 0);} break;
        
        case Op_adc:
        case Op_add:
        case Op_and:
        case Op_xor:
        case Op_or:
        case Op_sub:
        case Op_sbb:
        {
            if(Register0 && Register1)     {Result = ClocksTransfers(3, 0);}
            if(Register0 && Memory1)       {Result = ClocksTransfers(9, 1, EA);}
            if(Memory0 && Register1)       {Result = ClocksTransfers(16, 2, EA);}
            if(Register0 && Immediate1)    {Result = ClocksTransfers(4, 0);}
            if(Memory0 && Immediate1)      {Result = ClocksTransfers(17, 2, EA);}
        } break;
        
        case Op_call:
        {
            if(Memory0)
            {
                if(Far)
                {
                    Result = ClocksTransfers(37, 4, EA);
                }
                else
                {
                    Result = ClocksTransfers(21, 2, EA);
                }
            }
            else if(Register0)
            {
                Result = ClocksTransfers(16, 1);
            }
            else
            {
                if(Far)
                {
                    Result = ClocksTransfers(28, 2);
                }
                else
                {
                    Result = ClocksTransfers(19, 1);
                }
            }
        } break;
        
        case Op_cmp:
        {
            if(Register0 && Register1)     {Result = ClocksTransfers(3, 0);}
            if(Register0 && Memory1)       {Result = ClocksTransfers(9, 1, EA);}
            if(Memory0 && Register1)       {Result = ClocksTransfers(9, 1, EA);}
            if(Register0 && Immediate1)    {Result = ClocksTransfers(4, 0);}
            if(Memory0 && Immediate1)      {Result = ClocksTransfers(10, 1, EA);}
        } break;
        
        case Op_cmps:
        {
            if(Rep)
            {
                Result = ClocksTransfers(9 + 22*Rep, 2*Rep);
            }
            else
            {
                Result = ClocksTransfers(22, 2);
            }
        } break;
        
        case Op_dec:
        case Op_inc:
        {
            if(Register0 && !Wide) {Result = ClocksTransfers(3, 0);}
            if(Register0 && Wide)  {Result = ClocksTransfers(2, 0);}
            if(Memory0)            {Result = ClocksTransfers(15, 2, EA);}
        } break;
        
        case Op_div:
        {
            if(Register0 && !Wide) {Result = ClockRangeTransfers(80, 90, 0);}
            if(Register0 && Wide)  {Result = ClockRangeTransfers(144, 162, 0);}
            if(Memory0 && !Wide)   {Result = ClockRangeTransfers(86, 96, 1, EA);}
            if(Memory0 && Wide)    {Result = ClockRangeTransfers(150, 168, 1, EA);}
        } break;
        
        case Op_esc:
        {
            if(Immediate0 && Memory1)   {Result = ClocksTransfers(8, 1, EA);}
            if(Immediate0 && Register1) {Result = ClocksTransfers(2, 0);}
        } break;
        
        case Op_idiv:
        {
            if(Register0 && !Wide) {Result = ClockRangeTransfers(101, 112, 0);}
            if(Register0 && Wide)  {Result = ClockRangeTransfers(165, 184, 0);}
            if(Memory0 && !Wide)   {Result = ClockRangeTransfers(107, 118, 1, EA);}
            if(Memory0 && Wide)    {Result = ClockRangeTransfers(171, 190, 1, EA);}
        } break;
        
        case Op_imul:
        {
            if(Register0 && !Wide) {Result = ClockRangeTransfers(80, 98, 0);}
            if(Register0 && Wide)  {Result = ClockRangeTransfers(128, 154, 0);}
            if(Memory0 && !Wide)   {Result = ClockRangeTransfers(86, 104, 1, EA);}
            if(Memory0 && Wide)    {Result = ClockRangeTransfers(134, 160, 1, EA);}
        } break;
        
        case Op_in:
        {
            if(Register0 && Immediate1) {Result = ClocksTransfers(10, 1);}
            if(Register0 && Register1)  {Result = ClocksTransfers(8, 1);}
        } break;
        
        case Op_int:
        {
            if(Instruction.Operands[0].Immediate.Value == 3)
            {
                Result = ClocksTransfers(52, 5);
            }
            else
            {
                Result = ClocksTransfers(51, 5);
            }
        } break;
        
        case Op_int3: {Result = ClocksTransfers(52, 5);} break;
        case Op_into: {Result = ClockRangeTransfers(4, 53, 5);} break;
        case Op_iret: {Result = ClocksTransfers(24, 3);} break;
        
        case Op_je:
        case Op_jl:
        case Op_jle:
        case Op_jb:
        case Op_jbe:
        case Op_jp:
        case Op_jo:
        case Op_js:
        case Op_jne:
        case Op_jnl:
        case Op_jg:
        case Op_jnb:
        case Op_ja:
        case Op_jnp:
        case Op_jno:
        case Op_jns:
        {
            Result = ClocksTransfers(Taken ? 16 : 4, 0);
        } break;
        
        case Op_jcxz:
        {
            Result = ClocksTransfers(Taken ? 18 : 6, 0);
        } break;
        
        case Op_jmp:
        {
            if(Memory0 && Far)  {Result = ClocksTransfers(24, 2, EA);}
            if(Memory0 && !Far) {Result = ClocksTransfers(18, 1, EA);}
            if(Immediate0)      {Result = ClocksTransfers(15, 0);}
            if(Register0)       {Result = ClocksTransfers(11, 0);}
        } break;
        
        case Op_lds: {Result = ClocksTransfers(16, 2, EA);} break;
        case Op_lea: {Result = ClocksTransfers(2, 0, EA);} break;
        case Op_les: {Result = ClocksTransfers(16, 2, EA);} break;
        
        case Op_lods:
        {
            if(Rep)
            {
                Result = ClocksTransfers(9 + 13*Rep, Rep);
            }
            else
            {
                Result = ClocksTransfers(12, 1);
            }
        } break;
        
        case Op_loop:   {Result = ClocksTransfers(Taken ? 17 : 5, 0);} break;
        case Op_loopz:  {Result = ClocksTransfers(Taken ? 18 : 6, 0);} break;
        case Op_loopnz: {Result = ClocksTransfers(Taken ? 19 : 5, 0);} break;
        
        case Op_mov:
        {
            /* TODO(casey): These numbers are what the manual claims, but it seems absurd that the 8086 somehow
               _didn't_ have to do the effective address calculation when it was moving an accumulator. I
               am fairly certain it is a misprint, and EA _should_ be included. Unfortunately I have no way
               of testing this myself. */
               
            if(Memory0 && Register1)
            {
                if(UsedAccumulator)
                {
                    Result = ClocksTransfers(10, 1);
                }
                else
                {
                    Result = ClocksTransfers(9, 1, EA);
                }
            }
            
            if(Register0 && Memory1)
            {
                if(UsedAccumulator)
                {
                    Result = ClocksTransfers(10, 1);
                }
                else
                {
                    Result = ClocksTransfers(8, 1, EA);
                }
            }
            
            if(Register0 && Register1)  {Result = ClocksTransfers(2, 0);}
            if(Register0 && Immediate1) {Result = ClocksTransfers(4, 0);}
            if(Memory0 && Immediate1)   {Result = ClocksTransfers(10, 1, EA);}
        } break;
        
        case Op_movs:
        {
            if(Rep)
            {
                Result = ClocksTransfers(9 + 17*Rep, 2*Rep);
            }
            else
            {
                Result = ClocksTransfers(18, 2);
            }
        } break;
        
        case Op_mul:
        {
            if(Register0 && !Wide) {Result = ClockRangeTransfers(70, 77, 0);}
            if(Register0 && Wide)  {Result = ClockRangeTransfers(118, 133, 0);}
            if(Memory0 && !Wide)   {Result = ClockRangeTransfers(76, 83, 1, EA);}
            if(Memory0 && Wide)    {Result = ClockRangeTransfers(124, 139, 1, EA);}
        } break;
        
        case Op_neg:
        case Op_not:
        {
            if(Register0) {Result = ClocksTransfers(3, 0);}
            if(Memory0)   {Result = ClocksTransfers(16, 2, EA);}
        } break;
        
        case Op_out:
        {
            if(Immediate0 && Register1) {Result = ClocksTransfers(10, 1);}
            if(Register0 && Register1)  {Result = ClocksTransfers(8, 1);}
        } break;
        
        case Op_pop:
        {
            if(Register0) {Result = ClocksTransfers(8, 1);}
            if(Memory0)   {Result = ClocksTransfers(17, 2, EA);}
        } break;
        
        case Op_popf: {Result = ClocksTransfers(8, 1);} break;
        
        case Op_push:
        {
            /* TODO(casey): It seems suspicious that push takes one less clock to push a segment register,
               but pop doens't take one less clock to pop it. It's _possible_ that the 8086 worked that way,
               but, it's also possible this is another misprint. */
            if(Register0) {Result = ClocksTransfers(UsedSegReg ? 10 : 11, 1);}
            if(Memory0)   {Result = ClocksTransfers(16, 2);}
        } break;
        
        case Op_pushf: {Result = ClocksTransfers(10, 1);} break;
        
        case Op_ret:
        {
            Result = ClocksTransfers(Immediate0 ? 12 : 8, 1);
        } break;
        
        case Op_retf:
        {
            Result = ClocksTransfers(Immediate0 ? 17 : 18, 2);
        } break;
        
        case Op_rcl:
        case Op_rcr:
        case Op_rol:
        case Op_ror:
        case Op_shl:
        case Op_sar:
        case Op_shr:
        {
            if(Register0 && Immediate1) {Result = ClocksTransfers(2, 0);}
            if(Register0 && Register1)  {Result = ClocksTransfers(8 + 4*CL, 0);}
            if(Memory0 && Immediate1)   {Result = ClocksTransfers(15, 2, EA);}
            if(Memory0 && Register1)    {Result = ClocksTransfers(20 + 4*CL, 2, EA);}
        } break;
        
        case Op_scas:
        {
            if(Rep)
            {
                Result = ClocksTransfers(9 + 15*Rep, Rep);
            }
            else
            {
                Result = ClocksTransfers(15, 1);
            }
        } break;
        
        case Op_stos:
        {
            if(Rep)
            {
                Result = ClocksTransfers(9 + 10*Rep, Rep);
            }
            else
            {
                Result = ClocksTransfers(11, 1);
            }
        } break;
        
        case Op_test:
        {
            if(Register0 && Register1)     {Result = ClocksTransfers(3, 0);}
            if(Register0 && Memory1)       {Result = ClocksTransfers(9, 1, EA);}
            if(Register0 && Immediate1)    {Result = ClocksTransfers(UsedAccumulator ? 4 : 5, 0);}
            if(Memory0 && Immediate1)      {Result = ClocksTransfers(11, 0, EA);}
        } break;
        
        case Op_wait: {Result = ClocksTransfers(3 + 5*Rep, 0);} break;
        
        case Op_xchg:
        {
            if(Memory0 && Register1)      {Result = ClocksTransfers(17, 2, EA);}
            if(Register0 && Register1)    {Result = ClocksTransfers(UsedAccumulator ? 3 : 4, 0);}
        } break;
        
        case Op_xlat: {Result = ClocksTransfers(11, 1);} break;
        
        case Op_None:
        case Op_Count:
        {
        } break;
    }
    
    return Result;
}

static void UpdateTimingForExec(timing_state *State, exec_result Exec)
{
    State->AssumeBranchTaken = Exec.BranchTaken;
    State->AssumeAddressUnanaligned = Exec.AddressIsUnaligned;
    State->AssumeRepCount = Exec.RepCount;
    State->AssumeShiftCount = Exec.ShiftCount;
}

static instruction_clock_interval ExpectedClocksFrom(timing_state State, instruction Instruction, instruction_timing Timing)
{
    u32 ExtraClocks = Timing.EAClocks;
    
    if((Instruction.Flags & Inst_Wide) && (State.Assume8088 || State.AssumeAddressUnanaligned))
    {
        ExtraClocks += 4*Timing.Transfers;
    }
    
    instruction_clock_interval Result = Timing.Base;
    Result.Min += ExtraClocks;
    Result.Max += ExtraClocks;
    
    return Result;
}
