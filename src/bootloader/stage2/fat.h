#pragma once

#include "stdint.h"
#include "disk.h"

/** @brief Declarações de funções e estruturas para manipulação da FAT.
 * @file fat.h
 * @author Gabryel-lima
 * @date 2024-06
 */

#pragma pack(push, 1)

/** Estrutura que representa o setor de boot da FAT */
typedef struct FAT_DirectoryEntry {
    uint8_t Name[11];   // Nome do arquivo (8.3)
    uint8_t Attributes; // Atributos do arquivo
    uint8_t _Reserved; // Reservado
    uint8_t CreatedTimeTenths; // Décimos de segundo da criação
    uint16_t CreatedTime; // Hora de criação
    uint16_t CreatedDate; // Data de criação
    uint16_t AccessedDate; // Data de último acesso
    uint16_t FirstClusterHigh; // Número do primeiro cluster (parte alta)
    uint16_t ModifiedTime; // Hora da última modificação
    uint16_t ModifiedDate; // Data da última modificação
    uint16_t FirstClusterLow; // Número do primeiro cluster (parte baixa)
    uint32_t Size; // Tamanho do arquivo em bytes
} FAT_DirectoryEntry;

#pragma pack(pop)

/** Estrutura que representa um arquivo ou diretório aberto */
typedef struct FAT_File {
    int Handle;         // Handle do arquivo (índice na tabela de arquivos abertos, ou ROOT_DIRECTORY_HANDLE para o diretório raiz)
    bool IsDirectory;   // Indica se é um diretório
    uint32_t Position;  // Posição atual de leitura/escrita no arquivo
    uint32_t Size;      // Tamanho do arquivo em bytes
} FAT_File;

/** Estrutura que representa os dados da FAT */
enum FAT_Attributes {
    FAT_ATTRIBUTE_READ_ONLY = 0x01,     // Atributo de somente leitura
    FAT_ATTRIBUTE_HIDDEN = 0x02,        // Atributo de arquivo oculto
    FAT_ATTRIBUTE_SYSTEM = 0x04,        // Atributo de arquivo de sistema
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,     // Atributo de rótulo de volume
    FAT_ATTRIBUTE_DIRECTORY = 0x10,     // Atributo de diretório
    FAT_ATTRIBUTE_ARCHIVE = 0x20,       // Atributo de arquivo de arquivamento
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID // Atributo de entrada de nome longo (LFN)
};

/** Inicializa a FAT lendo o setor de boot, a tabela FAT e o diretório raiz.
 *
 *  @param disk: Um ponteiro para a estrutura DISK representando o disco onde a FAT está localizada.
 *  @return: true se a inicialização for bem-sucedida, false caso contrário.
 */
bool FAT_Initialize(DISK* disk);
/** Abre um arquivo ou diretório na FAT a partir de seu caminho.
 *
 *  @param disk: Um ponteiro para a estrutura DISK representando o disco onde a FAT está localizada.
 *  @param path: O caminho do arquivo ou diretório a ser aberto, usando '/' como separador. Exemplo: "/DIR1/FILE.TXT".
 *  @return: Um ponteiro para a estrutura FAT_File representando o arquivo ou diretório aberto, ou NULL se não for encontrado ou ocorrer um erro.
 */
FAT_File far* FAT_Open(DISK* disk, const char* path);
/** Lê dados de um arquivo ou diretório aberto na FAT.
 *
 *  @param disk: Um ponteiro para a estrutura DISK representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura FAT_File representando o arquivo ou diretório aberto.
 *  @param byteCount: O número de bytes a serem lidos.
 *  @param dataOut: Um ponteiro para o buffer onde os dados lidos serão armazenados.
 *  @return: O número de bytes efetivamente lidos, ou 0 se ocorrer um erro.
 */
uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t byteCount, void* dataOut);
/** Lê dados de um arquivo aberto na FAT para um buffer far.
 *
 *  @param disk: Um ponteiro para a estrutura DISK representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura FAT_File representando o arquivo ou diretório aberto.
 *  @param byteCount: O número de bytes a serem lidos.
 *  @param dataOut: Um ponteiro far para o buffer onde os dados lidos serão armazenados.
 *  @return: O número de bytes efetivamente lidos, ou 0 se ocorrer um erro.
 */
uint32_t FAT_ReadFar(DISK* disk, FAT_File far* file, uint32_t byteCount, void far* dataOut);
/** Lê a próxima entrada de diretório de um diretório aberto na FAT.
 *
 *  @param disk: Um ponteiro para a estrutura DISK representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura FAT_File representando o diretório aberto.
 *  @param dirEntry: Um ponteiro para a estrutura FAT_DirectoryEntry onde a entrada lida será armazenada.
 *  @return: true se uma entrada foi lida com sucesso, false se não houver mais entradas ou ocorrer um erro.
 */
bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* dirEntry);
/** Fecha um arquivo ou diretório aberto na FAT, liberando quaisquer recursos associados.
 *
 *  @param file: Um ponteiro para a estrutura FAT_File representando o arquivo ou diretório a ser fechado.
 */
void FAT_Close(FAT_File far* file);
