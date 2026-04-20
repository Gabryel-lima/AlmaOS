#pragma once

/** @file pic.h
 *  @brief Programmable Interrupt Controller (8259A) - remap e controle.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note O PIC é um chip de hardware responsável por gerenciar as interrupções 
 *  de hardware. Ele é composto por um PIC mestre e um PIC escravo, cada um 
 *  controlando um conjunto de IRQs.
 */

#include "kernel.h"

#define PIC1_CMD    0x20    /* Porta de comando do PIC mestre */
#define PIC1_DATA   0x21    /* Porta de dados do PIC mestre */
#define PIC2_CMD    0xA0    /* Porta de comando do PIC escravo */
#define PIC2_DATA   0xA1    /* Porta de dados do PIC escravo */

#define PIC_EOI     0x20    /* Comando para enviar End-Of-Interrupt (EOI) ao PIC */

/** Offset padrao para remapeamento das IRQs. */
#define PIC_MASTER_OFFSET   0x20    /* IRQ 0-7  → INT 0x20-0x27 */
#define PIC_SLAVE_OFFSET    0x28    /* IRQ 8-15 → INT 0x28-0x2F */

/** @brief Remapeia o PIC para que IRQs nao colidam com excecoes da CPU.
 *  @param master_offset Offset para o PIC mestre (normalmente 0x20).
 *  @param slave_offset Offset para o PIC escravo (normalmente 0x28).
 */
void pic_remap(uint8_t master_offset, uint8_t slave_offset);

/** @brief Envia End-Of-Interrupt para o PIC correto.
 *  @param irq Numero da IRQ atendida (0-7 = mestre, 8-15 = escravo).
 */
void pic_send_eoi(uint8_t irq);

/** @brief Mascara (desabilita) uma IRQ especifica.
 *  @param irq Numero da IRQ a mascarar (0-15).
 */
void pic_set_mask(uint8_t irq);

/** @brief Desmascara (habilita) uma IRQ especifica.
 *  @param irq Numero da IRQ a desmascarar (0-15).
 */
void pic_clear_mask(uint8_t irq);
