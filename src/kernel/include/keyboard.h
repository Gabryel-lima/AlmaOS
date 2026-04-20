#pragma once

/** @file keyboard.h
 *  @brief Driver de teclado PS/2 com buffer circular e scancode set 1.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Este driver de teclado suporta o scancode set 1, que é o mais comum para 
 *  teclados PS/2. Ele mantém um buffer circular de caracteres lidos,
 *  permitindo a leitura de caracteres de forma eficiente. O handler 
 *  de interrupção do teclado (IRQ 1) lê os scancodes da porta de dados (0x60),
 *  converte-os em caracteres ASCII usando uma tabela de mapeamento, e os armazena 
 *  no buffer circular. O driver também fornece funções para ler caracteres do buffer 
 *  de forma bloqueante.
 */

#include "kernel.h"

#define KB_DATA_PORT    0x60    // Porta de dados do teclado (scancodes lidos aqui).
#define KB_STATUS_PORT  0x64    // Porta de status do teclado (bit 0 = output buffer full, bit 1 = input buffer full).

#define KB_BUFFER_SIZE  128     // Tamanho do buffer circular de caracteres lidos do teclado.

/** @brief Layouts de teclado suportados. */
typedef enum kb_layout_t {
    KB_LAYOUT_US    = 0,    /**< US QWERTY (padrão norte-americano). */
    KB_LAYOUT_ABNT2 = 1,    /**< Brasileiro ABNT2 (padrão). */
} kb_layout_t;

/** @brief Inicializa o driver de teclado (habilita IRQ 1).
 *  @note Configura o handler de interrupcao e prepara o buffer circular.
 */
void keyboard_init(void);

/** @brief Handler do teclado chamado pelo dispatcher de interrupcoes (IRQ 1).
 *  @note Le o scancode da porta 0x60, converte para ASCII e armazena no buffer circular.
 */
void keyboard_handler(void);

/** @brief Le um caractere do buffer (nao-bloqueante).
 *  @return Caractere lido, ou 0 se o buffer estiver vazio.
 */
char keyboard_read(void);

/** @brief Le um caractere do buffer (bloqueante; suspende a CPU com hlt ate ter dado).
 *  @return Caractere lido do buffer.
 */
char keyboard_getchar(void);

/** @brief Verifica se existe pelo menos um caractere no buffer.
 *  @return true se houver dado disponivel, false se o buffer estiver vazio.
 */
bool keyboard_has_data(void);

/** @brief Define o layout ativo do teclado.
 *  @param layout KB_LAYOUT_US ou KB_LAYOUT_ABNT2.
 *  @note Pode ser chamada em qualquer momento apos keyboard_init().
 */
void keyboard_set_layout(kb_layout_t layout);

/** @brief Retorna o layout atualmente ativo.
 *  @return KB_LAYOUT_US ou KB_LAYOUT_ABNT2.
 */
kb_layout_t keyboard_get_layout(void);
