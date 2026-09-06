#include "gfx_math.h"

#include <stdint.h>

/** @brief Implementações simples de funções matemáticas 
 *  para evitar erros de link quando a libm não está vinculada. 
 *  @author Gabryel-lima
 *  @date 2026-03-02
 *  @file core/gfx_math.c
 *  @note Estas funções são pequenas e portáteis.
*/

Mat4 mat4_identity(void) {
    Mat4 m = {
        .col = {
            { 1.0f, 0.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f, 1.0f },
        }
    };
    return m;
}

int gfx_min(int a, int b) { return a < b ? a : b; }

int gfx_max(int a, int b) { return a > b ? a : b; }

/** @brief Verifica o sinal de um número de ponto flutuante
 *  @param value Valor a ser verificado
 *  @return 1 se o valor for negativo, 0 caso contrário
*/
static int gfx_float_signbit(float value) {
    union {
        float f;
        uint32_t u;
    } bits = { value };

    return (bits.u >> 31) != 0;
}

static int gfx_float_isnan(float value) {
    return value != value;
}

/** Calcula o mínimo entre dois valores 
 *  @param a Primeiro valor
 *  @param b Segundo valor
 *  @return O menor valor entre `a` e `b`
 *  @note A implementação atual é um placeholder. 
 *  Para uma implementação mais robusta, considere casos como NaN e infinitos.
*/
float gfx_fminf(float a, float b) {
    if (gfx_float_isnan(a)) {
        return b;
    }
    if (gfx_float_isnan(b)) {
        return a;
    }
    if (a < b) {
        return a;
    }
    if (b < a) {
        return b;
    }

    if (a == 0.0f && b == 0.0f) {
        return (gfx_float_signbit(a) || gfx_float_signbit(b)) ? -0.0f : 0.0f;
    }

    return a;
}

/** Calcula o máximo entre dois valores
 *  @param a Primeiro valor
 *  @param b Segundo valor
 *  @return O maior valor entre `a` e `b` 
 *  @note A implementação atual é um placeholder. 
 *  Para uma implementação mais robusta, considere casos como NaN e infinitos.
*/
float gfx_fmaxf(float a, float b) {
    if (gfx_float_isnan(a)) {
        return b;
    }
    if (gfx_float_isnan(b)) {
        return a;
    }
    if (a > b) {
        return a;
    }
    if (b > a) {
        return b;
    }

    if (a == 0.0f && b == 0.0f) {
        return (gfx_float_signbit(a) && gfx_float_signbit(b)) ? -0.0f : 0.0f;
    }

    return a;
}

/** Calcula o mínimo entre dois vetores
 *  @param a Primeiro vetor
 *  @param b Segundo vetor
 *  @return Vetor contendo os menores valores de cada componente
*/
Vec3 vec3_min(Vec3 a, Vec3 b) {
    Vec3 r = { gfx_fminf(a.x, b.x), gfx_fminf(a.y, b.y), gfx_fminf(a.z, b.z) };
    return r;
}

/** Calcula o máximo entre dois vetores
 *  @param a Primeiro vetor
 *  @param b Segundo vetor
 *  @return Vetor contendo os maiores valores de cada componente
*/
Vec3 vec3_max(Vec3 a, Vec3 b) {
    Vec3 r = { gfx_fmaxf(a.x, b.x), gfx_fmaxf(a.y, b.y), gfx_fmaxf(a.z, b.z) };
    return r;
}

/** Limita os valores de um vetor dentro de um intervalo
 *  @param v Vetor a ser limitado
 *  @param lo Limite inferior
 *  @param hi Limite superior
 *  @return Vetor com os valores limitados
*/
Vec3 vec3_clamp(Vec3 v, float lo, float hi) {
    Vec3 r = { gfx_fmaxf(lo, gfx_fminf(hi, v.x)), gfx_fmaxf(lo, gfx_fminf(hi, v.y)), gfx_fmaxf(lo, gfx_fminf(hi, v.z)) };
    return r;
}

/** Calcula a área de um triângulo 2D usando coordenadas homogêneas
 *  @param a Primeiro vértice
 *  @param b Segundo vértice
 *  @param c Terceiro vértice
 *  @return Área do triângulo
*/
float gfx_edge2d(Vec4 a, Vec4 b, Vec4 c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

/** Converte um vetor de cor para um valor RGBA
 *  @param color Vetor de cor
 *  @return Valor RGBA
*/
uint32_t vec3_to_rgba(Vec3 color) {
    Vec3 c = vec3_clamp(color, 0.0f, 1.0f);
    uint8_t r = (uint8_t)(c.x * 255);
    uint8_t g = (uint8_t)(c.y * 255);
    uint8_t b = (uint8_t)(c.z * 255);
    return (r << 24) | (g << 16) | (b << 8) | 0xFF; // RGBA com alpha fixo em 255
}
