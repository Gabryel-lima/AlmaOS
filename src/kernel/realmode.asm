; realmode.asm - Trampolim modo protegido 32-bit -> modo real 16-bit -> volta.
;
; O kernel entra em modo protegido logo no boot (boot.asm) e, a partir dai,
; nao pode mais chamar a BIOS: as rotinas da BIOS sao codigo de 16 bits que
; espera modo real, com segmentacao real e a IVT em 0x0000. Este arquivo faz a
; ponte, para que o kernel possa usar `int 10h` (VGA/VBE) sem abrir mao do
; modo protegido no resto do tempo.
;
; Sequencia:
;   1. modo protegido 32-bit  -> far jmp para um segmento de codigo de 16 bits;
;   2. modo protegido 16-bit  -> limpa PE em CR0, far jmp para modo real;
;   3. modo real              -> carrega a IVT, restaura os registradores do
;                                chamador e faz a chamada far para o handler da
;                                BIOS (equivalente a `int N`, com o vetor lido
;                                da IVT antes da troca de modo);
;   4. modo real              -> seta PE, far jmp de volta para 32 bits;
;   5. modo protegido 32-bit  -> restaura GDT/IDT/stack e retorna ao chamador.
;
; O passo 1 nao e opcional: enquanto CS/SS carregarem descritores de 32 bits,
; o cache oculto do descritor mantem limite de 4 GB e operandos de 32 bits
; mesmo depois de PE=0, e o modo real passa a se comportar de forma
; imprevisivel. Por isso boot.asm ganhou descritores 16-bit (0x18/0x20).
;
; O corpo do trampolim e copiado para REALMODE_STUB_ADDR porque o modo real
; enderecha por segment:offset: com o codigo numa base alinhada e conhecida,
; todo offset interno vira uma constante de montagem (label - stub_start) e
; CS pode ser usado como base de dados. O kernel esta linkado em 0x12200, que
; nao tem base alinhada util para isso.
;
; Interrupcoes ficam desligadas o tempo todo, e realmode.c mascara o PIC
; antes de chamar: a BIOS pode dar STI por conta propria, e uma IRQ nesse
; ponto saltaria por uma IVT que aponta para handlers da BIOS enquanto o PIC
; ja esta remapeado para 0x20-0x2F pelo kernel.

bits 32

section .note.GNU-stack noalloc noexec nowrite progbits

; ---- Layout da regiao do trampolim (espelhado em include/realmode.h) ----
REALMODE_STUB_ADDR   equ 0x00030000     ; codigo copiado para ca (segmento 0x3000)
REALMODE_STUB_SEG    equ 0x3000
REALMODE_DATA_ADDR   equ 0x00030400     ; bloco de estado compartilhado com o C
REALMODE_DATA_OFF    equ 0x0400         ; mesmo bloco, relativo a CS em modo real
REALMODE_STACK_TOP   equ 0x2000         ; SS:SP = 0x3000:0x2000 -> 0x32000

; ---- Offsets dentro de realmode_state_t (include/realmode.h) ----
RM_SAVED_ESP    equ 0x00
RM_SAVED_GDTR   equ 0x04
RM_SAVED_IDTR   equ 0x0C
RM_IVT_IDTR     equ 0x14
RM_BIOS_HANDLER equ 0x1C
RM_EAX          equ 0x20
RM_EBX          equ 0x24
RM_ECX          equ 0x28
RM_EDX          equ 0x2C
RM_ESI          equ 0x30
RM_EDI          equ 0x34
RM_EBP          equ 0x38
RM_DS           equ 0x3C
RM_ES           equ 0x3E
RM_EFLAGS       equ 0x40

; ---- Seletores da GDT instalada por boot.asm ----
GDT_CODE32      equ 0x08
GDT_DATA32      equ 0x10
GDT_CODE16      equ 0x18
GDT_DATA16      equ 0x20

; Endereco absoluto (linear) de um rotulo do stub depois de copiado. Vale para
; os seletores de 32 bits, cuja base e 0.
%define STUB_ABS(label) (REALMODE_STUB_ADDR + ((label) - realmode_stub_start))
; Offset do mesmo rotulo relativo a base do stub. Vale para os seletores de 16
; bits (base ajustada por realmode_init) e para REALMODE_STUB_SEG em modo real.
%define STUB_OFF(label) ((label) - realmode_stub_start)

section .text

global realmode_stub_start
global realmode_stub_end

; ---------------------------------------------------------------------------
; Corpo do trampolim. Tudo entre realmode_stub_start e realmode_stub_end e
; copiado para REALMODE_STUB_ADDR por realmode_init() e chamado ali — nunca
; execute isto no lugar em que foi linkado.
;
; Chamado como uma funcao C sem argumentos: `((void (*)(void))0x30000)()`.
; Os argumentos e os resultados trafegam pelo bloco em REALMODE_DATA_ADDR.
; ---------------------------------------------------------------------------
align 16
realmode_stub_start:
    pushad
    pushfd
    cli

    ; Guarda o estado de 32 bits que precisa sobreviver a ida ao modo real.
    mov [REALMODE_DATA_ADDR + RM_SAVED_ESP], esp
    sgdt [REALMODE_DATA_ADDR + RM_SAVED_GDTR]
    sidt [REALMODE_DATA_ADDR + RM_SAVED_IDTR]

    ; Passo 1: entra em modo protegido de 16 bits (recarrega CS com um
    ; descritor de 16 bits). Far jump codificado a mao porque o destino e uma
    ; expressao calculada em tempo de montagem. O offset e relativo a base do
    ; descritor 0x18, que realmode_init() ajusta para REALMODE_STUB_ADDR — com
    ; base 0 o limite de 64 KB do descritor nem alcancaria o stub.
    db 0xEA
    dd STUB_OFF(stub_pm16)
    dw GDT_CODE16

