org 0x0000           ; O stage2 carrega este payload no segmento 0x1200 com offset 0x0000
bits 16              ; Continuamos em modo real de 16 bits

%define ENDL 0x0D, 0x0A         ; Constante para nova linha

;
; Ponto de entrada do kernel
; O bootloader deve carregar este código para a memória e saltar para ele (kernel)
start:
    push cs                     ; O kernel precisa usar o próprio segmento como DS/ES
    pop ax
    mov ds, ax
    mov es, ax
    jmp main                    ; Salta para a função principal do kernel

;
; Função principal do kernel
main:
    mov si, msg_kernel          ; Prepara a mensagem do kernel
    call puts                   ; Chama a rotina de impressão
; 
; Após imprimir a mensagem, o kernel pode entrar em um loop infinito ou hibernar, aguardando uma interrupção (neste caso, não haverá, então o sistema ficará congelado)
; Aqui, optamos por usar a instrução HLT para colocar o processador em um estado de baixo consumo, aguardando uma interrupção (que não ocorrerá, então o sistema ficará congelado). Alternativamente, poderíamos usar um loop infinito, mas isso consumiria mais recursos do processador.
.halt:
    hlt      ; Para o processador, aguardando uma interrupção (que não ocorrerá)
    cli      ; Desabilita interrupções antes do loop
    jmp .halt ; Loop infinito para garantir que o sistema permaneça congelado

;
; Imprime uma string na tela usando interrupções do BIOS.
; A string deve ser terminada com nulo. (kernel)
puts:
    push ax          ; salva registradores usados por esta rotina
    push si          ; salva o índice da string

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
