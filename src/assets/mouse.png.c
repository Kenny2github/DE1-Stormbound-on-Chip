/**
 * Data for mouse.png
 */
#include "assets.h"

static const uint16_t mouse_data[] = {
	0x0005, 0x00c7, 0x0002, 0xffff, 0x0002, 0x00c7, 0x0002, 0xffff, 0x0005,
	0x00c7,
};

const struct image mouse = {
    VGA_RLE, 4, 4, 10, mouse_data
};
