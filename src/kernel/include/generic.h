#ifndef GENERIC_H
#define GENERIC_H

/** @brief Operações genéricas para tipos primitivos. 
 *  @author Gabriel-lima 
 *  @date 2024-04-30
 */

/** @file generic.h
 *  @brief Operações genéricas para tipos primitivos.
 *
 * AVISO: Este header define macros com nomes comuns (ex.: `min`). Não o
 * inclua em headers públicos; inclua-o apenas em arquivos de
 * implementação (.c) ou via generic_inst.h. Se for necessário usá-lo
 * em headers, proteja os protótipos contra expansão de macro.
 *
 * Este arquivo define macros para gerar funções genéricas como min, max,
 * swap, etc., para os tipos primitivos usados no kernel. Ele é incluído
 * por generic_inst.h, que instancia essas funções para os tipos desejados.
 *
 * Para adicionar uma nova operação, defina uma macro aqui e chame-a em
 * DEFINE_ALL(T) para os tipos que precisam dela.
 */

/* ── Comparação ─────────────────────────────────────── */

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_MIN(T)                            \
    static inline T min_##T(T a, T b) {          \
        return a < b ? a : b;                    \
    }

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_MAX(T)                            \
    static inline T max_##T(T a, T b) {          \
        return a > b ? a : b;                    \
    }

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_CLAMP(T)                          \
    static inline T clamp_##T(T v, T lo, T hi) { \
        return v < lo ? lo : (v > hi ? hi : v);  \
    }

/* ── Swap ────────────────────────────────────────────── */

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_SWAP(T)                           \
    static inline void swap_##T(T* a, T* b) {   \
        T tmp = *a;                              \
        *a    = *b;                              \
        *b    = tmp;                             \
    }

/* ── Array ───────────────────────────────────────────── */

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_ARRAY_FILL(T)                                \
    static inline void array_fill_##T(T* arr,               \
                                      usize n, T val) {     \
        for (usize i = 0; i < n; i++) arr[i] = val;        \
    }

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_ARRAY_SUM(T)                                  \
    static inline T array_sum_##T(const T* arr, usize n) {  \
        T acc = 0;                                           \
        for (usize i = 0; i < n; i++) acc += arr[i];        \
        return acc;                                          \
    }

/** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @return: A função genérica instanciada para o tipo T. */
#define DEFINE_ARRAY_FIND(T)                                      \
    static inline isize array_find_##T(const T* arr,              \
                                       usize n, T val) {          \
        for (usize i = 0; i < n; i++)                             \
            if (arr[i] == val) return (isize)i;                   \
        return -1;                                                 \
    }

/* ── Atalhos de chamada ──────────────────────────────── */

/** Macros de função genérica usando C11 _Generic. 
   @param a, b, v, lo, hi: Os argumentos para as funções genéricas. 
   @return: O resultado da função genérica instanciada para o tipo dos argumentos. */
#define min(T, a, b)          min_##T(a, b)
/** Mesmo tratamento para `max`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param a, b: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define max(T, a, b)          max_##T(a, b)
/** Mesmo tratamento para `clamp`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param v, lo, hi: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define clamp(T, v, lo, hi)   clamp_##T(v, lo, hi)
/** Mesmo tratamento para `swap`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param a, b: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define swap(T, a, b)         swap_##T(&(a), &(b))
/** Mesmo tratamento para `array_fill`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param arr, n, val: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define array_fill(T, ...)    array_fill_##T(__VA_ARGS__)
/** Mesmo tratamento para `array_sum`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param arr, n: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define array_sum(T, ...)     array_sum_##T(__VA_ARGS__)
/** Mesmo tratamento para `array_find`. 
   @param T: O tipo para o qual a função genérica será instanciada. 
   @param arr, n, val: Os argumentos para a função genérica. 
   @return: O resultado da função genérica instanciada para o tipo T. */
#define array_find(T, ...)    array_find_##T(__VA_ARGS__)

/* ── Pacote: instancia tudo para um tipo de uma vez ─── */

/** Chama todas as macros de definição para um tipo T. 
   @param T: O tipo para o qual todas as funções genéricas serão instanciadas. 
   @return: Todas as funções genéricas instanciadas para o tipo T. */
#define DEFINE_ALL(T)       \
    DEFINE_MIN(T)           \
    DEFINE_MAX(T)           \
    DEFINE_CLAMP(T)         \
    DEFINE_SWAP(T)          \
    DEFINE_ARRAY_FILL(T)    \
    DEFINE_ARRAY_SUM(T)     \
    DEFINE_ARRAY_FIND(T)

#endif /* GENERIC_H */
