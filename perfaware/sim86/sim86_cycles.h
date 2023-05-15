/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

struct instruction_clock_interval
{
    u32 Min;
    u32 Max;
};

struct instruction_timing
{
    instruction_clock_interval Base;
    u32 Transfers;
    u32 EAClocks;
};

struct timing_state
{
    b32 Assume8088;
    b32 AssumeBranchTaken;
    b32 AssumeAddressUnanaligned;
    u32 AssumeRepCount;
    u32 AssumeShiftCount;
};

static instruction_timing EstimateInstructionClocks(timing_state State, instruction Instruction);
static void UpdateTimingForExec(timing_state *State, exec_result Exec);
static instruction_clock_interval ExpectedClocksFrom(timing_state State, instruction Instruction, instruction_timing Timing);
