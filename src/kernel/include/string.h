#pragma once

/** @file string.h
 *  @brief Funcoes de string e memoria minimas para o kernel (sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Esta é uma implementação mínima de string.h para uso no kernel,
 *  definindo funções básicas de manipulação de strings e memória 
 *  (ex: strlen, strcmp, memcpy, memset). Ela é compatível com C e C++.
 */

#include "stdint.h"
#include "stddef.h"

/** @brief Calcula o comprimento de uma string.
 *  @param s Ponteiro para a string.
 *  @return O número de caracteres na string, excluindo o caractere nulo.
 */
size_t strlen(const char *s);

/** @brief Compara duas strings.
 *  @param a Ponteiro para a primeira string.
 *  @param b Ponteiro para a segunda string.
 *  @return Um valor negativo, zero ou positivo se a for menor, 
 *  igual ou maior que b, respectivamente.
 */
int strcmp(const char *a, const char *b);

/** @brief Compara até n caracteres de duas strings.
 *  @param a Ponteiro para a primeira string.
 *  @param b Ponteiro para a segunda string.
 *  @param n Número máximo de caracteres a comparar.
 *  @return Um valor negativo, zero ou positivo se a for menor, 
 *  igual ou maior que b, respectivamente.
 */
int strncmp(const char *a, const char *b, size_t n);

/** @brief Copia uma string para outra.
 *  @param dst Ponteiro para o destino.
 *  @param src Ponteiro para a origem.
 *  @return Ponteiro para o destino.
 */
char *strcpy(char *dst, const char *src);

/** @brief Copia até n caracteres de uma string para outra.
 *  @param dst Ponteiro para o destino.
 *  @param src Ponteiro para a origem.
 *  @param n Número máximo de caracteres a copiar.
 *  @return Ponteiro para o destino.
 */
char *strncpy(char *dst, const char *src, size_t n);

/** @brief Copia n bytes de memória de uma área para outra.
 *  @param dst Ponteiro para o destino.
 *  @param src Ponteiro para a origem.
 *  @param n Número de bytes a copiar.
 *  @return Ponteiro para o destino.
 */
void *memcpy(void *dst, const void *src, size_t n);

/** @brief Preenche uma área de memória com um valor.
 *  @param dst Ponteiro para a área de memória.
 *  @param val Valor a preencher.
 *  @param n Número de bytes a preencher.
 *  @return Ponteiro para a área de memória.
 */
void *memset(void *dst, int val, size_t n);

/** @brief Compara n bytes de duas áreas de memória.
 *  @param a Ponteiro para a primeira área.
 *  @param b Ponteiro para a segunda área.
 *  @param n Número de bytes a comparar.
 *  @return Um valor negativo, zero ou positivo se a for menor, 
 *  igual ou maior que b, respectivamente.
 */
int memcmp(const void *a, const void *b, size_t n);
