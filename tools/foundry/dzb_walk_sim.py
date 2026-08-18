#!/usr/bin/env python3
# ============================================================================
# dzb_walk_sim.py — REPRODUCE THE RECEIVER'S OWN GROUND QUERY, OFFLINE.
#
# WHY THIS EXISTS (Integrator, 2026-08-17). The Outset trace ended in a
# paradox no existing instrument could split: History cast all 24 spawn
# columns against the parsed dzb and found FLOOR-FACING ROOM TERRAIN under
# every one of them, at the exact record heights — while the running game's
# ground query returns -INF at Link's column and real ground a thousand units
# away. Data right, runtime wrong, and the only remaining suspect is the
# spatial index the engine walks to FIND candidate triangles.
#
# Every measurement so far has been either (a) the DATA read with a fresh
# reader, or (b) the RUNTIME observed through hooks. Neither can localise a
# walk bug: a fresh reader that brute-forces all triangles will always find
# the floor (History's caster does exactly that, which is why it agrees with
# the engine only where the engine happens to work), and a hook can only say
# the answer was wrong.
#
# THIS TOOL IS THE THIRD THING: it re-implements `cBgW::GroundCrossRp` /
# `GroundCrossGrpRp` (d_bg_w.cpp:664-750) **structurally** — the same group
# AABB gates, the same 8-way tree descent, the same block/gnd indirection —
# over the disc's own bytes. Then it runs BOTH strategies at one column:
#
#     BRUTE  : every triangle, no index          (what History measured)
#     WALKED : only triangles the tree walk reaches (what the engine sees)
#
# BRUTE finds floor and WALKED does not  => the INDEX is the bug, and the
#   walk's own trace (printed) names the node where it diverges.
# BOTH find floor                        => the index is sound offline, so
#   the fault is in the runtime's converted copy (rebase/endianness), not in
#   the algorithm — a different fix entirely.
# NEITHER finds floor                    => the column genuinely has no floor
#   and History's cast and mine disagree; that discrepancy is then the lead.
#
# ZERO-BAKE: reads the user's own disc extract, writes nothing but a report.
# The dzb is consumed EXACTLY as the receiver's `cBgD_t` describes it —
# big-endian words, file-relative offsets (pre-ConvDzb form) — because that
# is the form on the disc.
#
# Usage:
#   dzb_walk_sim.py <stage> <room> <X> <Z> [--y START]
#   dzb_walk_sim.py sea 44 -205596 316562
#   dzb_walk_sim.py --selftest        (runs the known-good column too)
# ============================================================================
import struct
import sys
from pathlib import Path

EXTRACT = Path("D:/XXXXXXX/Ex WW/files/res/Stage")
INF = 1000000000.0


def be32(b, o):
    return struct.unpack_from(">i", b, o)[0]


def be16(b, o):
    return struct.unpack_from(">H", b, o)[0]


def bef(b, o):
    return struct.unpack_from(">f", b, o)[0]


# ---------------------------------------------------------------------------
# RARC extraction — minimal, enough to pull one named file out of an arc.
# Yaz0 is handled because several stage arcs ship compressed.
# ---------------------------------------------------------------------------
def yaz0(data):
    if data[:4] != b"Yaz0":
        return data
    size = struct.unpack_from(">I", data, 4)[0]
    out = bytearray(size)
    src, dst, code, bits = 16, 0, 0, 0
    while dst < size:
        if bits == 0:
            code = data[src]
            src += 1
            bits = 8
        if code & 0x80:
            out[dst] = data[src]
            src += 1
            dst += 1
        else:
            b1, b2 = data[src], data[src + 1]
            src += 2
            dist = ((b1 & 0xF) << 8) | b2
            start = dst - dist - 1
            n = b1 >> 4
            if n == 0:
                n = data[src] + 0x12
                src += 1
            else:
                n += 2
            for _ in range(n):
                out[dst] = out[start]
                dst += 1
                start += 1
        code <<= 1
        bits -= 1
    return bytes(out)


def rarc_find(arc, want):
    """Return the bytes of `want` inside a RARC image, or None."""
    arc = yaz0(arc)
    if arc[:4] != b"RARC":
        return None
    header_len = struct.unpack_from(">I", arc, 0x0C)[0]
    info = 0x20
    node_count = struct.unpack_from(">I", arc, info + 0x00)[0]
    file_off = struct.unpack_from(">I", arc, info + 0x0C)[0] + info
    file_count = struct.unpack_from(">I", arc, info + 0x10)[0]
    str_off = struct.unpack_from(">I", arc, info + 0x14)[0] + info
    (void_) = node_count
    for i in range(file_count):
        e = file_off + i * 0x14
        name_off = struct.unpack_from(">H", arc, e + 0x06)[0]
        s = str_off + name_off
        end = arc.index(b"\0", s)
        name = arc[s:end].decode("ascii", "replace")
        if name.lower() == want.lower():
            data_off = struct.unpack_from(">I", arc, e + 0x08)[0]
            data_len = struct.unpack_from(">I", arc, e + 0x0C)[0]
            # RARC's fileDataOffset (@0x0C) is relative to the 0x20-byte
            # header, not absolute. Omitting the +0x20 shifted every dzb field
            # by five slots and made the header read as garbage (v=1016320) —
            # caught because History's measured counts (3722/4221) appeared in
            # the WRONG fields, which is a much better error than plausible
            # numbers in the right ones.
            base = 0x20 + header_len
            return arc[base + data_off: base + data_off + data_len]
    return None


