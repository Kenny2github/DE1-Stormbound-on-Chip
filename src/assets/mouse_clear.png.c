/**
 * Data for mouse_clear.png
 */
#include "assets.h"

static const uint16_t mouse_clear_data[] = {
	0x0010, 0x00c7,
};

const struct image mouse_clear = {
    VGA_RLE, 4, 4, 2, mouse_clear_data
};
