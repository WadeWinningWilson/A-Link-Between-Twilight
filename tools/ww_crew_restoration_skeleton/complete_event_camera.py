#!/usr/bin/env python3
"""§288 (History's §286-addendum): ONE root, one pass — complete tale_1's
CAMERA machinery to match the working end-to-end reference (awake).

THE DIFF (receipts, full-staff dump):
  awake (works: fade + finish):  CAMERA cuts PAUSE(Stay=1) -> STBWAIT
    (wait-flags=[its PACKAGE PLAY flag]; data Center/Eye/Fovy restore);
    event.mFlags = (PACKAGE WAIT, CAMERA STBWAIT, -1)  — TWO conditions.
  tale_1: CAMERA has only the donor's pre-reload TALK cut; mFlags awaited only
    the PACKAGE WAIT (§287). The fade AND the finish are this one gap: PAUSE/
    STBWAIT is the STB camera handover + fade machinery AND the second finish
    flag. (No DIRECTOR staff exists in awake either — the camera chain IS it.)

WHAT THIS DOES (all verified, idempotent):
  1. clones awake's PAUSE+STBWAIT cuts + data CROSS-FILE (F_DL01 list ->
     R_DL01 list), flagIds kept verbatim (861/863) after a dest-wide
     collision scan (safety stop on collision);
  2. rewrites STBWAIT's wait-flag to tale_1's OWN PACKAGE PLAY flagId (3) —
     the one non-verbatim field, receipted (855 was awake's PLAY);
  3. repoints tale_1's CAMERA staff at the new chain (donor TALK cut stays
     orphaned in the pool — revert = repoint back);
  4. sets tale_1.mFlags = (9, 863, -1) — awake's two-condition pattern.

Usage: complete_event_camera.py [--dry-run]
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
SRC_ARC = MOD / "files" / "res" / "Stage" / "F_DL01" / "STG_00.arc"
DST_ARC = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"
SRC_EVENT, DST_EVENT = "awake", "tale_1"
PKG_PLAY_FLAG_DST = 3          # tale_1's cloned PACKAGE PLAY flagId (§281)
PKG_WAIT_FLAG_DST = 9          # tale_1's PACKAGE WAIT flagId (§287 mFlags[0])


def find_staff(ef, ev_idx, want):
    ev = ef.rec(ef.etop, ev_idx, ESZ)
    n = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(n):
        si = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if ef.name(ef.stop, si, SSZ) == want:
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
    src_raw = dict(g.list_rarc_files(SRC_ARC.read_bytes()))["event_list.dat"]
    dst_members = dict(g.list_rarc_files(DST_ARC.read_bytes()))
    dst_raw = dst_members["event_list.dat"]
    src, dst = EventFile(src_raw), EventFile(dst_raw)

    dst_ev = dst.find_event(DST_EVENT)
    dst_cam = find_staff(dst, dst_ev, "CAMERA")
    assert dst_ev >= 0 and dst_cam >= 0

    # idempotency: does tale_1's CAMERA already run a PAUSE chain?
    c0 = struct.unpack_from(">i", dst.rec(dst.stop, dst_cam, SSZ), 0x30)[0]
    if 0 <= c0 < dst.cnum and \
            bytes(dst.rec(dst.ctop, c0, CSZ)[:32]).split(b"\0")[0] == b"PAUSE":
        print("tale_1 CAMERA already runs the PAUSE chain (idempotent) — done")
        return 0

    src_ev = src.find_event(SRC_EVENT)
    src_cam = find_staff(src, src_ev, "CAMERA")
    assert src_ev >= 0 and src_cam >= 0
    # walk source camera chain: cuts + data
    cut_ids, data_ids = [], []
    c = struct.unpack_from(">i", src.rec(src.stop, src_cam, SSZ), 0x30)[0]
    while 0 <= c < src.cnum:
        cut_ids.append(c)
        d = struct.unpack_from(">i", src.rec(src.ctop, c, CSZ), 0x38)[0]
        while 0 <= d < src.dnum:
            data_ids.append(d)
            d = struct.unpack_from(">i", src.rec(src.dtop, d, DSZ), 0x30)[0]
        c = struct.unpack_from(">i", src.rec(src.ctop, c, CSZ), 0x3C)[0]
    names = [bytes(src.rec(src.ctop, c, CSZ)[:32]).split(b"\0")[0].decode()
             for c in cut_ids]
    print(f"cloning awake.CAMERA chain {names} (cuts={cut_ids}, "
          f"data={data_ids}) cross-file")
    assert names == ["PAUSE", "STBWAIT"], names

    # collision scan: source cut flagIds must be free in dest
    src_fids = [struct.unpack_from(">I", src.rec(src.ctop, c, CSZ), 0x34)[0]
                for c in cut_ids]
    used = all_flag_ids(dst)
    clash = [f for f in src_fids if f in used]
    if clash:
        raise SystemExit(f"flagId collision in dest: {clash} — remap needed, "
                         "safety stop")

    c_map = {old: dst.cnum + i for i, old in enumerate(cut_ids)}
    d_map = {old: dst.dnum + i for i, old in enumerate(data_ids)}

    f_add, i_add, sd_add = bytearray(), bytearray(), bytearray()
    new_data = []
    for old in data_ids:
        r = src.rec(src.dtop, old, DSZ)
        ty, di, num, nxt = struct.unpack_from(">iiii", r, 0x24)
        if ty == 4:
            end = src_raw.index(b"\0", src.sdtop + di)
            new_di = dst.sdnum + len(sd_add)
            sd_add += src_raw[src.sdtop + di: end + 1]
        elif ty == 3:
            new_di = dst.inum + len(i_add) // 4
            i_add += src_raw[src.itop + di * 4: src.itop + (di + num) * 4]
        else:
            cnt = num * (3 if ty == 1 else 1)
            new_di = dst.fnum + len(f_add) // 4
            f_add += src_raw[src.ftop + di * 4: src.ftop + (di + cnt) * 4]
        struct.pack_into(">i", r, 0x28, new_di)
        struct.pack_into(">i", r, 0x30, d_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x20, d_map[old])
        new_data.append(r)

    new_cuts = []
    for old in cut_ids:
        r = src.rec(src.ctop, old, CSZ)
        dtop = struct.unpack_from(">i", r, 0x38)[0]
        nxt = struct.unpack_from(">i", r, 0x3C)[0]
        struct.pack_into(">i", r, 0x38, d_map.get(dtop, -1) if dtop >= 0 else -1)
        struct.pack_into(">i", r, 0x3C, c_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x24, c_map[old])
        # THE one non-verbatim rewrite: STBWAIT waits on the dest's PLAY flag
        if bytes(r[:32]).split(b"\0")[0] == b"STBWAIT":
            struct.pack_into(">iii", r, 0x28, PKG_PLAY_FLAG_DST, -1, -1)
        new_cuts.append(r)

    # repoint dest CAMERA staff + set awake's two-condition mFlags
    dst_raw_new = bytearray(dst_raw)
    st_base = dst.stop + dst_cam * SSZ
    struct.pack_into(">i", dst_raw_new, st_base + 0x30, c_map[cut_ids[0]])
    struct.pack_into(">i", dst_raw_new, st_base + 0x38, c_map[cut_ids[0]])
    ev_base = dst.etop + dst_ev * ESZ
    stbwait_fid = src_fids[names.index("STBWAIT")]
    struct.pack_into(">iii", dst_raw_new, ev_base + 0x88,
                     PKG_WAIT_FLAG_DST, stbwait_fid, -1)

    # rebuild dest (P2: index-linked, safe)
    def chunk(top, num, size):
        return bytes(dst_raw_new[top: top + num * size])

    events = chunk(dst.etop, dst.enum, ESZ)
    staff = chunk(dst.stop, dst.snum, SSZ)
    cuts = chunk(dst.ctop, dst.cnum, CSZ) + b"".join(bytes(x) for x in new_cuts)
    datas = chunk(dst.dtop, dst.dnum, DSZ) + b"".join(bytes(x) for x in new_data)
    fdat = bytes(dst_raw_new[dst.ftop: dst.ftop + dst.fnum * 4]) + bytes(f_add)
    idat = bytes(dst_raw_new[dst.itop: dst.itop + dst.inum * 4]) + bytes(i_add)
    sdat = bytes(dst_raw_new[dst.sdtop: dst.sdtop + dst.sdnum]) + bytes(sd_add)

    out = bytearray(0x40)
    cur = 0x40
    offs = []
    counts = [dst.enum, dst.snum, dst.cnum + len(new_cuts),
              dst.dnum + len(new_data), dst.fnum + len(f_add) // 4,
              dst.inum + len(i_add) // 4, dst.sdnum + len(sd_add)]
    for blob in (events, staff, cuts, datas, fdat, idat, sdat):
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    # verify: full re-dump of tale_1's camera + mFlags on the new bytes
    chk = EventFile(bytes(out))
    ci = find_staff(chk, chk.find_event(DST_EVENT), "CAMERA")
    c = struct.unpack_from(">i", chk.rec(chk.stop, ci, SSZ), 0x30)[0]
    got = []
    while 0 <= c < chk.cnum:
        cut = chk.rec(chk.ctop, c, CSZ)
        got.append((bytes(cut[:32]).split(b"\0")[0].decode(),
                    struct.unpack_from(">3i", cut, 0x28),
                    struct.unpack_from(">I", cut, 0x34)[0]))
        c = struct.unpack_from(">i", cut, 0x3C)[0]
    mf = struct.unpack_from(">3i", out,
                            chk.etop + chk.find_event(DST_EVENT) * ESZ + 0x88)
    print(f"verify CAMERA chain: {got}")
    print(f"verify mFlags: {mf}")
    assert [g0[0] for g0 in got] == ["PAUSE", "STBWAIT"]
    assert got[1][1][0] == PKG_PLAY_FLAG_DST
    assert mf == (PKG_WAIT_FLAG_DST, stbwait_fid, -1)
    if dry:
        print("dry-run — nothing written")
        return 0
    files = [(nm, b) for nm, b in g.list_rarc_files(DST_ARC.read_bytes())
             if nm != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = DST_ARC.with_suffix(DST_ARC.suffix + ".pre-camchain-bak")
    if not bak.is_file():
        bak.write_bytes(DST_ARC.read_bytes())
        print(f"backup -> {bak.name}")
    DST_ARC.write_bytes(g.pack_rarc(files))
    print(f"wrote {DST_ARC}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
