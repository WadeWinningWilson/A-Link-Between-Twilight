#!/usr/bin/env python3

# ============================================================================
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
# ============================================================================
# TIER-3 QUARANTINE (kit audit + OUTPUT LAW wiring). This tool writes donor-
# format bytes and belongs to the era the project is leaving. It must NOT run
# in the disc-native porting wave. Kept RE-RUNNABLE per the No116 ceremony:
# pass --tier3-override; the run is then LEDGERED so no donor-format write
# happens silently. IMPORT-SAFE: gates only under __main__, so Tier-1 kits
# that import helpers from this file are unaffected.
# ============================================================================
import sys as _q_sys
if __name__ == "__main__":
    if "--tier3-override" not in _q_sys.argv:
        _q_sys.stderr.write(
            "TIER-3 QUARANTINED (kit audit): retired from the disc-native wave. "
            "Deliberate rerun: --tier3-override (ledgered).\n")
        raise SystemExit(3)
    _q_sys.argv.remove("--tier3-override")
    import io as _q_io
    from pathlib import Path as _q_P
    _q_led = _q_P(__file__).resolve().parents[2] / "docs" / "state" / "ww-staging" / "OUTPUT-LEDGER.md"
    with _q_io.open(_q_led, "a", encoding="utf-8", newline="\r\n") as _q_f:
        _q_f.write("| TIER3-OVERRIDE RUN: %s | deliberate rerun | quarantine stands |\n"
                   % _q_P(__file__).name)

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
