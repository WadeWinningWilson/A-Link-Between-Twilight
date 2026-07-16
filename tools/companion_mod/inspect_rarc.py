#!/usr/bin/env python3
import struct
import sys
from pathlib import Path


def yaz0_decompress(data: bytes) -> bytes:
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


def parse_rarc_tree(data: bytes):
    info = struct.unpack(">I", data[8:12])[0]
    file_data_off = struct.unpack(">I", data[12:16])[0]
    num_nodes = struct.unpack(">I", data[info : info + 4])[0]
    node_off = struct.unpack(">I", data[info + 4 : info + 8])[0]
    num_files = struct.unpack(">I", data[info + 8 : info + 12])[0]
    file_off = struct.unpack(">I", data[info + 12 : info + 16])[0]
    str_off = struct.unpack(">I", data[info + 20 : info + 24])[0]
    nodes_base = info + node_off
    files_base = info + file_off
    str_base = info + str_off
    archive_data = info + file_data_off
    nodes = []
    for i in range(num_nodes):
        o = nodes_base + i * 0x10
        typ, name_off, field8, num_ent, first_idx = struct.unpack(">IIHHI", data[o : o + 0x10])
        ns = data.find(b"\x00", str_base + name_off)
        name = data[str_base + name_off : ns].decode("ascii", "replace")
        nodes.append((typ, name, num_ent, first_idx))
    files = []
    for i in range(num_files):
        o = files_base + i * 0x14
        file_id, _hash, type_name, data_off, data_size, _extra = struct.unpack(
            ">HHIIII", data[o : o + 0x14]
        )
        flags = type_name >> 24
        name_off = type_name & 0xFFFFFF
        ns = data.find(b"\x00", str_base + name_off)
        name = data[str_base + name_off : ns].decode("ascii", "replace")
        payload = data[archive_data + data_off : archive_data + data_off + data_size]
        files.append(
            dict(
                idx=i,
                id=file_id,
                flags=flags,
                name=name,
                off=data_off,
                size=data_size,
                payload=payload,
            )
        )
    return nodes, files


def main() -> int:
    arc_path = Path(sys.argv[1])
    raw = arc_path.read_bytes()
    rarc = yaz0_decompress(raw) if raw[:4] == b"Yaz0" else raw
    nodes, files = parse_rarc_tree(rarc)
    print("===", arc_path)
    for typ, name, num_ent, first_idx in nodes:
        print(" node", typ.to_bytes(4, "big"), name, "entries", num_ent, "first", first_idx)
    for f in files:
        if f["size"] < 500:
            continue
        j = f["payload"].find(b"J3D2")
        print(
            f"  id=0x{f['id']:04x} name={f['name']!s} sz={f['size']} j3d={j} head={f['payload'][:4]!r}",
            flush=True,
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
