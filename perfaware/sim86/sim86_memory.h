/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

struct segmented_access
{
    u8 *Memory;
    u32 Mask;
    u16 SegmentBase;
    u16 SegmentOffset;
};

static u32 GetHighestAddress(segmented_access SegMem);
static u32 GetAbsoluteAddressOf(segmented_access SegMem, u16 Offset = 0);
static segmented_access MoveBaseBy(segmented_access Access, s32 Offset);

static u8 *AccessMemory(segmented_access SegMem, u16 Offset = 0);

static b32 IsValid(segmented_access SegMem);
static segmented_access FixedMemoryPow2(u32 SizePow2, u8 *Memory);
