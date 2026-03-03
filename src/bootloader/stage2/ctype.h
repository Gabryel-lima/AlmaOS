#pragma once

#include "stdint.h"
#include "stdbool.h"

/** Verifica se um caractere é uma letra minúscula.
 *
 *  @param chr: O caractere a ser verificado.
 *  @return: true se o caractere for uma letra minúscula, false caso contrário.
 */
bool islower(char chr);
/** Converte um caractere para maiúsculo.
 *
 *  @param chr: O caractere a ser convertido.
 *  @return: O caractere convertido para maiúsculo, se aplicável.
 */
char toupper(char chr);
