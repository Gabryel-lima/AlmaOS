#include "include/shell.h"
#include "include/vga.h"
#include "include/keyboard.h"
#include "include/pit.h"
#include "include/panic.h"
#include "include/boot_info.h"
#include "include/string.h"
#include "include/io.h"

#include "root/echo.h"

#define CMD_MAX_LEN 64      // Tamanho máximo do comando (incluindo null terminator)
#define PROMPT "AlmaOS> "   // Prompt do shell

// Tipo de função para comandos builtin
typedef int (*cmd_fn_t)(int argc, char **argv);

/** Estrutura para entrada de comando builtin 
 *  @param name Nome do comando (ex: "help")
 *  @param fn Ponteiro para a função que implementa o comando
*/
typedef struct cmd_entry_t { 
    const char *name; // Nome do comando
    cmd_fn_t fn;      // Ponteiro para a função que implementa o comando
} cmd_entry_t;

static char cmd_buf[CMD_MAX_LEN]; // Buffer para armazenar o comando digitado pelo usuário
static int  cmd_len;    // Comprimento atual do comando no buffer

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

static int cmd_help(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_puts("Comandos disponiveis:\n");
    vga_puts("  help   - mostra esta ajuda\n");
    vga_puts("  clear  - limpa a tela\n");
    vga_puts("  mem    - mostra mapa de memoria e heap\n");
    vga_puts("  ticks  - mostra ticks do PIT\n");
    vga_puts("  reboot - reinicia o sistema\n");
    vga_puts("  echo   - exibe texto (ex: echo Hello)\n");
    return 0;
}

static int cmd_clear(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_clear();
    return 0;
}

static int cmd_mem(int argc, char **argv) {
    (void)argc; (void)argv;
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
    return 0;
}

static int cmd_ticks(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_printf("System ticks: %u\n", pit_get_ticks());
    return 0;
}

static int cmd_reboot(int argc, char **argv) {
    (void)argc; (void)argv;
    vga_puts("Reiniciando...\n");
    outb(0x64, 0xFE);
    cli();
    for (;;) halt();
    return 0;
}

static const cmd_entry_t cmd_table[] = {
    {"help",   cmd_help},
    {"clear",  cmd_clear},
    {"mem",    cmd_mem},
    {"ticks",  cmd_ticks},
    {"reboot", cmd_reboot},
    {"echo",   echo},
};

// Número de comandos na tabela (calculado automaticamente)
#define CMD_TABLE_SIZE (sizeof(cmd_table) / sizeof(cmd_table[0]))

static void execute_cmd(void) {
    cmd_buf[cmd_len] = '\0';

    if (cmd_len == 0)
        return;

    /* Tokenizar cmd_buf em argv (modifica in-place) */
    char *argv[16];
    int argc = 0;
    char *p = cmd_buf;

    while (*p && argc < 16) {
        while (*p == ' ') p++;
        if (!*p) break;
        argv[argc++] = p;
        while (*p && *p != ' ') p++;
        if (*p) *p++ = '\0';
    }

    if (argc == 0)
        return;

    for (size_t i = 0; i < CMD_TABLE_SIZE; i++) {
        if (strcmp(argv[0], cmd_table[i].name) == 0) {
            cmd_table[i].fn(argc, argv);
            return;
        }
    }
    vga_printf("Comando desconhecido: %s\n", argv[0]);
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
