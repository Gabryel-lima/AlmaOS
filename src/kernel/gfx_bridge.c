#include "include/gfx_bridge.h"
#include "include/video.h"
#include "include/kalloc.h"
#include "include/string.h"

/** Profundidade inicial do z-buffer.
 *
 *  O rasterizador do gfx usa "menor z ganha", entao o valor de limpeza tem de
 *  ser maior do que qualquer profundidade que um triangulo possa produzir.
 */
#define GFX_BRIDGE_DEPTH_FAR 1.0e30f

static Framebuffer bridge_framebuffer;
static float *bridge_depth;
static bool bridge_ready;

/* Descricao do modo para o qual o backbuffer atual foi dimensionado. Guardada
 * por valor porque `video_current()` muda assim que alguem troca de modo, e
 * apresentar um backbuffer com as dimensoes de outro modo escreveria fora do
 * framebuffer do hardware. */
static video_mode_info_t bridge_mode;

/** Extrai um canal de 8 bits de uma cor RGBA do gfx (0xRRGGBBAA).
 *  @param rgba Cor produzida pelo gfx.
 *  @param shift Deslocamento do canal (24 = R, 16 = G, 8 = B).
 *  @return Valor do canal em 8 bits.
 */
static inline uint8_t channel_of(uint32_t rgba, unsigned shift) {
    return (uint8_t)((rgba >> shift) & 0xFFu);
}

/** Reduz um canal de 8 bits para a largura que o modo de video usa.
 *  @param value Canal em 8 bits.
 *  @param size Numero de bits que o modo dedica a esse canal.
 *  @return Canal truncado; 0 se o modo nao tiver o canal.
 */
static inline uint32_t narrow_channel(uint8_t value, uint8_t size) {
    if (size == 0 || size >= 8)
        return value;
    return (uint32_t)(value >> (8u - size));
}

/** Converte uma cor RGBA do gfx para o formato de cor direta do modo VBE.
 *  @param rgba Cor produzida pelo gfx.
 *  @return Valor pronto para ser gravado no framebuffer, ja posicionado.
 *  @note Usa as posicoes e larguras de canal que a propria BIOS reportou, em
 *  vez de assumir um layout: modos VBE variam entre BGRX, RGB565 e outros.
 */
static uint32_t rgba_to_direct(uint32_t rgba) {
    uint32_t red   = narrow_channel(channel_of(rgba, 24), bridge_mode.red_size);
    uint32_t green = narrow_channel(channel_of(rgba, 16), bridge_mode.green_size);
    uint32_t blue  = narrow_channel(channel_of(rgba, 8),  bridge_mode.blue_size);

    return (red   << bridge_mode.red_pos)
         | (green << bridge_mode.green_pos)
         | (blue  << bridge_mode.blue_pos);
}

/** Converte uma cor RGBA do gfx para um indice da paleta RGB 3-3-2.
 *  @param rgba Cor produzida pelo gfx.
 *  @return Indice de 8 bits para o modo 13h.
 *  @note E o par exato da paleta que `video_set_vga13()` instala: 3 bits de
 *  vermelho, 3 de verde e 2 de azul, o que torna a conversao um truncamento.
 */
static uint8_t rgba_to_332(uint32_t rgba) {
    uint32_t red   = channel_of(rgba, 24) >> 5;   /* 3 bits */
    uint32_t green = channel_of(rgba, 16) >> 5;   /* 3 bits */
    uint32_t blue  = channel_of(rgba, 8)  >> 6;   /* 2 bits */

    return (uint8_t)((red << 5) | (green << 2) | blue);
}

