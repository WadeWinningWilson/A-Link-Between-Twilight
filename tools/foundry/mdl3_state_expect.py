#!/usr/bin/env python3
# ============================================================================
# mdl3_state_expect.py — §391d: the AUTHORITATIVE donor material state, read
# from MDL3 instead of MAT3.
#
# WHY THIS EXISTS, AND WHY THE PREVIOUS ANSWER WAS BUILT ON THE WRONG BLOCK:
# LinkRM/Room0.arc's model is `J3D2bdl4` and carries an MDL3 block. In a BDL the
# baked per-material GX display list IS the material state the hardware receives;
# MAT3 is the source data it was compiled from and can disagree. That is exactly
# why J3D's PATCHED material path reads its registers out of MDL3 rather than
# MAT3. Decoding MAT3 and calling it "what the donor authors" is therefore not
# safe for a BDL — it is one step upstream of the truth.
#
# So this walks each material's MDL3 packet as a GX display list and pulls the
# registers that decide whether a cutout card can appear:
#   BP 0xF3 TEV_ALPHAFUNC -> ref0, ref1, comp0, comp1, logic op
#   BP 0x00 GEN_MODE      -> cull mode (bits 14-15), texgen/colour counts
#   BP 0x40 PE_ZMODE      -> depth test/func/update
#   BP 0x41 PE_CMODE0     -> blend enable / factors
# Every value printed is a decoded register write found in the donor's own baked
# list — no inference, and anything absent prints NOT SET rather than a default.
#
# STATUS ON FIRST RUN: NOT VALIDATED — DO NOT CITE ITS OUTPUT.
# On LinkRM/Room0 model.bdl, 11 of 19 packets decode to NOT SET for every
# register, and several that do decode give values contradicting MAT3 (cull=FRONT
# on a wall). Either the per-material packet offsets need the MDL3 index table at
# +0x1C, or the packets do not begin on a command boundary this walker
# understands. The walk is deliberately written to STOP at an opcode it does not
# know rather than advance by a guessed stride — so "NOT SET" here means
# undecoded, not absent. Until a validation gate passes (e.g. reproducing a
# register value measured live for a material whose state is independently
# known), this tool's verdict for this file is UNKNOWN, per №31-C.
#
# Read-only. Usage: mdl3_state_expect.py <arc> <model member> [--log <log>]
# With --log, diffs against the live per-material probe lines in a Dusklight log
# (§373b alphaComp/blend + §377 cull), index-aligned, and states a verdict.
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

CMP = {0: "NEVER", 1: "LESS", 2: "EQUAL", 3: "LEQUAL", 4: "GREATER",
       5: "NEQUAL", 6: "GEQUAL", 7: "ALWAYS"}
AOP = {0: "AND", 1: "OR", 2: "XOR", 3: "XNOR"}
CULL = {0: "NONE", 1: "FRONT", 2: "BACK", 3: "ALL"}


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


def string_table(bdl, base, expect=None):
    num = be16(bdl, base)
    if expect is not None and num != expect:
        return None
    out, p = [], base + 4
    for _ in range(num):
        s = base + be16(bdl, p + 2)
        e = bdl.index(b"\0", s)
        out.append(bdl[s:e].decode("ascii", "replace"))
        p += 4
    return out


def walk_dl(d, start, size):
    """Decode a GX display list, returning {BP register: 24-bit value}.

    Opcodes: 0x00 NOP, 0x61 load BP reg (reg in the top byte of the u32),
    0x08 load CP reg, 0x10 load XF regs (count-1, address, then u32 data).
    Anything unrecognised stops the walk — a wrong stride would silently invent
    register values, so the walk refuses to guess."""
    bp = {}
    p, end = start, start + size
    while p < end:
        op = d[p]
        if op == 0x00:
            p += 1
        elif op == 0x61:
            if p + 5 > end:
                break
            v = be32(d, p + 1)
            bp[(v >> 24) & 0xFF] = v & 0xFFFFFF
            p += 5
        elif op == 0x08:
            p += 6
        elif op == 0x10:
            if p + 5 > end:
                break
            cnt = be16(d, p + 1) + 1
            p += 5 + 4 * cnt
        else:
            break
    return bp


def decode(bp):
    out = {}
    if 0xF3 in bp:
        v = bp[0xF3]
        out["alpha"] = ((v >> 16) & 7, v & 0xFF, (v >> 22) & 3,
                        (v >> 19) & 7, (v >> 8) & 0xFF)
    if 0x00 in bp:
        out["cull"] = (bp[0x00] >> 14) & 3
    if 0x40 in bp:
        v = bp[0x40]
        out["zmode"] = (v & 1, (v >> 1) & 7, (v >> 4) & 1)
    if 0x41 in bp:
        v = bp[0x41]
        out["blend"] = (v & 1, (v >> 11) & 7, (v >> 8) & 7)
    return out


