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
; LISTING 51
; ========================================================================

bits 16

mov word [1000], 1
mov word [1002], 2
mov word [1004], 3
mov word [1006], 4

mov bx, 1000
mov word [bx + 4], 10

mov bx, word [1000]
mov cx, word [1002]
mov dx, word [1004]
mov bp, word [1006]
