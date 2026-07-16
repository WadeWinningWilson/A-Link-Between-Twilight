#!/usr/bin/env python3
"""Restore in-game-exported BMD bytes to on-disk BE layout for SuperBMD.

Dusklight's PC J3D loader byte-swaps VTX1 attr tables and vertex arrays in the
mpRawData buffer. SuperBMD expects vanilla big-endian file layout.
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

GX_VA_NULL = 0xFF
ATTR_ORDER = [
    ("pos", 0x0C),
    ("nrm", 0x10),
    ("nbt", 0x14),
    ("c0", 0x18),
    ("c1", 0x1C),
    ("t0", 0x20),
    ("t1", 0x24),
    ("t2", 0x28),
    ("t3", 0x2C),
    ("t4", 0x30),
    ("t5", 0x34),
    ("t6", 0x38),
    ("t7", 0x3C),
]


def u32be(b: bytes, off: int) -> int:
    return struct.unpack(">I", b[off : off + 4])[0]


def u32le(b: bytes, off: int) -> int:
    return struct.unpack("<I", b[off : off + 4])[0]


def write_u32be(buf: bytearray, off: int, val: int) -> None:
    buf[off : off + 4] = struct.pack(">I", val & 0xFFFFFFFF)


def bswap_u32(v: int) -> int:
    v &= 0xFFFFFFFF
    return struct.unpack(">I", struct.pack("<I", v))[0]


def iter_j3d2_blocks(data: bytes):
    if data[:4] != b"J3D2":
        raise ValueError("not J3D2")
    off = 0x20
    end = len(data)
    while off + 8 <= end:
        btype = data[off : off + 4]
        bsize = u32be(data, off + 4)
        if bsize < 8 or off + bsize > end:
            break
        yield off, btype, bsize
        off += bsize


SUPERBMD_ATTRS = frozenset({9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20})


def rebuild_vtx_attr_fmt_list(buf: bytearray, vtx_off: int, list_off: int) -> list[tuple[int, int, int, int]]:
    """Rewrite ArrayFormat table as BE u32s + 00ffffff tails; return parsed formats."""
    pos = list_off
    parsed: list[tuple[int, int, int, int]] = []
    while pos + 16 <= len(buf):
        array_type = u32le(buf, pos)
        if array_type == 0 or array_type == GX_VA_NULL:
            break
        if array_type not in SUPERBMD_ATTRS:
            break
        parsed.append(
            (
                array_type,
                u32le(buf, pos + 4),
                u32le(buf, pos + 8),
                buf[pos + 12],
            )
        )
        pos += 16

    # fmt table ends where the first vertex array begins.
    ptr_offs = [u32be(buf, vtx_off + rel) for _, rel in ATTR_ORDER if u32be(buf, vtx_off + rel) != 0]
    table_end = min(ptr_offs) if ptr_offs else (u32be(buf, vtx_off + 4) - (vtx_off - list_off))
    table_bytes = max(0, table_end - (list_off - vtx_off))
    if table_bytes <= 0:
        table_bytes = 0x40

    out = bytearray(table_bytes)
    write_pos = 0
    for array_type, cnt, typ, frac in parsed:
        write_u32be(out, write_pos, array_type)
        write_u32be(out, write_pos + 4, cnt)
        write_u32be(out, write_pos + 8, typ)
        out[write_pos + 12 : write_pos + 16] = bytes([frac & 0xFF, 0xFF, 0xFF, 0xFF])
        write_pos += 16
    if write_pos + 4 <= len(out):
        write_u32be(out, write_pos, GX_VA_NULL)

    buf[list_off : list_off + len(out)] = out
    return parsed


def comp_stride(attr_name: str, cnt: int, typ: int) -> int:
    if attr_name.startswith("c"):
        if typ in (0, 5):  # RGB565, RGBA4
            return 2
        return 1
    if typ == 4:  # GX_F32
        return 4
    if typ in (2, 3):  # u16/s16
        return 2
    return 1


def fmt_for_attr(buf: bytearray, list_off: int, attr_id: int) -> tuple[int, int, int] | None:
    pos = list_off
    while pos + 16 <= len(buf):
        a = u32be(buf, pos)
        if a == GX_VA_NULL or a == 0:
            return None
        if a == attr_id:
            c = u32be(buf, pos + 4)
            t = u32be(buf, pos + 8)
            f = buf[pos + 12]
            return c, t, f
        pos += 16
    return None


def unfix_array(buf: bytearray, start: int, end: int, stride: int) -> None:
    """Host floats/ints -> big-endian file layout."""
    if start <= 0 or end <= start:
        return
    if stride == 1:
        return
    if stride == 2:
        pos = start
        while pos + 2 <= end:
            host_u16_to_be_bytes(buf, pos)
            pos += 2
        return
    if stride == 3:
        pos = start
        while pos + 3 <= end:
            b0, b1, b2 = buf[pos : pos + 3]
            buf[pos : pos + 3] = bytes([b2, b1, b0])
            pos += 3
        return
    pos = start
    while pos + 4 <= end:
        host_u32_to_be_bytes(buf, pos)
        pos += 4


def attr_id_for_name(name: str) -> int | None:
    mapping = {
        "pos": 9,
        "nrm": 10,
        "nbt": 25,
        "c0": 11,
        "c1": 12,
    }
    if name in mapping:
        return mapping[name]
    if name.startswith("t") and name[1:].isdigit():
        return 13 + int(name[1:])
    return None


def host_u32_to_be_bytes(buf: bytearray, off: int) -> None:
    """PC loader stores ArrayFormat u32s as little-endian — rewrite as big-endian."""
    write_u32be(buf, off, u32le(buf, off))


def unfix_vtx1(buf: bytearray, vtx_off: int) -> None:
    fmt_rel = u32be(buf, vtx_off + 8)
    list_off = vtx_off + fmt_rel

    formats = rebuild_vtx_attr_fmt_list(buf, vtx_off, list_off)

    ptrs: dict[str, int] = {}
    for name, rel_off in ATTR_ORDER:
        rel = u32be(buf, vtx_off + rel_off)
        if rel == 0:
            continue
        ptrs[name] = vtx_off + rel

    ordered = [n for n, _ in ATTR_ORDER if n in ptrs]

    def data_end(name: str) -> int:
        idx = ordered.index(name)
        for later in ordered[idx + 1 :]:
            return ptrs[later]
        return vtx_off + u32be(buf, vtx_off + 4)

    for name in ordered:
        aid = attr_id_for_name(name)
        if aid is None:
            continue
        fmt_tuple = next((f for f in formats if f[0] == aid), None)
        if fmt_tuple is None:
            continue
        _a, cnt, typ, _frac = fmt_tuple
        start = ptrs[name]
        end = data_end(name)
        stride = comp_stride(name, cnt, typ)
        unfix_array(buf, start, end, stride)


def host_u16_to_be_bytes(buf: bytearray, off: int) -> None:
    struct.pack_into(">H", buf, off, struct.unpack("<H", buf[off : off + 2])[0])


def host_f32_to_be_bytes(buf: bytearray, off: int) -> None:
    host_u32_to_be_bytes(buf, off)


def unfix_jnt1(buf: bytearray, off: int) -> None:
    joint_num = struct.unpack(">H", buf[off + 8 : off + 10])[0]
    init_rel = u32be(buf, off + 0x0C)
    idx_rel = u32be(buf, off + 0x10)
    init_base = off + init_rel
    idx_base = off + idx_rel
    for i in range(joint_num):
        idx_off = idx_base + i * 2
        host_u16_to_be_bytes(buf, idx_off)
        idx = struct.unpack(">H", buf[idx_off : idx_off + 2])[0]
        joff = init_base + idx * 0x30
        host_u16_to_be_bytes(buf, joff)  # mKind
        for f in range(3):
            host_f32_to_be_bytes(buf, joff + 0x04 + f * 4)  # scale
        for f in range(3):
            host_u16_to_be_bytes(buf, joff + 0x10 + f * 2)  # rotation
        for f in range(3):
            host_f32_to_be_bytes(buf, joff + 0x18 + f * 4)  # translate
        host_f32_to_be_bytes(buf, joff + 0x24)  # radius
        for f in range(3):
            host_f32_to_be_bytes(buf, joff + 0x28 + f * 4)  # min
        # mMax shares tail of struct — only min fits in 0x30; skip if OOB


def unfix_vtx_desc_list(buf: bytearray, list_off: int, region_end: int) -> None:
    pos = list_off
    while pos + 8 <= region_end and pos + 8 <= len(buf):
        host_u32_to_be_bytes(buf, pos)
        host_u32_to_be_bytes(buf, pos + 4)
        if struct.unpack(">I", buf[pos : pos + 4])[0] == GX_VA_NULL:
            break
        pos += 8


def unfix_shp1(buf: bytearray, off: int) -> None:
    shape_num = struct.unpack(">H", buf[off + 8 : off + 10])[0]
    init_rel = u32be(buf, off + 0x0C)
    idx_rel = u32be(buf, off + 0x10)
    vtx_rel = u32be(buf, off + 0x18)
    mtx_rel = u32be(buf, off + 0x1C)
    vtx_base = off + vtx_rel
    vtx_region_end = off + mtx_rel if mtx_rel > vtx_rel else vtx_base + 0x80

    list_starts: set[int] = set()
    if init_rel and idx_rel and shape_num:
        init_base = off + init_rel
        idx_base = off + idx_rel
        for i in range(shape_num):
            shape_idx = struct.unpack(">H", buf[idx_base + i * 2 : idx_base + i * 2 + 2])[0]
            shape_off = init_base + shape_idx * 0x28
            if shape_off + 6 <= len(buf):
                desc_idx = struct.unpack(">H", buf[shape_off + 4 : shape_off + 6])[0]
                list_starts.add(vtx_base + desc_idx)

    if not list_starts:
        list_starts.add(vtx_base)

    for list_off in sorted(list_starts):
        if list_off < vtx_base or list_off >= vtx_region_end:
            continue
        unfix_vtx_desc_list(buf, list_off, vtx_region_end)


def unfix_bmd(data: bytes) -> bytes:
    buf = bytearray(data)
    for off, btype, _size in iter_j3d2_blocks(buf):
        if btype == b"VTX1":
            unfix_vtx1(buf, off)
        elif btype == b"JNT1":
            unfix_jnt1(buf, off)
        elif btype == b"SHP1":
            unfix_shp1(buf, off)
    return bytes(buf)


def main() -> int:
    if len(sys.argv) < 3:
        print(f"usage: {sys.argv[0]} input.bmd output.bmd", file=sys.stderr)
        return 2
    src = Path(sys.argv[1])
    dst = Path(sys.argv[2])
    dst.write_bytes(unfix_bmd(src.read_bytes()))
    print(f"wrote {dst} ({dst.stat().st_size} bytes)")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
