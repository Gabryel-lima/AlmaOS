org 0x7C00
bits 16

entry:
    cli
    ; configura pilha
    mov ax, 0
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 7C00h

    ; alternar para modo protegido
    cli                     ; 1 - Desabilitar interrupções
    call EnableA20          ; 2 - Habilitar a linha A20
    call LoadGDT            ; 3 - Carregar GDT

    ; 4 - definir flag de habilitar proteção em CR0
    mov eax, cr0
    or al, 1
    mov cr0, eax

    ; 5 - salto para modo protegido (far jump)
    jmp dword 08h:.pmode


.pmode:
    ; agora estamos em modo protegido!
    [bits 32]
    
    ; 6 - configurar registradores de segmento
    mov ax, 0x10
    mov ds, ax
    mov ss, ax


    ; imprimir "hello world"
    mov esi, g_Hello
    mov edi, ScreenBuffer
    cld

    mov bl, 1

.loop:
    lodsb
    or al, al
    jz .done

    mov [edi], al
    inc edi

    mov [edi], bl
    inc edi
    inc bl
    jmp .loop

.done:
    ; voltar para modo real
    jmp word 18h:.pmode16         ; 1 - salto para segmento 16-bit usado na transição

.pmode16:
    [bits 16]

    ; 2 - desabilitar o bit de modo protegido em CR0
    mov eax, cr0
    and al, ~1
    mov cr0, eax

    ; 3 - salto para modo real
    jmp word 00h:.rmode

.rmode:
    ; 4 - configurar segmentos
    mov ax, 0
    mov ds, ax
    mov ss, ax

    ; 5 - habilitar interrupções
    sti

    ; imprimir "hello world" usando int 10h
    mov si, g_HelloR

.rloop:
    lodsb
    or al, al
    jz .rdone
    mov ah, 0eh
    int 10h
    jmp .rloop

.rdone:

    ; Para voltar ao modo protegido, desabilite interrupções e repita os passos 4-6


.halt:
    jmp .halt


EnableA20:
    [bits 16]
    ; desabilitar teclado
    call A20WaitInput
    mov al, KbdControllerDisableKeyboard
    out KbdControllerCommandPort, al

    ; ler porta de controle de saída
    call A20WaitInput
    mov al, KbdControllerReadCtrlOutputPort
    out KbdControllerCommandPort, al

    call A20WaitOutput
    in al, KbdControllerDataPort
    push eax

    ; escrever porta de controle de saída
    call A20WaitInput
    mov al, KbdControllerWriteCtrlOutputPort
    out KbdControllerCommandPort, al
    
    call A20WaitInput
    pop eax
    or al, 2                                    ; bit 2 = bit A20
    out KbdControllerDataPort, al

    ; habilitar teclado
    call A20WaitInput
    mov al, KbdControllerEnableKeyboard
    out KbdControllerCommandPort, al

    call A20WaitInput
    ret


A20WaitInput:
    [bits 16]
    ; aguarda até que o bit 2 do status (input buffer) seja 0
    ; lendo a porta de comando obtemos o byte de status
    in al, KbdControllerCommandPort
    test al, 2
    jnz A20WaitInput
    ret

A20WaitOutput:
    [bits 16]
    ; aguarda até que o bit 1 do status (output buffer) seja 1 para poder ler
    in al, KbdControllerCommandPort
    test al, 1
    jz A20WaitOutput
    ret


LoadGDT:
    [bits 16]
    lgdt [g_GDTDesc]
    ret



KbdControllerDataPort               equ 0x60
KbdControllerCommandPort            equ 0x64
KbdControllerDisableKeyboard        equ 0xAD
KbdControllerEnableKeyboard         equ 0xAE
KbdControllerReadCtrlOutputPort     equ 0xD0
KbdControllerWriteCtrlOutputPort    equ 0xD1

ScreenBuffer                        equ 0xB8000

g_GDT:      ; descritor NULO
            dq 0

            ; segmento de código 32-bit
            dw 0FFFFh                   ; limite (bits 0-15) = 0xFFFFF para alcance 32-bit completo
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10011010b                ; acesso (presente, anel 0, segmento de código, executável, direção 0, legível)
            db 11001111b                ; granularidade (páginas 4K, modo protegido 32-bit) + limite (bits 16-19)
            db 0                        ; base alto

            ; segmento de dados 32-bit
            dw 0FFFFh                   ; limite (bits 0-15) = 0xFFFFF para alcance 32-bit completo
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10010010b                ; acesso (presente, anel 0, segmento de dados, não-executável, direção 0, gravável)
            db 11001111b                ; granularidade (páginas 4K, modo protegido 32-bit) + limite (bits 16-19)
            db 0                        ; base alto

            ; segmento de código 16-bit
            dw 0FFFFh                   ; limite (bits 0-15) = 0xFFFFF
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10011010b                ; acesso (presente, anel 0, segmento de código, executável, direção 0, legível)
            db 00001111b                ; granularidade (páginas 1B, modo protegido 16-bit) + limite (bits 16-19)
            db 0                        ; base alto

            ; segmento de dados 16-bit
            dw 0FFFFh                   ; limite (bits 0-15) = 0xFFFFF
            dw 0                        ; base (bits 0-15) = 0x0
            db 0                        ; base (bits 16-23)
            db 10010010b                ; acesso (presente, anel 0, segmento de dados, não-executável, direção 0, gravável)
            db 00001111b                ; granularidade (páginas 1B, modo protegido 16-bit) + limite (bits 16-19)
            db 0                        ; base alto

g_GDTDesc:  dw g_GDTDesc - g_GDT - 1    ; limite = tamanho da GDT
            dd g_GDT                    ; endereço da GDT

g_Hello:    db "Hello world from protected mode!", 0
g_HelloR:   db "Hello world from real mode!", 0

times 510-($-$$) db 0
dw 0AA55h
