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
; LISTING 55
; ========================================================================

bits 16

; Start image after one row, to avoid overwriting our code!
mov bp, 64*4

; Draw the solid rectangle red/blue/alpha
mov dx, 64
y_loop_start:
	
	mov cx, 64
	x_loop_start:
		mov byte [bp + 0], cl  ; Red
		mov byte [bp + 1], 0   ; Green
		mov byte [bp + 2], dl  ; Blue
		mov byte [bp + 3], 255 ; Alpha
		add bp, 4
			
		loop x_loop_start
	
	sub dx, 1
	jnz y_loop_start

; Add the line rectangle green
mov bp, 64*4 + 4*64 + 4
mov bx, bp
mov cx, 62
outline_loop_start:
	
	mov byte [bp + 1], 255 ; Top line
	mov byte [bp + 61*64*4 + 1], 255 ; Bottom line
	mov byte [bx + 1], 255 ; Left line
	mov byte [bx + 61*4 + 1], 255 ; Right  line
	
	add bp, 4
	add bx, 4*64
			
	loop outline_loop_start
