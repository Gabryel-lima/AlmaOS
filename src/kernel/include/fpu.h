#pragma once

/** @file fpu.h
 *  @brief Habilitacao da FPU x87.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  O rasterizador do gfx (third_party/gfx) trabalha em ponto flutuante, entao
 *  a FPU precisa estar utilizavel antes de qualquer chamada grafica. Na
 *  pratica a BIOS costuma deixar CR0 num estado que ja funciona, mas depender
 *  disso e depender de sorte: com CR0.EM ligado, a primeira instrucao x87
 *  levanta #NM (Device Not Available) em vez de calcular.
 */

#include "kernel.h"

/** @brief Coloca a FPU x87 num estado utilizavel.
 *  @return true se ha FPU e ela respondeu ao teste, false caso contrario.
 *  @note Limpa CR0.EM (nao emular), liga CR0.MP (monitorar coprocessador) e
 *  executa FNINIT. Se a maquina nao tiver FPU, o codigo grafico nao deve
 *  rodar — nao ha caminho de emulacao em software aqui.
 */
bool fpu_init(void);
