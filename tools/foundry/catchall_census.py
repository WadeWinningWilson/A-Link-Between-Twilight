# ============================================================================
# catchall_census.py — FOUNDRY's arc-contents census for the generic-prop
# ("catch-all") practicality ruling.
#
# Assignment: measure, per distinct object name placed in Outset (sea/Room44),
# WHAT THE NAME'S ARC ACTUALLY CONTAINS, by TYPE rather than by assumption:
#   model      first member whose data begins 'J3D2'  (the spec's step 2)
#   collision  any member ending .dzb                 (step 3)
#   anim       any member ending .bck / .bca          (Housing's Q2)
#   other      everything else, counted not named
#
# DISCOVER, NEVER AUTHOR: nothing here reads an index. Members are classified
# by magic bytes and extension, which is the same discipline the spec demands
# of the handler itself — if the census authored indices it could not honestly
# rule on a design whose whole point is not authoring them.
# ============================================================================
import struct
import sys
from collections import Counter
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")
sys.path.insert(0, str(Path(__file__).parent))
import scls_scan as S

OBJ = Path(r"D:/XXXXXXX/Ex WW/files/res/Object")

# ---- 1. the names, re-derived from the room's own placement chunks --------
raw = S.yaz0((S.ROOT / "sea" / "Room44.arc").read_bytes())
dzr = None
for nm, body in S.rarc_members(raw):
    if nm.lower() == "room.dzr":
        dzr = body
        break

names = Counter()
n = struct.unpack_from(">I", dzr, 0)[0]
for c in range(n):
    tag = dzr[4 + c * 12: 8 + c * 12]
    cnt = struct.unpack_from(">I", dzr, 8 + c * 12)[0]
    off = struct.unpack_from(">I", dzr, 12 + c * 12)[0]
    t = tag.decode("ascii", "replace")
    # ACTR family = ACTR/ACT0..ACTb/TRES-like actor rows (0x20);
    # SCOB family (0x24) carries scale/pad after the actor row.
    if t.startswith("ACT"):
        stride = 0x20
    elif t.startswith("SCO"):
        stride = 0x24
    else:
        continue
    for e in range(cnt):
        eo = off + e * stride
        if eo + 8 > len(dzr):
            break
        nm8 = dzr[eo:eo + 8].split(b"\0")[0].decode("ascii", "replace").strip()
        if nm8:
            names[nm8] += 1

print("distinct names: %d · placements: %d" % (len(names), sum(names.values())))
print()


# ---- 2. per-name arc contents, classified by TYPE ------------------------
def arc_for(name):
    """Donor arcs are named by the object, case varying. Try exact then
    case-insensitive; report MISSING rather than guessing a mapping."""
    p = OBJ / (name + ".arc")
    if p.is_file():
        return p
    low = name.lower()
    for q in OBJ.iterdir():
        if q.stem.lower() == low:
            return q
    return None


def classify(path):
    try:
        data = S.yaz0(path.read_bytes())
    except Exception as e:
        return None, str(e)
    if data[:4] != b"RARC":
        return None, "not RARC"
    model = coll = anim = other = 0
    for nm, body in S.rarc_members(data):
        low = nm.lower()
        if body[:4] == b"J3D2":
            model += 1
        elif low.endswith(".dzb"):
            coll += 1
        elif low.endswith((".bck", ".bca")):
            anim += 1
        else:
            other += 1
    return (model, coll, anim, other), None


rows = []
for name, count in names.most_common():
    p = arc_for(name)
    if p is None:
        rows.append((name, count, None, None, "NO ARC"))
        continue
    res, err = classify(p)
    rows.append((name, count, res, p.name, err))

out = Path(sys.argv[1]) if len(sys.argv) > 1 else None
lines = []
lines.append("%-12s %5s  %5s %5s %5s %6s  %s"
             % ("name", "plc", "model", "dzb", "bck", "other", "arc"))
for name, count, res, arcname, err in rows:
    if res is None:
        lines.append("%-12s %5d  %-30s %s" % (name, count, err or "?", arcname or ""))
    else:
        m, c, a, o = res
        lines.append("%-12s %5d  %5d %5d %5d %6d  %s"
                     % (name, count, m, c, a, o, arcname))
text = "\n".join(lines)
print(text)
if out:
    out.write_text(text + "\n", encoding="utf-8")
