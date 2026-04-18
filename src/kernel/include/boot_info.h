#pragma once

/** @file boot_info.h
 *  @brief Leitura do bloco de boot info passado pelo stage2 (real mode far pointers → flat).
 *  O bloco vive em 0x60000 e segue o layout de MEMORY_BootInfo do stage2.
 *  @author Gabriel-lima
 *  @date 2026-04-18
 *  @note O stage2 escreve o bloco de boot info em 0x60000 antes de entrar em modo protegido. 
 *  O kernel pode ler esse bloco para obter informações sobre o ambiente de boot, como o mapa 
 *  de memória E820, a região de heap reservada, e o framebuffer. 
 *  Os far pointers são convertidos para endereços flat usando a função far_to_flat(), 
 *  assumindo segmentação padrão (segmento * 16 + offset).
 */

#include "kernel.h"

#define BOOT_INFO_ADDR 0x60000 // Endereco onde o stage2 escreve o bloco de boot info (layout binario identico a MEMORY_BootInfo).
#define E820_MAP_ADDR 0x61000 // Endereco onde o stage2 escreve o mapa de memoria E820 (array de e820_entry_t, layout binario identico a MEMORY_Map).

/** Converte um far pointer OpenWatcom (segment:offset em uint32_t) para endereco flat. 
 *  @param far_ptr: Ponteiro far no formato segment<<16 | offset.
 *  @return: Ponteiro flat correspondente.
*/
static inline void *far_to_flat(uint32_t far_ptr) {
    uint16_t segment = (uint16_t)(far_ptr >> 16);
    uint16_t offset  = (uint16_t)(far_ptr & 0xFFFF);
    return (void *)((uint32_t)segment * 16 + offset);
}

/** Entrada do mapa de memoria E820. Layout identico ao do stage2. 
 *  @param base: Endereco fisico inicial da regiao.
 *  @param length: Tamanho da regiao em bytes.
 *  @param type: Tipo da regiao (1=RAM, 2=Reservada, 3=ACPI, 4=Defeituosa).
 *  @param ext_attrs: Atributos estendidos (usados para RAM, bit 0 = cacheável).
*/
typedef struct __attribute__((packed)) {
    uint64_t base;          // Endereco fisico inicial da regiao.
    uint64_t length;        // Tamanho da regiao em bytes.
    uint32_t type;          // Tipo da regiao (1=RAM, 2=Reservada, 3=ACPI, 4=Defeituosa).
    uint32_t ext_attrs;     // Atributos estendidos (usados para RAM, bit 0 = cacheável).
} e820_entry_t;

/** Alocador bump do stage2 (layout binario identico). 
 *  @param base_physical: Endereco fisico inicial da regiao de heap.
 *  @param capacity: Capacidade total da regiao de heap.
 *  @param offset: Deslocamento atual dentro da regiao de heap.
 */
typedef struct __attribute__((packed)) {
    uint32_t base_physical; // Endereco fisico inicial da regiao de heap.
    uint32_t capacity;      // Capacidade total da regiao de heap.
    uint32_t offset;        // Deslocamento atual dentro da regiao de heap.
} boot_allocator_t;

/** Bloco de boot info escrito pelo stage2 em 0x60000.
 *  Os campos far pointer sao armazenados como uint32_t (segment<<16 | offset).
 *  @param boot_drive: Numero do drive BIOS de boot.
 *  @param video_mode: Modo de video inicial (0=texto, 1=graficos).
 *  @param flags: Flags auxiliares (bit 0 = tem mapa de memoria E820).
 *  @param mmap_entries_far: Far pointer para o array de entradas E820 (e820_entry_t[]).
 *  @param mmap_count: Numero de entradas validas no array de entradas E820.
 *  @param mmap_capacity: Capacidade total do array de entradas E820 (tamanho alocado pelo stage2).
 *  @param heap_base: Endereco fisico inicial da regiao de heap reservada para o kernel.
 *  @param heap_capacity: Capacidade total da regiao de heap reservada para o kernel.
 *  @param heap_offset: Deslocamento atual dentro da regiao de heap (inicialmente 0).
 *  @param io_buffer_far: Far pointer para um buffer reservado para I/O (usado para leitura de setores, etc).
 *  @param io_buffer_size: Tamanho do buffer de I/O em bytes.
 *  @param framebuffer_far: Far pointer para o framebuffer, se existir (modo grafico).
 *  @param fb_width: Largura do framebuffer em pixels.
 *  @param fb_height: Altura do framebuffer em pixels.
 *  @param fb_pitch: Pitch do framebuffer em bytes por linha.
 *  @param fb_bpp: Bits por pixel do framebuffer.
 *  @param fb_flags: Flags auxiliares de video (bit 0 = framebuffer linear, bit 1 = framebuffer suporta escrita direta).
 */
typedef struct __attribute__((packed)) {
    uint8_t  boot_drive;            /* +0  */
    uint8_t  video_mode;            /* +1  */
    uint16_t flags;                 /* +2  */
    /* MEMORY_Map inline */
    uint32_t mmap_entries_far;      /* +4  far pointer para e820_entry_t[] */
    uint16_t mmap_count;            /* +8  */
    uint16_t mmap_capacity;         /* +10 */
    /* MEMORY_Allocator inline */
    uint32_t heap_base;             /* +12 */
    uint32_t heap_capacity;         /* +16 */
    uint32_t heap_offset;           /* +20 */
    /* Buffers */
    uint32_t io_buffer_far;         /* +24 */
    uint32_t io_buffer_size;        /* +28 */
    uint32_t framebuffer_far;       /* +32 */
    uint16_t fb_width;              /* +36 */
    uint16_t fb_height;             /* +38 */
    uint16_t fb_pitch;              /* +40 */
    uint8_t  fb_bpp;                /* +42 */
    uint8_t  fb_flags;              /* +43 */
} boot_info_raw_t;

/** Retorna ponteiro direto para o bloco de boot info. 
 *  @param: Nenhum. O bloco de boot info é lido diretamente do endereço fixo 0x60000.
 *  @return: Ponteiro para o bloco de boot info, interpretado como boot_info_raw_t. 
 *  O layout binário é idêntico ao de MEMORY_BootInfo do stage2
*/
static inline const boot_info_raw_t *boot_info_get(void) {
    return (const boot_info_raw_t *)BOOT_INFO_ADDR;
}

/** Retorna ponteiro flat para o array de entradas E820. 
 *  @param: Nenhum. O array de entradas E820 é acessado através do far pointer 
 *  armazenado no bloco de boot info.
 *  @return: Ponteiro flat para o array de entradas E820 (e820_entry_t[]). 
 *  O layout binário é idêntico ao de MEMORY_Map do stage2.
*/
static inline const e820_entry_t *boot_info_e820_entries(void) {
    const boot_info_raw_t *bi = boot_info_get();
    return (const e820_entry_t *)far_to_flat(bi->mmap_entries_far);
}
