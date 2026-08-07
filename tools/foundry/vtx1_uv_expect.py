#!/usr/bin/env python3
# ============================================================================
# vtx1_uv_expect.py — §393b: the donor side of the texcoord differ, decoded with
# BLOCK-BOUNDED extents, and an independent cross-check of Housing's §393 figures.
#
# WHY AN INDEPENDENT REIMPLEMENTATION: Housing published u[-18.004, 112.379],
# v[-2.000, 115.125], 598 of 1736 coords outside [0,1] — after retracting a first
# read that ran 322 KB past the array end because TEX0 is the LAST VTX1 array and
# its extent had been allowed to fall back to end-of-file. That is the same
# failure family as my own §392b off-by-one-short. Two lanes that have each just
# published a bad decode should not proceed on one lane's corrected numbers
# unverified, so this decodes the array from scratch and PRINTS THE COMPARISON.
# If the figures agree, both decoders are validated and the differ can be built on
# them; if they disagree, we find out before anyone builds.
#
# EXTENT DERIVATION (the thing that must never be assumed):
#   VTX1 header carries 13 array offsets (POS, NRM, NBT, CLR0, CLR1, TEX0..TEX7),
#   each RELATIVE to the block start. An array ends at the next NON-ZERO offset
#   that follows it, or — for the last populated array — at the BLOCK SIZE from
#   the block header. Never at end-of-file. The count of coords is then derived
#   from that span and the attribute's component count and type size, and this
#   tool refuses to report if the span is not a whole number of coords.
#
# Read-only. Usage: vtx1_uv_expect.py <arc> <model member> [--csv <out.csv>]
# ============================================================================
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                      "ww_crew_restoration_skeleton"))
from jstudio_stb import yaz0_dec, rarc_members

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
s16 = lambda d, o: struct.unpack_from(">h", d, o)[0]

# Housing's §393 corrected figures, for the cross-check gate.
HOUSING = {"n": 1736, "umin": -18.004, "umax": 112.379,
           "vmin": -2.000, "vmax": 115.125, "outside": 598}

COMP_SIZE = {0: 1, 1: 1, 2: 2, 3: 2, 4: 4}   # U8 S8 U16 S16 F32
COMP_NAME = {0: "U8", 1: "S8", 2: "U16", 3: "S16", 4: "F32"}
# The 13 VTX1 array slots, in header order.
SLOTS = ["POS", "NRM", "NBT", "CLR0", "CLR1", "TEX0", "TEX1", "TEX2",
         "TEX3", "TEX4", "TEX5", "TEX6", "TEX7"]


def get_member(arc, want):
    raw = Path(arc).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    for nm, blob in rarc_members(raw):
        if nm.lower() == want.lower():
            return yaz0_dec(blob) if blob[:4] == b"Yaz0" else blob
    return None


def find_block(bdl, tag):
    n = be32(bdl, 12)
    off = 0x20
    for _ in range(n):
        if bdl[off:off + 4] == tag:
            return off, be32(bdl, off + 4)
        off += be32(bdl, off + 4)
    return None, None


