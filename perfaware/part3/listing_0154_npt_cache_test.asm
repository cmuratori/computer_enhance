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
;  LISTING 154
;  ========================================================================

global DoubleLoopRead_32x8

section .text

;
; NOTE(casey): This ASM routine is written for the Windows 64-bit ABI.
;
;    rcx: block count
;    rdx: data pointer
;     r8: 256-byte reads per block
;

DoubleLoopRead_32x8:
    align 64

.outer_loop:
    mov r9, r8   ; Reset the reads-per-block counter
    mov rax, rdx ; Reset the read pointer to the beginning of the block

.inner_loop:
    ; Read 256 bytes
    vmovdqu ymm0, [rax]
    vmovdqu ymm0, [rax + 0x20]
    vmovdqu ymm0, [rax + 0x40]
    vmovdqu ymm0, [rax + 0x60]
    vmovdqu ymm0, [rax + 0x80]
    vmovdqu ymm0, [rax + 0xa0]
    vmovdqu ymm0, [rax + 0xc0]
    vmovdqu ymm0, [rax + 0xe0]
    add rax, 0x100 ; Advance the read pointer by 256 bytes
    dec r9
    jnz .inner_loop

    dec rcx
    jnz .outer_loop
    ret

