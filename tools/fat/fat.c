
// Inclusão das bibliotecas padrão de C
#include <stdio.h>      // Funções de entrada/saída
#include <stdint.h>     // Tipos inteiros com tamanho fixo
#include <stdlib.h>     // Funções utilitárias (malloc, free, etc)
#include <string.h>     // Manipulação de strings e memória
#include <ctype.h>      // Funções para verificação de caracteres


// Definição do tipo booleano usando uint8_t
typedef uint8_t bool;
#define true 1   // Valor verdadeiro
#define false 0  // Valor falso


// Estrutura que representa o setor de boot da FAT
typedef struct 
{
    uint8_t BootJumpInstruction[3]; // Instrução de salto do boot
    uint8_t OemIdentifier[8];       // Identificador OEM
    uint16_t BytesPerSector;        // Bytes por setor
    uint8_t SectorsPerCluster;      // Setores por cluster
    uint16_t ReservedSectors;       // Setores reservados
    uint8_t FatCount;               // Quantidade de tabelas FAT
    uint16_t DirEntryCount;         // Número de entradas no diretório raiz
    uint16_t TotalSectors;          // Total de setores
    uint8_t MediaDescriptorType;    // Tipo de mídia
    uint16_t SectorsPerFat;         // Setores por FAT
    uint16_t SectorsPerTrack;       // Setores por trilha
    uint16_t Heads;                 // Cabeças
    uint32_t HiddenSectors;         // Setores ocultos
    uint32_t LargeSectorCount;      // Contagem de setores grandes

    // Registro estendido de boot
    uint8_t DriveNumber;            // Número da unidade
    uint8_t _Reserved;              // Reservado
    uint8_t Signature;              // Assinatura
    uint32_t VolumeId;              // ID do volume (serial)
    uint8_t VolumeLabel[11];        // Rótulo do volume (11 bytes, preenchido com espaços)
    uint8_t SystemId[8];            // Identificador do sistema

    // ... não nos importamos com o código ...

} __attribute__((packed)) BootSector; // packed: sem alinhamento de bytes


// Estrutura que representa uma entrada de diretório na FAT
typedef struct 
{
    uint8_t Name[11];           // Nome do arquivo (8.3)
    uint8_t Attributes;         // Atributos do arquivo
    uint8_t _Reserved;          // Reservado
    uint8_t CreatedTimeTenths;  // Décimos de segundo da criação
    uint16_t CreatedTime;       // Hora da criação
    uint16_t CreatedDate;       // Data da criação
    uint16_t AccessedDate;      // Data de acesso
    uint16_t FirstClusterHigh;  // Parte alta do cluster inicial (FAT32)
    uint16_t ModifiedTime;      // Hora da modificação
    uint16_t ModifiedDate;      // Data da modificação
    uint16_t FirstClusterLow;   // Parte baixa do cluster inicial
    uint32_t Size;              // Tamanho do arquivo em bytes
} __attribute__((packed)) DirectoryEntry; // packed: sem alinhamento


// Variáveis globais para armazenar dados do disco
BootSector g_BootSector;              // Setor de boot
uint8_t* g_Fat = NULL;                // Tabela FAT
DirectoryEntry* g_RootDirectory = NULL; // Diretório raiz
uint32_t g_RootDirectoryEnd;          // LBA do fim do diretório raiz

// Lê o setor de boot do disco
bool readBootSector(FILE* disk)
{
    // Lê os bytes do setor de boot para a estrutura g_BootSector
    return fread(&g_BootSector, sizeof(g_BootSector), 1, disk) > 0;
}


// Lê setores do disco a partir de um LBA
bool readSectors(FILE* disk, uint32_t lba, uint32_t count, void* bufferOut)
{
    bool ok = true; // Variável de controle
    // Move o ponteiro do arquivo para o setor desejado
    ok = ok && (fseek(disk, lba * g_BootSector.BytesPerSector, SEEK_SET) == 0);
    // Lê os setores para o buffer de saída
    ok = ok && (fread(bufferOut, g_BootSector.BytesPerSector, count, disk) == count);
    return ok;
}


// Lê a tabela FAT do disco
bool readFat(FILE* disk)
{
    // Aloca memória para a FAT
    g_Fat = (uint8_t*) malloc(g_BootSector.SectorsPerFat * g_BootSector.BytesPerSector);
    // Lê os setores da FAT para a memória
    return readSectors(disk, g_BootSector.ReservedSectors, g_BootSector.SectorsPerFat, g_Fat);
}


