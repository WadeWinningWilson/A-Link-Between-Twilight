"""Compare Alwd MDL3 SharedDL census vs surviving sea Room44 BDLs."""
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


def walk_chunks(bdl: bytes):
    assert bdl[:8] == b"J3D2bdl4"
    nblocks = struct.unpack_from(">I", bdl, 0x0C)[0]
    off = 0x20
    for _ in range(nblocks):
        magic = bdl[off : off + 4]
        size = struct.unpack_from(">I", bdl, off + 4)[0]
        yield magic.decode("ascii", "replace"), off, size, bdl[off : off + size]
        off += size


def fifo_census(pkt: bytes) -> dict:
    i = 0
    bp = xf = other = pad = drawish = 0
    ops: set[int] = set()
    first_bp: list[str] = []
    while i < len(pkt):
        op = pkt[i]
        ops.add(op)
        if op == 0:
            while i < len(pkt) and pkt[i] == 0:
                i += 1
                pad += 1
            continue
        if op == 0x61:
            if len(first_bp) < 8 and i + 5 <= len(pkt):
                first_bp.append(f"{pkt[i + 1]:02X}")
            bp += 1
            i += 5
            continue
        if op == 0x10:
            n = struct.unpack_from(">H", pkt, i + 1)[0] + 1
            xf += 1
            i += 5 + n * 4
            continue
        if 0x80 <= op <= 0xBF:
            drawish += 1
        other += 1
        i += 1
    return {
        "bp": bp,
        "xf": xf,
        "drawish": drawish,
        "other": other,
        "pad": pad,
        "first_bp": first_bp,
        "ops": [hex(o) for o in sorted(ops)],
    }


def summarize_bdl(bdl: bytes, label: str) -> list[str]:
    lines = [f"=== {label} size={len(bdl)} ==="]
    has_mdl3 = False
    for magic, off, size, chunk in walk_chunks(bdl):
        if magic in ("SHP1", "MAT3", "TEX1", "VTX1", "INF1"):
            extra = ""
            if magic == "SHP1":
                extra = f" shapes={struct.unpack_from('>H', chunk, 0x08)[0]}"
            lines.append(f"{magic} size={size}{extra}")
        if magic != "MDL3":
            continue
        has_mdl3 = True
        mat_num = struct.unpack_from(">H", chunk, 0x08)[0]
        dl_init = struct.unpack_from(">I", chunk, 0x0C)[0]
        lines.append(f"MDL3 mats={mat_num}")
        for mi in range(mat_num):
            entry = dl_init + mi * 8
            rel = struct.unpack_from(">I", chunk, entry)[0]
            sz = struct.unpack_from(">I", chunk, entry + 4)[0]
            abs_off = entry + rel
            pkt = chunk[abs_off : abs_off + sz]
            c = fifo_census(pkt)
            lines.append(
                f"  mat{mi}: dl={sz} bp={c['bp']} xf={c['xf']} "
                f"drawish={c['drawish']} other={c['other']} pad={c['pad']} "
                f"first_bp={c['first_bp']} ops={c['ops']}"
            )
    if not has_mdl3:
        lines.append("MDL3: ABSENT")
    return lines


def main() -> None:
    report: list[str] = []
    alwd = OUT / "alwd.bdl"
    if alwd.exists():
        report.extend(summarize_bdl(alwd.read_bytes(), "alwd.bdl"))
    else:
        report.append("alwd.bdl missing — run _research_alwd_mdl3.py first")

    arc = load_arc(Path(r"D:\XXXXXXX\WW DP\orig\GZLE01\files\res\Stage\sea\Room44.arc"))
    idx = 0
    n = 0
    while True:
        off = arc.find(b"J3D2bdl4", idx)
        if off < 0:
            break
        size = struct.unpack_from(">I", arc, off + 8)[0]
        name = f"room44_bdl{n}.bdl"
        blob = arc[off : off + size]
        (OUT / name).write_bytes(blob)
        report.append("")
        report.extend(summarize_bdl(blob, name))
        n += 1
        idx = off + 4
    report.append(f"\nroom44 bdl count={n}")

    out = OUT / "alwd_vs_room44_mdl3.txt"
    out.write_text("\n".join(report), encoding="utf-8")
    print(out.read_text(encoding="utf-8"))
    print("wrote", out)


if __name__ == "__main__":
    main()
