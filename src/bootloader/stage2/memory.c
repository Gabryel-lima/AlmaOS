#include "memory.h"
#include "x86.h"

static void far* MEMORY_PhysicalToFar(uint32_t physicalAddress) {
    uint16_t segment = (uint16_t)(physicalAddress >> 4);
    uint16_t offset = (uint16_t)(physicalAddress & 0x0F);

    return (void far*)(((uint32_t)segment << 16) | offset);
}

void far* memcpy(void far* dst, const void far* src, uint16_t num) {
    uint8_t far* u8Dst = (uint8_t far *)dst;
    const uint8_t far* u8Src = (const uint8_t far *)src;

    for (uint16_t i = 0; i < num; i++)
        u8Dst[i] = u8Src[i];

    return dst;
}

void far * memset(void far * ptr, int value, uint16_t num) {
    uint8_t far* u8Ptr = (uint8_t far *)ptr;

    for (uint16_t i = 0; i < num; i++)
        u8Ptr[i] = (uint8_t)value;

    return ptr;
}

int memcmp(const void far* ptr1, const void far* ptr2, uint16_t num) {
    const uint8_t far* u8Ptr1 = (const uint8_t far *)ptr1;
    const uint8_t far* u8Ptr2 = (const uint8_t far *)ptr2;

    for (uint16_t i = 0; i < num; i++)
        if (u8Ptr1[i] != u8Ptr2[i])
            return 1;

    return 0;
}

void MEMORY_Map_Init(memory_map_t* map, memory_e820_entry_t far* entries, uint16_t capacity) {
    if (map == 0)
        return;

    map->Entries = entries;
    map->EntryCount = 0;
    map->Capacity = capacity;

    if (entries != 0)
        memset(entries, 0, (uint16_t)(capacity * (uint16_t)sizeof(memory_e820_entry_t)));
}

bool MEMORY_Map_Collect(memory_map_t* map) {
    uint16_t entryCount = 0;

    if (map == 0 || map->Entries == 0 || map->Capacity == 0)
        return false;

    memset(map->Entries, 0, (uint16_t)(map->Capacity * (uint16_t)sizeof(memory_e820_entry_t)));

    if (!x86_Memory_GetMap(map->Entries, map->Capacity, &entryCount))
        return false;

    map->EntryCount = entryCount;
    return true;
}

void MEMORY_Allocator_Init(memory_allocator_t* allocator, uint32_t basePhysical, uint32_t capacity) {
    if (allocator == 0)
        return;

    allocator->BasePhysical = basePhysical;
    allocator->Capacity = capacity;
    allocator->Offset = 0;
}

bool MEMORY_Allocator_InitFromMap(memory_allocator_t* allocator,
                                  const memory_map_t* map,
                                  uint32_t basePhysical,
                                  uint32_t desiredSize) {
    uint32_t regionEnd = basePhysical + desiredSize;

    if (allocator == 0 || map == 0 || map->Entries == 0 || map->EntryCount == 0)
        return false;

    for (uint16_t i = 0; i < map->EntryCount; i++) {
        const memory_e820_entry_t far* entry = map->Entries + i;
        uint64_t entryBase = entry->BaseAddress;
        uint64_t entryEnd = entry->BaseAddress + entry->Length;

        if (entry->Type != 1)
            continue;

        if (entryBase > basePhysical)
            continue;

        if (entryEnd < regionEnd)
            continue;

        MEMORY_Allocator_Init(allocator, basePhysical, desiredSize);
        return true;
    }

    return false;
}

void far* MEMORY_Allocator_Alloc(memory_allocator_t* allocator, uint32_t size, uint16_t alignment) {
    uint32_t alignedOffset;
    uint32_t remainder;

    if (allocator == 0 || size == 0)
        return 0;

    if (alignment < 1)
        alignment = 1;

    alignedOffset = allocator->Offset;
    remainder = alignedOffset % alignment;
    if (remainder != 0)
        alignedOffset += alignment - remainder;

    if (alignedOffset > allocator->Capacity)
        return 0;

    if (size > allocator->Capacity - alignedOffset)
        return 0;

    allocator->Offset = alignedOffset + size;
    return MEMORY_PhysicalToFar(allocator->BasePhysical + alignedOffset);
}

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
                          uint8_t framebufferFlags) {
    uint16_t flags = 0;

    if (bootInfo == 0)
        return;

    bootInfo->BootDrive = bootDrive;
    bootInfo->VideoMode = videoMode;

    if (map != 0 && map->Entries != 0 && map->EntryCount != 0)
        flags |= MEMORY_BOOT_INFO_HAS_MEMORY_MAP;

    if (ioBuffer != 0 && ioBufferSize != 0)
        flags |= MEMORY_BOOT_INFO_HAS_IO_BUFFER;

    if (framebuffer != 0)
        flags |= MEMORY_BOOT_INFO_HAS_FRAMEBUFFER;

    bootInfo->Flags = flags;

    if (map != 0)
        bootInfo->MemoryMap = *map;
    else {
        bootInfo->MemoryMap.Entries = 0;
        bootInfo->MemoryMap.EntryCount = 0;
        bootInfo->MemoryMap.Capacity = 0;
    }

    if (allocator != 0)
        bootInfo->KernelAllocator = *allocator;
    else {
        bootInfo->KernelAllocator.BasePhysical = 0;
        bootInfo->KernelAllocator.Capacity = 0;
        bootInfo->KernelAllocator.Offset = 0;
    }

    bootInfo->IoBuffer = ioBuffer;
    bootInfo->IoBufferSize = ioBufferSize;
    bootInfo->Framebuffer = framebuffer;
    bootInfo->FramebufferWidth = framebufferWidth;
    bootInfo->FramebufferHeight = framebufferHeight;
    bootInfo->FramebufferPitch = framebufferPitch;
    bootInfo->FramebufferBpp = framebufferBpp;
    bootInfo->FramebufferFlags = framebufferFlags;
}
