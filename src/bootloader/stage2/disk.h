#pragma once

#include "stdint.h"
#include "stdbool.h"

/** @brief Declarações de funções e estruturas para manipulação de discos.
 * @file disk.h
 * @author Gabryel-lima
 * @date 2024-06
 */

/** Representa um disco */
typedef struct DISK {
    uint8_t id; // Número do drive (0 para o primeiro disco, 1 para o segundo, etc.)
    uint16_t cylinders; // Número de cilindros do disco
    uint16_t sectors;   // Número de setores por trilha
    uint16_t heads;     // Número de cabeças do disco
} DISK;

/** Inicializa um disco lendo seus parâmetros (cilindros, setores, cabeças) usando a BIOS.
 *
 *  @param disk: Um ponteiro para a estrutura DISK a ser inicializada.
 *  @param driveNumber: O número do drive a ser inicializado (0 para o primeiro disco, 1 para o segundo, etc.).
 *  @return: true se a inicialização for bem-sucedida, false caso contrário.
 */
bool DISK_Initialize(DISK* disk, uint8_t driveNumber);
/** Lê setores de um disco usando endereçamento LBA.
 *
 *  @param disk: Um ponteiro para a estrutura DISK.
 *  @param lba: O endereço LBA do primeiro setor a ser lido.
 *  @param sectors: O número de setores a serem lidos.
 *  @param dataOut: Um ponteiro para o buffer onde os dados lidos serão armazenados.
 *  @return: true se a leitura for bem-sucedida, false caso contrário.
 */
bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut);
