#pragma once

/** @brief Declarações de funções para manipulação de memória.
 *  @file memory.h
 *  @author Gabryel-lima
 *  @date 2026-03
 */

#include "stdbool.h"
#include "stdint.h"
#include "memdefs.h"

#pragma pack(push, 1)

/** Entrada do mapa de memória E820 retornado pela BIOS. 
 * 	@param BaseAddress: Endereço físico inicial da região de memória.
 * 	@param Length: Tamanho da região de memória em bytes.
 * 	@param Type: Tipo da região de memória (1 = RAM, 2 = Reservada, 3 = ACPI, etc.).
 * 	@param ExtendedAttributes: Atributos estendidos para a região, se suportados pela BIOS (opcional).
*/
typedef struct memory_e820_entry_t {
	uint64_t BaseAddress;			// Endereço físico inicial da região de memória.
	uint64_t Length;			    // Tamanho da região de memória em bytes.
	uint32_t Type;			        // Tipo da região de memória (1 = RAM, 2 = Reservada, 3 = ACPI, etc.).
	uint32_t ExtendedAttributes;	// Atributos estendidos para a região, se suportados pela BIOS (opcional).
} memory_e820_entry_t;

/** Visão do mapa de memória carregado pelo stage2. 
 * 	@param Entries: Ponteiro far para o buffer onde as entradas E820 são armazenadas.
 * 	@param EntryCount: Quantidade de entradas válidas atualmente no buffer.
 * 	@param Capacity: Capacidade máxima de entradas que o buffer pode armazenar.
*/
typedef struct memory_map_t {
	memory_e820_entry_t far* Entries;	// Ponteiro far para o buffer onde as entradas E820 são armazenadas.
	uint16_t EntryCount;			// Quantidade de entradas válidas atualmente no buffer.
	uint16_t Capacity;				// Capacidade máxima de entradas que o buffer pode armazenar.
} memory_map_t;

/** Alocador bump simples para o kernel.
 *  O alocador opera em uma região física fixa validada pelo mapa E820 e não
 *  tenta liberar blocos individualmente. Isso é suficiente para os estágios
 *  iniciais do boot, quando queremos previsibilidade e contratos curtos.
 * 	@param BasePhysical: Endereço físico inicial da região reservada para alocação.
 * 	@param Capacity: Tamanho máximo da região disponível para alocação.
 * 	@param Offset: Deslocamento atual dentro da região, indicando o próximo byte livre
 */
typedef struct memory_allocator_t {
	uint32_t BasePhysical;	// Endereço físico inicial da região reservada para alocação.
	uint32_t Capacity;		// Tamanho máximo da região disponível para alocação.
	uint32_t Offset;		// Deslocamento atual dentro da região, indicando o próximo byte livre.
} memory_allocator_t;

/** Contrato mínimo de boot repassado ao kernel.
 *  O bloco vive em uma região fixa e aponta para o mapa E820, alocador,
 *  buffers de I/O e informações visuais reservadas para fases seguintes.
 * 	@param BootDrive: Número do drive BIOS do boot.
 * 	@param VideoMode: Modo de vídeo inicial (0 = texto, 1 = gráfico).
 * 	@param Flags: Flags indicando quais campos são válidos (ex: presença do mapa E820, framebuffer, etc.).
 * 	@param MemoryMap: Visão do mapa de memória carregado pelo stage2.
 */
typedef struct memory_boot_info_t {
	uint8_t BootDrive;					// Número do drive BIOS do boot.
	uint8_t VideoMode;					// Modo de vídeo inicial (0 = texto, 1 = gráfico).
	uint16_t Flags;						// Flags indicando quais campos são válidos (ex: presença do mapa E820, framebuffer, etc.).
	memory_map_t MemoryMap;				// Visão do mapa de memória carregado pelo stage2.
	memory_allocator_t KernelAllocator;	// Alocador simples para o kernel.
	void far* IoBuffer;					// Buffer de I/O.
	uint32_t IoBufferSize;				// Tamanho do buffer de I/O.
	void far* Framebuffer;				// Ponteiro para o framebuffer.
	uint16_t FramebufferWidth;			// Largura do framebuffer.
	uint16_t FramebufferHeight;			// Altura do framebuffer.
	uint16_t FramebufferPitch;			// Pitch do framebuffer.
	uint8_t FramebufferBpp;				// Bits por pixel do framebuffer.
	uint8_t FramebufferFlags;			// Flags indicando propriedades do framebuffer.
} memory_boot_info_t;

#pragma pack(pop)

/** Modos de vídeo suportados pelo bootloader. 
 * 	@param MEMORY_VIDEO_MODE_TEXT: Modo de vídeo texto (80x25).
 * 	@param MEMORY_VIDEO_MODE_GRAPHICS: Modo de vídeo gráfico (VGA,
*/
enum memory_video_mode {
	MEMORY_VIDEO_MODE_TEXT = 0,			// Modo de vídeo texto (80x25).
	MEMORY_VIDEO_MODE_GRAPHICS = 1,		// Modo de vídeo gráfico (VGA, etc.).
};

