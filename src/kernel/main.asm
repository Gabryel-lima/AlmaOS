org 0x7E00           ; O kernel será carregado no endereço 0x7E00, logo após o bootloader, então definimos a origem para esse endereço
bits 16              ; Continuamos em modo real de 16 bits

%define ENDL 0x0D, 0x0A         ; Constante para nova linha

;
; Ponto de entrada do kernel
; O bootloader deve carregar este código para a memória e saltar para ele (kernel)
start:
    mov si, msg_kernel          ; Prepara a mensagem do kernel
    call puts                   ; Chama a rotina de impressão

;
; Imprime uma string na tela usando interrupções do BIOS.
; A string deve ser terminada com nulo. (kernel)
puts:
    push ax          ; salva registradores usados por esta rotina
    push si          ; salva o índice da string

    .halt:
        jmp .halt        ; Trava a execução do kernel aqui

    .loop:
        lodsb            ; carrega o próximo byte da string em AL
        or al, al        ; zero? fim da string
        jz .done         ; se sim, terminamos

        mov ah, 0x0E     ; saída teletipo do BIOS
        mov bh, 0x00     ; página 0
        int 0x10         ; imprime AL
        jmp .loop        ; continua com o próximo caractere

    .done:
        pop si           ; restaura registradores
        pop ax           ; restaura AX
        ret              ; retorna ao chamador

msg_kernel:          ; A mensagem de boas-vindas do kernel, seguida por um terminador nulo
    db 'Welcome to AlmaOS Kernel!', ENDL, 0          ; A string seguida por um terminador nulo
