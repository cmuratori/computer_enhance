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
; LISTING 44
; ========================================================================

bits 16

mov ax, 1
mov bx, 2
mov cx, 3
mov dx, 4

mov sp, ax
mov bp, bx
mov si, cx
mov di, dx

mov dx, sp
mov cx, bp
mov bx, si
mov ax, di
