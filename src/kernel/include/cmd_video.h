#pragma once

/** @file cmd_video.h
 *  @brief Comandos de shell que exercitam o caminho grafico.
 *  @author Gabriel-lima
 *  @date 2026-09-06
 *
 *  Ficam fora de `root/` de proposito: `root/` guarda comandos portateis, e
 *  estes dependem de BIOS, modo de video e do framebuffer da maquina.
 */

/** @brief Comando `mode`: mostra ou troca o modo de video.
 *  @param argc Numero de argumentos.
 *  @param argv Argumentos; sem nenhum, apenas descreve o modo ativo.
 *  @return 0 em caso de sucesso, 1 em caso de erro.
 */
int cmd_video_mode(int argc, char **argv);

/** @brief Comando `gfxdemo`: desenha um cubo com o rasterizador do gfx.
 *  @param argc Numero de argumentos.
 *  @param argv Argumentos; aceita o mesmo formato de modo que `mode`.
 *  @return 0 em caso de sucesso, 1 em caso de erro.
 *  @note Prova de ponta a ponta da integracao: troca de modo pela BIOS,
 *  rasterizacao por software do gfx e blit para o formato do hardware.
 */
int cmd_video_gfxdemo(int argc, char **argv);