/** Flags indicando quais campos do memory_boot_info_t são válidos. 
 * 	@param MEMORY_BOOT_INFO_HAS_MEMORY_MAP: Indica que o campo MemoryMap contém dados válidos.
 * 	@param MEMORY_BOOT_INFO_HAS_IO_BUFFER: Indica que os campos IoBuffer e IoBufferSize contêm dados válidos.
 * 	@param MEMORY_BOOT_INFO_HAS_FRAMEBUFFER: Indica que os campos relacionados ao framebuffer contêm dados válidos.
*/
enum memory_boot_info_flags {
	MEMORY_BOOT_INFO_HAS_MEMORY_MAP = 0x01,		// Indica que o campo MemoryMap contém dados válidos.
	MEMORY_BOOT_INFO_HAS_IO_BUFFER = 0x02,		// Indica que os campos IoBuffer e IoBufferSize contêm dados válidos.
	MEMORY_BOOT_INFO_HAS_FRAMEBUFFER = 0x04,	// Indica que os campos relacionados ao framebuffer contêm dados válidos.
};

/** Função que copia uma quantidade especificada de bytes de uma área de memória para outra
 * @param dst: Um ponteiro para a área de destino onde os bytes serão copiados.
 * @param src: Um ponteiro para a área de origem de onde os bytes serão copiados.
 * @param num: O número de bytes a serem copiados.
 * @return: Um ponteiro para a área de destino (dst).
*/
void far* memcpy(void far* dst, const void far* src, uint16_t num);

/** Função que preenche uma área de memória com um valor especificado
 * @param ptr: Um ponteiro para a área de memória a ser preenchida.
 * @param value: O valor a ser preenchido.
 * @param num: O número de bytes a serem preenchidos.
 * @return: Um ponteiro para a área de memória (ptr).
 */
void far* memset(void far* ptr, int value, uint16_t num);

/** Função que compara duas áreas de memória
 * @param ptr1: Um ponteiro para a primeira área de memória.
 * @param ptr2: Um ponteiro para a segunda área de memória.
 * @param num: O número de bytes a serem comparados.
 * @return: 0 se as áreas forem iguais, um valor diferente de 0 caso contrário.
 */
int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num);

/** Inicializa uma visão de mapa E820.
 *  @param map: Estrutura que receberá o buffer e o estado inicial.
 *  @param entries: Buffer far onde as entradas E820 serão armazenadas.
 *  @param capacity: Quantidade máxima de entradas disponíveis no buffer.
 */
void MEMORY_Map_Init(memory_map_t* map, memory_e820_entry_t far* entries, uint16_t capacity);

/** Coleta o mapa E820 da BIOS e armazena no buffer configurado.
 *  @param map: Estrutura do mapa já inicializada.
 *  @return: true se a coleta foi bem-sucedida, false caso contrário.
 */
bool MEMORY_Map_Collect(memory_map_t* map);

/** Inicializa um alocador simples em uma região física fixa.
 *  @param allocator: Alocador a ser configurado.
 *  @param basePhysical: Endereço físico inicial da região.
 *  @param capacity: Tamanho máximo da região disponível para alocação.
 */
void MEMORY_Allocator_Init(memory_allocator_t* allocator, uint32_t basePhysical, uint32_t capacity);

/** Configura o alocador a partir de uma região fixa validada pelo mapa E820.
 *  @param allocator: Alocador a ser configurado.
 *  @param map: Mapa E820 já coletado.
 *  @param basePhysical: Endereço físico inicial da região reservada.
 *  @param desiredSize: Tamanho desejado para a região reservada.
 *  @return: true se a região existir e couber na memória disponível, false caso contrário.
 */
bool MEMORY_Allocator_InitFromMap(memory_allocator_t* allocator,
								  const memory_map_t* map,
								  uint32_t basePhysical,
								  uint32_t desiredSize);

/** Aloca um bloco alinhado dentro da região reservada.
 *  @param allocator: Alocador bump a ser usado.
 *  @param size: Tamanho do bloco a ser reservado.
 *  @param alignment: Alinhamento desejado em bytes. Valores 0 ou 1 viram alinhamento de 1 byte.
 *  @return: Ponteiro far para o bloco reservado, ou NULL se não houver espaço.
 */
void far* MEMORY_Allocator_Alloc(memory_allocator_t* allocator, uint32_t size, uint16_t alignment);

/** Preenche o bloco de boot info compartilhado com o kernel.
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
void MEMORY_BootInfo_Init(memory_boot_info_t far* bootInfo,
						  uint8_t bootDrive,
						  uint8_t videoMode,
						  const memory_map_t* map,
						  const memory_allocator_t* allocator,
						  void far* ioBuffer,
						  uint32_t ioBufferSize,
						  void far* framebuffer,
						  uint16_t framebufferWidth,
						  uint16_t framebufferHeight,
						  uint16_t framebufferPitch,
						  uint8_t framebufferBpp,
						  uint8_t framebufferFlags);
