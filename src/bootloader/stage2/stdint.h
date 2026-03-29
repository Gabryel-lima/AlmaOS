#pragma once

/** @brief Tipos de inteiros padrão para C99 e definições de ponteiros para o bootloader.
 *  @file stdint.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

#ifdef __INTELLISENSE__
    #define far /**< Define 'far' como macro para ponteiros far no IntelliSense */
    #define _cdecl /**< Define '_cdecl' como macro para a convenção de chamada cdecl no IntelliSense */
    #define __far /**< Define '__far' como macro para ponteiros far no IntelliSense */
    #define __near /**< Define '__near' como macro para ponteiros near no IntelliSense */
    #define __huge /**< Define '__huge' como macro para ponteiros huge no IntelliSense */
    #define __interrupt /**< Define '__interrupt' como macro para funções de interrupção no IntelliSense */
#endif

#ifndef far
    #define far __far   /**< Define 'far' como macro para ponteiros far */
#endif

/// stdint.h - Tipos de inteiros padrão para C99
typedef signed char int8_t; // Inteiro de 8 bits com sinal
typedef unsigned char uint8_t; // Inteiro de 8 bits sem sinal
typedef signed short int16_t; // Inteiro de 16 bits com sinal
typedef unsigned short uint16_t; // Inteiro de 16 bits sem sinal
typedef signed long int int32_t; // Inteiro de 32 bits com sinal
typedef unsigned long int uint32_t; // Inteiro de 32 bits sem sinal
typedef signed long long int int64_t; // Inteiro de 64 bits com sinal
typedef unsigned long long int uint64_t; // Inteiro de 64 bits sem sinal