// Lê o diretório raiz do disco
bool readRootDirectory(FILE* disk)
{
    // Calcula o LBA inicial do diretório raiz
    uint32_t lba = g_BootSector.ReservedSectors + g_BootSector.SectorsPerFat * g_BootSector.FatCount;
    // Calcula o tamanho total do diretório raiz
    uint32_t size = sizeof(DirectoryEntry) * g_BootSector.DirEntryCount;
    // Calcula quantos setores são necessários para armazenar o diretório raiz
    uint32_t sectors = (size / g_BootSector.BytesPerSector);
    if (size % g_BootSector.BytesPerSector > 0)
        sectors++; // Se sobrar bytes, adiciona mais um setor

    g_RootDirectoryEnd = lba + sectors; // Salva o LBA do fim do diretório raiz
    // Aloca memória para o diretório raiz
    g_RootDirectory = (DirectoryEntry*) malloc(sectors * g_BootSector.BytesPerSector);
    // Lê os setores do diretório raiz para a memória
    return readSectors(disk, lba, sectors, g_RootDirectory);
}


// Procura um arquivo pelo nome no diretório raiz
DirectoryEntry* findFile(const char* name)
{
    // Percorre todas as entradas do diretório raiz
    for (uint32_t i = 0; i < g_BootSector.DirEntryCount; i++)
    {
        // Compara o nome do arquivo (11 bytes)
        if (memcmp(name, g_RootDirectory[i].Name, 11) == 0)
            return &g_RootDirectory[i]; // Retorna o ponteiro para a entrada
    }

    return NULL; // Não encontrado
}


// Lê o conteúdo de um arquivo a partir de sua entrada de diretório
bool readFile(DirectoryEntry* fileEntry, FILE* disk, uint8_t* outputBuffer)
{
    bool ok = true; // Variável de controle
    uint16_t currentCluster = fileEntry->FirstClusterLow; // Cluster inicial do arquivo

    // Percorre os clusters do arquivo
    do {
        // Calcula o LBA do cluster atual
        uint32_t lba = g_RootDirectoryEnd + (currentCluster - 2) * g_BootSector.SectorsPerCluster;
        // Lê os setores do cluster para o buffer de saída
        ok = ok && readSectors(disk, lba, g_BootSector.SectorsPerCluster, outputBuffer);
        // Avança o ponteiro do buffer
        outputBuffer += g_BootSector.SectorsPerCluster * g_BootSector.BytesPerSector;

        // Calcula o índice na FAT para o próximo cluster
        uint32_t fatIndex = currentCluster * 3 / 2;
        if (currentCluster % 2 == 0)
            // Cluster par: pega os 12 bits menos significativos
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) & 0x0FFF;
        else
            // Cluster ímpar: pega os 12 bits mais significativos
            currentCluster = (*(uint16_t*)(g_Fat + fatIndex)) >> 4;

    } while (ok && currentCluster < 0x0FF8); // Continua até o fim do arquivo

    return ok;
}


// Função principal do programa
int main(int argc, char** argv)
{
    // Verifica se os argumentos foram passados corretamente
    if (argc < 3) {
        printf("Syntax: %s <disk image> <file name>, the strings have contain 11 characters and uppercase ./build/fat ./build/floppy.img \"TEST    TXT\"\n", argv[0]); // Exibe sintaxe
        return -1;
    }

    // Abre o arquivo de imagem do disco
    FILE* disk = fopen(argv[1], "rb");
    if (!disk) {
        fprintf(stderr, "Cannot open disk image %s!\n", argv[1]); // Erro ao abrir
        return -1;
    }

    // Lê o setor de boot
    if (!readBootSector(disk)) {
        fprintf(stderr, "Could not read boot sector!\n");
        return -2;
    }

    // Lê a tabela FAT
    if (!readFat(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        return -3;
    }

    // Lê o diretório raiz
    if (!readRootDirectory(disk)) {
        fprintf(stderr, "Could not read FAT!\n");
        free(g_Fat);
        free(g_RootDirectory);
        return -4;
    }

    // Procura o arquivo pelo nome
    DirectoryEntry* fileEntry = findFile(argv[2]);
    if (!fileEntry) {
        fprintf(stderr, "Could not find file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        return -5;
    }

    // Aloca buffer para o conteúdo do arquivo
    uint8_t* buffer = (uint8_t*) malloc(fileEntry->Size + g_BootSector.BytesPerSector);
    // Lê o arquivo para o buffer
    if (!readFile(fileEntry, disk, buffer)) {
        fprintf(stderr, "Could not read file %s!\n", argv[2]);
        free(g_Fat);
        free(g_RootDirectory);
        free(buffer);
        return -5;
    }

    // Exibe o conteúdo do arquivo
    for (size_t i = 0; i < fileEntry->Size; i++)
    {
        if (isprint(buffer[i])) // Se o byte é imprimível
            fputc(buffer[i], stdout); // Imprime o caractere
        else
            printf("<%02x>", buffer[i]); // Imprime o byte em hexadecimal
    }
    printf("\n");

    // Libera memória alocada
    free(buffer);
    free(g_Fat);
    free(g_RootDirectory);
    return 0;
}
