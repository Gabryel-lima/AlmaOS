bits 16         ; Continuamos em modo real de 16 bits

section _TEXT class=CODE ; Seção de código para o bootloader, onde o código executável será colocado

global _x86_Video_WriteCharTeletype ; Tornamos a função de escrita de caractere teletipo global para que possa ser chamada de C (x86)

;
; Função para escrever um caractere usando o modo teletipo do BIOS (x86)
; Parâmetros:
;   - AL: O caractere a ser impresso
;   - BH: A página de vídeo (normalmente 0)
_x86_Video_WriteCharTeletype:
    ; Salva os registradores usados por esta rotina
    push bp        ; Salva o ponteiro de base
    mov bp, sp     ; Estabelece o ponteiro de base para acessar os parâmetros passados na pilha

    ; Parâmetros são passados na pilha, então precisamos acessar os valores usando BP
    push bx        ; Salva BX, que usaremos para acessar os parâmetros

    ; Configura os registradores para a chamada de interrupção do BIOS
    mov ah, 0x0E   ; Função de escrita de caractere teletipo do BIOS
    mov al, [bp + 4] ; O caractere a ser impresso está no primeiro parâmetro (offset 4 da pilha)
    mov bh, [bp + 6] ; A página de vídeo está no segundo parâmetro (offset 6 da pilha)

    int 0x10    ; Chama a interrupção de vídeo para imprimir o caractere

    pop bx      ; Restaura BX

    ; Restaura os registradores e retorna
    mov sp, bp  ; Restaura o ponteiro de pilha
    pop bp      ; Restaura o ponteiro de base
    ret         ; Retorna ao chamador
