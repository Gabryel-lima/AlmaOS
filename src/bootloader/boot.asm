org 0x7C00           ; O BIOS carrega o bootloader no endereço 0x7C00, então definimos a origem para esse endereço
bits 16              ; Estamos escrevendo código de 16 bits para a arquitetura x86

%define ENDL 0x0D, 0x0A         ; Define uma constante para os caracteres de nova linha (retorno de carro e avanço de linha)

;
; FAT12 Header (BPB - BIOS Parameter Block)
; Necessário para que o mcopy e o sistema operacional reconheçam o disco
;
jmp short main       ; Pula para o início do código principal, ignorando o BPB
nop                  ; Instrução NOP para preencher o espaço entre o JMP e o início do BPB, garantindo que o BPB esteja no local correto

bdb_oem:                    db 'MSWIN4.1'           ; Nome do fabricante (8 bytes)
bdb_bytes_per_sector:       dw 512                  ; Bytes por setor
bdb_sectors_per_cluster:    db 1                    ; Setores por cluster
bdb_reserved_sectors:       dw 1                    ; Setores reservados (geralmente 1 para o bootloader)
bdb_fat_count:              db 2                    ; Quantidade de tabelas FAT
bdb_dir_entries_count:      dw 224                  ; Número de entradas no diretório raiz
bdb_total_sectors:          dw 2880                 ; Total de setores no disco (1.44MB = 2880 * 512 bytes)
bdb_media_descriptor_type:  db 0xF0                 ; Tipo de mídia (F0 = Disquete 3.5")
bdb_sectors_per_fat:        dw 9                    ; Setores por cada tabela FAT
bdb_sectors_per_track:      dw 18                   ; Setores por trilha
bdb_heads:                  dw 2                    ; Número de cabeças (lados do disquete)
bdb_hidden_sectors:         dd 0                    ; Setores ocultos
bdb_large_sector_count:     dd 0                    ; Contador de setores grandes (usado se total_sectors for 0)

; Registro de Boot Estendido (Extended Boot Record)
ebr_drive_number:           db 0                    ; Número do drive (0x00 para disquete, 0x80 para HD)
                            db 0                    ; Reservado (usado pelo Windows NT)
ebr_signature:              db 0x29                 ; Assinatura do EBR (0x28 ou 0x29)
ebr_volume_id:              db 0x12, 0x34, 0x56, 0x78   ; ID do volume (número de série aleatório)
ebr_volume_label:           db 'ALMA OS :) '        ; Rótulo do volume (Exatamente 11 bytes, completar com espaços)
ebr_system_id:              db 'FAT12   '           ; Tipo do sistema de arquivos (Exatamente 8 bytes)

main:
    ; Configura os registradores de segmento e o ponteiro de pilha.
    mov ax, 0         ; Define AX como 0, que será usado para os registradores de segmento
    mov ds, ax        ; Segmento de dados é definido para 0, pois usaremos endereçamento absoluto
    mov es, ax        ; Segmento extra não é usado, mas definimos como 0 por segurança

    ; Configura o segmento de pilha e o ponteiro de pilha.
    mov ss, ax        ; Segmento de pilha é definido como 0
    mov sp, 0x7C00    ; Pilha cresce para baixo a partir do final do bootloader

    mov si, msg_boot  ; Carrega o endereço da string em SI
    call puts         ; Chama a função puts para imprimir a string

    hlt               ; Para a CPU por enquanto (o bootloader ainda não carrega o kernel)

;
; Erros de cabeçalho do disco ou falhas de leitura
;

floppy_error:
    mov si, msg_read_failed   ; Carrega o endereço da mensagem de erro em SI 
    call puts                 ; Chama a função puts para imprimir a mensagem de erro
    jmp_wait_key_and_reboot   ; Loop para esperar por uma tecla e reiniciar o sistema

jmp_wait_key_and_reboot:
    mov ah, 0                 ; Função de leitura de tecla do BIOS
    int 0x16                  ; Chama a interrupção do BIOS para ler uma tecla
    jmp 0FFFFh:0              ; Reinicia o sistema pulando para o endereço de reset
    hlt                       ; Para a CPU e espera por uma tecla

.halt:
    cli         ; Desabilita interrupções para evitar que o sistema seja interrompido
    hlt         ; Para a CPU e espera por uma interrupção (que nunca virá, pois as interrupções estão desabilitadas)

;
; Imprime uma string na tela usando interrupções do BIOS.
; A string deve ser terminada com nulo. (boot)
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
    push ax         ; Salva EAX, EBX, ECX
    push dx         ; Salva DX

    ; Calcula o setor (DL)
    xor dx, dx      ; Limpa DX para armazenar o resultado
    div word [bdb_sectors_per_track]      ; Divide LBA por setores por trilha

    ; O resultado da divisão está em AX (cylinders) e DX (remainder)
    inc dx          ; Setores começam em 1, então incrementamos o resultado
    mov cx, dx      ; Armazena o setor em CX para uso posterior

    ; Calcula a cabeça (DH)
    xor dx, dx      ; Limpa DX para armazenar o resultado da próxima divisão
    div word [bdb_heads]      ; Divide o resultado anterior por cabeças

    ; O resultado da divisão está em AX (cylinders) e DX (remainder)
    mov dh, dl      ; Armazena a cabeça em DH
    mov ch, al      ; Armazena o cilindro em CH
    shl ah, 6       ; O cilindro é armazenado em CH, mas os bits mais altos vão para AH
    or cl, ah       ; Combina os bits do cilindro em CL

    ; Restaura os registradores e retorna
    pop ax          ; Restaura EAX, EBX, ECX
    mov dl, al      ; O setor está em AL, então movemos para DL
    pop ax          ; Restaura EAX, EBX, ECX 
    ret             ; Retorna ao chamador

;
; Lê setores do disco usando a interrupção 0x13 do BIOS.
; Parâmetros:
;   - ax: endereço LBA do setor a ser lido
;   - cx: número de setores a ler (até 128)
;   - es: segmento do buffer de destino
;   - bx: deslocamento do buffer de destino
; Retorna:
;   - CF setado em caso de erro, com o código de erro em AX
;   - CF limpo em caso de sucesso
disk_read:
    push ax        ; Salva AX, que contém o endereço LBA
    push bx        ; Salva BX, que contém o deslocamento do buffer
    push cx        ; Salva CX, que contém o número de setores a ler
    push dx        ; Salva DX, que será usado para armazenar os valores de cilindro, cabeça e setor

    push cx        ; Salva CX, que contém o número de setores a ler
    call lba_to_chs  ; <-- Descrição na função
    pop ax         ; Restaura CX, que contém o número de setores a ler

    mov ah, 0x02   ; Função de leitura de setores do BIOS
    int 0x13       ; Chama a interrupção do BIOS para ler do disco
    mov di, 3      ; O setor 0 é o bootloader, então começamos a ler a partir do setor 1

.retry:
    pusha          ; Salva todos os registradores
    stc            ; Seta o flag de carry para indicar que estamos tentando ler
    int 0x13       ; Tenta ler do disco
    jnc .done      ; Se não houve erro, terminamos

    ; Se houve um erro, tentamos novamente
    popa           ; Restaura os registradores
    call disk_reset  ; Tenta resetar o controlador de disco

    dec di         ; Decrementa o setor para tentar ler novamente
    test di, di    ; Verifica se já tentamos ler o setor 0
    jnz .retry     ; Se ainda não tentamos o setor 0, tenta novamente

.fail:
    jmp floppy_error  ; Se falhamos ao ler o setor 0, mostramos uma mensagem de erro

.done:
    popa           ; Restaura os registradores

    push ax        ; Salva AX, que contém o endereço LBA
    push bx        ; Salva BX, que contém o deslocamento do buffer
    push cx        ; Salva CX, que contém o número de setores a ler
    push dx        ; Salva DX, que será usado para armazenar os valores de cilindro, cabeça e setor
    ret            ; Retorna ao chamador

;
; Reinicio de disco
;
disk_reset:
    

msg_boot:            ; A mensagem de boas-vindas do bootloader, seguida por um terminador nulo
    db 'AlmaOS Bootloader...', ENDL, 0         ; A string seguida por um terminador nulo

msg_read_failed:     ; A mensagem de erro caso a leitura do disco falhe, seguida por um terminador nulo
    db 'Error reading disk!', ENDL, 0          ; A string seguida por um terminador nulo

times 510 - ($ - $$) db 0         ; Preenche o restante do bootloader com zeros até atingirmos 510 bytes   
dw 0xAA55         ; Assinatura de boot, deve ser 0xAA55 para o BIOS reconhecer isto como um disco inicializável
