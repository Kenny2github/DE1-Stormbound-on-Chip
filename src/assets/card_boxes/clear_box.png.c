/**
 * Data for clear_box.png
 */
#include "assets.h"

static const uint16_t clear_box_data[] = {
	0x0002, 0x4947, 0x004c, 0x00c7, 0x0003, 0x4947, 0x004e, 0x00c7, 0x0001,
	0x4947, 0x0280, 0x00c7, 0x0001, 0x4947, 0x004e, 0x00c7, 0x0003, 0x4947,
	0x004c, 0x00c7, 0x0002, 0x4947,
};

const struct image clear_box = {
    VGA_RLE, 80, 12, 22, clear_box_data
};
