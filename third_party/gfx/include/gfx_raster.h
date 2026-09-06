#pragma once

#include <stdint.h>

#include "gfx_math.h"

/**
 * @brief Framebuffer portátil e rasterizador de triângulos por software.
 * @author Gabryel-lima
 * @date 2026-04-08
 * @file include/gfx_raster.h
 * @note Este header não depende de nada além de <stdint.h> e gfx_math.h — é a
 *       parte do gfx pensada para funcionar sem sistema operacional (ex.: um
 *       framebuffer linear entregue por um bootloader). Acesso ao dispositivo
 *       de framebuffer de um SO hospedeiro (como /dev/fb0 no Linux) fica em
 *       headers de plataforma separados, ex. gfx_platform_linux_fb.h.
 */

/** Framebuffer RGBA genérico: um ponteiro de pixels já mapeado/alocado.
 * @param pixels Ponteiro para os pixels (formato 0xRRGGBBAA)
 * @param width Largura do framebuffer em pixels
 * @param height Altura do framebuffer em pixels
 * @param pitch Número de bytes por linha (usado para calcular offsets)
*/
typedef struct Framebuffer {
    uint32_t *pixels;   /**< Ponteiro para os pixels */
    uint32_t  width, height, pitch; /**< Largura, altura e pitch (bytes por linha) */
} Framebuffer;

/**
 * Escreve um pixel em coordenadas inteiras. x,y devem estar dentro dos limites.
 * @param fb Ponteiro para Framebuffer
 * @param x Coordenada X
 * @param y Coordenada Y
 * @param rgba Cor no formato 0xRRGGBBAA (uint32_t)
 */
void gfx_fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba);

/** Preenche o framebuffer com uma cor RGBA.
 * @param fb Ponteiro para Framebuffer
 * @param rgba Cor de limpeza no formato 0xRRGGBBAA (uint32_t)
 * @return void
*/
void gfx_fb_clear(Framebuffer *fb, uint32_t rgba);

/**
 * Rasteriza um único triângulo no framebuffer e no z-buffer fornecidos.
 *
 * Contrato dos vértices:
 * - `x` e `y` estão em pixels de tela (o centro do pixel (i,j) é i+0.5, j+0.5);
 * - `z` é a profundidade já comparável, interpolada linearmente em espaço de
 *   tela — o que é correto para um z de NDC, isto é, já dividido por w;
 * - `w` é o w de clip do vértice, usado para corrigir a perspectiva na
 *   interpolação das cores. Passar 1.0 em todos os três dá interpolação afim,
 *   que é o comportamento correto para projeção ortográfica e o que os
 *   chamadores mais antigos assumiam.
 *
 * Interpolar cor linearmente em espaço de tela sob projeção perspectiva é
 * errado: é o que faz a cor de uma superfície inclinada "escorregar" na
 * direção do ponto de fuga. Por isso as cores são interpoladas com pesos
 * divididos por w e renormalizados.
 *
 * Pixels exatamente sobre uma aresta são resolvidos pela regra top-left, de
 * modo que dois triângulos que compartilham uma aresta não desenhem o mesmo
 * pixel duas vezes nem deixem buraco entre eles.
 *
 * Não há clipping: um vértice com w <= 0 está atrás do plano da câmera e faz
 * a interpolação cair para o modo afim, em vez de produzir cores sem sentido.
 * Cortar o triângulo contra o frustum é responsabilidade de quem chama.
 *
 * @param fb Ponteiro para o framebuffer
 * @param zbuf Ponteiro para o z-buffer
 * @param p0, p1, p2 Vértices do triângulo (coordenadas homogêneas)
 * @param c0, c1, c2 Cores dos vértices (RGB)
*/
void gfx_rasterize_triangle(Framebuffer *fb, float *zbuf,
                            Vec4 p0, Vec4 p1, Vec4 p2,
                            Vec3 c0, Vec3 c1, Vec3 c2);
