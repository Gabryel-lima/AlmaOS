#include "include/pit.h"
#include "include/io.h"

#define PIT_BASE_FREQ   1193182     /* cristal do 8254 em Hz */

static volatile uint32_t tick_count; // Contador de ticks desde a inicialização do PIT

void pit_init(uint32_t frequency) {
    if (frequency == 0) frequency = PIT_DEFAULT_FREQ;

    uint32_t divisor = PIT_BASE_FREQ / frequency;
    if (divisor > 0xFFFF) divisor = 0xFFFF;
    if (divisor < 1) divisor = 1;

    tick_count = 0;

    /* canal 0, access mode lobyte/hibyte, mode 3 (square wave) */
    outb(PIT_CMD, 0x36);
    outb(PIT_CHANNEL0_DATA, (uint8_t)(divisor & 0xFF));
    outb(PIT_CHANNEL0_DATA, (uint8_t)((divisor >> 8) & 0xFF));
}

uint32_t pit_get_ticks(void) {
    return tick_count;
}

void pit_handler(void) {
    tick_count++;
}
