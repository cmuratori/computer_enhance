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
; LISTING 64
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

xor     ax, ax
cmp     di, 4
jb      .LBB6_3
shr     di, 1
shr     di, 1
xor     ax, ax

.LBB6_2:
	add     al, byte [bp]
	add     al, byte [bp + 1]
	add     al, byte [bp + 2]
	add     al, byte [bp + 3]
	add     bp, 4
	dec     di
	jne     .LBB6_2
		
.LBB6_3:
	ret
