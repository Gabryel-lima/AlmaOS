#pragma once

/** @file getopt.h
 *  @brief Parsing de opções curtas e longas para builtins do kernel.
 *  @author Gabriel-lima
 *  @date 2026-04-19
 *  @note Implementa getopt() (opções curtas) e getopt_long() (opções longas
 *  no estilo GNU: --nome, --nome=valor, --nome valor).
 *  As variáveis globais optarg, optind, opterr e optopt são compartilhadas
 *  com unistd.h; inclua apenas um dos dois headers por translation unit.
 *  Reinicialize optind = 1 antes de cada chamada a getopt()/getopt_long()
 *  com um novo vetor argv.
 */

#include "stddef.h"

/* ------------------------------------------------------------------ */
/* Variáveis globais de estado                                         */
/* ------------------------------------------------------------------ */

/** Ponteiro para o argumento da opção atual (ex: valor de -o ARG ou --out=ARG). */
extern char *optarg;

/** Índice do próximo elemento a processar em argv.
 *  @note Reinicializar para 1 antes de cada novo parse.
 */
extern int optind;

/** Controla mensagens de erro: 0 = silencioso, 1 = reporta (padrão). */
extern int opterr;

/** Armazena a opção inválida quando getopt()/getopt_long() retorna '?'. */
extern int optopt;

/* ------------------------------------------------------------------ */
/* Constantes para has_arg em option                                   */
/* ------------------------------------------------------------------ */

#define no_argument       0  /**< A opção longa não aceita argumento. */
#define required_argument 1  /**< A opção longa exige argumento. */
#define optional_argument 2  /**< A opção longa aceita argumento opcional. */

/* ------------------------------------------------------------------ */
/* Estrutura de opção longa                                            */
/* ------------------------------------------------------------------ */

/** @brief Descreve uma opção longa para getopt_long().
 *  @note O vetor deve ser terminado por um elemento com name == NULL.
 */
typedef struct option {
    const char *name;   /**< Nome da opção longa (sem "--"), ex: "help". */
    int  has_arg;       /**< no_argument, required_argument ou optional_argument. */
    int *flag;          /**< Se não NULL, *flag = val e retorna 0; se NULL, retorna val. */
    int  val;           /**< Valor retornado (ou gravado em *flag). */
} option_t;

/* ------------------------------------------------------------------ */
/* Declarações                                                         */
/* ------------------------------------------------------------------ */

/** @brief Analisa opções curtas no estilo POSIX.
 *  @param argc Número de argumentos.
 *  @param argv Vetor de strings.
 *  @param optstring Opções válidas (ex: "hno:"). Prefixo ':' = modo silencioso.
 *  @return Caractere da opção, '?' (inválida), ':' (argumento ausente,
 *          modo silencioso) ou -1 (fim das opções).
 */
int getopt(int argc, char * const argv[], const char *optstring);

/** @brief Analisa opções curtas e longas no estilo GNU.
 *  @details Processa tokens do tipo:
 *  - `-x`  `-xVALOR`  `-x VALOR`  (opções curtas via optstring)
 *  - `--nome`  `--nome=VALOR`  `--nome VALOR`  (opções longas via longopts)
 *  - `--` encerra o processamento de opções.
 *  @param argc Número de argumentos.
 *  @param argv Vetor de strings.
 *  @param optstring Opções curtas válidas. Pode ser "" se não houver opções curtas.
 *  @param longopts Vetor de option_t, terminado por {NULL,0,NULL,0}.
 *  @param longindex Se não NULL, recebe o índice da opção longa encontrada.
 *  @return val da opção longa (ou 0 se flag != NULL), caractere da opção
 *          curta, '?' (opção inválida ou argumento ausente) ou -1 (fim).
 *  @pre longopts != NULL.
 *  @note optind deve ser reinicializado para 1 entre chamadas com
 *  diferentes vetores argv.
 */
int getopt_long(int argc, char * const argv[], const char *optstring,
                const option_t *longopts, int *longindex);
