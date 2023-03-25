; ========================================================================
;
; (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
; This software is provided 'as-is', without any express or implied
; warranty. In no event will the authors be held liable for any damages
; arising from the use of this software.
;
; Please see https://computerenhance.com for further information
;
; ========================================================================

; ========================================================================
; LISTING 45
; ========================================================================

bits 16

mov ax, 0x2222
mov bx, 0x4444
mov cx, 0x6666
mov dx, 0x8888

mov ss, ax
mov ds, bx
mov es, cx

mov al, 0x11
mov bh, 0x33
mov cl, 0x55
mov dh, 0x77

mov ah, bl
mov cl, dh

mov ss, ax
mov ds, bx
mov es, cx

mov sp, ss
mov bp, ds
mov si, es
mov di, dx
