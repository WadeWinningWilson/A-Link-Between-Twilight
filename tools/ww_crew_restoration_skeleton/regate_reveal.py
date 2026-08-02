#!/usr/bin/env python3
"""§291: re-gate tale_1's opening reveal to the storyboard-START signal.

THE DEFECT (§290's one wrong gate): reveal cut 2 (FADE -0.02) waited on flag 3
= the PACKAGE PLAY cut's COMPLETION flag — and specialProcPackage ends PLAY at
demo_mode()==2, i.e. the storyboard's END. The tale played under black and
revealed at the finish (History's probe: fired ~9455, expected ~7100).

THE MECHANISM (the user's suspicion, confirmed): fade timing signals ARE other
staffs' cut-completion flags — per-event data, nothing global. Which flag
means "storyboard started"? **CAMERA PAUSE's flagId (861)**: chain-order proof
— STBWAIT is PAUSE's successor and is active DURING the storyboard (it waits
for the end), therefore PAUSE completed at the start (the camera handoff).
PLAY's flag = STB END; PAUSE's flag = STB START. Cuts 1/3/4 stay as-is
(probe-verified correct).

One 12-byte edit: cut 2 wait-flags (3,-1,-1) -> (861,-1,-1).
Idempotent; backup .pre-regate-bak; verifies by chain re-dump.

Usage: regate_reveal.py [--dry-run]
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from merge_event import EventFile, ESZ, SSZ, CSZ

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
EVENT = "tale_1"
OLD_GATE, NEW_GATE = 3, 861   # PLAY cutEnd (STB end) -> PAUSE cutEnd (STB start)


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    raw = bytearray(members["event_list.dat"])
    ef = EventFile(bytes(raw))
    ev_idx = ef.find_event(EVENT)
    assert ev_idx >= 0
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    hit = None
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        st = ef.rec(ef.stop, si, SSZ)
        if struct.unpack_from(">i", st, 0x2C)[0] != 6:   # DIRECTOR
            continue
        c = struct.unpack_from(">i", st, 0x30)[0]
        chain = []
        while 0 <= c < ef.cnum:
            base = ef.ctop + c * CSZ
            wait = struct.unpack_from(">3i", raw, base + 0x28)
            fid = struct.unpack_from(">I", raw, base + 0x34)[0]
            chain.append((c, wait, fid))
            c = struct.unpack_from(">i", raw, base + 0x3C)[0]
        print(f"DIRECTOR chain (cut, wait, flagId): {chain}")
        for c, wait, fid in chain:
            if wait[0] == NEW_GATE:
                print(f"cut {c} already gated on {NEW_GATE} (idempotent)")
                return 0
            if wait[0] == OLD_GATE:
                hit = c
        break
    assert hit is not None, f"no DIRECTOR cut gated on {OLD_GATE} found"
    struct.pack_into(">iii", raw, ef.ctop + hit * CSZ + 0x28,
                     NEW_GATE, -1, -1)
    chk = struct.unpack_from(">3i", raw, ef.ctop + hit * CSZ + 0x28)
    print(f"verify: cut {hit} wait-flags now {chk} "
          f"(reveal fires at CAMERA PAUSE handoff = storyboard START)")
    assert chk == (NEW_GATE, -1, -1)
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(raw)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-regate-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
