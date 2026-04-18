#pragma once

/** @brief Declarações de funções para operações de ponto flutuante no bootloader.
 *  @file stdfloat.h
 *  @author Gabryel-lima
 *  @date 2026-04-30
 */

#ifndef ALMAOS_FLOAT_TYPES_DEFINED
typedef float float8_t;   /**< Tipo de ponto flutuante de 8 bits (precisão reduzida, não padrão) */
typedef float float16_t;  /**< Tipo de ponto flutuante de 16 bits (precisão reduzida, não padrão) */
typedef float float32_t;  /**< Tipo de ponto flutuante de 32 bits (precisão simples) */
typedef double float64_t; /**< Tipo de ponto flutuante de 64 bits (precisão dupla) */
typedef long double float128_t; /**< Tipo de ponto flutuante de 128 bits (precisão estendida, não padrão) */
typedef long double long_double; /**< Alias para long double, usado para evitar problemas de token em macros genéricos */
/** Tipos de ponto flutuante definidos 
 * @tparam ALMAOS_FLOAT_TYPES_DEFINED Indica que os tipos de ponto flutuante foram definidos, evitando redefinições em outros arquivos de inclusão.
 * @note Devido às limitações do ambiente do bootloader, esses tipos de ponto flutuante podem não ser conformes aos padrões IEEE 754 e podem ter precisão reduzida.
*/
#define ALMAOS_FLOAT_TYPES_DEFINED
#endif
