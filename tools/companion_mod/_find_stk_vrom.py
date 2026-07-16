#!/usr/bin/env python3
import struct
from pathlib import Path

from mm_extract_object_stk import yaz0_decompress, rgba16_to_rgba, parse_dmadata, DMADATA_OFF

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
rom = ROM.read_bytes()
entries = parse_dmadata(rom, DMADATA_OFF)
cache = {}

def get_blob(e):
    p0, p1 = e["p0"], e["p1"]
    if p0 == 0xFFFFFFFF:
        return None
    size = e["v1"] - e["v0"]
    if p1 == 0:
        return rom[p0 : p0 + size]
    if p0 not in cache:
        try:
            cache[p0] = yaz0_decompress(rom[p0:p1])
        except Exception:
            return None
    return cache[p0][:size]

SKIN_OFF = 0x123D8
EYE_OFF = 0x1A5A0

for e in entries:
    sz = e["v1"] - e["v0"]
    if sz < 0x1C000 or sz > 0x21000:
        continue
    blob = get_blob(e)
    if not blob or len(blob) < 0x1A700:
        continue
    skin = blob[SKIN_OFF : SKIN_OFF + 512]
    eye = blob[EYE_OFF : EYE_OFF + 128]
    if len(set(skin)) < 40:
        continue
    if len(set(eye)) < 8:
        continue
    print(
        "MATCH",
        hex(e["v0"]),
        "size",
        hex(sz),
        "p0",
        hex(e["p0"]),
        "skin uniq",
        len(set(skin)),
        "eye uniq",
        len(set(eye)),
    )
