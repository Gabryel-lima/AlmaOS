; boot.asm - Ponto de entrada do kernel em modo real.
; Habilita A20, carrega GDT, entra em modo protegido e salta para o kernel em C.

org 0
bits 16

KERNEL_PHYS_BASE equ 0x12000       ; endereco fisico onde o stage2 carrega o kernel
KERNEL_C_ENTRY   equ 0x12200       ; entry point do kernel em C (base + 512 bytes)
GDT_CODE_SEL     equ 0x08          ; seletor do segmento de codigo 32-bit
GDT_DATA_SEL     equ 0x10          ; seletor do segmento de dados 32-bit
GDT_CODE16_SEL   equ 0x18          ; seletor de codigo 16-bit (usado pelo trampolim de modo real)
GDT_DATA16_SEL   equ 0x20          ; seletor de dados 16-bit (usado pelo trampolim de modo real)
KERNEL_STACK_TOP equ 0x78000       ; topo da stack do kernel (cresce para baixo)

start:
    cli
    push cs
    pop ds                              ; DS = CS = 0x1200

    ; ---- Habilita A20 via System Control Port A (fast A20) ----
    in al, 0x92
    test al, 2
    jnz .a20_done
    or al, 2
    and al, 0xFE                        ; evita reset acidental do sistema
    out 0x92, al
.a20_done:

    ; ---- Carrega a GDT ----
    lgdt [gdt_desc]

    ; ---- Habilita modo protegido (PE bit no CR0) ----
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    ; ---- Far jump para codigo 32-bit (atualiza CS com seletor de codigo) ----
    ; Codificacao manual: 0x66 (operand-size override) + 0xEA (far jump)
    db 0x66, 0xEA
    dd (pm_entry + KERNEL_PHYS_BASE)    ; endereco fisico absoluto de pm_entry
    dw GDT_CODE_SEL                     ; seletor de codigo da GDT

bits 32
pm_entry:
    ; Configura todos os registradores de segmento com seletor flat de dados
    mov ax, GDT_DATA_SEL
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Configura stack pointer
    mov esp, KERNEL_STACK_TOP

    ; Salta para o entry point do kernel C
    ; Usa registrador para evitar bug de enderecamento relativo com org 0
    mov eax, KERNEL_C_ENTRY
    jmp eax

    ; Nunca deve chegar aqui
    cli
    hlt

; ---- Global Descriptor Table ----
align 8
gdt_start:
    ; Descritor nulo (obrigatorio, indice 0)
    dq 0
    ; Segmento de codigo: base=0, limite=4GB, 32-bit, ring 0, execute/read
    dw 0xFFFF                           ; limite low (bits 0-15)
    dw 0x0000                           ; base low (bits 0-15)
    db 0x00                             ; base mid (bits 16-23)
    db 0x9A                             ; access: present, DPL=0, code, execute/read
    db 0xCF                             ; flags: granularidade 4KB, 32-bit + limite high
    db 0x00                             ; base high (bits 24-31)
    ; Segmento de dados: base=0, limite=4GB, 32-bit, ring 0, read/write
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92                             ; access: present, DPL=0, data, read/write
    db 0xCF
    db 0x00
    ; Segmento de codigo 16-bit: limite=64KB, granularidade de byte.
    ; Existe para o trampolim de modo real (src/kernel/realmode.asm): antes de
    ; limpar o bit PE e preciso recarregar CS/SS com descritores de 16 bits,
    ; senao o cache oculto do descritor continua com atributos de 32 bits e
    ; 4 GB e o modo real se comporta de forma imprevisivel.
    ;
    ; A base fica zerada aqui e e preenchida por realmode_init() com o
    ; endereco do stub (REALMODE_STUB_ADDR, em include/realmode.h). O limite de
    ; 64 KB e o proprio motivo: com base 0 nao daria para alcancar o stub, que
    ; mora acima de 0xFFFF. A base tambem tem de bater com a do segmento de
    ; modo real usado depois de PE=0, senao o far jump cai em outro lugar.
    dw 0xFFFF                           ; limite low
    dw 0x0000                           ; base low   (preenchido em runtime)
    db 0x00                             ; base mid   (preenchido em runtime)
    db 0x9A                             ; access: present, DPL=0, code, execute/read
    db 0x00                             ; flags: granularidade de byte, 16-bit
    db 0x00                             ; base high  (preenchido em runtime)
    ; Segmento de dados 16-bit: mesma base e mesmo limite do de codigo.
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 0x92                             ; access: present, DPL=0, data, read/write
    db 0x00
    db 0x00
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1         ; limite da GDT (tamanho - 1)
    dd gdt_start + KERNEL_PHYS_BASE    ; base da GDT (endereco fisico linear)

; Preenche ate exatamente 512 bytes
times 512 - ($ - $$) db 0
