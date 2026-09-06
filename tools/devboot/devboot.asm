; devboot.asm - Carregador de desenvolvimento (somente NASM) para o kernel do AlmaOS.
;
; Motivacao: o stage2 real e escrito em C de 16 bits e depende do OpenWatcom
; (wcc/wlink). Em ambientes que nao tem esse toolchain — CI, contêineres, uma
; maquina nova — nao da para gerar a imagem de boot e portanto nao da para
; testar o kernel. Este carregador cobre exatamente o contrato que o kernel
; espera do stage2, e nada mais:
;
;   1. carrega kernel.bin do disco para 0x12000 (0x1200:0000);
;   2. coleta o mapa E820 em 0x61000;
;   3. escreve o bloco boot_info em 0x60000 no mesmo layout binario de
;      memory_boot_info_t (src/bootloader/stage2/memory.h);
;   4. faz far jump para 0x1200:0000.
;
; NAO substitui o stage2: nao tem FAT12, nao tem driver de disco com retry,
; nao le arquivos por nome. A imagem e crua (setor 0 = este carregador,
; setor 1 em diante = kernel.bin). Serve para rodar e depurar o kernel.
;
; Monte com: nasm -f bin -DKERNEL_SECTORS=<n> devboot.asm -o devboot.bin

bits 16
org 0x7C00

%ifndef KERNEL_SECTORS
%define KERNEL_SECTORS 64                 ; fallback: 32 KiB de kernel
%endif

KERNEL_SEGMENT      equ 0x1200            ; 0x1200:0000 = 0x00012000
BOOT_INFO_SEGMENT   equ 0x6000            ; 0x6000:0000 = 0x00060000
E820_SEGMENT        equ 0x6100            ; 0x6100:0000 = 0x00061000
E820_CAPACITY       equ 32                ; entradas reservadas em 0x61000
HEAP_BASE           equ 0x00078000
HEAP_CAPACITY       equ 0x00020000
IO_BUFFER_FAR       equ 0x62000000        ; far pointer 0x6200:0000
IO_BUFFER_SIZE      equ 0x00002000
TEXT_FB_FAR         equ 0xB8000000        ; far pointer 0xB800:0000

; memory_boot_info_flags (src/bootloader/stage2/memory.h)
BI_HAS_MEMORY_MAP   equ 0x01
BI_HAS_IO_BUFFER    equ 0x02
BI_HAS_FRAMEBUFFER  equ 0x04

; Geometria de disquete 1.44 MiB, usada na conversao LBA -> CHS.
SECTORS_PER_TRACK   equ 18
HEADS               equ 2

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00                        ; stack logo abaixo do carregador
    sti

    mov [boot_drive], dl                  ; a BIOS entrega o drive de boot em DL

    mov si, msg_loading
    call print

    call load_kernel
    call collect_e820
    call fill_boot_info

    cli
    jmp KERNEL_SEGMENT:0x0000

; ---------------------------------------------------------------------------
; load_kernel - le KERNEL_SECTORS setores a partir do LBA 1 para 0x1200:0000.
; Le um setor por vez: com 512 bytes por setor, avancar ES em 0x20 por setor
; mantem o offset sempre em zero e evita qualquer aritmetica de 64 KiB.
; ---------------------------------------------------------------------------
load_kernel:
    mov word [lba], 1
    mov ax, KERNEL_SEGMENT
    mov [dest_segment], ax
    mov cx, KERNEL_SECTORS

.next_sector:
    push cx
    call read_sector
    pop cx

    add word [lba], 1
    add word [dest_segment], 0x20         ; 512 bytes = 0x20 paragrafos
    loop .next_sector
    ret

; ---------------------------------------------------------------------------
; read_sector - le o setor [lba] para [dest_segment]:0000, com ate 4 tentativas.
; Converte LBA em CHS com a geometria fixa de disquete declarada acima:
;   cilindro = lba / (SECTORS_PER_TRACK * HEADS)
;   cabeca   = (lba / SECTORS_PER_TRACK) % HEADS
;   setor    = (lba % SECTORS_PER_TRACK) + 1
; ---------------------------------------------------------------------------
read_sector:
    mov byte [retries], 4

.attempt:
    mov ax, [lba]
    xor dx, dx
    mov bx, SECTORS_PER_TRACK
    div bx                                ; AX = lba / SPT, DX = lba % SPT
    inc dx
    mov cl, dl                            ; CL = setor (base 1)

    xor dx, dx
    mov bx, HEADS
    div bx                                ; AX = cilindro, DX = cabeca
    mov ch, al                            ; CH = cilindro (bits 0-7)
    mov dh, dl                            ; DH = cabeca

    mov dl, [boot_drive]
    mov ax, [dest_segment]
    mov es, ax
    xor bx, bx                            ; ES:BX = destino

    mov ax, 0x0201                        ; AH=02 (ler), AL=01 (um setor)
    int 0x13
    jnc .done

    ; Falhou: reseta o controlador e tenta de novo.
    xor ah, ah
    mov dl, [boot_drive]
    int 0x13

    dec byte [retries]
    jnz .attempt

    mov si, msg_disk_error
    call print
    jmp halt_forever

