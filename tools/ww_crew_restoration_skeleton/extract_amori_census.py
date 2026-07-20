#!/usr/bin/env python3
"""Extract A_mori room.dzr ACTR-family placements into interior_placements.csv.

WW DZR layout: u32 chunkHdrCount; then count×{tag[4], entryCount, dataOff};
entries at dataOff. ACTR/ACTn = 0x20, SCOB/SCOn = 0x24.
"""
from __future__ import annotations

import os
import struct
import sys
from collections import Counter
from pathlib import Path

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration"
)
ARC = MOD / "arcs" / "A_mori.arc"
CSV = MOD / "population" / "interior_placements.csv"
STAGE = "A_mori"


def be32(d: bytes, o: int) -> int:
    return struct.unpack_from(">I", d, o)[0]


def list_rarc(data: bytes):
    assert data[:4] == b"RARC"
    data_abs = 0x20 + be32(data, 0x0C)
    info = 0x20
    n = be32(data, info + 0x08)
    ent = info + be32(data, info + 0x0C)
    strs = info + be32(data, info + 0x14)
    out = []
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", data, e + 4)[0] & 0x1100 != 0x1100:
            continue
        name_off = struct.unpack_from(">H", data, e + 6)[0]
        end = data.index(b"\0", strs + name_off)
        name = data[strs + name_off : end].decode("ascii", "replace")
        doff, size = be32(data, e + 8), be32(data, e + 12)
        out.append((name, data_abs + doff, size))
    return out


def entry_size(tag: str) -> int | None:
    if tag.startswith("ACT"):
        return 0x20
    if tag.startswith("SCO"):
        return 0x24
    if tag in ("TGOB", "TGSC", "TGDR", "Door", "DOOR", "TRES", "PLYR"):
        return 0x24 if tag in ("TGSC", "TGDR", "SCOB", "Door", "DOOR") else 0x20
    return None


def parse_dzr(blob: bytes):
    hdr_n = be32(blob, 0)
    rows = []
    for i in range(hdr_n):
        o = 4 + i * 12
        tag = blob[o : o + 4].decode("ascii", "replace")
        count = be32(blob, o + 4)
        data_off = be32(blob, o + 8)
        es = entry_size(tag)
        if es is None or count <= 0:
            continue
        layer = "-"
        if len(tag) == 4 and tag[3] in "0123456789ABCDEF" and tag[:3] in ("ACT", "SCO"):
            layer = tag[3]
        for k in range(count):
            e = data_off + k * es
            if e + es > len(blob):
                break
            name = blob[e : e + 8].split(b"\0", 1)[0].decode("ascii", "replace").strip()
            if not name:
                continue
            params = be32(blob, e + 8)
            x, y, z = struct.unpack_from(">fff", blob, e + 0xC)
            rx, ry, rz = struct.unpack_from(">hhh", blob, e + 0x18)
            rows.append((tag, layer, name, params, x, y, z, rx, ry, rz))
    return rows


def main() -> int:
    if not ARC.is_file():
        print("missing", ARC, file=sys.stderr)
        return 1
    data = ARC.read_bytes()
    dzr = None
    for name, off, size in list_rarc(data):
        if name == "room.dzr":
            dzr = data[off : off + size]
            break
    if dzr is None:
        print("no room.dzr", file=sys.stderr)
        return 1
    rows = parse_dzr(dzr)
    counts = Counter(r[2] for r in rows)
    print("rows", len(rows))
    print("name counts:", sorted(counts.items(), key=lambda kv: (-kv[1], kv[0])))
    for key in ("mo2", "Bk", "gmos", "kuro_t", "Zl1", "P1a", "P1b"):
        print(key, counts.get(key, 0))

    text = CSV.read_text(encoding="utf-8", errors="replace") if CSV.is_file() else ""
    # Drop prior A_mori block if re-running.
    lines = [ln for ln in text.splitlines() if not ln.startswith("A_mori,")]
    if not lines or not lines[0].startswith("stage,"):
        lines = ["stage,chunk,layer,name,params_hex,x,y,z,rx,ry,rz"] + [
            ln for ln in lines if ln and not ln.startswith("stage,")
        ]
    for chunk, layer, name, params, x, y, z, rx, ry, rz in rows:
        lines.append(
            f"{STAGE},{chunk},{layer},{name},{params:08x},{x:.1f},{y:.1f},{z:.1f},{rx},{ry},{rz}"
        )
    CSV.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {len(rows)} A_mori rows -> {CSV}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
