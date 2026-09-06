#pragma once

/** @file serial.h
 *  @brief Saida de log pela porta serial COM1 (16550 UART).
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *  @note Existe por dois motivos. Primeiro, depuracao: a tela de texto some
 *  quando o kernel troca para modo grafico, e o log serial continua. Segundo,
 *  automacao: com `qemu -serial stdio -display none` a saida do kernel vira
 *  texto num pipe, o que permite testar o boot sem olhar a tela.
 *
 *  Quando `serial_init()` tiver sucesso, `vga_putchar()` espelha cada caractere
 *  aqui — nao ha um segundo printf a manter.
 */

#include "kernel.h"

#define SERIAL_COM1_PORT 0x3F8      /* Porta base da COM1 no PC padrao. */

/** @brief Inicializa a COM1 em 38400 baud, 8N1, sem controle de fluxo.
 *  @return true se a UART respondeu ao teste de loopback, false caso contrario.
 *  @note Em caso de falha o espelhamento fica desligado e o kernel segue
 *  funcionando so com a saida de video — a serial e opcional por contrato.
 */
bool serial_init(void);

/** @brief Informa se a COM1 foi inicializada com sucesso.
 *  @return true se `serial_init()` validou a UART.
 */
bool serial_is_enabled(void);

/** @brief Envia um caractere pela COM1.
 *  @param c Caractere a enviar; '\n' vira "\r\n" para terminais de verdade.
 *  @note Vira no-op se a serial nao estiver habilitada.
 */
void serial_putchar(char c);

/** @brief Envia uma string terminada em nulo pela COM1.
 *  @param s Ponteiro para a string.
 */
void serial_puts(const char *s);

/** @brief Informa se ha um byte recebido esperando na COM1.
 *  @return true se `serial_getchar()` pode ser chamada sem bloquear.
 *  @note Sempre false enquanto a serial nao estiver habilitada.
 */
bool serial_has_data(void);

/** @brief Le um byte ja recebido da COM1.
 *  @return O byte recebido, ou 0 se nao havia nada pendente.
 *  @note Nao bloqueia: cheque `serial_has_data()` antes.
 */
char serial_getchar(void);
