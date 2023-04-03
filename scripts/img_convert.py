import sys
import re
from typing import Optional, Literal
from pathlib import Path
from itertools import cycle
from PIL import Image

TRANSPARENT = 0xbffc # RGB #C0FFEE in VGA format

def rle(data: list[int]) -> list[int]:
    result: list[int] = []
    count: int = 0
    last_byte: Optional[int] = None
    for byte in data:
        if byte != last_byte:
            if last_byte is not None:
                result += [count, last_byte]
            count = 0
            last_byte = byte
        count += 1
    if last_byte is not None:
        result += [count, last_byte]
    return result

def rgb_to_vga(rgba: tuple[int, int, int, int]) -> int:
    r, g, b, a = rgba
    if a == 0:
        return TRANSPARENT
    r = int(r / 0xff * 0b11111)
    g = int(g / 0xff * 0b111111)
    b = int(b / 0xff * 0b11111)
    return (r << 11) | (g << 5) | b

with Image.open(sys.argv[1]) as im, open(sys.argv[2], 'w') as cfile:
    im.convert("RGBA")
    data = list(map(rgb_to_vga, im.getdata()))
    data_rle = rle(data)
    mode: Literal["VGA", "VGA_RLE"]
    print('  RLE', len(data_rle), 'vs raw', len(data))
    if len(data_rle) < len(data):
        mode = "VGA_RLE"
        data = data_rle
    else:
        mode = "VGA"

    # assets/xyz.123!1.png => xyz_123_1(_data)
    filename = Path(sys.argv[1]).name
    var_name = re.sub(r'[^a-zA-Z0-9_]', '_', filename.rsplit('.', 1)[0])
    data_var_name = var_name + '_data'

    print(f"""
/**
 * Data for {filename}
 */
#include "assets.h"
#include "image_data.h"

struct image {var_name} = {{ {mode}, {im.width}, {im.height}, {data_var_name} }};
""".lstrip(), file=cfile)
    cfile.write(f'uint16_t {data_var_name} = {{')
    for count, word in zip(cycle(range(9)), data):
        if count == 0:
            cfile.write('\n\t')
        cfile.write(f'{word:#06x}, ')
    print('\n};', file=cfile)
