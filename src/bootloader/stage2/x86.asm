bits 16         ; Continuamos em modo real de 16 bits

section _TEXT class=CODE ; Seção de código para o bootloader, onde o código executável será colocado

global _x86_Video_WriteCharTeletype ; Tornamos a função de escrita de caractere teletipo global para que possa ser chamada de C (x86)
global _x86_div64_32 ; Tornamos a função de divisão global para que possa ser chamada de C (x86)

;
; Função para dividir um número de 64 bits por um número de 32 bits, retornando o quociente e o resto (x86)
; Parâmetros:
;   - dividend: O número de 64 bits a ser dividido (passado em EDX:EAX, onde EDX é a parte alta e EAX é a parte baixa)
;   - divisor: O número de 32 bits pelo qual o dividend será dividido (passado em ECX)
;   - quotientOut: Ponteiro para onde o quociente de 64 bits será armazenado (passado em EBX, onde a parte alta do quociente será armazenada em [EBX + 4] e a parte baixa em [EBX])
;   - remainderOut: Ponteiro para onde o resto de 32 bits será armazenado (passado em EBX, onde o resto será armazenado em [EBX])
_x86_div64_32:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    push bx

    ; divide upper 32 bits
    mov eax, [bp + 8]   ; eax <- upper 32 bits of dividend
    mov ecx, [bp + 12]  ; ecx <- divisor
    xor edx, edx
    div ecx             ; eax - quot, edx - remainder

    ; store upper 32 bits of quotient
    mov bx, [bp + 16]
    mov [bx + 4], eax

    ; divide lower 32 bits
    mov eax, [bp + 4]   ; eax <- lower 32 bits of dividend
                        ; edx <- old remainder
    div ecx

    ; store results
    mov [bx], eax
    mov bx, [bp + 18]
    mov [bx], edx

    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret

;
; Função para escrever um caractere usando o modo teletipo do BIOS (x86)
; Parâmetros:
;   - AL: O caractere a ser impresso
;   - BH: A página de vídeo (normalmente 0)
_x86_Video_WriteCharTeletype:
    ; make new call frame
    push bp             ; save old call frame
    mov bp, sp          ; initialize new call frame

    ; save bx
    push bx

    ; [bp + 0] - old call frame
    ; [bp + 2] - return address (small memory model => 2 bytes)
    ; [bp + 4] - first argument (character)
    ; [bp + 6] - second argument (page)
    ; note: bytes are converted to words (you can't push a single byte on the stack)
    mov ah, 0Eh
    mov al, [bp + 4]
    mov bh, [bp + 6]

    int 10h

    ; restore bx
    pop bx

    ; restore old call frame
    mov sp, bp
    pop bp
    ret
