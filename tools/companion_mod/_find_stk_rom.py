#!/usr/bin/env python3
from pathlib import Path

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
rom = bytearray(ROM.read_bytes())


def find_all(data: bytes, needle: bytes, limit=20):
    out = []
    i = 0
    while True:
        j = data.find(needle, i)
        if j < 0:
            return out
        out.append(j)
        if len(out) >= limit:
            return out
        i = j + 1


for label, data in [("raw", bytes(rom)), ("bs16", bytes(rom[i] ^ 0 for i in range(0)))]:
    pass

# 16-bit byte swap (n64 format)
bs = bytearray(len(rom))
for i in range(0, len(rom) - 1, 2):
    bs[i], bs[i + 1] = rom[i + 1], rom[i]

for name, data in [("z64", bytes(rom)), ("swapped", bytes(bs))]:
    print("===", name, "===")
    print("object_stk", data.find(b"object_stk"))
    for off in find_all(data, b"object"):
        print(" object@", hex(off), data[off : off + 32])
