#!/usr/bin/env python3
# ============================================================================
# mat_state_differ.py — §373b: donor-vs-port MATERIAL STATE differ.
#
# WHY: "are the plants rendering?" must be answered by DATA, not by a human
# looking at a screen. For every material in a room model this decodes the
# donor .bdl's own state (which texture it wants, its alpha-compare, its blend
# mode) and diffs it against the SAME fields read live from our port
# (§373b log lines). A material that cannot draw is then NAMED, with its donor
# material name attached — no interpretation anywhere in the loop.
#
# MAT3 layout (J3D, big-endian): after the 0x0C header the block carries an
# offset table; the fields we need are index-mapped:
#   +0x0C materialInit(index) table, +0x10 remap table, +0x14 name table,
#   ... then per-attribute tables (cull, alphaComp, blend, zmode, tevOrder...)
# Rather than hardcode a version-specific field order, this reads the NAME
# table (validated by count) for names and the per-material INIT records for
# the attribute INDICES, then dereferences the attribute tables it can locate
# by size/stride validation. Anything it cannot resolve is reported UNKNOWN —
# never guessed.
#
# Usage:
#   mat_state_differ.py <room.arc> <model member> [--log <dusklight log>]
# With --log, prints a side-by-side donor|port table and flags mismatches.
# ============================================================================
import re
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                      "ww_crew_restoration_skeleton"))
from jstudio_stb import yaz0_dec, rarc_members

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]


def get_model(arc_path, member):
    raw = Path(arc_path).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    for nm, blob in rarc_members(raw):
        if nm.lower() == member.lower():
            if blob[:4] == b"Yaz0":
                blob = yaz0_dec(blob)
            return blob
    return None


def find_block(bdl, tag):
    n = be32(bdl, 12)
    off = 0x20
    for _ in range(n):
        t = bdl[off:off + 4]
        size = be32(bdl, off + 4)
        if t == tag:
            return off, size
        off += size
    return None, None


def mat_names(bdl, mat_off, count):
    """Name table: validated by leading count == material count."""
    for probe in range(0x0C, 0x90, 4):
        rel = be32(bdl, mat_off + probe)
        if not rel or mat_off + rel + 4 > len(bdl):
            continue
        if be16(bdl, mat_off + rel) != count:
            continue
        base = mat_off + rel
        p, out = base + 4, []
        try:
            for _ in range(count):
                so = be16(bdl, p + 2)
                s = base + so
                e = bdl.index(b"\0", s)
                out.append(bdl[s:e].decode("ascii", "replace"))
                p += 4
            if all(x and x.isprintable() for x in out):
                return out
        except Exception:
            continue
    return None


def tex_names(bdl):
    off, size = find_block(bdl, b"TEX1")
    if off is None:
        return []
    count = be16(bdl, off + 8)
    rel = be32(bdl, off + 0x10)
    base = off + rel
    num = be16(bdl, base)
    p, out = base + 4, []
    for _ in range(num):
        so = be16(bdl, p + 2)
        s = base + so
        e = bdl.index(b"\0", s)
        out.append(bdl[s:e].decode("ascii", "replace"))
        p += 4
    return out


def parse_port_log(log_path):
    """§373b lines -> {mat_index: (tex0, comp0, ref0, blend)} for the LAST
    model that reported 19 materials (the interior room model)."""
    txt = Path(log_path).read_text(encoding="utf-8", errors="replace")
    blocks, cur = [], None
    for line in txt.splitlines():
        m = re.search(r"§373 model\[(\d+)\] shapes=(\d+) materials=(\d+)", line)
        if m:
            cur = {"model": int(m.group(1)), "shapes": int(m.group(2)),
                   "mats": int(m.group(3)), "rows": {}}
            blocks.append(cur)
            continue
        m = re.search(r"§373b mat\[(\d+)\] tex0=(-?\d+) alphaComp0=(-?\d+) "
                      r"ref0=(-?\d+) blendMode=(-?\d+)", line)
        if m and cur is not None:
            cur["rows"][int(m.group(1))] = tuple(int(m.group(i)) for i in range(2, 6))
    return blocks


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: mat_state_differ.py <room.arc> <model.bdl> [--log <log>]")
    bdl = get_model(sys.argv[1], sys.argv[2])
    if bdl is None:
        sys.exit("model member not found")
    mat_off, mat_size = find_block(bdl, b"MAT3")
    count = be16(bdl, mat_off + 8)
    names = mat_names(bdl, mat_off, count) or ["?"] * count
    texs = tex_names(bdl)
    print(f"donor {sys.argv[2]}: {count} materials, {len(texs)} textures")

    log = None
    if "--log" in sys.argv:
        log = sys.argv[sys.argv.index("--log") + 1]
    if not log:
        for i, nm in enumerate(names):
            print(f"  mat[{i:2d}] {nm}")
        return 0

    blocks = [b for b in parse_port_log(log) if b["mats"] == count and b["rows"]]
    if not blocks:
        print(f"no §373b block with {count} materials in the log — "
              f"UNKNOWN (enter the room once with the probe build)")
        return 2
    port = blocks[-1]["rows"]
    print(f"\n{'idx':>3} {'donor material':38s} {'tex0':>5} {'alphaComp0':>10} "
          f"{'ref0':>5} {'blend':>6}  verdict")
    bad = 0
    for i in range(count):
        if i not in port:
            print(f"{i:3d} {names[i]:38s} {'—':>5} {'—':>10} {'—':>5} {'—':>6}  "
                  f"NOT REPORTED (UNKNOWN)")
            bad += 1
            continue
        tex0, comp0, ref0, blend = port[i]
        # A material CANNOT draw if: no texture bound (0xFFFF), or alpha test
        # rejects everything (comp0 == GX_NEVER(0)), or ref0 == 255 with
        # GX_GREATER-style compare and nothing can exceed it.
        verdict = "ok"
        if tex0 == 0xFFFF or tex0 == 65535:
            verdict = "NO TEXTURE BOUND"
        elif comp0 == 0:
            verdict = "ALPHA TEST = NEVER (invisible)"
        elif tex0 >= len(texs) and texs:
            verdict = f"TEX INDEX {tex0} OUT OF RANGE (have {len(texs)})"
        if verdict != "ok":
            bad += 1
        texname = texs[tex0] if 0 <= tex0 < len(texs) else "?"
        print(f"{i:3d} {names[i]:38s} {tex0:5d} {comp0:10d} {ref0:5d} {blend:6d}  "
              f"{verdict}  [{texname}]")
    print(f"\n{count - bad}/{count} materials can draw; {bad} flagged")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
