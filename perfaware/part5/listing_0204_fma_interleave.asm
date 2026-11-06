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
;  LISTING 204
;  ========================================================================

global FMADepChain1ASM
global FMADepChain2ASM
global FMADepChain4ASM
global FMADepChain8ASM
global FMADepChain8x2ASM

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
%endmacro

%macro RestoreVolatileYMMs 0
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

FMADepChain1ASM:
  SaveVolatileYMMs

  .ChainLoop:

    vxorpd ymm0, ymm0

    vxorpd ymm8, ymm8
    vxorpd ymm9, ymm9

    mov r8, rdx

    .LengthLoop:

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm0, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm0, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm0, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm0, ymm8, ymm9

      sub r8,8
      jnz .LengthLoop

    sub rcx,1
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret


FMADepChain2ASM:
  SaveVolatileYMMs

  .ChainLoop:

    vxorpd ymm0, ymm0
    vxorpd ymm1, ymm1

    vxorpd ymm8, ymm8
    vxorpd ymm9, ymm9

    mov r8, rdx

    .LengthLoop:

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      sub r8,4
      jnz .LengthLoop

    sub rcx,2
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret


FMADepChain4ASM:
  SaveVolatileYMMs

  .ChainLoop:

    vxorpd ymm0, ymm0
    vxorpd ymm1, ymm1
    vxorpd ymm2, ymm2
    vxorpd ymm3, ymm3

    vxorpd ymm8, ymm8
    vxorpd ymm9, ymm9

    mov r8, rdx

    .LengthLoop:

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm2, ymm8, ymm9
      vfmadd132pd ymm3, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm2, ymm8, ymm9
      vfmadd132pd ymm3, ymm8, ymm9

      sub r8,2
      jnz .LengthLoop

    sub rcx,4
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret


FMADepChain8ASM:
  SaveVolatileYMMs

  .ChainLoop:

    vxorpd ymm0, ymm0
    vxorpd ymm1, ymm1
    vxorpd ymm2, ymm2
    vxorpd ymm3, ymm3
    vxorpd ymm4, ymm4
    vxorpd ymm5, ymm5
    vxorpd ymm6, ymm6
    vxorpd ymm7, ymm7

    vxorpd ymm8, ymm8
    vxorpd ymm9, ymm9

    mov r8, rdx

    .LengthLoop:

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm2, ymm8, ymm9
      vfmadd132pd ymm3, ymm8, ymm9

      vfmadd132pd ymm4, ymm8, ymm9
      vfmadd132pd ymm5, ymm8, ymm9

      vfmadd132pd ymm6, ymm8, ymm9
      vfmadd132pd ymm7, ymm8, ymm9

      sub r8,1
      jnz .LengthLoop

    sub rcx,8
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret


FMADepChain8x2ASM:
  SaveVolatileYMMs

  .ChainLoop:

    vxorpd ymm0, ymm0
    vxorpd ymm1, ymm1
    vxorpd ymm2, ymm2
    vxorpd ymm3, ymm3
    vxorpd ymm4, ymm4
    vxorpd ymm5, ymm5
    vxorpd ymm6, ymm6
    vxorpd ymm7, ymm7

    vxorpd ymm8, ymm8
    vxorpd ymm9, ymm9

    mov r8, rdx

    .LengthLoop:

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm2, ymm8, ymm9
      vfmadd132pd ymm3, ymm8, ymm9

      vfmadd132pd ymm4, ymm8, ymm9
      vfmadd132pd ymm5, ymm8, ymm9

      vfmadd132pd ymm6, ymm8, ymm9
      vfmadd132pd ymm7, ymm8, ymm9

      vfmadd132pd ymm0, ymm8, ymm9
      vfmadd132pd ymm1, ymm8, ymm9

      vfmadd132pd ymm2, ymm8, ymm9
      vfmadd132pd ymm3, ymm8, ymm9

      vfmadd132pd ymm4, ymm8, ymm9
      vfmadd132pd ymm5, ymm8, ymm9

      vfmadd132pd ymm6, ymm8, ymm9
      vfmadd132pd ymm7, ymm8, ymm9

      sub r8,2
      jnz .LengthLoop

    sub rcx,8
    jnz .ChainLoop

  RestoreVolatileYMMs
  ret

