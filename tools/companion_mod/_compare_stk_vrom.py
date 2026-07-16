#!/usr/bin/env python3
import hashlib
from pathlib import Path

from mm_extract_object_stk import DMADATA_OFF, ROM, load_object_stk_blob, parse_dmadata

rom = ROM.read_bytes()
entries = parse_dmadata(rom, DMADATA_OFF)
for vrom in [0x11FB000, 0x1802000, 0x1820000]:
    b = load_object_stk_blob(rom, entries, vrom)
    raw = b[0x123D8 : 0x123D8 + 512]
    print(hex(vrom), "len", len(b), "skin md5", hashlib.md5(raw).hexdigest()[:12])
