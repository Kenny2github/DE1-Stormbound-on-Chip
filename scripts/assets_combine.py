import re
from pathlib import Path

with open(Path('include') / 'assets.h', 'w') as hfile:
    print("""
#ifndef ASSETS_H
#define ASSETS_H

#include "image_data.h"
""".lstrip(), file=hfile)

    for file in Path('assets').glob('*'):
        # assets/xyz.123!1.png => xyz_123_1(_data)
        filename = file.name
        var_name = re.sub(r'[^a-zA-Z0-9_]', '_', filename.rsplit('.', 1)[0])
        data_var_name = var_name + '_data'
        print(f"""
// {filename}
extern struct image {var_name};
extern uint16_t {data_var_name}[];
""".lstrip(), file=hfile)

    print('#endif', file=hfile)
