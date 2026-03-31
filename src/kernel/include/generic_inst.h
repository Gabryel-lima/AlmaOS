#ifndef GENERIC_INST_H
#define GENERIC_INST_H

/** @brief Instancia operações genéricas e define macros de função.
 *  @author Gabriel-lima
 *  @date 2024-04-30
 *  @file generic_inst.h
 *  AVISO: Este header instancia operações genéricas e define macros de
 * função (ex.: `min`, `max`, `swap`, `array_sum`, `array_find`) via
 * C11 _Generic. Não inclua este arquivo em headers públicos — inclua-o
 * somente em arquivos de implementação (.c) onde o uso desses macros
 * é desejado. Se for necessário incluí-lo em um header, proteja os
 * protótipos contra expansão de macro (por exemplo, usando `#undef`).
 */

#include "generic.h"

#ifndef ALMAOS_FLOAT_TYPES_DEFINED
typedef float float32_t;
typedef double float64_t;
typedef long double long_double;
/* Tipos de dados genéricos: uptr/isize/usize são tipos de ponteiro/índice genéricos. 
 * Eles são definidos como aliases para os tipos de tamanho apropriados (ex: uintptr_t, ptrdiff_t, size_t).
 * Isso permite que as operações genéricas sejam aplicadas a ponteiros e índices sem precisar de macros separados.
 */
#define ALMAOS_FLOAT_TYPES_DEFINED
#endif

/** X-macro: lista canônica de tipos do kernel.
 *  Para adicionar um tipo novo, basta inserir X(tipo) aqui.
 *  @param X: O macro a ser aplicado a cada tipo da lista.
 *  @return: A expansão do macro X para cada tipo da lista.
 */
#define KERNEL_TYPES(X) \
    X(uint8_t)        \
    X(uint16_t)       \
    X(uint32_t)       \
    X(uint64_t)       \
    X(int8_t)         \
    X(int16_t)        \
    X(int32_t)        \
    X(int64_t)        \
    X(uptr)           \
    X(isize)          \
    X(usize)          \
    X(float32_t)      \
    X(float64_t)      \
    X(long_double)    \

/* Expande DEFINE_ALL para cada tipo da lista */
KERNEL_TYPES(DEFINE_ALL)

/*
 * Se algum tipo precisar apenas de operações específicas,
 * instancie manualmente aqui:
 *
 * DEFINE_SWAP(uptr)
 * DEFINE_MIN(uptr)
 */

