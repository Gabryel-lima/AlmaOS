#include "include/pic.h"
#include "include/io.h"

void pic_remap(uint8_t master_offset, uint8_t slave_offset) {
    uint8_t mask1 = inb(PIC1_DATA);    /* salva mascaras atuais */
    uint8_t mask2 = inb(PIC2_DATA);

    /* ICW1: inicia sequencia de inicializacao, ICW4 necessario */
    outb(PIC1_CMD, 0x11);  io_wait();
    outb(PIC2_CMD, 0x11);  io_wait();

    /* ICW2: offsets dos vetores */
    outb(PIC1_DATA, master_offset); io_wait();
    outb(PIC2_DATA, slave_offset);  io_wait();

    /* ICW3: master tem slave no IRQ2, slave tem cascade identity 2 */
    outb(PIC1_DATA, 0x04); io_wait();
    outb(PIC2_DATA, 0x02); io_wait();

    /* ICW4: modo 8086 */
    outb(PIC1_DATA, 0x01); io_wait();
    outb(PIC2_DATA, 0x01); io_wait();

    /* restaura mascaras */
    outb(PIC1_DATA, mask1);
    outb(PIC2_DATA, mask2);
}

void pic_send_eoi(uint8_t irq) {
    if (irq >= 8)
        outb(PIC2_CMD, PIC_EOI);
    outb(PIC1_CMD, PIC_EOI);
}

void pic_set_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t val = inb(port) | (1 << irq);
    outb(port, val);
}

void pic_clear_mask(uint8_t irq) {
    uint16_t port;
    if (irq < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        irq -= 8;
    }
    uint8_t val = inb(port) & ~(1 << irq);
    outb(port, val);
}
