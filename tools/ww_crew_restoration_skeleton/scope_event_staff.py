#!/usr/bin/env python3
"""§275: scope hosted WW events to their room — drop the `ALL` staff
(host-context translation; pitfall-A/B hardening from ww-interior-host-pitfalls).

WHY THIS IS A TRANSLATION, NOT AN EDIT OF DONOR INTENT
------------------------------------------------------
Donor TALE_DEMO/TALE_DEMO2 carry a TYPE_ALL staff (cuts = ['dummy']) — in the
donor's ONE-ROOM LinkRM it is inert scaffolding. In the 6-room R_DL01 host,
an all-scope staff is what flips event mode to ALL-ROOM load (§274 receipt:
6 rooms decoded, overlap/OOM). Dropping it for the HOST context is the same
class of translation as the SCLS stage-name rewrite (§267): donor meaning
preserved (the dummy cut does nothing), host semantics corrected.

Mechanics: remove the staff INDEX from the event's mStaff[20] list and
decrement mNStaff. The staff record itself stays in the pool, unreferenced —
no other indices shift, so nothing else can break.

Usage: scope_event_staff.py [--dry-run]   (targets TALE_DEMO, TALE_DEMO2)
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from merge_event import EventFile, ESZ, SSZ

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
EVENTS = ["TALE_DEMO", "TALE_DEMO2"]
DROP_STAFF = "ALL"


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    raw = bytearray(members["event_list.dat"])
    ef = EventFile(bytes(raw))
    changed = 0
    for ev_name in EVENTS:
        i = ef.find_event(ev_name)
        if i < 0:
            print(f"{ev_name}: NOT FOUND — skip")
            continue
        base = ef.etop + i * ESZ
        nstaff = struct.unpack_from(">i", raw, base + 0x7C)[0]
        staff_ids = [struct.unpack_from(">i", raw, base + 0x2C + k * 4)[0]
                     for k in range(nstaff)]
        names = [ef.name(ef.stop, s, SSZ) for s in staff_ids]
        if DROP_STAFF not in names:
            print(f"{ev_name}: staff {names} — no '{DROP_STAFF}' (idempotent)")
            continue
        keep = [s for s, nm in zip(staff_ids, names) if nm != DROP_STAFF]
        print(f"{ev_name}: staff {names} -> "
              f"{[n for n in names if n != DROP_STAFF]}")
        for k in range(len(staff_ids)):
            val = keep[k] if k < len(keep) else -1
            struct.pack_into(">i", raw, base + 0x2C + k * 4, val)
        struct.pack_into(">i", raw, base + 0x7C, len(keep))
        changed += 1
    if changed == 0:
        print("nothing to do")
        return 0
    # verify with the same reader
    chk = EventFile(bytes(raw))
    for ev_name in EVENTS:
        i = chk.find_event(ev_name)
        base = chk.etop + i * ESZ
        n = struct.unpack_from(">i", raw, base + 0x7C)[0]
        names = [chk.name(chk.stop,
                          struct.unpack_from(">i", raw, base + 0x2C + k * 4)[0],
                          SSZ) for k in range(n)]
        assert DROP_STAFF not in names, ev_name
        print(f"verify {ev_name}: staff = {names}")
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(raw)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-scope-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
