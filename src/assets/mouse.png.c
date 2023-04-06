/**
 * Data for mouse.png
 */
#include "assets.h"

static uint16_t mouse_data[] = {
	0x0004, 0xffff,
};

struct image mouse = { VGA_RLE, 2, 2, 2, mouse_data };
