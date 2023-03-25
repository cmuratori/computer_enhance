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

mov ax, 2222h
mov bx, 4444h
mov cx, 6666h
mov dx, 8888h

mov ss, ax
mov ds, bx
mov es, cx

mov al, 11h
mov bh, 33h
mov cl, 55h
mov dh, 77h

mov ah, bl
mov cl, dh

mov ss, ax
mov ds, bx
mov es, cx

mov sp, ss
mov bp, ds
mov si, es
mov di, dx
