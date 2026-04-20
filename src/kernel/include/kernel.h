#pragma once

/** @file kernel.h
 *  @brief Tipos basicos compartilhados pelo kernel (freestanding, sem libc).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Este arquivo define os tipos basicos usados em todo o kernel, como uint32_t, 
 *  bool, size_t, etc. Ele é incluído por outros arquivos de cabeçalho do kernel.
 */

#include "stdint.h"
#include "stdbool.h"
#include "stddef.h"
#include "stdarg.h"

/** @brief Desabilita interrupcoes (CLI).
 *  @note Usar com cuidado; reabilitar com sti() o mais rapido possivel para nao perder eventos.
 */
static inline void cli(void) { __asm__ volatile("cli"); }

/** @brief Habilita interrupcoes (STI).
 *  @note Chamar apenas depois que o stack e os handlers estiverem configurados.
 */
static inline void sti(void) { __asm__ volatile("sti"); }

/** @brief Coloca a CPU em halt ate a proxima interrupcao.
 *  @note Requer que o handler correspondente esteja registrado na IDT; caso contrario,
 *  a CPU ficara travada indefinidamente.
 */
static inline void halt(void) { __asm__ volatile("hlt"); }
