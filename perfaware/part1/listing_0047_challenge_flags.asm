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
; LISTING 47
; ========================================================================

bits 16

add bx, 30000
add bx, 10000
sub bx, 5000
sub bx, 5000

mov bx, 1
mov cx, 100
add bx, cx

mov dx, 10
sub cx, dx

add bx, 40000
add cx, -90

mov sp, 99
mov bp, 98
cmp bp, sp
