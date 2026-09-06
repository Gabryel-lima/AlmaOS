#include "include/serial.h"
#include "include/io.h"

/* Deslocamentos dos registradores da UART 16550 a partir da porta base. */
#define UART_DATA           0   /* RBR/THR quando DLAB=0; divisor low quando DLAB=1 */
#define UART_INT_ENABLE     1   /* IER; divisor high quando DLAB=1 */
#define UART_FIFO_CTRL      2   /* FCR (escrita) */
#define UART_LINE_CTRL      3   /* LCR */
#define UART_MODEM_CTRL     4   /* MCR */
#define UART_LINE_STATUS    5   /* LSR */

#define LCR_DLAB            0x80    /* Divisor Latch Access Bit */
#define LCR_8N1             0x03    /* 8 bits de dados, sem paridade, 1 stop bit */

#define LSR_DATA_READY      0x01    /* Ha byte recebido no RBR */
#define LSR_THR_EMPTY       0x20    /* Transmit Holding Register vazio */

/* 115200 / 38400 = 3 */
#define BAUD_DIVISOR        3

static bool serial_enabled;

bool serial_init(void) {
    uint8_t probe;

    serial_enabled = false;

    outb(SERIAL_COM1_PORT + UART_INT_ENABLE, 0x00);     /* sem interrupcoes: polling */
    outb(SERIAL_COM1_PORT + UART_LINE_CTRL, LCR_DLAB);
    outb(SERIAL_COM1_PORT + UART_DATA, BAUD_DIVISOR & 0xFF);
    outb(SERIAL_COM1_PORT + UART_INT_ENABLE, (BAUD_DIVISOR >> 8) & 0xFF);
    outb(SERIAL_COM1_PORT + UART_LINE_CTRL, LCR_8N1);   /* limpa DLAB e fixa 8N1 */
    outb(SERIAL_COM1_PORT + UART_FIFO_CTRL, 0xC7);      /* FIFO ligado, limpo, trigger 14 */
    outb(SERIAL_COM1_PORT + UART_MODEM_CTRL, 0x0B);     /* DTR + RTS + OUT2 */

    /* Teste de loopback: sem ele, uma maquina sem UART aceitaria as escritas
     * em silencio e o kernel acharia que tem log serial quando nao tem. */
    outb(SERIAL_COM1_PORT + UART_MODEM_CTRL, 0x1E);     /* modo loopback */
    outb(SERIAL_COM1_PORT + UART_DATA, 0xAE);
    probe = inb(SERIAL_COM1_PORT + UART_DATA);
    if (probe != 0xAE)
        return false;

    outb(SERIAL_COM1_PORT + UART_MODEM_CTRL, 0x0B);     /* volta ao modo normal */
    serial_enabled = true;
    return true;
}

bool serial_is_enabled(void) {
    return serial_enabled;
}

/** Espera o THR esvaziar antes de escrever o proximo byte.
 *  @note O laco tem limite: se a UART parar de responder, o kernel perde o
 *  caractere em vez de travar para sempre num periferico opcional.
 */
static void serial_wait_ready(void) {
    for (uint32_t spins = 0; spins < 100000U; spins++) {
        if (inb(SERIAL_COM1_PORT + UART_LINE_STATUS) & LSR_THR_EMPTY)
            return;
    }
}

void serial_putchar(char c) {
    if (!serial_enabled)
        return;

    if (c == '\n') {
        serial_wait_ready();
        outb(SERIAL_COM1_PORT + UART_DATA, '\r');
    }

    serial_wait_ready();
    outb(SERIAL_COM1_PORT + UART_DATA, (uint8_t)c);
}

void serial_puts(const char *s) {
    if (!s)
        return;
    while (*s)
        serial_putchar(*s++);
}

bool serial_has_data(void) {
    if (!serial_enabled)
        return false;
    return (inb(SERIAL_COM1_PORT + UART_LINE_STATUS) & LSR_DATA_READY) != 0;
}

char serial_getchar(void) {
    if (!serial_has_data())
        return 0;
    return (char)inb(SERIAL_COM1_PORT + UART_DATA);
}
