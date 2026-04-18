/* linker.ld - Script de linkagem para o kernel C (32-bit protected mode).
 * O entry point (_start) deve ficar no inicio do binario, alinhado com
 * o endereco fisico 0x12200 (logo apos o boot.bin de 512 bytes).
 */

ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0x12200;

    .text : {
        *(.text._start)
        *(.text .text.*)
    }

    .rodata : {
        *(.rodata .rodata.*)
    }

    .data : {
        *(.data .data.*)
    }

    .bss (NOLOAD) : {
        *(.bss .bss.*)
        *(COMMON)
    }

    _kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame*)
        *(.comment)
        *(.note*)
    }
}
