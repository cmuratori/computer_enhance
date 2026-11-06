/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 221
   ======================================================================== */

#include "listing_0220_puzzle_utils.cpp"

#define SHUFIMM_2x4(E0, E1) ((E0) | ((E1) << 4))
#define SHUFIMM_4x2(E0, E1, E2, E3) ((E0) | ((E1) << 2) | ((E2) << 4) | ((E3) << 6))
#define SHUFIMM_4x1(E0, E1, E2, E3) ((E0) | ((E1) << 1) | ((E2) << 2) | ((E3) << 3))

int main(void)
{
    char ValuesA[SIMD_BYTE_COUNT + 1] = "abcdefgh" "ijklmnop" "qrstuvwx" "12345678";
    char ValuesB[SIMD_BYTE_COUNT + 1] = "ABCDEFGH" "IJKLMNOP" "QRSTUVWX" ".,;-=[]|";
    
    __m256 As = _mm256_loadu_ps((f32 *)ValuesA);
    __m256 Bs = _mm256_loadu_ps((f32 *)ValuesB);
    __m256d Ad = _mm256_loadu_pd((f64 *)ValuesA);
    __m256d Bd = _mm256_loadu_pd((f64 *)ValuesB);
    __m256i Ai = _mm256_loadu_si256((__m256i *)ValuesA);
    __m256i Bi = _mm256_loadu_si256((__m256i *)ValuesB);
    
    Print("Input A", As);
    Print("Input B", Bs);
    
    Category("Unary Immediate Within-128");
    
    Section("VPSHUFLW", "Immediate 16-bit reordering within LOW 64 bits of each 128-bit lane");
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (0,1,2,3), Ai);
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (3,2,1,0), Ai);
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (0,0,0,0), Ai);
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (1,1,1,1), Ai);
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (2,2,2,2), Ai);
    PRINT_SHUFFLE(_mm256_shufflelo_epi16, SHUFIMM_4x2, (3,3,3,3), Ai);
    
    Section("VPSHUFHW", "Immediate 16-bit reordering within HIGH 64 bits of each 128-bit lane");
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (0,1,2,3), Ai);
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (3,2,1,0), Ai);
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (0,0,0,0), Ai);
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (1,1,1,1), Ai);
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (2,2,2,2), Ai);
    PRINT_SHUFFLE(_mm256_shufflehi_epi16, SHUFIMM_4x2, (3,3,3,3), Ai);

    Section("VPSHUFD / VPERMILPS (immediate)", "Immediate 32-bit reordering within each 128-bit lane");
    // NOTE(casey): For VPERMILPS, use _mm256_permute_ps
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (0,1,2,3), Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (3,2,1,0), Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (0,0,0,0), Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (1,1,1,1), Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (2,2,2,2), Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi32, SHUFIMM_4x2, (3,3,3,3), Ai);
    
    Section("VPERMILPD (immediate)", "Immediate 64-bit reordering within each 128-bit lane");
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (0,1,0,1), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (1,0,1,0), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (1,0,0,1), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (0,1,1,0), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (0,0,0,0), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (0,0,1,1), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (1,1,0,0), Ad);
    PRINT_SHUFFLE(_mm256_permute_pd, SHUFIMM_4x1, (1,1,1,1), Ad);
    
    Category("Unary Parametric Within-128");
    
    Section("VPERMILPS (with vector)", "Parametric 32-bit reordering within each 128-bit lane");
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (0,1,2,3,0,1,2,3), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (3,2,1,0,3,2,1,0), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (0,1,2,3,3,2,1,0), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (0,0,0,0,0,0,0,0), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (1,1,1,1,1,1,1,1), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (2,2,2,2,2,2,2,2), As);
    PRINT_SHUFFLE(_mm256_permutevar_ps, _mm256_setr_epi32, (3,3,3,3,3,3,3,3), As);
    
    Section("VPERMILPD (vector)", "Parametric 64-bit reordering within each 128-bit lane");
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (0,2,0,2), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (2,0,2,0), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (2,0,0,2), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (0,2,2,0), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (0,0,0,0), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (0,0,2,2), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (2,2,0,0), Ad);
    PRINT_SHUFFLE(_mm256_permutevar_pd, _mm256_setr_epi64x, (2,2,2,2), Ad);
    
    Section("VPSHUFB", "Parametric 8-bit reordering within each 128-bit lane");
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  ( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15),
                  Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  ( 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15,
                   15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
                  Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  (15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15),
                  Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  (15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                   15,14,13,12,11,10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0),
                  Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  ( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0),
                  Ai);
    PRINT_SHUFFLE(_mm256_shuffle_epi8, _mm256_setr_epi8,
                  ( 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0,
                    0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0,-1, 0, 0, 0),
                  Ai);

    Category("Binary Immediate Within-128");
    
    Section("VSHUFPS", "64-bit interleaving of immediate-selected 32-bit lanes");
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (0,1,2,3), As, Bs);
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (3,2,1,0), As, Bs);
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (0,0,0,0), As, Bs);
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (1,1,1,1), As, Bs);
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (2,2,2,2), As, Bs);
    PRINT_SHUFFLE(_mm256_shuffle_ps, SHUFIMM_4x2, (3,3,3,3), As, Bs);
    
    Section("VSHUFPD", "64-bit interleaving of immediate-selected 64-bit lanes");
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (0,1,0,1), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (1,0,1,0), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (1,0,0,1), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (0,1,1,0), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (0,0,0,0), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (0,0,1,1), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (1,1,0,0), Ad, Bd);
    PRINT_SHUFFLE(_mm256_shuffle_pd, SHUFIMM_4x1, (1,1,1,1), Ad, Bd);
    
    Category("Unary Immediate Cross-128");
    
    Section("VPERMPD / VPERMQ", "Immediate arbitrary reordering of 64-bit lanes");
    // NOTE(casey): For VPERMQ, use _mm256_permute4x64_epi64
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (0,1,2,3), Ad);
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (3,2,1,0), Ad);
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (0,0,0,0), Ad);
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (1,1,1,1), Ad);
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (2,2,2,2), Ad);
    PRINT_SHUFFLE(_mm256_permute4x64_pd, SHUFIMM_4x2, (3,3,3,3), Ad);
    
    Category("Unary Parametric Cross-128");
    
    Section("VPERMPS / VPERMD", "Parametric selection of any 8 32-bit lanes from 1 256-bit input");
    // NOTE(casey): For VPERMD, use _mm256_permutevar8x32_epi32 instead
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 0, 1, 2, 3, 4, 5, 6, 7), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 7, 6, 5, 4, 3, 2, 1, 0), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 0, 0, 0, 0, 0, 0, 0, 0), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 1, 1, 1, 1, 1, 1, 1, 1), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 2, 2, 2, 2, 2, 2, 2, 2), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 3, 3, 3, 3, 3, 3, 3, 3), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 4, 4, 4, 4, 4, 4, 4, 4), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 5, 5, 5, 5, 5, 5, 5, 5), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 6, 6, 6, 6, 6, 6, 6, 6), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, ( 7, 7, 7, 7, 7, 7, 7, 7), As);
    PRINT_SHUFFLE(_mm256_permutevar8x32_ps, _mm256_setr_epi32, (-1,-1,-1,-1,-1,-1,-1,-1), As);
    
    Category("Binary Immediate Cross-128");
    
    Section("VPERM2I128 / VPERM2F128", "Immediate selection of any 2 128-bit lanes from 2 256-bit inputs (or zero)");
    // NOTE(casey): For VPERM2F128, use _mm256_permute2f128_ps,
    // _mm256_permute2f128_pd, or _mm256_permute2f128_si256
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 0, 1), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 2, 3), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 0, 0), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 1, 1), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 2, 2), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 3, 3), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, ( 0,15), Ai, Bi);
    PRINT_SHUFFLE(_mm256_permute2x128_si256, SHUFIMM_2x4, (15, 1), Ai, Bi);
}
