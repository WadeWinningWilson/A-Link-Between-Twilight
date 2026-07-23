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
"""
from __future__ import annotations

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
