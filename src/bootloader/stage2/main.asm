bits 16         ; Continuamos em modo real de 16 bits

section _ENTRY class=CODE ; Seção de código para o ponto de entrada do bootloader

extern _cstart 
global entry

entry:
    ; Configura uma stack fixa reservada para o stage2 e o kernel inicial
    cli
    mov ax, 0x7400
    mov ss, ax
    mov sp, 0x4000
    mov bp, sp
    sti

    ; Chama a função principal do bootloader, passando o número da unidade de boot (bootDrive) como argumento
    xor dh, dh
    push dx
    call _cstart

    ; Se a função _cstart retornar, entra em um loop infinito para evitar que o sistema continue a execução
    cli
    hlt
