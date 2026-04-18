#include "include/idt.h"
#include "include/io.h"
#include "include/vga.h"
#include "include/string.h"

/* Tabela de stubs definida em isr_stubs.asm */
extern uint32_t isr_stub_table[48];

/** Entrada da IDT (8 bytes por vetor). */
typedef struct __attribute__((packed)) {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} idt_entry_t;

/** Ponteiro para a IDT (carregado por LIDT). */
typedef struct __attribute__((packed)) {
    uint16_t limit;
    uint32_t base;
} idt_ptr_t;

static idt_entry_t idt[IDT_ENTRIES];
static idt_ptr_t   idtp;
static isr_handler_t handlers[IDT_ENTRIES];

static void idt_set_gate(uint8_t n, uint32_t handler, uint16_t sel, uint8_t flags) {
    idt[n].offset_low  = (uint16_t)(handler & 0xFFFF);
    idt[n].selector    = sel;
    idt[n].zero        = 0;
    idt[n].type_attr   = flags;
    idt[n].offset_high = (uint16_t)((handler >> 16) & 0xFFFF);
}

void idt_init(void) {
    memset(idt, 0, sizeof(idt));
    memset(handlers, 0, sizeof(handlers));

    /* registra os 48 stubs (excecoes 0-31 + IRQs 32-47) */
    for (int i = 0; i < 48; i++)
        idt_set_gate((uint8_t)i, isr_stub_table[i], 0x08, 0x8E);

    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;

    __asm__ volatile("lidt %0" : : "m"(idtp));
}

void idt_register_handler(uint8_t vector, isr_handler_t handler) {
    handlers[vector] = handler;
}

/* Nomes das excecoes para mensagens de erro */
static const char *exception_names[32] = {
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Overflow",
    "Bound Range Exceeded",
    "Invalid Opcode",
    "Device Not Available",
    "Double Fault",
    "Coprocessor Segment Overrun",
    "Invalid TSS",
    "Segment Not Present",
    "Stack-Segment Fault",
    "General Protection Fault",
    "Page Fault",
    "Reserved",
    "x87 FPU Exception",
    "Alignment Check",
    "Machine Check",
    "SIMD FPU Exception",
    "Virtualization Exception",
    "Control Protection",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved", "Reserved", "Reserved",
    "Reserved", "Reserved"
};

/** Dispatcher central chamado por isr_common em isr_stubs.asm. */
void isr_dispatch(interrupt_frame_t *frame) {
    if (handlers[frame->isr_num]) {
        handlers[frame->isr_num](frame);
        return;
    }

    /* excecao sem handler registrado: imprime e para */
    if (frame->isr_num < 32) {
        vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
        vga_printf("\n*** EXCEPTION: %s (#%u) ***\n", exception_names[frame->isr_num], frame->isr_num);
        vga_printf("Error code: 0x%08x\n", frame->error_code);
        vga_printf("EIP: 0x%08x  CS: 0x%04x  EFLAGS: 0x%08x\n",
                   frame->eip, frame->cs, frame->eflags);
        vga_printf("EAX: 0x%08x  EBX: 0x%08x  ECX: 0x%08x  EDX: 0x%08x\n",
                   frame->eax, frame->ebx, frame->ecx, frame->edx);
        vga_printf("ESI: 0x%08x  EDI: 0x%08x  EBP: 0x%08x\n",
                   frame->esi, frame->edi, frame->ebp);
        cli();
        for (;;) halt();
    }
}
