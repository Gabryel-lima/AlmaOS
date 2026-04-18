#pragma once

/** @brief Declarações de funções para manipulação de caracteres, como verificação de letras minúsculas e conversão para maiúsculas. 
 * Estas funções são utilizadas em várias partes do código para processar caracteres, especialmente ao lidar com nomes de arquivos e diretórios na FAT, onde a distinção entre maiúsculas e minúsculas é importante.
 * @file ctype.h
 * @author Gabryel-lima
 * @date 2026-03
 */

#include "stdint.h"
#include "stdbool.h"

/** Verifica se um caractere é uma letra minúscula.
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
