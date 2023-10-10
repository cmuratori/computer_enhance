/* ========================================================================

   (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 125
   ======================================================================== */

struct buffer
{
    size_t Count;
    u8 *Data;
};

#define CONSTANT_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

/* NOTE(casey): None of these functions should actually be inline - they
   should all just be "static". But I marked them as inline because that
   prevents compilers from complaining when they aren't used. */

inline b32 IsValid(buffer Buffer)
{
    b32 Result = (Buffer.Data != 0);
    return Result;
}

inline b32 IsInBounds(buffer Source, u64 At)
{
    b32 Result = (At < Source.Count);
    return Result;
}

inline b32 AreEqual(buffer A, buffer B)
{
    if(A.Count != B.Count)
    {
        return false;
    }
    
    for(u64 Index = 0; Index < A.Count; ++Index)
    {
        if(A.Data[Index] != B.Data[Index])
        {
            return false;
        }
    }
    
    return true;
}

inline buffer AllocateBuffer(size_t Count)
{
    buffer Result = {};
    Result.Data = (u8 *)malloc(Count);
    if(Result.Data)
    {
        Result.Count = Count;
    }
    else
    {
        fprintf(stderr, "ERROR: Unable to allocate %llu bytes.\n", Count);
    }
    
    return Result;
}

inline void FreeBuffer(buffer *Buffer)
{
    if(Buffer->Data)
    {
        free(Buffer->Data);
    }
    *Buffer = {};
}