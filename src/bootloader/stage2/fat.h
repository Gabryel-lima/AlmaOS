#pragma once

/** @brief Declarações de funções e estruturas para manipulação da FAT.
 *  @file fat.h
 *  @author Gabryel-lima
 *  @date 2026-03
*/

#include "stdint.h"
#include "disk.h"

#pragma pack(push, 1)

/** Estrutura que representa uma entrada de diretório da FAT. */
typedef struct fat_dir_entry_t {
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
} fat_dir_entry_t;

#pragma pack(pop)

/** Estado de um arquivo ou diretório aberto.
 *  O campo Handle aponta para a tabela interna de arquivos abertos; a raiz
 *  usa ROOT_DIRECTORY_HANDLE.
 *  @param Handle: Índice interno do arquivo aberto, ou ROOT_DIRECTORY_HANDLE para a raiz
 *  @param IsDirectory: true para diretórios, false para arquivos
 *  @param Position: Posição atual de leitura em bytes
 *  @param Size: Tamanho do arquivo em bytes
 */
typedef struct fat_file_t {
    int Handle;         // Índice interno do arquivo aberto, ou ROOT_DIRECTORY_HANDLE para a raiz
    bool IsDirectory;   // true para diretórios, false para arquivos
    uint32_t Position;  // Posição atual de leitura em bytes
    uint32_t Size;      // Tamanho do arquivo em bytes
} fat_file_t;

/** Máscaras de atributos usadas por entradas de diretório FAT. 
 *  @param FAT_ATTRIBUTE_READ_ONLY: Atributo de somente leitura
 *  @param FAT_ATTRIBUTE_HIDDEN: Atributo de arquivo oculto
 *  @param FAT_ATTRIBUTE_SYSTEM: Atributo de arquivo de sistema
 *  @param FAT_ATTRIBUTE_VOLUME_ID: Atributo de rótulo de volume
 *  @param FAT_ATTRIBUTE_DIRECTORY: Atributo de diretório
 *  @param FAT_ATTRIBUTE_ARCHIVE: Atributo de arquivo de arquivamento
 *  @param FAT_ATTRIBUTE_LFN: Combinação usada por entradas de nome longo (LFN)
*/
enum fat_attributes {
    FAT_ATTRIBUTE_READ_ONLY = 0x01,     // Atributo de somente leitura
    FAT_ATTRIBUTE_HIDDEN = 0x02,        // Atributo de arquivo oculto
    FAT_ATTRIBUTE_SYSTEM = 0x04,        // Atributo de arquivo de sistema
    FAT_ATTRIBUTE_VOLUME_ID = 0x08,     // Atributo de rótulo de volume
    FAT_ATTRIBUTE_DIRECTORY = 0x10,     // Atributo de diretório
    FAT_ATTRIBUTE_ARCHIVE = 0x20,       // Atributo de arquivo de arquivamento
    FAT_ATTRIBUTE_LFN = FAT_ATTRIBUTE_READ_ONLY | FAT_ATTRIBUTE_HIDDEN | FAT_ATTRIBUTE_SYSTEM | FAT_ATTRIBUTE_VOLUME_ID // Combinação usada por entradas de nome longo (LFN)
};

/** Inicializa a FAT lendo o setor de boot, a tabela FAT e o diretório raiz.
 *  @param disk: Um ponteiro para a estrutura disk_t representando o disco onde a FAT está localizada.
 *  @return: true se a inicialização for bem-sucedida, false caso contrário.
 */
bool FAT_Initialize(disk_t* disk);

/** Abre um arquivo ou diretório na FAT a partir de seu caminho.
 *  @param disk: Um ponteiro para a estrutura disk_t representando o disco onde a FAT está localizada.
 *  @param path: O caminho do arquivo ou diretório a ser aberto, usando '/' como separador. Exemplo: "/DIR1/FILE.TXT".
 *  @return: Um ponteiro para a estrutura fat_file_t representando o arquivo ou diretório aberto, ou NULL se não for encontrado ou ocorrer um erro.
 */
fat_file_t far* FAT_Open(disk_t* disk, const char* path);

/** Lê dados de um arquivo ou diretório aberto na FAT.
 *  @param disk: Um ponteiro para a estrutura disk_t representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura fat_file_t representando o arquivo ou diretório aberto.
 *  @param byteCount: O número de bytes a serem lidos.
 *  @param dataOut: Um ponteiro para o buffer onde os dados lidos serão armazenados.
 *  @return: O número de bytes efetivamente lidos, ou 0 se ocorrer um erro.
 */
uint32_t FAT_Read(disk_t* disk, fat_file_t far* file, uint32_t byteCount, void* dataOut);

/** Lê dados de um arquivo aberto na FAT para um buffer far.
 *  @param disk: Um ponteiro para a estrutura disk_t representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura fat_file_t representando o arquivo ou diretório aberto.
 *  @param byteCount: O número de bytes a serem lidos.
 *  @param dataOut: Um ponteiro far para o buffer onde os dados lidos serão armazenados.
 *  @return: O número de bytes efetivamente lidos, ou 0 se ocorrer um erro.
 */
uint32_t FAT_ReadFar(disk_t* disk, fat_file_t far* file, uint32_t byteCount, void far* dataOut);

/** Lê a próxima entrada de diretório de um diretório aberto na FAT.
 *  @param disk: Um ponteiro para a estrutura disk_t representando o disco onde a FAT está localizada.
 *  @param file: Um ponteiro para a estrutura fat_file_t representando o diretório aberto.
 *  @param dirEntry: Um ponteiro para a estrutura fat_dir_entry_t onde a entrada lida será armazenada.
 *  @return: true se uma entrada foi lida com sucesso, false se não houver mais entradas ou ocorrer um erro.
 */
bool FAT_ReadEntry(disk_t* disk, fat_file_t far* file, fat_dir_entry_t* dirEntry);

/** Fecha um arquivo ou diretório aberto na FAT, liberando quaisquer recursos associados.
 *  @param file: Um ponteiro para a estrutura fat_file_t representando o arquivo ou diretório a ser fechado.
 */
void FAT_Close(fat_file_t far* file);
