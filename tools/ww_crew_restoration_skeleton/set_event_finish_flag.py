#!/usr/bin/env python3

# ============================================================================
"""§287 (History's §286 ask): reconcile tale_1's finish-flag accounting.

THE STRAND (flag-graph dump receipts):
  finishCheck() (d_event_data.cpp:380) awaits event.mFlags[3] (-1-terminated).
  Working invariant across ALL working events: mFlags[0] == the flagId of the
  TERMINAL cut of the staff that carries the performance:
    TALE_DEMO    mFlags=(9)    == PACKAGE WAIT flagId 9
    Ba1_Get_Itm  mFlags=(56)   == Link's last cut flagId 56
    tale_2       mFlags=(1712) == PACKAGE PLAY flagId 1712
  tale_1 awaits (1701) == Ba1's WAIT cut — donor accounting authored for the
  RELOAD teardown (donor cut_move_START_TALE1 tears the stage down via
  setNextStage mid-event; the event never finishes in place). In the §281
  collapsed one-step, the performance carrier is the absorbed PACKAGE staff.

THE RECONCILE (no invented flags): tale_1.mFlags[0] = 9 — the cloned PACKAGE
WAIT cut's flagId, mirroring TALE_DEMO's PROVEN teardown accounting exactly.
The event finishes when the storyboard package completes, as TALE_DEMO does.
(Cross-event flagId duplication with TALE_DEMO is harmless: flags reset per
event init, d_event_data.cpp:374, and only one event runs at a time.)

In-place 4-byte edit (same-size member; P1 concerns don't apply, and P2 says
event_list is index-linked anyway). Idempotent; backup .pre-finishflag-bak.

Usage: set_event_finish_flag.py [--dry-run]
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


def package_wait_flag(ef: EventFile, raw: bytes, ev_idx: int) -> int:
    """flagId of the LAST cut of the event's PACKAGE staff."""
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if ef.name(ef.stop, si, SSZ) != "PACKAGE":
            continue
        c = struct.unpack_from(">i", ef.rec(ef.stop, si, SSZ), 0x30)[0]
        last = -1
        while 0 <= c < ef.cnum:
            cut = ef.rec(ef.ctop, c, CSZ)
            last = struct.unpack_from(">I", cut, 0x34)[0]
            c = struct.unpack_from(">i", cut, 0x3C)[0]
        return last
    return -1


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    raw = bytearray(members["event_list.dat"])
    ef = EventFile(bytes(raw))
    i = ef.find_event(EVENT)
    assert i >= 0, EVENT
    base = ef.etop + i * ESZ
    cur = struct.unpack_from(">3i", raw, base + 0x88)
    want = package_wait_flag(ef, bytes(raw), i)
    assert want > 0, "no PACKAGE terminal cut flag found — run add_package_staff first"
    print(f"{EVENT}: mFlags={cur}; PACKAGE terminal cut flagId={want}")
    if cur[0] == want:
        print("already reconciled (idempotent)")
        return 0
    struct.pack_into(">iii", raw, base + 0x88, want, -1, -1)
    chk = struct.unpack_from(">3i", raw, base + 0x88)
    print(f"verify: mFlags now {chk} (finishCheck passes when the storyboard "
          "package completes — TALE_DEMO's proven accounting)")
    assert chk == (want, -1, -1)
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(raw)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-finishflag-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
