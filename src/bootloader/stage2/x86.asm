bits 16         ; Continuamos em modo real de 16 bits

section _TEXT class=CODE ; Seção de código para o bootloader, onde o código executável será colocado

global __U4M
global _x86_Video_WriteCharTeletype
global _x86_div64_32
global _x86_Disk_Reset
global _x86_Disk_Read
global _x86_Disk_GetDriveParams

;
; U4M
; Operação:       multiplicação inteira de quatro bytes
; Entradas:       DX;AX   inteiro M1
;                 CX;BX   inteiro M2
; Saídas:         DX;AX   produto
; Volátil:        CX, BX destruídos
;
__U4M:
    shl edx, 16         ; dx para a metade superior de edx
    mov dx, ax          ; m1 em edx
    mov eax, edx        ; m1 em eax

    shl ecx, 16         ; cx para a metade superior de ecx
    mov cx, bx          ; m2 em ecx

    mul ecx             ; resultado em edx:eax (precisamos apenas de eax)
    mov edx, eax        ; move a metade superior para dx
    shr edx, 16

    ret

;
; U4D
; Operação:       divisão inteira de quatro bytes sem sinal
; Entradas:       DX;AX   dividendo
;                 CX;BX   divisor
; Saídas:         DX;AX   quociente
;                 CX;BX   resto
; Volátil:        nenhum
;
__U4D:
    shl edx, 16         ; dx para a metade superior de edx
    mov dx, ax          ; dividendo em edx
    mov eax, edx        ; dividendo em eax

    shl ecx, 16         ; cx para a metade superior de ecx
    mov cx, bx          ; divisor em ecx

    xor edx, edx        ; limpa edx para a divisão
    div ecx             ; eax / ecx -> quociente em eax, resto em edx

    mov ecx, edx        ; move o resto para ecx:ebx
    mov ebx, edx
    shr ecx, 16         ; resto (alto) em ecx, (baixo) em bx

    mov edx, eax        ; move o quociente para edx:eax
    shr edx, 16         ; quociente (alto) em edx, (baixo) em ax

    ret

;
; Função para dividir um número de 64 bits por um número de 32 bits, retornando o quociente e o resto (x86)
; Parâmetros:
;   - dividend: O número de 64 bits a ser dividido (passado em EDX:EAX, onde EDX é a parte alta e EAX é a parte baixa)
;   - divisor: O número de 32 bits pelo qual o dividend será dividido (passado em ECX)
;   - quotientOut: Ponteiro para onde o quociente de 64 bits será armazenado (passado em EBX, onde a parte alta do quociente será armazenada em [EBX + 4] e a parte baixa em [EBX])
;   - remainderOut: Ponteiro para onde o resto de 32 bits será armazenado (passado em EBX, onde o resto será armazenado em [EBX])
_x86_div64_32:
    ; criar novo frame de chamada
    push bp             ; salvar frame antigo
    mov bp, sp          ; inicializar novo frame

    push bx

    ; dividir os 32 bits superiores
    mov eax, [bp + 8]   ; eax <- 32 bits superiores do dividendo
    mov ecx, [bp + 12]  ; ecx <- divisor
    xor edx, edx
    div ecx             ; eax - quociente, edx - resto

    ; armazenar os 32 bits superiores do quociente
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; dividir os 32 bits inferiores
    mov eax, [bp + 4]   ; eax <- 32 bits inferiores do dividendo
                        ; edx <- resto anterior
    div ecx

    ; armazenar resultados
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restaurar frame antigo
    mov sp, bp
    pop bp
    ret

;
; Função para escrever um caractere usando o modo teletipo do BIOS (x86)
; Parâmetros:
;   - AL: O caractere a ser impresso
;   - BH: A página de vídeo (normalmente 0)
_x86_Video_WriteCharTeletype:
    ; criar novo frame de chamada
    push bp             ; salvar frame antigo
    mov bp, sp          ; inicializar novo frame

    ; salvar bx
    push bx

    ; [bp + 0] - frame antigo
    ; [bp + 2] - endereço de retorno (modelo de memória small => 2 bytes)
    ; [bp + 4] - primeiro argumento (caractere)
    ; [bp + 6] - segundo argumento (página)
    ; nota: bytes são convertidos para words (não é possível fazer push de um único byte na pilha)
    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    ; restaurar bx
    pop bx

    ; restaurar frame antigo
    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_Reset(uint8_t drive);
