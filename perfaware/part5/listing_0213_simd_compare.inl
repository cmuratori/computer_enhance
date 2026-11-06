/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 213
   ======================================================================== */

#if !defined(Unordered)
#define Unordered true
#endif

#if !defined(Ordered)
#define Ordered false
#endif

#if !defined(Quiet)
#define Quiet false
#endif

#if !defined(Signaling)
#define Signaling true
#endif


SIMD_CMP(==, Ordered, Quiet, 0, _CMP_EQ_OQ)
SIMD_CMP(==, Ordered, Signaling, 16, _CMP_EQ_OS)
SIMD_CMP(==, Unordered, Quiet, 8, _CMP_EQ_UQ)
SIMD_CMP(==, Unordered, Signaling, 24, _CMP_EQ_US)

SIMD_CMP(<, Ordered, Quiet, 17, _CMP_LT_OQ)
SIMD_CMP(<, Ordered, Signaling, 1, _CMP_LT_OS)
SIMD_CMP(<, Unordered, Quiet, 25, _CMP_NGE_UQ)
SIMD_CMP(<, Unordered, Signaling, 9, _CMP_NGE_US)

SIMD_CMP(<=, Ordered, Quiet, 18, _CMP_LE_OQ)
SIMD_CMP(<=, Ordered, Signaling, 2, _CMP_LE_OS)
SIMD_CMP(<=, Unordered, Quiet, 26, _CMP_NGT_UQ)
SIMD_CMP(<=, Unordered, Signaling, 10, _CMP_NGT_US)

SIMD_CMP(False, Ordered, Quiet, 11, _CMP_FALSE_OQ)
SIMD_CMP(False, Ordered, Signaling, 27, _CMP_FALSE_OS)
SIMD_CMP(False, Unordered, Quiet, 3, _CMP_UNORD_Q)
SIMD_CMP(False, Unordered, Signaling, 19, _CMP_UNORD_S)

SIMD_CMP(!=, Ordered, Quiet, 12, _CMP_NEQ_OQ)
SIMD_CMP(!=, Ordered, Signaling, 28, _CMP_NEQ_OS)
SIMD_CMP(!=, Unordered, Quiet, 4, _CMP_NEQ_UQ)
SIMD_CMP(!=, Unordered, Signaling, 20, _CMP_NEQ_US)

SIMD_CMP(>=, Ordered, Quiet, 29, _CMP_GE_OQ)
SIMD_CMP(>=, Ordered, Signaling, 13, _CMP_GE_OS)
SIMD_CMP(>=, Unordered, Quiet, 21, _CMP_NLT_UQ)
SIMD_CMP(>=, Unordered, Signaling, 5, _CMP_NLT_US)

SIMD_CMP(>, Ordered, Quiet, 30, _CMP_GT_OQ)
SIMD_CMP(>, Ordered, Signaling, 14, _CMP_GT_OS)
SIMD_CMP(>, Unordered, Quiet, 22, _CMP_NLE_UQ)
SIMD_CMP(>, Unordered, Signaling, 6, _CMP_NLE_US)

SIMD_CMP(True, Ordered, Quiet, 7, _CMP_ORD_Q)
SIMD_CMP(True, Ordered, Signaling, 23, _CMP_ORD_S)
SIMD_CMP(True, Unordered, Quiet, 15, _CMP_TRUE_UQ)
SIMD_CMP(True, Unordered, Signaling, 31, _CMP_TRUE_US)


#undef Signaling
#undef Quiet
#undef Ordered
#undef Unordered
#undef SIMD_CMP
