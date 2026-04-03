#include "stdint.h"
#include "stdio.h"
#include "disk.h"
#include "fat.h"
#include "x86.h"

void far* g_data = (void far*)0x00500200;

#define PROTECTED_MODE_LOAD_ADDR ((void far*)0x00007C00)

void _cdecl cstart(uint16_t bootDrive) {
    DISK disk;
    if (!DISK_Initialize(&disk, bootDrive)) {
        printf("Disk init error\r\n");
        goto end;
    }

    DISK_ReadSectors(&disk, 19, 1, g_data);

    if (!FAT_Initialize(&disk)) {
        printf("FAT init error\r\n");
        goto end;
    }

    // browse files in root
    FAT_File far* fd = FAT_Open(&disk, "/");
    FAT_DirectoryEntry entry;
    int i = 0;
    while (FAT_ReadEntry(&disk, fd, &entry) && i++ < 5) {
        // 1) end of directory
        if (entry.Name[0] == 0x00) break;
        // 2) deleted entry
        if ((uint8_t)entry.Name[0] == 0xE5) continue;
        // 3) LFN entry (attribute 0x0F)
        if (entry.Attributes == FAT_ATTRIBUTE_LFN) continue;
        // 4) volume label or other non-file entries
        if (entry.Attributes & FAT_ATTRIBUTE_VOLUME_ID) continue;
        // 5) only print first 5 entries to avoid flooding the output
        if (i++ >= 5) break;

        // safe to print SFN (11-byte 8.3). Replace non-printables with space.
        printf("  ");
        for (int j = 0; j < 11; j++) {
            char ch = entry.Name[j];
            if ((unsigned char)ch < 32) ch = ' ';
            putc(ch);
        }
        printf("\r\n");
    }
    FAT_Close(fd);

    // read test.txt
    char buffer[100];
    uint32_t read;
    uint8_t lastWasNewline = 1;
    fd = FAT_Open(&disk, "test.txt");
    while ((read = FAT_Read(&disk, fd, sizeof(buffer), buffer))) {
        for (uint32_t i = 0; i < read; i++) {
            if (buffer[i] == '\r')
                continue;
            if (buffer[i] == '\n') {
                putc('\r');
                lastWasNewline = 1;
            } else {
                lastWasNewline = 0;
            }
            putc_utf8(buffer[i]);
        }
    }
    if (!lastWasNewline) {
        putc('\r');
        putc('\n');
    }
    FAT_Close(fd);

    fd = FAT_Open(&disk, "protect.bin");
    if (fd == 0) {
        printf("Protected mode payload not found\r\n");
        goto end;
    }

    if (FAT_ReadFar(&disk, fd, fd->Size, PROTECTED_MODE_LOAD_ADDR) != fd->Size) {
        printf("Protected mode load error\r\n");
        FAT_Close(fd);
        goto end;
    }

    FAT_Close(fd);

    x86_FarJump(0x0000, 0x7C00);

end:
    for (;;);
}