; Reseta o controlador de disco.
; Parâmetros:
;   - drive: Número da unidade de disco (ex: 0 para floppy, 80h para HD).
; Retorno:
;   - AX: 1 em caso de sucesso, 0 em caso de erro.
;
_x86_Disk_Reset:
    ; criar novo frame de chamada
    push bp             ; salvar frame antigo
    mov bp, sp          ; inicializar novo frame

    mov ah, 0
    mov dl, [bp + 4]    ; dl - unidade (drive)
    stc
    int 0x13

    mov ax, 1
    sbb ax, 0           ; 1 em caso de sucesso, 0 em caso de falha

    ; restaurar frame antigo
    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_Read(uint8_t drive,
;                           uint16_t cylinder,
;                           uint16_t sector,
;                           uint16_t head,
;                           uint8_t count,
;                           void far * dataOut);
; Lê setores de um disco para a memória usando CHS (Cylinder, Head, Sector).
; Parâmetros:
;   - drive: Número da unidade de disco.
;   - cylinder: Índice do cilindro (0 a 1023).
;   - sector: Índice do setor (1 a 63).
;   - head: Índice da cabeça (0 a 15).
;   - count: Número de setores a serem lidos.
;   - dataOut: Ponteiro long (segmento:offset) para o buffer de destino.
; Retorno:
;   - AX: 1 em caso de sucesso, 0 em caso de erro.
;
_x86_Disk_Read:
    ; criar novo frame de chamada
    push bp             ; salvar frame antigo
    mov bp, sp          ; inicializar novo frame

    ; salvar registradores modificados
    push bx
    push es

    ; configurar argumentos
    mov dl, [bp + 4]    ; dl - unidade (drive)

    mov ch, [bp + 6]    ; ch - cilindro (8 bits inferiores)
    mov cl, [bp + 7]    ; cl - cilindro para bits 6-7
    shl cl, 6
    
    mov al, [bp + 8]    ; cl - setor para bits 0-5
    and al, 0x3F
    or cl, al

    mov dh, [bp + 10]   ; dh - cabeça

    mov al, [bp + 12]   ; al - quantidade (count)

    mov bx, [bp + 16]   ; es:bx - ponteiro far para saída de dados
    mov es, bx
    mov bx, [bp + 14]

    ; chamar int0x13
    mov ah, 0x02
    stc
    int 0x13

    ; definir valor de retorno
    mov ax, 1
    sbb ax, 0           ; 1 em caso de sucesso, 0 em caso de falha

    ; restaurar registradores
    pop es
    pop bx

    ; restaurar frame antigo
    mov sp, bp
    pop bp
    ret

;
; bool _cdecl x86_Disk_GetDriveParams(uint8_t drive,
;                                     uint8_t* driveTypeOut,
;                                     uint16_t* cylindersOut,
;                                     uint16_t* sectorsOut,
;                                     uint16_t* headsOut);
; Obtém os parâmetros e geometria do drive de disco usando a INT 13h, AH=08h.
; Parâmetros:
;   - drive: Número da unidade de disco de entrada.
;   - driveTypeOut: Ponteiro para retorno do tipo de unidade (BL).
;   - cylindersOut: Ponteiro para retorno do número total de cilindros.
;   - sectorsOut: Ponteiro para retorno de setores por trilha.
;   - headsOut: Ponteiro para retorno de número total de cabeças.
; Retorno:
;   - AX: 1 em caso de sucesso, 0 em caso de erro.
;
_x86_Disk_GetDriveParams:
    ; criar novo frame de chamada
    push bp             ; salvar frame antigo
    mov bp, sp          ; inicializar novo frame

    ; salvar registradores
    push es
    push bx
    push si
    push di

    ; chamar int0x13
    mov dl, [bp + 4]    ; dl - unidade de disco
    mov ah, 0x08
    mov di, 0           ; es:di - 0000:0000
    mov es, di
    stc
    int 0x13

    ; retorno
    mov ax, 1
    sbb ax, 0

    ; parâmetros de saída
    mov si, [bp + 6]    ; tipo da unidade a partir de bl
    mov [si], bl

    mov bl, ch          ; cilindros - bits inferiores em ch
    mov bh, cl          ; cilindros - bits superiores em cl (6-7)
    shr bh, 6
    mov si, [bp + 8]
    mov [si], bx

    xor ch, ch          ; setores - 5 bits inferiores em cl
    and cl, 0x3F
    mov si, [bp + 10]
    mov [si], cx

    mov cl, dh          ; cabeças - dh
    mov si, [bp + 12]
    mov [si], cx

    ; restaurar registradores
    pop di
    pop si
    pop bx
    pop es

    ; restaurar frame antigo
    mov sp, bp
    pop bp
    ret
