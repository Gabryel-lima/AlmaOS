#include "include/cmd_video.h"
#include "include/video.h"
#include "include/gfx_bridge.h"
#include "include/kalloc.h"
#include "include/keyboard.h"
#include "include/serial.h"
#include "include/vga.h"
#include "include/pit.h"
#include "include/string.h"

#include "gfx_raster.h"

/** Nome legivel de uma familia de modo de video.
 *  @param kind Familia do modo.
 *  @return String constante descrevendo o modo.
 */
static const char *video_kind_name(video_kind_t kind) {
    switch (kind) {
    case VIDEO_KIND_TEXT:    return "texto";
    case VIDEO_KIND_VGA13:   return "VGA 13h";
    case VIDEO_KIND_VBE_LFB: return "VBE (framebuffer linear)";
    default:                 return "desconhecido";
    }
}

/** Le um inteiro decimal a partir de `*cursor`, avancando o cursor.
 *  @param cursor Ponteiro para a posicao de leitura; e avancado.
 *  @param out Recebe o valor lido.
 *  @return true se havia pelo menos um digito.
 */
static bool parse_uint(const char **cursor, uint32_t *out) {
    const char *p = *cursor;
    uint32_t value = 0;
    bool any = false;

    while (*p >= '0' && *p <= '9') {
        value = value * 10u + (uint32_t)(*p - '0');
        p++;
        any = true;
    }

    *cursor = p;
    *out = value;
    return any;
}

/** Interpreta uma especificacao de modo grafico.
 *
 *  Formatos aceitos: `vga13`, `<largura>x<altura>` e `<largura>x<altura>x<bpp>`.
 *
 *  @param spec Texto a interpretar.
 *  @param width Recebe a largura; 0 para `vga13`.
 *  @param height Recebe a altura; 0 para `vga13`.
 *  @param bpp Recebe os bits por pixel; 32 e o padrao quando omitido.
 *  @return true se o texto foi reconhecido.
 */
static bool parse_mode_spec(const char *spec, uint32_t *width, uint32_t *height, uint32_t *bpp) {
    const char *cursor = spec;

    if (strcmp(spec, "vga13") == 0) {
        *width = 0;
        *height = 0;
        *bpp = 8;
        return true;
    }

    if (!parse_uint(&cursor, width))
        return false;
    if (*cursor != 'x')
        return false;
    cursor++;
    if (!parse_uint(&cursor, height))
        return false;

    if (*cursor == '\0') {
        *bpp = 32;
        return true;
    }
    if (*cursor != 'x')
        return false;
    cursor++;
    if (!parse_uint(&cursor, bpp))
        return false;

    return *cursor == '\0';
}

/** Descreve o modo ativo na saida do console. */
static void print_current_mode(void) {
    const video_mode_info_t *mode = video_current();

    vga_printf("Modo atual: %s\n", video_kind_name(mode->kind));
    vga_printf("  framebuffer: 0x%08x  %ux%u  pitch=%u  bpp=%u\n",
               mode->framebuffer, mode->width, mode->height, mode->pitch, mode->bpp);

    if (mode->kind == VIDEO_KIND_VBE_LFB) {
        vga_printf("  modo VBE 0x%03x  R:%u@%u G:%u@%u B:%u@%u\n",
                   mode->vbe_mode,
                   mode->red_size, mode->red_pos,
                   mode->green_size, mode->green_pos,
                   mode->blue_size, mode->blue_pos);
    }

    if (video_vbe_available())
        vga_printf("VBE %u.%u disponivel\n",
                   (video_vbe_version() >> 8) & 0xFF, video_vbe_version() & 0xFF);
    else
        vga_puts("VBE indisponivel nesta maquina\n");
}

/** Ativa o modo descrito por `spec`.
 *  @param spec Especificacao ja validada por `parse_mode_spec`.
 *  @return true se a BIOS aceitou a troca.
 */
static bool apply_mode(const char *spec) {
    uint32_t width;
    uint32_t height;
    uint32_t bpp;

    if (strcmp(spec, "text") == 0)
        return video_set_text();

    if (!parse_mode_spec(spec, &width, &height, &bpp))
        return false;

    if (width == 0)
        return video_set_vga13();

    return video_set_vbe(width, height, (uint8_t)bpp);
}

int cmd_video_mode(int argc, char **argv) {
    if (argc < 2) {
        print_current_mode();
        vga_puts("Uso: mode <text|vga13|LARGURAxALTURA[xBPP]>\n");
        return 0;
    }

    if (!apply_mode(argv[1])) {
        vga_printf("mode: nao foi possivel ativar '%s'\n", argv[1]);
        return 1;
    }

    /* A partir daqui, se o modo for grafico, a tela de texto sumiu; o log
     * continua saindo pela COM1 (veja serial.h). */
    print_current_mode();
    return 0;
}

/* ---------------------------------------------------------------------------
 * gfxdemo: cubo girando desenhado pelo rasterizador do gfx.
 * ------------------------------------------------------------------------- */

