"""Orphan placement sweep: parse every DZR/DZS chunk table across all retail
stage arcs, collect every 8-char actor name in ACTR/SCOB/TGOB/TGSC/DOOR-family
chunks (layer-aware: ACT0-b / SCO0-b), and diff against l_objectName in
src/d/d_stage.cpp. A placement name absent from the spawn table is a GHOST
PLACEMENT — evidence of an actor removed after rooms were authored.
Usage: python dzr_orphans.py
"""
import os
import re
import struct
import sys
from collections import defaultdict

from dzr_placements import rarc_members, STAGE_ROOT

D_STAGE = os.path.join(os.path.dirname(__file__), "..", "..", "src", "d", "d_stage.cpp")

# ============================================================
# Chunk families that carry an 8-byte name at entry offset 0.
# Entry sizes per TP dStage layout.
# ============================================================
NAME_CHUNKS = {}
for fam, size in (("ACTR", 32), ("SCOB", 36), ("TRES", 32), ("Door", 48)):
    NAME_CHUNKS[fam] = size
    for c in "0123456789abcde":
        NAME_CHUNKS[fam[:3] + c] = size
NAME_CHUNKS["TGOB"] = 32
NAME_CHUNKS["TGSC"] = 36
NAME_CHUNKS["DOOR"] = 48
NAME_CHUNKS["TGDR"] = 48

NAME_RE = re.compile(rb"^[ -~]{1,8}$")


def table_names():
    src = open(D_STAGE, encoding="utf-8", errors="replace").read()
    return set(re.findall(r'OBJNAME\("([^"]+)"', src))


def scan_dzx(blob, sink, noise):
    if len(blob) < 4:
        return
    count = struct.unpack_from(">I", blob, 0)[0]
    if count == 0 or count > 0x200:
        return
    hdr = 4
    if len(blob) < hdr + count * 12:
        return
    for i in range(count):
        tag = blob[hdr + i * 12:hdr + i * 12 + 4]
        num, off = struct.unpack_from(">II", blob, hdr + i * 12 + 4)
        try:
            t = tag.decode("ascii")
        except UnicodeDecodeError:
            continue
        esz = NAME_CHUNKS.get(t)
        if esz is None:
            noise[t] += num
            continue
        for j in range(num):
            base = off + j * esz
            if base + 8 > len(blob):
                break
            raw = blob[base:base + 8].split(b"\0", 1)[0]
            if raw and NAME_RE.match(raw):
                sink[(raw.decode("ascii"), t)] += 1


def main():
    known = table_names()
    print(f"spawn table: {len(known)} names from d_stage.cpp")
    placed = defaultdict(int)
    where = defaultdict(set)
    unknown_tags = defaultdict(int)
    arcs = 0
    for stage in sorted(os.listdir(STAGE_ROOT)):
        sdir = os.path.join(STAGE_ROOT, stage)
        if not os.path.isdir(sdir):
            continue
        for fn in sorted(os.listdir(sdir)):
            if not fn.lower().endswith(".arc"):
                continue
            try:
                for mname, blob in rarc_members(os.path.join(sdir, fn)):
                    if not mname.lower().endswith((".dzr", ".dzs")):
                        continue
                    arcs += 1
                    local = defaultdict(int)
                    scan_dzx(blob, local, unknown_tags)
                    for key, c in local.items():
                        placed[key] += c
                        where[key].add(f"{stage}/{fn}")
            except Exception as ex:
                print(f"parse fail {stage}/{fn}: {ex}", file=sys.stderr)

    known_lc = {k.lower() for k in known}
    orphans = {k: c for k, c in placed.items() if k[0] not in known}
    print(f"\nscanned {arcs} dzr/dzs members; {len(placed)} distinct (name,chunk) keys")
    print(f"\n== ORPHANS (placed but NOT in l_objectName): {len(orphans)}")
    for key in sorted(orphans, key=lambda k: -orphans[k]):
        n, tag = key
        locs = sorted(where[key])
        shown = ", ".join(locs[:4]) + (f" (+{len(locs)-4} more)" if len(locs) > 4 else "")
        case = "  [case-match in table]" if n.lower() in known_lc else ""
        print(f"   {n!r} [{tag}] x{orphans[key]:<4} @ {shown}{case}")
    print(f"\n== chunk tags skipped (no name field parsed): "
          f"{dict(sorted(unknown_tags.items()))}")


if __name__ == "__main__":
    main()
