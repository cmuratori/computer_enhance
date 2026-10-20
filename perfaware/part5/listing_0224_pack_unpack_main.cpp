/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 224
   ======================================================================== */

#include "listing_0220_puzzle_utils.cpp"

int main(void)
{
    char ValuesA[SIMD_BYTE_COUNT + 1] = "abcdefgh" "ijklmnop" "qrstuvwx" "12345678";
    char ValuesB[SIMD_BYTE_COUNT + 1] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" ".,;-=[]|";
    
    __m256i Ai = _mm256_loadu_si256((__m256i *)ValuesA);
    __m256i Bi = _mm256_loadu_si256((__m256i *)ValuesB);
    
    Print("Input A", Ai);
    Print("Input B", Bi);

    //
    // NOTE(casey): "Unpacks"
    //
    
    Section("VPUNPCKLBW", "Interleave 8-bit values from the first and third 64-bit lanes of two sources");
    PRINT_PACK(_mm256_unpacklo_epi8, Ai, Bi);
    
    Section("VPUNPCKLWD", "Interleave 16-bit values from the first and third 64-bit lanes of two sources");
    PRINT_PACK(_mm256_unpacklo_epi16, Ai, Bi);
    
    Section("VPUNPCKLDQ / VUNPCKLPS", "Interleave 32-bit values from the first and third 64-bit lanes of two sources");
    // NOTE(casey): Use _mm256_unpacklo_ps for VUNPCKLPS
    PRINT_PACK(_mm256_unpacklo_epi32, Ai, Bi);
    
    Section("VPUNPCKLQDQ / VUNPCKLPD", "Interleave the first and third 64-bit lanes of two sources");
    // NOTE(casey): Use _mm256_unpacklo_pd for VUNPCKLPD
    PRINT_PACK(_mm256_unpacklo_epi64, Ai, Bi);
    
    Section("VPUNPCKHBW", "Interleave 8-bit values from the second and fourth 64-bit lanes of two sources");
    PRINT_PACK(_mm256_unpackhi_epi8, Ai, Bi);
    
    Section("VPUNPCKHWD", "Interleave 16-bit values from the second and fourth 64-bit lanes of two sources");
    PRINT_PACK(_mm256_unpackhi_epi16, Ai, Bi);
    
    Section("VPUNPCKHDQ / VUNPCKHPS", "Interleave 32-bit values from the second and fourth 64-bit lanes of two sources");
    // NOTE(casey): Use _mm256_unpackhi_ps for VUNPCKHPS
    PRINT_PACK(_mm256_unpackhi_epi32, Ai, Bi);
    
    Section("VPUNPCKHQDQ / VUNPCKHPD", "Interleave the second and fourth 64-bit lanes of two sources");
    PRINT_PACK(_mm256_unpackhi_epi64, Ai, Bi);
    
    /* NOTE(casey): "Packs"
    
       We don't need these for our present purposes,
       but just so you know what they are:
       
       VPACKSSWB - _mm256_packs_epi16(Ai, Bi) - 16->8 with signed saturation
       VPACKSSDW - _mm256_packs_epi32(Ai, Bi) - 32->16 with signed saturation
       
       VPACKUSWB - _mm256_packus_epi16(Ai, Bi) - 16->8 with unsigned saturation
       VPACKUSDW - _mm256_packus_epi32(Ai, Bi) - 32->16 with unsigned saturation
    */
}
    