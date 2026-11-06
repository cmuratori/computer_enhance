/* ========================================================================

   (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
   
   This software is provided 'as-is', without any express or implied
   warranty. In no event will the authors be held liable for any damages
   arising from the use of this software.
   
   Please see https://computerenhance.com for more information
   
   ======================================================================== */

/* ========================================================================
   LISTING 222
   ======================================================================== */

#include "listing_0220_puzzle_utils.cpp"
    
#define SHUFIMM_2x4(E0, E1) ((E0) | ((E1) << 4))
#define SHUFIMM_4x2(E0, E1, E2, E3) ((E0) | ((E1) << 2) | ((E2) << 4) | ((E3) << 6))
#define SHUFIMM_4x1(E0, E1, E2, E3) ((E0) | ((E1) << 1) | ((E2) << 2) | ((E3) << 3))

static m256 Puzzle0(m256 A)
{
    __m256 Result = SolutionGoesHere(A);
    
    return M256From(Result);
}

static m256 Puzzle1(m256 A)
{
    __m256 Result = SolutionGoesHere(A);
    
    return M256From(Result);
}

static m256 Puzzle2(m256 A)
{
    __m256 Result = SolutionGoesHere(A);
    
    return M256From(Result);
}

static m256 Puzzle3(m256 A)
{
    __m256 Result = SolutionGoesHere(A);
    
    return M256From(Result);
}

static m256 Puzzle4(m256 A, m256 B)
{
    __m256 Result = SolutionGoesHere(A, B);
    
    return M256From(Result);
}

static m256 Puzzle5(m256 A, m256 B)
{
    __m256 Result = SolutionGoesHere(A, B);
    
    return M256From(Result);
}

static m256 Puzzle6(m256 A, m256 B)
{
    __m256 Result = SolutionGoesHere(A, B);
    
    return M256From(Result);
}

int main(void)
{
    char ValuesA[SIMD_BYTE_COUNT + 1] = "abcd" "efgh" "ijkl" "mnop" "qrst" "uvwx" "1234" "5678";
    char ValuesB[SIMD_BYTE_COUNT + 1] = "ABCD" "EFGH" "IJKL" "MNOP" "QRST" "UVWX" ".,;-" "=[]|";
    
    m256 A, B;
    A.D = _mm256_loadu_pd((f64 *)ValuesA);
    B.D = _mm256_loadu_pd((f64 *)ValuesB);
    
    //
    // NOTE(casey): One-input puzzles
    //
    
    Match("Puzzle0", Puzzle0(A), "ijkl" "mnop" "abcd" "efgh" "1234" "5678" "qrst" "uvwx");
    Match("Puzzle1", Puzzle1(A), "5678" "qrst" "ijkl" "uvwx" "1234" "abcd" "efgh" "mnop");
    Match("Puzzle2", Puzzle2(A), "keia" "dbcm" "fglj" "nohp" "sr5t" "uq8w" "x123" "4v67");
    Match("Puzzle3", Puzzle3(A), "3fch" "1267" "ab8d" "45eg" "uxlq" "vmin" "opwj" "ktrs");
    
    //
    // NOTE(casey): Two-input puzzles
    //
    
    Match("Puzzle4", Puzzle4(A, B), "ijkl" "mnop" "ABCD" "EFGH" "qrst" "uvwx" ".,;-" "=[]|");
    Match("Puzzle5", Puzzle5(A, B), "ijkl" "abcd" "MNOP" "EFGH" "1234" "qrst" "=[]|" "UVWX");
    Match("Puzzle6", Puzzle6(A, B), ";V=0" "X.|W" "T,Q]" "URS-" "u7x8" "0123" "rsqv" "04wt");
    
    return 0;
}