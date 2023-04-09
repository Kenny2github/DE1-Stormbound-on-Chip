/**
 * Data for empty_tile.png
 */
#include "assets.h"

static const uint16_t empty_tile_data[] = {
	0x0410, 0xef7d,
};

const struct image empty_tile = {
    VGA_RLE, 40, 26, 2, empty_tile_data
};
