#pragma once

#include "stdint.h"
#include "stdbool.h"

/** @brief Declarações de funções utilitárias para o bootloader.
 *  @file x86.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

/** Divide um número de 64 bits por um número de 32 bits.
 *
 *  @param dividend: O dividendo de 64 bits.
 *  @param divisor: O divisor de 32 bits.
 *  @param quotientOut: Um ponteiro para armazenar o quociente de 64 bits.
 *  @param remainderOut: Um ponteiro para armazenar o resto de 32 bits.
 */
void _cdecl x86_div64_32(uint64_t dividend, 
                         uint32_t divisor, 
                         uint64_t* quotientOut, 
                         uint32_t* remainderOut);
/** Escreve um caractere na tela usando o modo teletipo.
 *
 *  @param c: O caractere a ser escrito.
 *  @param page: A página de vídeo onde o caractere será escrito.
 */
void _cdecl x86_Video_WriteCharTeletype(char c, uint8_t page);
/** Reseta um disco.
 *
 *  @param drive: O número do drive a ser resetado.
 *  @return: true se o reset for bem-sucedido, false caso contrário.
 */
bool _cdecl x86_Disk_Reset(uint8_t drive);

/** Lê setores de um disco.
 *
 *  @param drive: O número do drive.
 *  @param cylinder: O cilindro a ser lido.
 *  @param sector: O setor a ser lido.
 *  @param head: A cabeça a ser lida.
 *  @param count: O número de setores a serem lidos.
 *  @param dataOut: Um ponteiro para o buffer onde os dados lidos serão armazenados.
 *  @return: true se a leitura for bem-sucedida, false caso contrário.
 */
bool _cdecl x86_Disk_Read(uint8_t drive,
                          uint16_t cylinder,
                          uint16_t sector,
                          uint16_t head,
                          uint8_t count,
                          void far * dataOut);

/** Obtém os parâmetros de um drive.
 *
 *  @param drive: O número do drive.
 *  @param driveTypeOut: Um ponteiro para armazenar o tipo do drive.
 *  @param cylindersOut: Um ponteiro para armazenar o número de cilindros.
 *  @param sectorsOut: Um ponteiro para armazenar o número de setores por trilha.
 *  @param headsOut: Um ponteiro para armazenar o número de cabeças.
 *  @return: true se a operação for bem-sucedida, false caso contrário.
 */
bool _cdecl x86_Disk_GetDriveParams(uint8_t drive,
                                    uint8_t* driveTypeOut,
                                    uint16_t* cylindersOut,
                                    uint16_t* sectorsOut,
                                    uint16_t* headsOut);
