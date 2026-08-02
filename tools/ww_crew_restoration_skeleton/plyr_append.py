#!/usr/bin/env python3
"""§296: grow R_DL01's PLYR receptor to carry the donor re-entrance points —
the donor's setNextStage arg stays VERBATIM; the receptor accommodates.

THE GAP (History's §295/§296 receipt): ba1's donor cut fires
setNextStage(host, 0xC8/0xCA) — donor args verbatim — but R_DL01's
R00_00.arc/room.dzr PLYR defines only point [0] -> playerInit fatal.

THE NATIVE CORRECTION (zero invented data): donor LinkRM Room0's own PLYR
carries the re-entrance points — id 200 (angle.z 0xFFC8, pos -289,375,83 =
the loft) and id 202 (0xFFCA, same loft) — and the receptor's existing entry
(id 0, pos -255,0,1125) MATCHES donor PLYR[0]'s position, proving the host
room runs donor-local coordinates. So the fix is: append the donor's id-200
and id-202 entries BYTE-VERBATIM (full 0x20 records incl. the angle.z flag
bits) into the receptor's PLYR chunk.

OFFSET-STABLE GROW (P1 law): the PLYR chunk (self-contained) relocates to EOF
with its old + new entries; ONLY its chunk-table {count, offset} fields change;
every other byte of room.dzr keeps its original offset — any pointered chunks
stay valid by construction. Old PLYR region becomes dead bytes (harmless).

Idempotent; backup .pre-plyr-bak; verification battery before write.

Usage: plyr_append.py [--dry-run]
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from adapt_bdl_arcs import be32
from bake_room_chunks import read_dzr

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "R00_00.arc"
DONOR_ROOM = Path(r"D:/XXXXXXX/Ex WW/files/res/Stage/LinkRM/Room0.arc")
WANT_IDS = [200, 202]          # donor 0xC8 / 0xCA re-entrance points
ENTRY = 0x20


def plyr_chunk(data):
    n = be32(data, 0)
    for i in range(n):
        if data[4 + i * 0xC: 8 + i * 0xC] == b"PLYR":
            return i, be32(data, 8 + i * 0xC), be32(data, 12 + i * 0xC)
    return None, 0, 0


def entry_id(e: bytes) -> int:
    return struct.unpack(">h", e[0x1C:0x1E])[0] & 0xFF


def main() -> int:
    dry = "--dry-run" in sys.argv
    members, dzr = read_dzr(TARGET)
    dzr = bytearray(dzr)
    idx, cnt, off = plyr_chunk(dzr)
    assert idx is not None, "receptor has no PLYR chunk"
    have = {entry_id(bytes(dzr[off + k * ENTRY: off + (k + 1) * ENTRY]))
            for k in range(cnt)}
    todo = [i for i in WANT_IDS if i not in have]
    print(f"receptor PLYR: {cnt} entries, ids {sorted(have)}; to add: {todo}")
    if not todo:
        print("all donor points present (idempotent) — done")
        return 0

    _, ddzr = read_dzr(DONOR_ROOM)
    didx, dcnt, doff = plyr_chunk(ddzr)
    donor_entries = {}
    for k in range(dcnt):
        e = bytes(ddzr[doff + k * ENTRY: doff + (k + 1) * ENTRY])
        donor_entries[entry_id(e)] = e
    missing = [i for i in todo if i not in donor_entries]
    assert not missing, f"donor lacks points {missing} — wrong donor room?"

    old_entries = bytes(dzr[off: off + cnt * ENTRY])
    add = b"".join(donor_entries[i] for i in todo)
    out = bytearray(dzr)
    while len(out) % 4:
        out.append(0)
    new_off = len(out)
    out += old_entries + add
    struct.pack_into(">II", out, 8 + idx * 0xC, cnt + len(todo), new_off)

    # ---- verification battery -------------------------------------------
    # (a) untouched: everything except the 8 table bytes + appended tail
    ta = 8 + idx * 0xC
    assert bytes(out[:ta]) == bytes(dzr[:ta])
    assert bytes(out[ta + 8: len(dzr)]) == bytes(dzr[ta + 8:]), \
        "untouched region changed — abort"
    # (b) re-read with the same reader: ids now include the donor points
    i2, c2, o2 = plyr_chunk(out)
    got = []
    for k in range(c2):
        e = bytes(out[o2 + k * ENTRY: o2 + (k + 1) * ENTRY])
        x, y, z = struct.unpack(">3f", e[0xC:0x18])
        az = struct.unpack(">h", e[0x1C:0x1E])[0]
        got.append((entry_id(e), f"az={az:#06x}", f"({x:.0f},{y:.0f},{z:.0f})"))
    print(f"verify PLYR ({c2} entries): {got}")
    assert {g0[0] for g0 in got} >= set(WANT_IDS)
    # (c) donor byte-verbatim check on the appended entries
    for i in todo:
        assert donor_entries[i] in bytes(out), "appended entry not verbatim"
    if dry:
        print("dry-run — nothing written")
        return 0
    dzr_name = next(n for n, _b in members if n.lower().endswith(".dzr"))
    files = [(n, b) for n, b in members if n != dzr_name]
    files.append((dzr_name, bytes(out)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-plyr-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
