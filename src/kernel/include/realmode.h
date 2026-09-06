#pragma once

/** @file realmode.h
 *  @brief Trampolim que permite ao kernel 32-bit chamar rotinas da BIOS.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  O kernel entra em modo protegido no boot e, a partir dai, perde acesso a
 *  BIOS: as rotinas dela sao codigo de 16 bits que espera modo real. Este
 *  modulo faz a ida e volta (PM32 -> RM16 -> PM32) para chamadas pontuais —
 *  na pratica, `int 10h` para trocar o modo de video (veja video.h).
 *
 *  Nao e um caminho barato nem reentrante: cada chamada desliga interrupcoes,
 *  mascara o PIC, troca GDT/IDT duas vezes e volta. Use em inicializacao e em
 *  troca de modo, nunca dentro de um laco de renderizacao.
 *
 *  A mecanica esta em realmode.asm; aqui ficam so o contrato e o layout do
 *  bloco de estado compartilhado entre C e assembly.
 */

#include "kernel.h"

/** Regiao de memoria baixa usada pelo trampolim.
 *
 *  Precisa ficar abaixo de 1 MiB e numa base alinhada a 16 bytes, porque o
 *  modo real enderecha por segment:offset. A faixa 0x30000-0x33FFF esta livre
 *  no mapa de memdefs.h (fica logo depois do stage2, que ja terminou quando o
 *  kernel assume).
 */
#define REALMODE_STUB_ADDR      0x00030000  /* codigo do trampolim (segmento 0x3000) */
#define REALMODE_STUB_SEG       0x3000      /* mesmo endereco como segmento de modo real */
#define REALMODE_DATA_ADDR      0x00030400  /* bloco realmode_state_t compartilhado */
#define REALMODE_STACK_TOP_ADDR 0x00032000  /* topo da stack de modo real (0x3000:0x2000) */

/** Buffer de transferencia em memoria baixa para estruturas da BIOS.
 *
 *  A BIOS so consegue escrever abaixo de 1 MiB e enderecada por segmento, e
 *  os buffers do kernel podem estar em qualquer lugar. Quem chama a BIOS
 *  aponta ES:DI para ca e depois copia o resultado para onde quiser.
 */
#define REALMODE_BUFFER_ADDR    0x00033000
#define REALMODE_BUFFER_SEG     0x3300
#define REALMODE_BUFFER_SIZE    0x1000

/** @brief Registradores de entrada e saida de uma chamada a BIOS.
 *
 *  Os campos valem nos dois sentidos: sao carregados antes da chamada e
 *  sobrescritos com o que a BIOS devolveu.
 *
 *  @param eax,ebx,ecx,edx,esi,edi,ebp Registradores de proposito geral.
 *  @param ds,es Segmentos de dados vistos pela rotina da BIOS.
 *  @param eflags EFLAGS de retorno; o bit 0 (carry) e o indicador de erro
 *         usado pela maioria das rotinas de BIOS.
 */
typedef struct realmode_regs_t {
    uint32_t eax, ebx, ecx, edx, esi, edi, ebp;
    uint16_t ds, es;
    uint32_t eflags;
} realmode_regs_t;

/** Bit de carry em EFLAGS: a convencao de erro das rotinas da BIOS. */
#define REALMODE_FLAG_CARRY 0x0001u

/** @brief Prepara o trampolim: copia o stub para a memoria baixa.
 *  @return true se o stub coube na regiao reservada, false caso contrario.
 *  @note Precisa ser chamada uma vez antes de qualquer `realmode_int()`,
 *        depois de `idt_init()` e `pic_remap()`.
 */
bool realmode_init(void);

/** @brief Informa se o trampolim foi preparado com sucesso.
 *  @return true se `realmode_init()` teve sucesso.
 */
bool realmode_is_ready(void);

/** @brief Executa uma interrupcao da BIOS em modo real.
 *  @param vector Numero do vetor (ex.: 0x10 para video).
 *  @param regs Registradores de entrada; recebe os de saida. Nao pode ser NULL.
 *  @return true se a chamada foi executada, false se o trampolim nao estiver
 *          pronto ou o vetor nao tiver handler na IVT. O resultado da BIOS em
 *          si esta em `regs` (tipicamente no carry de `regs->eflags`).
 *  @note Desliga interrupcoes e mascara todas as IRQs durante a chamada; as
 *        mascaras do PIC sao restauradas ao voltar. IRQs pendentes so serao
 *        atendidas depois do retorno.
 */
bool realmode_int(uint8_t vector, realmode_regs_t *regs);
