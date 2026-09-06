#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "memory.h"
#include "memdefs.h"
#include "x86.h"

#define BOOT_KERNEL_PATH "/kernel.bin"

static void boot_fail(const char* message) {
    printf("%s\r\n", message);
    for (;;)
        ;
}

void _cdecl cstart(uint16_t bootDrive) {
    disk_t disk;
    fat_file_t far* kernel;
    memory_map_t memoryMap;
    memory_allocator_t kernelAllocator;
    memory_boot_info_t far* bootInfo = (memory_boot_info_t far*)MEMORY_BOOT_INFO_ADDR;
    memory_e820_entry_t far* memoryEntries = (memory_e820_entry_t far*)MEMORY_E820_MAP_ADDR;

    if (!DISK_Initialize(&disk, bootDrive))
        boot_fail("Disk init error");

    if (!FAT_Initialize(&disk))
        boot_fail("FAT init error");

    MEMORY_Map_Init(&memoryMap, memoryEntries, MEMORY_E820_MAP_CAPACITY);
    if (!MEMORY_Map_Collect(&memoryMap))
        boot_fail("Memory map error");

    if (!MEMORY_Allocator_InitFromMap(&kernelAllocator,
                                      &memoryMap,
                                      MEMORY_HEAP_MIN_PHYSICAL,
                                      MEMORY_KERNEL_HEAP_SIZE))
        boot_fail("Kernel heap error");

    /* O stage2 entrega o controle com o modo texto 80x25 ativo, então é ele
     * que o boot_info descreve. Antes esses seis campos iam zerados, o que
     * fazia o kernel ler "não há framebuffer" quando na verdade havia um —
     * só não era gráfico. Ligar um modo gráfico aqui seria pior: o kernel
     * perderia a saída de texto (panic, log, shell) logo no boot. Quem troca
     * de modo é o kernel, em runtime, pelo trampolim de modo real
     * (src/kernel/realmode.h + src/kernel/video.h). */
    MEMORY_BootInfo_Init(bootInfo,
                         bootDrive,
                         MEMORY_VIDEO_MODE_TEXT,
                         &memoryMap,
                         &kernelAllocator,
                         (void far*)MEMORY_IO_BUFFER_ADDR,
                         MEMORY_IO_BUFFER_SIZE,
                         MEMORY_TEXT_FRAMEBUFFER_ADDR,
                         MEMORY_TEXT_FRAMEBUFFER_WIDTH,
                         MEMORY_TEXT_FRAMEBUFFER_HEIGHT,
                         MEMORY_TEXT_FRAMEBUFFER_PITCH,
                         MEMORY_TEXT_FRAMEBUFFER_BPP,
                         0);

    printf("Memory map: %u entries, heap=%lx size=%lu\r\n",
           memoryMap.EntryCount,
           kernelAllocator.BasePhysical,
           kernelAllocator.Capacity);

    kernel = FAT_Open(&disk, BOOT_KERNEL_PATH);
    if (kernel == 0)
        boot_fail("kernel.bin not found");

    if (FAT_ReadFar(&disk, kernel, kernel->Size, MEMORY_KERNEL_ADDR) != kernel->Size) {
        FAT_Close(kernel);
        boot_fail("Kernel load error");
    }

    FAT_Close(kernel);

    x86_FarJump(MEMORY_KERNEL_SEGMENT, MEMORY_KERNEL_OFFSET);

    for (;;)
        ;
}
