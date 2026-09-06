#include "include/realmode.h"
#include "include/pic.h"
#include "include/io.h"
#include "include/string.h"

/* Corpo do trampolim, definido em realmode.asm. Sao rotulos, nao variaveis:
 * o que interessa e o endereco de cada um. */
extern uint8_t realmode_stub_start[];
extern uint8_t realmode_stub_end[];

/** Bloco de estado compartilhado com realmode.asm, em REALMODE_DATA_ADDR.
 *
 *  O layout e replicado la como constantes `RM_*`; qualquer mudanca aqui
 *  precisa ser espelhada nelas. As assercoes estaticas abaixo transformam um
 *  desencontro em erro de compilacao em vez de num travamento no boot.
 *
 *  @param saved_esp ESP do kernel, guardado antes de sair do modo protegido.
 *  @param saved_gdtr,saved_idtr Pseudo-descritores salvos com SGDT/SIDT.
 *  @param ivt_idtr Pseudo-descritor da IVT da BIOS (base 0, limite 0x3FF).
 *  @param bios_handler Entrada da IVT do vetor pedido (offset | segmento<<16).
 */
typedef struct __attribute__((packed)) {
    uint32_t saved_esp;         /* 0x00 */
    uint8_t  saved_gdtr[6];     /* 0x04 */
    uint8_t  pad0[2];           /* 0x0A */
    uint8_t  saved_idtr[6];     /* 0x0C */
    uint8_t  pad1[2];           /* 0x12 */
    uint8_t  ivt_idtr[6];       /* 0x14 */
    uint8_t  pad2[2];           /* 0x1A */
    uint32_t bios_handler;      /* 0x1C */
    uint32_t eax;               /* 0x20 */
    uint32_t ebx;               /* 0x24 */
    uint32_t ecx;               /* 0x28 */
    uint32_t edx;               /* 0x2C */
    uint32_t esi;               /* 0x30 */
    uint32_t edi;               /* 0x34 */
    uint32_t ebp;               /* 0x38 */
    uint16_t ds;                /* 0x3C */
    uint16_t es;                /* 0x3E */
    uint32_t eflags;            /* 0x40 */
} realmode_state_t;             /* 0x44 */

_Static_assert(sizeof(realmode_state_t) == 0x44, "realmode_state_t mudou de tamanho; ajuste os RM_* em realmode.asm");
_Static_assert(__builtin_offsetof(realmode_state_t, saved_gdtr) == 0x04, "RM_SAVED_GDTR");
_Static_assert(__builtin_offsetof(realmode_state_t, saved_idtr) == 0x0C, "RM_SAVED_IDTR");
_Static_assert(__builtin_offsetof(realmode_state_t, ivt_idtr) == 0x14, "RM_IVT_IDTR");
_Static_assert(__builtin_offsetof(realmode_state_t, bios_handler) == 0x1C, "RM_BIOS_HANDLER");
_Static_assert(__builtin_offsetof(realmode_state_t, eax) == 0x20, "RM_EAX");
_Static_assert(__builtin_offsetof(realmode_state_t, ds) == 0x3C, "RM_DS");
_Static_assert(__builtin_offsetof(realmode_state_t, eflags) == 0x40, "RM_EFLAGS");

/* O stub nao pode invadir o bloco de estado que vem logo depois dele. */
#define REALMODE_STUB_MAX_SIZE (REALMODE_DATA_ADDR - REALMODE_STUB_ADDR)

/* Indices dos descritores de 16 bits na GDT montada por boot.asm. */
#define GDT_INDEX_CODE16 3
#define GDT_INDEX_DATA16 4

/** Pseudo-descritor lido com SGDT. */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} gdt_ptr_t;

static bool realmode_ready;

static realmode_state_t *realmode_state(void) {
    return (realmode_state_t *)REALMODE_DATA_ADDR;
}

/** Escreve a base de um descritor da GDT, mantendo o resto intacto.
 *
 *  Os 32 bits de base ficam espalhados em tres campos do descritor de 8
 *  bytes (bytes 2-3, 4 e 7) por compatibilidade com o 80286.
 *
 *  @param descriptor Ponteiro para o descritor de 8 bytes.
 *  @param base Endereco linear que vira a base do segmento.
 */
static void gdt_set_base(uint8_t *descriptor, uint32_t base) {
    descriptor[2] = (uint8_t)(base & 0xFF);
    descriptor[3] = (uint8_t)((base >> 8) & 0xFF);
    descriptor[4] = (uint8_t)((base >> 16) & 0xFF);
    descriptor[7] = (uint8_t)((base >> 24) & 0xFF);
}

