#include "../include/vga.h"
#include "../include/string.h"

int echo(int argc, char **argv) {
    int newline = 1;
    int i = 1;

    for (; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-newline") == 0) {
            newline = 0;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            vga_puts("O comando echo imprime argumentos na saída padrão.\n"
                     "Uso: echo [-n] [texto...]\n");
            return 0;
        } else {
            break;
        }
    }

    for (; i < argc; i++) {
        vga_puts(argv[i]);
        if (i < argc - 1) vga_putchar(' ');
    }
    if (newline) vga_putchar('\n');
    return 0;
}
