#include "fat.h"
#include "stdio.h"
#include "memdefs.h"
#include "utility.h"
#include "string.h"
#include "memory.h"
#include "ctype.h"
#include "stddef.h"

#define SECTOR_SIZE             512 // Tamanho de um setor em bytes
#define MAX_PATH_SIZE           256 // Tamanho máximo de um caminho de arquivo
#define MAX_FILE_HANDLES        10 // Número máximo de arquivos abertos simultaneamente
#define ROOT_DIRECTORY_HANDLE   -1 // Handle especial para o diretório raiz

#pragma pack(push, 1)

/** Estrutura que representa o setor de boot da FAT */
typedef struct  {
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

    // registro de boot estendido

    // registro de boot estendido
    uint8_t DriveNumber;        // Número da unidade
    uint8_t _Reserved;          // Reservado
    uint8_t Signature;          // Assinatura
    uint32_t VolumeId;          // ID do volume (serial)
    uint8_t VolumeLabel[11];    // Rótulo do volume (11 bytes, preenchido com espaços)
    uint8_t SystemId[8];        // Identificador do sistema

    // ... não nos importamos com o código ...

} FAT_BootSector;

#pragma pack(pop)

// Estrutura que representa uma entrada de diretório na FAT
typedef struct {
    uint8_t Buffer[SECTOR_SIZE];      // Buffer para armazenar os dados do cluster atual
    FAT_File Public;                  // Estrutura pública do arquivo (handle, posição, tamanho, etc)
    bool Opened;                      // Indica se o arquivo está aberto
    uint32_t FirstCluster;            // Primeiro cluster do arquivo
    uint32_t CurrentCluster;          // Cluster atual do arquivo
    uint32_t CurrentSectorInCluster;  // Setor atual dentro do cluster

} FAT_FileData;

// Estrutura que representa os dados da FAT
typedef struct {
    union {
        FAT_BootSector BootSector;    // Buffer para armazenar o setor de boot
        uint8_t BootSectorBytes[SECTOR_SIZE]; // Acesso ao setor de boot como bytes
    } BS;

    FAT_FileData RootDirectory;       // Estrutura que representa o diretório raiz

    FAT_FileData OpenedFiles[MAX_FILE_HANDLES];  // Arquivos abertos

} FAT_Data;

static FAT_Data far* g_Data;      // Ponteiro para os dados da FAT
static uint8_t far* g_Fat = NULL; // Ponteiro para a tabela FAT
static uint32_t g_DataSectionLba; // LBA do início da seção de dados (onde os clusters começam)

static bool FAT_ReadBootSector(DISK* disk) {
    return DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes);
}

static bool FAT_ReadFat(DISK* disk) {
    return DISK_ReadSectors(disk, g_Data->BS.BootSector.ReservedSectors, g_Data->BS.BootSector.SectorsPerFat, g_Fat);
}

bool FAT_Initialize(DISK* disk) {
    g_Data = (FAT_Data far*)MEMORY_FAT_ADDR;

    // ler o setor de boot
    if (!DISK_ReadSectors(disk, 0, 1, g_Data->BS.BootSectorBytes)) {
        printf("FAT: leitura do setor de boot falhou\r\n");
        return false;
    }

    if (g_Data->BS.BootSector.BytesPerSector != 512 || g_Data->BS.BootSector.SectorsPerCluster == 0) {
        return false;
    }

    // leitura da tabela FAT
    g_Fat = (uint8_t far*)g_Data + sizeof(FAT_Data);
    uint32_t fatSize = g_Data->BS.BootSector.BytesPerSector * g_Data->BS.BootSector.SectorsPerFat;
    if (sizeof(FAT_Data) + fatSize >= MEMORY_FAT_SIZE) {
        printf("FAT: memoria insuficiente para ler a FAT! Necessario %lu, possui apenas %u\r\n", sizeof(FAT_Data) + fatSize, MEMORY_FAT_SIZE);
        return false;
    }

    if (!FAT_ReadFat(disk)) {
        printf("FAT: leitura da FAT falhou\r\n");
        return false;
    }

    // abre o diretório raiz
    uint32_t rootDirLba = g_Data->BS.BootSector.ReservedSectors + g_Data->BS.BootSector.SectorsPerFat * g_Data->BS.BootSector.FatCount; // O diretório raiz começa após as áreas reservada e FAT
    uint32_t rootDirSize = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount; // O diretório raiz tem um tamanho fixo baseado no número de entradas

    g_Data->RootDirectory.Public.Handle = ROOT_DIRECTORY_HANDLE; // Handle especial para o diretório raiz
    g_Data->RootDirectory.Public.IsDirectory = true; // O diretório raiz é sempre um diretório
    g_Data->RootDirectory.Public.Position = 0; // A posição inicial é 0
    g_Data->RootDirectory.Public.Size = sizeof(FAT_DirectoryEntry) * g_Data->BS.BootSector.DirEntryCount; // O tamanho do diretório raiz é o número de entradas vezes o tamanho de cada entrada
    g_Data->RootDirectory.Opened = true; // O diretório raiz está sempre "aberto"
    g_Data->RootDirectory.FirstCluster = rootDirLba; // O diretório raiz começa no LBA calculado
    g_Data->RootDirectory.CurrentCluster = rootDirLba; // O cluster atual começa no mesmo LBA
    g_Data->RootDirectory.CurrentSectorInCluster = 0; // O setor atual dentro do cluster começa em 0

    if (!DISK_ReadSectors(disk, rootDirLba, 1, g_Data->RootDirectory.Buffer)) {
        printf("FAT: falha na leitura do diretorio raiz\r\n");
        return false;
    }

    // calcula o LBA do início da seção de dados
    uint32_t rootDirSectors = ((uint32_t)g_Data->BS.BootSector.DirEntryCount * sizeof(FAT_DirectoryEntry) + g_Data->BS.BootSector.BytesPerSector - 1) / g_Data->BS.BootSector.BytesPerSector;
    g_DataSectionLba = rootDirLba + rootDirSectors;

    // reseta os arquivos abertos
    for (int i = 0; i < MAX_FILE_HANDLES; i++)
        g_Data->OpenedFiles[i].Opened = false;

    return true;
}

