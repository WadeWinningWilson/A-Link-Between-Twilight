#!/usr/bin/env python3
# ===========================================================================
# room_layout.py — the ROOM-RELATIONSHIP tell (§834 follow-up, user-requested).
#
# WHY: Outset's Orca/Sturgeon house is TWO ROOMS SHARING ONE COORDINATE SPACE,
# stacked in Y (Room1's XZ sits entirely inside Room0's; Y bands 0..512 vs
# -84..1000). Nothing told us. It surfaced only because a room-index failure
# produced symptoms that made no sense for separated rooms, and because the
# user remembered hearing about it. A stage whose rooms overlap behaves
# DIFFERENTLY under a wrong room index — the player lands in the right place
# on the wrong floor instead of somewhere obviously broken.
#
# WHAT IT DOES: reads every room's collision (room.dzb) bounding box straight
# off the donor disc and reports the RELATIONSHIP between rooms of a stage.
#
# IT REPORTS RELATIONSHIPS, NOT A FIXED TAXONOMY. The user's ask was explicit:
# "there could be rooms with maybe stacks vertically/horizontally OR some other
# unique layouts we are yet to be privy to." So the output states the measured
# overlap on each axis and lets an unknown layout show itself as an unfamiliar
# combination, rather than being forced into a known bucket.
#
# Usage:
#   room_layout.py [iso] [stage ...] [--flagged] [--verbose]
#     iso defaults from the receiver config (backend.extraIsoPath) / WW_ISO;
#     stages default to every res/Stage/* dir; --flagged = interacting only
# Read-only. Never writes. Exit 0 always (a census, not a gate).
#
# FOUNDRY-OWNED since tale §838 (§837 handover). Hardening receipts:
#  · Room enumeration comes from the FST, not range(8) — the prototype's cap
#    measured sea's layout on 8 of its 50 rooms (its own headline undercounted)
#  · ISO defaults from the receiver config — the key every disc instrument uses
#  · NOT-COVERED accounting (№31-C): rooms skipped for unreadable/missing
#    room.dzb are COUNTED AND NAMED, never silently green
#  · stage_pairs()/stage_summary() are the importable API — port_planner folds
#    layout class into PORT-QUEUE so a stage's shape is known BEFORE porting
#  · (Recovery note: the §837 prototype was truncated by a failed patch run —
#    open('w') truncates before the exception, the recorded shell-hazard
#    lesson — and reconstructed verbatim from the reviewed copy, hardened.)
# ===========================================================================
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import ww_disc as wd


def room_box(f, boot, stage, room):
    """Bounding box + triangle count of one room's collision, or None."""
    try:
        arc = wd.iso_read_file(f, boot, "res/Stage/%s/%s.arc" % (stage, room))
    except Exception:
        return None
    dzb = [i[1] for i in wd.rarc_list(arc)
           if str(i[0]).lower().endswith("room.dzb")]
    if not dzb:
        return None
    d = dzb[0]
    if len(d) < 16:
        return None
    v_num, v_off, t_num, _ = struct.unpack(">IIII", d[0:16])
    if v_num == 0 or v_off + v_num * 12 > len(d):
        return None
    pts = [struct.unpack(">fff", d[v_off + i * 12: v_off + i * 12 + 12])
           for i in range(v_num)]
    xs = [p[0] for p in pts]
    ys = [p[1] for p in pts]
    zs = [p[2] for p in pts]
    return {"room": room, "tris": t_num,
            "x": (min(xs), max(xs)), "y": (min(ys), max(ys)),
            "z": (min(zs), max(zs))}


def overlap(a, b):
    """Overlap length of two (lo,hi) spans; negative means a gap."""
    return min(a[1], b[1]) - max(a[0], b[0])


def span(a):
    return a[1] - a[0]


def relate(a, b):
    """Describe how two room boxes sit relative to each other, per axis."""
    ox, oy, oz = overlap(a["x"], b["x"]), overlap(a["y"], b["y"]), overlap(a["z"], b["z"])
    # Footprint = the XZ plane; height = Y. A building's floors share footprint.
    foot = ox > 0 and oz > 0
    # "Contained" = one footprint sits wholly inside the other's.
    inside = (a["x"][0] >= b["x"][0] and a["x"][1] <= b["x"][1] and
              a["z"][0] >= b["z"][0] and a["z"][1] <= b["z"][1])
    rinside = (b["x"][0] >= a["x"][0] and b["x"][1] <= a["x"][1] and
               b["z"][0] >= a["z"][0] and b["z"][1] <= a["z"][1])
    identical = (a["x"] == b["x"] and a["y"] == b["y"] and a["z"] == b["z"])

    if identical:
        kind = "IDENTICAL-BOX"
    elif foot and oy <= 0:
        kind = "STACKED"          # shares footprint, separated in height
    elif foot and (inside or rinside):
        kind = "NESTED"           # one footprint inside the other, heights meet
    elif foot:
        kind = "CO-LOCATED"       # overlaps on all three axes
    elif oy > 0 and (ox > 0 or oz > 0):
        kind = "ADJACENT"         # touches on some axes, not a shared footprint
    else:
        kind = "disjoint"
    return kind, (ox, oy, oz)


