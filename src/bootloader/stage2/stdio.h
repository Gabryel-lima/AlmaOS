#pragma once

#include "stdbool.h"
#include "stdint.h"

/**
 *  stdio.h - Funções de E/S padrão para o carregador de boot estágio 2.
 *
 *  Este cabeçalho declara as funções básicas de E/S usadas no carregador de boot estágio 2.
 *  Ele fornece uma interface simples para exibir caracteres e cadeias de caracteres no console.
 *
 *  Observação: Esta implementação é mínima e não inclui todas as funções padrão de E/S.
 */
void putc(char c);
/**
 *  puts - Imprime uma string terminada em nulo no console.
 *
 *  @param str: Um ponteiro para a string terminada em nulo a ser exibida.
 *
 *  Esta função exibe cada caractere da string no console usando a função putc.
 *  Ela continua até encontrar o terminador nulo da string.
 */
void puts(const char* str);
/** 
 *  puts_f - Imprime uma string terminada em nulo localizada em memória far no console.
 *
 *  @param str: Um ponteiro para a string terminada em nulo localizada em memória far a ser exibida.
 *
 *  Esta função é semelhante à puts, mas é usada para strings localizadas em memória far, que podem estar além do limite de 64KB.
 *  Ela exibe cada caractere da string no console usando a função putc, continuando até encontrar o terminador nulo da string.
 */
void puts_f(const char far* str);
/**
 *  printf - Imprime uma string formatada no console.
 *
 *  @param fmt: A string de formato que especifica como os argumentos variáveis devem ser formatados e exibidos.
 *  @param ...: Os argumentos variáveis a serem formatados e exibidos de acordo com a string de formato.
 *
 *  Esta função processa a string de formato e os argumentos variáveis para exibir uma saída formatada no console.
 *  Ela suporta um subconjunto dos especificadores de formato padrão, como %d, %s, %x, etc.
 */
void _cdecl printf(const char* fmt, ...);
