#include "include/panic.h"
#include "include/vga.h"

void panic(const char *msg) {
    cli();
    vga_set_color(VGA_COLOR_WHITE, VGA_COLOR_RED);
    vga_puts("\n*** KERNEL PANIC ***\n");
    vga_puts(msg);
    vga_puts("\n\nSistema parado.\n");
    for (;;) halt();
}

void klog(const char *fmt, ...) {
    vga_set_color(VGA_COLOR_DARK_GREY, VGA_COLOR_BLACK);
    vga_puts("[LOG] ");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);

    vga_putchar('\n');
}
