#!/usr/bin/env python3
"""§281 (History's §280 ask): give tale_1 its STB PLAYER — clone TALE_DEMO's
working PACKAGE staff chain onto tale_1.

WHY (P5, stage-data-bake-pitfalls.md): tale_1 merged donor-verbatim has staff
{Ba1, CAMERA, Link} — in the DONOR the stb is played by the separate TALE_DEMO
entrance event. The port orders tale_1 directly; dDemo_c::start fires ONLY from
a PACKAGE PLAY cut (History's 10-hypothesis probe), so the demo idles at
frame 0. Host adaptation: absorb the PACKAGE staff into tale_1 — cloned
VERBATIM from the same file's TALE_DEMO chain (PLAY{FileName='tale.stb',
Stage='LinkRM', StartCode=0, RoomNo=0, Layer=0} -> WAIT), so no invented data.
Donor two-step remains the reference (P5 note): revert to the re-entrance
chain if behavioral deltas surface.

event_list rebuild is SAFE here (P2: index-linked, not offset-linked).
Idempotent; backup .pre-pkgstaff-bak; verifies by full chain re-dump.

Usage: add_package_staff.py [--dry-run]
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from merge_event import EventFile, ESZ, SSZ, CSZ, DSZ

MOD = (Path(os.environ["APPDATA"]) / "TwilitRealm" / "Dusklight"
       / "model_replacements" / "WW-Crew-Restoration")
TARGET = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
SRC_EVENT, DST_EVENT, STAFF = "TALE_DEMO", "tale_1", "PACKAGE"


def staff_of(ef: EventFile, raw: bytes, ev_idx: int, want: str) -> int:
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if ef.name(ef.stop, si, SSZ) == want:
            return si
    return -1


def main() -> int:
    dry = "--dry-run" in sys.argv
    members = dict(g.list_rarc_files(TARGET.read_bytes()))
    raw = members["event_list.dat"]
    ef = EventFile(raw)
    src_ev = ef.find_event(SRC_EVENT)
    dst_ev = ef.find_event(DST_EVENT)
    assert src_ev >= 0 and dst_ev >= 0, (src_ev, dst_ev)
    if staff_of(ef, raw, dst_ev, STAFF) >= 0:
        print(f"{DST_EVENT} already has a {STAFF} staff (idempotent) — done")
        return 0
    s_idx = staff_of(ef, raw, src_ev, STAFF)
    assert s_idx >= 0, f"{SRC_EVENT} has no {STAFF} staff to clone"

    # ---- walk the source chain (staff -> cuts -> data -> payloads) -------
    src_staff = ef.rec(ef.stop, s_idx, SSZ)
    cut_ids, data_ids = [], []
    c = struct.unpack_from(">i", src_staff, 0x30)[0]
    while 0 <= c < ef.cnum:
        cut_ids.append(c)
        cut = ef.rec(ef.ctop, c, CSZ)
        d = struct.unpack_from(">i", cut, 0x38)[0]
        while 0 <= d < ef.dnum:
            data_ids.append(d)
            d = struct.unpack_from(">i", ef.rec(ef.dtop, d, DSZ), 0x30)[0]
        c = struct.unpack_from(">i", cut, 0x3C)[0]
    print(f"cloning {SRC_EVENT}.{STAFF}: staff[{s_idx}] cuts={cut_ids} "
          f"data={data_ids}")

    c_map = {old: ef.cnum + i for i, old in enumerate(cut_ids)}
    d_map = {old: ef.dnum + i for i, old in enumerate(data_ids)}

    f_add, i_add, sd_add = bytearray(), bytearray(), bytearray()
    new_data = []
    for old in data_ids:
        r = ef.rec(ef.dtop, old, DSZ)
        ty, di, num, nxt = struct.unpack_from(">iiii", r, 0x24)
        if ty == 4:
            end = raw.index(b"\0", ef.sdtop + di)
            blob = raw[ef.sdtop + di: end + 1]
            new_di = ef.sdnum + len(sd_add)
            sd_add += blob
        elif ty == 3:
            new_di = ef.inum + len(i_add) // 4
            i_add += raw[ef.itop + di * 4: ef.itop + (di + num) * 4]
        else:
            cnt = num * (3 if ty == 1 else 1)
            new_di = ef.fnum + len(f_add) // 4
            f_add += raw[ef.ftop + di * 4: ef.ftop + (di + cnt) * 4]
        struct.pack_into(">i", r, 0x28, new_di)
        struct.pack_into(">i", r, 0x30, d_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x20, d_map[old])
        new_data.append(r)

    new_cuts = []
    for old in cut_ids:
        r = ef.rec(ef.ctop, old, CSZ)
        dtop = struct.unpack_from(">i", r, 0x38)[0]
        nxt = struct.unpack_from(">i", r, 0x3C)[0]
        struct.pack_into(">i", r, 0x38, d_map.get(dtop, -1) if dtop >= 0 else -1)
        struct.pack_into(">i", r, 0x3C, c_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x24, c_map[old])
        new_cuts.append(r)

    new_staff = bytearray(src_staff)
    sc = struct.unpack_from(">i", new_staff, 0x30)[0]
    struct.pack_into(">i", new_staff, 0x30, c_map.get(sc, -1))
    struct.pack_into(">i", new_staff, 0x38, c_map.get(sc, -1))
    struct.pack_into(">I", new_staff, 0x24, ef.snum)

    ev = ef.rec(ef.etop, dst_ev, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    assert n < 20, "staff list full"
    struct.pack_into(">i", ev, 0x2C + n * 4, ef.snum)
    struct.pack_into(">i", ev, 0x7C, n + 1)

    # ---- rebuild (P2: event_list is index-linked — rebuild is SAFE) ------
    def chunk(top, num, size):
        return raw[top: top + num * size]

    events = bytearray(chunk(ef.etop, ef.enum, ESZ))
    events[dst_ev * ESZ:(dst_ev + 1) * ESZ] = ev
    staff = chunk(ef.stop, ef.snum, SSZ) + bytes(new_staff)
    cuts = chunk(ef.ctop, ef.cnum, CSZ) + b"".join(bytes(x) for x in new_cuts)
    datas = chunk(ef.dtop, ef.dnum, DSZ) + b"".join(bytes(x) for x in new_data)
    fdat = raw[ef.ftop: ef.ftop + ef.fnum * 4] + bytes(f_add)
    idat = raw[ef.itop: ef.itop + ef.inum * 4] + bytes(i_add)
    sdat = raw[ef.sdtop: ef.sdtop + ef.sdnum] + bytes(sd_add)

    out = bytearray(0x40)
    cur = 0x40
    offs, counts = [], [ef.enum, ef.snum + 1, ef.cnum + len(new_cuts),
                        ef.dnum + len(new_data), ef.fnum + len(f_add) // 4,
                        ef.inum + len(i_add) // 4, ef.sdnum + len(sd_add)]
    for blob in (events, staff, cuts, datas, fdat, idat, sdat):
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    # ---- verify: full chain re-dump on the new bytes ---------------------
    chk = EventFile(bytes(out))
    si = staff_of(chk, bytes(out), chk.find_event(DST_EVENT), STAFF)
    assert si >= 0, "clone not attached"
    st = chk.rec(chk.stop, si, SSZ)
    c = struct.unpack_from(">i", st, 0x30)[0]
    chain = []
    while 0 <= c < chk.cnum:
        cut = chk.rec(chk.ctop, c, CSZ)
        cnm = bytes(cut[:32]).split(b"\0")[0].decode()
        d = struct.unpack_from(">i", cut, 0x38)[0]
        datas_txt = []
        while 0 <= d < chk.dnum:
            r = chk.rec(chk.dtop, d, DSZ)
            dnm = bytes(r[:32]).split(b"\0")[0].decode()
            ty, di, num, nxt = struct.unpack_from(">iiii", r, 0x24)
            if ty == 4:
                end = bytes(out).index(b"\0", chk.sdtop + di)
                datas_txt.append(f"{dnm}={out[chk.sdtop+di:end].decode()!r}")
            elif ty == 3:
                datas_txt.append(
                    f"{dnm}={struct.unpack_from('>i', out, chk.itop+di*4)[0]}")
            d = nxt
        chain.append((cnm, datas_txt))
        c = struct.unpack_from(">i", cut, 0x3C)[0]
    print(f"verify {DST_EVENT}.{STAFF}: {chain}")
    assert chain and chain[0][0] == "PLAY" and \
        any("tale.stb" in t for t in chain[0][1]), "chain wrong"
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(TARGET.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = TARGET.with_suffix(TARGET.suffix + ".pre-pkgstaff-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
