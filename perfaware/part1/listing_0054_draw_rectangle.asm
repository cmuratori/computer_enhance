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
; LISTING 54
; ========================================================================

bits 16

; Start image after one row, to avoid overwriting our code!
mov bp, 64*4

mov dx, 0
y_loop_start:
	
	mov cx, 0
	x_loop_start:
		; Fill pixel
		mov word [bp + 0], cx ; Red
		mov word [bp + 2], dx ; Blue
		mov byte [bp + 3], 255 ; Alpha
			
		; Advance pixel location
		add bp, 4
			
		; Advance X coordinate and loop
		add cx, 1
		cmp cx, 64
		jnz x_loop_start
	
	; Advance Y coordinate and loop
	add dx, 1
	cmp dx, 64
	jnz y_loop_start
