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

/** Cores padrao do VGA text mode. 
 *  @param VGA_COLOR_BLACK: Preto
 *  @param VGA_COLOR_BLUE: Azul
 *  @param VGA_COLOR_GREEN: Verde
 *  @param VGA_COLOR_CYAN: Ciano
 *  @param VGA_COLOR_RED: Vermelho
 *  @param VGA_COLOR_MAGENTA: Magenta
 *  @param VGA_COLOR_BROWN: Marrom
 *  @param VGA_COLOR_LIGHT_GREY: Cinza claro
 *  @param VGA_COLOR_DARK_GREY: Cinza escuro
 *  @param VGA_COLOR_LIGHT_BLUE: Azul claro
 *  @param VGA_COLOR_LIGHT_GREEN: Verde claro
 *  @param VGA_COLOR_LIGHT_CYAN: Ciano claro
 *  @param VGA_COLOR_LIGHT_RED: Vermelho claro
 *  @param VGA_COLOR_LIGHT_MAGENTA: Magenta claro
 *  @param VGA_COLOR_YELLOW: Amarelo
 *  @param VGA_COLOR_WHITE: Branco
*/
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

/** Inicializa o driver VGA text mode (limpa a tela). 
 *  Esta função deve ser chamada durante a inicialização do kernel 
 *  para configurar o VGA antes de usá-lo para saída de texto.
*/
void vga_init(void);

/** Limpa a tela com a cor atual. 
 *  Esta função preenche toda a tela com espaços usando 
 *  a cor atual, e move o cursor para a posição (0, 0).
*/
void vga_clear(void);

/** Define a cor atual (foreground + background). 
 *  @param fg: Cor do texto (foreground), usando os valores do enum vga_color.
 *  @param bg: Cor de fundo (background), usando os valores do enum vga_color.
*/
void vga_set_color(uint8_t fg, uint8_t bg);

/** Imprime um caractere na posicao atual e avanca o cursor. 
 *  @param c: Caractere a ser impresso. Caracteres de controle 
 *  como '\n' e '\r' devem ser tratados adequadamente (ex: nova linha, retorno de carro).
*/
void vga_putchar(char c);

/** Imprime uma string terminada em nulo. 
 *  @param s: Ponteiro para a string a ser impressa. 
 *  A string deve ser terminada com um caractere nulo ('\0').
*/
void vga_puts(const char *s);

/** Printf basico para o kernel (suporta %c, %s, %d, %u, %x, %p, %%). 
 *  @param fmt: String de formato (similar a printf) para a mensagem a ser impressa.
 *  @param ...: Argumentos adicionais para a string de formato.
*/
void vga_printf(const char *fmt, ...);

/** Variante com va_list para uso interno (klog, panic). 
 *  @param fmt: String de formato (similar a printf) para a mensagem a ser impressa.
 *  @param args: Lista de argumentos variadicos, já processada por va_start
*/
void vga_vprintf(const char *fmt, va_list args);

/** Move o cursor de hardware para a posicao (col, row). 
 *  @param col: Coluna para mover o cursor (0 a VGA_WIDTH-1).
 *  @param row: Linha para mover o cursor (0 a VGA_HEIGHT-1).
*/
void vga_set_cursor(int col, int row);

/** Retorna a coluna atual do cursor. 
 *  @return A coluna atual do cursor (0 a VGA_WIDTH-1).
*/
int vga_get_col(void);

/** Retorna a linha atual do cursor. 
 *  @return A linha atual do cursor (0 a VGA_HEIGHT-1).
*/
int vga_get_row(void);