def default_iso():
    import json
    import os
    pth = os.environ.get("WW_ISO", "")
    if not pth:
        try:
            cfg = os.path.expandvars(r"%APPDATA%\TwilitRealm\Dusklight\config.json")
            pth = json.load(open(cfg, encoding="utf-8")).get("backend.extraIsoPath", "")
        except OSError:
            pth = ""
    return pth or None


def stage_rooms(f, boot, stage=None):
    """{stage: [RoomN names]} straight from the FST — the prototype's
    range(8) cap measured sea on 8 of its 50 rooms."""
    import re
    out = {}
    for path, _off, _len in wd.fst_walk(f, boot):
        m = re.match(r"res/Stage/([^/]+)/(Room\d+)\.arc$", str(path))
        if m and (stage is None or m.group(1) == stage):
            out.setdefault(m.group(1), []).append(m.group(2))
    for s in out:
        out[s].sort(key=lambda r: int(r[4:]))
    return out


def stage_pairs(f, boot, stage, rooms=None):
    """(boxes, pairs, unreadable) for one stage — the importable API."""
    if rooms is None:
        rooms = stage_rooms(f, boot, stage).get(stage, [])
    boxes, unreadable = [], []
    for r in rooms:
        b = room_box(f, boot, stage, r)
        if b:
            boxes.append(b)
        else:
            unreadable.append(r)
    pairs = []
    for i in range(len(boxes)):
        for j in range(i + 1, len(boxes)):
            kind, ov = relate(boxes[i], boxes[j])
            if kind != "disjoint":
                pairs.append((boxes[i], boxes[j], kind, ov))
    return boxes, pairs, unreadable


def stage_summary(f, boot, stage, rooms=None):
    """One-line layout class for a stage, e.g. 'NESTED x1' — planner food."""
    boxes, pairs, unreadable = stage_pairs(f, boot, stage, rooms)
    if len(boxes) < 2:
        return "single-room" if len(boxes) == 1 else "UNREADABLE"
    kinds = {}
    for _a, _b, kind, _ov in pairs:
        kinds[kind] = kinds.get(kind, 0) + 1
    if not kinds:
        return "disjoint rooms"
    s = " ".join("%s x%d" % (k, v) for k, v in
                 sorted(kinds.items(), key=lambda kv: -kv[1]))
    if unreadable:
        s += "  (%d room(s) unreadable)" % len(unreadable)
    return s


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flagged_only = "--flagged" in sys.argv
    verbose = "--verbose" in sys.argv
    iso = None
    if args and (args[0].lower().endswith(".iso") or "/" in args[0] or "\\" in args[0]):
        iso, args = args[0], args[1:]
    else:
        iso = default_iso()
    if not iso:
        print("usage: room_layout.py [iso] [stage ...] [--flagged]  "
              "(iso defaults from receiver config / WW_ISO)")
        return 0
    f, boot = wd.iso_open(iso)

    room_map = stage_rooms(f, boot)
    stages = args if args else sorted(room_map)

    print("ROOM-LAYOUT CENSUS — relationships between rooms of a stage")
    print("  STACKED    = shared XZ footprint, separated in Y (floors of one building)")
    print("  NESTED     = one footprint inside the other, heights meet")
    print("  CO-LOCATED = overlaps on all three axes")
    print("  IDENTICAL-BOX / ADJACENT / disjoint")
    print()
    n_flagged = 0
    not_covered = []
    for stage in stages:
        rooms = room_map.get(stage, [])
        boxes, pairs, unreadable = stage_pairs(f, boot, stage, rooms)
        if unreadable:
            not_covered.append("%s(%d)" % (stage, len(unreadable)))
        if len(boxes) < 2:
            continue
        if not pairs and flagged_only:
            continue
        if pairs:
            n_flagged += 1
        # SUMMARISE, do not enumerate. N rooms give N(N-1)/2 pairs — Abship's 8
        # rooms produce 28 lines, and a census that shouts is one lanes learn to
        # skip (the S389b calibration lesson). One line per stage; --verbose for
        # the pair list when a stage is being investigated.
        kinds = {}
        for _a, _b, kind, _ov in pairs:
            kinds[kind] = kinds.get(kind, 0) + 1
        summary = " ".join("%s x%d" % (k, v) for k, v in
                           sorted(kinds.items(), key=lambda kv: -kv[1]))
        print("%-12s %d rooms  %s" % (stage, len(boxes), summary))
        if verbose:
            for b in boxes:
                print("    %-6s tris=%-5d X %8.0f..%-8.0f Y %8.0f..%-8.0f Z %8.0f..%-8.0f"
                      % (b["room"], b["tris"], b["x"][0], b["x"][1],
                         b["y"][0], b["y"][1], b["z"][0], b["z"][1]))
            for a, b, kind, ov in pairs:
                print("    ** %s <-> %s : %s  (overlap X=%.0f Y=%.0f Z=%.0f)"
                      % (a["room"], b["room"], kind, ov[0], ov[1], ov[2]))
    print("stages with interacting rooms: %d" % n_flagged)
    if not_covered:
        # №31-C: absent is UNKNOWN, never PASS — unreadable rooms are named.
        print("NOT COVERED (unreadable room.dzb): %s" % " ".join(not_covered))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
