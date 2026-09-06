#include "include/video.h"
#include "include/realmode.h"
#include "include/io.h"
#include "include/string.h"

/* ---- Portas do DAC do VGA (paleta do modo 13h) ---- */
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA        0x3C9

/* ---- Servicos de video da BIOS (int 10h) ---- */
#define BIOS_VIDEO_INT          0x10
#define BIOS_SET_MODE           0x0000  /* AH=00h, AL=modo */
#define VBE_GET_CONTROLLER_INFO 0x4F00
#define VBE_GET_MODE_INFO       0x4F01
#define VBE_SET_MODE            0x4F02
#define VBE_SUPPORTED           0x004F  /* AX de retorno de uma chamada VBE aceita */

#define VBE_MODE_LINEAR_FB      0x4000  /* bit de "use o linear framebuffer" em 4F02h */
#define VBE_MODE_LIST_END       0xFFFF

/* ---- Offsets do VbeInfoBlock (VBE 2.0, 512 bytes) ---- */
#define VBE_INFO_SIGNATURE      0x00    /* "VESA" na resposta */
#define VBE_INFO_VERSION        0x04    /* uint16, BCD */
#define VBE_INFO_MODE_PTR       0x0E    /* far pointer para a lista de modos */

/* ---- Offsets do ModeInfoBlock (VBE 2.0, 256 bytes) ---- */
#define VBE_MODE_ATTRIBUTES     0x00    /* uint16 */
#define VBE_MODE_PITCH          0x10    /* uint16, bytes por scanline */
#define VBE_MODE_WIDTH          0x12    /* uint16 */
#define VBE_MODE_HEIGHT         0x14    /* uint16 */
#define VBE_MODE_BPP            0x19    /* uint8  */
#define VBE_MODE_MEMORY_MODEL   0x1B    /* uint8  */
#define VBE_MODE_RED_SIZE       0x1F    /* uint8  */
#define VBE_MODE_RED_POS        0x20
#define VBE_MODE_GREEN_SIZE     0x21
#define VBE_MODE_GREEN_POS      0x22
#define VBE_MODE_BLUE_SIZE      0x23
#define VBE_MODE_BLUE_POS       0x24
#define VBE_MODE_PHYS_BASE      0x28    /* uint32, endereco do linear framebuffer */

/* ModeAttributes: bits que precisamos. */
#define VBE_ATTR_SUPPORTED      0x0001
#define VBE_ATTR_GRAPHICS       0x0010
#define VBE_ATTR_LINEAR_FB      0x0080

/* MemoryModel: os dois unicos que a ponte grafica sabe converter. */
#define VBE_MEMORY_PACKED       4
#define VBE_MEMORY_DIRECT       6

/* Enderecos fixos dos framebuffers legados. */
#define VGA_TEXT_FRAMEBUFFER    0x000B8000
#define VGA13_FRAMEBUFFER       0x000A0000

/** Quantos modos VBE guardamos da lista da BIOS.
 *  A lista costuma vir da ROM de video, mas algumas BIOS a colocam dentro do
 *  proprio VbeInfoBlock — que fica no mesmo buffer que a consulta de cada modo
 *  vai sobrescrever. Por isso a lista e copiada antes de qualquer 4F01h.
 */
#define VBE_MAX_MODES 128

static video_mode_info_t current_mode;
static bool vbe_available;
static uint16_t vbe_version;

/** Converte um far pointer de modo real (segmento<<16 | offset) em endereco linear.
 *  @param far_ptr Ponteiro far como a BIOS o devolve.
 *  @return Endereco fisico correspondente.
 */
static uint32_t far_ptr_to_linear(uint32_t far_ptr) {
    return ((far_ptr >> 16) & 0xFFFFu) * 16u + (far_ptr & 0xFFFFu);
}

static uint8_t buffer_u8(uint32_t offset) {
    return *(const volatile uint8_t *)(REALMODE_BUFFER_ADDR + offset);
}

static uint16_t buffer_u16(uint32_t offset) {
    return (uint16_t)(buffer_u8(offset) | ((uint16_t)buffer_u8(offset + 1) << 8));
}

static uint32_t buffer_u32(uint32_t offset) {
    return (uint32_t)buffer_u16(offset) | ((uint32_t)buffer_u16(offset + 2) << 16);
}

/** Registra o modo texto 80x25 como estado corrente.
 *  @note Usado no init e sempre que voltamos ao texto, para que a descricao
 *  do framebuffer nunca fique falando de um modo que ja saiu do ar.
 */
static void video_record_text_mode(void) {
    memset(&current_mode, 0, sizeof(current_mode));
    current_mode.kind = VIDEO_KIND_TEXT;
    current_mode.framebuffer = VGA_TEXT_FRAMEBUFFER;
    current_mode.width = 80;    /* colunas, nao pixels */
    current_mode.height = 25;   /* linhas, nao pixels */
    current_mode.pitch = 160;   /* 80 celulas de 2 bytes */
    current_mode.bpp = 16;      /* caractere + atributo */
}