def attr_formats(bdl, voff):
    """GXVtxAttrFmt list: {u32 attr, u32 cnt, u32 type, u8 frac, u8[3] pad},
    terminated by attr == 0xFF (GX_VA_NULL)."""
    p = voff + be32(bdl, voff + 0x08)
    out = {}
    while p + 0x10 <= len(bdl):
        a = be32(bdl, p)
        if a == 0xFF:
            break
        out[a] = {"cnt": be32(bdl, p + 4), "type": be32(bdl, p + 8),
                  "frac": bdl[p + 0x0C]}
        p += 0x10
    return out


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: vtx1_uv_expect.py <arc> <model.bdl> [--csv <out.csv>]")
    bdl = get_member(sys.argv[1], sys.argv[2])
    if bdl is None:
        sys.exit("member not found")
    voff, vsize = find_block(bdl, b"VTX1")
    if voff is None:
        sys.exit("no VTX1 block")

    offs = [be32(bdl, voff + 0x0C + i * 4) for i in range(13)]
    print(f"VTX1 at {voff:#x}, block size {vsize:#x} ({vsize} bytes)")
    print("array offsets (relative to block start):")
    for i, o in enumerate(offs):
        print(f"  [{i:2d}] {SLOTS[i]:5s} {o:#08x}" + ("" if o else "   (absent)"))

    tex0_rel = offs[5]
    if not tex0_rel:
        sys.exit("this model has no TEX0 array")

    # ---- EXTENT: next non-zero offset after TEX0, else the BLOCK SIZE ---------
    later = [o for o in offs[6:] if o]
    end_rel = min(later) if later else vsize
    span = end_rel - tex0_rel
    is_last = not later
    print(f"\nTEX0 array: rel {tex0_rel:#x} .. {end_rel:#x}  span {span} bytes"
          f"   ({'LAST populated array -> bounded by BLOCK SIZE' if is_last else 'bounded by the next array'})")
    if is_last:
        print("  (this is exactly the trap Housing hit: falling back to "
              "end-of-file here reads far past the array)")

    fmts = attr_formats(bdl, voff)
    # GX_VA_TEX0 == 13
    f = fmts.get(13)
    if f is None:
        sys.exit("no TEX0 entry in the attribute-format list — UNKNOWN")
    ncomp = 2 if f["cnt"] == 1 else 1          # GX_TEX_ST=1 -> 2 comps, GX_TEX_S=0 -> 1
    csize = COMP_SIZE.get(f["type"])
    if csize is None:
        sys.exit(f"unsupported component type {f['type']} — UNKNOWN")
    print(f"TEX0 format: {COMP_NAME[f['type']]} x{ncomp} frac={f['frac']} "
          f"({csize} bytes per component)")

    stride = csize * ncomp
    if span % stride:
        print(f"\nEXTENT GATE FAILED — span {span} is not a whole number of "
              f"{stride}-byte coords. Refusing to report values.")
        return 2
    n = span // stride
    print(f"EXTENT GATE PASSED — {span}/{stride} = {n} coords")

    base = voff + tex0_rel
    scale = float(1 << f["frac"])
    uvs = []
    for i in range(n):
        o = base + i * stride
        if f["type"] == 3:      # S16
            u = s16(bdl, o) / scale
            v = s16(bdl, o + 2) / scale if ncomp == 2 else 0.0
        elif f["type"] == 4:    # F32
            u = struct.unpack_from(">f", bdl, o)[0]
            v = struct.unpack_from(">f", bdl, o + 4)[0] if ncomp == 2 else 0.0
        else:
            print("  (only S16 and F32 decoded here — UNKNOWN)")
            return 2
        uvs.append((u, v))

    us = [a for a, _ in uvs]
    vs = [b for _, b in uvs]
    outside = sum(1 for a, b in uvs if not (0.0 <= a <= 1.0 and 0.0 <= b <= 1.0))
    print(f"\nDECODED: {len(uvs)} coords")
    print(f"  u [{min(us):.3f}, {max(us):.3f}]")
    print(f"  v [{min(vs):.3f}, {max(vs):.3f}]")
    print(f"  outside [0,1]: {outside} of {len(uvs)} "
          f"({100.0 * outside / len(uvs):.1f}%)")

    # ---- cross-check gate against Housing's published figures -----------------
    print("\nCROSS-CHECK vs Housing §393 (independent decoder, same file):")
    checks = [
        ("count", len(uvs), HOUSING["n"], 0),
        ("u min", min(us), HOUSING["umin"], 0.01),
        ("u max", max(us), HOUSING["umax"], 0.01),
        ("v min", min(vs), HOUSING["vmin"], 0.01),
        ("v max", max(vs), HOUSING["vmax"], 0.01),
        ("outside", outside, HOUSING["outside"], 0),
    ]
    bad = 0
    for name, mine, theirs, tol in checks:
        ok = abs(mine - theirs) <= tol
        bad += 0 if ok else 1
        print(f"  {name:8s} mine={mine!r:>12}  Housing={theirs!r:>12}  "
              f"{'agree' if ok else '*** DISAGREE ***'}")
    if bad:
        print(f"\n{bad} figure(s) disagree. Two independent decoders of the same "
              f"bytes must not differ — one of them is wrong, and NEITHER set "
              f"should be built on until that is resolved.")
    else:
        print("\nAll figures agree. Both decoders are validated against each "
              "other; the donor side of the UV differ can be built on these.")

    if "--csv" in sys.argv:
        out = Path(sys.argv[sys.argv.index("--csv") + 1])
        with out.open("w", encoding="utf-8", newline="") as fh:
            fh.write("index,u,v,outside_unit_square\n")
            for i, (a, b) in enumerate(uvs):
                o = 0 if (0.0 <= a <= 1.0 and 0.0 <= b <= 1.0) else 1
                fh.write(f"{i},{a:.6f},{b:.6f},{o}\n")
        print(f"\nwrote {out} ({len(uvs)} rows) — the reference the live emitted "
              f"coordinates get diffed against")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
