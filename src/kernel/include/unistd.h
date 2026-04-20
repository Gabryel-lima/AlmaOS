#pragma once

/** @file unistd.h
 *  @brief Subconjunto POSIX mínimo para o kernel (freestanding, sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-19
 *  @note Cobre apenas o que faz sentido num kernel bare-metal:
 *  - Tipo ssize_t
 *  - Constantes SEEK_SET / SEEK_CUR / SEEK_END
 *  - Descritores simbólicos STDIN/STDOUT/STDERR (sem I/O de processo real)
 *  Para parsing de flags use getopt.h (getopt e getopt_long).
 */

#include "stddef.h"
#include "stdint.h"

/* ------------------------------------------------------------------ */
/* Tipos                                                               */
/* ------------------------------------------------------------------ */

/** Inteiro com sinal capaz de representar o resultado de operações de I/O. */
typedef int32_t ssize_t;

/* ------------------------------------------------------------------ */
/* Constantes SEEK                                                     */
/* ------------------------------------------------------------------ */

#define SEEK_SET 0  /**< Posicionar a partir do início do arquivo. */
#define SEEK_CUR 1  /**< Posicionar a partir da posição atual. */
#define SEEK_END 2  /**< Posicionar a partir do fim do arquivo. */

/* ------------------------------------------------------------------ */
/* Descritores padrão (simbólicos; não há VFS neste estágio)          */
/* ------------------------------------------------------------------ */

#define STDIN_FILENO  0  /**< Descriptor simbólico de entrada padrão. */
#define STDOUT_FILENO 1  /**< Descriptor simbólico de saída padrão. */
#define STDERR_FILENO 2  /**< Descriptor simbólico de erro padrão. */
