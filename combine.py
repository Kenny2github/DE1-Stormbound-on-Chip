import sys
import os
import re
from pathlib import Path

INCLUDE_RE = re.compile(r'^#include "([^"]+)"', re.M)
INCLUDE_DIR = Path('include')
SRC_DIR = Path('src')

def read_file(match: re.Match[str]) -> str:
    h_filename = match.group(1)
    print(f'{Path(filename).name}: Expanding {h_filename}')
    with open(INCLUDE_DIR / h_filename) as h_file:
        return h_file.read()

with open("combined.c", "a") as combined_file:
    for filename in sys.argv[1:]:
        with open(filename, "r") as c_file:
            contents = c_file.read()
        while re.search(INCLUDE_RE, contents):
            contents = INCLUDE_RE.sub(read_file, contents)
        combined_file.write(contents)

os.system('type combined.c | clip')