def parse_live(log_path):
    """Live per-material state from the existing §373b/§377 probe lines, for the
    LAST 19-material model reported (the interior room shell)."""
    txt = Path(log_path).read_text(encoding="utf-8", errors="replace")
    live, seen = {}, False
    for line in txt.splitlines():
        if re.search(r"373 model\[\d+\] shapes=19 materials=19", line):
            live, seen = {}, True
            continue
        if not seen:
            continue
        m = re.search(r"373b mat\[(\d+)\] tex0=(-?\d+) alphaComp0=(-?\d+) "
                      r"ref0=(-?\d+) blendMode=(-?\d+)", line)
        if m:
            i = int(m.group(1))
            live.setdefault(i, {}).update(
                tex0=int(m.group(2)), comp0=int(m.group(3)),
                ref0=int(m.group(4)), blend=int(m.group(5)))
        m = re.search(r"377 mat\[(\d+)\] cullMode=(-?\d+)", line)
        if m:
            live.setdefault(int(m.group(1)), {}).update(cull=int(m.group(2)))
    return live


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: mdl3_state_expect.py <arc> <model.bdl> [--log <log>]")
    bdl = get_member(sys.argv[1], sys.argv[2])
    if bdl is None:
        sys.exit("member not found")
    if bdl[:8] != b"J3D2bdl4":
        print(f"NOTE: magic is {bdl[:8]!r}, not J3D2bdl4 — no MDL3 is expected "
              f"in a BMD, and MAT3 is then the authoritative state.")
    moff, msize = find_block(bdl, b"MDL3")
    if moff is None:
        sys.exit("no MDL3 block — nothing for this tool to decode")
    mat_off, _ = find_block(bdl, b"MAT3")
    count = be16(bdl, moff + 8)
    names = None
    if mat_off is not None:
        names = string_table(bdl, mat_off + be32(bdl, mat_off + 0x14),
                             be16(bdl, mat_off + 8))
    names = names or [f"mat{i}" for i in range(count)]
    pktBase = moff + be32(bdl, moff + 0x0C)

    print(f"{sys.argv[2]}: MDL3 with {count} baked material packets "
          f"(block {msize:#x} bytes)")
    print(f"\n{'idx':>3} {'material':30s} {'MDL3 alphaFunc':30s} {'cull':6s} "
          f"{'blend':22s} zmode")
    rows = {}
    for i in range(count):
        e = pktBase + i * 8
        off, size = be32(bdl, e), be32(bdl, e + 4)
        st = decode(walk_dl(bdl, moff + off, size))
        rows[i] = st
        a = st.get("alpha")
        astr = ("NOT SET" if a is None else
                f"{CMP[a[0]]}/{a[1]} {AOP[a[2]]} {CMP[a[3]]}/{a[4]}")
        c = st.get("cull")
        bl = st.get("blend")
        blstr = ("NOT SET" if bl is None else
                 f"{'BLEND' if bl[0] else 'NONE'} src={bl[2]} dst={bl[1]}")
        zm = st.get("zmode")
        print(f"{i:3d} {names[i][:30]:30s} {astr:30s} "
              f"{(CULL.get(c, c) if c is not None else 'NOT SET'):6s} "
              f"{blstr:22s} {zm if zm else 'NOT SET'}")

    if "--log" not in sys.argv:
        return 0
    live = parse_live(sys.argv[sys.argv.index("--log") + 1])
    if not live:
        print("\nno 19-material live probe block in that log — UNKNOWN")
        return 2
    print(f"\n===== §391d DONOR (MDL3) vs LIVE, index-aligned =====")
    print(f"{'idx':>3} {'material':30s} {'donor alpha':22s} {'live alpha':22s} "
          f"{'cull d/l':10s} verdict")
    bad = 0
    for i in range(count):
        st = rows.get(i, {})
        a = st.get("alpha")
        lv = live.get(i)
        if lv is None:
            print(f"{i:3d} {names[i][:30]:30s} {'':22s} {'':22s} {'':10s} "
                  f"NOT REPORTED (UNKNOWN)")
            continue
        dstr = "NOT SET" if a is None else f"{CMP[a[0]]}/{a[1]}"
        lstr = f"{CMP.get(lv['comp0'], lv['comp0'])}/{lv['ref0']}"
        cd = CULL.get(st.get("cull"), "?")
        cl = CULL.get(lv.get("cull"), "?")
        ok_a = a is not None and a[0] == lv["comp0"] and a[1] == lv["ref0"]
        ok_c = st.get("cull") is not None and st.get("cull") == lv.get("cull")
        if a is None or st.get("cull") is None:
            verdict = "UNKNOWN (register not in packet)"
        elif ok_a and ok_c:
            verdict = "MATCH"
        else:
            verdict = ("MISMATCH:" + ("" if ok_a else " alpha") +
                       ("" if ok_c else " cull"))
            bad += 1
        print(f"{i:3d} {names[i][:30]:30s} {dstr:22s} {lstr:22s} "
              f"{cd}/{cl:6s} {verdict}")
    print(f"\n{count - bad}/{count} materials match the donor's baked state; "
          f"{bad} differ")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
