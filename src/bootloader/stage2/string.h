#pragma once

/** @brief Declarações de funções de manipulação de strings. 
 * @file string.h
 * @author Gabryel-lima
 * @date 2024-06
 */

/** Declarações de funções de manipulação de strings. 
 * @param str: A string a ser processada.
 * @param chr: O caractere a ser encontrado na string.
 * @return: Um ponteiro para a primeira ocorrência do caractere na string, ou NULL se o caractere não for encontrado.
 */
const char* strchr(const char* str, char chr);
/** Copia a string src para dst, incluindo o caractere nulo de terminação.
 * @param dst: O buffer de destino onde a string será copiada. Deve ser grande o suficiente para conter a string src.
 * @param src: A string de origem a ser copiada.
 * @return: Um ponteiro para dst.
 */
char* strcpy(char* dst, const char* src);
/** Copia até n caracteres da string src para dst. Se src for menor que n caracteres, dst será preenchido com caracteres nulos até atingir n caracteres.
 * @param dst: O buffer de destino onde a string será copiada. Deve ser grande o suficiente para conter n caracteres.
 * @param src: A string de origem a ser copiada.
 * @param n: O número máximo de caracteres a serem copiados de src para dst.
 * @return: Um ponteiro para dst.
 */
unsigned strlen(const char* str);