bits 16
stub_pm16:
    ; Recarrega os demais seletores com o descritor de dados de 16 bits, para
    ; que os caches ocultos tambem passem a valer limite de 64 KB.
    mov ax, GDT_DATA16
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Passo 2: desliga o modo protegido.
    ; Nada de push/pop daqui ate SS:SP ser refeito: ESP ainda aponta para a
    ; stack do kernel (~0x78000), acima do limite de 64 KB deste SS.
    mov eax, cr0
    and eax, 0xFFFFFFFE
    mov cr0, eax

    ; Far jump para modo real usando a base do proprio stub como CS.
    db 0xEA
    dw STUB_OFF(stub_realmode)
    dw REALMODE_STUB_SEG

stub_realmode:
    ; Passo 3: modo real. CS = REALMODE_STUB_SEG, entao `cs:` alcanca tanto o
    ; codigo quanto o bloco de estado sem depender de DS.
    mov ax, REALMODE_STUB_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov sp, REALMODE_STACK_TOP

    ; IVT da BIOS: base 0, limite 0x3FF.
    o32 lidt [cs:REALMODE_DATA_OFF + RM_IVT_IDTR]

    ; Restaura os registradores pedidos pelo chamador. DS e ES ficam por
    ; ultimo justamente porque sao os que deixariam de apontar para ca.
    mov eax, [cs:REALMODE_DATA_OFF + RM_EAX]
    mov ebx, [cs:REALMODE_DATA_OFF + RM_EBX]
    mov ecx, [cs:REALMODE_DATA_OFF + RM_ECX]
    mov edx, [cs:REALMODE_DATA_OFF + RM_EDX]
    mov esi, [cs:REALMODE_DATA_OFF + RM_ESI]
    mov edi, [cs:REALMODE_DATA_OFF + RM_EDI]
    mov ebp, [cs:REALMODE_DATA_OFF + RM_EBP]
    mov es,  [cs:REALMODE_DATA_OFF + RM_ES]
    mov ds,  [cs:REALMODE_DATA_OFF + RM_DS]

    ; Equivalente a `int N`: `int` empilha FLAGS e faz um far call; o handler
    ; termina em `iret`, que desempilha IP, CS e FLAGS. Fazendo o pushf + far
    ; call na mao, o vetor vira um dado (lido da IVT pelo C antes da troca de
    ; modo) em vez de exigir codigo automodificavel.
    pushf
    call far [cs:REALMODE_DATA_OFF + RM_BIOS_HANDLER]

    ; Devolve o resultado. EAX sai primeiro porque AX e necessario logo abaixo
    ; para refazer SS:SP — e `mov` nao altera EFLAGS, entao as flags de retorno
    ; da BIOS sobrevivem ate o pushf.
    mov [cs:REALMODE_DATA_OFF + RM_EAX], eax

    ; A BIOS pode voltar com SS:SP em outro lugar; refaz o nosso antes de
    ; qualquer push.
    mov ax, REALMODE_STUB_SEG
    mov ss, ax
    mov sp, REALMODE_STACK_TOP

    ; pushfd/pop eax: os 32 bits inteiros de EFLAGS. Com `pushf`/`pop ax` so
    ; a metade baixa seria valida e a alta ficaria com lixo do EAX da BIOS.
    pushfd
    pop eax
    mov [cs:REALMODE_DATA_OFF + RM_EFLAGS], eax

    mov [cs:REALMODE_DATA_OFF + RM_EBX], ebx
    mov [cs:REALMODE_DATA_OFF + RM_ECX], ecx
    mov [cs:REALMODE_DATA_OFF + RM_EDX], edx
    mov [cs:REALMODE_DATA_OFF + RM_ESI], esi
    mov [cs:REALMODE_DATA_OFF + RM_EDI], edi
    mov [cs:REALMODE_DATA_OFF + RM_EBP], ebp
    mov ax, es
    mov [cs:REALMODE_DATA_OFF + RM_ES], ax
    mov ax, ds
    mov [cs:REALMODE_DATA_OFF + RM_DS], ax

    ; Passo 4: volta para o modo protegido.
    cli
    o32 lgdt [cs:REALMODE_DATA_OFF + RM_SAVED_GDTR]
    mov eax, cr0
    or eax, 1
    mov cr0, eax

    db 0x66, 0xEA                       ; far jmp com operando de 32 bits
    dd STUB_ABS(stub_pm32)
    dw GDT_CODE32

bits 32
stub_pm32:
    ; Passo 5: de volta ao ambiente do kernel.
    mov ax, GDT_DATA32
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, [REALMODE_DATA_ADDR + RM_SAVED_ESP]
    lidt [REALMODE_DATA_ADDR + RM_SAVED_IDTR]

    popfd
    popad
    ret

align 16
realmode_stub_end:
