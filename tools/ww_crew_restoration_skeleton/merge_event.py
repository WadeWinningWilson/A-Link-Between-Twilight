#!/usr/bin/env python3
"""№152: merge a donor event into the receiver stage's own event_list.dat.

WHY MERGE AND NOT REPLACE
-------------------------
`F_DL01/STG_00.arc` already carries an event_list.dat inherited from the
template stage, and the §27 knob doors run on events inside it
(`DEFAULT_KNOB_DOOR_F_OPEN` / `_B_OPEN`). Dropping a donor-derived file over the
top would take every door on the island with it. So the donor event is appended
and every index it carries is renumbered into the receiver's tables.

WHY THIS IS POSSIBLE AT ALL (№149/№151)
---------------------------------------
The two engines share the event container. The donor's file parses cleanly with
the receiver's own structs — `event_binary_data_header` (0x40), `dEvDtEvent_c`
(0xB0), `dEvDtStaff_c` (0x50), `dEvDtCut_c` (0x50), `dEvDtData_c` (0x40) — so
this is a data merge, not a format conversion.

WHAT GETS COPIED
----------------
The dependency closure of one event, and nothing else:

    event -> its staff[] -> each staff's cut chain (via mNext)
          -> each cut's data chain (via mNext)
          -> each data node's payload in fData / iData / sData

Every index rewritten on the way in. Nothing donor-authored enters the repo:
output is written to the mod folder only.

Usage:
    python merge_event.py <event-name> [--dry-run]
"""
from __future__ import annotations

import io
import os
import struct
import sys
from pathlib import Path

import grow_rdl01_stg as g
from adapt_bdl_arcs import be32, yaz0_dec

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
DONOR_STAGE = Path("D:/XXXXXXX/Ex WW/files/res/Stage/sea/Stage.arc")
TARGET_STG = MOD / "files" / "res" / "Stage" / "F_DL01" / "STG_00.arc"  # DEFAULT (exterior); --target overrides. See §158 note below.

HDR = 0x40
ESZ, SSZ, CSZ, DSZ = 0xB0, 0x50, 0x50, 0x40


def arc_member(path: Path, want: str) -> bytes | None:
    d = bytearray(path.read_bytes())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)
    data_abs = 0x20 + be32(d, 0x0C)
    info = 0x20
    n = be32(d, info + 8)
    ent = info + be32(d, info + 0x0C)
    strs = info + be32(d, info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", d, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", d, e + 6)[0]
        nm = bytes(d[strs + no : d.index(b"\0", strs + no)]).decode("ascii", "replace")
        if nm.lower() == want.lower():
            off, size = be32(d, e + 8), be32(d, e + 12)
            return bytes(d[data_abs + off : data_abs + off + size])
    return None


class EventFile:
    """Reader/writer over the shared event container."""

    def __init__(self, raw: bytes):
        self.raw = raw
        h = struct.unpack_from(">IiIiIiIiIiIiIi", raw, 0)
        (self.etop, self.enum, self.stop, self.snum, self.ctop, self.cnum,
         self.dtop, self.dnum, self.ftop, self.fnum, self.itop, self.inum,
         self.sdtop, self.sdnum) = h

    def rec(self, top: int, idx: int, size: int) -> bytearray:
        return bytearray(self.raw[top + idx * size : top + (idx + 1) * size])

    def name(self, top: int, idx: int, size: int) -> str:
        r = self.rec(top, idx, size)
        return bytes(r[:32] if size != SSZ else r[:8]).split(b"\0")[0].decode(
            "ascii", "replace"
        )

    def find_event(self, want: str) -> int:
        for i in range(self.enum):
            if self.name(self.etop, i, ESZ) == want:
                return i
        return -1


