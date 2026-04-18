#pragma once

/** @file io.h
 *  @brief Operacoes de I/O em portas x86 (inb, outb, inw, outw).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Essas funcoes usam instrucoes de I/O em linha para ler e escrever 
 *  em portas de I/O x86. A funcao io_wait() fornece uma pequena pausa para dispositivos lentos.
 */

#include "stdint.h"

/** Escreve um byte em uma porta de I/O. 
 *  @param port A porta de I/O a escrever (16-bit).
 *  @param val O valor a ser escrito (8-bit).
*/
static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

/** Le um byte de uma porta de I/O.
 *  @param port A porta de I/O a ler (16-bit).
 *  @return O valor lido (8-bit). 
*/
static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/** Escreve um word (16-bit) em uma porta de I/O. 
 *  @param port A porta de I/O a escrever (16-bit).
 *  @param val O valor a ser escrito (16-bit).
 */
static inline void outw(uint16_t port, uint16_t val) {
    __asm__ volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/** Le um word (16-bit) de uma porta de I/O.
 *  @param port A porta de I/O a ler (16-bit).
 *  @return O valor lido (16-bit).
 */
static inline uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ volatile("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

/** Pequena pausa para I/O lento (escreve em porta 0x80 = POST diagnostic). 
 *  @param void Nao tem parametros.
 *  @return void Nao retorna nada.
*/
static inline void io_wait(void) {
    outb(0x80, 0);
}
