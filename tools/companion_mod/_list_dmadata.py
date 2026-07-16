#!/usr/bin/env python3
import struct
from pathlib import Path

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
rom = ROM.read_bytes()
off = 0x1A500
entries = []
while off + 16 <= len(rom):
    v0, v1, p0, p1 = struct.unpack_from(">IIII", rom, off)
    if v0 == 0 and v1 == 0 and p0 == 0 and p1 == 0:
        break
    entries.append((v0, v1, p0, p1))
    off += 16
print("entries", len(entries))
for v0, v1, p0, p1 in entries:
    sz = v1 - v0
    if 0x17000 <= sz <= 0x21000:
        print(hex(v0), hex(v1), "size", hex(sz), "p", hex(p0), hex(p1) if p1 else 0)
