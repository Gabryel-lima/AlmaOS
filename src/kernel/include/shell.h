#pragma once

/** @file shell.h
 *  @brief Shell interativo minimo para o AlmaOS.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note Este shell é um ambiente de linha de comando simples 
 *  que permite ao usuário interagir com o sistema operacional. 
 *  Ele suporta comandos básicos como "help", "clear", e "echo".
 */

/** Inicializa o estado do shell. 
 *  Esta funcao deve ser chamada durante a inicializacao 
 *  do kernel para configurar o shell antes de entrar no loop principal.
*/
void shell_init(void);

/** Loop principal do shell (le comandos do teclado e executa). 
 *  Este loop deve ser chamado após a inicializacao do shell, e
 *  normalmente é a ultima funcao chamada na rotina de inicializacao do kernel.
*/
void shell_run(void);
