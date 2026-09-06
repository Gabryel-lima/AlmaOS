#include <stddef.h>
#include <stdint.h>

#include "gfx_raster.h"

/** Define a cor de um pixel no framebuffer.
 *  @param fb Ponteiro para o framebuffer.
 *  @param x Coordenada x do pixel.
 *  @param y Coordenada y do pixel.
 *  @param rgba Cor do pixel no formato RGBA.
 */
void gfx_fb_set_pixel(Framebuffer *fb, int x, int y, uint32_t rgba) {
	size_t stride;

	if (!fb || !fb->pixels || x < 0 || y < 0) {
		return;
	}

	if (fb->width == 0 || fb->height == 0 || (unsigned int)x >= fb->width || (unsigned int)y >= fb->height) {
		return;
	}

	if ((fb->pitch % sizeof(uint32_t)) != 0) {
		return;
	}

	stride = (size_t)fb->pitch / sizeof(uint32_t);
	if (stride < (size_t)fb->width) {
		return;
	}

	fb->pixels[(size_t)y * stride + (size_t)x] = rgba;
}

void gfx_fb_clear(Framebuffer *fb, uint32_t rgba) {
	size_t stride;
	size_t count;

	if (!fb || !fb->pixels || fb->width == 0 || fb->height == 0) return;
	if ((fb->pitch % sizeof(uint32_t)) != 0) return;

	stride = (size_t)fb->pitch / sizeof(uint32_t);
	if (stride < (size_t)fb->width) return;

	count = stride * (size_t)fb->height;
	for (size_t i = 0; i < count; ++i) fb->pixels[i] = rgba;
}
