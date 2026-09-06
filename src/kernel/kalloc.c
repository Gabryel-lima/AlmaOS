#include "include/kalloc.h"
#include "include/boot_info.h"

/* Nada abaixo de 1 MiB entra: essa faixa esta cheia de coisas com endereco
 * fixo — IVT, area de dados da BIOS, kernel em 0x12000, stage2, boot_info,
 * o trampolim de modo real em 0x30000, a stack e o framebuffer VGA. */
#define KALLOC_MIN_PHYSICAL 0x00100000u

/* Tipo E820 de memoria utilizavel. */
#define E820_TYPE_USABLE 1

static uint32_t kalloc_region_base;
static uint32_t kalloc_region_capacity;
static uint32_t kalloc_region_offset;

/** Arredonda para cima ate o proximo multiplo do tamanho de pagina.
 *  @param value Valor a alinhar.
 *  @return Valor alinhado, ou 0 se o arredondamento estourasse 32 bits.
 */
static uint32_t kalloc_align_up(uint32_t value) {
    if (value > 0xFFFFFFFFu - (KALLOC_PAGE_SIZE - 1u))
        return 0;
    return (value + KALLOC_PAGE_SIZE - 1u) & ~(KALLOC_PAGE_SIZE - 1u);
}

bool kalloc_init(void) {
    const boot_info_raw_t *bi = boot_info_get();
    const e820_entry_t *entries = boot_info_e820_entries();

    kalloc_region_base = 0;
    kalloc_region_capacity = 0;
    kalloc_region_offset = 0;

    if (!bi || !entries || bi->mmap_count == 0)
        return false;

    for (uint16_t i = 0; i < bi->mmap_count; i++) {
        uint64_t base = entries[i].base;
        uint64_t length = entries[i].length;
        uint64_t end;
        uint32_t usable_base;
        uint32_t usable_size;

        if (entries[i].type != E820_TYPE_USABLE)
            continue;
        if (base < KALLOC_MIN_PHYSICAL)
            continue;

        /* O kernel e 32-bit sem paginacao: so enderecca os primeiros 4 GiB.
         * Uma regiao que comece acima disso e invisivel daqui. */
        if (base > 0xFFFFFFFFull)
            continue;

        end = base + length;
        if (end > 0x100000000ull)
            end = 0x100000000ull;

        usable_base = (uint32_t)base;
        usable_size = (uint32_t)(end - base);

        /* Alinha o inicio a pagina; o que sobra na frente e descartado. */
        uint32_t aligned_base = kalloc_align_up(usable_base);
        if (aligned_base == 0 || aligned_base - usable_base >= usable_size)
            continue;
        usable_size -= (aligned_base - usable_base);

        if (usable_size > kalloc_region_capacity) {
            kalloc_region_base = aligned_base;
            kalloc_region_capacity = usable_size & ~(KALLOC_PAGE_SIZE - 1u);
        }
    }

    return kalloc_region_capacity > 0;
}

void *kalloc_pages(uint32_t size) {
    uint32_t aligned_size;
    uint32_t offset;

    if (kalloc_region_capacity == 0 || size == 0)
        return NULL;

    aligned_size = kalloc_align_up(size);
    if (aligned_size == 0)
        return NULL;

    if (aligned_size > kalloc_region_capacity - kalloc_region_offset)
        return NULL;

    offset = kalloc_region_offset;
    kalloc_region_offset += aligned_size;
    return (void *)(kalloc_region_base + offset);
}

uint32_t kalloc_base(void)     { return kalloc_region_base; }
uint32_t kalloc_capacity(void) { return kalloc_region_capacity; }
uint32_t kalloc_used(void)     { return kalloc_region_offset; }
