org 0x7C00           ; O BIOS carrega o bootloader no endereço 0x7C00, então definimos a origem para esse endereço
bits 16              ; Estamos escrevendo código de 16 bits para a arquitetura x86

%define ENDL 0x0D, 0x0A         ; Define uma constante para os caracteres de nova linha (retorno de carro e avanço de linha)

;
; FAT12 Header (BPB - BIOS Parameter Block)
; Necessário para que o mcopy e o sistema operacional reconheçam o disco
jmp short main       ; Pula para o início do código principal, ignorando o BPB
nop                  ; Instrução NOP para preencher o espaço entre o JMP e o início do BPB, garantindo que o BPB esteja no local correto

; O BPB (BIOS Parameter Block) contém informações sobre a estrutura do sistema de arquivos FAT12 no disquete
; Nome do fabricante (8 bytes)
bdb_oem:                    db 'MSWIN4.1'
; Bytes por setor           
bdb_bytes_per_sector:       dw 512
; Setores por cluster
bdb_sectors_per_cluster:    db 1                    
; Setores reservados (geralmente 1 para o bootloader)
bdb_reserved_sectors:       dw 1                    
; Quantidade de tabelas FAT
bdb_fat_count:              db 2                    
; Número de entradas no diretório raiz
bdb_dir_entries_count:      dw 224                  
; Total de setores no disco (1.44MB = 2880 * 512 bytes)
bdb_total_sectors:          dw 2880                 
; Tipo de mídia (F0 = Disquete 3.5")
bdb_media_descriptor_type:  db 0xF0                 
; Setores por cada tabela FAT
bdb_sectors_per_fat:        dw 9                    
; Setores por trilha
bdb_sectors_per_track:      dw 18                   
; Número de cabeças (lados do disquete)
bdb_heads:                  dw 2                    
; Setores ocultos
bdb_hidden_sectors:         dd 0                    
; Contador de setores grandes (usado se total_sectors for 0)
bdb_large_sector_count:     dd 0                    

; Registro de Boot Estendido (Extended Boot Record)
; Número do drive (0x00 para disquete, 0x80 para HD)
ebr_drive_number:           db 0                    
; Reservado (usado pelo Windows NT)
                            db 0                    
; Assinatura do EBR (0x28 ou 0x29)
ebr_signature:              db 0x29                 
; ID do volume (número de série aleatório)
ebr_volume_id:              db 0x12, 0x34, 0x56, 0x78   
; Rótulo do volume (Exatamente 11 bytes, completar com espaços)
ebr_volume_label:           db 'ALMA OS :) '        
; Tipo do sistema de arquivos (Exatamente 8 bytes)
ebr_system_id:              db 'FAT12   '           

;
; Função principal do bootloader
main:
    ; Configura os registradores de segmento e o ponteiro de pilha.
    mov ax, 0         ; Define AX como 0, que será usado para os registradores de segmento
    mov ds, ax        ; Segmento de dados é definido para 0, pois usaremos endereçamento absoluto
    mov es, ax        ; Segmento extra não é usado, mas definimos como 0 por segurança

    ; Configura o segmento de pilha e o ponteiro de pilha.
    mov ss, ax        ; Segmento de pilha é definido como 0
    mov sp, 0x7C00    ; Pilha cresce para baixo a partir do final do bootloader

    ; Salva o número do drive (fornecido pelo BIOS em DL) para uso posterior, como leitura do disco
    push es           ; Salva ES para preservar seu valor, pois o número do drive será armazenado no EBR
    push word .after  ; Salva o endereço de retorno para depois de salvar o número do drive
    retf              ; Retorna para o endereço após a instrução de salvamento do número do drive, continuando a execução normal do bootloader

    .after:
        ; lê algo do floppy disk
        ; BIOS deve definir DL para o número do driver
        mov [ebr_drive_number], dl ; Armazena o número do drive no EBR para uso posterior

        ;
        mov si, msg_boot  ; Carrega o endereço da string em SI
        call puts         ; Chama a função puts para imprimir a string

        ; Lê o cabeçalho do disco para obter informações sobre a geometria do disco (setores por trilha e número de cabeças)
        push es           ; Salva ES para a chamada de leitura do disco
        mov ah, 0x08      ; Função de leitura de cabeçalho do disco do BIOS
        int 0x13          ; Chama a interrupção do BIOS para ler o cabeçalho do disco, que contém informações sobre a geometria do disco
        jc floppy_error   ; Se a leitura falhar, mostra uma mensagem de erro
        pop es            ; Restaura ES após a leitura do disco

        ; Extrai as informações de setores por trilha e número de cabeças do cabeçalho do disco
        and cl, 0x3F      ; Os setores por trilha estão nos bits 0-5 de CL, então aplicamos uma máscara para extrair apenas esses bits
        xor ch, ch        ; Limpa CH para garantir que o valor do cilindro seja apenas o resultado da divisão
        mov [bdb_sectors_per_track], cx ; Armazena os setores por trilha no BPB

        ; O número de cabeças está em DH, mas começa a contar a partir de 0, então incrementamos para obter o número real de cabeças
        inc dh            ; Os cabeças estão em DH, mas começam a contar a partir de 0, então incrementamos para obter o número real de cabeças
        movzx ax, dh      ; O número de cabeças está em DH, mas começa a contar a partir de 0, então incrementamos para obter o número real de cabeças. Como o número de cabeças pode ser maior que 255, usamos movzx para garantir que o valor seja corretamente armazenado em um registrador de 16 bits (AX) antes de armazenar no BPB
        mov [bdb_heads], ax ; Armazena o número de cabeças no BPB

        ; Calcula o deslocamento do diretório raiz com base nas informações de geometria do disco
        mov ax, [bdb_sectors_per_fat] ; Calcula o deslocamento do diretório raiz, que é o número de setores reservados + o número total de setores usados pelas tabelas FAT
        mov bl, [bdb_fat_count] ; O número total de setores usados pelas tabelas FAT é o número de setores por FAT multiplicado pelo número de FATs
        mul bl            ; Calcula o número total de setores usados pelas tabelas FAT (set
        add ax, [bdb_reserved_sectors] ; Adiciona os setores reservados para obter o deslocamento do diretório raiz
        push ax           ; Salva o deslocamento do diretório raiz para uso posterior, se necessário

        ; Calcula o deslocamento do stage2 com base nas informações de geometria do disco
        mov ax, [bdb_dir_entries_count] ; O número de setores usados pelo diretório raiz é o número de entradas no diretório raiz multiplicado pelo tamanho de cada entrada (32 bytes), dividido pelo tamanho do setor (512 bytes)
        shl ax, 5         ; Multiplica o número de setores por FAT por 32 (shl ax, 5 é equivalente a ax * 32) para obter o número total de setores usados pelas FATs, que é o deslocamento do stage2 em setores
        xor dx, dx        ; Limpa DX para a divisão de 32 bits (AX:DX / divisor)
        div word [bdb_bytes_per_sector] ; Divide o número total de setores usados pelas FATs pelo tamanho do setor para obter o número de setores usados pelas FATs

        ; Verifica se o número de trilhas usadas pelas FATs é 0, o que indicaria um erro na geometria do disco. Se for 0, o stage2 pode ser carregado imediatamente após o diretório raiz. Caso contrário, incrementamos o número de trilhas para garantir que o stage2 seja carregado após as FATs.
        test dx, dx       ; Verifica se o número de trilhas usadas pelas FATs é 0 (o que indicaria um erro na geometria do disco)
        jz .root_dir_after ; Se não houver trilhas usadas pelas FATs, o stage2 pode ser carregado imediatamente após o diretório raiz
        inc ax            ; Se houver trilhas usadas pelas FATs, incrementamos o número de trilhas para garantir que o stage2 seja carregado após as FATs

        .root_dir_after:
            ; O número de setores usados pelo diretório raiz é o número de entradas no diretório raiz multiplicado pelo tamanho de cada entrada (32 bytes), dividido pelo tamanho do setor (512 bytes), que é o valor que calculamos em AX. Armazenamos esse valor em CL para uso posterior, como o número de setores a serem lidos para carregar o stage2.
            mov cl, al    ; O número de setores usados pelo diretório raiz é o número de entradas no diretório raiz multiplicado pelo tamanho de cada entrada (32 bytes), dividido pelo tamanho do setor (512 bytes), que é o valor que calculamos em AX. Armazenamos esse valor em CL para uso posterior, como o número de setores a serem lidos para carregar o stage2.
            pop ax        ; AX agora contém o número de setores usados pelo diretório raiz, que é o deslocamento do stage2 em setores
            mov dl, [ebr_drive_number] ; Carrega o número do drive para DL, que é necessário para a função de leitura do disco
            mov bx, buffer ; O buffer de destino para a leitura do stage2, que é o endereço onde o stage2 será carregado na memória
            call disk_read ; Chama a função de leitura do disco para ler o stage2 para a

            ; Após ler o stage2 para a memória, o bootloader deve saltar para o endereço onde o stage2 foi carregado para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
            xor bx, bx     ; Limpa BX para garantir que o endereço de destino seja 0, que é onde o stage2 será carregado na memória
            mov di, buffer ; O buffer de destino para a leitura do stage2, que é o endereço onde o stage2 será carregado na memória

            ; Agora que temos o stage2 carregado na memória, podemos saltar para ele para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
            .search_stage2:
                mov si, file_stage2_bin ; Carrega o endereço da string do nome do arquivo do stage2 em SI para comparar com as entradas do diretório raiz
                mov cx, 11 ; O nome do arquivo do stage2 tem 11 bytes, então definimos CX para 11 para comparar os 11 bytes do nome do arquivo com as entradas do diretório raiz
                push di    ; Salva DI para a comparação, pois a função de comparação irá modificar DI
                repe cmpsb ; Compara os 11 bytes do nome do arquivo do stage2 com a entrada atual do diretório raiz. Se os 11 bytes forem iguais, o zero flag será setado e a comparação terminará. Caso contrário, o zero flag será limpo e a comparação continuará até que CX seja 0 ou o zero flag seja setado.
                pop di     ; Restaura DI após a comparação
                je .found_stage2 ; Se os 11 bytes do nome do arquivo do stage2 forem iguais à entrada atual do diretório raiz, saltamos para a etiqueta .found_stage2 para carregar o stage2

                add di, 32
                inc bx
                cmp bx, [bdb_dir_entries_count] ; Compara o número de entradas do diretório raiz com o número de entradas que já verificamos (BX). Se BX for igual ao número de entradas do diretório raiz, significa que verificamos todas as entradas e não encontramos o stage2, então podemos mostrar uma mensagem de erro ou reiniciar o sistema.
                jl .search_stage2 ; Se ainda houver entradas do diretório raiz para verificar, continuamos a comparação

                ; stage2 não encontrado após verificar todas as entradas do diretório raiz, então mostramos uma mensagem de erro ou reiniciamos o sistema
                jmp stage2_not_found_error

                .found_stage2:
                    ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2
                    mov ax, [di + 26] ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2
                    mov [stage2_cluster], ax ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então armazenamos esse valor para uso posterior, como o número de setores a serem lidos para carregar o stage2

                    ; Calcula o deslocamento do stage2 em setores com base no número do cluster do stage2 e nas informações de geometria do disco. O deslocamento do stage2 em setores é calculado como:
                    mov ax, [bdb_reserved_sectors] ; O número de setores reservados é o número de setores antes do início das FATs, então começamos com esse valor
                    mov bx, buffer ; O buffer de destino para a leitura do stage2, que é o endereço onde o stage2 será carregado na memória
                    mov cl, [bdb_sectors_per_fat] ; O número de setores por FAT é o número de setores usados por cada FAT, então multiplicamos esse valor pelo número de FATs para obter o número total de setores usados pelas FATs
                    mov dl, [ebr_drive_number] ; Carrega o número do drive para DL, que é necessário para a função de leitura do disco
                    call disk_read ; Chama a função de leitura do disco para ler o stage2 para a

                    ; Após calcular o deslocamento do stage2 em setores, o bootloader deve saltar para o endereço onde o stage2 foi carregado para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
                    mov bx, STAGE2_LOAD_SEGMENT ; O segmento onde o stage2 será carregado, que é 0x2000 para que o stage2 seja carregado no endereço físico 0x7E00 (0x2000:0x7E00 = 0x7E00)
                    mov es, bx ; Define ES para o segmento onde o stage2 será carregado, que é 0x2000 para que o stage2 seja carregado no endereço físico 0x7E00 (0x2000:0x7E00 = 0x7E00)
                    mov bx, STAGE2_LOAD_OFFSET ;

                    .load_stage2_loop:
                        mov ax, [stage2_cluster] ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2

                        ; hardcoded: não está legal!
                        add ax, 31

                        mov cl, 1 ; O número de setores a serem lidos para carregar o stage2, que é 1 setor por cluster para FAT12, então definimos CL para 1
                        mov dl, [ebr_drive_number] ; Carrega o número do drive para DL, que é necessário para a função de leitura do disco
                        call disk_read ; Chama a função de leitura do disco para ler o stage2 para a memória

                        ; outro bug: irá causar overflow se o stage2 tiver mais de 64kb (128 setores), porque estamos usando um registrador de 16 bits para armazenar o deslocamento do stage2 em setores, então precisamos garantir que o stage2 seja pequeno o suficiente para caber em 64kb ou usar um registrador de 32 bits para armazenar o deslocamento do stage2 em setores
                        add bx, [bdb_bytes_per_sector] ; Incrementa o deslocamento do stage2 em bytes para a próxima leitura, que é o número de bytes por setor (512 bytes para FAT12)

                        ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2
                        mov ax, [stage2_cluster] ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2
                        mov cx, 3 ; O número de setores por cluster para FAT12 é 1, então multiplicamos o número do cluster do stage2 por 1 para obter o deslocamento do stage2 em setores. No entanto, como estamos usando um registrador de 16 bits para armazenar o deslocamento do stage2 em setores, precisamos garantir que o número do cluster do stage2 seja pequeno o suficiente para não causar overflow. Para isso, podemos multiplicar o número do cluster do stage2 por 3 (o número máximo de setores por cluster para FAT12) para garantir que o deslocamento do stage2 em setores seja pequeno o suficiente para caber em um registrador de 16 bits.
                        mul cx    ; O resultado da multiplicação está em AX, que é o deslocamento do stage2 em setores, então armazenamos esse valor para uso posterior, como o número de setores a serem lidos para carregar o stage2
                        mov cx, 2 ; O número de setores por trilha para FAT12 é 18, então dividimos o deslocamento do stage2 em setores pelo número de setores por trilha para obter o número de trilhas a serem puladas para carregar o stage2. No entanto, como estamos usando um registrador de 16 bits para armazenar o número de trilhas a serem puladas, precisamos garantir que o número de trilhas a serem puladas seja pequeno o suficiente para caber em um registrador de 16 bits. Para isso, podemos dividir o deslocamento do stage2 em setores por 2 (o número máximo de setores por trilha para FAT12) para garantir que o número de trilhas a serem puladas seja pequeno o suficiente para caber em um registrador de 16 bits.
                        div cx    ; O resultado da divisão está em AX, que é o número de trilhas a serem puladas para carregar o stage2, então armazenamos esse valor para uso posterior, como o número de trilhas a serem puladas para carregar o stage2

                        ; O número de trilhas a serem puladas para carregar o stage2 está em AX, então adicionamos esse valor ao endereço do buffer para obter o endereço do próximo setor a ser lido para carregar o stage2
                        mov si, buffer ; O buffer de destino para a leitura do stage2, que é o endereço onde o stage2 será carregado na memória, então carregamos esse valor em SI para uso posterior, como o endereço do próximo setor a ser lido para carregar o stage2
                        add si, ax ; O número de trilhas a serem puladas para carregar o stage2 está em AX, então adicionamos esse valor ao endereço do buffer para obter o endereço do próximo setor a ser lido para carregar o stage2
                        mov ax, [ds:si] ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2

                        ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2. Para FAT12, os números de cluster são armazenados em 12 bits, então precisamos verificar se o número do cluster do stage2 é par ou ímpar para determinar se o número do cluster do stage2 está armazenado em 16 bits (2 bytes) ou em 12 bits na entrada do diretório raiz. Se o número do cluster do stage2 for par, isso indica que o número do cluster do stage2 está armazenado em 16 bits (2 bytes) na entrada do diretório raiz, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2. Caso contrário, se o número do cluster do stage2 for ímpar, isso indica que o número do cluster do stage2 está armazenado em 12 bits na entrada do diretório raiz, então precisamos deslocar os bits de AX para a direita em 4 posições para obter os 12 bits mais significativos do número do cluster do stage2.
                        or dx, dx ; Verifica se o número do cluster do stage2 é par ou ímpar, pois isso indica se o número do cluster do stage2 está armazenado em 16 bits (2 bytes) ou em 12 bits na entrada do diretório raiz. Se o número do cluster do stage2 for par, isso indica que o número do cluster do stage2 está armazenado em 16 bits (2 bytes) na entrada do diretório raiz, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2. Caso contrário, se o número do cluster do stage2 for ímpar, isso indica que o número do cluster do stage2 está armazenado em 12 bits na entrada do diretório raiz, então precisamos deslocar os bits de AX para a direita em 4 posições para obter os 12 bits mais significativos do número do cluster do stage2.
                        jz .even ; Se o número do cluster do stage2 for par, isso indica que o número do cluster do stage2 está armazenado em 16 bits (2 bytes) na entrada do diretório raiz, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2. Caso contrário, se o número do cluster do stage2 for ímpar, isso indica que o número do cluster do stage2 está armazenado em 12 bits na entrada do diretório raiz, então precisamos deslocar os bits de AX para a direita em 4 posições para obter os 12 bits mais significativos do número do cluster do stage2.

                        .odd:
                            shr ax, 4 ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2. Para FAT12, os números de cluster são armazenados em 12 bits, então para obter o número do cluster do stage2, precisamos deslocar os bits de AX para a direita em 4 posições para obter os 12 bits mais significativos do número do cluster do stage2
                            jmp .next_cluster_after ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2. Para FAT12, os números de cluster são armazenados em 12 bits, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2                 

                        .even:
                            ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2. Para FAT12, os números de cluster são armazenados em 12 bits, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2
                            and ax, 0x0FFF ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2. Para FAT12, os números de cluster são armazenados em 12 bits, então aplicamos uma máscara para extrair apenas os 12 bits do número do cluster do stage2

                            .next_cluster_after:
                                ; Compara o número do cluster do stage2 com 0x0FF8, que é o valor de fim de arquivo para FAT12. Se o número do cluster do stage2 for maior ou igual a 0x0FF8, isso indica que chegamos ao final do arquivo do stage2, então terminamos de carregar o stage2 para a memória. Caso contrário, continuamos a carregar o stage2 para a memória, lendo o próximo setor do stage2 com base no número do cluster do stage2 e nas informações de geometria do disco.
                                cmp ax, 0x0FF8 ; Compara o número do cluster do stage2 com 0x0FF8, que é o valor de fim de arquivo para FAT12. Se o número do cluster do stage2 for maior ou igual a 0x0FF8, isso indica que chegamos ao final do arquivo do stage2, então terminamos de carregar o stage2 para a memória. Caso contrário, continuamos a carregar o stage2 para a memória, lendo o próximo setor do stage2 com base no número do cluster do stage2 e nas informações de geometria do disco.
                                jae .read_finish ; Se o número do cluster do stage2 for maior ou igual a 0x0FF8, que é o valor de fim de arquivo para FAT12, então terminamos de carregar o stage2 para a memória, pois isso indica que chegamos ao final do arquivo do stage2. Caso contrário, continuamos a carregar o stage2 para a memória, lendo o próximo setor do stage2 com base no número do cluster do stage2 e nas informações de geometria do disco.

                                ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então carregamos esse valor em AX para uso posterior, como o número de setores a serem lidos para carregar o stage2
                                mov [stage2_cluster], ax ; O número do cluster do stage2 está na entrada do diretório raiz, nos bytes 26-27, então armazenamos esse valor para uso posterior, como o número de setores a serem lidos para carregar o stage2
                                jmp .load_stage2_loop ; Continua a carregar o stage2 para a memória até que tenhamos carregado todos os setores do stage2, que é indicado por um número de cluster maior ou igual a 0x0FF8, que é o valor de fim de arquivo para FAT12

                                .read_finish:
                                    ; O stage2 foi carregado com sucesso, então agora podemos saltar para o endereço onde o stage2 foi carregado para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
                                    mov dl, [ebr_drive_number] ; Carrega o número do drive para DL, que é necessário para a função de leitura do disco

                                    ; Após carregar o stage2 para a memória, o bootloader deve saltar para o endereço onde o stage2 foi carregado para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
                                    mov ax, STAGE2_LOAD_SEGMENT ; O segmento onde o stage2 será carregado, que é 0x2000 para que o stage2 seja carregado no endereço físico 0x7E00 (0x2000:0x7E00 = 0x7E00)
                                    mov ds, ax ; O segmento onde o stage2 será carregado, que é 0x2000 para que o stage2 seja carregado no endereço físico 0x7E00 (0x2000:0x7E00 = 0x7E00)
                                    mov es, ax ; O segmento onde o stage2 será carregado, que é 0x2000 para que o stage2 seja carregado no endereço físico 0x7E00 (0x2000:0x7E00 = 0x7E00)

                                    ; Após carregar o stage2 para a memória, o bootloader deve saltar para o endereço onde o stage2 foi carregado para iniciar a execução do stage2. O endereço onde o stage2 foi carregado é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, podemos usar uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.
                                    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET ; Salta para o endereço onde o stage2 foi carregado para iniciar a execução do stage2, que é 0x0000:0x7E00, que é o endereço físico 0x7E00. Para saltar para esse endereço, usamos uma instrução de salto far (JMP FAR) que especifica o segmento e o deslocamento do endereço de destino.

                                    ; Se o stage2 for carregado com sucesso, mas o número do cluster do stage2 for maior ou igual a 0x0FF8, que é o valor de fim de arquivo para FAT12, então mostramos uma mensagem de erro ou reiniciamos o sistema
                                    jmp wait_key_and_reboot ; Se o stage2 for carregado com sucesso, mas o número do cluster do stage2 for maior ou igual a 0x0FF8, que é o valor de fim de arquivo para FAT12, então mostramos uma mensagem de erro ou reiniciamos o sistema

                                    ; Se o stage2 for carregado com sucesso, mas o número do cluster do stage2 for maior ou igual a 0x0FF8, que é o valor de fim de arquivo para FAT12, então mostramos uma mensagem de erro ou reiniciamos o sistema
                                    cli               ; Desabilita interrupções para evitar que o sistema seja interrompido
                                    hlt               ; Para a CPU por enquanto (o bootloader ainda não carrega o stage2)

;
; Erros de cabeçalho do disco ou falhas de leitura
; Parâmetros:
;   - SI: endereço da mensagem de erro a ser exibida
; A rotina irá imprimir a mensagem de erro e aguardar o usuário pressionar uma tecla antes
floppy_error:
    mov si, msg_read_failed   ; Carrega o endereço da mensagem de erro em SI 
    call puts                 ; Chama a função puts para imprimir a mensagem de erro
    jmp wait_key_and_reboot ; Aguarda o usuário pressionar uma tecla e reinicia o sistema

;
; Erro de stage2 não encontrado
; Parâmetros:
;   - SI: endereço da mensagem de erro a ser exibida
; A rotina irá imprimir a mensagem de erro e aguardar o usuário pressionar uma tecla antes
stage2_not_found_error:
    mov si, msg_stage2_not_found   ; Carrega o endereço da mensagem de erro em SI 
    call puts                 ; Chama a função puts para imprimir a mensagem de erro
    jmp wait_key_and_reboot ; Aguarda o usuário pressionar uma tecla e reinicia o sistema

;
; Rotina de impressão de string para o bootloader, que é chamada tanto para a mensagem de boas-vindas quanto para mensagens de erro
; Parâmetros:
;   - SI: endereço da string a ser impressa
; A rotina irá imprimir a string na tela usando a função de saída teletipo do BIOS
; Retorna:
;   - Retorna ao chamador após imprimir a string
wait_key_and_reboot:
    mov ah, 0                 ; Função de leitura de tecla do BIOS
    int 0x16                  ; Chama a interrupção do BIOS para ler uma tecla
    jmp 0FFFFh:0              ; Reinicia o sistema pulando para o endereço de reset
    hlt                       ; Para a CPU e espera por uma tecla

;
; Imprime uma string na tela usando interrupções do BIOS.
; A string deve ser terminada com nulo. (bootloader)
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

;
; Rotinas de disco
;

;
; Converte um endereço LBA (Logical Block Addressing) para CHS (Cylinder-Head-Sector).
; Entrada: 
;       EAX = LBA, EBX = setores por trilha, ECX = cabeças
; Saída: 
;       DX = cilindro, DH = cabeça, DL = setor
; Fórmula:
;       setor = (LBA % setores_por_trilha) + 1
;       cabeça = (LBA / setores_por_trilha) % cabeças
;       cilindro = LBA / (setores_por_trilha * cabeças)
lba_to_chs:
    ; Salva os registradores usados por esta rotina
    push ax         ; Salva AX (LBA originário)

    ; Calcula o setor (DL)
    xor dx, dx      ; Limpa DX para a divisão de 32 bits (AX:DX / divisor)
    div word [bdb_sectors_per_track]      ; Divide LBA por setores por trilha

    ; O resultado da divisão está em AX (LBA / SPT) e DX (LBA % SPT)
    inc dx          ; Setores começam em 1, então incrementamos o resto
    mov cx, dx      ; Armazena o setor em CX temporariamente

    ; Calcula a cabeça (DH) e cilindro (CH)
    xor dx, dx      ; Limpa DX para a próxima divisão
    div word [bdb_heads]      ; Divide (LBA / SPT) por cabeças

    ; Resultado em AX (cilindro) e DX (cabeça)
    mov dh, dl      ; Armazena a cabeça em DH
    mov ch, al      ; Armazena o cilindro (bits baixos) em CH
    shl ah, 6       ; Desloca bits altos do cilindro
    or cl, ah       ; Combina os bits altos do cilindro com o setor (bits baixos)

    ; Restaura os registradores e retorna
    pop ax          ; Restaura o AX original
    ret             ; Retorna ao chamador

;
; Lê setores do disco usando a interrupção 0x13 do BIOS.
; Parâmetros:
;   - ax: endereço LBA do setor a ser lido
;   - bx: deslocamento do buffer de destino
;   - cl: número de setores a ler
;   - dl: número do drive
; Retorna:
;   - CF setado em caso de erro, com o código de erro em AX
;   - CF limpo em caso de sucesso
disk_read:
    push ax        ; Salva AX (LBA)
    push bx        ; Salva BX (Buffer)
    push cx        ; Salva CX (Setores)
    push dx        ; Salva DX (Drive)
    push di        ; Salva DI

    push cx        ; Salva CX temporariamente
    call lba_to_chs  ; Converte LBA para CHS
    pop ax         ; AL = número de setores para a interrupção 0x13
    
    mov ah, 0x02   ; Função de leitura de setores do BIOS
    mov dl, [ebr_drive_number] ; Restaura o número do drive, pois lba_to_chs destrói DL
    mov di, 3      ; Contador de tentativas

    .retry:
        pusha          ; Salva todos os registradores para a tentativa
        int 0x13       ; Tenta ler do disco
        jnc .done      ; Se não houve erro, terminamos

        ; Se houve um erro, tentamos novamente
        popa           ; Restaura os registradores da tentativa
        call disk_reset  ; Tenta resetar o controlador de disco

        dec di         ; Decrementa o contador de tentativas
        test di, di    ; Verifica se ainda há tentativas
        jnz .retry     ; Tenta novamente se di > 0

    .fail:
        jmp floppy_error  ; Se falhou após 3 tentativas

    .done:
        popa           ; Restaura os registradores da última tentativa (sucesso)
        pop di         ; Restaura registradores na ordem inversa
        pop dx         ; Restaura DX
        pop cx         ; Restaura CX
        pop bx         ; Restaura BX
        pop ax         ; Restaura AX
        ret            ; Retorna ao chamador

;
; Reinicio de disco controlador usando a função de reset do BIOS.
; Parâmetros:
;   - dl: número do drive (0x00 para disquete, 0x80 para HD)
; Retorna:
;   - CF setado em caso de erro, com o código de erro em AX
;   - CF limpo em caso de sucesso
disk_reset:
    pusha           ; Salva todos os registradores
    mov ah, 0       ; Função de reset do BIOS para o controlador de disco
    stc             ; Seta o flag de carry para indicar que estamos tentando resetar
    int 0x13        ; Chama a interrupção do BIOS para resetar o controlador de disco
    jc floppy_error ; Se o reset falhar, mostramos uma mensagem de erro
    popa            ; Restaura os registradores
    ret             ; Retorna ao chamador

; A mensagem de boas-vindas do bootloader, seguida por um terminador nulo
msg_boot:           
    db 'AlmaOS Bootloader...', ENDL, 0
; A mensagem de erro caso a leitura do disco falhe, seguida por um terminador nulo
msg_read_failed:    
    db 'Error reading disk!', ENDL, 0
; A mensagem de erro caso o stage2 não seja encontrado, seguida por um terminador nulo
msg_stage2_not_found: 
    db 'STAGE2.BIN not found!', ENDL, 0
; O nome do arquivo do stage2 a ser carregado, que é "STAGE2.BIN" seguido por um terminador nulo. Este nome será usado para comparar com as entradas do diretório raiz para encontrar o número do cluster do stage2.
file_stage2_bin:    
    db 'STAGE2  BIN', 0
; Variável para armazenar o número do cluster do stage2, que será lido do diretório raiz e usado para calcular o deslocamento do stage2 em setores para leitura posterior
stage2_cluster:
    dw 0 ; Variável para armazenar o número do cluster do stage2, que será lido do diretório raiz e usado para calcular o deslocamento do stage2 em setores para leitura posterior

; O endereço onde o stage2 será carregado na memória, que é 0x0000:0x7E00 (endereço físico 0x7E00)
STAGE2_LOAD_SEGMENT equ 0x2000
; O deslocamento do stage2 em setores a partir do início do disco, que será calculado com base no número do cluster do stage2 e nas informações de geometria do disco
STAGE2_LOAD_OFFSET equ 0

times 510 - ($ - $$) db 0         ; Preenche o restante do bootloader com zeros até atingirmos 510 bytes   
dw 0xAA55         ; Assinatura de boot, deve ser 0xAA55 para o BIOS reconhecer isto como um disco inicializável

buffer:
