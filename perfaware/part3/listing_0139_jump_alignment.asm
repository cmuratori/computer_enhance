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
;  LISTING 139
;  ========================================================================

global NOPAligned64
global NOPAligned1
global NOPAligned15
global NOPAligned31
global NOPAligned63

section .text

;
; NOTE(casey): These ASM routines are written for the Windows
; 64-bit ABI. They expect RCX to be the first parameter (the count),
; and if applicable, RDX to be the second parameter (the data pointer).
; To use these on a platform with a different ABI, you would have to
; change those registers to match the ABI.
;

NOPAligned64:
    xor rax, rax
align 64
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

NOPAligned1:
    xor rax, rax
align 64
nop
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

NOPAligned15:
    xor rax, rax
align 64
%rep 15
nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

NOPAligned31:
    xor rax, rax
align 64
%rep 31
nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret

NOPAligned63:
    xor rax, rax
align 64
%rep 63
nop
%endrep
.loop:
    inc rax
    cmp rax, rcx
    jb .loop
    ret
