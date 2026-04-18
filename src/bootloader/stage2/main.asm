bits 16         ; Continuamos em modo real de 16 bits

section _ENTRY class=CODE ; Seção de código para o ponto de entrada do bootloader

extern _cstart 
global entry

entry:
    ; Configura a stack dentro do mesmo segmento de dados.
    ; O modelo small do OpenWatcom exige SS == DS para que ponteiros near
    ; de variaveis locais (stack) sejam acessados corretamente via DS.
    cli
    mov ax, ds              ; DS ja configurado pelo stage1 (0x2000)
    mov ss, ax              ; SS = DS (exigido pelo modelo small)
    mov sp, 0xFFF0          ; stack no topo do segmento de 64KB, acima do codigo+dados (~10KB)
    mov bp, sp
    sti

    ; Chama a função principal do bootloader, passando o número da unidade de boot (bootDrive) como argumento
    xor dh, dh
    push dx
    call _cstart

    ; Se a função _cstart retornar, entra em um loop infinito para evitar que o sistema continue a execução
    cli
    hlt
