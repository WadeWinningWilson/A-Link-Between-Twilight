#!/usr/bin/env python3

# ============================================================================
"""§276: trim the R_DL01 host MULT group to room 0 — History's §275 fix (2),
the decisive one.

WHY (History's chunk-diff receipt, §275): the stage ALWAYS defines a 6-room
stitched MULT group whose entries overlap in space (host-shell authoring
artifact — gameplay entry ignores MULT and loads the arrival room only). REVT
presence flips stage entry into event-mode, which loads the FULL MULT group ->
all 6 interiors co-render (the §274 pile-up). TP forces REVT ids to be their
own index (getEventName indexes entries[mEventInfoIdx] directly), so id 0
cannot be moved out of low-probe range — the robust fix is the MULT side:
**a group containing only room 0 cannot pile up, whatever trips event-mode.**

Safe because the WW host only USES room 0 (History's call, §275); future
hosted events in other rooms are single-room too and need no stitched group.

Usage: mult_trim.py [--dry-run]
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
from adapt_bdl_arcs import be32

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
KEEP_ROOM = 0
ENTRY = 0xC


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    dzs = bytearray(members["stage.dzs"])
    n = be32(dzs, 0)
    for i in range(n):
        tag = dzs[4 + i * 0xC: 8 + i * 0xC].decode("ascii", "replace")
        if tag != "MULT":
            continue
        cnt, off = be32(dzs, 8 + i * 0xC), be32(dzs, 12 + i * 0xC)
        rooms = [dzs[off + k * ENTRY + 0xA] for k in range(cnt)]
        keep = [k for k in range(cnt) if dzs[off + k * ENTRY + 0xA] == KEEP_ROOM]
        print(f"MULT: {cnt} entries, rooms {rooms} -> keeping "
              f"{len(keep)} (room {KEEP_ROOM})")
        if len(keep) == cnt:
            print("nothing to trim (idempotent)")
            return 0
        # compact kept entries in place; zero the tail; shrink the count.
        kept_bytes = b"".join(
            bytes(dzs[off + k * ENTRY: off + (k + 1) * ENTRY]) for k in keep)
        dzs[off: off + len(kept_bytes)] = kept_bytes
        for b in range(off + len(kept_bytes), off + cnt * ENTRY):
            dzs[b] = 0
        struct.pack_into(">I", dzs, 8 + i * 0xC, len(keep))
        # verify with a re-read
        cnt2, off2 = be32(dzs, 8 + i * 0xC), be32(dzs, 12 + i * 0xC)
        rooms2 = [dzs[off2 + k * ENTRY + 0xA] for k in range(cnt2)]
        assert rooms2 == [KEEP_ROOM] * len(keep), rooms2
        print(f"verify: MULT now {cnt2} entries, rooms {rooms2}")
        break
    else:
        print("no MULT chunk found — nothing to do")
        return 0
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "stage.dzs"]
    files.append(("stage.dzs", bytes(dzs)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-multtrim-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
