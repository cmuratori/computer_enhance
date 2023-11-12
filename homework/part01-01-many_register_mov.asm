; ========================================================================
; Here we are giving (encoding) CPU instructions on what to do.

; We want to be copying the contents from one 16 bit register to another.
; The instruction that copy bits from one place to another, is called "mov".
; To target all 16 bits we use the x-suffix.
; To only target the high or low bits, we could use the h- or l-suffixes.

; The destination register comes first, followed by the source register.

; To assemble these files, we need to use an assembler, such as The Netwide Assembler, NASM.
; ========================================================================
; LISTING 38
; ========================================================================

bits 16

mov cx, bx
mov ch, ah
mov dx, bx
mov si, bx
mov bx, di
mov al, cl
mov ch, ch
mov bx, ax
mov bx, si
mov sp, di
mov bp, ax