/*
 * Wrappers genéricos (C11 _Generic): permitem usar nomes iguais
 * (ex: min(a,b), max(a,b), swap(a,b), array_sum(arr,n), array_find(arr,n,val))
 * sem precisar passar o tipo como primeiro argumento.
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)

    /* remover as versões antigas que exigiam o tipo como primeiro argumento */
    #undef min
    #undef max
    #undef clamp
    #undef swap
    #undef array_sum
    #undef array_find

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param a: O primeiro argumento da operação (ex: min(a,b) -> a).
     * @param b: O segundo argumento da operação (ex: min(a,b) -> b).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'a'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes 
     * estejam definidas (ex: min_uint32_t).
    */
    #define min(a, b) (_Generic((a), \
        uint8_t: min_uint8_t, \
        uint16_t: min_uint16_t, \
        uint32_t: min_uint32_t, \
        uint64_t: min_uint64_t, \
        int8_t: min_int8_t, \
        int16_t: min_int16_t, \
        int32_t: min_int32_t, \
        int64_t: min_int64_t, \
        /* uptr/isize/usize are aliases; omit to avoid _Generic duplicates */ \
        float32_t: min_float32_t, \
        float64_t: min_float64_t, \
        long_double: min_long_double, \
        default: min_uint32_t))(a,b)

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param a: O primeiro argumento da operação (ex: max(a,b) -> a).
     * @param b: O segundo argumento da operação (ex: max(a,b) -> b).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'a'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes estejam definidas (ex: max_uint32_t).
    */
    #define max(a, b) (_Generic((a), \
        uint8_t: max_uint8_t, \
        uint16_t: max_uint16_t, \
        uint32_t: max_uint32_t, \
        uint64_t: max_uint64_t, \
        int8_t: max_int8_t, \
        int16_t: max_int16_t, \
        int32_t: max_int32_t, \
        int64_t: max_int64_t, \
        /* uptr/isize/usize are aliases; omit to avoid _Generic duplicates */ \
        float32_t: max_float32_t, \
        float64_t: max_float64_t, \
        long_double: max_long_double, \
        default: max_uint32_t))(a,b)

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param v: O valor a ser limitado (ex: clamp(v,lo,hi) -> v).
     * @param lo: O limite inferior (ex: clamp(v,lo,hi) -> lo).
     * @param hi: O limite superior (ex: clamp(v,lo,hi) -> hi).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'v'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes estejam definidas (ex: clamp_uint32_t).
    */
    #define clamp(v, lo, hi) (_Generic((v), \
        uint8_t: clamp_uint8_t, \
        uint16_t: clamp_uint16_t, \
        uint32_t: clamp_uint32_t, \
        uint64_t: clamp_uint64_t, \
        int8_t: clamp_int8_t, \
        int16_t: clamp_int16_t, \
        int32_t: clamp_int32_t, \
        int64_t: clamp_int64_t, \
        /* uptr/isize/usize are aliases; omit to avoid _Generic duplicates */ \
        float32_t: clamp_float32_t, \
        float64_t: clamp_float64_t, \
        long_double: clamp_long_double, \
        default: clamp_uint32_t))(v,lo,hi)

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param a: O primeiro argumento da operação (ex: swap(a,b) -> a).
     * @param b: O segundo argumento da operação (ex: swap(a,b) -> b).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'a'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes estejam definidas (ex: swap_uint32_t).
    */
    #define swap(a, b) (_Generic((a), \
        uint8_t: swap_uint8_t, \
        uint16_t: swap_uint16_t, \
        uint32_t: swap_uint32_t, \
        uint64_t: swap_uint64_t, \
        int8_t: swap_int8_t, \
        int16_t: swap_int16_t, \
        int32_t: swap_int32_t, \
        int64_t: swap_int64_t, \
        /* uptr/isize/usize are aliases; omit to avoid _Generic duplicates */ \
        float32_t: swap_float32_t, \
        float64_t: swap_float64_t, \
        long_double: swap_long_double, \
        default: swap_uint32_t))(&(a), &(b))

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param arr: O array a ser processado (ex: array_sum(arr,n) -> arr).
     * @param n: O número de elementos no array (ex: array_sum(arr,n) -> n).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'arr'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes estejam definidas (ex: array_sum_uint32_t).
    */
    #define array_sum(arr, n) (_Generic((arr), \
        uint8_t*: array_sum_uint8_t, \
        uint16_t*: array_sum_uint16_t, \
        uint32_t*: array_sum_uint32_t, \
        uint64_t*: array_sum_uint64_t, \
        int8_t*: array_sum_int8_t, \
        int16_t*: array_sum_int16_t, \
        int32_t*: array_sum_int32_t, \
        int64_t*: array_sum_int64_t, \
        /* uptr/isize/usize pointer types omitted to avoid duplicates */ \
        float32_t*: array_sum_float32_t, \
        float64_t*: array_sum_float64_t, \
        long_double*: array_sum_long_double, \
        default: array_sum_uint32_t))(arr,n)

    /** Implementação explícita do _Generic para cada tipo listado em KERNEL_TYPES. 
     * @param arr: O array a ser processado (ex: array_find(arr,n,val) -> arr).
     * @param n: O número de elementos no array (ex: array_find(arr,n,val) -> n).
     * @param val: O valor a ser encontrado no array (ex: array_find(arr,n,val) -> val).
     * @protocol O _Generic seleciona a função correta com base no tipo de 'arr'.
     * AVISO: Se um tipo tiver operações específicas, certifique-se de que 
     * ele esteja listado em KERNEL_TYPES 
     * e que as funções correspondentes estejam definidas (ex: array_find_uint32_t).
    */
    #define array_find(arr, n, val) (_Generic((arr), \
        uint8_t*: array_find_uint8_t, \
        uint16_t*: array_find_uint16_t, \
        uint32_t*: array_find_uint32_t, \
        uint64_t*: array_find_uint64_t, \
        int8_t*: array_find_int8_t, \
        int16_t*: array_find_int16_t, \
        int32_t*: array_find_int32_t, \
        int64_t*: array_find_int64_t, \
        /* uptr/isize/usize pointer types omitted to avoid duplicates */ \
        float32_t*: array_find_float32_t, \
        float64_t*: array_find_float64_t, \
        long_double*: array_find_long_double, \
        default: array_find_uint32_t))(arr,n,val)

#endif /* __STDC_VERSION__ */

#endif /* GENERIC_INST_H */
