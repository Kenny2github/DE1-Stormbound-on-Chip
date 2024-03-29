/**
 * Data for poisoned.png
 */
#include "assets.h"

static const uint16_t poisoned_data[] = {
	0x0104, 0x4947, 0x0002, 0x8ab3, 0x0003, 0x4947, 0x0002, 0xdc5f, 0x0021,
	0x4947, 0x0002, 0x8ab3, 0x0003, 0x4947, 0x0002, 0xdc5f, 0x001a, 0x4947,
	0x0002, 0xdc5f, 0x0026, 0x4947, 0x0002, 0xdc5f, 0x007e, 0x4947, 0x0002,
	0xdc5f, 0x001f, 0x4947, 0x0002, 0x8ab3, 0x0005, 0x4947, 0x0002, 0xdc5f,
	0x001f, 0x4947, 0x0002, 0x8ab3, 0x000b, 0x4947, 0x0002, 0x8ab3, 0x0026,
	0x4947, 0x0002, 0x8ab3, 0x0015, 0x4947, 0x0002, 0xdc5f, 0x0026, 0x4947,
	0x0002, 0xdc5f, 0x0033, 0x4947, 0x0002, 0xdc5f, 0x0026, 0x4947, 0x0002,
	0xdc5f, 0x001c, 0x4947, 0x0002, 0x8ab3, 0x0026, 0x4947, 0x0002, 0x8ab3,
	0x000b, 0x4947, 0x0002, 0x8ab3, 0x0026, 0x4947, 0x0002, 0x8ab3, 0x0084,
	0x4947,
};

const struct image poisoned = {
    VGA_RLE, 40, 26, 82, poisoned_data
};
