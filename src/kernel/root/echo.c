#include "../include/vga.h"
#include "../include/string.h"

int echo(int argc, char **argv) {
    for (int i = 1; i < argc; i++) { 
      vga_puts(argv[i]);
	    if (i < argc - 1) vga_putchar(' ');
    }
    vga_putchar('\n');
    return 0;
}
