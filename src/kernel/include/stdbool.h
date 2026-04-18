#ifndef _KERNEL_STDBOOL_H
#define _KERNEL_STDBOOL_H

/** @file stdbool.h
 *  @brief Tipo booleano para o kernel (freestanding, sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Esta é uma implementação mínima de stdbool.h para uso no kernel, 
 *  definindo o tipo bool e os valores true/false. Ela é compatível com C e C++
 */

#ifndef __cplusplus

#define bool  _Bool     /* Define bool como o tipo _Bool do C99 */
#define true  1         /* Define true como 1 */
#define false 0         /* Define false como 0 */

#else

#define _Bool bool      /* Em C++, bool é um tipo nativo, então _Bool é definido como bool */
#define bool  bool      /* Define bool como o tipo bool nativo do C++ */
#define false false     /* Define false como o valor false nativo do C++ */
#define true  true      /* Define true como o valor true nativo do C++ */

#endif

#define __bool_true_false_are_defined 1     /* Macro para indicar que bool, true e false estão definidos */

#endif /* _KERNEL_STDBOOL_H */
