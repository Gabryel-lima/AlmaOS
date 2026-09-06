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

/** Decide se uma aresta dirigida é "topo" ou "esquerda" pela regra top-left.
 *
 *  A regra existe para resolver quem desenha os pixels que caem exatamente
 *  sobre uma aresta compartilhada por dois triângulos. Para uma aresta a->b
 *  vista pelo triângulo vizinho como b->a, a função de aresta troca de sinal;
 *  então o único caso ambíguo é o zero exato, e aí `is_top_left(a,b)` e
 *  `is_top_left(b,a)` dão resultados opostos. Isso atribui o pixel a
 *  exatamente um dos dois: nem buraco, nem cobertura dupla.
 *
 *  As comparações valem para triângulos já normalizados com área positiva,
 *  o que em coordenadas de tela (y para baixo) corresponde a winding horário.
 *
 *  @param a Primeiro vértice da aresta dirigida.
 *  @param b Segundo vértice da aresta dirigida.
 *  @return Diferente de zero se a aresta for de topo ou de esquerda.
 */
static int gfx_edge_is_top_left(Vec4 a, Vec4 b) {
    if (a.y == b.y) {
        return b.x > a.x;   /* aresta horizontal com o interior abaixo */
    }
    return b.y < a.y;       /* aresta descendo pela lateral esquerda */
}

/** Troca dois vértices e as cores correspondentes.
 *  @param p Primeiro vértice.
 *  @param c Cor do primeiro vértice.
 *  @param q Segundo vértice.
 *  @param d Cor do segundo vértice.
 */
static void gfx_swap_vertex(Vec4 *p, Vec3 *c, Vec4 *q, Vec3 *d) {
    Vec4 temp_position = *p;
    Vec3 temp_color = *c;

    *p = *q;
    *c = *d;
    *q = temp_position;
    *d = temp_color;
}

/** Rasteriza um triângulo em um framebuffer.
 *
 *  A profundidade é interpolada linearmente em espaço de tela, que é o
 *  correto para um z já dividido por w (z de NDC). As cores, ao contrário,
 *  são interpoladas com correção perspectiva usando `w`: interpolar cor
 *  linearmente em espaço de tela é o erro clássico que faz uma superfície
 *  inclinada mostrar a cor "escorregando" na direção do ponto de fuga.
 *
 *  Com todos os `w` iguais a 1 — o caso afim, e o que os chamadores antigos
 *  passavam — a correção se reduz exatamente à interpolação anterior.
 *
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
    float area;
    float inv_area;
    float inv_w0;
    float inv_w1;
    float inv_w2;
    int perspective;
    int top_left_12;
    int top_left_20;
    int top_left_01;

    if (!fb || !fb->pixels || !zbuf || fb->width == 0 || fb->height == 0) {
        return;
    }

    width = (size_t)fb->width;
    height = (size_t)fb->height;

    area = gfx_edge2d(p0, p1, p2);   // área do triângulo inteiro

    if (area == 0.0f || area != area) {
        return;
    }

    /* Normaliza o winding para área positiva. Sem isso a regra top-left
     * precisaria de dois conjuntos de comparações, um por sentido. */
    if (area < 0.0f) {
        gfx_swap_vertex(&p1, &c1, &p2, &c2);
        area = -area;
    }

    inv_area = 1.0f / area;

    /* Correção perspectiva só é possível com w positivo nos três vértices.
     * Um w <= 0 significa vértice atrás do plano da câmera, o que é trabalho
     * de clipping — que não acontece aqui. Nesse caso a interpolação cai para
     * o modo afim em vez de produzir cores sem sentido. */
    perspective = (p0.w > 0.0f && p1.w > 0.0f && p2.w > 0.0f);
    inv_w0 = perspective ? (1.0f / p0.w) : 1.0f;
    inv_w1 = perspective ? (1.0f / p1.w) : 1.0f;
    inv_w2 = perspective ? (1.0f / p2.w) : 1.0f;

    top_left_12 = gfx_edge_is_top_left(p1, p2);
    top_left_20 = gfx_edge_is_top_left(p2, p0);
    top_left_01 = gfx_edge_is_top_left(p0, p1);

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

    for (int y = miny; y <= maxy; y++) {
        for (int x = minx; x <= maxx; x++) {
            Vec4 p = {x + 0.5f, y + 0.5f, 0, 0};  // centro do pixel
            float e0 = gfx_edge2d(p1, p2, p);
            float e1 = gfx_edge2d(p2, p0, p);
            float e2 = gfx_edge2d(p0, p1, p);
            float b0;
            float b1;
            float b2;
            float weight0;
            float weight1;
            float weight2;
            float weight_sum;
            float z;
            size_t idx;
            Vec3 color;

            if (e0 < 0.0f || e1 < 0.0f || e2 < 0.0f) continue;

            /* Zero exato é o único caso em que dois triângulos vizinhos
             * cobririam o mesmo pixel; a regra top-left decide de quem é. */
            if (e0 == 0.0f && !top_left_12) continue;
            if (e1 == 0.0f && !top_left_20) continue;
            if (e2 == 0.0f && !top_left_01) continue;

            b0 = e0 * inv_area;
            b1 = e1 * inv_area;
            b2 = e2 * inv_area;

            // Profundidade: linear em espaço de tela, sem correção.
            z = b0 * p0.z + b1 * p1.z + b2 * p2.z;
            idx = (size_t)y * width + (size_t)x;
            if (z >= zbuf[idx]) continue;   // depth test
            zbuf[idx] = z;

            // Cor: pesos divididos por w e renormalizados.
            weight0 = b0 * inv_w0;
            weight1 = b1 * inv_w1;
            weight2 = b2 * inv_w2;
            weight_sum = weight0 + weight1 + weight2;

            if (weight_sum == 0.0f) continue;

            color.x = (weight0 * c0.x + weight1 * c1.x + weight2 * c2.x) / weight_sum;
            color.y = (weight0 * c0.y + weight1 * c1.y + weight2 * c2.y) / weight_sum;
            color.z = (weight0 * c0.z + weight1 * c1.z + weight2 * c2.z) / weight_sum;

            gfx_fb_set_pixel(fb, x, y, vec3_to_rgba(color));
        }
    }
}
