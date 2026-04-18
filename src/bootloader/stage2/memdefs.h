#pragma once

/** @brief Definições de endereços de memória para o bootloader.
 *  @file memdefs.h
 *  @author Gabryel-lima
 *  @date 2026-03
 */

// 0x00000000 - 0x000003FF - interrupt vector table
// 0x00000400 - 0x000004FF - BIOS data area

#define MEMORY_MIN          0x00000500  // 0x00000500 - início da memória convencional utilizável
#define MEMORY_MAX          0x000A0000  // 0x00000500 - 0x0009FFFF - limite da memória convencional

// 0x00000500 - 0x00010500 - FAT driver
#define MEMORY_FAT_ADDR     ((void far*)0x00500000) // 0x00000500 - 0x00010500 - FAT driver
#define MEMORY_FAT_SIZE     0x00010000 // 0x00000500 - 0x00010500 - FAT driver

// 0x00012000 - 0x00020000 - kernel payload carregado pelo stage2
#define MEMORY_KERNEL_ADDR      ((void far*)0x12000000) // 0x00012000 - 0x00020000 - kernel payload carregado pelo stage2
#define MEMORY_KERNEL_SIZE      0x000E0000 // 0x00012000 - 0x00020000 - kernel payload carregado pelo stage2
#define MEMORY_KERNEL_SEGMENT   0x1200  // Segmento onde o kernel é carregado (0x1200:0000 = 0x12000000)
#define MEMORY_KERNEL_OFFSET    0x0000  // Offset dentro do segmento onde o kernel é carregado (0x1200:0000 = 0x12000000)

// 0x00060000 - 0x00060FFF - boot info fixo passado ao kernel
#define MEMORY_BOOT_INFO_ADDR   ((void far*)0x60000000)     // 0x00060000 - 0x00060FFF - boot info fixo passado ao kernel
#define MEMORY_BOOT_INFO_SIZE   0x00001000       // 0x00060000 - 0x00060FFF - boot info fixo passado ao kernel

// 0x00061000 - 0x00061FFF - buffer do mapa de memória E820
#define MEMORY_E820_MAP_ADDR        ((void far*)0x61000000)   // 0x00061000 - 0x00061FFF - buffer do mapa de memória E820
#define MEMORY_E820_MAP_CAPACITY    32      // Capacidade para 32 entradas de mapa de memória E820

// 0x00062000 - 0x00063FFF - buffer de I/O genérico do sistema
#define MEMORY_IO_BUFFER_ADDR   ((void far*)0x62000000)     // 0x00062000 - 0x00063FFF - buffer de I/O genérico do sistema
#define MEMORY_IO_BUFFER_SIZE   0x00002000      // 0x00062000 - 0x00063FFF - buffer de I/O genérico do sistema

// 0x00074000 - 0x00077FFF - stack do stage2 e da fase inicial do kernel
#define MEMORY_STAGE2_STACK_SEGMENT 0x7400      // Segmento onde a stack do stage2 é configurada (0x7400:0000 = 0x74000000)
#define MEMORY_STAGE2_STACK_TOP     0x4000      // Tamanho da stack do stage2 (0x7400:0000 - 0x7400:4000 = 0x00004000)
#define MEMORY_STAGE2_STACK_SIZE    0x00004000  // 0x00074000 - 0x00077FFF - stack do stage2 e da fase inicial do kernel

// 0x00078000 - 0x00097FFF - heap simples do kernel, validado pelo mapa de memória
#define MEMORY_KERNEL_HEAP_ADDR   ((void far*)0x78000000)       // 0x00078000 - 0x00097FFF - heap simples do kernel, validado pelo mapa de memória
#define MEMORY_KERNEL_HEAP_SIZE   0x00020000        // 0x00078000 - 0x00097FFF - heap simples do kernel, validado pelo mapa de memória
#define MEMORY_HEAP_MIN_PHYSICAL  0x00078000        // Endereço físico mínimo do heap do kernel, usado para validação contra o mapa de memória E820

// 0x000A0000 - 0x000BFFFF - framebuffer/VGA reservado para backend gráfico futuro
#define MEMORY_FRAMEBUFFER_ADDR   ((void far*)0xA0000000)   // 0x000A0000 - 0x000BFFFF - framebuffer/VGA reservado para backend gráfico futuro
#define MEMORY_FRAMEBUFFER_SIZE   0x00020000        // 0x000A0000 - 0x000BFFFF - framebuffer/VGA reservado para backend gráfico futuro

// 0x00020000 - 0x00030000 - stage2

// 0x00030000 - 0x00080000 - free

// 0x00080000 - 0x0009FFFF - Extended BIOS data area
// 0x000A0000 - 0x000C7FFF - Video
// 0x000C8000 - 0x000FFFFF - BIOS
