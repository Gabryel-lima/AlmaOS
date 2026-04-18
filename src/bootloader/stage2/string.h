#pragma once

/** @brief Funções básicas de manipulação de strings para o bootloader.
 *  @file string.h
 *  @author Gabryel-lima
 *  @date 2026-03
 *
 *  Declarações das rotinas de string usadas pelo estágio 2.
 */

/** Procura a primeira ocorrência de chr em str.
 *  @param str: String terminada em NUL onde a busca será feita.
 *  @param chr: Caractere a ser procurado.
 *  @return: Ponteiro para a primeira ocorrência de chr em str, ou NULL se str for NULL ou chr não for encontrado.
 */
const char* strchr(const char* str, char chr);

/** Copia src para dst e garante terminação NUL.
 *  @param dst: Buffer de destino; deve ter espaço suficiente para src.
 *  @param src: String de origem.
 *  @return: Ponteiro original de dst. Se dst for NULL, retorna NULL. Se src for NULL, grava string vazia.
 */
char* strcpy(char* dst, const char* src);

/** Mede o comprimento de uma string terminada em NUL.
 *  @param str: String a ser medida. Deve apontar para uma sequência terminada em NUL.
 *  @return: Número de caracteres antes do primeiro caractere nulo.
 */
unsigned strlen(const char* str);
