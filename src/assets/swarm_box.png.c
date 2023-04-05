/**
 * Data for swarm_box.png
 */
#include "assets.h"

static uint16_t swarm_box_data[] = {
	0x0002, 0x4947, 0x004c, 0x59c4, 0x0003, 0x4947, 0x0002, 0x59c4, 0x004a,
	0x7a86, 0x0002, 0x59c4, 0x0001, 0x4947, 0x0002, 0x59c4, 0x0002, 0x7a86,
	0x0048, 0xbbc9, 0x0002, 0x7a86, 0x0003, 0x59c4, 0x0002, 0x7a86, 0x004a,
	0xbbc9, 0x0002, 0x7a86, 0x0002, 0x59c4, 0x0001, 0x7a86, 0x004c, 0xbbc9,
	0x0001, 0x7a86, 0x0002, 0x59c4, 0x0001, 0x7a86, 0x004c, 0xbbc9, 0x0001,
	0x7a86, 0x0002, 0x59c4, 0x0001, 0x7a86, 0x004c, 0xbbc9, 0x0001, 0x7a86,
	0x0002, 0x59c4, 0x0001, 0x7a86, 0x004c, 0xbbc9, 0x0001, 0x7a86, 0x0002,
	0x59c4, 0x0002, 0x7a86, 0x004a, 0xbbc9, 0x0002, 0x7a86, 0x0003, 0x59c4,
	0x0002, 0x7a86, 0x0048, 0xbbc9, 0x0002, 0x7a86, 0x0002, 0x59c4, 0x0001,
	0x4947, 0x0002, 0x59c4, 0x004a, 0x7a86, 0x0002, 0x59c4, 0x0003, 0x4947,
	0x004c, 0x59c4, 0x0002, 0x4947,
};

struct image swarm_box = { VGA_RLE, 80, 94, swarm_box_data };