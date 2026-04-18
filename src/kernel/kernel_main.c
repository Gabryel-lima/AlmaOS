#include "include/kernel.h"
#include "include/vga.h"
#include "include/idt.h"
#include "include/pic.h"
#include "include/pit.h"
#include "include/keyboard.h"
#include "include/panic.h"
#include "include/shell.h"

/* Wrappers de IRQ que delegam para os drivers e enviam EOI */

static void irq0_handler(interrupt_frame_t *frame) {
    (void)frame;
    pit_handler();
    pic_send_eoi(0);
}

static void irq1_handler(interrupt_frame_t *frame) {
    (void)frame;
    keyboard_handler();
    pic_send_eoi(1);
}

void kernel_main(void) {
    /* 1. VGA text mode */
    vga_init();
    vga_set_color(VGA_COLOR_LIGHT_CYAN, VGA_COLOR_BLACK);
    vga_puts("AlmaOS kernel inicializando...\n\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    /* 2. IDT */
    idt_init();
    vga_puts("[OK] IDT carregada\n");

    /* 3. PIC */
    pic_remap(PIC_MASTER_OFFSET, PIC_SLAVE_OFFSET);
    vga_puts("[OK] PIC remapeado (IRQ 0x20-0x2F)\n");

    /* 4. PIT (~100 Hz) */
    pit_init(PIT_DEFAULT_FREQ);
    idt_register_handler(0x20, irq0_handler);
    pic_clear_mask(0);
    vga_puts("[OK] PIT configurado (100 Hz)\n");

    /* 5. Teclado */
    keyboard_init();
    idt_register_handler(0x21, irq1_handler);
    pic_clear_mask(1);
    vga_puts("[OK] Teclado habilitado (IRQ 1)\n");

    /* 6. Habilita interrupcoes */
    sti();
    vga_puts("[OK] Interrupcoes habilitadas\n\n");

    /* 7. Mensagem de boas-vindas */
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_puts("Bem-vindo ao AlmaOS!\n");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    vga_puts("Digite 'help' para lista de comandos.\n\n");

    /* 8. Shell interativo */
    shell_init();
    shell_run();

    /* Nunca deve chegar aqui */
    panic("shell_run retornou inesperadamente");
}
