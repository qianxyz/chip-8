#!/usr/bin/env python

"""Generate rom bytes from raw hex string.

Usage: ./gen.py <path/to/txt>
"""

import os
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('text_file')
args = parser.parse_args()
with open(args.text_file) as f:
    hex_string = f.read()

rom_bytes = bytes.fromhex(hex_string)

dir_name = os.path.dirname(__file__)
rom_name = os.path.join(dir_name, "gen.ch8")
with open(rom_name, "wb") as f:
    f.write(rom_bytes)
