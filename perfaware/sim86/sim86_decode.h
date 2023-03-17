/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

struct decode_context
{
    register_index DefaultSegment;
    u32 AdditionalFlags;
};

static instruction DecodeInstruction(decode_context *Context, instruction_table *Table, memory *Memory, segmented_access *At);
