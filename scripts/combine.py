from __future__ import annotations
import sys
import os
import re
from pathlib import Path

INCLUDE_RE = re.compile(r'^#include "([^"]+)"', re.M)
INCLUDE_DIR = Path('include')
SRC_DIR = Path('src')

try:
    with open('headers.txt', 'r') as h_files:
        EXPANDED = set(h_files.read().strip().splitlines())
except FileNotFoundError:
    with open('headers.txt', 'w'):
        EXPANDED = set()

def read_file(match: re.Match[str]) -> str:
    """Read a header file identified by the match."""
    h_filename = match.group(1)
    if h_filename in EXPANDED:
        return ''
    EXPANDED.add(h_filename)
    with open('headers.txt', 'a') as h_files:
        print(h_filename, file=h_files)
    print(f'{Path(filename).name}: Expanding {h_filename}')
    with open(INCLUDE_DIR / h_filename) as h_file:
        # recursively expand headers inside headers
        return INCLUDE_RE.sub(read_file, h_file.read())

with open("combined.c", "a") as combined_file:
    for filename in sys.argv[1:]:
        with open(filename, "r") as c_file:
            contents = c_file.read()
        contents = INCLUDE_RE.sub(read_file, contents)
        # write expanded contents after a file header
        print(f'/******** {Path(filename).name} ********/\n', file=combined_file)
        combined_file.write(contents)
        print(file=combined_file)

# copy file contents to clipboard on Windows
if sys.platform.startswith('win'):
    os.system('type combined.c | clip')
