#include "stdint.h"
#include "stdio.h"

void _cdecl cstart(uint16_t bootDrive) {
    puts("Hello, World!");
    for(;;);
}
