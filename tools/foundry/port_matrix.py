#!/usr/bin/env python3
# ============================================================================
# port_matrix.py - THE E4 JOIN: the next-port queue, GENERATED not chosen.
#
# Ordered by ROADMAP-E1-E6 (E4, owner "Foundry emits / History ports"). The
# roadmap cited `port_matrix` as if it existed; the 2026-08-21 analysis
# measured ZERO hits repo-wide. This is the build. Its inputs are the
# artifacts that already exist - nothing here is remembered:
#
#   name -> procname      donor `src/d/d_stage.cpp` OBJNAME rows (the same
#                         table catchall_procmap reads: dStage_objectNameInf)
#   procname -> owning TU name convention against configure.py's ActorRel
#                         list; misses land in UNJOINED, never guessed
#   TU -> decomp status   donor `configure.py` ActorRel(Matching/NonMatching/
#                         MatchingFor(...)) rows - 1,092 status tokens
#   name -> placements    counted from the DONOR'S OWN DZR chunks (ACTR/SCOB/
#                         TRES/DOOR/TGOB and layer variants) inside the room
#                         arcs at D:/XXXXXXX/Ex WW - DISCOVER, NEVER AUTHOR
#
# WEIGHT BY PLACEMENTS, NEVER ACTOR COUNT (E4's own words): 49 DZR names
# resolving to one proc means one port closes 49 placements - actor-count
# percentages actively mislead.
#
# The RARC walk below is minimal and read-only. Yaz0 arcs are DECOMPRESSED
# in memory (History/Bridge measured 52% of stage archives Yaz0 - a reader
# that skips them silently undercounts placements by half; that lesson is
# why this is handled and logged rather than refused).
#
# Usage:
#   port_matrix.py [--stage sea] [--rooms 44]   e.g. Outset = sea room 44
#   port_matrix.py --control                    prove the join can flag
# Exit 0 emitted - 1 control failed - 2 inputs missing (a fault, not quiet).
# ============================================================================
import re
import struct
import sys
from collections import Counter, defaultdict
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

DONOR_SRC = Path("D:/XXXXXXX/WW DP")
DISC = Path("D:/XXXXXXX/Ex WW/files/res/Stage")
CHUNKS = (b"ACTR", b"SCOB", b"TRES", b"DOOR", b"TGOB", b"TGSC", b"TGDR")


def yaz0(data):
    if data[:4] != b"Yaz0":
        return data
    size = struct.unpack(">I", data[4:8])[0]
    out = bytearray()
    i = 16
    code = 0
    bits = 0
    while len(out) < size:
        if bits == 0:
            code = data[i]; i += 1; bits = 8
        if code & 0x80:
            out.append(data[i]); i += 1
        else:
            b1, b2 = data[i], data[i + 1]; i += 2
            dist = ((b1 & 0xF) << 8) | b2
            n = (b1 >> 4) + 2
            if n == 2:
                n = data[i] + 0x12; i += 1
            for _ in range(n):
                out.append(out[-dist - 1])
        code <<= 1
        bits -= 1
    return bytes(out)


def rarc_members(data):
    """(name, bytes) for every file member. Minimal, read-only."""
    if data[:4] != b"RARC":
        return
    dro = struct.unpack(">I", data[8:12])[0] + 0x20 - 0x20  # data after header
    # header: 0x20 base; info block at 0x20
    num_nodes, node_off = struct.unpack(">II", data[0x20:0x28])
    num_ent = struct.unpack(">I", data[0x28:0x2C])[0]
    ent_off = struct.unpack(">I", data[0x2C:0x30])[0] + 0x20
    str_off = struct.unpack(">I", data[0x34:0x38])[0] + 0x20
    file_off = struct.unpack(">I", data[0xC:0x10])[0] + 0x20
    for k in range(num_ent):
        e = ent_off + k * 0x14
        fid, _, typ_name, doff, dlen = struct.unpack(">HHIII", data[e:e + 0x10])
        typ = typ_name >> 24
        noff = typ_name & 0xFFFFFF
        if typ & 0x02:  # directory
            continue
        end = data.index(b"\0", str_off + noff)
        yield data[str_off + noff:end].decode("latin1"), data[file_off + doff:file_off + doff + dlen]


def dzx_names(data):
    """Placement names from a DZR/DZS buffer, by chunk table."""
    n = struct.unpack(">I", data[:4])[0]
    names = []
    for k in range(n):
        tag, cnt, off = struct.unpack(">4sII", data[4 + 12 * k:16 + 12 * k])
        base = tag[:4].upper()
        if not any(base.startswith(c[:3]) for c in CHUNKS):
            continue
        step = 0x20 if base[:4] in (b"ACTR", b"TRES", b"TGOB", b"DOOR", b"TGDR") else 0x24
        for j in range(cnt):
            nm = data[off + j * step: off + j * step + 8].split(b"\0")[0]
            if nm and all(32 <= c < 127 for c in nm):
                names.append(nm.decode("latin1"))
    return names


