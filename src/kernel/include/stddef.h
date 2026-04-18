#ifndef _KERNEL_STDDEF_H
#define _KERNEL_STDDEF_H

/** @file stddef.h
 *  @brief Tipos e macros basicos para o kernel (freestanding, sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Esta é uma implementação mínima de stddef.h para uso no kernel,
 *  definindo tipos como size_t e ptrdiff_t, a macro NULL, e a macro offsetof. 
 *  Ela é compatível com C e C++.
 */

#ifndef NULL
    #ifdef __cplusplus
        #define NULL 0      /* Define NULL como 0 para C++, onde 0 é um ponteiro nulo válido */
    #else
        #define NULL ((void *)0)    /* Define NULL como um ponteiro nulo (void*) para C, e como 0 para C++ */
    #endif
#endif

typedef unsigned int size_t;    // Tipo para representar tamanhos de objetos (em bytes).
typedef int ptrdiff_t;          // Tipo para representar a diferença entre ponteiros (em número de elementos).

#define offsetof(type, member) __builtin_offsetof(type, member) /* Macro para calcular o deslocamento de um membro dentro de uma struct. */

#endif /* _KERNEL_STDDEF_H */
