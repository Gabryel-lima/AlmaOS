#pragma once

#include <stdint.h>

/**
 * @brief Tipos matemáticos e estruturas básicas compartilhadas pelo sistema gráfico.
 * @author Gabryel-lima
 * @date 2026-03-02
 * @file include/gfx_math.h
 */

/** Vetor 2D float
 * @param x Coordenada X ou componente UV
 * @param y Coordenada Y ou componente UV
*/
typedef struct Vec2 {
    float x, y; /**< coordenadas 2D ou UV */
} Vec2;

/** Vetor 3D float 
 * @param x Coordenada X ou componente RGB
 * @param y Coordenada Y ou componente RGB
 * @param z Coordenada Z ou componente RGB
*/
typedef struct Vec3 {
    float x, y, z; /**< coordenadas 3D ou cor RGB (0.0-1.0) */
} Vec3;

/** Vetor 4-componentes float (por exemplo RGBA ou vetor homogêneo) 
 * @param x Coordenada X ou componente R
 * @param y Coordenada Y ou componente G
 * @param z Coordenada Z ou componente B
 * @param w Coordenada W ou componente A
*/
typedef struct Vec4 {
    float x, y, z, w; /**< coordenadas 3D homogêneas ou cor RGBA (0.0-1.0) */
} Vec4;

/** Matriz 4x4 (armazenada como 4 vetores Vec4) 
 * @param col Array de 4 colunas, onde cada coluna é um Vec4
*/
typedef struct Mat4 { Vec4 col[4]; } Mat4;

/** Matriz identidade 4x4.
 * @return Mat4 identidade (diagonal 1, resto 0).
 * @note Evita que cada chamador reescreva o mesmo literal de 16 floats.
*/
Mat4 mat4_identity(void);

/** gfx_min calcula o valor mínimo entre dois inteiros
 * @param a Primeiro valor
 * @param b Segundo valor
 * @return O menor valor entre a e b
*/
int gfx_min(int a, int b);

/** gfx_max calcula o valor máximo entre dois inteiros
 * @param a Primeiro valor
 * @param b Segundo valor
 * @return O maior valor entre a e b
*/
int gfx_max(int a, int b);

/** gfx_fminf calcula o menor valor entre dois floats 
 * @param a Primeiro valor
 * @param b Segundo valor
 * @return O menor valor entre a e b
*/
float gfx_fminf(float a, float b);

/** gfx_fmaxf calcula o valor máximo entre dois floats
 * @param a Primeiro valor
 * @param b Segundo valor
 * @return O maior valor entre a e b
*/
float gfx_fmaxf(float a, float b);

/** Component-wise minimum for Vec3
 * @param a Primeiro vetor
 * @param b Segundo vetor
 * @return Vetor cujos componentes são o menor valor entre `a` e `b`
 */
Vec3 vec3_min(Vec3 a, Vec3 b);

/** Component-wise maximum for Vec3
 * @param a Primeiro vetor
 * @param b Segundo vetor
 * @return Vetor cujos componentes são o maior valor entre `a` e `b`
 */
Vec3 vec3_max(Vec3 a, Vec3 b);

/** Clamp cada componente de `v` entre `lo` e `hi` 
 * @param v Vetor a ser limitado
 * @param lo Valor mínimo para cada componente
 * @param hi Valor máximo para cada componente
 * @return Vetor resultante da limitação de `v`
*/
Vec3 vec3_clamp(Vec3 v, float lo, float hi);

/** Calcula o valor da aresta em 2D para rasterização 
 * @param a Primeiro vértice do triângulo
 * @param b Segundo vértice do triângulo
 * @param c Ponto a ser testado
 * @return O valor da aresta (positivo se c estiver à esquerda de a->b, negativo se à direita, zero se colinear)
*/
float gfx_edge2d(Vec4 a, Vec4 b, Vec4 c);

/** Função utilitária para converter cor Vec3 (RGB) para uint32_t RGBA 
 * @param color Cor em formato Vec3 (valores entre 0.0 e 1.0)
 * @return Cor convertida para formato uint32_t RGBA (0xRRGGBBAA)
*/
uint32_t vec3_to_rgba(Vec3 color);