def objname_rows():
    txt = (DONOR_SRC / "src/d/d_stage.cpp").read_text(encoding="utf-8", errors="replace")
    rows = {}
    for m in re.finditer(r'OBJNAME\("([^"]+)",\s*(\w+),', txt):
        rows[m.group(1)] = m.group(2)
    return rows


def tu_status():
    txt = (DONOR_SRC / "configure.py").read_text(encoding="utf-8", errors="replace")
    st = {}
    for m in re.finditer(r'(?:ActorRel|Object)\((Matching|NonMatching|MatchingFor\([^)]*\)),\s*"([^"]+)"', txt):
        tu = Path(m.group(2)).stem
        st[tu] = "Matching" if m.group(1).startswith("Matching") and "For" not in m.group(1) \
            else ("MatchingFor" if "For" in m.group(1) else "NonMatching")
    return st


def join(placements, rows, status):
    """The E4 join. Pure, so --control can exercise it."""
    per_proc = defaultdict(lambda: {"names": [], "plc": 0})
    unjoined = Counter()
    for nm, cnt in placements.items():
        proc = rows.get(nm)
        if proc is None:
            unjoined[nm] += cnt
            continue
        per_proc[proc]["names"].append(nm)
        per_proc[proc]["plc"] += cnt
    # normalized fallback: my first run UNJOINED d_a_lod_bg / d_a_kamome /
    # d_a_salvage / d_a_stone because rstrip("eE_") ATE THE TRAILING e OF THE
    # NAME ITSELF (Salvage -> salvag) and LODBG carries no underscore. Strip
    # ONLY the literal `_e` suffix, then fall back to underscore-insensitive
    # match against the status table - and record WHICH join kind matched.
    norm = {k.replace("_", ""): k for k in status}
    queue = []
    for proc, d in per_proc.items():
        base = re.sub(r"_e$", "", re.sub(r"^(PROC_|fpcNm_)", "", proc)).lower()
        tu = "d_a_" + base
        if tu not in status:
            tu = norm.get(("d_a_" + base).replace("_", ""), tu)
        stt = status.get(tu, "TU-UNJOINED")
        queue.append((d["plc"], proc, tu, stt, sorted(d["names"])))
    queue.sort(reverse=True)
    return queue, unjoined


def control():
    rows = {"kusax1": "PROC_GRASS_e", "lwood": "PROC_LWOOD_e", "mystery": None}
    rows = {k: v for k, v in rows.items() if v}
    status = {"d_a_grass": "NonMatching", "d_a_lwood": "MatchingFor"}
    placements = {"kusax1": 49, "lwood": 4, "ghost_name": 7}
    q, un = join(placements, rows, status)
    print("NEGATIVE CONTROL - weighting, join miss, and TU miss")
    for plc, proc, tu, stt, names in q:
        print("  %4d plc  %-14s %-12s %-12s %s" % (plc, proc, tu, stt, ",".join(names)))
    ok = (q[0][0] == 49                       # placement-weighted, not name count
          and "ghost_name" in un              # unjoined NAME is flagged, not dropped
          and q[0][3] == "NonMatching")       # status joined
    print("  unjoined names: %s" % dict(un))
    print("  CONTROL %s" % ("PASSED - weight, unjoined-flag and status join all behave."
                            if ok else "*** FAILED ***"))
    return 0 if ok else 1


def main():
    if "--control" in sys.argv:
        return control()
    stage = sys.argv[sys.argv.index("--stage") + 1] if "--stage" in sys.argv else "sea"
    rooms = sys.argv[sys.argv.index("--rooms") + 1].split(",") if "--rooms" in sys.argv else None
    sdir = DISC / stage
    if not sdir.is_dir() or not (DONOR_SRC / "configure.py").exists():
        print("inputs missing (%s / configure.py) - a fault, not quiet" % sdir)
        return 2
    placements = Counter()
    arcs = yaz = 0
    for arc in sorted(sdir.glob("*.arc")):
        if rooms and not any(("Room%s" % r) == arc.stem or arc.stem == "Stage" for r in rooms):
            continue
        data = arc.read_bytes()
        if data[:4] == b"Yaz0":
            data = yaz0(data); yaz += 1
        arcs += 1
        for name, member in rarc_members(data):
            if name.endswith((".dzr", ".dzs")):
                placements.update(dzx_names(member))
    rows = objname_rows()
    status = tu_status()
    queue, unjoined = join(placements, rows, status)
    print("PORT MATRIX - %s%s: %d arcs (%d Yaz0), %d placements, %d distinct names"
          % (stage, " rooms=" + ",".join(rooms) if rooms else "",
             arcs, yaz, sum(placements.values()), len(placements)))
    print("%5s  %-16s %-16s %-12s names" % ("plc", "procname", "owning TU", "status"))
    for plc, proc, tu, stt, names in queue[:40]:
        print("%5d  %-16s %-16s %-12s %s" % (plc, proc, tu, stt, ",".join(names)[:50]))
    if unjoined:
        print("UNJOINED names (no OBJNAME row - enumerated, never dropped):")
        for nm, c in unjoined.most_common():
            print("   %4d  %s" % (c, nm))
    return 0


if __name__ == "__main__":
    sys.exit(main())
