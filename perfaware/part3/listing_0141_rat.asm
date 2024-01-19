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
;  LISTING 141
;  ========================================================================

global RATAdd
global RATMovAdd

section .text

;
; NOTE(casey): These ASM routines are written for the Windows 64-bit ABI,
; and assume that rax and rcx are volatile.
;

RATAdd:
align 64
    mov rax, 1000000000
.loop:
    add rcx, 1
    add rcx, 1
    dec rax
    jnz .loop
    ret

RATMovAdd:
align 64
    mov rax, 1000000000
.loop:
    mov rcx, rax
    add rcx, 1
    mov rcx, rax
    add rcx, 1
    dec rax
    jnz .loop
    ret