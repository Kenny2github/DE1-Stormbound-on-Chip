#include <stdint.h>
#include <string.h>
#include "address_map_arm.h"
#include "vga.h"

volatile int pixel_buffer_start;
volatile int char_buffer_start;

void wait_for_vsync(void) {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;
	volatile int* char_ctrl_ptr = (int*)CHAR_BUF_CTRL_BASE;
	// write 1 to buffer
	*pixel_ctrl_ptr = 1;
	*char_ctrl_ptr = 1;
	// wait for S bit to go to 0
	while ((*(pixel_ctrl_ptr + 3) & 1) && (*(char_ctrl_ptr + 3) & 1));
}

void update_back_buffer(void) {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;
	volatile int* char_ctrl_ptr = (int*)CHAR_BUF_CTRL_BASE;
	pixel_buffer_start = *(pixel_ctrl_ptr + 1);
	char_buffer_start = *(char_ctrl_ptr + 1);
}

void plot_pixel(int x, int y, uint16_t color) {
    *(uint16_t *)(pixel_buffer_start + (y << 10) + (x << 1)) = color;
}

void fill_screen(uint16_t color) {
	draw_rectangle(0, 0, SCREEN_W, SCREEN_H, color);
}

void draw_rectangle(int x, int y, int w, int h, uint16_t color) {
	for (int xi = x; xi < x + w; ++xi) {
		for (int yi = y; yi < y + h; ++yi) {
			plot_pixel(xi, yi, color);
		}
	}
}

static void draw_character(int x, int y, char c) {
	*(uint8_t *)(char_buffer_start + (y << 7) + x) = c;
}


void clear_char_screen() {
	for (int x = 0; x < SCREEN_CHAR_W; ++x) {
		for (int y = 0; y < SCREEN_CHAR_H; ++y) {
			draw_character(x, y, 0);
		}
	}
}

void configure_vga(void) {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;
	volatile int* char_ctrl_ptr = (int*)CHAR_BUF_CTRL_BASE;

    /* set front pixel and char buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = FPGA_ONCHIP_BASE;	// first store the address in the
    *(char_ctrl_ptr + 1) = FPGA_CHAR_BASE;		// back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel and char buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
	char_buffer_start = *char_ctrl_ptr;
    fill_screen(BACKGROUND); // pixel_buffer_start points to the pixel buffer
	clear_char_screen();
}

void write_string(int x, int y, const char* str) {
	int len = strlen(str);
	int curr_x = x, curr_y = y;

	for (int i = 0; i < len; ++i) {
		// line break
		if (str[i] == '\n' || curr_x == SCREEN_CHAR_W) {
			curr_x = x;
			if (++curr_y == SCREEN_CHAR_H) return;
			if (str[i] == '\n') continue;
		}
		draw_character(curr_x++, curr_y, str[i]);
	}
}