/** Instala uma paleta RGB 3-3-2 nos 256 indices do DAC.
 *
 *  Com essa paleta o indice de 8 bits vira uma funcao pura da cor de 24 bits
 *  (3 bits de vermelho, 3 de verde, 2 de azul), entao converter um pixel RGBA
 *  para o modo 13h e so truncamento — sem tabela de busca nem dithering.
 *  O DAC do VGA guarda 6 bits por componente, dai o deslocamento de 2.
 */
static void video_install_332_palette(void) {
    outb(VGA_DAC_WRITE_INDEX, 0);

    for (unsigned index = 0; index < 256; index++) {
        unsigned red   = (index >> 5) & 0x07;    /* 3 bits */
        unsigned green = (index >> 2) & 0x07;    /* 3 bits */
        unsigned blue  = index & 0x03;           /* 2 bits */

        /* Expande cada campo para 8 bits e depois corta para os 6 do DAC. */
        outb(VGA_DAC_DATA, (uint8_t)((red   * 255u / 7u) >> 2));
        outb(VGA_DAC_DATA, (uint8_t)((green * 255u / 7u) >> 2));
        outb(VGA_DAC_DATA, (uint8_t)((blue  * 255u / 3u) >> 2));
    }
}

/** Executa `int 10h` com AX dado e ES:DI apontando para o buffer de transferencia.
 *  @param ax Valor de AX na entrada.
 *  @param cx Valor de CX na entrada (numero do modo nas chamadas VBE).
 *  @return true se a chamada saiu sem carry e com AX = 0x004F.
 */
static bool vbe_call(uint16_t ax, uint16_t cx) {
    realmode_regs_t regs;

    memset(&regs, 0, sizeof(regs));
    regs.eax = ax;
    regs.ecx = cx;
    regs.es  = REALMODE_BUFFER_SEG;
    regs.edi = 0;

    if (!realmode_int(BIOS_VIDEO_INT, &regs))
        return false;
    if (regs.eflags & REALMODE_FLAG_CARRY)
        return false;

    return (regs.eax & 0xFFFFu) == VBE_SUPPORTED;
}

void video_init(void) {
    video_record_text_mode();
    vbe_available = false;
    vbe_version = 0;

    if (!realmode_is_ready())
        return;

    /* A assinatura "VBE2" na entrada pede o bloco estendido do VBE 2.0.
     * Sem ela, uma BIOS 2.0 responde no formato 1.x. */
    memcpy((void *)REALMODE_BUFFER_ADDR, "VBE2", 4);

    if (!vbe_call(VBE_GET_CONTROLLER_INFO, 0))
        return;

    if (buffer_u8(VBE_INFO_SIGNATURE + 0) != 'V' ||
        buffer_u8(VBE_INFO_SIGNATURE + 1) != 'E' ||
        buffer_u8(VBE_INFO_SIGNATURE + 2) != 'S' ||
        buffer_u8(VBE_INFO_SIGNATURE + 3) != 'A')
        return;

    vbe_version = buffer_u16(VBE_INFO_VERSION);
    vbe_available = true;
}

const video_mode_info_t *video_current(void) {
    return &current_mode;
}

bool video_vbe_available(void) {
    return vbe_available;
}

uint16_t video_vbe_version(void) {
    return vbe_version;
}

bool video_set_text(void) {
    realmode_regs_t regs;

    if (!realmode_is_ready())
        return false;

    memset(&regs, 0, sizeof(regs));
    regs.eax = BIOS_SET_MODE | 0x03;    /* AH=00h, AL=03h: texto 80x25 colorido */

    if (!realmode_int(BIOS_VIDEO_INT, &regs))
        return false;

    video_record_text_mode();
    return true;
}

bool video_set_vga13(void) {
    realmode_regs_t regs;

    if (!realmode_is_ready())
        return false;

    memset(&regs, 0, sizeof(regs));
    regs.eax = BIOS_SET_MODE | 0x13;    /* AH=00h, AL=13h: 320x200, 256 cores */

    if (!realmode_int(BIOS_VIDEO_INT, &regs))
        return false;

    video_install_332_palette();

    memset(&current_mode, 0, sizeof(current_mode));
    current_mode.kind = VIDEO_KIND_VGA13;
    current_mode.framebuffer = VGA13_FRAMEBUFFER;
    current_mode.width = 320;
    current_mode.height = 200;
    current_mode.pitch = 320;
    current_mode.bpp = 8;
    return true;
}

