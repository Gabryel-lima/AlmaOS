/** @file gfx_core_test.c
 *  @brief Teste de host da copia vendorizada do nucleo do gfx.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  Nao substitui a suite do proprio gfx: aqui a pergunta e outra. O kernel
 *  embute uma copia (third_party/gfx) e a ponte grafica
 *  (src/kernel/gfx_bridge.c) depende de contratos especificos dela — o formato
 *  de cor 0xRRGGBBAA, o pitch em bytes, o sentido do teste de profundidade e a
 *  correcao perspectiva das cores. Um `git pull` no gfx upstream pode mudar
 *  qualquer um desses sem quebrar nada la, e aqui quebraria tudo.
 *
 *  Roda no host porque a mesma fonte compila nos dois lugares; conferir aqui e
 *  muito mais barato do que descobrir sob QEMU.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gfx_raster.h"

#define TEST_WIDTH  16
#define TEST_HEIGHT 16

static uint32_t pixels[TEST_WIDTH * TEST_HEIGHT];
static float depth[TEST_WIDTH * TEST_HEIGHT];

/** Reinicia o alvo de renderizacao entre casos de teste.
 *  @param clear_color Cor de fundo no formato 0xRRGGBBAA.
 *  @param out_fb Recebe o framebuffer configurado.
 */
static void reset_target(uint32_t clear_color, Framebuffer *out_fb) {
    int i;

    out_fb->pixels = pixels;
    out_fb->width = TEST_WIDTH;
    out_fb->height = TEST_HEIGHT;
    out_fb->pitch = TEST_WIDTH * (uint32_t)sizeof(uint32_t);

    gfx_fb_clear(out_fb, clear_color);
    for (i = 0; i < TEST_WIDTH * TEST_HEIGHT; ++i) {
        depth[i] = 1.0e30f;
    }
}

/** Compara um canal de 8 bits com o esperado, com folga de 1.
 *  @param rgba Cor no formato 0xRRGGBBAA.
 *  @param shift Deslocamento do canal (24 = R, 16 = G, 8 = B).
 *  @param expected Valor esperado.
 *  @return Diferente de zero se estiver dentro da tolerancia.
 */
static int channel_close(uint32_t rgba, unsigned shift, int expected) {
    int actual = (int)((rgba >> shift) & 0xFFu);
    int delta = actual - expected;

    return (delta < 0 ? -delta : delta) <= 1;
}

/** Confere o formato de cor que a ponte grafica assume.
 *  @return 0 em caso de sucesso.
 */
static int test_color_format(void) {
    Vec3 color = { 1.0f, 0.5f, 0.0f };
    uint32_t rgba = vec3_to_rgba(color);

    if (!channel_close(rgba, 24, 255) || !channel_close(rgba, 16, 127) ||
        !channel_close(rgba, 8, 0) || (rgba & 0xFFu) != 0xFFu) {
        fprintf(stderr, "vec3_to_rgba nao esta em 0xRRGGBBAA: 0x%08x\n", rgba);
        return 1;
    }

    return 0;
}

/** Confere que o teste de profundidade mantem o triangulo mais proximo.
 *  @return 0 em caso de sucesso.
 *  @note E o contrato de que o gfxdemo depende para o cubo nao mostrar as
 *  faces de tras por cima das da frente.
 */
static int test_depth_ordering(void) {
    Framebuffer fb;
    Vec3 far_color = { 1.0f, 0.0f, 0.0f };
    Vec3 near_color = { 0.0f, 0.0f, 1.0f };
    uint32_t center;

    reset_target(0x000000FFu, &fb);

    /* Triangulo distante primeiro, proximo depois: o proximo deve vencer. */
    gfx_rasterize_triangle(&fb, depth,
                           (Vec4){ 0.0f, 0.0f, 5.0f, 1.0f },
                           (Vec4){ 16.0f, 0.0f, 5.0f, 1.0f },
                           (Vec4){ 0.0f, 16.0f, 5.0f, 1.0f },
                           far_color, far_color, far_color);
    gfx_rasterize_triangle(&fb, depth,
                           (Vec4){ 0.0f, 0.0f, 2.0f, 1.0f },
                           (Vec4){ 16.0f, 0.0f, 2.0f, 1.0f },
                           (Vec4){ 0.0f, 16.0f, 2.0f, 1.0f },
                           near_color, near_color, near_color);

    center = pixels[2 * TEST_WIDTH + 2];
    if (!channel_close(center, 8, 255) || !channel_close(center, 24, 0)) {
        fprintf(stderr, "z-buffer nao manteve o triangulo mais proximo: 0x%08x\n", center);
        return 1;
    }

    /* E na ordem inversa o resultado tem de ser o mesmo. */
    reset_target(0x000000FFu, &fb);
    gfx_rasterize_triangle(&fb, depth,
                           (Vec4){ 0.0f, 0.0f, 2.0f, 1.0f },
                           (Vec4){ 16.0f, 0.0f, 2.0f, 1.0f },
                           (Vec4){ 0.0f, 16.0f, 2.0f, 1.0f },
                           near_color, near_color, near_color);
    gfx_rasterize_triangle(&fb, depth,
                           (Vec4){ 0.0f, 0.0f, 5.0f, 1.0f },
                           (Vec4){ 16.0f, 0.0f, 5.0f, 1.0f },
                           (Vec4){ 0.0f, 16.0f, 5.0f, 1.0f },
                           far_color, far_color, far_color);

    center = pixels[2 * TEST_WIDTH + 2];
    if (!channel_close(center, 8, 255) || !channel_close(center, 24, 0)) {
        fprintf(stderr, "z-buffer dependeu da ordem de desenho: 0x%08x\n", center);
        return 1;
    }

    return 0;
}

