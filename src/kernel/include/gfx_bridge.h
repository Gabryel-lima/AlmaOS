#pragma once

/** @file gfx_bridge.h
 *  @brief Ponte entre o modo de video do AlmaOS e o nucleo grafico do gfx.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  O gfx (third_party/gfx) traz matematica, framebuffer generico e um
 *  rasterizador de triangulos por software que nao sabem nada sobre hardware:
 *  para ele, um framebuffer e um ponteiro para pixels RGBA de 32 bits mais
 *  largura, altura e pitch. Este modulo e o unico ponto do kernel que sabe as
 *  duas coisas — o que o gfx quer e o que a placa de video oferece.
 *
 *  A estrategia e sempre a mesma: o gfx desenha num backbuffer RGBA de 32 bits
 *  alocado por kalloc, e `gfx_bridge_present()` converte esse backbuffer para
 *  o formato do modo ativo (indices de 8 bits no VGA 13h, cor direta de
 *  15/16/24/32 bits no VBE). Apontar o gfx direto para o framebuffer do
 *  hardware seria mais rapido, mas so funcionaria no caso 32 bits e ainda
 *  assim com os canais trocados — a maioria dos modos VBE guarda BGRX, e o
 *  gfx produz 0xRRGGBBAA.
 */

#include "kernel.h"
#include "gfx_raster.h"

/** @brief Prepara a ponte para o modo de video atualmente ativo.
 *  @return true se o modo e grafico e o backbuffer pode ser alocado.
 *  @note Chame depois de `video_set_vga13()` ou `video_set_vbe()`. Trocar de
 *  modo depois disso exige chamar de novo: o backbuffer tem o tamanho do modo.
 *  Requer `kalloc_init()` e `fpu_init()` bem-sucedidos.
 */
bool gfx_bridge_init(void);

/** @brief Informa se a ponte esta pronta para desenhar.
 *  @return true se `gfx_bridge_init()` teve sucesso para o modo corrente.
 */
bool gfx_bridge_is_ready(void);

/** @brief Framebuffer do gfx que representa o backbuffer.
 *  @return Ponteiro para o `Framebuffer`, ou NULL se a ponte nao estiver pronta.
 *  @note E o que se passa para `gfx_fb_clear`, `gfx_rasterize_triangle` e
 *  companhia. Os pixels sao RGBA de 32 bits (0xRRGGBBAA), como o gfx espera.
 */
Framebuffer *gfx_bridge_framebuffer(void);

/** @brief Z-buffer paralelo ao backbuffer.
 *  @return Ponteiro para width*height floats, ou NULL se a ponte nao estiver pronta.
 *  @note `gfx_rasterize_triangle` exige um; use `gfx_bridge_clear_depth()`
 *  antes de cada frame.
 */
float *gfx_bridge_depth_buffer(void);

/** @brief Preenche o z-buffer com o valor "infinitamente longe".
 *  @note Sem isso o teste de profundidade compara contra lixo e o segundo
 *  frame desenha por cima de valores do primeiro.
 */
void gfx_bridge_clear_depth(void);

/** @brief Converte o backbuffer para o formato do hardware e o envia a tela.
 *  @note E o unico momento em que o kernel escreve no framebuffer real.
 */
void gfx_bridge_present(void);
