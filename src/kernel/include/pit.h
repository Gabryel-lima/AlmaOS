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

/** Inicializa o PIT no canal 0 com a frequencia desejada em Hz. 
 *  @param frequency: Frequencia desejada para as interrupcoes do PIT (em Hz).
 *  O valor de divisor é calculado como divisor = PIT_BASE_FREQ / frequency, 
 *  onde PIT_BASE_FREQ é a frequencia de entrada do PIT (normalmente 1193182 Hz).
*/
void pit_init(uint32_t frequency);

/** Retorna o contador global de ticks desde o boot. 
 *  Este contador é incrementado a cada interrupcao do PIT (IRQ 0), 
 *  e pode ser usado para medir o tempo decorrido ou implementar temporizadores.
*/
uint32_t pit_get_ticks(void);

/** Handler chamado pelo dispatcher de interrupcoes (IRQ 0). 
 *  Este handler deve ser registrado na IDT para o vetor 
 *  correspondente ao IRQ 0 (normalmente INT 0x20 ou 0x30, 
 *  dependendo do remapeamento do PIC).
*/
void pit_handler(void);