/** Seno via x87.
 *  @param radians Angulo em radianos.
 *  @return Seno do angulo.
 *  @note O kernel nao tem libm; `fsin` do proprio 387 resolve sem trazer uma
 *  aproximacao polinomial so para a demo. Requer `fpu_init()`.
 */
static float kernel_sinf(float radians) {
    float result;
    __asm__ volatile("fsin" : "=t"(result) : "0"(radians));
    return result;
}

/** Cosseno via x87.
 *  @param radians Angulo em radianos.
 *  @return Cosseno do angulo.
 */
static float kernel_cosf(float radians) {
    float result;
    __asm__ volatile("fcos" : "=t"(result) : "0"(radians));
    return result;
}

/** Os 8 vertices de um cubo centrado na origem, com aresta 1. */
static const Vec3 cube_vertices[8] = {
    { -0.5f, -0.5f, -0.5f }, {  0.5f, -0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f }, { -0.5f,  0.5f, -0.5f },
    { -0.5f, -0.5f,  0.5f }, {  0.5f, -0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f }, { -0.5f,  0.5f,  0.5f },
};

/** As 12 faces triangulares do cubo, como indices em `cube_vertices`. */
static const uint8_t cube_triangles[12][3] = {
    { 0, 1, 2 }, { 0, 2, 3 },   /* tras   */
    { 5, 4, 7 }, { 5, 7, 6 },   /* frente */
    { 4, 0, 3 }, { 4, 3, 7 },   /* esquerda */
    { 1, 5, 6 }, { 1, 6, 2 },   /* direita  */
    { 3, 2, 6 }, { 3, 6, 7 },   /* topo   */
    { 4, 5, 1 }, { 4, 1, 0 },   /* base   */
};

/** Uma cor por par de triangulos, para que cada face do cubo se distinga. */
static const Vec3 cube_face_colors[6] = {
    { 0.90f, 0.20f, 0.20f },
    { 0.20f, 0.80f, 0.30f },
    { 0.25f, 0.45f, 0.95f },
    { 0.95f, 0.80f, 0.15f },
    { 0.85f, 0.30f, 0.85f },
    { 0.20f, 0.85f, 0.85f },
};

/** Distancia da camera ao centro do cubo, em unidades de mundo. */
#define GFXDEMO_CAMERA_DISTANCE 3.0f

/** Projeta um vertice do cubo em coordenadas de tela.
 *  @param vertex Vertice em espaco de modelo.
 *  @param angle Angulo de rotacao em radianos.
 *  @param width Largura do framebuffer em pixels.
 *  @param height Altura do framebuffer em pixels.
 *  @return Vertice projetado; `.x`/`.y` em pixels e `.z` com a profundidade
 *          em espaco de visao, que e o que o z-buffer do gfx compara.
 */
static Vec4 project_vertex(Vec3 vertex, float angle, uint32_t width, uint32_t height) {
    float sin_a = kernel_sinf(angle);
    float cos_a = kernel_cosf(angle);
    float sin_b = kernel_sinf(angle * 0.7f);
    float cos_b = kernel_cosf(angle * 0.7f);
    Vec4 out;

    /* Rotacao em Y, depois em X. */
    float x1 =  vertex.x * cos_a + vertex.z * sin_a;
    float z1 = -vertex.x * sin_a + vertex.z * cos_a;
    float y2 =  vertex.y * cos_b - z1 * sin_b;
    float z2 =  vertex.y * sin_b + z1 * cos_b;

    float view_z = z2 + GFXDEMO_CAMERA_DISTANCE;
    /* O cubo tem raio menor que a distancia da camera, entao view_z nunca
     * chega a zero; a guarda existe para o caso de alguem mexer nas constantes. */
    float scale = (view_z > 0.001f) ? (0.8f * (float)height / view_z) : 0.0f;

    out.x = (float)width  * 0.5f + x1 * scale;
    out.y = (float)height * 0.5f - y2 * scale;
    out.z = view_z;
    out.w = 1.0f;
    return out;
}

/** Desenha um frame do cubo no backbuffer da ponte.
 *  @param angle Angulo de rotacao em radianos.
 */
static void gfxdemo_draw_frame(float angle) {
    Framebuffer *framebuffer = gfx_bridge_framebuffer();
    float *depth = gfx_bridge_depth_buffer();
    Vec4 projected[8];

    if (!framebuffer || !depth)
        return;

    gfx_fb_clear(framebuffer, 0x101830FFu);
    gfx_bridge_clear_depth();

    for (unsigned i = 0; i < 8; i++)
        projected[i] = project_vertex(cube_vertices[i], angle,
                                      framebuffer->width, framebuffer->height);

    for (unsigned t = 0; t < 12; t++) {
        Vec3 color = cube_face_colors[t / 2];
        gfx_rasterize_triangle(framebuffer, depth,
                               projected[cube_triangles[t][0]],
                               projected[cube_triangles[t][1]],
                               projected[cube_triangles[t][2]],
                               color, color, color);
    }

    gfx_bridge_present();
}

