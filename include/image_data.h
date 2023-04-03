/**
 * Definition of image data structures
 */
#ifndef IMAGE_DATA_H
#define IMAGE_DATA_H

#include "stdint.h"

/**** Image data data structures ****/

enum image_encoding {
	VGA,
	VGA_RLE,
};

struct image {
	enum image_encoding encoding;
	int width, height;
	uint16_t* data;
};

#endif
