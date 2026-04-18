#ifndef _KERNEL_STDINT_H
#define _KERNEL_STDINT_H

/** @file stdint.h
 *  @brief Tipos inteiros de largura fixa para o kernel (freestanding, sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Esta é uma implementação mínima de stdint.h para uso no kernel,
 *  definindo tipos inteiros de largura fixa (ex: int32_t, uint64_t) e seus limites. 
 *  Ela é compatível com C e C++.
 */

typedef signed char int8_t;          // Tipo inteiro com sinal de 8 bits
typedef unsigned char uint8_t;       // Tipo inteiro sem sinal de 8 bits
typedef signed short int16_t;        // Tipo inteiro com sinal de 16 bits
typedef unsigned short uint16_t;     // Tipo inteiro sem sinal de 16 bits
typedef signed int int32_t;          // Tipo inteiro com sinal de 32 bits
typedef unsigned int uint32_t;       // Tipo inteiro sem sinal de 32 bits
typedef signed long long   int64_t;  // Tipo inteiro com sinal de 64 bits
typedef unsigned long long uint64_t; // Tipo inteiro sem sinal de 64 bits

typedef int8_t   int_least8_t;   // Tipo inteiro com sinal de pelo menos 8 bits
typedef uint8_t  uint_least8_t;  // Tipo inteiro sem sinal de pelo menos 8 bits
typedef int16_t  int_least16_t;  // Tipo inteiro com sinal de pelo menos 16 bits
typedef uint16_t uint_least16_t; // Tipo inteiro sem sinal de pelo menos 16 bits
typedef int32_t  int_least32_t;  // Tipo inteiro com sinal de pelo menos 32 bits
typedef uint32_t uint_least32_t; // Tipo inteiro sem sinal de pelo menos 32 bits
typedef int64_t  int_least64_t;  // Tipo inteiro com sinal de pelo menos 64 bits
typedef uint64_t uint_least64_t; // Tipo inteiro sem sinal de pelo menos 64 bits

typedef int8_t   int_fast8_t;    // Tipo inteiro com sinal mais rápido de pelo menos 8 bits
typedef uint8_t  uint_fast8_t;   // Tipo inteiro sem sinal mais rápido de pelo menos 8 bits
typedef int32_t  int_fast16_t;   // Tipo inteiro com sinal mais rápido de pelo menos 16 bits
typedef uint32_t uint_fast16_t;  // Tipo inteiro sem sinal mais rápido de pelo menos 16 bits
typedef int32_t  int_fast32_t;   // Tipo inteiro com sinal mais rápido de pelo menos 32 bits
typedef uint32_t uint_fast32_t;  // Tipo inteiro sem sinal mais rápido de pelo menos 32 bits
typedef int64_t  int_fast64_t;   // Tipo inteiro com sinal mais rápido de pelo menos 64 bits
typedef uint64_t uint_fast64_t;  // Tipo inteiro sem sinal mais rápido de pelo menos 64 bits

typedef int32_t  intptr_t;       // Tipo inteiro capaz de armazenar um ponteiro
typedef uint32_t uintptr_t;      // Tipo inteiro sem sinal capaz de armazenar um ponteiro

typedef int64_t  intmax_t;       // Tipo inteiro com sinal de maior largura disponível
typedef uint64_t uintmax_t;      // Tipo inteiro sem sinal de maior largura disponível

#define INT8_MIN    (-128)       // Valor mínimo para int8_t
#define INT8_MAX    127          // Valor máximo para int8_t
#define UINT8_MAX   255          // Valor máximo para uint8_t

#define INT16_MIN   (-32768)     // Valor mínimo para int16_t
#define INT16_MAX   32767        // Valor máximo para int16_t
#define UINT16_MAX  65535        // Valor máximo para uint16_t

#define INT32_MIN   (-2147483647 - 1) // Valor mínimo para int32_t
#define INT32_MAX   2147483647        // Valor máximo para int32_t
#define UINT32_MAX  4294967295U       // Valor máximo para uint32_t

#define INT64_MIN   (-9223372036854775807LL - 1) // Valor mínimo para int64_t
#define INT64_MAX   9223372036854775807LL        // Valor máximo para int64_t
#define UINT64_MAX  18446744073709551615ULL     // Valor máximo para uint64_t

#define SIZE_MAX    UINT32_MAX      // Valor máximo para size_t (assumindo 32 bits)
#define INTPTR_MIN  INT32_MIN       // Valor mínimo para intptr_t
#define INTPTR_MAX  INT32_MAX       // Valor máximo para intptr_t
#define UINTPTR_MAX UINT32_MAX      // Valor máximo para uintptr_t

#endif /* _KERNEL_STDINT_H */
