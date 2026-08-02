#!/usr/bin/env python3
# ============================================================================
# restore_event_staff.py — Pass-2 bake: restore a donor STAFF (+its cut/data
# chain) to an EXISTING host event. First target: the ALL/dummy staff the
# original merge dropped from TALE_DEMO/TALE_DEMO2 (§323 checklist row 2,
# surfaced as a diffable gap by the §326 kit regression).
#
# Mechanism = merge_event.py's proven pool-append + re-base (event_list.dat is
# index-linked — P2: pool growth is safe when every top/count is rewritten and
# indices re-based). The only in-place change is the HOST EVENT RECORD: donor
# staff index appended to its staff array (+0x2C, 20 slots) and count (+0x7C)
# incremented. mFlags and every other event field are left byte-identical and
# the tool PRINTS the donor-vs-host event-record diff for the ledger.
#
# DEFAULT DRY-RUN. --write backs up STG_00.arc (.pre-allstaff-bak) first.
# ============================================================================
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import grow_rdl01_stg as g
from merge_event import EventFile, arc_member, ESZ, SSZ, CSZ, DSZ

DONOR_STAGE = Path("D:/XXXXXXX/Ex WW/files/res/Stage/LinkRM/Stage.arc")
HOST_STG = Path("C:/Users/xxxxx/AppData/Roaming/TwilitRealm/Dusklight/"
                "model_replacements/WW-Crew-Restoration/files/res/Stage/"
                "R_DL01/STG_00.arc")
HDR = 56
TARGETS = [("TALE_DEMO", "ALL"), ("TALE_DEMO2", "ALL")]


def staff_chain(src, ev_idx, staff_name):
    """The donor staff (by name) within one event + its cut/data closure."""
    ev = src.rec(src.etop, ev_idx, ESZ)
    nstaff = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(nstaff):
        s = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        nm = bytes(src.rec(src.stop, s, SSZ)[:8]).split(b"\0")[0].decode(
            "ascii", "replace")
        if nm != staff_name:
            continue
        cuts, datas = [], []
        c = struct.unpack_from(">i", src.rec(src.stop, s, SSZ), 0x30)[0]
        guard = 0
        while 0 <= c < src.cnum and guard < 512:
            guard += 1
            cuts.append(c)
            cut = src.rec(src.ctop, c, CSZ)
            d = struct.unpack_from(">i", cut, 0x38)[0]
            dg = 0
            while 0 <= d < src.dnum and dg < 512:
                dg += 1
                datas.append(d)
                d = struct.unpack_from(">i", src.rec(src.dtop, d, DSZ), 0x30)[0]
            c = struct.unpack_from(">i", cut, 0x3C)[0]
        return s, cuts, datas
    return None, [], []


def host_has_staff(dst, ev_idx, staff_name):
    ev = dst.rec(dst.etop, ev_idx, ESZ)
    nstaff = struct.unpack_from(">i", ev, 0x7C)[0]
    for k in range(nstaff):
        s = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        nm = bytes(dst.rec(dst.stop, s, SSZ)[:8]).split(b"\0")[0].decode(
            "ascii", "replace")
        if nm == staff_name:
            return True
    return False


