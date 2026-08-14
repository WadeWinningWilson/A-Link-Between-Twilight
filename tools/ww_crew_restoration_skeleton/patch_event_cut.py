#!/usr/bin/env python3
"""Rename an event CUT in a stage's event_list.dat, in place.

WHY THIS EXISTS
---------------
`awake`'s CAMERA staff was merged from the donor as `PAUSE -> FIXEDFRM`.
`FIXEDFRM` pins the camera to fixed Center/Eye/Fovy values and never consults
the storyboard, so the STB's own 1,144-byte camera animation (a ~12 s pan, as
`seq 0x80` keyframes interleaved with `seq 0x02` frame waits) was overridden by
a static shot.

The receiver has a cut whose entire purpose is the opposite: `STBWAIT` maps to
`dCamera_c::stbWaitEvCamera` (d_camera.cpp ActionNames[27]), which reads
`dDemo_c::getCamera()` every frame and copies target/eye/fovy from the running
storyboard. That is the receiver-correct way to express "the storyboard drives
the camera", and it makes this a DATA fix rather than an engine change.

The cut name lives in a fixed 0x20 zero-padded field, so a shorter name is
written with the remainder cleared. A `.pre165-bak` backup is written first.

    python patch_event_cut.py <arc> <OLDNAME> <NEWNAME>

Usage:
  patch_event_cut.py <stage-arc> <old-cut-name> <new-cut-name>

  Inputs   : <stage-arc> (a stage STG_*.arc containing event_list.dat)
  Outputs  : the SAME arc, rewritten in place (arc.write_bytes)
  Idempotent: yes by name — a second run finds no <old-cut-name> and changes nothing.
  Order    : after the event exists in the target arc (i.e. after merge_event.py).
  R1 note  : takes an explicit target, so a recipe step must name the arc; it has no
             default and must never be given one (silently patching the wrong stage is
             the whole hazard this contract exists to prevent).
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

import shutil
import sys
from pathlib import Path

FIELD = 0x20


def main() -> int:
    if len(sys.argv) != 4:
        print(__doc__)
        return 1
    arc, old, new = Path(sys.argv[1]), sys.argv[2].encode(), sys.argv[3].encode()
    if len(new) >= FIELD:
        print(f"refused: '{new.decode()}' does not fit a {FIELD}-byte name field")
        return 1

    data = bytearray(arc.read_bytes())
    hits = []
    start = 0
    while True:
        i = data.find(old, start)
        if i < 0:
            break
        hits.append(i)
        start = i + 1

    if len(hits) != 1:
        # Refuse on 0 or >1: a blind replace could hit an unrelated string and
        # there is no cheap way to prove which one is the cut name.
        print(f"refused: expected exactly 1 occurrence of '{old.decode()}', found {len(hits)}")
        return 1

    off = hits[0]
    # Verify it really is a zero-padded fixed field, not a substring of something.
    tail = data[off + len(old):off + FIELD]
    if any(tail):
        print(f"refused: bytes after '{old.decode()}' at {off} are not zero padding — "
              f"this does not look like a cut-name field")
        return 1

    bak = arc.with_suffix(arc.suffix + ".pre165-bak")
    if not bak.exists():
        shutil.copy2(arc, bak)
        print(f"backup  : {bak.name}")

    data[off:off + FIELD] = new + b"\0" * (FIELD - len(new))
    arc.write_bytes(bytes(data))
    print(f"patched : {arc.name} @ {off}  '{old.decode()}' -> '{new.decode()}'")

    # Round-trip read-back, because a write that silently did nothing looks
    # identical to a write that worked.
    back = arc.read_bytes()
    ok = back[off:off + len(new)] == new and old not in back
    print(f"verify  : {'OK' if ok else 'FAILED'}")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
