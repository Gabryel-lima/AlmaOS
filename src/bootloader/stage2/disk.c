#include "disk.h"
#include "x86.h"
#include "stdio.h"

bool DISK_Initialize(DISK* disk, uint8_t driveNumber) {
    uint8_t driveType;
    uint16_t cylinders, sectors, heads;

    disk->id = driveNumber;

    if (!x86_Disk_GetDriveParams(driveNumber, &driveType, &cylinders, &sectors, &heads))
        return false;
    disk->cylinders = cylinders + 1;
    disk->heads = heads + 1;
    disk->sectors = sectors;

    return true;
}

void DISK_LBA2CHS(DISK* disk, uint32_t lba, uint16_t* cylinderOut, uint16_t* sectorOut, uint16_t* headOut) {
    uint16_t lba16 = (uint16_t)lba;

    // setor = (LBA % setores por trilha + 1)
    *sectorOut = (lba16 % disk->sectors) + 1;

    // cilindro = (LBA / setores por trilha) / cabecas
    *cylinderOut = (lba16 / disk->sectors) / disk->heads;

    // cabeca = (LBA / setores por trilha) % cabecas
    *headOut = (lba16 / disk->sectors) % disk->heads;
}

bool DISK_ReadSectors(DISK* disk, uint32_t lba, uint8_t sectors, void far* dataOut) {
    uint16_t cylinder, sector, head;

    DISK_LBA2CHS(disk, lba, &cylinder, &sector, &head);

    for (int i = 0; i < 3; i++) {
        if (x86_Disk_Read(disk->id, cylinder, sector, head, sectors, dataOut))
            return true;

        x86_Disk_Reset(disk->id);
    }

    return false;
}