# ---------------------------------------------------------------------------
# cBgD_t, exactly as include/d/d_bg_w.h:98 declares it (disc form).
# ---------------------------------------------------------------------------
class Dzb(object):
    def __init__(self, b):
        self.b = b
        self.v_num = be32(b, 0x00); self.v_tbl = be32(b, 0x04)
        self.t_num = be32(b, 0x08); self.t_tbl = be32(b, 0x0C)
        self.b_num = be32(b, 0x10); self.b_tbl = be32(b, 0x14)
        self.tree_num = be32(b, 0x18); self.tree_tbl = be32(b, 0x1C)
        self.g_num = be32(b, 0x20); self.g_tbl = be32(b, 0x24)
        self.ti_num = be32(b, 0x28); self.ti_tbl = be32(b, 0x2C)
        self.flags = be32(b, 0x30)

    def vtx(self, i):
        o = self.v_tbl + i * 12
        return (bef(self.b, o), bef(self.b, o + 4), bef(self.b, o + 8))

    def tri(self, i):
        # cBgD_Tri_t = 0xA: three u16 vertex indices + u16 group + u16 attr
        o = self.t_tbl + i * 0xA
        return (be16(self.b, o), be16(self.b, o + 2), be16(self.b, o + 4))


def tri_ground_hit(dzb, ti, x, z, start_y):
    """Column test + floor-facing + height, the shape GroundCheck applies."""
    a, bb, c = (dzb.vtx(i) for i in dzb.tri(ti))
    # 2D containment in XZ (barycentric sign test)
    d1 = (x - bb[0]) * (a[2] - bb[2]) - (a[0] - bb[0]) * (z - bb[2])
    d2 = (x - c[0]) * (bb[2] - c[2]) - (bb[0] - c[0]) * (z - c[2])
    d3 = (x - a[0]) * (c[2] - a[2]) - (c[0] - a[0]) * (z - a[2])
    neg = (d1 < 0) or (d2 < 0) or (d3 < 0)
    pos = (d1 > 0) or (d2 > 0) or (d3 > 0)
    if neg and pos:
        return None
    # plane normal -> floor facing means +Y component
    ux, uy, uz = bb[0] - a[0], bb[1] - a[1], bb[2] - a[2]
    vx, vy, vz = c[0] - a[0], c[1] - a[1], c[2] - a[2]
    nx, ny, nz = uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx
    if abs(ny) < 1e-9 or ny <= 0.0:
        return None
    y = a[1] + (-(nx * (x - a[0]) + nz * (z - a[2]))) / ny
    if y > start_y:
        return None
    return y


def brute(dzb, x, z, start_y):
    best, hits = -INF, 0
    for ti in range(dzb.t_num):
        y = tri_ground_hit(dzb, ti, x, z, start_y)
        if y is not None:
            hits += 1
            if y > best:
                best = y
    return best, hits


