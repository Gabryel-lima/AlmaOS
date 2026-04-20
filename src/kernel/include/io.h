#pragma once

/** @file io.h
 *  @brief Operacoes de I/O em portas x86 (inb, outb, inw, outw).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Essas funcoes usam instrucoes de I/O em linha para ler e escrever 
 *  em portas de I/O x86. A funcao io_wait() fornece uma pequena pausa para dispositivos lentos.
 */

#include "stdint.h"

/** @brief Escreve um byte em uma porta de I/O.
 *  @param port Porta de I/O destino (16-bit).
 *  @param val Valor a ser escrito (8-bit).
 */
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/** @brief Le um byte de uma porta de I/O.
 *  @param port Porta de I/O fonte (16-bit).
 *  @return Valor lido (8-bit).
 */
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/** @brief Escreve um word (16-bit) em uma porta de I/O.
 *  @param port Porta de I/O destino (16-bit).
 *  @param val Valor a ser escrito (16-bit).
 */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/** @brief Le um word (16-bit) de uma porta de I/O.
 *  @param port Porta de I/O fonte (16-bit).
 *  @return Valor lido (16-bit).
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/** @brief Pequena pausa para I/O lento (escreve na porta 0x80 = POST diagnostic).
 */
static inline void io_wait(void) {
    outb(0x80, 0);
}
