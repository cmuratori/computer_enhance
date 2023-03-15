/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

struct memory
{
    u8 Bytes[1024*1024];
};
#define MEMORY_ACCESS_MASK 0xfffff
static_assert((ArrayCount(memory::Bytes) - 1) == MEMORY_ACCESS_MASK, "Memory size doesn't match access mask");

struct segmented_access
{
    u16 SegmentBase;
    u16 SegmentOffset;
};

static u32 GetAbsoluteAddressOf(u16 SegmentBase, u16 SegmentOffset, u16 AdditionalOffset = 0);
static u32 GetAbsoluteAddressOf(segmented_access Access, u16 AdditionalOffset = 0);

static u8 ReadMemory(memory *Memory, u32 AbsoluteAddress);

static u32 LoadMemoryFromFile(char *FileName, memory *Memory, u32 AtOffset);
