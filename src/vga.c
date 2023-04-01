#include <stdint.h>
#include "address_map_arm.h"
#include "vga.h"

volatile int pixel_buffer_start;

void wait_for_vsync(void) {
	volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
	*pixel_ctrl_ptr = 1; // write 1 to buffer
	while (*(pixel_ctrl_ptr + 3) & 1); // wait for S to become 0
}

/* draw line_colored pixel at (x, y) */
static void plot_pixel(int x, int y, uint16_t line_color)
{
    *(uint16_t *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

void fill_screen(uint16_t color) {
	for (int x = 0; x < SCREEN_W; ++x) {
		for (int y = 0; y < SCREEN_H; ++y) {
			plot_pixel(x, y, color);
		}
	}
}

void configure_vga(void) {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    fill_screen(BLACK); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    fill_screen(BLACK); // pixel_buffer_start points to the pixel buffer
}

void draw_img_map(int x, int y, int height, int width, int img_map[height][width]) {
	for (int h = 0; h < height; ++h) {
		// stop function if at bottom edge of screen
		if (y + h == SCREEN_H) return;
		for (int w = 0; w < width; ++w) {
			// stop for loop if at right edge of screen
			if (x + w == SCREEN_W) break;
			// if not transparent, fill pixel
			if (img_map[h][w] < 0x10000) {
				plot_pixel(x + w, y + h, img_map[h][w] % 0x10000);
			}
		}
	}
}

void draw_RLE_img_map(int x, int y, int width, int rle_img_map[]) {
	int h = 0, w = 0;
	int size = sizeof(rle_img_map)/sizeof(rle_img_map[0]);
	for (int i = 0; i < size; i += 2) {
		// if not transparent, fill some consecutive pixels
		if (rle_img_map[i + 1] < 0x10000) {
			for (int j = 0; j < rle_img_map[0]; ++j) {
				plot_pixel(x + w, y + h, rle_img_map[i + 1] % 0x10000);
				if (++w == width || x + w == SCREEN_W) {
					// skip pixels if at right edge of screen
					if (x + w == SCREEN_W) j += width - w;

					w = 0;
					++h;

					// stop function if at bottom edge of screen
					if (y + h == SCREEN_H) return;
				}
			}
			
		}
	}
}