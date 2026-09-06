#pragma once

/** @file kalloc.h
 *  @brief Alocador fisico bump para o kernel em modo protegido.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  O heap que o stage2 reserva (`boot_info->heap_base`, 128 KiB abaixo de
 *  1 MiB) serve para estruturas pequenas, mas nao para nada do tamanho de um
 *  framebuffer: um backbuffer de 640x480 em 32 bits sozinho ja pede 1,2 MiB.
 *  Este modulo cobre essa lacuna usando a memoria alta que o mapa E820
 *  descreve e que so e enderecavel porque o kernel roda em modo protegido.
 *
 *  E deliberadamente um alocador bump: reserva sempre para frente e nunca
 *  libera bloco individual. Isso basta enquanto as alocacoes sao poucas e
 *  vivem pelo resto do boot (framebuffers, z-buffer). Um alocador de verdade,
 *  com free, so faz sentido depois que houver paginacao e processos.
 */

#include "kernel.h"

/** Tamanho da pagina usada como unidade de alinhamento. */
#define KALLOC_PAGE_SIZE 4096u

/** @brief Escolhe a regiao de trabalho a partir do mapa E820 do boot_info.
 *  @return true se havia uma regiao utilizavel acima de 1 MiB.
 *  @note Pega a maior regiao marcada como utilizavel cujo inicio esteja acima
 *  de 1 MiB, evitando a memoria baixa (onde moram kernel, stage2, buffers da
 *  BIOS e o trampolim de modo real).
 */
bool kalloc_init(void);

/** @brief Reserva um bloco alinhado a pagina.
 *  @param size Tamanho pedido em bytes; e arredondado para cima em paginas.
 *  @return Ponteiro para o bloco, ou NULL se nao houver espaco.
 *  @note A memoria nao vem zerada.
 */
void *kalloc_pages(uint32_t size);

/** @brief Endereco fisico inicial da regiao gerenciada.
 *  @return Endereco base, ou 0 se `kalloc_init()` nao teve sucesso.
 */
uint32_t kalloc_base(void);

/** @brief Tamanho total da regiao gerenciada em bytes.
 *  @return Capacidade total, ou 0 se `kalloc_init()` nao teve sucesso.
 */
uint32_t kalloc_capacity(void);

/** @brief Quantidade ja reservada na regiao em bytes.
 *  @return Bytes consumidos desde o inicio da regiao.
 */
uint32_t kalloc_used(void);
