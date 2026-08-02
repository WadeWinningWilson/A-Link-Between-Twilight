#!/usr/bin/env python3
"""§292: re-reference tale_1's OPENING from native mid-gameplay donors —
no prescribed delays, the donor's own gate structure.

THE WRONG REFERENCE (§290's root defect, History's finding): cut 1 was cloned
from MK_GAMESTART — a TITLE start that begins from black, where an ungated
opening FADE is correct. Mid-gameplay cutscenes are structured differently.

THE NATIVE MID-GAMEPLAY PATTERN (three donor receipts, unanimous):
  ARRIVAL_GND:        DIR WAIT(ungated) -> FADE gated on TIMEKEEP COUNTDOWN cut
  DEFAULT_NPC_NZ_ESA: DIR WAIT(ungated) -> FADE gated on the NPC's TALK cut
  MEGAMI_DEMO:        DIR WAIT(ungated) -> FADE gated on a DUMMY cut
  => the chain OPENS WITH AN UNGATED WAIT (director idles while the event
     establishes: control lock + letterbox), and the first FADE gates on an
     EARLY CUT BOUNDARY of the scene's driving staff — never a raw delay.

THE TRANSLATION (zero invented numbers):
  - insert a leading WAIT cut (dataless, donor convention) at the chain head;
  - re-gate FADE(+0.03) from ungated -> wait on flag 1699 = Ba1's
    START_TALE1 cut completion — the tale's establishment beat, timed by the
    DONOR'S OWN mEvTimer in cut_move_START_TALE1 (the §271 receipt).
  Ordering achieved: [lock + letterbox] -> donor-timed beat -> fade ->
  storyboard. Cuts 2/3/4 untouched (§291-verified).

Idempotent; backup .pre-opening-bak; verifies by full chain re-dump.

Usage: rework_opening_fade.py [--dry-run]
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
GATE_FLAG = 1699          # Ba1 START_TALE1 cut completion (donor-timed beat)
NEW_WAIT_FLAG = 3850      # free id above the §290 block (scan-verified)


def all_flag_ids(ef):
    ids = set()
    for s in range(ef.snum):
        ids.add(struct.unpack_from(">I", ef.rec(ef.stop, s, SSZ), 0x28)[0])
    for c in range(ef.cnum):
        ids.add(struct.unpack_from(">I", ef.rec(ef.ctop, c, CSZ), 0x34)[0])
    return ids


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    raw = bytearray(members["event_list.dat"])
    ef = EventFile(bytes(raw))
    ev_idx = ef.find_event(EVENT)
    assert ev_idx >= 0
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    dir_staff = -1
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if struct.unpack_from(">i", ef.rec(ef.stop, si, SSZ), 0x2C)[0] == 6:
            dir_staff = si
            break
    assert dir_staff >= 0, "no DIRECTOR staff — run add_director_staff first"
    st_base = ef.stop + dir_staff * SSZ
    first_cut = struct.unpack_from(">i", raw, st_base + 0x30)[0]
    first_name = bytes(ef.rec(ef.ctop, first_cut, CSZ)[:32]).split(b"\0")[0]
    if first_name == b"WAIT":
        print("DIRECTOR chain already opens with WAIT (idempotent) — done")
        return 0
    assert first_name == b"FADE", first_name
    if NEW_WAIT_FLAG in all_flag_ids(ef):
        raise SystemExit(f"flagId {NEW_WAIT_FLAG} in use — safety stop")

    # new WAIT cut: clone the FADE cut record's shape, rename, no data,
    # ungated, next -> the existing FADE chain head
    wait_cut = bytearray(ef.rec(ef.ctop, first_cut, CSZ))
    wait_cut[:32] = b"WAIT".ljust(32, b"\0")
    struct.pack_into(">iii", wait_cut, 0x28, -1, -1, -1)   # ungated
    struct.pack_into(">I", wait_cut, 0x34, NEW_WAIT_FLAG)
    struct.pack_into(">i", wait_cut, 0x38, -1)             # dataless
    struct.pack_into(">i", wait_cut, 0x3C, first_cut)
    struct.pack_into(">I", wait_cut, 0x24, ef.cnum)

    # re-gate the opening FADE on the donor-timed establishment beat
    fade_base = ef.ctop + first_cut * CSZ
    struct.pack_into(">iii", raw, fade_base + 0x28, GATE_FLAG, -1, -1)
    # repoint the DIRECTOR staff at the new WAIT
    struct.pack_into(">i", raw, st_base + 0x30, ef.cnum)
    struct.pack_into(">i", raw, st_base + 0x38, ef.cnum)

    # rebuild (P2: index-linked)
    def chunk(top, num, size):
        return bytes(raw[top: top + num * size])

    out = bytearray(0x40)
    cur = 0x40
    offs = []
    blobs = [chunk(ef.etop, ef.enum, ESZ),
             chunk(ef.stop, ef.snum, SSZ),
             chunk(ef.ctop, ef.cnum, CSZ) + bytes(wait_cut),
             chunk(ef.dtop, ef.dnum, 0x40),
             chunk(ef.ftop, ef.fnum, 4),
             chunk(ef.itop, ef.inum, 4),
             bytes(raw[ef.sdtop: ef.sdtop + ef.sdnum])]
    counts = [ef.enum, ef.snum, ef.cnum + 1, ef.dnum, ef.fnum, ef.inum,
              ef.sdnum]
    for blob in blobs:
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    # verify: full chain re-dump
    chk = EventFile(bytes(out))
    ci = None
    ev2 = chk.rec(chk.etop, chk.find_event(EVENT), ESZ)
    for k in range(struct.unpack_from(">i", ev2, 0x7C)[0]):
        si = struct.unpack_from(">i", ev2, 0x2C + k * 4)[0]
        if struct.unpack_from(">i", chk.rec(chk.stop, si, SSZ), 0x2C)[0] == 6:
            ci = si
    c = struct.unpack_from(">i", chk.rec(chk.stop, ci, SSZ), 0x30)[0]
    got = []
    while 0 <= c < chk.cnum:
        cut = chk.rec(chk.ctop, c, CSZ)
        got.append((bytes(cut[:32]).split(b"\0")[0].decode(),
                    struct.unpack_from(">3i", cut, 0x28)[0],
                    struct.unpack_from(">I", cut, 0x34)[0]))
        c = struct.unpack_from(">i", cut, 0x3C)[0]
    print(f"verify DIRECTOR chain (name, wait, flagId): {got}")
    assert [x[0] for x in got] == ["WAIT", "FADE", "FADE", "FADE", "FADE"]
    assert got[0][1] == -1 and got[1][1] == GATE_FLAG
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-opening-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