def walked(dzb, x, z, start_y, trace):
    """Structural re-implementation of the group/tree descent."""
    best = [-INF]
    seen_tris = [0]

    def node(idx, depth):
        if idx < 0 or idx >= dzb.tree_num:
            trace.append("node %d OUT OF RANGE (tree_num=%d)" % (idx, dzb.tree_num))
            return
        o = dzb.tree_tbl + idx * 0x14
        flag = be16(dzb.b, o)
        ids = [be16(dzb.b, o + 4 + k * 2) for k in range(8)]
        if flag & 1:                    # leaf: ids[0] indexes the BLOCK table
            blk = ids[0]
            trace.append("%sleaf node=%d block=%d" % ("  " * depth, idx, blk))
            if blk == 0xFFFF or blk >= dzb.b_num:
                return
            # BLOCK TABLE CORRECTED (History/Bridge, 2026-08-17). My first
            # reading treated the block entry as an INDEX INTO ti_tbl and
            # walked to a 0xFFFF terminator — that is the wrong one of the
            # two readings the table admits, and it mis-addressed the floor
            # (I published node 195/block 158; the truth is node 115/block
            # 92). The entries are TRIANGLE-RANGE STARTS: block N covers
            # triangles entry[N] .. entry[N+1]-1. Their proof is arithmetic:
            # block 92 -> 410..419 (contains tri 410, the floor) while block
            # 158 -> 677..678, and node 195's own AABB excludes the column.
            if blk + 1 > dzb.b_num:
                return
            t_start = be16(dzb.b, dzb.b_tbl + blk * 2)
            t_end = be16(dzb.b, dzb.b_tbl + (blk + 1) * 2)
            if t_start == 0xFFFF or t_end < t_start:
                return
            for t in range(t_start, min(t_end, dzb.t_num)):
                seen_tris[0] += 1
                y = tri_ground_hit(dzb, t, x, z, start_y)
                if y is not None and y > best[0]:
                    best[0] = y
                    trace.append("%s  HIT tri=%d y=%.1f node=%d block=%d"
                                 % ("  " * depth, t, y, idx, blk))
            return
        trace.append("%sbranch node=%d children=%s" %
                     ("  " * depth, idx, [i for i in ids if i != 0xFFFF]))
        for cid in ids:
            if cid != 0xFFFF:
                node(cid, depth + 1)

    # groups: start at group 0 and follow first_child / next_sibling
    def group(g, depth):
        if g == 0xFFFF or g < 0 or g >= dzb.g_num:
            return
        go = dzb.g_tbl + g * 0x34
        # FIELD OFFSETS FROM THE RECEIVER'S OWN cBgD_Grp_t (d_bg_w.h:89-94),
        # not guessed: next_sibling@0x26, first_child@0x28, tree_idx@0x2E.
        tree_idx = be16(dzb.b, go + 0x2E)
        first_child = be16(dzb.b, go + 0x28)
        next_sib = be16(dzb.b, go + 0x26)
        trace.append("%sgroup %d tree=%s child=%s sib=%s" %
                     ("  " * depth, g,
                      "-" if tree_idx == 0xFFFF else tree_idx,
                      "-" if first_child == 0xFFFF else first_child,
                      "-" if next_sib == 0xFFFF else next_sib))
        if tree_idx != 0xFFFF:
            node(tree_idx, depth + 1)
        c = first_child
        while c != 0xFFFF:
            group(c, depth + 1)
            co = dzb.g_tbl + c * 0x34
            c = be16(dzb.b, co + 0x26)

    group(0, 0)
    return best[0], seen_tris[0]


def run(stage, room, x, z, start_y):
    arc = EXTRACT / stage / ("Room%d.arc" % room)
    if not arc.exists():
        print("NO ARC: %s" % arc)
        return 2
    dzb_bytes = rarc_find(arc.read_bytes(), "room.dzb")
    if dzb_bytes is None:
        print("no room.dzb inside %s" % arc)
        return 2
    dzb = Dzb(dzb_bytes)
    print("dzb: v=%d t=%d blk=%d tree=%d grp=%d ti=%d flags=0x%08X"
          % (dzb.v_num, dzb.t_num, dzb.b_num, dzb.tree_num, dzb.g_num,
             dzb.ti_num, dzb.flags & 0xFFFFFFFF))
    by, bh = brute(dzb, x, z, start_y)
    trace = []
    wy, wt = walked(dzb, x, z, start_y, trace)
    print("column (%.0f, %.0f) from y=%.0f" % (x, z, start_y))
    print("  BRUTE  : y=%s  (%d triangles contain the column)"
          % ("-INF" if by == -INF else "%.1f" % by, bh))
    print("  WALKED : y=%s  (%d triangles reached by the index)"
          % ("-INF" if wy == -INF else "%.1f" % wy, wt))
    if by != -INF and wy == -INF:
        print("  VERDICT: THE INDEX IS THE BUG — floor exists, the walk never "
              "reaches it. Trace below names the divergence.")
    elif by != -INF and wy != -INF:
        print("  VERDICT: index sound offline — the runtime's copy is the "
              "suspect (rebase/endianness), not the algorithm.")
    elif by == -INF:
        print("  VERDICT: no floor in this column by brute force either — my "
              "reader and History's disagree; that gap is the lead.")
    lim = len(trace) if "--full" in sys.argv else 40
    for line in trace[:lim]:
        print("    " + line)
    return 0


def main():
    if "--selftest" in sys.argv:
        print("== Link's column (the failing one) ==")
        run("sea", 44, -205596.0, 316562.0, 640.0)
        print()
        print("== the column the engine ANSWERS (control) ==")
        return run("sea", 44, -204600.0, 316340.0, 622.0)
    if len(sys.argv) < 5:
        print(__doc__ or "usage: dzb_walk_sim.py <stage> <room> <X> <Z>")
        return 2
    y = 100000.0
    if "--y" in sys.argv:
        y = float(sys.argv[sys.argv.index("--y") + 1])
    return run(sys.argv[1], int(sys.argv[2]), float(sys.argv[3]),
               float(sys.argv[4]), y)


if __name__ == "__main__":
    raise SystemExit(main())
