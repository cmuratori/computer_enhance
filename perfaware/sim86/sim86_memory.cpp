/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

static u32 GetHighestAddress(segmented_access SegMem)
{
    u32 Result = SegMem.Mask;
    return Result;
}

static u32 GetAbsoluteAddressOf(u32 Mask, u16 SegmentBase, u16 SegmentOffset, u16 AdditionalOffset)
{
    u32 Result = (((u32)SegmentBase << 4) + (u32)(SegmentOffset + AdditionalOffset)) & Mask;
    return Result;
}

static u32 GetAbsoluteAddressOf(segmented_access SegMem, u16 Offset)
{
    u32 Result = GetAbsoluteAddressOf(SegMem.Mask, SegMem.SegmentBase, SegMem.SegmentOffset, Offset);
    return Result;
}

static segmented_access MoveBaseBy(segmented_access Access, s32 Offset)
{
    Access.SegmentOffset += Offset;
    
    segmented_access Result = Access;
    
    Result.SegmentBase += (Result.SegmentOffset >> 4);
    Result.SegmentOffset &= 0xf;

    assert(GetAbsoluteAddressOf(Result, 0) == GetAbsoluteAddressOf(Access, 0));
    
    return Result;
}

static u8 *AccessMemory(segmented_access SegMem, u16 Offset)
{
    u32 AbsAddr = GetAbsoluteAddressOf(SegMem, Offset);
    u8 *Result = SegMem.Memory + AbsAddr;
    return Result;
}

static b32 IsValid(segmented_access SegMem)
{
    b32 Result = (SegMem.Mask != 0);
    return Result;
}

static segmented_access FixedMemoryPow2(u32 SizePow2, u8 *Memory)
{
    segmented_access Result = {};
    
    Result.Memory = Memory;
    Result.Mask = (1 << SizePow2) - 1;
    
    return Result;
}
