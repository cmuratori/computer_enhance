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
; LISTING 57
; ========================================================================

bits 16

mov bx, 1000
mov bp, 2000
mov si, 3000
mov di, 4000

mov cx, [bp + di]
mov [bx + si], cx

mov cx, [bp + si]
mov [bx + di], cx

mov cx, [bp + di + 1000]
mov [bx + si + 1000], cx

mov cx, [bp + si + 1000]
mov [bx + di + 1000], cx

add dx, [bp + si + 1000]

add word [bp + si], 76

add dx, [bp + si + 1001]
add [di + 999], dx
add word [bp + si], 75
