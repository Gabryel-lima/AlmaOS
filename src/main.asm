org 0x7C00           ; O BIOS carrega o bootloader no endereço 0x7C00, então definimos a origem para esse endereço
bits 16              ; Estamos escrevendo código de 16 bits para a arquitetura x86

%define ENDL 0x0D, 0x0A         ; Define uma constante para os caracteres de nova linha (retorno de carro e avanço de linha)

start:
    jmp main         ; Salta para a função main para iniciar a execução

;
; Imprime uma string na tela usando interrupções do BIOS.
; A string deve ser terminada com nulo.
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
    pop ax
    ret              ; retorna ao chamador

main:

    ; Configura os registradores de segmento e o ponteiro de pilha.
    mov ax, 0         ; Define AX como 0, que será usado para os registradores de segmento
    mov ds, ax        ; Segmento de dados é definido para 0, pois usaremos endereçamento absoluto
    mov es, ax        ; Segmento extra não é usado, mas definimos como 0 por segurança

    ; Configura o segmento de pilha e o ponteiro de pilha.
    mov ss, ax        ; Segmento de pilha é definido como 0
    mov sp, 0x7C00    ; Pilha cresce para baixo a partir do final do bootloader

    mov si, msg_hello ; Carrega o endereço da string em SI
    call puts         ; Chama a função puts para imprimir a string

.halt:
    jmp .halt         ; Loop infinito para travar a CPU após imprimir a string

msg_hello:
    db 'Hello, World!', ENDL, 0         ; A string "Hello, World!" seguida por um terminador nulo

times 510 - ($ - $$) db 0         ; Preenche o restante do bootloader com zeros até atingirmos 510 bytes   
dw 0xAA55         ; Assinatura de boot, deve ser 0xAA55 para o BIOS reconhecer isto como um disco inicializável
