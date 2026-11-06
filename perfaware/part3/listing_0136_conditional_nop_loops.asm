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
;  LISTING 136
;  ========================================================================

global ConditionalNOP

section .text

;
; NOTE(casey): These ASM routines are written for the Windows
; 64-bit ABI. They expect RCX to be the first parameter (the count),
; and if applicable, RDX to be the second parameter (the data pointer).
; To use these on a platform with a different ABI, you would have to
; change those registers to match the ABI.
;

ConditionalNOP:
    xor rax, rax
.loop:
    mov r10, [rdx + rax]
	inc rax
	test r10, 1
    jnz .skip
	nop
.skip:
    cmp rax, rcx
    jb .loop
    ret
