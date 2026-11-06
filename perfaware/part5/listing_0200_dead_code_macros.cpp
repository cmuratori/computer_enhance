/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 200
   ======================================================================== */

#if defined(__clang__) || defined(__GNUC__)
#define PretendToWrite(Variable) asm volatile ("" : "+v"(Variable))
#define PretendToRead(Variable) asm volatile ("" : : "v"(Variable))
#else
#define PretendToWrite(Variable) (void)(Variable)
#define PretendToRead(Variable) (void)(Variable)
#pragma message("WARNING: Dead code macros are not available on this compiler.")
#endif
