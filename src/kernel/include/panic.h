#pragma once

/** @file panic.h
 *  @brief Mecanismos de erro fatal, assert e log minimo para depuracao.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note A funcao panic e a macro kassert sao usadas para lidar com erros 
 *  fatais no kernel, enquanto a funcao klog fornece um mecanismo de 
 *  log simples para depuracao.
 */

#include "kernel.h"

/** @brief Para o sistema com mensagem de erro fatal. Nao retorna.
 *  @param msg Mensagem de erro a ser exibida.
 */
void panic(const char *msg) __attribute__((noreturn));

/** @brief Assert simples. Se a condicao falhar, executa panic.
 *  @param cond Condicao a ser verificada.
 *  @param msg Mensagem de erro exibida se cond for falsa.
 */
#define kassert(cond, msg) \
    do { if (!(cond)) panic("ASSERT FAILED: " msg); } while (0)

/** @brief Log minimo para depuracao (imprime no VGA com prefixo [LOG]).
 *  @param fmt String de formato (similar a printf).
 *  @param ... Argumentos adicionais para a string de formato.
 */
void klog(const char *fmt, ...);
