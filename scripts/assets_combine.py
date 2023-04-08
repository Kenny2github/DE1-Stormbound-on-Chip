from pathlib import Path

from var_name import var_names

with open(Path('include') / 'assets.h', 'w') as hfile:
    print("""
#ifndef ASSETS_H
#define ASSETS_H

#include "image_data.h"
""".lstrip(), file=hfile)

    for file in Path('assets').rglob('*'):
        # assets/xyz.123!1.png => xyz_123_1(_data)
        filename = file.name
        var_name, data_var_name = var_names(filename)

        if filename.endswith('.txt'): # handle text files specially
            print(f"""
// {filename}
extern const char {data_var_name}[];
""".lstrip(), file=hfile)
            continue

        if filename.endswith('.md'):
            continue # don't include internal documentation

        print(f"""
// {filename}
extern const struct image {var_name};
""".lstrip(), file=hfile)

    print('#endif', file=hfile)
