#pragma once

/** @file pit.h
 *  @brief Programmable Interval Timer (8253/8254) - canal 0, ~100 Hz.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note O PIT é um chip de hardware usado para gerar interrupções 
 *  periódicas em um intervalo de tempo configurável. Ele é comumente 
 *  usado para manter o controle do tempo no sistema operacional.
 */

#include "kernel.h"

#define PIT_CHANNEL0_DATA   0x40    /* Porta de dados do canal 0 do PIT */
#define PIT_CMD             0x43    /* Porta de comando do PIT */

#define PIT_DEFAULT_FREQ    100     /* Hz */

/** @brief Inicializa o PIT no canal 0 com a frequencia desejada em Hz.
 *  @details O divisor e calculado como PIT_BASE_FREQ / frequency, onde PIT_BASE_FREQ = 1193182 Hz.
 *  @param frequency Frequencia desejada para as interrupcoes (em Hz).
 */
void pit_init(uint32_t frequency);

/** @brief Retorna o contador global de ticks desde o boot.
 *  @note Incrementado a cada interrupcao do PIT (IRQ 0). Pode ser usado para medir tempo.
 *  @return Numero de ticks acumulados desde a inicializacao.
 */
uint32_t pit_get_ticks(void);

/** @brief Handler do PIT chamado pelo dispatcher de interrupcoes (IRQ 0).
 *  @note Registrar na IDT para INT 0x20 (apos remapeamento do PIC).
 */
void pit_handler(void);
