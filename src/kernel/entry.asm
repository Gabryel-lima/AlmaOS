; entry.asm - Stub 32-bit que recebe controle do boot.asm e chama kernel_main em C.

bits 32

section .note.GNU-stack noalloc noexec nowrite progbits

section .text._start

global _start
extern kernel_main

_start:
    call kernel_main
    cli
.hang:
    hlt
    jmp .hang
