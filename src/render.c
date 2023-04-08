/**
 * Routines for optimizing rendering.
 *
 * We manage a stack of data to be rendered, and draw pixels
 * in pop order. Pixels drawn are marked drawn so that they are
 * not drawn again, which wastes time.
 */
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include "render.h"
#include "vga.h"

#define RENDER_STACK_SIZE 128
#define BITFIELD_SIZE (SCREEN_W * SCREEN_H / (sizeof(int) * 8))

/**** Global variables ****/

static volatile size_t r_stack_size = 0;
static volatile struct surface r_stack[RENDER_STACK_SIZE];

/**** Static functions ****/

static inline bool is_plotted(int x, int y, int bitfield[BITFIELD_SIZE]) {
	// pixel index
	int pix_idx = y * SCREEN_H + x;
	// index into bit in bitfield element
	int bit_idx = pix_idx & (sizeof(int) * 8 - 1);
	// index into the bitfield array
	int word_idx = (pix_idx ^ bit_idx) >> 5;
	// return that bit
	return bitfield[word_idx] & (1 << bit_idx);
}

static inline void set_plotted(int x, int y, int bitfield[BITFIELD_SIZE]) {
	// pixel index
	int pix_idx = y * SCREEN_H + x;
	// index into bit in bitfield element
	int bit_idx = pix_idx & (sizeof(int) * 8 - 1);
	// index into the bitfield array
	int word_idx = (pix_idx ^ bit_idx) >> 5;
	// set that bit
	bitfield[word_idx] |= 1 << bit_idx;
}

static void do_draw_RLE_image(int bitfield[BITFIELD_SIZE], struct surface surf) {
	const struct image* img = surf.data;
	int size = img->length;
	int w = 0, h = 0, x = surf.x, y = surf.y;
	for (int i = 0; i < size; i += 2) {
		for (int j = 0; j < img->data[i]; ++j) {
			if (
				// The thing! skip already-plotted pixels
				!is_plotted(x + w, y + h, bitfield)
				// skip transparent pixels
				&& img->data[i + 1] != TRANSPARENT
			) {
				set_plotted(x + w, y + h, bitfield);

				if (0 <= x + w && 0 <= y + h) {
					plot_pixel(x + w, y + h, img->data[i + 1]);
				}
				// other end of bounds guaranteed by
				// code after this if-statement
			}

			if (x + (++w) >= SCREEN_W) {
				// skip pixels beyond right screen bound
				j += img->width - w;
				w = 0;
				++h;
			} else if (w >= img->width) {
				// move to next line
				w = 0;
				++h;
			}
			// skip pixels beyond bottom screen bound
			if (y + h >= SCREEN_H) return;
		}
	}
}

static void do_draw_image(int bitfield[BITFIELD_SIZE], struct surface surf) {
	const struct image* img = surf.data;
	if (img->encoding == VGA_RLE) {
		do_draw_RLE_image(bitfield, surf);
		return;
	}
	uint16_t (*data)[img->width] = (uint16_t (*)[img->width])img->data;
	int x = surf.x, y = surf.y;
	for (int h = y < 0 ? -y : 0; h < img->height; ++h) {
		// stop at bottom edge of screen
		if (y + h >= SCREEN_H) return;

		for (int w = x < 0 ? -x : 0; w < img->width; ++w) {
			// stop at right edge of screen
			if (x + w >= SCREEN_W) break;
			// The thing! skip already-plotted pixels
			if (is_plotted(x + w, y + h, bitfield)) continue;
			// skip transparent pixels
			if (data[h][w] == TRANSPARENT) continue;
			// unavoidably do the plot
			plot_pixel(x + w, y + h, data[h][w]);
			set_plotted(x + w, y + h, bitfield);
		}
	}
}

/**** Exported functions ****/

void render_stack(void) {
	// bitfield of "has this pixel been rendered?"
	int bitfield[BITFIELD_SIZE] = {0};
	while (r_stack_size > 0) {
		// image popped from stack
		struct surface surf = r_stack[--r_stack_size];
		do_draw_image(bitfield, surf);
	}
	wait_for_vsync();
	update_back_buffer();
}

void r_stack_push(struct surface surf) {
	r_stack[r_stack_size++] = surf;
}

void push_image(int x, int y, const struct image* img) {
	struct surface surf = {x, y, img};
	r_stack_push(surf);
}

bool rects_collide(
	int x1, int y1, int w1, int h1,
	int x2, int y2, int w2, int h2
) {
	return (
		x1 < x2 + w2
		&& x2 < x1 + w1
		&& y1 < y2 + h2
		&& y2 < y1 + h1
	);
}