uint32_t FAT_ClusterToLba(uint32_t cluster) {
    uint32_t lba = g_DataSectionLba + (cluster - 2) * g_Data->BS.BootSector.SectorsPerCluster;
    // printf("Cluster %lu -> LBA %lu\r\n", cluster, lba);
    return lba;
}

FAT_File far* FAT_OpenEntry(DISK* disk, FAT_DirectoryEntry* entry) {
    // encontrar handle vazio
    int handle = -1;
    for (int i = 0; i < MAX_FILE_HANDLES && handle < 0; i++) {

        if (!g_Data->OpenedFiles[i].Opened)
            handle = i;
    }

    // sem handles disponíveis
    if (handle < 0) {
        printf("FAT: sem handles de arquivo disponiveis\r\n");
        return NULL;
    }

    // configura variáveis
    FAT_FileData far* fd = &g_Data->OpenedFiles[handle];
    fd->Public.Handle = handle;
    fd->Public.IsDirectory = (entry->Attributes & FAT_ATTRIBUTE_DIRECTORY) != 0;
    fd->Public.Position = 0;
    fd->Public.Size = entry->Size;
    fd->FirstCluster = entry->FirstClusterLow + ((uint32_t)entry->FirstClusterHigh << 16);
    fd->CurrentCluster = fd->FirstCluster;
    fd->CurrentSectorInCluster = 0;

    if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster), 1, fd->Buffer)) {
        printf("FAT: erro de leitura\r\n");
        return false;
    }

    fd->Opened = true;
    return &fd->Public;
}

uint32_t FAT_NextCluster(uint32_t currentCluster) {    
    uint32_t fatIndex = currentCluster * 3 / 2;
    uint8_t far* ptr = g_Fat + fatIndex;
    uint16_t val;
    
    // Lê 16 bits de forma segura para ponteiros far, byte a byte
    val = (uint16_t)ptr[0] | ((uint16_t)ptr[1] << 8);

    if (currentCluster % 2 == 0)
        return val & 0x0FFF;
    else
        return val >> 4;
}