def main():
    dry = "--write" not in sys.argv
    src = EventFile(arc_member(DONOR_STAGE, "event_list.dat"))
    dst = EventFile(arc_member(HOST_STG, "event_list.dat"))

    new_staff, new_cuts, new_data = [], [], []
    f_add, i_add, sd_add = bytearray(), bytearray(), bytearray()
    ev_patches = []  # (host_ev_idx, new_staff_global_idx)

    for ev_name, staff_name in TARGETS:
        si = src.find_event(ev_name)
        di = dst.find_event(ev_name)
        if si < 0 or di < 0:
            print(f"SKIP {ev_name}: donor idx {si} host idx {di}")
            continue
        if host_has_staff(dst, di, staff_name):
            print(f"SKIP {ev_name}: host already has staff '{staff_name}'")
            continue
        s_old, cuts, datas = staff_chain(src, si, staff_name)
        if s_old is None:
            print(f"SKIP {ev_name}: donor has no staff '{staff_name}'")
            continue

        # donor-vs-host event record diff, for the ledger (mFlags visibility)
        dev, hev = src.rec(src.etop, si, ESZ), dst.rec(dst.etop, di, ESZ)
        difs = [f"+0x{o:02X}:{dev[o]:02X}->{hev[o]:02X}"
                for o in range(ESZ) if dev[o] != hev[o]]
        print(f"{ev_name}: staff '{staff_name}' donor#{s_old} cuts={cuts} "
              f"datas={datas}; event-record byte diffs (donor->host): "
              f"{len(difs)} at {', '.join(difs[:12])}{'…' if len(difs) > 12 else ''}")

        s_map = {s_old: dst.snum + len(new_staff)}
        c_map = {old: dst.cnum + len(new_cuts) + i for i, old in enumerate(cuts)}
        d_map = {old: dst.dnum + len(new_data) + i for i, old in enumerate(datas)}

        for old in datas:
            r = src.rec(src.dtop, old, DSZ)
            ty, di_, num, nxt = struct.unpack_from(">iiii", r, 0x24)
            if ty == 4:
                end = src.raw.index(b"\0", src.sdtop + di_)
                blob = src.raw[src.sdtop + di_:end + 1]
                new_di = dst.sdnum + len(sd_add)
                sd_add += blob
            elif ty == 3:
                new_di = dst.inum + len(i_add) // 4
                i_add += src.raw[src.itop + di_ * 4:src.itop + (di_ + num) * 4]
            else:
                cnt = num * (3 if ty == 1 else 1)
                new_di = dst.fnum + len(f_add) // 4
                f_add += src.raw[src.ftop + di_ * 4:src.ftop + (di_ + cnt) * 4]
            struct.pack_into(">i", r, 0x28, new_di)
            struct.pack_into(">i", r, 0x30,
                             d_map.get(nxt, -1) if nxt >= 0 else -1)
            struct.pack_into(">I", r, 0x20, d_map[old])
            new_data.append(bytes(r))

        for old in cuts:
            r = src.rec(src.ctop, old, CSZ)
            d0 = struct.unpack_from(">i", r, 0x38)[0]
            nxt = struct.unpack_from(">i", r, 0x3C)[0]
            struct.pack_into(">i", r, 0x38,
                             d_map.get(d0, -1) if d0 >= 0 else -1)
            struct.pack_into(">i", r, 0x3C,
                             c_map.get(nxt, -1) if nxt >= 0 else -1)
            struct.pack_into(">I", r, 0x24, c_map[old])
            new_cuts.append(bytes(r))

        r = src.rec(src.stop, s_old, SSZ)
        sc = struct.unpack_from(">i", r, 0x30)[0]
        struct.pack_into(">i", r, 0x30, c_map.get(sc, -1) if sc >= 0 else -1)
        struct.pack_into(">i", r, 0x38, c_map.get(sc, -1) if sc >= 0 else -1)
        struct.pack_into(">I", r, 0x24, s_map[s_old])
        new_staff.append(bytes(r))
        ev_patches.append((di, s_map[s_old]))

    if not ev_patches:
        print("nothing to do")
        return 0
    if dry:
        print("dry-run — use --write to bake")
        return 0

    def chunk(top, num, size):
        return dst.raw[top:top + num * size]

    events = bytearray(chunk(dst.etop, dst.enum, ESZ))
    for di, s_new in ev_patches:
        base = di * ESZ
        n = struct.unpack_from(">i", events, base + 0x7C)[0]
        assert n < 20, "staff array full"
        struct.pack_into(">i", events, base + 0x2C + n * 4, s_new)
        struct.pack_into(">i", events, base + 0x7C, n + 1)

    staff = chunk(dst.stop, dst.snum, SSZ) + b"".join(new_staff)
    cuts = chunk(dst.ctop, dst.cnum, CSZ) + b"".join(new_cuts)
    datas = chunk(dst.dtop, dst.dnum, DSZ) + b"".join(new_data)
    fdat = dst.raw[dst.ftop:dst.ftop + dst.fnum * 4] + bytes(f_add)
    idat = dst.raw[dst.itop:dst.itop + dst.inum * 4] + bytes(i_add)
    sdat = dst.raw[dst.sdtop:dst.sdtop + dst.sdnum] + bytes(sd_add)

    out = bytearray(HDR)
    offs, cur = [], HDR
    for blob in (events, staff, cuts, datas, fdat, idat, sdat):
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    counts = [dst.enum, dst.snum + len(new_staff), dst.cnum + len(new_cuts),
              dst.dnum + len(new_data), dst.fnum + len(f_add) // 4,
              dst.inum + len(i_add) // 4, dst.sdnum + len(sd_add)]
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    chk = EventFile(bytes(out))
    for ev_name, staff_name in TARGETS:
        di = chk.find_event(ev_name)
        assert di >= 0 and host_has_staff(chk, di, staff_name), \
            f"verify failed: {ev_name} lacks {staff_name} after bake"
    print(f"verified: targets carry their restored staff "
          f"(events={chk.enum} staff={chk.snum} cuts={chk.cnum} "
          f"data={chk.dnum}, {len(out)} bytes)")

    files = [(n, b) for n, b in g.list_rarc_files(HOST_STG.read_bytes())
             if n != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = HOST_STG.with_suffix(HOST_STG.suffix + ".pre-allstaff-bak")
    if not bak.is_file():
        bak.write_bytes(HOST_STG.read_bytes())
        print(f"backup -> {bak.name}")
    HOST_STG.write_bytes(g.pack_rarc(files))
    print(f"wrote {HOST_STG}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
