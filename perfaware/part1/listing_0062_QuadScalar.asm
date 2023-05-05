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
; LISTING 62
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
je      .LBB4_1
mov     cx, di
xor     dx, dx
xor     ax, ax
xor     bx, bx
xor     di, di

.LBB4_5:
	add     al, [bp + di]
	add     bl, [bp + di + 1]
	add     bh, [bp + di + 2]
	add     dl, [bp + di + 3]
	add     di, 4
	cmp     di, cx
	jb      .LBB4_5
	add     al, bl
	add     bh, dl
	add     al, bh
	ret
	
.LBB4_1:
	xor     ax, ax
	ret
		