uint32_t FAT_Read(DISK* disk, FAT_File far* file, uint32_t byteCount, void* dataOut) {
    // obter dados do arquivo
    FAT_FileData far* fd = (file->Handle == ROOT_DIRECTORY_HANDLE) 
        ? &g_Data->RootDirectory 
        : &g_Data->OpenedFiles[file->Handle];

    uint8_t* u8DataOut = (uint8_t*)dataOut;

    // não ler além do fim do arquivo
    if (!fd->Public.IsDirectory) 
        byteCount = min(byteCount, fd->Public.Size - fd->Public.Position);

    while (byteCount > 0) {
        uint32_t leftInBuffer = SECTOR_SIZE - (fd->Public.Position % SECTOR_SIZE);
        uint32_t take = min(byteCount, leftInBuffer);

        memcpy(u8DataOut, fd->Buffer + fd->Public.Position % SECTOR_SIZE, take);
        u8DataOut += take;
        fd->Public.Position += take;
        byteCount -= take;

        // printf("leftInBuffer=%lu take=%lu\r\n", leftInBuffer, take);
        // Verifique se precisamos ler mais dados
        if (leftInBuffer == take) {
            // Tratamento especial para o diretório raiz
            if (fd->Public.Handle == ROOT_DIRECTORY_HANDLE) {
                // Não ler além do número fixo de entradas do diretório raiz
                if (fd->Public.Position >= fd->Public.Size) {
                    break;
                }

                ++fd->CurrentCluster; // Para o diretório raiz, isso incrementa o LBA

                // ler próximo setor
                if (!DISK_ReadSectors(disk, fd->CurrentCluster, 1, fd->Buffer)) {
                    printf("FAT: erro de leitura!\r\n");
                    break;
                }
            }
            else {
                // calcular próximo cluster e setor para ler
                if (++fd->CurrentSectorInCluster >= g_Data->BS.BootSector.SectorsPerCluster) {
                    fd->CurrentSectorInCluster = 0;
                    uint32_t next = FAT_NextCluster(fd->CurrentCluster);
                    // printf("Cluster %lu -> %lu\r\n", fd->CurrentCluster, next);
                    fd->CurrentCluster = next;
                }

                if (fd->CurrentCluster >= 0xFF8) {
                    // Marcar fim do arquivo
                    fd->Public.Size = fd->Public.Position;
                    break;
                }

                // ler próximo setor
                if (!DISK_ReadSectors(disk, FAT_ClusterToLba(fd->CurrentCluster) + fd->CurrentSectorInCluster, 1, fd->Buffer)) {
                    printf("FAT: erro de leitura!\r\n");
                    break;
                }
            }
        }
    }

    return u8DataOut - (uint8_t*)dataOut;
}

bool FAT_ReadEntry(DISK* disk, FAT_File far* file, FAT_DirectoryEntry* dirEntry) {
    return FAT_Read(disk, file, sizeof(FAT_DirectoryEntry), dirEntry) == sizeof(FAT_DirectoryEntry);
}

void FAT_Close(FAT_File far* file) {
    if (file->Handle == ROOT_DIRECTORY_HANDLE) {
        file->Position = 0;
        g_Data->RootDirectory.CurrentCluster = g_Data->RootDirectory.FirstCluster;
    }
    else {
        g_Data->OpenedFiles[file->Handle].Opened = false;
    }
}

bool FAT_FindFile(DISK* disk, FAT_File far* file, const char* name, FAT_DirectoryEntry* entryOut) {
    char fatName[12];
    FAT_DirectoryEntry entry;

    // converter nome para formato FAT
    memset(fatName, ' ', sizeof(fatName));
    fatName[11] = '\0';

    const char* ext = strchr(name, '.');
    if (ext == NULL)
        ext = name + 11;

    for (int i = 0; i < 8 && name[i] && name + i < ext; i++)
        fatName[i] = toupper(name[i]);

    if (ext != NULL && *ext == '.') {
        for (int i = 0; i < 3 && ext[i + 1]; i++)
            fatName[i + 8] = toupper(ext[i + 1]);
    }

    while (FAT_ReadEntry(disk, file, &entry)) {
        if (memcmp(fatName, entry.Name, 11) == 0) {
            *entryOut = entry;
            return true;
        }
    }
    
    return false;
}

FAT_File far* FAT_Open(DISK* disk, const char* path) {
    char name[MAX_PATH_SIZE];

    // ignorar barra inicial
    if (path[0] == '/')
        path++;

    FAT_File far* current = &g_Data->RootDirectory.Public;

    while (*path) {
        // extrair próximo nome de arquivo do caminho
        bool isLast = false;
        const char* delim = strchr(path, '/');
        if (delim != NULL) {
            memcpy(name, path, delim - path);
            name[delim - path] = '\0';
            path = delim + 1;
        }
        else {
            unsigned len = strlen(path);
            memcpy(name, path, len);
            name[len] = '\0';
            path += len;
            isLast = true;
        }
        
        // encontrar entrada no diretório atual
        FAT_DirectoryEntry entry;
        if (FAT_FindFile(disk, current, name, &entry)) {
            FAT_Close(current);

            // verificar se é um diretório
            if (!isLast && (entry.Attributes & FAT_ATTRIBUTE_DIRECTORY) == 0) {
                printf("FAT: %s nao e um diretorio\r\n", name);
                return NULL;
            }

            // abrir nova entrada de diretório
            current = FAT_OpenEntry(disk, &entry);
        }
        else {
            FAT_Close(current);

            printf("FAT: %s nao encontrado\r\n", name);
            return NULL;
        }
    }

    return current;
}
