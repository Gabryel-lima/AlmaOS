#pragma once

#include "stdint.h"

/** @brief Declarações de funções utilitárias para o bootloader.
 *  @file utility.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

/** Retorna o menor valor entre a e b.
 *
 *  @param a: O primeiro valor a ser comparado.
 *  @param b: O segundo valor a ser comparado.
 *  @return: O menor valor entre a e b.
 */
/* Protege o protótipo caso um macro com o mesmo nome já exista
 * (por exemplo, macros genéricos definidos em headers do kernel).
 * Se `min` estiver definido como macro, desfazemos para que a
 * declaração da função não seja expandida pelo pré-processador.
 */
#ifdef min
#undef min
#endif

int min(int a, int b);
/** Retorna o maior valor entre a e b.
 *
 *  @param a: O primeiro valor a ser comparado.
 *  @param b: O segundo valor a ser comparado.
 *  @return: O maior valor entre a e b.
 */
/* Mesmo tratamento para `max`. */
#ifdef max
#undef max
#endif

int max(int a, int b);
/** Alinha um número para o próximo múltiplo de alignTo.
 *
 *  @param number: O número a ser alinhado.
 *  @param alignTo: O valor para o qual o número deve ser alinhado (deve ser uma potência de 2).
 *  @return: O número alinhado para o próximo múltiplo de alignTo.
 */
uint32_t align(uint32_t number, uint32_t alignTo);