.done:
    xor ax, ax
    mov es, ax
    ret

; ---------------------------------------------------------------------------
; collect_e820 - preenche 0x6100:0000 com ate E820_CAPACITY entradas de 24
; bytes (int 15h AX=E820h). Grava a contagem em [e820_count].
; ---------------------------------------------------------------------------
collect_e820:
    mov ax, E820_SEGMENT
    mov es, ax
    xor di, di
    xor ebx, ebx
    xor bp, bp                            ; BP = numero de entradas aceitas

.next_entry:
    mov eax, 0xE820
    mov edx, 0x534D4150                   ; 'SMAP'
    mov ecx, 24
    mov dword [es:di + 20], 1             ; ACPI 3.0: assume entrada valida
    int 0x15
    jc .done                              ; CF na primeira chamada = sem suporte
    cmp eax, 0x534D4150
    jne .done

    jcxz .skip                            ; entrada de tamanho zero
    cmp cl, 20
    jbe .accept
    test byte [es:di + 20], 1             ; bit 0 limpo = ignorar entrada
    jz .skip

.accept:
    mov eax, [es:di + 8]                  ; length low
    or eax, [es:di + 12]                  ; length high
    jz .skip                              ; regiao de tamanho zero

    inc bp
    add di, 24
    cmp bp, E820_CAPACITY
    jae .done

.skip:
    test ebx, ebx                         ; EBX = 0 marca a ultima entrada
    jnz .next_entry

.done:
    mov [e820_count], bp
    xor ax, ax
    mov es, ax
    ret

; ---------------------------------------------------------------------------
; fill_boot_info - escreve o bloco boot_info em 0x6000:0000.
; O layout e o de memory_boot_info_t (packed), espelhado em
; src/kernel/include/boot_info.h como boot_info_raw_t.
; ---------------------------------------------------------------------------
fill_boot_info:
    mov ax, BOOT_INFO_SEGMENT
    mov es, ax
    xor di, di

    ; Zera os 64 primeiros bytes antes de gravar os campos.
    mov cx, 64
    xor al, al
    rep stosb
    xor di, di

    mov al, [boot_drive]
    mov [es:di + 0], al                   ; boot_drive
    mov byte [es:di + 1], 0               ; video_mode = MEMORY_VIDEO_MODE_TEXT

    mov ax, BI_HAS_IO_BUFFER | BI_HAS_FRAMEBUFFER
    cmp word [e820_count], 0
    je .no_map
    or ax, BI_HAS_MEMORY_MAP
.no_map:
    mov [es:di + 2], ax                   ; flags

    mov dword [es:di + 4], E820_SEGMENT << 16   ; mmap_entries_far = 0x6100:0000
    mov ax, [e820_count]
    mov [es:di + 8], ax                   ; mmap_count
    mov word [es:di + 10], E820_CAPACITY  ; mmap_capacity

    mov dword [es:di + 12], HEAP_BASE     ; heap_base
    mov dword [es:di + 16], HEAP_CAPACITY ; heap_capacity
    mov dword [es:di + 20], 0             ; heap_offset

    mov dword [es:di + 24], IO_BUFFER_FAR ; io_buffer_far
    mov dword [es:di + 28], IO_BUFFER_SIZE

    ; Framebuffer: o modo ativo no boot e o texto 80x25 em 0xB8000.
    ; Descreve-lo aqui e mais honesto do que gravar zeros: e o framebuffer
    ; que existe de fato quando o kernel assume. fb_width/fb_height contam
    ; celulas de caractere (nao pixels) enquanto video_mode == TEXT.
    mov dword [es:di + 32], TEXT_FB_FAR   ; framebuffer_far
    mov word [es:di + 36], 80             ; fb_width  (colunas)
    mov word [es:di + 38], 25             ; fb_height (linhas)
    mov word [es:di + 40], 160            ; fb_pitch  (bytes por linha)
    mov byte [es:di + 42], 16             ; fb_bpp    (bits por celula)
    mov byte [es:di + 43], 0              ; fb_flags

    xor ax, ax
    mov es, ax
    ret

; ---------------------------------------------------------------------------
; print - escreve a string ASCIIZ em DS:SI usando teletype da BIOS.
; ---------------------------------------------------------------------------
print:
    push ax
    push bx
.loop:
    lodsb
    test al, al
    jz .done
    mov ah, 0x0E
    mov bh, 0
    int 0x10
    jmp .loop
.done:
    pop bx
    pop ax
    ret

halt_forever:
    cli
.spin:
    hlt
    jmp .spin

; ---------------------------------------------------------------------------
; Dados
; ---------------------------------------------------------------------------
boot_drive:     db 0
retries:        db 0
lba:            dw 0
dest_segment:   dw 0
e820_count:     dw 0

; Mensagens curtas de proposito: o setor de boot tem 512 bytes contados.
msg_loading:    db 'devboot', 13, 10, 0
msg_disk_error: db 'disk err', 13, 10, 0

times 510 - ($ - $$) db 0
dw 0xAA55
