#pragma once

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0 /* Define NULL como 0 para C++ */
    #else
        #define NULL ((void *)0) /* Define NULL como um ponteiro nulo para C */ 
    #endif
#endif

typedef unsigned int size_t; // Tipo para representar o tamanho de objetos em bytes
typedef int ptrdiff_t; // Tipo para representar a diferença entre ponteiros
