#pragma once

#ifdef __INTELLISENSE__
    #define far /**< Define 'far' as a macro for far pointers in IntelliSense */
    #define _cdecl /**< Define '_cdecl' as a macro for the cdecl calling convention in IntelliSense */
    #define __far /**< Define '__far' as a macro for far pointers in IntelliSense */
    #define __near /**< Define '__near' as a macro for near pointers in IntelliSense */
    #define __huge /**< Define '__huge' as a macro for huge pointers in IntelliSense */
    #define __interrupt /**< Define '__interrupt' as a macro for interrupt functions in IntelliSense */
#endif

#ifndef far
    #define far __far   /**< Define 'far' as a macro for far pointers */
#endif

/// stdint.h - Standard integer types for C99
typedef signed char int8_t; // 8-bit signed integer
typedef unsigned char uint8_t; // 8-bit unsigned integer
typedef signed short int16_t; // 16-bit signed integer
typedef unsigned short uint16_t; // 16-bit unsigned integer
typedef signed long int int32_t; // 32-bit signed integer
typedef unsigned long int uint32_t; // 32-bit unsigned integer
typedef signed long long int int64_t; // 64-bit signed integer
typedef unsigned long long int uint64_t; // 64-bit unsigned integer
