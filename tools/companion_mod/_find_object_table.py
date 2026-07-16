#!/usr/bin/env python3
import struct
from pathlib import Path

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
rom = ROM.read_bytes()
for v0, sz in [(0x1820000, 0x1D530), (0x1802000, 0x1D3E0), (0x11FB000, 0x1C8C0)]:
    v1 = v0 + sz
    pat = struct.pack(">II", v0, v1)
    idx = rom.find(pat)
    print(hex(v0), "pair at", hex(idx) if idx >= 0 else None)

# object_stk is object id 0x192
OBJECT_STK_ID = 0x192
# scan for aligned table: 8-byte entries
hits = []
for off in range(0, len(rom) - 8 * (OBJECT_STK_ID + 4), 4):
    base = off + OBJECT_STK_ID * 8
    if base + 8 > len(rom):
        continue
    v0, v1 = struct.unpack_from(">II", rom, base)
    if v1 <= v0 or (v1 - v0) < 0x1C000 or (v1 - v0) > 0x22000:
        continue
    # prior entry should look like valid vrom pair too
    p0, p1 = struct.unpack_from(">II", rom, base - 8)
    if p1 > p0 and (p1 - p0) < 0x500000:
        hits.append((off, v0, v1 - v0))
print("table candidates", len(hits))
stk_off = 0x1DCD0
table = stk_off - 0x192 * 8
print("table", hex(table))
for i in [0x190, 0x191, 0x192, 0x193, 0x194]:
    v0, v1 = struct.unpack_from(">II", rom, table + i * 8)
    print(i, hex(v0), hex(v1), hex(v1 - v0))
