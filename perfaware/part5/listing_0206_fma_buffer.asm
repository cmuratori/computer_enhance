;  ========================================================================
;
;  (C) Copyright 2026 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 206
;  ========================================================================

global FMADepChainSpacedASM

section .text

;
; NOTE(casey): These ASM routines are written for the Windows 64-bit ABI.
;
;    rcx: ChainCount
;    rdx: ChainLength

%macro SaveVolatileYMMs 0
vmovdqu [rsp-0x020], ymm6
vmovdqu [rsp-0x040], ymm7
vmovdqu [rsp-0x060], ymm8
vmovdqu [rsp-0x080], ymm9
vmovdqu [rsp-0x0a0], ymm10
vmovdqu [rsp-0x0c0], ymm11
vmovdqu [rsp-0x0e0], ymm12
vmovdqu [rsp-0x100], ymm13
vmovdqu [rsp-0x120], ymm14
vmovdqu [rsp-0x140], ymm15
sub rsp, 0x140
%endmacro

%macro RestoreVolatileYMMs 0
add rsp, 0x140
vmovdqu ymm6, [rsp-0x020]
vmovdqu ymm7, [rsp-0x040]
vmovdqu ymm8, [rsp-0x060]
vmovdqu ymm9, [rsp-0x080]
vmovdqu ymm10, [rsp-0x0a0]
vmovdqu ymm11, [rsp-0x0c0]
vmovdqu ymm12, [rsp-0x0e0]
vmovdqu ymm13, [rsp-0x100]
vmovdqu ymm14, [rsp-0x120]
vmovdqu ymm15, [rsp-0x140]
%endmacro

%define SPACING_COUNT 16
%define SPACING_SIZE (SPACING_COUNT*0x20)

%macro Constant4x 1
dq %1, %1, %1, %1
%endmacro
DummyMultA: Constant4x 1.01
DummyMultB: Constant4x 1.001
DummyAddend: Constant4x 0.01

FMADepChainSpacedASM:
  SaveVolatileYMMs

  lea rax, [rsp-SPACING_SIZE]
  and rax, ~0x1f
  mov r10, rax
  sub r10, SPACING_SIZE

  vmovdqu ymm0, [rel DummyMultA]
  mov r8, SPACING_COUNT
  .ClearLoop:
    vmovdqu [rax], ymm0
    add rax, 0x20
    dec r8
    jnz .ClearLoop

  .ChainLoop:

    mov rax, r10
    add rax, SPACING_SIZE
    mov r8, rdx
    vmovdqu ymm8, [rel DummyAddend]
    vmovdqu ymm9, [rel DummyMultB]

    .LengthLoop:

      mov r9, SPACING_COUNT
      
      .SpacingLoop:

        vmovdqu ymm0, [rax]
        add rax, 0x20

        vfmadd132pd ymm0, ymm8, ymm9
        vfmadd132pd ymm0, ymm8, ymm9

        vfmadd132pd ymm0, ymm8, ymm9
        vfmadd132pd ymm0, ymm8, ymm9

        vfmadd132pd ymm0, ymm8, ymm9
        vfmadd132pd ymm0, ymm8, ymm9

        vfmadd132pd ymm0, ymm8, ymm9
        vfmadd132pd ymm0, ymm8, ymm9

        vmovdqu [r10], ymm0
        add r10, 0x20

        dec r9
        jnz .SpacingLoop

      sub r10, SPACING_SIZE
      mov rax, r10

      sub r8, 8
      jnz .LengthLoop

    sub rcx, SPACING_COUNT
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret
