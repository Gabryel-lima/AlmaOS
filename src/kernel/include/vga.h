#pragma once

/** @file vga.h
 *  @brief Driver de modo texto VGA (80x25, 0xB8000).
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Este driver é responsável por controlar a saída de texto no modo 
 *  texto do VGA, utilizando a memória mapeada em 0xB8000. Ele suporta operações 
 *  básicas como impressão de caracteres, strings e formatação básica.
 */

#include "kernel.h"

#define VGA_WIDTH  80       /* Número de colunas no modo texto do VGA */
#define VGA_HEIGHT 25       /* Número de linhas no modo texto do VGA */
#define VGA_ADDR   ((volatile uint16_t *)0xB8000)       /* Endereço base da memória do VGA text mode */

/** @brief Cores padrao do VGA text mode (4-bit, foreground e background). */
enum vga_color {
    VGA_COLOR_BLACK         = 0,    // Preto
    VGA_COLOR_BLUE          = 1,    // Azul
    VGA_COLOR_GREEN         = 2,    // Verde
    VGA_COLOR_CYAN          = 3,    // Ciano
    VGA_COLOR_RED           = 4,    // Vermelho
    VGA_COLOR_MAGENTA       = 5,    // Magenta
    VGA_COLOR_BROWN         = 6,    // Marrom
    VGA_COLOR_LIGHT_GREY    = 7,    // Cinza claro
    VGA_COLOR_DARK_GREY     = 8,    // Cinza escuro
    VGA_COLOR_LIGHT_BLUE    = 9,    // Azul claro
    VGA_COLOR_LIGHT_GREEN   = 10,   // Verde claro
    VGA_COLOR_LIGHT_CYAN    = 11,   // Ciano claro
    VGA_COLOR_LIGHT_RED     = 12,   // Vermelho claro
    VGA_COLOR_LIGHT_MAGENTA = 13,   // Magenta claro
    VGA_COLOR_YELLOW        = 14,   // Amarelo
    VGA_COLOR_WHITE         = 15,   // Branco
};

/** @brief Inicializa o driver VGA text mode (limpa a tela). */
void vga_init(void);

/** @brief Limpa a tela com a cor atual e move o cursor para (0, 0). */
void vga_clear(void);

/** @brief Define a cor atual (foreground + background).
 *  @param fg Cor do texto (foreground), usando enum vga_color.
 *  @param bg Cor de fundo (background), usando enum vga_color.
 */
void vga_set_color(uint8_t fg, uint8_t bg);

/** @brief Imprime um caractere na posicao atual e avanca o cursor.
 *  @param c Caractere a imprimir; '\n' e '\r' sao tratados como controle.
 */
void vga_putchar(char c);

/** @brief Imprime uma string terminada em nulo.
 *  @param s Ponteiro para a string (terminada em '\0').
 */
void vga_puts(const char *s);

/** @brief Retorna a coluna atual do cursor (0-based).
 *  @return Coluna atual (0 = inicio da linha).
 */
int vga_get_col(void);

/** @brief Printf basico para o kernel (suporta %c, %s, %d, %u, %x, %p, %%).
 *  @param fmt String de formato.
 *  @param ... Argumentos adicionais para o formato.
 */
void vga_printf(const char *fmt, ...);

/** @brief Variante com va_list para uso interno (klog, panic).
 *  @param fmt String de formato.
 *  @param args Lista de argumentos variadicos, ja processada por va_start.
 */
void vga_vprintf(const char *fmt, va_list args);

/** @brief Move o cursor de hardware para a posicao (col, row).
 *  @param col Coluna destino (0 a VGA_WIDTH-1).
 *  @param row Linha destino (0 a VGA_HEIGHT-1).
 */
void vga_set_cursor(int col, int row);

/** @brief Retorna a linha atual do cursor.
 *  @return Linha atual (0 a VGA_HEIGHT-1).
 */
int vga_get_row(void);
