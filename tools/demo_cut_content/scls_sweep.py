"""SCLS exit-destination sweep: parse the stage-change (exit) tables of every
DZR/DZS across all retail stage arcs and diff destination stage names against
the shipped stage set. Destinations that are not shipped stages = references to
CUT DEV STAGES surviving in room data (found 2026-07-18: T_SP05, T_DEMO,
T_OBJ01, T_TOMI; see docs/TPHistory.md §3.8).

SCLS entry format (TP, empirically verified via hexdump): entries are
VARIABLE-LENGTH — null-terminated stage-name string followed by 5 fixed bytes
(spawn/room/wipe fields). Fixed-stride parses (12/16) frame-shift and produce
fragment noise; do not use them.
Usage: python scls_sweep.py
"""
import os
import struct
import sys
from collections import defaultdict

from dzr_placements import rarc_members, STAGE_ROOT


def main():
    shipped = {d for d in os.listdir(STAGE_ROOT)
               if os.path.isdir(os.path.join(STAGE_ROOT, d))}
    dests = defaultdict(set)
    for stage in sorted(shipped):
        sdir = os.path.join(STAGE_ROOT, stage)
        for fn in sorted(os.listdir(sdir)):
            if not fn.lower().endswith(".arc"):
                continue
            try:
                for mname, blob in rarc_members(os.path.join(sdir, fn)):
                    if not mname.lower().endswith((".dzr", ".dzs")):
                        continue
                    if len(blob) < 4:
                        continue
                    n = struct.unpack_from(">I", blob, 0)[0]
                    if n == 0 or n > 0x200:
                        continue
                    for i in range(n):
                        tag = blob[4 + i * 12:8 + i * 12]
                        num, off = struct.unpack_from(">II", blob, 4 + i * 12 + 4)
                        if tag != b"SCLS":
                            continue
                        p = off
                        for _ in range(num):
                            e = blob.index(b"\0", p)
                            name = blob[p:e].decode("ascii", "replace")
                            p = e + 1 + 5
                            if name:
                                dests[name].add(f"{stage}/{fn}")
            except Exception as ex:
                print(f"parse fail {stage}/{fn}: {ex}", file=sys.stderr)

    orph = {k: v for k, v in dests.items() if k not in shipped}
    print(f"distinct destinations: {len(dests)}; NOT shipped: {len(orph)}")
    for k in sorted(orph):
        locs = sorted(orph[k])
        more = " (+more)" if len(locs) > 10 else ""
        print(f"   {k!r} x{len(locs)} <- {', '.join(locs[:10])}{more}")


if __name__ == "__main__":
    main()