/** Angulo do quadro fixo do modo `still`.
 *
 *  O valor nao importa em si; o que importa e ser sempre o mesmo, para que
 *  uma verificacao automatizada possa comparar contra numeros conhecidos.
 *  Nesta orientacao tres faces do cubo ficam visiveis ao mesmo tempo, que e
 *  justamente o que prova que o z-buffer esta discriminando profundidade.
 */
#define GFXDEMO_STILL_ANGLE 0.9f

/** Le um pixel de volta do framebuffer do hardware.
 *  @param row Inicio da linha no framebuffer.
 *  @param x Coluna do pixel.
 *  @param bpp Bits por pixel do modo ativo.
 *  @return Valor bruto do pixel, ja montado a partir dos bytes do hardware.
 */
static uint32_t read_hardware_pixel(const uint8_t *row, uint32_t x, uint8_t bpp) {
    switch (bpp) {
    case 8:  return row[x];
    case 15:
    case 16: return ((const uint16_t *)row)[x];
    case 24: return (uint32_t)row[x * 3 + 0]
                  | ((uint32_t)row[x * 3 + 1] << 8)
                  | ((uint32_t)row[x * 3 + 2] << 16);
    case 32: return ((const uint32_t *)row)[x];
    default: return 0;
    }
}

/** Le de volta o framebuffer do hardware e resume o que foi desenhado.
 *
 *  Le do hardware, e nao do backbuffer, de proposito: assim o resumo cobre a
 *  cadeia inteira — rasterizador do gfx, conversao de cor e escrita no
 *  framebuffer real — em vez de so o que o gfx achou que desenhou. E o que
 *  permite `tools/devboot/smoke.sh` verificar o caminho grafico sem tela.
 *
 *  Reporta a cor de fundo, quantos pixels diferem dela e quantos valores
 *  distintos aparecem — o suficiente para distinguir "desenhou um cubo com
 *  varias faces" de "pintou a tela de uma cor so" ou "nao desenhou nada".
 */
static void gfxdemo_report_framebuffer(void) {
    const video_mode_info_t *mode = video_current();
    const uint8_t *framebuffer = (const uint8_t *)mode->framebuffer;
    uint32_t background;
    uint32_t covered = 0;
    uint32_t distinct = 0;
    uint32_t seen[16];

    if (mode->bpp == 0 || mode->width == 0 || mode->height == 0) {
        vga_puts("gfxdemo: modo sem pixels, resumo indisponivel\n");
        return;
    }

    /* O pixel (0,0) esta fora do cubo, entao serve como cor de fundo. */
    background = read_hardware_pixel(framebuffer, 0, mode->bpp);

    for (uint32_t y = 0; y < mode->height; y++) {
        const uint8_t *row = framebuffer + (size_t)y * mode->pitch;

        for (uint32_t x = 0; x < mode->width; x++) {
            uint32_t pixel = read_hardware_pixel(row, x, mode->bpp);
            uint32_t i;

            if (pixel == background)
                continue;

            covered++;

            for (i = 0; i < distinct; i++) {
                if (seen[i] == pixel)
                    break;
            }
            if (i == distinct && distinct < 16)
                seen[distinct++] = pixel;
        }
    }

    vga_printf("gfxdemo: fundo=0x%08x cobertos=%u cores=%u\n",
               background, covered, distinct);
}

int cmd_video_gfxdemo(int argc, char **argv) {
    const char *spec = "320x200x32";
    bool still = false;
    float angle = 0.0f;

    /* `still` desenha um unico quadro em angulo fixo, em vez de animar: e o
     * caminho deterministico, para verificacao automatizada. */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "still") == 0)
            still = true;
        else
            spec = argv[i];
    }

    if (!apply_mode(spec)) {
        vga_printf("gfxdemo: nao foi possivel ativar '%s'\n", spec);
        /* Se a troca falhou no meio, garante que a tela de texto volte. */
        video_set_text();
        return 1;
    }

    if (!gfx_bridge_init()) {
        serial_puts("gfxdemo: ponte grafica indisponivel\n");
        video_set_text();
        vga_printf("gfxdemo: ponte grafica indisponivel (kalloc livre=%u bytes)\n",
                   kalloc_capacity() - kalloc_used());
        return 1;
    }

    if (still)
        serial_puts("gfxdemo: quadro fixo\n");
    else
        serial_puts("gfxdemo: renderizando; pressione uma tecla para sair\n");

    if (still) {
        gfxdemo_draw_frame(GFXDEMO_STILL_ANGLE);
        gfxdemo_report_framebuffer();
    } else {
        /* O laco termina na primeira tecla. Sem isso a unica saida seria o
         * reset, ja que em modo grafico o shell fica invisivel. */
        while (!keyboard_has_data() && !serial_has_data()) {
            gfxdemo_draw_frame(angle);
            angle += 0.05f;
        }
    }

    /* Consome a tecla que encerrou o laco para ela nao virar entrada do shell. */
    if (keyboard_has_data())
        (void)keyboard_getchar();
    if (serial_has_data())
        (void)serial_getchar();

    video_set_text();
    vga_clear();
    vga_puts("gfxdemo: encerrado, de volta ao modo texto\n");
    return 0;
}
