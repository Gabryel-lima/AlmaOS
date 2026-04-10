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
    DISK disk;
    FAT_File far* kernel;
    MEMORY_Map memoryMap;
    MEMORY_Allocator kernelAllocator;
    MEMORY_BootInfo far* bootInfo = (MEMORY_BootInfo far*)MEMORY_BOOT_INFO_ADDR;
    MEMORY_E820Entry far* memoryEntries = (MEMORY_E820Entry far*)MEMORY_E820_MAP_ADDR;

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

    MEMORY_BootInfo_Init(bootInfo,
                         bootDrive,
                         MEMORY_VIDEO_MODE_TEXT,
                         &memoryMap,
                         &kernelAllocator,
                         (void far*)MEMORY_IO_BUFFER_ADDR,
                         MEMORY_IO_BUFFER_SIZE,
                         0,
                         0,
                         0,
                         0,
                         0,
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
