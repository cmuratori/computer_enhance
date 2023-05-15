/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

#include "sim86.h"
#include "sim86_instruction.h"
#include "sim86_instruction_table.h"

// NOTE(casey): This ridiculousness is just here so that we can preprocess these files
// and still have #ifdef's in the resulting file to support compilation via C-like
// languages, etc.
#define ifdefcpp #ifdef __cplusplus
#define externc extern "C" {
#define endif #endif
#define closebrace }

ifdefcpp
externc
endif
u32 Sim86_GetVersion(void);
void Sim86_Decode8086Instruction(u32 SourceSize, u8 *Source, instruction *Dest);
char const *Sim86_RegisterNameFromOperand(register_access *RegAccess);
char const *Sim86_MnemonicFromOperationType(operation_type Type);
void Sim86_Get8086InstructionTable(instruction_table *Dest);
ifdefcpp
closebrace
endif
