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
;  LISTING 157
;  ========================================================================

global ReadStrided_32x2

section .text

;
; NOTE(casey): This ASM routine is written for the Windows 64-bit ABI.
;
;    rcx: block count
;    rdx: data pointer
;     r8: 64-byte reads per block
;     r9: stride for the read pointer
;

ReadStrided_32x2:
    align 64

.outer_loop:
    mov r10, r8  ; Reset the reads-per-block counter
    mov rax, rdx ; Reset the read pointer to the beginning of the block

.inner_loop:
    ; Read one cache line (64 bytes on x64 - you would change this to read 128 bytes for M-series, etc.)
    vmovdqu ymm0, [rax]
    vmovdqu ymm0, [rax + 0x20]
    add rax, r9 ; Advance the read pointer by the stride
    dec r10
    jnz .inner_loop

    dec rcx
    jnz .outer_loop
    ret

