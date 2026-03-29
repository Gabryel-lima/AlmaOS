#pragma once

/** @brief Definições de tipos e macros comuns para o bootloader.
 *  @file stddef.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0 /* Define NULL como 0 para C++ */
    #else
        #define NULL ((void *)0) /* Define NULL como um ponteiro nulo para C */ 
    #endif
#endif

typedef unsigned int size_t; // Tipo para representar o tamanho de objetos em bytes
typedef int ptrdiff_t; // Tipo para representar a diferença entre ponteiros