/** Aponta os descritores de 16 bits da GDT para a base do stub.
 *
 *  boot.asm declara esses dois descritores com base zero. Eles tem limite de
 *  64 KB (granularidade de byte, que e o que torna o modo real previsivel
 *  depois de PE=0), entao com base zero nem alcancariam o stub, que mora em
 *  REALMODE_STUB_ADDR. A base tem de ser exatamente essa, porque e a mesma
 *  que o segmento de modo real REALMODE_STUB_SEG produz depois da troca.
 *
 *  @return true se a GDT carregada tem espaco para os dois descritores.
 */
static bool realmode_patch_gdt(void) {
    gdt_ptr_t gdtr;
    uint8_t *gdt;

    __asm__ volatile("sgdt %0" : "=m"(gdtr));

    /* Precisa existir ate o descritor de indice 4 (offset 0x20..0x27). */
    if (gdtr.limit < (GDT_INDEX_DATA16 * 8 + 7))
        return false;

    gdt = (uint8_t *)gdtr.base;
    gdt_set_base(gdt + GDT_INDEX_CODE16 * 8, REALMODE_STUB_ADDR);
    gdt_set_base(gdt + GDT_INDEX_DATA16 * 8, REALMODE_STUB_ADDR);
    return true;
}

bool realmode_init(void) {
    realmode_state_t *state = realmode_state();
    size_t stub_size = (size_t)(realmode_stub_end - realmode_stub_start);

    realmode_ready = false;

    if (stub_size == 0 || stub_size > REALMODE_STUB_MAX_SIZE)
        return false;

    if (!realmode_patch_gdt())
        return false;

    memcpy((void *)REALMODE_STUB_ADDR, realmode_stub_start, stub_size);
    memset(state, 0, sizeof(*state));

    /* Pseudo-descritor da IVT da BIOS: 256 vetores de 4 bytes a partir de 0. */
    state->ivt_idtr[0] = 0xFF;  /* limite low  */
    state->ivt_idtr[1] = 0x03;  /* limite high (0x03FF) */
    state->ivt_idtr[2] = 0x00;  /* base 0      */
    state->ivt_idtr[3] = 0x00;
    state->ivt_idtr[4] = 0x00;
    state->ivt_idtr[5] = 0x00;

    realmode_ready = true;
    return true;
}

bool realmode_is_ready(void) {
    return realmode_ready;
}

bool realmode_int(uint8_t vector, realmode_regs_t *regs) {
    realmode_state_t *state = realmode_state();
    /* A IVT ainda esta intacta em 0x0000: nem o stage2 nem o kernel escrevem
     * ali. Ler o vetor aqui, em modo protegido, evita codigo automodificavel
     * no stub — o trampolim so precisa de um far call para este ponteiro. */
    const volatile uint32_t *ivt = (const volatile uint32_t *)0;
    uint32_t handler;
    uint8_t mask_master;
    uint8_t mask_slave;

    if (!realmode_ready || !regs)
        return false;

    handler = ivt[vector];
    if (handler == 0)
        return false;   /* vetor sem handler: nao vale saltar para 0000:0000 */

    state->bios_handler = handler;
    state->eax = regs->eax;
    state->ebx = regs->ebx;
    state->ecx = regs->ecx;
    state->edx = regs->edx;
    state->esi = regs->esi;
    state->edi = regs->edi;
    state->ebp = regs->ebp;
    state->ds  = regs->ds;
    state->es  = regs->es;
    state->eflags = 0;

    /* O stub roda com CLI, mas nada impede a BIOS de dar STI por conta
     * propria. Se uma IRQ chegasse ali, o salto sairia pela IVT da BIOS
     * enquanto o PIC ja esta remapeado para 0x20-0x2F pelo kernel — ou seja,
     * o vetor errado. Mascarar tudo fecha essa janela. */
    mask_master = inb(PIC1_DATA);
    mask_slave  = inb(PIC2_DATA);
    outb(PIC1_DATA, 0xFF);
    outb(PIC2_DATA, 0xFF);

    ((void (*)(void))REALMODE_STUB_ADDR)();

    outb(PIC1_DATA, mask_master);
    outb(PIC2_DATA, mask_slave);

    regs->eax = state->eax;
    regs->ebx = state->ebx;
    regs->ecx = state->ecx;
    regs->edx = state->edx;
    regs->esi = state->esi;
    regs->edi = state->edi;
    regs->ebp = state->ebp;
    regs->ds  = state->ds;
    regs->es  = state->es;
    regs->eflags = state->eflags;
    return true;
}
