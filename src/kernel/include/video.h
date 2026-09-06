#pragma once

/** @file video.h
 *  @brief Selecao de modo de video via BIOS e descricao do framebuffer ativo.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  O kernel nao programa os registradores do VGA na mao: usa `int 10h` pelo
 *  trampolim de modo real (realmode.h). Sao dois caminhos graficos:
 *
 *  - VGA modo 13h: 320x200, 8 bits paletados, framebuffer fixo em 0xA0000.
 *    Existe em qualquer VGA, nao precisa de VESA, e e o caminho garantido.
 *  - VBE 2.0 com linear framebuffer: resolucoes maiores e cor direta, quando
 *    a BIOS de video oferecer. O framebuffer costuma ficar acima de 1 MiB,
 *    o que so e enderecavel porque o kernel roda em modo protegido flat.
 *
 *  O modo ativo e descrito por um `video_mode_info_t`, que e o que a ponte
 *  grafica (gfx_bridge.h) consome para montar um framebuffer do gfx.
 */

#include "kernel.h"

/** Familia do modo de video ativo.
 *  @param VIDEO_KIND_TEXT Modo texto 80x25; width/height contam celulas.
 *  @param VIDEO_KIND_VGA13 VGA 13h, 8 bits por pixel, paleta indexada.
 *  @param VIDEO_KIND_VBE_LFB Modo VBE com framebuffer linear e cor direta.
 */
typedef enum video_kind_t {
    VIDEO_KIND_TEXT = 0,
    VIDEO_KIND_VGA13,
    VIDEO_KIND_VBE_LFB,
} video_kind_t;

/** Descricao do framebuffer atualmente ativo.
 *  @param kind Familia do modo (texto, 13h, VBE).
 *  @param framebuffer Endereco fisico do inicio do framebuffer.
 *  @param width Largura em pixels (celulas de caractere no modo texto).
 *  @param height Altura em pixels (linhas no modo texto).
 *  @param pitch Bytes por linha; nao e necessariamente width * bpp/8.
 *  @param bpp Bits por pixel.
 *  @param red_pos,red_size Posicao e largura do canal vermelho (modos diretos).
 *  @param green_pos,green_size Idem para o verde.
 *  @param blue_pos,blue_size Idem para o azul.
 *  @param vbe_mode Numero do modo VBE ativo, ou 0 se o modo nao for VBE.
 */
typedef struct video_mode_info_t {
    video_kind_t kind;
    uint32_t framebuffer;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t  bpp;
    uint8_t  red_pos,   red_size;
    uint8_t  green_pos, green_size;
    uint8_t  blue_pos,  blue_size;
    uint16_t vbe_mode;
} video_mode_info_t;

/** @brief Inicializa o modulo de video.
 *  @note Registra o modo texto como estado corrente e consulta a BIOS VESA
 *  uma unica vez. Precisa do trampolim ja pronto (`realmode_init()`).
 */
void video_init(void);

/** @brief Descreve o modo de video ativo.
 *  @return Ponteiro para a descricao; nunca NULL.
 */
const video_mode_info_t *video_current(void);

/** @brief Informa se a BIOS de video anunciou suporte a VBE.
 *  @return true se a chamada VBE 4F00h respondeu com a assinatura "VESA".
 */
bool video_vbe_available(void);

/** @brief Versao do VBE anunciada pela BIOS.
 *  @return Versao no formato BCD do VESA (0x0200 = 2.0), ou 0 se nao houver VBE.
 */
uint16_t video_vbe_version(void);

/** @brief Volta ao modo texto 80x25.
 *  @return true se a BIOS aceitou a troca.
 *  @note E o modo em que `vga.c` funciona; qualquer saida de texto depois de
 *  um modo grafico precisa passar por aqui antes.
 */
bool video_set_text(void);

/** @brief Ativa o VGA modo 13h (320x200, 256 cores).
 *  @return true se a BIOS aceitou a troca.
 *  @note Instala tambem uma paleta RGB 3-3-2, para que um indice de 8 bits
 *  possa ser derivado de uma cor de 24 bits por truncamento — e o que permite
 *  a ponte grafica converter um backbuffer RGBA sem tabela de busca.
 */
bool video_set_vga13(void);

/** @brief Procura e ativa um modo VBE com framebuffer linear.
 *  @param width Largura desejada em pixels.
 *  @param height Altura desejada em pixels.
 *  @param bpp Bits por pixel desejados (15, 16, 24 ou 32).
 *  @return true se um modo compativel foi encontrado e ativado.
 *  @note Escolhe o modo de resolucao exata com bpp igual ao pedido; se nao
 *  houver, aceita a mesma resolucao com outro bpp direto.
 */
bool video_set_vbe(uint32_t width, uint32_t height, uint8_t bpp);
