#include "include/shell.h"
#include "include/vga.h"
#include "include/keyboard.h"
#include "include/pit.h"
#include "include/panic.h"
#include "include/boot_info.h"
#include "include/string.h"
#include "include/io.h"

#define CMD_MAX_LEN 64
#define PROMPT "AlmaOS> "

static char cmd_buf[CMD_MAX_LEN];
static int  cmd_len;

static const char *e820_type_str(uint32_t type) {
    switch (type) {
    case 1:  return "Usable";
    case 2:  return "Reserved";
    case 3:  return "ACPI Reclaimable";
    case 4:  return "ACPI NVS";
    case 5:  return "Bad Memory";
    default: return "Unknown";
    }
}

static void cmd_help(void) {
    vga_puts("Comandos disponiveis:\n");
    vga_puts("  help   - mostra esta ajuda\n");
    vga_puts("  clear  - limpa a tela\n");
    vga_puts("  mem    - mostra mapa de memoria e heap\n");
    vga_puts("  ticks  - mostra ticks do PIT\n");
    vga_puts("  reboot - reinicia o sistema\n");
}

static void cmd_mem(void) {
    const boot_info_raw_t *bi = boot_info_get();
    const e820_entry_t *entries = boot_info_e820_entries();

    vga_printf("Boot drive: 0x%02x\n", bi->boot_drive);
    vga_printf("Mapa de memoria: %u entradas\n", bi->mmap_count);

    for (uint16_t i = 0; i < bi->mmap_count; i++) {
        uint32_t base_lo = (uint32_t)entries[i].base;
        uint32_t len_lo  = (uint32_t)entries[i].length;
        uint32_t end     = base_lo + len_lo - 1;
        vga_printf("  0x%08x - 0x%08x: %s (%u bytes)\n",
                   base_lo, end, e820_type_str(entries[i].type), len_lo);
    }

    vga_printf("Kernel heap: base=0x%08x, size=%u, used=%u\n",
               bi->heap_base, bi->heap_capacity, bi->heap_offset);
}

static void cmd_ticks(void) {
    vga_printf("System ticks: %u\n", pit_get_ticks());
}

static void cmd_reboot(void) {
    vga_puts("Reiniciando...\n");
    /* pulsa a linha de reset via controlador de teclado */
    outb(0x64, 0xFE);
    /* se falhar, triple fault */
    cli();
    for (;;) halt();
}

static void execute_cmd(void) {
    cmd_buf[cmd_len] = '\0';

    if (cmd_len == 0)
        return;

    if (strcmp(cmd_buf, "help") == 0)
        cmd_help();
    else if (strcmp(cmd_buf, "clear") == 0)
        vga_clear();
    else if (strcmp(cmd_buf, "mem") == 0)
        cmd_mem();
    else if (strcmp(cmd_buf, "ticks") == 0)
        cmd_ticks();
    else if (strcmp(cmd_buf, "reboot") == 0)
        cmd_reboot();
    else
        vga_printf("Comando desconhecido: %s\n", cmd_buf);
}

void shell_init(void) {
    cmd_len = 0;
}

void shell_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_puts(PROMPT);
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);

    for (;;) {
        char c = keyboard_getchar();

        if (c == '\n') {
            vga_putchar('\n');
            execute_cmd();
            cmd_len = 0;
            vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
            vga_puts(PROMPT);
            vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
        } else if (c == '\b') {
            if (cmd_len > 0) {
                cmd_len--;
                vga_putchar('\b');
            }
        } else if (cmd_len < CMD_MAX_LEN - 1) {
            cmd_buf[cmd_len++] = c;
            vga_putchar(c);
        }
    }
}
