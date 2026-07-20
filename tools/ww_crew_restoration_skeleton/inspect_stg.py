#!/usr/bin/env python3
"""Inspect STG_00.arc RARC members + stage.dzs / roomN.dzs chunk headers."""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path


def yaz0_decompress(data: bytes) -> bytes:
    assert data[:4] == b"Yaz0"
    size = struct.unpack(">I", data[4:8])[0]
    src = memoryview(data)[16:]
    dst = bytearray()
    i = 0
    while len(dst) < size:
        code = src[i]
        i += 1
        for bit in range(8):
            if len(dst) >= size:
                break
            if code & (0x80 >> bit):
                dst.append(src[i])
                i += 1
            else:
                b1 = src[i]
                b2 = src[i + 1]
                i += 2
                dist = ((b1 & 0x0F) << 8) | b2
                copy = b1 >> 4
                if copy == 0:
                    copy = src[i] + 0x12
                    i += 1
                else:
                    copy += 2
                for _ in range(copy):
                    dst.append(dst[-dist - 1])
    return bytes(dst)


def list_rarc(path: Path):
    data = path.read_bytes()
    if data[:4] == b"Yaz0":
        data = yaz0_decompress(data)
    assert data[:4] == b"RARC"
    _file_size, _header_size, data_offset, _data_length = struct.unpack(">IIII", data[4:20])
    num_nodes, node_off, num_files, file_off, str_off = struct.unpack(">IIIII", data[0x20:0x34])
    base = 0x20

    def sname(off: int) -> str:
        sn = base + str_off + off
        end = data.index(0, sn)
        return data[sn:end].decode("ascii", "replace")

    nodes = []
    for i in range(num_nodes):
        o = base + node_off + i * 0x10
        name_off, _hash, file_count, first = struct.unpack(">IHHI", data[o + 4 : o + 16])
        nodes.append((sname(name_off), file_count, first))

    files = []
    for i in range(num_files):
        o = base + file_off + i * 0x14
        fid, _h = struct.unpack(">HH", data[o : o + 4])
        type_name = struct.unpack(">I", data[o + 4 : o + 8])[0]
        doff, dsize = struct.unpack(">II", data[o + 8 : o + 16])
        name_off = type_name & 0x00FFFFFF
        ftype = (type_name >> 24) & 0xFF
        files.append((sname(name_off), ftype, doff, dsize, fid))
    return data, files, nodes, data_offset


def parse_dzs(blob: bytes, label: str) -> None:
    if len(blob) < 4:
        return
    n = struct.unpack(">I", blob[0:4])[0]
    print(f"  {label}: chunks={n} size={len(blob)}")
    for i in range(min(n, 64)):
        o = 4 + i * 12
        tag = blob[o : o + 4].decode("ascii", "replace")
        ent, off = struct.unpack(">II", blob[o + 4 : o + 12])
        print(f"    {tag} n={ent} off={off:#x}")


def main() -> int:
    paths = sys.argv[1:]
    if not paths:
        app = Path(os.environ["APPDATA"])
        paths = [
            str(
                app
                / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration/files/res/Stage/R_DL01/STG_00.arc"
            ),
            r"D:\XXXXXXX\Ex TP\files\res\Stage\R_SP01\STG_00.arc",
            r"D:\XXXXXXX\Ex TP\files\res\Stage\R_SP300\STG_00.arc",
        ]
    for p in paths:
        path = Path(p)
        data, files, nodes, data_off = list_rarc(path)
        print(f"==== {path.name} ({path.parent.name}) size={len(data)} data_off={data_off:#x}")
        print("nodes", nodes)
        for name, ftype, doff, dsize, _fid in files:
            if name in (".", ".."):
                continue
            print(f"  {name:20} type={ftype:02x} off={doff:08x} size={dsize}")
            if name.endswith(".dzs") and dsize > 0:
                blob = data[data_off + doff : data_off + doff + dsize]
                parse_dzs(blob, name)
        print()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
