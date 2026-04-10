#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "memdefs.h"

/** @brief Declarações de funções para manipulação de memória.
 *  @file memory.h
 *  @author Gabryel-lima
 *  @date 2024-06
 */

#pragma pack(push, 1)

/** Entrada do mapa de memória E820 retornado pela BIOS. */
typedef struct MEMORY_E820Entry {
	uint64_t BaseAddress;
	uint64_t Length;
	uint32_t Type;
	uint32_t ExtendedAttributes;
} MEMORY_E820Entry;

/** Visão do mapa de memória carregado pelo stage2. */
typedef struct MEMORY_Map {
	MEMORY_E820Entry far* Entries;
	uint16_t EntryCount;
	uint16_t Capacity;
} MEMORY_Map;

/** Alocador bump simples para o kernel.
 *
 *  O alocador opera em uma região física fixa validada pelo mapa E820 e não
 *  tenta liberar blocos individualmente. Isso é suficiente para os estágios
 *  iniciais do boot, quando queremos previsibilidade e contratos curtos.
 */
typedef struct MEMORY_Allocator {
	uint32_t BasePhysical;
	uint32_t Capacity;
	uint32_t Offset;
} MEMORY_Allocator;

/** Contrato mínimo de boot repassado ao kernel.
 *
 *  O bloco vive em uma região fixa e aponta para o mapa E820, alocador,
 *  buffers de I/O e informações visuais reservadas para fases seguintes.
 */
typedef struct MEMORY_BootInfo {
	uint8_t BootDrive;
	uint8_t VideoMode;
	uint16_t Flags;
	MEMORY_Map MemoryMap;
	MEMORY_Allocator KernelAllocator;
	void far* IoBuffer;
	uint32_t IoBufferSize;
	void far* Framebuffer;
	uint16_t FramebufferWidth;
	uint16_t FramebufferHeight;
	uint16_t FramebufferPitch;
	uint8_t FramebufferBpp;
	uint8_t FramebufferFlags;
} MEMORY_BootInfo;

#pragma pack(pop)

enum MEMORY_VideoMode {
	MEMORY_VIDEO_MODE_TEXT = 0,
	MEMORY_VIDEO_MODE_GRAPHICS = 1,
};

enum MEMORY_BootInfoFlags {
	MEMORY_BOOT_INFO_HAS_MEMORY_MAP = 0x01,
	MEMORY_BOOT_INFO_HAS_IO_BUFFER = 0x02,
	MEMORY_BOOT_INFO_HAS_FRAMEBUFFER = 0x04,
};

/** Função que copia uma quantidade especificada de bytes de uma área de memória para outra
 *  
 * @param dst: Um ponteiro para a área de destino onde os bytes serão copiados.
 * @param src: Um ponteiro para a área de origem de onde os bytes serão copiados.
 * @param num: O número de bytes a serem copiados.
 * @return: Um ponteiro para a área de destino (dst).
*/
void far* memcpy(void far* dst, const void far* src, uint16_t num);
/** Função que preenche uma área de memória com um valor especificado
 * 
 * @param ptr: Um ponteiro para a área de memória a ser preenchida.
 * @param value: O valor a ser preenchido.
 * @param num: O número de bytes a serem preenchidos.
 * @return: Um ponteiro para a área de memória (ptr).
 */
void far* memset(void far* ptr, int value, uint16_t num);
/** Função que compara duas áreas de memória
 * 
 * @param ptr1: Um ponteiro para a primeira área de memória.
 * @param ptr2: Um ponteiro para a segunda área de memória.
 * @param num: O número de bytes a serem comparados.
 * @return: 0 se as áreas forem iguais, um valor diferente de 0 caso contrário.
 */
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);

/** Inicializa uma visão de mapa E820.
 *
 *  @param map: Estrutura que receberá o buffer e o estado inicial.
 *  @param entries: Buffer far onde as entradas E820 serão armazenadas.
 *  @param capacity: Quantidade máxima de entradas disponíveis no buffer.
 */
void MEMORY_Map_Init(MEMORY_Map* map, MEMORY_E820Entry far* entries, uint16_t capacity);

/** Coleta o mapa E820 da BIOS e armazena no buffer configurado.
 *
 *  @param map: Estrutura do mapa já inicializada.
 *  @return: true se a coleta foi bem-sucedida, false caso contrário.
 */
bool MEMORY_Map_Collect(MEMORY_Map* map);

/** Inicializa um alocador simples em uma região física fixa.
 *
 *  @param allocator: Alocador a ser configurado.
 *  @param basePhysical: Endereço físico inicial da região.
 *  @param capacity: Tamanho máximo da região disponível para alocação.
 */
void MEMORY_Allocator_Init(MEMORY_Allocator* allocator, uint32_t basePhysical, uint32_t capacity);

/** Configura o alocador a partir de uma região fixa validada pelo mapa E820.
 *
 *  @param allocator: Alocador a ser configurado.
 *  @param map: Mapa E820 já coletado.
 *  @param basePhysical: Endereço físico inicial da região reservada.
 *  @param desiredSize: Tamanho desejado para a região reservada.
 *  @return: true se a região existir e couber na memória disponível, false caso contrário.
 */
bool MEMORY_Allocator_InitFromMap(MEMORY_Allocator* allocator,
								  const MEMORY_Map* map,
								  uint32_t basePhysical,
								  uint32_t desiredSize);

/** Aloca um bloco alinhado dentro da região reservada.
 *
 *  @param allocator: Alocador bump a ser usado.
 *  @param size: Tamanho do bloco a ser reservado.
 *  @param alignment: Alinhamento desejado em bytes. Valores 0 ou 1 viram alinhamento de 1 byte.
 *  @return: Ponteiro far para o bloco reservado, ou NULL se não houver espaço.
 */
void far* MEMORY_Allocator_Alloc(MEMORY_Allocator* allocator, uint32_t size, uint16_t alignment);

/** Preenche o bloco de boot info compartilhado com o kernel.
 *
 *  @param bootInfo: Estrutura far onde os dados serão gravados.
 *  @param bootDrive: Número do drive BIOS do boot.
 *  @param videoMode: Modo de vídeo inicial.
 *  @param map: Mapa E820 coletado.
 *  @param allocator: Alocador de heap simples.
 *  @param ioBuffer: Buffer far reservado para I/O.
 *  @param ioBufferSize: Tamanho do buffer de I/O.
 *  @param framebuffer: Ponteiro far para framebuffer, se existir.
 *  @param framebufferWidth: Largura do framebuffer.
 *  @param framebufferHeight: Altura do framebuffer.
 *  @param framebufferPitch: Pitch do framebuffer em bytes.
 *  @param framebufferBpp: Bits por pixel.
 *  @param framebufferFlags: Flags auxiliares de vídeo.
 */
void MEMORY_BootInfo_Init(MEMORY_BootInfo far* bootInfo,
						  uint8_t bootDrive,
						  uint8_t videoMode,
						  const MEMORY_Map* map,
						  const MEMORY_Allocator* allocator,
						  void far* ioBuffer,
						  uint32_t ioBufferSize,
						  void far* framebuffer,
						  uint16_t framebufferWidth,
						  uint16_t framebufferHeight,
						  uint16_t framebufferPitch,
						  uint8_t framebufferBpp,
						  uint8_t framebufferFlags);
