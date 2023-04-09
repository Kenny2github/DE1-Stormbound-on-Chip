# DE1-Stormbound-on-Chip
A rudimentary version of [Stormbound](https://paladinstudios.com/stormbound/) implemented on the DE1-SoC.

## Prerequisites
* Python 3
* [Pillow](https://pypi.org/project/Pillow)

## CPUlator
For convenience of use on [CPUlator](https://cpulator.01xz.net/?sys=arm-de1soc), a Python script and Makefile are provided that combine all C files under `src/` into one file and expand quote-`#include`s (i.e. `#include "header.h"` and not `#include <stdio.h>`). The resulting file is suitable for use in CPUlator. On Windows, the contents of the file is placed in the clipboard as well.

## Running on DE1-SoC
To actually run on the DE1-SoC you will need the [Monitor program](https://fpgacademy.org/tools.html). A project file for that purpose is in source control.

## Converting image assets to C files
Run `make assets` to convert `assets/*` to `src/assets/*.c` and produce `include/assets.h`.
