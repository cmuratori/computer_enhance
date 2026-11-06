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
;  LISTING 159
;  ========================================================================

global StandardWrite
global StreamingWrite

section .text

;
; NOTE(casey): These ASM routines are written for the Windows 64-bit ABI.
;
;    rcx: block count
;    rdx: source pointer
;     r8: inner loop count
;     r9: dest pointer
;

StandardWrite:
    align 64

.outer_loop:
    mov r10, r8  ; Reset the inner loop counter
    mov rax, rdx ; Reset the source pointer to the beginning of the input

.inner_loop:
    ; Read and write 256 bytes of data
    vmovdqa ymm0, [rax]
    vmovdqa [r9], ymm0
    vmovdqa ymm0, [rax + 0x20]
    vmovdqa [r9 + 0x20], ymm0
    vmovdqa ymm0, [rax + 0x40]
    vmovdqa [r9 + 0x40], ymm0
    vmovdqa ymm0, [rax + 0x60]
    vmovdqa [r9 + 0x60], ymm0
    vmovdqa ymm0, [rax + 0x80]
    vmovdqa [r9 + 0x80], ymm0
    vmovdqa ymm0, [rax + 0xa0]
    vmovdqa [r9 + 0xa0], ymm0
    vmovdqa ymm0, [rax + 0xc0]
    vmovdqa [r9 + 0xc0], ymm0
    vmovdqa ymm0, [rax + 0xe0]
    vmovdqa [r9 + 0xe0], ymm0
    add rax, 0x100  ; Advance the source pointer by 256 bytes
    add r9, 0x100   ; Advance the dest pointer by 256 bytes
    dec r10
    jnz .inner_loop

    dec rcx
    jnz .outer_loop
    ret


StreamingWrite:
    align 64

.outer_loop:
    mov r10, r8  ; Reset the inner loop counter
    mov rax, rdx ; Reset the source pointer to the beginning of the input

.inner_loop:
    ; Read and write 256 bytes of data
    vmovdqa ymm0, [rax]
    vmovntdq [r9], ymm0
    vmovdqa ymm0, [rax + 0x20]
    vmovntdq [r9 + 0x20], ymm0
    vmovdqa ymm0, [rax + 0x40]
    vmovntdq [r9 + 0x40], ymm0
    vmovdqa ymm0, [rax + 0x60]
    vmovntdq [r9 + 0x60], ymm0
    vmovdqa ymm0, [rax + 0x80]
    vmovntdq [r9 + 0x80], ymm0
    vmovdqa ymm0, [rax + 0xa0]
    vmovntdq [r9 + 0xa0], ymm0
    vmovdqa ymm0, [rax + 0xc0]
    vmovntdq [r9 + 0xc0], ymm0
    vmovdqa ymm0, [rax + 0xe0]
    vmovntdq [r9 + 0xe0], ymm0
    add rax, 0x100  ; Advance the source pointer by 256 bytes
    add r9, 0x100   ; Advance the dest pointer by 256 bytes
    dec r10
    jnz .inner_loop

    dec rcx
    jnz .outer_loop
    ret
