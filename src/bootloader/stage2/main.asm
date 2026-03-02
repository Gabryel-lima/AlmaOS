bits 16         ; Continuamos em modo real de 16 bits

section _ENTRY class=CODE ; Seção de código para o ponto de entrada do bootloader

extern _cstart ; Declaração da função _cstart, que é a função principal do bootloader escrita em C 
global entry ; Define o símbolo 'entry' como global para que o linker possa encontrá-lo (main)

entry:
    ; Configura o segmento de pilha e o ponteiro de base para o modo protegido
    cli
    mov ax, ds
    mov ss, ax
    mov sp, 0
    mov bp, sp
    sti

    ; Chama a função principal do bootloader, passando o número da unidade de boot (bootDrive) como argumento
    xor dh, dh
    push dx
    call _cstart

    ; Se a função _cstart retornar, entra em um loop infinito para evitar que o sistema continue a execução
    cli
    hlt