/** Confere que o campo `w` do vertice corrige a perspectiva das cores.
 *  @return 0 em caso de sucesso.
 *  @note Com w = 1 nos tres vertices a interpolacao tem de continuar afim;
 *  e o modo em que chamadores antigos operavam.
 */
static int test_perspective_correction(void) {
    Framebuffer fb;
    Vec3 red = { 1.0f, 0.0f, 0.0f };
    Vec3 blue = { 0.0f, 0.0f, 1.0f };
    const Vec4 corner_a = { 0.5f, 0.5f, 0.0f, 1.0f };
    const Vec4 corner_c = { 0.5f, 8.5f, 0.0f, 1.0f };
    Vec4 corner_b = { 8.5f, 0.5f, 0.0f, 1.0f };
    uint32_t sample;

    reset_target(0x000000FFu, &fb);
    gfx_rasterize_triangle(&fb, depth, corner_a, corner_b, corner_c, red, blue, red);

    sample = pixels[1 * TEST_WIDTH + 4];
    if (!channel_close(sample, 24, 127) || !channel_close(sample, 8, 127)) {
        fprintf(stderr, "interpolacao afim (w=1) mudou: 0x%08x\n", sample);
        return 1;
    }

    reset_target(0x000000FFu, &fb);
    corner_b.w = 9.0f;
    gfx_rasterize_triangle(&fb, depth, corner_a, corner_b, corner_c, red, blue, red);

    sample = pixels[1 * TEST_WIDTH + 4];
    if (!channel_close(sample, 24, 229) || !channel_close(sample, 8, 25)) {
        fprintf(stderr, "correcao perspectiva ausente ou errada: 0x%08x\n", sample);
        return 1;
    }

    return 0;
}

/** Confere que o pitch e contado em bytes, nao em pixels.
 *  @return 0 em caso de sucesso.
 *  @note gfx_bridge_init() monta o Framebuffer com `width * sizeof(uint32_t)`;
 *  se o gfx passasse a contar pixels, a ponte escreveria a cada quatro linhas.
 */
static int test_pitch_is_in_bytes(void) {
    uint32_t small[8];
    Framebuffer fb;

    memset(small, 0, sizeof(small));
    fb.pixels = small;
    fb.width = 2;
    fb.height = 2;
    fb.pitch = 2 * (uint32_t)sizeof(uint32_t);

    gfx_fb_set_pixel(&fb, 0, 1, 0xAABBCCDDu);

    if (small[2] != 0xAABBCCDDu) {
        fprintf(stderr, "pitch nao esta em bytes: linha 1 nao caiu no indice 2\n");
        return 1;
    }

    return 0;
}

int main(void) {
    struct {
        const char *name;
        int (*run)(void);
    } tests[] = {
        { "formato de cor RGBA",        test_color_format },
        { "ordenacao por z-buffer",     test_depth_ordering },
        { "correcao perspectiva",       test_perspective_correction },
        { "pitch em bytes",             test_pitch_is_in_bytes },
    };
    size_t i;
    int failures = 0;

    for (i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i) {
        int result = tests[i].run();

        printf("[%s] %s\n", result == 0 ? "PASS" : "FAIL", tests[i].name);
        failures += (result != 0);
    }

    if (failures != 0) {
        fprintf(stderr, "%d teste(s) da copia vendorizada do gfx falharam\n", failures);
        return 1;
    }

    printf("copia vendorizada do gfx: todos os testes passaram\n");
    return 0;
}
