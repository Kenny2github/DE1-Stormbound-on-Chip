#include <stdint.h>
#include "address_map_arm.h"
#include "vga.h"

volatile int pixel_buffer_start;

void configure_vga(void) {
	volatile int* pixel_ctrl_ptr = (int*)PIXEL_BUF_CTRL_BASE;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the
                                        // back buffer
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync();
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
    clear_screen(); // pixel_buffer_start points to the pixel buffer
    /* set back pixel buffer to start of SDRAM memory */
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer
    clear_screen(); // pixel_buffer_start points to the pixel buffer
}

/* wait for vsync, then swap front and back buffers */
static void wait_for_vsync(void) {
	volatile int * pixel_ctrl_ptr = (int *)PIXEL_BUF_CTRL_BASE;
	*pixel_ctrl_ptr = 1; // write 1 to buffer
	while (*(pixel_ctrl_ptr + 3) & 1); // wait for S to become 0
}

/* draw line_colored pixel at (x, y) */
void plot_pixel(int x, int y, uint16_t line_color)
{
    *(uint16_t *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

/* reset screen to all black */
void clear_screen(void)
{
	for (int x = 0; x < SCREEN_W; ++x) {
		for (int y = 0; y < SCREEN_H; ++y) {
			plot_pixel(x, y, 0x0);
		}
	}
}