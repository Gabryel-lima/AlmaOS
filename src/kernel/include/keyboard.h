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

/** Inicializa o driver de teclado (habilita IRQ 1). 
 *  @note Esta funcao deve ser chamada durante a inicializacao do kernel 
 *  para configurar o handler de interrupcao do teclado e preparar 
 *  o buffer circular para uso.
*/
void keyboard_init(void);

/** Handler chamado pelo dispatcher de interrupcoes (IRQ 1). 
 *  @note Esta funcao deve ser registrada como handler para o vetor 
 *  de interrupcao correspondente ao teclado (IRQ 1) na IDT.
 *  Ela le os scancodes da porta de dados, converte-os 
 *  em caracteres ASCII usando uma tabela de 
 *  mapeamento, e os armazena no buffer circular.
*/
void keyboard_handler(void);

/** Le um caractere do buffer (nao-bloqueante).
 *  @note Esta funcao retorna imediatamente, mesmo que o buffer esteja vazio.
 *  @return O caractere lido, ou 0 se o buffer estiver vazio.
 */
char keyboard_read(void);

/** Le um caractere do buffer (bloqueante, aguarda com hlt). 
 *  @note Esta funcao bloqueia a CPU usando hlt até que haja pelo menos um caractere no buffer,
 *  garantindo que o kernel não desperdice ciclos de CPU em polling.
 *  @return O caractere lido do buffer.
*/
char keyboard_getchar(void);

/** Retorna true se existe pelo menos um caractere no buffer. 
 *  @note Esta funcao pode ser usada para verificar se ha dados 
 *  disponiveis antes de chamar keyboard_getchar(),
 *  evitando bloqueios desnecessarios.
 *  @return true se o buffer tiver pelo menos um caractere, false caso contrario.
*/
bool keyboard_has_data(void);
