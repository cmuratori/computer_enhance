;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 161
;  ========================================================================

global PeriodicRead
global PeriodicPrefetchedRead

section .text

;
; NOTE(casey): These ASM routines are written for the Windows 64-bit ABI.
;
;    rcx: outer loop count
;    rdx: source pointer
;     r8: inner loop count
;

PeriodicRead:
    align 64

.outer_loop:
    vmovdqa ymm0, [rdx]         ; Load the cache line as "data"
    vmovdqa ymm1, [rdx + 0x20]  ;
    
    mov rdx, [rdx]              ; Load the next block pointer out of the current block
    mov r10, r8                 ; Reset the inner loop counter

.inner_loop:                    ; Do a loop of pretend operations on the "data"
    vpxor ymm0, ymm1
    vpaddd ymm0, ymm1
    dec r10
    jnz .inner_loop
	
    dec rcx
    jnz .outer_loop
    ret


PeriodicPrefetchedRead:
    align 64

.outer_loop:
    vmovdqa ymm0, [rdx]         ; Load the cache line as "data"
    vmovdqa ymm1, [rdx + 0x20]  ;
    
    mov rdx, [rdx]              ; Load the next block pointer out of the current block
    mov r10, r8                 ; Reset the inner loop counter

    prefetcht0 [rdx]            ; Start prefetching the next cache line while we work on this one

.inner_loop:                    ; Do a loop of pretend operations on the "data"
    vpxor ymm0, ymm1
    vpaddd ymm0, ymm1
    dec r10
    jnz .inner_loop
	
    dec rcx
    jnz .outer_loop
    ret
