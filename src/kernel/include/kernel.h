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

/** Desabilita interrupcoes (CLI). 
 *  @note Esta funcao deve ser usada com cuidado, pois desabilitar as interrupcoes 
 *  por muito tempo pode causar perda de eventos importantes. Use cli() apenas 
 *  quando necessario e reabilite as interrupcoes o mais rapido possivel com sti().
*/
static inline void cli(void) { __asm__ volatile("cli"); }

/** Habilita interrupcoes (STI). 
 *  @note Esta funcao deve ser chamada apenas depois que o kernel tiver configurado 
 *  um stack seguro para o processador usar durante as interrupcoes, 
 *  e somente depois de ter desabilitado as interrupcoes com cli().
*/
static inline void sti(void) { __asm__ volatile("sti"); }

/** Coloca a CPU em halt ate a proxima interrupcao. 
 *  @note Esta funcao deve ser usada apenas quando o kernel tiver configurado um handler 
 *  de interrupcao adequado para lidar com a interrupcao que pode ocorrer. 
 *  Caso contrario, a CPU pode ficar travada.
*/
static inline void halt(void) { __asm__ volatile("hlt"); }
