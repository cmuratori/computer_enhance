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
; LISTING 61
; ========================================================================

bits 16

mov     di, 8      ; Count
mov     bp, 1000   ; Input
mov     byte [bp + 0], 9  ; Fake input
mov     byte [bp + 1], 17
mov     byte [bp + 2], 23
mov     byte [bp + 3], 4
mov     byte [bp + 4], 27
mov     byte [bp + 5], 41
mov     byte [bp + 6], 39
mov     byte [bp + 7], 31

; Lightly modified from CLANG:

test    di, di
je      .LBB3_1
mov     cx, di
xor     dx, dx
xor     ax, ax
xor     di, di

.LBB3_5:
	add     al, byte [bp + di]
	add     bl, byte [bp + di + 1]
	add     di, 2
	cmp     di, cx
	jb      .LBB3_5
	add     al, bl
	ret
	
.LBB3_1:
	xor     ax, ax
	ret

