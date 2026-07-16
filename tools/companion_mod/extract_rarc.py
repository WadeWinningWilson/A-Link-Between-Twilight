#!/usr/bin/env python3
"""Extract files from Yaz0-compressed RARC (TP/GC object arcs)."""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path


def yaz0_decompress(data: bytes) -> bytes:
    if data[:4] != b"Yaz0":
        raise ValueError("not Yaz0")
    dec_size = struct.unpack(">I", data[4:8])[0]
    out = bytearray(dec_size)
    src, dst = 16, 0
    while dst < dec_size:
        code = data[src]
        src += 1
        for bit in range(8):
            if dst >= dec_size:
                break
            if code & (0x80 >> bit):
                out[dst] = data[src]
                dst += 1
                src += 1
            else:
                b1, b2 = data[src], data[src + 1]
                src += 2
                dist = ((b1 & 0xF) << 8) | b2
                length = (b1 >> 4) + 2
                offset = dst - dist - 1
                for i in range(length):
                    if dst >= dec_size:
                        break
                    out[dst] = out[offset + i]
                    dst += 1
    return bytes(out)


def parse_rarc(data: bytes) -> list[tuple[int, str, int, int, bytes]]:
    if data[:4] != b"RARC":
        raise ValueError("not RARC")
    header_len = struct.unpack(">I", data[8:12])[0]
    file_data_off = struct.unpack(">I", data[12:16])[0]
    info = header_len
    num_nodes, node_off, num_files, file_off, _str_len, str_off = struct.unpack(
        ">IIIIII", data[info : info + 24]
    )
    del num_nodes  # unused for flat listing
    files_base = info + file_off
    str_base = info + str_off
    archive_data = header_len + file_data_off

    entries: list[tuple[int, str, int, int, bytes]] = []
    for i in range(num_files):
        o = files_base + i * 0x14
        file_id, _hash, type_name, data_off, data_size, _extra = struct.unpack(
            ">HHIIII", data[o : o + 0x14]
        )
        flags = type_name >> 24
        name_off = type_name & 0xFFFFFF
        if flags & 0x02:
            continue  # directory
        ns = data.find(b"\x00", str_base + name_off)
        name = data[str_base + name_off : ns].decode("ascii", "replace")
        payload = data[archive_data + data_off : archive_data + data_off + data_size]
        entries.append((file_id, name, data_off, data_size, payload))
    return entries


def pack_rarc(entries: list[tuple[int, str, bytes]], source_rarc: bytes) -> bytes:
    """Repack RARC preserving structure; replace file payloads by file_id."""
    data = bytearray(source_rarc)
    header_len = struct.unpack(">I", data[8:12])[0]
    file_data_off = struct.unpack(">I", data[12:16])[0]
    info = header_len
    num_files = struct.unpack(">I", data[info + 8 : info + 12])[0]
    file_off = struct.unpack(">I", data[info + 12 : info + 16])[0]
    files_base = info + file_off
    archive_data = header_len + file_data_off

    replace = {fid: payload for fid, _name, payload in entries}
    new_blob = bytearray()
    file_layout: list[tuple[int, int, int]] = []
    for i in range(num_files):
        o = files_base + i * 0x14
        file_id, _hash, type_name, data_off, data_size, _extra = struct.unpack(
            ">HHIIII", data[o : o + 0x14]
        )
        flags = type_name >> 24
        if flags & 0x02:
            continue
        payload = replace.get(file_id, data[archive_data + data_off : archive_data + data_off + data_size])
        aligned = (len(new_blob) + 31) & ~31
        new_blob.extend(b"\x00" * (aligned - len(new_blob)))
        off = len(new_blob)
        new_blob.extend(payload)
        file_layout.append((o, off, len(payload)))

    for fo, new_off, new_size in file_layout:
        struct.pack_into(">I", data, fo + 8, new_off)
        struct.pack_into(">I", data, fo + 12, new_size)

    aligned = (len(new_blob) + 31) & ~31
    new_blob.extend(b"\x00" * (aligned - len(new_blob)))
    old_archive = archive_data
    tail = data[old_archive + len(new_blob) :] if False else b""

    out = bytearray(data[:archive_data])
    out.extend(new_blob)
    struct.pack_into(">I", out, 4, len(out))
    struct.pack_into(">I", out, 16, len(new_blob))
    return bytes(out)


def yaz0_compress_best_effort(data: bytes) -> bytes:
    """Wrap uncompressed RARC in Yaz0 (store mode chunks only — fine for mod-sized arcs)."""
    import io

    out = io.BytesIO()
    out.write(b"Yaz0")
    out.write(struct.pack(">I", len(data)))
    out.write(b"\x00" * 8)
    src = 0
    while src < len(data):
        block_start = out.tell()
        out.write(b"\x00")  # placeholder code byte
        code_pos = block_start
        codes = 0
        code_bits = 0
        chunk = min(8, (len(data) - src + 7) // 8)
        for _ in range(min(8, len(data) - src)):
            out.write(bytes([data[src]]))
            codes = (codes << 1) | 1
            code_bits += 1
            src += 1
            if src >= len(data):
                break
        while code_bits < 8:
            codes <<= 1
            code_bits += 1
        code_byte = (codes & 0xFF).to_bytes(1, "big")
        out.seek(code_pos)
        out.write(code_byte)
        out.seek(0, 2)
    return out.getvalue()


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("arc")
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--id", type=lambda x: int(x, 0), action="append", default=[])
    args = ap.parse_args()

    raw = Path(args.arc).read_bytes()
    rarc = yaz0_decompress(raw) if raw[:4] == b"Yaz0" else raw
    entries = parse_rarc(rarc)
    args.out.mkdir(parents=True, exist_ok=True)
    for file_id, name, _off, size, payload in entries:
        if args.id and file_id not in args.id:
            continue
        out_name = f"{file_id:04x}_{name}" if name else f"{file_id:04x}.bin"
        (args.out / out_name).write_bytes(payload)
        print(f"0x{file_id:04x} {name} ({size} bytes)")
    return 0


if __name__ == "__main__":
    sys.exit(main())
