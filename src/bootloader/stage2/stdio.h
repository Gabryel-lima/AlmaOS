#pragma once

#include "stdbool.h"
#include "stdint.h"

/** @brief Funções de E/S padrão do estágio 2 do bootloader.
 *  @file stdio.h
 *  @author Gabryel-lima
 *  @date 2024-06
 *
 *  Expõe saída de texto limitada para o console: caracteres ASCII, bytes UTF-8,
 *  wide chars UTF-16 e uma printf parcial com suporte a strings normais e far.
 */

/** Escreve um caractere na saída de vídeo.
 *
 *  @param c: Caractere a ser exibido.
 */
void putc(char c);
/** Decodifica bytes UTF-8 incrementalmente e emite o caractere completo quando disponível.
 *
 *  @param c: Próximo byte da sequência UTF-8.
 *  @note O decodificador mantém estado entre chamadas até completar a sequência.
 */
void putc_utf8(char c);
/** Escreve uma string terminada em NUL armazenada em memória próxima.
 *
 *  @param str: String a ser exibida.
 */
void puts(const char* str);
/** Escreve uma string terminada em NUL armazenada em memória far.
 *
 *  @param str: String a ser exibida.
 */
void puts_f(const char far* str);
/** Escreve um caractere UTF-16 convertendo-o para CP437 quando necessário.
 *
 *  @param c: Caractere UTF-16 a ser exibido.
 */
void putwc(uint16_t c);
/** Escreve uma string UTF-16 terminada em NUL convertida para CP437.
 *
 *  @param str: String UTF-16 a ser exibida.
 */
void putws(const uint16_t* str);
/** Imprime texto formatado com um subconjunto de printf.
 *
 *  @param fmt: String de formato.
 *  @param ...: Argumentos variáveis compatíveis com o formato.
 *  @note Suporta %c, %C, %s, %S, %d, %i, %u, %x, %X, %p, %o e %%.
 *  @note Em %s, o modificador l ou ll seleciona leitura de string far.
 *  @note Especificadores não suportados são ignorados.
 */
void _cdecl printf(const char* fmt, ...);