/** Copia a lista de modos VBE da BIOS para um array do kernel.
 *  @param modes Destino com espaco para `capacity` entradas.
 *  @param capacity Numero maximo de modos a copiar.
 *  @return Quantidade de modos copiados.
 *  @note Precisa acontecer antes de qualquer 4F01h: a lista pode viver dentro
 *  do proprio buffer de transferencia, que a consulta de modo sobrescreve.
 */
static uint32_t vbe_collect_modes(uint16_t *modes, uint32_t capacity) {
    const volatile uint16_t *list;
    uint32_t count = 0;

    memcpy((void *)REALMODE_BUFFER_ADDR, "VBE2", 4);
    if (!vbe_call(VBE_GET_CONTROLLER_INFO, 0))
        return 0;

    list = (const volatile uint16_t *)far_ptr_to_linear(buffer_u32(VBE_INFO_MODE_PTR));
    if (!list)
        return 0;

    while (count < capacity && list[count] != VBE_MODE_LIST_END)
        count++;

    for (uint32_t i = 0; i < count; i++)
        modes[i] = list[i];

    return count;
}

/** Pontuacao de um modo candidato; quanto maior, melhor.
 *  @param mode_bpp Bits por pixel do modo.
 *  @param wanted_bpp Bits por pixel pedidos pelo chamador.
 *  @return 2 para casamento exato, 1 para um bpp direto aceitavel, 0 para descarte.
 */
static int vbe_score_bpp(uint8_t mode_bpp, uint8_t wanted_bpp) {
    if (mode_bpp == wanted_bpp)
        return 2;
    if (mode_bpp == 32 || mode_bpp == 24 || mode_bpp == 16 || mode_bpp == 15)
        return 1;
    return 0;
}

bool video_set_vbe(uint32_t width, uint32_t height, uint8_t bpp) {
    uint16_t modes[VBE_MAX_MODES];
    uint32_t mode_count;
    realmode_regs_t regs;
    uint16_t best_mode = 0;
    int best_score = 0;
    video_mode_info_t candidate;

    if (!realmode_is_ready() || !vbe_available)
        return false;

    mode_count = vbe_collect_modes(modes, VBE_MAX_MODES);
    if (mode_count == 0)
        return false;

    memset(&candidate, 0, sizeof(candidate));

    for (uint32_t i = 0; i < mode_count; i++) {
        uint16_t attributes;
        uint8_t  memory_model;
        uint8_t  mode_bpp;
        int score;

        if (!vbe_call(VBE_GET_MODE_INFO, modes[i]))
            continue;

        attributes = buffer_u16(VBE_MODE_ATTRIBUTES);
        if ((attributes & VBE_ATTR_SUPPORTED) == 0)
            continue;
        if ((attributes & VBE_ATTR_GRAPHICS) == 0)
            continue;
        /* Sem linear framebuffer sobraria so o acesso por janelas de 64 KB,
         * que a ponte grafica nao implementa — melhor descartar o modo. */
        if ((attributes & VBE_ATTR_LINEAR_FB) == 0)
            continue;

        if (buffer_u16(VBE_MODE_WIDTH) != width || buffer_u16(VBE_MODE_HEIGHT) != height)
            continue;

        memory_model = buffer_u8(VBE_MODE_MEMORY_MODEL);
        if (memory_model != VBE_MEMORY_PACKED && memory_model != VBE_MEMORY_DIRECT)
            continue;

        mode_bpp = buffer_u8(VBE_MODE_BPP);
        score = vbe_score_bpp(mode_bpp, bpp);
        if (score <= best_score)
            continue;

        best_score = score;
        best_mode = modes[i];

        candidate.kind = VIDEO_KIND_VBE_LFB;
        candidate.framebuffer = buffer_u32(VBE_MODE_PHYS_BASE);
        candidate.width = width;
        candidate.height = height;
        candidate.pitch = buffer_u16(VBE_MODE_PITCH);
        candidate.bpp = mode_bpp;
        candidate.red_size    = buffer_u8(VBE_MODE_RED_SIZE);
        candidate.red_pos     = buffer_u8(VBE_MODE_RED_POS);
        candidate.green_size  = buffer_u8(VBE_MODE_GREEN_SIZE);
        candidate.green_pos   = buffer_u8(VBE_MODE_GREEN_POS);
        candidate.blue_size   = buffer_u8(VBE_MODE_BLUE_SIZE);
        candidate.blue_pos    = buffer_u8(VBE_MODE_BLUE_POS);
        candidate.vbe_mode    = modes[i];
    }

    if (best_score == 0 || candidate.framebuffer == 0)
        return false;

    memset(&regs, 0, sizeof(regs));
    regs.eax = VBE_SET_MODE;
    regs.ebx = (uint32_t)(best_mode | VBE_MODE_LINEAR_FB);

    if (!realmode_int(BIOS_VIDEO_INT, &regs))
        return false;
    if ((regs.eax & 0xFFFFu) != VBE_SUPPORTED)
        return false;

    current_mode = candidate;
    return true;
}
