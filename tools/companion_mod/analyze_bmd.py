#!/usr/bin/env python3
"""Analyze TP BMDR/BMD payloads from RARC extract."""
import struct
import sys
from pathlib import Path


def hexdump(data: bytes, n=64) -> str:
    return " ".join(f"{b:02x}" for b in data[:n])


def find_magics(data: bytes) -> list[tuple[int, bytes]]:
    magics = [
        b"J3D2",
        b"INF1",
        b"VTX1",
        b"SHP1",
        b"MAT3",
        b"MDL3",
        b"TEX1",
        b"JNT1",
        b"DRW1",
        b"EVP1",
        b"bmd3",
        b"bdl3",
    ]
    hits = []
    for m in magics:
        off = 0
        while True:
            i = data.find(m, off)
            if i < 0:
                break
            hits.append((i, m))
            off = i + 1
    return sorted(hits)


def parse_j3d2_container(data: bytes) -> None:
    if len(data) < 16:
        print("  too small")
        return
    if data[:4] == b"J3D2":
        kind = data[4:8]
        print(f"  J3D2 container kind={kind!r}")
        # J3DModelFileHeader
        block_num = struct.unpack(">I", data[8:12])[0]
        print(f"  block_num={block_num}")
        off = 12
        for i in range(min(block_num, 32)):
            if off + 8 > len(data):
                break
            btype = data[off : off + 4]
            bsize = struct.unpack(">I", data[off + 4 : off + 8])[0]
            print(f"    block[{i}] {btype!r} size=0x{bsize:x} @0x{off:x}")
            off += bsize
        return
    # Maybe raw display-list model (no J3D2 wrapper)
    block_num = struct.unpack(">I", data[0:4])[0]
    if block_num < 64:
        print(f"  possible raw header block_num={block_num}")
        off = 4
        for i in range(min(block_num, 32)):
            if off + 8 > len(data):
                break
            btype = data[off : off + 4]
            bsize = struct.unpack(">I", data[off + 4 : off + 8])[0]
            if bsize < 8 or bsize > len(data):
                print(f"    block[{i}] invalid size {bsize}")
                break
            print(f"    block[{i}] {btype!r} size=0x{bsize:x} @0x{off:x}")
            off += bsize


def main() -> int:
    for path in map(Path, sys.argv[1:]):
        data = path.read_bytes()
        print(f"\n=== {path.name} ({len(data)} bytes) ===")
        print(f"  head: {hexdump(data)}")
        hits = find_magics(data)
        print(f"  magic hits ({len(hits)}):")
        for off, m in hits[:20]:
            print(f"    0x{off:05x} {m.decode('ascii', 'replace')}")
        if len(hits) > 20:
            print(f"    ... +{len(hits)-20} more")
        parse_j3d2_container(data)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
