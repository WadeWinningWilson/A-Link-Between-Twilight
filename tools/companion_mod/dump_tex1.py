#!/usr/bin/env python3
import struct
import sys
from pathlib import Path

FMT = {0: "I4", 1: "I8", 2: "IA4", 3: "IA8", 4: "RGB565", 5: "RGB5A3", 6: "RGBA32", 8: "C4", 9: "C8", 10: "C14X2", 14: "CMPR"}
BPP = {0: 0.5, 1: 1, 2: 1, 3: 2, 4: 2, 5: 2, 6: 4, 8: 0.5, 9: 1, 10: 2, 14: 0.5}


def dump(path: Path) -> None:
    b = path.read_bytes()
    n = struct.unpack_from(">I", b, 0xC)[0]
    o = 0x20
    sec = {}
    for _ in range(n):
        tag = b[o : o + 4].decode()
        sz = struct.unpack_from(">I", b, o + 4)[0]
        sec[tag] = (o, sz)
        o += sz
    base, _ = sec["TEX1"]
    nt = struct.unpack_from(">H", b, base + 8)[0]
    hdr = base + struct.unpack_from(">I", b, base + 0xC)[0]
    so = base + struct.unpack_from(">I", b, base + 0x10)[0]
    sc = struct.unpack_from(">H", b, so)[0]
    names = []
    for i in range(sc):
        off = struct.unpack_from(">H", so + 4 + i * 4 + 2)[0]
        e = b.index(0, so + off)
        names.append(b[so + off : e].decode())
    print(path, f"{nt} textures")
    for i in range(nt):
        h = hdr + i * 0x20
        fmt = b[h]
        w = struct.unpack_from(">H", b, h + 2)[0]
        ht = struct.unpack_from(">H", b, h + 4)[0]
        dr = struct.unpack_from(">I", b, h + 0x1C)[0]
        sz = int(w * ht * BPP[fmt])
        nm = names[i] if i < len(names) else "?"
        print(f"  {i}: {nm} {w}x{ht} {FMT.get(fmt, fmt)} {sz}B")


if __name__ == "__main__":
    dump(Path(sys.argv[1]))
