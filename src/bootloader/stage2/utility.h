#pragma once

#include "stdint.h"

/** Retorna o menor valor entre a e b.
 *
 *  @param a: O primeiro valor a ser comparado.
 *  @param b: O segundo valor a ser comparado.
 *  @return: O menor valor entre a e b.
 */
int min(int a, int b);
/** Retorna o maior valor entre a e b.
 *
 *  @param a: O primeiro valor a ser comparado.
 *  @param b: O segundo valor a ser comparado.
 *  @return: O maior valor entre a e b.
 */
int max(int a, int b);
/** Alinha um número para o próximo múltiplo de alignTo.
 *
 *  @param number: O número a ser alinhado.
 *  @param alignTo: O valor para o qual o número deve ser alinhado (deve ser uma potência de 2).
 *  @return: O número alinhado para o próximo múltiplo de alignTo.
 */
uint32_t align(uint32_t number, uint32_t alignTo);
