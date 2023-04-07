/**
 * Data for neutral_box.png
 */
#include "assets.h"

static const uint16_t neutral_box_data[] = {
	0x0002, 0x4947, 0x004c, 0xce99, 0x0003, 0x4947, 0x0002, 0xce99, 0x004a,
	0x8430, 0x0002, 0xce99, 0x0001, 0x4947, 0x0002, 0xce99, 0x0002, 0x8430,
	0x0048, 0x5aeb, 0x0002, 0x8430, 0x0003, 0xce99, 0x0001, 0x8430, 0x004b,
	0x5aeb, 0x0002, 0x8430, 0x0002, 0xce99, 0x0001, 0x8430, 0x004c, 0x5aeb,
	0x0001, 0x8430, 0x0002, 0xce99, 0x0001, 0x8430, 0x004c, 0x5aeb, 0x0001,
	0x8430, 0x0002, 0xce99, 0x0001, 0x8430, 0x004c, 0x5aeb, 0x0001, 0x8430,
	0x0002, 0xce99, 0x0001, 0x8430, 0x004c, 0x5aeb, 0x0001, 0x8430, 0x0002,
	0xce99, 0x0002, 0x8430, 0x004a, 0x5aeb, 0x0002, 0x8430, 0x0003, 0xce99,
	0x0002, 0x8430, 0x0048, 0x5aeb, 0x0002, 0x8430, 0x0002, 0xce99, 0x0001,
	0x4947, 0x0002, 0xce99, 0x004a, 0x8430, 0x0002, 0xce99, 0x0003, 0x4947,
	0x004c, 0xce99, 0x0002, 0x4947,
};

const struct image neutral_box = {
    VGA_RLE, 80, 12, 94, neutral_box_data
};
