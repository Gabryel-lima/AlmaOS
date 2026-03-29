#ifndef _STDBOOL_H
#define _STDBOOL_H

/** @brief Definição do tipo booleano e valores para C99.
 *  @file stdbool.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

#ifndef __cplusplus

#define bool unsigned char  /* Define 'bool' as an unsigned char for C */
#define true 1  /* Define 'true' as 1 */
#define false 0 /* Define 'false' as 0 */

#else /* __cplusplus */

#define _Bool bool  /* Define '_Bool' as 'bool' for C++ */
#define bool bool   /* Define 'bool' as a macro for C++ */
#define false false /* Define 'false' as a macro for C++ */
#define true true   /* Define 'true' as a macro for C++ */

#endif /* __cplusplus */

#define __bool_true_false_are_defined 1 /* Indicate that the boolean type and values are defined */

#endif /* _STDBOOL_H */