def collect(src: EventFile, ev_idx: int):
    """Walk the event's closure. Returns the records plus payload slices."""
    ev = src.rec(src.etop, ev_idx, ESZ)
    nstaff = struct.unpack_from(">i", ev, 0x7C)[0]

    staff_ids, cut_ids, data_ids = [], [], []
    for k in range(nstaff):
        s_idx = struct.unpack_from(">i", ev, 0x2C + k * 4)[0]
        if s_idx < 0 or s_idx >= src.snum:
            continue
        staff_ids.append(s_idx)
        st = src.rec(src.stop, s_idx, SSZ)
        c = struct.unpack_from(">i", st, 0x30)[0]
        guard = 0
        while 0 <= c < src.cnum and guard < 512:
            guard += 1
            if c not in cut_ids:
                cut_ids.append(c)
            cut = src.rec(src.ctop, c, CSZ)
            d = struct.unpack_from(">i", cut, 0x38)[0]
            dguard = 0
            while 0 <= d < src.dnum and dguard < 512:
                dguard += 1
                if d not in data_ids:
                    data_ids.append(d)
                d = struct.unpack_from(">i", src.rec(src.dtop, d, DSZ), 0x30)[0]
            c = struct.unpack_from(">i", cut, 0x3C)[0]
    return ev, staff_ids, cut_ids, data_ids


def report_offsetpos(ef: "EventFile", ev_name: str) -> None:
    """Print the merged event's OffsetPos next to its camera target.

    WHY THIS IS IN THE TOOL AND NOT A NOTE
    --------------------------------------
    `OffsetPos` is handed straight to `dDemo_c::start(demo_data, xyzdata, ...)`
    and is the origin the storyboard stages its CAST from. It crosses over from
    the donor VERBATIM, and donor world coordinates are not receiver world
    coordinates. Outset's opening carries -220000/0/320000 while the island sits
    near -195000 — ~24,600 units out in X. A cast staged off the wrong origin
    lands in open ocean, which on screen is indistinguishable from "the actor
    never appears" (ledger №165, №175 — raised once, then forgotten for ten
    entries while the symptom was chased somewhere else).

    The camera proves nothing about the cast: Center/Eye come from a DIFFERENT
    field and are usually already correct, so a scene can frame the right spot
    while the performers are miles away. Printing both side by side is the whole
    point — a human can see the disagreement in one line instead of rediscovering
    it per island.
    """
    cam, off = None, None
    for d in range(ef.dnum):
        r = ef.rec(ef.dtop, d, DSZ)
        nm = bytes(r[:32]).split(b"\0")[0].decode("ascii", "replace")
        ty, di, num, _ = struct.unpack_from(">iiii", r, 0x24)
        if ty != 1:  # VEC only
            continue
        vals = struct.unpack_from(">3f", ef.raw, ef.ftop + di * 4)
        if nm == "OffsetPos":
            off = vals
        elif nm in ("Center", "Eye") and cam is None:
            cam = (nm, vals)

    print("\n--- OffsetPos check (affects EVERY Great Sea space) ---")
    if off is None:
        print("  no OffsetPos in this event — nothing to reconcile")
        return
    print(f"  OffsetPos      = ({off[0]:.0f}, {off[1]:.0f}, {off[2]:.0f})   <- cast origin")
    if cam is not None:
        nm, c = cam
        print(f"  camera {nm:<7}= ({c[0]:.0f}, {c[1]:.0f}, {c[2]:.0f})   <- usually already correct")
        dx, dz = abs(off[0] - c[0]), abs(off[2] - c[2])
        if dx > 5000 or dz > 5000:
            print(f"  ** MISMATCH: dX={dx:.0f} dZ={dz:.0f} **")
            print("  The cast will stage away from where the camera looks. Reconcile OffsetPos")
            print("  against the receiver-space position of the scene. It is DATA - no rebuild.")
        else:
            print(f"  agreement within dX={dx:.0f} dZ={dz:.0f} — cast should stage on camera")
    else:
        print("  no camera Center/Eye found to compare against — reconcile by hand")


