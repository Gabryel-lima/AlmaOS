#ifndef _KERNEL_STDARG_H
#define _KERNEL_STDARG_H

/** @file stdarg.h
 *  @brief Argumentos variadicos para o kernel (usa builtins do GCC).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Esta é uma implementação mínima de stdarg.h 
 *  para uso no kernel, utilizando os builtins do GCC.
 */

typedef __builtin_va_list va_list;  /* Tipo para lista de argumentos variadicos */

#define va_start(ap, last) __builtin_va_start(ap, last)     /* Inicializa ap para apontar para o primeiro argumento variadico */
#define va_end(ap)         __builtin_va_end(ap)             /* Limpa ap após o uso */
#define va_arg(ap, type)   __builtin_va_arg(ap, type)       /* Retorna o próximo argumento variadico do tipo especificado */
#define va_copy(dest, src) __builtin_va_copy(dest, src)     /* Copia o estado de ap de src para dest */

#endif /* _KERNEL_STDARG_H */
