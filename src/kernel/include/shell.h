#pragma once

/** @file shell.h
 *  @brief Shell interativo minimo para o AlmaOS.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Este shell é um ambiente de linha de comando simples 
 *  que permite ao usuário interagir com o sistema operacional. 
 *  Ele suporta comandos básicos como "help", "clear", e "echo".
 */

/** @brief Inicializa o estado do shell.
 *  @note Deve ser chamada durante a inicializacao do kernel, antes de shell_run().
 */
void shell_init(void);

/** @brief Loop principal do shell: le comandos do teclado e executa.
 *  @note Normalmente e a ultima funcao chamada na rotina de inicializacao do kernel.
 *  Nao retorna enquanto o sistema estiver em execucao.
 */
void shell_run(void);
