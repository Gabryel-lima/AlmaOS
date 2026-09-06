#include "include/fpu.h"

#define CR0_MP 0x00000002u  /* Monitor Coprocessor */
#define CR0_EM 0x00000004u  /* Emulation: se ligado, x87 levanta #NM */
#define CR0_TS 0x00000008u  /* Task Switched */

bool fpu_init(void) {
    uint32_t cr0;
    uint16_t status = 0x5A5A;   /* sentinela: FNSTSW tem de sobrescrever isto */

    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    cr0 &= ~(CR0_EM | CR0_TS);
    cr0 |= CR0_MP;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0));

    __asm__ volatile("fninit");
    __asm__ volatile("fnstsw %0" : "=m"(status));

    /* Depois de FNINIT a palavra de status e zero. Se a sentinela sobreviveu,
     * nao ha FPU respondendo e nao adianta seguir. */
    return status == 0;
}
