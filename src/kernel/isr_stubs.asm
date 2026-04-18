; isr_stubs.asm - Stubs de assembly para ISRs (excecoes) e IRQs (hardware).
; Cada stub empilha o numero da interrupcao e chama o handler comum.

bits 32

section .text

extern isr_dispatch

; ---- Macro para ISR sem error code ----
%macro ISR_NOERR 1
global isr%1
isr%1:
    push dword 0            ; error code dummy
    push dword %1           ; numero da ISR
    jmp isr_common
%endmacro

; ---- Macro para ISR com error code (empilhado pela CPU) ----
%macro ISR_ERR 1
global isr%1
isr%1:
    push dword %1           ; numero da ISR (error code ja esta na stack)
    jmp isr_common
%endmacro

; ---- Excecoes da CPU (ISR 0-31) ----
ISR_NOERR 0     ; Division by Zero
ISR_NOERR 1     ; Debug
ISR_NOERR 2     ; NMI
ISR_NOERR 3     ; Breakpoint
ISR_NOERR 4     ; Overflow
ISR_NOERR 5     ; Bound Range Exceeded
ISR_NOERR 6     ; Invalid Opcode
ISR_NOERR 7     ; Device Not Available
ISR_ERR   8     ; Double Fault
ISR_NOERR 9     ; Coprocessor Segment Overrun (legacy)
ISR_ERR   10    ; Invalid TSS
ISR_ERR   11    ; Segment Not Present
ISR_ERR   12    ; Stack-Segment Fault
ISR_ERR   13    ; General Protection Fault
ISR_ERR   14    ; Page Fault
ISR_NOERR 15    ; Reserved
ISR_NOERR 16    ; x87 Floating-Point Exception
ISR_ERR   17    ; Alignment Check
ISR_NOERR 18    ; Machine Check
ISR_NOERR 19    ; SIMD Floating-Point Exception
ISR_NOERR 20    ; Virtualization Exception
ISR_ERR   21    ; Control Protection Exception
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_NOERR 30
ISR_NOERR 31

; ---- IRQs de hardware (remapeados para ISR 32-47) ----
ISR_NOERR 32    ; IRQ 0 - PIT Timer
ISR_NOERR 33    ; IRQ 1 - Keyboard
ISR_NOERR 34    ; IRQ 2 - Cascade (slave PIC)
ISR_NOERR 35    ; IRQ 3 - COM2
ISR_NOERR 36    ; IRQ 4 - COM1
ISR_NOERR 37    ; IRQ 5 - LPT2
ISR_NOERR 38    ; IRQ 6 - Floppy
ISR_NOERR 39    ; IRQ 7 - LPT1 / Spurious
ISR_NOERR 40    ; IRQ 8 - RTC
ISR_NOERR 41    ; IRQ 9
ISR_NOERR 42    ; IRQ 10
ISR_NOERR 43    ; IRQ 11
ISR_NOERR 44    ; IRQ 12 - PS/2 Mouse
ISR_NOERR 45    ; IRQ 13 - FPU
ISR_NOERR 46    ; IRQ 14 - Primary ATA
ISR_NOERR 47    ; IRQ 15 - Secondary ATA

; ---- Handler comum para todas as interrupcoes ----
isr_common:
    pushad                  ; salva EAX, ECX, EDX, EBX, ESP, EBP, ESI, EDI
    push ds
    push es
    push fs
    push gs

    mov ax, 0x10            ; seletor de dados do kernel
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp                ; ponteiro para o interrupt_frame_t na stack
    call isr_dispatch
    add esp, 4

    pop gs
    pop fs
    pop es
    pop ds
    popad
    add esp, 8              ; remove ISR number e error code
    iret

; ---- Tabela de ponteiros para os stubs (usada por idt_init) ----
section .note.GNU-stack noalloc noexec nowrite progbits

section .data

global isr_stub_table
isr_stub_table:
%assign i 0
%rep 48
    dd isr%+i
%assign i i+1
%endrep