bool gfx_bridge_init(void) {
    const video_mode_info_t *mode = video_current();
    uint32_t pixel_count;
    uint32_t color_bytes;
    uint32_t depth_bytes;
    void *color_memory;
    void *depth_memory;

    bridge_ready = false;
    bridge_depth = NULL;
    memset(&bridge_framebuffer, 0, sizeof(bridge_framebuffer));

    if (mode->kind == VIDEO_KIND_TEXT)
        return false;   /* nao ha pixels para desenhar em modo texto */
    if (mode->framebuffer == 0 || mode->width == 0 || mode->height == 0)
        return false;

    pixel_count = mode->width * mode->height;
    color_bytes = pixel_count * (uint32_t)sizeof(uint32_t);
    depth_bytes = pixel_count * (uint32_t)sizeof(float);

    color_memory = kalloc_pages(color_bytes);
    if (!color_memory)
        return false;

    depth_memory = kalloc_pages(depth_bytes);
    if (!depth_memory)
        return false;

    bridge_mode = *mode;

    bridge_framebuffer.pixels = (uint32_t *)color_memory;
    bridge_framebuffer.width  = mode->width;
    bridge_framebuffer.height = mode->height;
    /* Pitch do backbuffer, nao o do hardware: o backbuffer e sempre compacto
     * em 32 bits, e a diferenca de pitch e resolvida no present. */
    bridge_framebuffer.pitch  = mode->width * (uint32_t)sizeof(uint32_t);

    bridge_depth = (float *)depth_memory;

    gfx_fb_clear(&bridge_framebuffer, 0x000000FFu);
    gfx_bridge_clear_depth();

    bridge_ready = true;
    return true;
}

bool gfx_bridge_is_ready(void) {
    return bridge_ready;
}

Framebuffer *gfx_bridge_framebuffer(void) {
    return bridge_ready ? &bridge_framebuffer : NULL;
}

float *gfx_bridge_depth_buffer(void) {
    return bridge_ready ? bridge_depth : NULL;
}

void gfx_bridge_clear_depth(void) {
    uint32_t count;

    if (!bridge_depth)
        return;

    count = bridge_framebuffer.width * bridge_framebuffer.height;
    for (uint32_t i = 0; i < count; i++)
        bridge_depth[i] = GFX_BRIDGE_DEPTH_FAR;
}

void gfx_bridge_present(void) {
    const uint32_t *source;
    uint8_t *destination;

    if (!bridge_ready)
        return;

    source = bridge_framebuffer.pixels;
    destination = (uint8_t *)bridge_mode.framebuffer;

    for (uint32_t y = 0; y < bridge_mode.height; y++) {
        const uint32_t *source_row = source + (size_t)y * bridge_mode.width;
        uint8_t *destination_row = destination + (size_t)y * bridge_mode.pitch;

        switch (bridge_mode.bpp) {
        case 8:
            /* VGA 13h: um indice da paleta 3-3-2 por pixel. */
            for (uint32_t x = 0; x < bridge_mode.width; x++)
                destination_row[x] = rgba_to_332(source_row[x]);
            break;

        case 15:
        case 16:
            for (uint32_t x = 0; x < bridge_mode.width; x++)
                ((uint16_t *)destination_row)[x] = (uint16_t)rgba_to_direct(source_row[x]);
            break;

        case 24:
            /* Sem tipo de 24 bits: grava os tres bytes na ordem em que as
             * posicoes de canal do modo mandam. */
            for (uint32_t x = 0; x < bridge_mode.width; x++) {
                uint32_t packed = rgba_to_direct(source_row[x]);
                destination_row[x * 3 + 0] = (uint8_t)(packed & 0xFF);
                destination_row[x * 3 + 1] = (uint8_t)((packed >> 8) & 0xFF);
                destination_row[x * 3 + 2] = (uint8_t)((packed >> 16) & 0xFF);
            }
            break;

        case 32:
            for (uint32_t x = 0; x < bridge_mode.width; x++)
                ((uint32_t *)destination_row)[x] = rgba_to_direct(source_row[x]);
            break;

        default:
            return;     /* bpp que a ponte nao sabe converter */
        }
    }
}
