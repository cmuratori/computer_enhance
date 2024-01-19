;  ========================================================================
;
;  (C) Copyright 2023 by Molly Rocket, Inc., All Rights Reserved.
;
;  This software is provided 'as-is', without any express or implied
;  warranty. In no event will the authors be held liable for any damages
;  arising from the use of this software.
;
;  Please see https://computerenhance.com for more information
;
;  ========================================================================

;  ========================================================================
;  LISTING 143
;  ========================================================================

;
;  NOTE(casey): Regular Homework
;
    mov rax, 1
    mov rbx, 2
    mov rcx, 3
    mov rdx, 4
    add rax, rbx
    add rcx, rdx
    add rax, rcx
    mov rcx, rbx
    inc rax
    dec rcx
    sub rax, rbx
    sub rcx, rdx
    sub rax, rcx

;
;  NOTE(casey): CHALLENGE MODE WITH ULTIMATE DIFFICULTY SETTINGS
;               DO NOT ATTEMPT THIS! IT IS MUCH TOO HARD FOR
;               A HOMEWORK ASSIGNMENT!1!11!!
;
top:
    pop rcx
    sub rsp, rdx
    mov rbx, rax
    shl rbx, 0
    not rbx
    loopne top
