#!/usr/bin/env python3

# ============================================================================
"""§290: add the native DIRECTOR FADE staff to tale_1 — the fade's true home
(specialProcDirector -> mDoGph_gInf_c::fadeOut; d_event_data.cpp:1037/1207).
Supersedes History's §289 fade-in bridge (they delete it on land).

DONOR-SHAPED CHAIN (all Rate values donor-verbatim from working references):
  1. FADE Rate=+0.03  ungated            -> to black immediately (hides the
     cast snap)                              [MK_GAMESTART's opening value]
  2. FADE Rate=-0.02  wait=PLAY flag 3   -> reveal once the storyboard rolls
                                             [MK_GAMESTART's reveal value]
  3. FADE Rate=+0.05  wait=WAIT flag 9   -> to black as the package completes
                                             [DEFAULT_NPC_NZ_ESA's pair]
  4. FADE Rate=-0.05  wait=STBWAIT 863   -> reveal after camera restore;
     terminal — its flagId JOINS mFlags so teardown waits for the reveal:
     mFlags = (9, 863, <fade4 flagId>).
FADE cut-end semantics are native: +rate ends at fadeRate>=1, -rate ends at
!isFade() (d_event_data.cpp:1207) — the chain self-paces.

Staff record cloned from donor MK_GAMESTART's DIRECTOR (type 6) for shape;
cut records cloned from its FADE cut; flagIds taken from donor's own
(3842/3843/3845/3846-range) after a dest-wide collision scan (safety stop).
Idempotent; backup .pre-director-bak.

Usage: add_director_staff.py [--dry-run]
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
from merge_event import EventFile, arc_member, ESZ, SSZ, CSZ, DSZ

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
DST_ARC = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
SRC_STAGE = Path(r"D:/XXXXXXX/Ex WW/files/res/Stage/sea/Stage.arc")
DST_EVENT = "tale_1"

# (rate, wait_flag or -1, new_flagId) — §290 chain, donor-verbatim rates
CHAIN = [(+0.03, -1, 3842),
         (-0.02, 3, 3843),
         (+0.05, 9, 3845),
         (-0.05, 863, 3846)]


def find_staff(ef, ev_idx, want_name=None, want_type=None):
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        st = ef.rec(ef.stop, si, SSZ)
        nm = bytes(st[:8]).split(b"\0")[0].decode("ascii", "replace")
        ty = struct.unpack_from(">i", st, 0x2C)[0]
        if (want_name is None or nm == want_name) and \
           (want_type is None or ty == want_type):
            return si
    return -1


def all_flag_ids(ef):
    ids = set()
    for s in range(ef.snum):
        ids.add(struct.unpack_from(">I", ef.rec(ef.stop, s, SSZ), 0x28)[0])
    for c in range(ef.cnum):
        ids.add(struct.unpack_from(">I", ef.rec(ef.ctop, c, CSZ), 0x34)[0])
    return ids


def main() -> int:
    dry = "--dry-run" in sys.argv
    src_raw = arc_member(SRC_STAGE, "event_list.dat")
    dst_members = dict(g.list_rarc_files(DST_ARC.read_bytes()))
    dst_raw = dst_members["event_list.dat"]
    src, dst = EventFile(src_raw), EventFile(dst_raw)

    dst_ev = dst.find_event(DST_EVENT)
    assert dst_ev >= 0
    if find_staff(dst, dst_ev, want_type=6) >= 0:
        print(f"{DST_EVENT} already has a DIRECTOR staff (idempotent) — done")
        return 0

    src_ev = src.find_event("MK_GAMESTART")
    src_dir = find_staff(src, src_ev, want_type=6)
    assert src_ev >= 0 and src_dir >= 0
    # template records: the DIRECTOR staff + its first FADE cut + Rate data
    tpl_staff = bytearray(src.rec(src.stop, src_dir, SSZ))
    c0 = struct.unpack_from(">i", tpl_staff, 0x30)[0]
    tpl_cut = bytearray(src.rec(src.ctop, c0, CSZ))
    assert bytes(tpl_cut[:32]).split(b"\0")[0] == b"FADE"
    d0 = struct.unpack_from(">i", src.rec(src.ctop, c0, CSZ), 0x38)[0]
    tpl_data = bytearray(src.rec(src.dtop, d0, DSZ))
    assert bytes(tpl_data[:32]).split(b"\0")[0] == b"Rate"

    used = all_flag_ids(dst)
    staff_fid = struct.unpack_from(">I", tpl_staff, 0x28)[0]
    want_ids = [f for _r, _w, f in CHAIN] + [staff_fid]
    clash = [f for f in want_ids if f in used]
    if clash:
        raise SystemExit(f"flagId collision in dest: {clash} — safety stop")

    # ---- build 4 data nodes (Rate floats) + 4 FADE cuts ------------------
    f_add = bytearray()
    new_data, new_cuts = [], []
    for i, (rate, wait, fid) in enumerate(CHAIN):
        r = bytearray(tpl_data)
        struct.pack_into(">i", r, 0x28, dst.fnum + i)      # di -> fData slot
        struct.pack_into(">i", r, 0x30, -1)                # no next data
        struct.pack_into(">I", r, 0x20, dst.dnum + i)
        f_add += struct.pack(">f", rate)
        new_data.append(r)
        cut = bytearray(tpl_cut)
        struct.pack_into(">iii", cut, 0x28, wait, -1, -1)  # wait-flags
        struct.pack_into(">I", cut, 0x34, fid)             # own flagId
        struct.pack_into(">i", cut, 0x38, dst.dnum + i)    # dataTop
        struct.pack_into(">i", cut, 0x3C,
                         dst.cnum + i + 1 if i + 1 < len(CHAIN) else -1)
        struct.pack_into(">I", cut, 0x24, dst.cnum + i)
        new_cuts.append(cut)

    new_staff = bytearray(tpl_staff)
    struct.pack_into(">i", new_staff, 0x30, dst.cnum)      # startCut
    struct.pack_into(">i", new_staff, 0x38, dst.cnum)
    struct.pack_into(">I", new_staff, 0x24, dst.snum)

    dst_new = bytearray(dst_raw)
    ev_base = dst.etop + dst_ev * ESZ
    n = struct.unpack_from(">i", dst_new, ev_base + 0x7C)[0]
    assert n < 20
    struct.pack_into(">i", dst_new, ev_base + 0x2C + n * 4, dst.snum)
    struct.pack_into(">i", dst_new, ev_base + 0x7C, n + 1)
    # teardown waits for the FINAL REVEAL: mFlags = (9, 863, fade4)
    struct.pack_into(">iii", dst_new, ev_base + 0x88, 9, 863, CHAIN[-1][2])

    # ---- rebuild (P2: index-linked) --------------------------------------
    def chunk(top, num, size):
        return bytes(dst_new[top: top + num * size])

    events = chunk(dst.etop, dst.enum, ESZ)
    staff = chunk(dst.stop, dst.snum, SSZ) + bytes(new_staff)
    cuts = chunk(dst.ctop, dst.cnum, CSZ) + b"".join(bytes(x) for x in new_cuts)
    datas = chunk(dst.dtop, dst.dnum, DSZ) + b"".join(bytes(x) for x in new_data)
    fdat = chunk(dst.ftop, dst.fnum, 4) + bytes(f_add)
    idat = chunk(dst.itop, dst.inum, 4)
    sdat = bytes(dst_new[dst.sdtop: dst.sdtop + dst.sdnum])

    out = bytearray(0x40)
    cur = 0x40
    offs = []
    counts = [dst.enum, dst.snum + 1, dst.cnum + len(new_cuts),
              dst.dnum + len(new_data), dst.fnum + len(f_add) // 4,
              dst.inum, dst.sdnum]
    for blob in (events, staff, cuts, datas, fdat, idat, sdat):
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    # ---- verify: full DIRECTOR chain re-dump -----------------------------
    chk = EventFile(bytes(out))
    di = find_staff(chk, chk.find_event(DST_EVENT), want_type=6)
    assert di >= 0
    c = struct.unpack_from(">i", chk.rec(chk.stop, di, SSZ), 0x30)[0]
    got = []
    while 0 <= c < chk.cnum:
        cut = chk.rec(chk.ctop, c, CSZ)
        d = struct.unpack_from(">i", cut, 0x38)[0]
        rd = chk.rec(chk.dtop, d, DSZ)
        fdi = struct.unpack_from(">i", rd, 0x28)[0]
        rate = struct.unpack_from(">f", out, chk.ftop + fdi * 4)[0]
        got.append((bytes(cut[:32]).split(b"\0")[0].decode(),
                    round(rate, 3),
                    struct.unpack_from(">3i", cut, 0x28)[0],
                    struct.unpack_from(">I", cut, 0x34)[0]))
        c = struct.unpack_from(">i", cut, 0x3C)[0]
    mf = struct.unpack_from(">3i", out,
                            chk.etop + chk.find_event(DST_EVENT) * ESZ + 0x88)
    print(f"verify DIRECTOR chain (name, rate, wait, flagId): {got}")
    print(f"verify mFlags: {mf}")
    assert [x[0] for x in got] == ["FADE"] * 4
    assert [x[1] for x in got] == [round(r, 3) for r, _w, _f in CHAIN]
    assert [x[2] for x in got] == [w for _r, w, _f in CHAIN]
    assert mf == (9, 863, CHAIN[-1][2])
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(DST_ARC.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = DST_ARC.with_suffix(DST_ARC.suffix + ".pre-director-bak")
    if not bak.is_file():
        bak.write_bytes(DST_ARC.read_bytes())
        print(f"backup -> {bak.name}")
    DST_ARC.write_bytes(g.pack_rarc(files))
    print(f"wrote {DST_ARC}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
