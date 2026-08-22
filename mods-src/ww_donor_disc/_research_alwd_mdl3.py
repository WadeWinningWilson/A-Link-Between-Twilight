"""Extract Alwd.bdl from Lwood.arc and decode MDL3 SharedDL packets."""
from __future__ import annotations

import struct
from pathlib import Path

OUT = Path(r"C:\Users\xxxxx\Documents\dusklight\mods-src\ww_donor_disc\_research")
OUT.mkdir(exist_ok=True)


def yaz0_decompress(data: bytes) -> bytes:
    assert data[:4] == b"Yaz0"
    out_size = struct.unpack(">I", data[4:8])[0]
    src = memoryview(data)[16:]
    dst = bytearray()
    i = 0
    while len(dst) < out_size:
        code = src[i]
        i += 1
        for bit in range(8):
            if len(dst) >= out_size:
                break
            if code & (0x80 >> bit):
                dst.append(src[i])
                i += 1
            else:
                b1, b2 = src[i], src[i + 1]
                i += 2
                dist = ((b1 & 0xF) << 8) | b2
                copy_src = len(dst) - (dist + 1)
                n = b1 >> 4
                if n == 0:
                    n = src[i] + 0x12
                    i += 1
                else:
                    n += 2
                for _ in range(n):
                    dst.append(dst[copy_src])
                    copy_src += 1
    return bytes(dst)


def load_arc(path: Path) -> bytes:
    raw = path.read_bytes()
    if raw[:4] == b"Yaz0":
        return yaz0_decompress(raw)
    return raw


def extract_bdl4(data: bytes, name: str) -> Path:
    off = data.find(b"J3D2bdl4")
    assert off >= 0, "no bdl4"
    size = struct.unpack_from(">I", data, off + 8)[0]
    out = OUT / name
    out.write_bytes(data[off : off + size])
    return out


def walk_chunks(bdl: bytes):
    assert bdl[:8] == b"J3D2bdl4"
    nblocks = struct.unpack_from(">I", bdl, 0x0C)[0]
    off = 0x20
    for _ in range(nblocks):
        magic = bdl[off : off + 4]
        size = struct.unpack_from(">I", bdl, off + 4)[0]
        yield magic.decode("ascii", "replace"), off, size, bdl[off : off + size]
        off += size


def decode_fifo(packet: bytes, label: str) -> list[str]:
    lines = [f"=== {label} size={len(packet)} ==="]
    i = 0
    n_bp = n_xf = n_other = n_pad = 0
    while i < len(packet):
        op = packet[i]
        if op == 0x00:
            # padding / NOP
            run = 1
            while i + run < len(packet) and packet[i + run] == 0:
                run += 1
            lines.append(f"  @{i:04X} NOP/pad x{run}")
            n_pad += run
            i += run
            continue
        if op == 0x61:  # BP
            if i + 5 > len(packet):
                lines.append(f"  @{i:04X} BP TRUNCATED")
                break
            reg = packet[i + 1]
            val = (packet[i + 2] << 16) | (packet[i + 3] << 8) | packet[i + 4]
            lines.append(f"  @{i:04X} BP reg=0x{reg:02X} val=0x{val:06X}")
            n_bp += 1
            i += 5
            continue
        if op == 0x10:  # XF
            if i + 5 > len(packet):
                lines.append(f"  @{i:04X} XF TRUNCATED")
                break
            n_minus1 = struct.unpack_from(">H", packet, i + 1)[0]
            addr = struct.unpack_from(">H", packet, i + 3)[0]
            nbytes = (n_minus1 + 1) * 4
            if i + 5 + nbytes > len(packet):
                lines.append(
                    f"  @{i:04X} XF addr=0x{addr:04X} n={n_minus1+1} TRUNCATED need {nbytes}"
                )
                break
            lines.append(f"  @{i:04X} XF addr=0x{addr:04X} words={n_minus1+1}")
            n_xf += 1
            i += 5 + nbytes
            continue
        # draw / other — report and stop detailed walk with hex dump
        dump = packet[i : i + 16].hex(" ")
        lines.append(f"  @{i:04X} OTHER op=0x{op:02X} next16={dump}")
        n_other += 1
        i += 1
        if n_other > 8:
            lines.append("  ... aborting after many OTHER")
            break
    lines.append(f"summary bp={n_bp} xf={n_xf} other={n_other} pad={n_pad}")
    return lines


def parse_mdl3(chunk: bytes, base_off: int) -> list[str]:
    lines = []
    assert chunk[:4] == b"MDL3"
    mat_num = struct.unpack_from(">H", chunk, 0x08)[0]
    dl_init_off = struct.unpack_from(">I", chunk, 0x0C)[0]
    lines.append(f"MDL3 mats={mat_num} mpDisplayListInit=0x{dl_init_off:X}")
    # J3DDisplayListInit: offset + size, relative to EACH entry
    for mi in range(mat_num):
        entry = dl_init_off + mi * 8
        rel = struct.unpack_from(">I", chunk, entry)[0]
        size = struct.unpack_from(">I", chunk, entry + 4)[0]
        # ptr = entry_addr + rel  (as in createLockedMaterial)
        abs_off = entry + rel
        lines.append(
            f"mat{mi}: entry@{entry:X} rel=0x{rel:X} size={size} abs=0x{abs_off:X}"
        )
        if abs_off + size <= len(chunk):
            pkt = chunk[abs_off : abs_off + size]
            (OUT / f"alwd_mat{mi}_dl.bin").write_bytes(pkt)
            lines.extend(decode_fifo(pkt, f"mat{mi}"))
        else:
            lines.append(f"  OOB abs+size={abs_off+size} chunk={len(chunk)}")
    return lines


def main():
    arc = Path(r"D:\XXXXXXX\WW DP\orig\GZLE01\files\res\Object\Lwood.arc")
    data = load_arc(arc)
    bdl_path = extract_bdl4(data, "alwd.bdl")
    bdl = bdl_path.read_bytes()
    report = [f"file={bdl_path} size={len(bdl)}"]
    for magic, off, size, chunk in walk_chunks(bdl):
        report.append(f"chunk {magic} @{off:X} size={size}")
        if magic == "MDL3":
            report.extend(parse_mdl3(chunk, off))
        if magic == "SHP1":
            # shape display list data offset
            shp_num = struct.unpack_from(">H", chunk, 0x08)[0]
            report.append(f"  SHP1 shapes={shp_num}")
    out_txt = OUT / "alwd_mdl3_decode.txt"
    out_txt.write_text("\n".join(report), encoding="utf-8")
    print(out_txt.read_text(encoding="utf-8"))


if __name__ == "__main__":
    main()
