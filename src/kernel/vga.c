#include "include/vga.h"
#include "include/io.h"
#include "include/string.h"

/* VGA text mode CRT controller ports */
#define VGA_CTRL_REG    0x3D4
#define VGA_DATA_REG    0x3D5

static int vga_col;
static int vga_row;
static uint8_t vga_attr;

static inline uint16_t vga_entry(char c, uint8_t attr) {
    return (uint16_t)c | ((uint16_t)attr << 8);
}

static void vga_update_cursor(void) {
    uint16_t pos = (uint16_t)(vga_row * VGA_WIDTH + vga_col);
    outb(VGA_CTRL_REG, 0x0F);
    outb(VGA_DATA_REG, (uint8_t)(pos & 0xFF));
    outb(VGA_CTRL_REG, 0x0E);
    outb(VGA_DATA_REG, (uint8_t)((pos >> 8) & 0xFF));
}

static void vga_scroll(void) {
    volatile uint16_t *vga = VGA_ADDR;
    /* move tudo uma linha para cima */
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++)
        vga[i] = vga[i + VGA_WIDTH];
    /* limpa a ultima linha */
    for (int i = (VGA_HEIGHT - 1) * VGA_WIDTH; i < VGA_HEIGHT * VGA_WIDTH; i++)
        vga[i] = vga_entry(' ', vga_attr);
    vga_row = VGA_HEIGHT - 1;
}

void vga_init(void) {
    vga_attr = (VGA_COLOR_BLACK << 4) | VGA_COLOR_LIGHT_GREY;
    vga_col = 0;
    vga_row = 0;
    vga_clear();
}

void vga_clear(void) {
    volatile uint16_t *vga = VGA_ADDR;
    for (int i = 0; i < VGA_WIDTH * VGA_HEIGHT; i++)
        vga[i] = vga_entry(' ', vga_attr);
    vga_col = 0;
    vga_row = 0;
    vga_update_cursor();
}

void vga_set_color(uint8_t fg, uint8_t bg) {
    vga_attr = (bg << 4) | (fg & 0x0F);
}

void vga_putchar(char c) {
    volatile uint16_t *vga = VGA_ADDR;

    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        vga_col = (vga_col + 8) & ~7;
    } else if (c == '\b') {
        if (vga_col > 0) {
            vga_col--;
            vga[vga_row * VGA_WIDTH + vga_col] = vga_entry(' ', vga_attr);
        }
    } else {
        vga[vga_row * VGA_WIDTH + vga_col] = vga_entry(c, vga_attr);
        vga_col++;
    }

    if (vga_col >= VGA_WIDTH) {
        vga_col = 0;
        vga_row++;
    }
    if (vga_row >= VGA_HEIGHT) {
        vga_scroll();
    }
    vga_update_cursor();
}

void vga_puts(const char *s) {
    while (*s)
        vga_putchar(*s++);
}

void vga_set_cursor(int col, int row) {
    vga_col = col;
    vga_row = row;
    vga_update_cursor();
}

int vga_get_col(void) { return vga_col; }
int vga_get_row(void) { return vga_row; }

/* ---- printf minimo ---- */

static void print_uint(uint32_t val, int base, int width, char pad) {
    char buf[12];
    int i = 0;
    if (val == 0) {
        buf[i++] = '0';
    } else {
        while (val) {
            uint32_t digit = val % base;
            buf[i++] = (digit < 10) ? ('0' + digit) : ('a' + digit - 10);
            val /= base;
        }
    }
    while (i < width) buf[i++] = pad;
    while (i--) vga_putchar(buf[i]);
}

static void print_int(int32_t val, int width, char pad) {
    if (val < 0) {
        vga_putchar('-');
        print_uint((uint32_t)(-val), 10, width ? width - 1 : 0, pad);
    } else {
        print_uint((uint32_t)val, 10, width, pad);
    }
}

void vga_vprintf(const char *fmt, va_list args) {
    while (*fmt) {
        if (*fmt != '%') {
            vga_putchar(*fmt++);
            continue;
        }
        fmt++;

        /* flags */
        char pad = ' ';
        int width = 0;
        if (*fmt == '0') { pad = '0'; fmt++; }
        while (*fmt >= '0' && *fmt <= '9') {
            width = width * 10 + (*fmt - '0');
            fmt++;
        }

        /* length modifier */
        int is_long = 0;
        if (*fmt == 'l') { is_long = 1; fmt++; }

        switch (*fmt) {
        case 'c':
            vga_putchar((char)va_arg(args, int));
            break;
        case 's': {
            const char *s = va_arg(args, const char *);
            if (!s) s = "(null)";
            vga_puts(s);
            break;
        }
        case 'd': case 'i':
            if (is_long)
                print_int((int32_t)va_arg(args, long), width, pad);
            else
                print_int(va_arg(args, int), width, pad);
            break;
        case 'u':
            if (is_long)
                print_uint((uint32_t)va_arg(args, unsigned long), 10, width, pad);
            else
                print_uint(va_arg(args, unsigned int), 10, width, pad);
            break;
        case 'x':
            if (is_long)
                print_uint((uint32_t)va_arg(args, unsigned long), 16, width, pad);
            else
                print_uint(va_arg(args, unsigned int), 16, width, pad);
            break;
        case 'p':
            vga_puts("0x");
            print_uint((uint32_t)(uintptr_t)va_arg(args, void *), 16, 8, '0');
            break;
        case '%':
            vga_putchar('%');
            break;
        default:
            vga_putchar('%');
            vga_putchar(*fmt);
            break;
        }
        fmt++;
    }
}

void vga_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vga_vprintf(fmt, args);
    va_end(args);
}
