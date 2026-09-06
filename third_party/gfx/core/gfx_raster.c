#include <stddef.h>

#include "gfx_raster.h"

/** Converte um valor de ponto flutuante para um inteiro, arredondando para baixo.
 *  @param value Valor de ponto flutuante a ser convertido.
 *  @return Valor inteiro arredondado para baixo.
 */
static int gfx_floor_to_int(float value) {
    int truncated = (int)value;

    if ((float)truncated > value) {
        truncated--;
    }

    return truncated;
}

/** Converte um valor de ponto flutuante para um inteiro, arredondando para cima.
 *  @param value Valor de ponto flutuante a ser convertido.
 *  @return Valor inteiro arredondado para cima.
 */
static int gfx_ceil_to_int(float value) {
    int truncated = (int)value;

    if ((float)truncated < value) {
        truncated++;
    }

    return truncated;
}

/** Rasteriza um triângulo em um framebuffer.
 *  @param fb Ponteiro para o framebuffer.
 *  @param zbuf Ponteiro para o buffer de profundidade.
 *  @param p0 Primeiro vértice do triângulo.
 *  @param p1 Segundo vértice do triângulo.
 *  @param p2 Terceiro vértice do triângulo.
 *  @param c0 Cor do primeiro vértice.
 *  @param c1 Cor do segundo vértice.
 *  @param c2 Cor do terceiro vértice.
 */
void gfx_rasterize_triangle(Framebuffer *fb, float *zbuf,
                            Vec4 p0, Vec4 p1, Vec4 p2,
                            Vec3 c0, Vec3 c1, Vec3 c2) {
    int minx;
    int maxx;
    int miny;
    int maxy;
    size_t width;
    size_t height;

    if (!fb || !fb->pixels || !zbuf || fb->width == 0 || fb->height == 0) {
        return;
    }

    width = (size_t)fb->width;
    height = (size_t)fb->height;

    minx = gfx_floor_to_int(gfx_fminf(gfx_fminf(p0.x, p1.x), p2.x));
    maxx = gfx_ceil_to_int(gfx_fmaxf(gfx_fmaxf(p0.x, p1.x), p2.x));
    miny = gfx_floor_to_int(gfx_fminf(gfx_fminf(p0.y, p1.y), p2.y));
    maxy = gfx_ceil_to_int(gfx_fmaxf(gfx_fmaxf(p0.y, p1.y), p2.y));

    if (maxx < 0 || maxy < 0) {
        return;
    }
    if (minx >= (int)width || miny >= (int)height) {
        return;
    }

    if (minx < 0) minx = 0;
    if (miny < 0) miny = 0;
    if (maxx >= (int)width) maxx = (int)width - 1;
    if (maxy >= (int)height) maxy = (int)height - 1;

    float area = gfx_edge2d(p0, p1, p2);   // área do triângulo inteiro

    if (area == 0.0f || area != area) {
        return;
    }

    float inv_area = 1.0f / area;

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            Vec4 p = {x + 0.5f, y + 0.5f, 0, 0};  // centro do pixel
            float w0 = gfx_edge2d(p1, p2, p) * inv_area;
            float w1 = gfx_edge2d(p2, p0, p) * inv_area;
            float w2 = gfx_edge2d(p0, p1, p) * inv_area;

            if (w0 < 0 || w1 < 0 || w2 < 0) continue;

            // Interpolação de profundidade para z-buffer
            float z = w0 * p0.z + w1 * p1.z + w2 * p2.z;
            size_t idx = (size_t)y * width + (size_t)x;
            if (z >= zbuf[idx]) continue;   // depth test
            zbuf[idx] = z;

            // Interpolação de cor
            Vec3 color = {
                w0*c0.x + w1*c1.x + w2*c2.x,
                w0*c0.y + w1*c1.y + w2*c2.y,
                w0*c0.z + w1*c1.z + w2*c2.z,
            };
            gfx_fb_set_pixel(fb, x, y, vec3_to_rgba(color));
        }
    }
}