def main() -> int:
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    ev_name = sys.argv[1]
    dry = "--dry-run" in sys.argv

    # №237 (SUPERSEDED 2026-07-26 §158 — dated per §106 stale-doctrine rule,
    # NOT deleted; this is the 2nd catch of this failure class, vfuku was 1st):
    #   ORIGINAL (true only for the ROOM-LANE build era): "our interiors are BG
    #   mounts inside F_DL01, so the TARGET stays the F_DL01 stage list for
    #   every donor stage." NO LONGER TRUE.
    #   §158: the interior entry now does a NATIVE STAGE TRANSPORT — the log
    #   reads `enter EXT_BG1 → transport=stage host='R_DL01' ... native
    #   setNextStage`. So an event that fires inside an interior must live in
    #   THAT stage's event_list (Ba1_Get_Itm → R_DL01/STG_00.arc), not F_DL01.
    #   Symptom that caught it: Ba1_Get_Itm merged into F_DL01 resolved -1 in
    #   the R_DL01 interior (empty stage list) → DEFAULT_GETITEM fallback, no
    #   cutscene. `awake` (exterior, F_DL01) still resolves — it's queried
    #   before the switch, which is why it fooled the first verification.
    #   The target is now SELECTABLE via --target; F_DL01 stays the DEFAULT for
    #   exterior events.
    donor_stage = DONOR_STAGE
    if "--donor" in sys.argv:
        donor_stage = Path(sys.argv[sys.argv.index("--donor") + 1])
    global TARGET_STG
    if "--target" in sys.argv:
        TARGET_STG = Path(sys.argv[sys.argv.index("--target") + 1])

    donor_raw = arc_member(donor_stage, "event_list.dat")
    if donor_raw is None:
        raise SystemExit(f"no event_list.dat in {donor_stage}")
    tgt_members = dict(g.list_rarc_files(TARGET_STG.read_bytes()))
    if "event_list.dat" not in tgt_members:
        # No stage event list yet — and that is SAFE to create here, proven by
        # observation rather than assumption: the §27 knob doors work today
        # while this arc contains no event_list.dat, so DEFAULT_KNOB_DOOR_* must
        # come from a global list (the 106 KB file the log shows loading beside
        # camstyle.dat / camtype.dat, which are global camera data). Adding a
        # STAGE list therefore cannot displace them.
        print(f"{TARGET_STG.name}: no stage event_list.dat — starting an empty one")
        empty = bytearray(HDR)
        for k in range(7):
            struct.pack_into(">Ii", empty, k * 8, HDR, 0)
        tgt_members["event_list.dat"] = bytes(empty)

    src = EventFile(donor_raw)
    dst = EventFile(tgt_members["event_list.dat"])

    ei = src.find_event(ev_name)
    if ei < 0:
        raise SystemExit(f"event '{ev_name}' not found in donor list")
    if dst.find_event(ev_name) >= 0:
        print(f"'{ev_name}' already present in the target — nothing to do")
        return 0

    ev, staff_ids, cut_ids, data_ids = collect(src, ei)
    print(f"donor '{ev_name}': staff={len(staff_ids)} cuts={len(cut_ids)} data={len(data_ids)}")
    print(f"target currently: events={dst.enum} staff={dst.snum} cuts={dst.cnum} data={dst.dnum}")

    s_map = {old: dst.snum + i for i, old in enumerate(staff_ids)}
    c_map = {old: dst.cnum + i for i, old in enumerate(cut_ids)}
    d_map = {old: dst.dnum + i for i, old in enumerate(data_ids)}

    # ---- payload slices, appended wholesale and re-based -------------------
    f_add, i_add, sd_add = bytearray(), bytearray(), bytearray()
    f_base = dst.fnum
    i_base = dst.inum
    sd_base = dst.sdnum

    new_data = []
    for old in data_ids:
        r = src.rec(src.dtop, old, DSZ)
        ty, di, num, nxt = struct.unpack_from(">iiii", r, 0x24)
        if ty == 4:  # STRING -> sData (byte offset)
            end = src.raw.index(b"\0", src.sdtop + di)
            blob = src.raw[src.sdtop + di : end + 1]
            new_di = sd_base + len(sd_add)
            sd_add += blob
        elif ty == 3:  # INT -> iData (element index)
            new_di = i_base + len(i_add) // 4
            i_add += src.raw[src.itop + di * 4 : src.itop + (di + num) * 4]
        else:  # FLOAT / VEC -> fData
            cnt = num * (3 if ty == 1 else 1)
            new_di = f_base + len(f_add) // 4
            f_add += src.raw[src.ftop + di * 4 : src.ftop + (di + cnt) * 4]
        struct.pack_into(">i", r, 0x28, new_di)
        struct.pack_into(">i", r, 0x30, d_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x20, d_map[old])
        new_data.append(r)

    new_cuts = []
    for old in cut_ids:
        r = src.rec(src.ctop, old, CSZ)
        dtop_i = struct.unpack_from(">i", r, 0x38)[0]
        nxt = struct.unpack_from(">i", r, 0x3C)[0]
        struct.pack_into(">i", r, 0x38, d_map.get(dtop_i, -1) if dtop_i >= 0 else -1)
        struct.pack_into(">i", r, 0x3C, c_map.get(nxt, -1) if nxt >= 0 else -1)
        struct.pack_into(">I", r, 0x24, c_map[old])
        new_cuts.append(r)

    new_staff = []
    for old in staff_ids:
        r = src.rec(src.stop, old, SSZ)
        sc = struct.unpack_from(">i", r, 0x30)[0]
        struct.pack_into(">i", r, 0x30, c_map.get(sc, -1) if sc >= 0 else -1)
        struct.pack_into(">i", r, 0x38, c_map.get(sc, -1) if sc >= 0 else -1)
        struct.pack_into(">I", r, 0x24, s_map[old])
        new_staff.append(r)

    new_ev = bytearray(ev)
    nstaff = struct.unpack_from(">i", new_ev, 0x7C)[0]
    for k in range(nstaff):
        old = struct.unpack_from(">i", new_ev, 0x2C + k * 4)[0]
        struct.pack_into(">i", new_ev, 0x2C + k * 4, s_map.get(old, -1))
    struct.pack_into(">I", new_ev, 0x20, dst.enum)

    if dry:
        print("dry run — nothing written")
        return 0

    # ---- rebuild: header + 7 chunks, each concatenated old+new -------------
    def chunk(top, num, size):
        return dst.raw[top : top + num * size]

    events = chunk(dst.etop, dst.enum, ESZ) + bytes(new_ev)
    staff = chunk(dst.stop, dst.snum, SSZ) + b"".join(bytes(x) for x in new_staff)
    cuts = chunk(dst.ctop, dst.cnum, CSZ) + b"".join(bytes(x) for x in new_cuts)
    datas = chunk(dst.dtop, dst.dnum, DSZ) + b"".join(bytes(x) for x in new_data)
    fdat = dst.raw[dst.ftop : dst.ftop + dst.fnum * 4] + bytes(f_add)
    idat = dst.raw[dst.itop : dst.itop + dst.inum * 4] + bytes(i_add)
    sdat = dst.raw[dst.sdtop : dst.sdtop + dst.sdnum] + bytes(sd_add)

    out = bytearray(HDR)
    offs = []
    cur = HDR
    for blob in (events, staff, cuts, datas, fdat, idat, sdat):
        offs.append(cur)
        out += blob
        while len(out) % 4:
            out.append(0)
        cur = len(out)
    counts = [dst.enum + 1, dst.snum + len(new_staff), dst.cnum + len(new_cuts),
              dst.dnum + len(new_data), dst.fnum + len(f_add) // 4,
              dst.inum + len(i_add) // 4, dst.sdnum + len(sd_add)]
    for k in range(7):
        struct.pack_into(">Ii", out, k * 8, offs[k], counts[k])

    # ---- verify by re-reading with the same reader -------------------------
    chk = EventFile(bytes(out))
    idx = chk.find_event(ev_name)
    if idx < 0:
        raise SystemExit("merge produced a file the reader cannot find the event in")
    print(f"merged: events={chk.enum} staff={chk.snum} cuts={chk.cnum} data={chk.dnum} "
          f"({len(out)} bytes)  '{ev_name}' at index {idx}")

    report_offsetpos(chk, ev_name)

    files = [(n, b) for n, b in g.list_rarc_files(TARGET_STG.read_bytes())
             if n != "event_list.dat"]
    files.append(("event_list.dat", bytes(out)))
    bak = TARGET_STG.with_suffix(TARGET_STG.suffix + ".pre152-bak")
    if not bak.is_file():
        bak.write_bytes(TARGET_STG.read_bytes())
        print(f"backup -> {bak.name}")
    TARGET_STG.write_bytes(g.pack_rarc(files))
    print(f"wrote {TARGET_STG}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
