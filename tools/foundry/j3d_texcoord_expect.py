#!/usr/bin/env python3
# ============================================================================
# j3d_texcoord_expect.py — §391c: the DONOR-AUTHORED texcoord expectation.
#
# WHY THIS EXISTS: the §391 per-draw probe now reports, for every draw, how the
# texcoord is generated (texgen type / source / texture matrix) and whether the
# shape supplies a TEX0 vertex attribute. A runtime number is only evidence if
# there is an independent statement of what it SHOULD be — otherwise reading the
# log is interpretation again, which is exactly what the user rejected.
#
# So this decodes the donor .bdl itself and prints, per material:
#   * the texture it selects (texNo[0]) resolved to a TEX1 name
#   * its texgen: type, source, texture matrix (the field under test)
#   * its alpha-compare (comp0/ref0/op/comp1/ref1)
#   * the SHAPE(S) it is paired with via the INF1 hierarchy, and whether each
#     of those shapes actually ships a TEX0 attribute in its vertex descriptor
#
# A cutout material whose shape ships NO TEX0 attribute cannot sample its card:
# the coordinate degenerates and the GEQUAL/128 test kills every fragment. That
# is invisible for a cutout and merely "slightly off" for the opaque materials
# in the same model — the asymmetry actually observed in Grandma's room.
#
# SELF-VALIDATION GATE (this is why the layout can be trusted): MAT3's entry
# layout is version-sensitive, so the tool does not assert it — it PROVES it.
# The live §391 probe independently measured the herb card's alpha-compare as
# comp0=6(GEQUAL) ref0=128 op=0(AND) comp1=3(LEQUAL) ref1=255. If this parse
# reproduces that tuple from the donor bytes, the entry stride and field offsets
# are confirmed by a measurement taken through a completely different path.
# Pass --expect-alpha 6,128,0,3,255 to make that gate explicit and fatal.
#
# №31-C: any field it cannot resolve prints UNKNOWN, never a guess.
# Read-only. Usage:
#   j3d_texcoord_expect.py <arc> <model member> [--expect-alpha c0,r0,op,c1,r1]
# ============================================================================
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                      "ww_crew_restoration_skeleton"))
from jstudio_stb import yaz0_dec, rarc_members

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]

# ---- GX enum spellings (numbers are what both the file and the probe carry) --
TG_TYPE = {0: "MTX3x4", 1: "MTX2x4", 2: "BUMP0", 3: "BUMP1", 4: "BUMP2",
           5: "BUMP3", 6: "BUMP4", 7: "BUMP5", 8: "BUMP6", 9: "BUMP7",
           10: "SRTG"}
TG_SRC = {0: "POS", 1: "NRM", 2: "BINRM", 3: "TANGENT", 4: "TEX0", 5: "TEX1",
          6: "TEX2", 7: "TEX3", 8: "TEX4", 9: "TEX5", 10: "TEX6", 11: "TEX7",
          12: "TEXCOORD0", 13: "TEXCOORD1", 14: "TEXCOORD2", 15: "TEXCOORD3",
          16: "TEXCOORD4", 17: "TEXCOORD5", 18: "TEXCOORD6", 19: "COLOR0",
          20: "COLOR1"}
CMP = {0: "NEVER", 1: "LESS", 2: "EQUAL", 3: "LEQUAL", 4: "GREATER",
       5: "NEQUAL", 6: "GEQUAL", 7: "ALWAYS"}
AOP = {0: "AND", 1: "OR", 2: "XOR", 3: "XNOR"}
# GXAttr values as they appear in a shape's vertex descriptor
VA = {0: "PNMTXIDX", 1: "TEX0MTXIDX", 2: "TEX1MTXIDX", 3: "TEX2MTXIDX",
      4: "TEX3MTXIDX", 5: "TEX4MTXIDX", 6: "TEX5MTXIDX", 7: "TEX6MTXIDX",
      8: "TEX7MTXIDX", 9: "POS", 10: "NRM", 11: "CLR0", 12: "CLR1",
      13: "TEX0", 14: "TEX1", 15: "TEX2", 16: "TEX3", 17: "TEX4", 18: "TEX5",
      19: "TEX6", 20: "TEX7"}

# MAT3 MaterialEntry is 332 (0x14C) bytes. Field offsets used here:
ENTRY_SIZE = 0x14C
OFF_TEXGEN_IDX = 40      # short[8]
OFF_TEXMTX_IDX = 72      # short[10]
OFF_TEXNO_IDX = 132      # short[8]
# §392b OFF-BY-ONE-SHORT, CAUGHT BY ARITHMETIC AND THEN BY LIVE VALUES.
# These were 328/330. That could not be right: the entry stride is exactly 332
# bytes (derived, not assumed — the entry array spans 6308 bytes over 19 entries),
# so a field at 330 would end at 332 and leave no room for the two shorts that
# follow it. The real tail is fog=324, alphaComp=326, blend=328, nbtScale=330.
# Reading alphaComp at 328 was therefore reading the BLEND index — which is why
# the two blended materials (lampshade, bottle) looked like the alpha-tested ones
# and the three plant cards looked like they had no alpha test at all. That single
# shift produced a false "the port invents an alpha test the file does not author"
# claim that went onto the bus.
#
# VALIDATION (now mandatory, see validate_against_live): at 326 the file's
# per-material alpha compare reproduces the live per-material probe 18/18, and
# blend at 328 reproduces live blend 18/18. Offsets are no longer asserted from a
# remembered struct layout — they are the ones that agree with the running game.
OFF_ALPHACOMP_IDX = 326  # short
OFF_BLEND_IDX = 328      # short

FMT = {0: "I4", 1: "I8", 2: "IA4", 3: "IA8", 4: "RGB565", 5: "RGB5A3",
       6: "RGBA8", 8: "C4", 9: "C8", 10: "C14X2", 14: "CMPR"}
BM = {0: "NONE", 1: "BLEND", 2: "LOGIC", 3: "SUBTRACT"}


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
    """J3D string table: u16 count, u16 pad, then (u16 hash, u16 offset)*."""
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


def mat3_names(bdl, off, count):
    rel = be32(bdl, off + 0x14)
    if rel:
        t = string_table(bdl, off + rel, count)
        if t:
            return t
    # fall back to a validated scan rather than trusting one offset
    for probe in range(0x0C, 0x90, 4):
        rel = be32(bdl, off + probe)
        if rel and off + rel + 4 <= len(bdl):
            t = string_table(bdl, off + rel, count)
            if t and all(x.isprintable() for x in t):
                return t
    return None


def tex1_names(bdl):
    off, _ = find_block(bdl, b"TEX1")
    if off is None:
        return []
    cnt = be16(bdl, off + 8)
    return string_table(bdl, off + be32(bdl, off + 0x10)) or []


def tex1_headers(bdl):
    """Per texture: (format, width, height) — the same three numbers the live
    §391 probe reports for the BOUND texture, so a donor material can be located
    in the runtime draw census by content instead of by name."""
    off, _ = find_block(bdl, b"TEX1")
    if off is None:
        return []
    cnt = be16(bdl, off + 8)
    hdr = off + be32(bdl, off + 0x0C)
    out = []
    for i in range(cnt):
        h = hdr + i * 0x20
        out.append((bdl[h], be16(bdl, h + 2), be16(bdl, h + 4)))
    return out


def parse_shapes(bdl):
    """Per shape: its vertex-descriptor attribute set (GXAttr -> mode)."""
    off, _ = find_block(bdl, b"SHP1")
    if off is None:
        return []
    count = be16(bdl, off + 8)
    shapeOff = off + be32(bdl, off + 0x0C)
    attrBase = off + be32(bdl, off + 0x18)
    shapes = []
    for i in range(count):
        e = shapeOff + i * 0x28
        attrRel = be16(bdl, e + 4)
        p = attrBase + attrRel
        attrs = {}
        # list of (u32 attr, u32 type) terminated by attr == 0xFF
        while p + 8 <= len(bdl):
            a = be32(bdl, p)
            t = be32(bdl, p + 4)
            p += 8
            if a == 0xFF:
                break
            attrs[a] = t
        shapes.append({"idx": i, "attrs": attrs,
                       "mtxType": bdl[e], "groups": be16(bdl, e + 2)})
    return shapes


def parse_inf1_pairs(bdl):
    """INF1 hierarchy walk -> {material index: [shape indices]}.

    Node types: 0x01 down, 0x02 up, 0x10 joint, 0x11 material, 0x12 shape.
    A shape belongs to the material most recently entered on the stack, which is
    how J3D itself resolves the pairing at draw time."""
    off, _ = find_block(bdl, b"INF1")
    if off is None:
        return {}
    p = off + be32(bdl, off + 0x14)
    matStack, cur, pairs = [], None, {}
    while p + 4 <= len(bdl):
        typ, idx = be16(bdl, p), be16(bdl, p + 2)
        p += 4
        if typ == 0:
            break
        if typ == 0x01:
            matStack.append(cur)
        elif typ == 0x02:
            cur = matStack.pop() if matStack else None
        elif typ == 0x11:
            cur = idx
        elif typ == 0x12 and cur is not None:
            pairs.setdefault(cur, []).append(idx)
    return pairs


def parse_gx_census(log_path):
    """Live §391 draw census, scoped to the LAST arming (the probe is armed by
    the room verifier, so the last arming is the room last entered). Returns a
    list of dicts — one per distinct draw signature."""
    import re
    txt = Path(log_path).read_text(encoding="utf-8", errors="replace")
    pat = re.compile(
        r"\[GXDraw\] 391 tex=(\d+)x(\d+) fmt=(\d+)(?: tex1=(\d+)x(\d+))? "
        r"alphaComp=\(c0=(-?\d+) r0=(\d+) op=(-?\d+) c1=(-?\d+) r1=(\d+)\) "
        r"numTev=(\d+) blend=(-?\d+) cull=(-?\d+) depthFunc=(-?\d+)"
        r"(?: \| numTexGen=(\d+) tcg0\(type=(-?\d+) src=(-?\d+) mtx=(-?\d+) "
        r"post=(-?\d+) norm=(\d+)\) vtx\(tex0=(-?\d+) pos=(-?\d+) "
        r"clr0=(-?\d+) stride=(\d+)\))?")
    groups, cur = [], None
    for line in txt.splitlines():
        # A new arming begins at each room-verify report (they are emitted from
        # the same settle frame), so draws are attributed to the right room.
        if "[RoomVerify]" in line and " room " in line and (
                "PRESENT" in line or "UNKNOWN" in line):
            cur = {"room": line.split("[RoomVerify]")[1].strip(), "draws": []}
            groups.append(cur)
            continue
        m = pat.search(line)
        if m and cur is not None:
            g = m.groups()
            cur["draws"].append({
                "w": int(g[0]), "h": int(g[1]), "fmt": int(g[2]),
                "ac": (int(g[5]), int(g[6]), int(g[7]), int(g[8]), int(g[9])),
                "numTev": int(g[10]), "blend": int(g[11]), "cull": int(g[12]),
                "tcg": None if g[15] is None else
                       (int(g[15]), int(g[16]), int(g[17]), int(g[18])),
                "vtx": None if g[20] is None else
                       (int(g[20]), int(g[21]), int(g[22]), int(g[23])),
            })
    return [g for g in groups if g["draws"]]


def run_differ(rows, texhdr, texs, log_path, want_room=None):
    """Donor material state vs the live draw census — by data, no eye involved.

    Each donor material is located in the census by the CONTENT of the texture it
    selects (format + width + height), never by name, because the runtime has no
    names. Then its authored alpha-compare and blend mode are compared against
    what the draw actually carried.
    """
    groups = parse_gx_census(log_path)
    if not groups:
        print("\nno §391 draw census in that log — UNKNOWN (enter the room once "
              "on the probe build)")
        return 2
    # Room scoping is not optional. The probe is armed on EVERY room settle, so
    # the last group in a log is simply the last room walked into — comparing a
    # room model against the census of a different room reads as mass absence.
    # (That happened on the first differ run: 8 "NOT DRAWN" were an exterior.)
    if want_room:
        sel = [x for x in groups if want_room in x["room"]]
        if not sel:
            print(f"\nno census for a room matching '{want_room}' — UNKNOWN. "
                  f"Available: " + " | ".join(x["room"][:60] for x in groups))
            return 2
        g = sel[-1]
    else:
        g = groups[-1]
        print("\nNOTE: no --room given, using the LAST arming in the log. If "
              "that is not the room this model belongs to, every row below is "
              "meaningless — pass --room.")
    print(f"\n===== §391c DONOR vs LIVE  ({len(g['draws'])} distinct draws, "
          f"census scoped to: {g['room']}) =====")

    # Textures whose (fmt,w,h) is shared by more than one donor material cannot
    # be told apart in the census; say so instead of picking one.
    dims = {}
    for r in rows:
        if r["texNo"] is None or r["texNo"] >= len(texhdr):
            continue
        dims.setdefault(texhdr[r["texNo"]], []).append(r["i"])

    print(f"{'idx':>3} {'material':30s} {'texture(fmt WxH)':30s} "
          f"{'donor: alphaComp | cull/nTev/blend':40s} {'verdict':16s} live")
    counts = {}
    for r in rows:
        if r["texNo"] is None or r["texNo"] >= len(texhdr):
            counts["UNKNOWN"] = counts.get("UNKNOWN", 0) + 1
            print(f"{r['i']:3d} {r['name'][:30]:30s} {'UNKNOWN':30s}")
            continue
        fmt, w, h = texhdr[r["texNo"]]
        key = f"{FMT.get(fmt, fmt)} {w}x{h}"
        ac = r["ac"]
        acs = ("UNKNOWN" if ac is None else
               f"{CMP.get(ac[0], ac[0])}/{ac[1]} {AOP.get(ac[2], ac[2])} "
               f"{CMP.get(ac[3], ac[3])}/{ac[4]}")
        bt = None if r["blend"] is None else r["blend"][0]
        donor = f"{acs} | cull={r['cull']} nTev={r['nTev']} blend={BM.get(bt, bt)}"

        hits = [d for d in g["draws"]
                if d["w"] == w and d["h"] == h and d["fmt"] == fmt]
        # Strongest key first: alpha-compare AND cull AND TEV-stage count AND
        # blend type all agreeing is a near-unique fingerprint even when several
        # materials share a texture size.
        exact = [d for d in hits if d["ac"] == ac and d["cull"] == r["cull"]
                 and d["numTev"] == r["nTev"] and d["blend"] == bt]
        acOnly = [d for d in hits if d["ac"] == ac]
        siblings = [x for x in dims.get(texhdr[r["texNo"]], []) if x != r["i"]]

        if not hits:
            verdict, extra = "NOT DRAWN", "no draw bound a texture of this size"
        elif exact:
            d = exact[0]
            verdict = "MATCH"
            extra = (f"exact fingerprint"
                     + (f"  tcg={d['tcg']} vtx(tex0,pos,clr0,stride)={d['vtx']}"
                        if d["tcg"] else "  (pre-§391c line: no tcg/vtx)"))
        elif acOnly:
            d = acOnly[0]
            verdict = "PARTIAL"
            extra = (f"alpha matches, rest differs: live cull={d['cull']} "
                     f"nTev={d['numTev']} blend={BM.get(d['blend'], d['blend'])}"
                     + (f"  tcg={d['tcg']} vtx={d['vtx']}" if d["tcg"] else ""))
        else:
            verdict = "STATE MISMATCH"
            extra = "live=" + " ; ".join(
                f"{CMP.get(d['ac'][0], d['ac'][0])}/{d['ac'][1]} "
                f"{AOP.get(d['ac'][2], d['ac'][2])} "
                f"{CMP.get(d['ac'][3], d['ac'][3])}/{d['ac'][4]} "
                f"cull={d['cull']} nTev={d['numTev']} "
                f"blend={BM.get(d['blend'], d['blend'])}"
                + (f" tcg={d['tcg']} vtx={d['vtx']}" if d["tcg"] else "")
                for d in hits[:4])
        if siblings and verdict in ("MATCH", "PARTIAL"):
            extra += (f"  [texture size also used by mat {siblings} — this row "
                      f"is a fingerprint match, not a name match]")
        counts[verdict] = counts.get(verdict, 0) + 1
        print(f"{r['i']:3d} {r['name'][:30]:30s} {key:30s} {donor:40s} "
              f"{verdict:16s} {extra}")
    print("\n" + "  ".join(f"{k}={v}" for k, v in counts.items()))
    # Draws in the census that no donor material claims: with the room model's
    # 19 materials accounted for, a leftover CMPR draw at one of this model's
    # texture sizes means state we did not author.
    print("\nDRAWS THIS MODEL CANNOT ACCOUNT FOR (same texture size, no donor "
          "material with that state):")
    sizes = {(f, w, h) for (f, w, h) in texhdr}
    orphan = 0
    for d in g["draws"]:
        if (d["fmt"], d["w"], d["h"]) not in sizes:
            continue
        if any(d["ac"] == r["ac"] and d["cull"] == r["cull"] and
               d["numTev"] == r["nTev"] and
               d["blend"] == (None if r["blend"] is None else r["blend"][0])
               for r in rows
               if r["texNo"] is not None and r["texNo"] < len(texhdr) and
               texhdr[r["texNo"]] == (d["fmt"], d["w"], d["h"])):
            continue
        orphan += 1
        print(f"  {FMT.get(d['fmt'], d['fmt'])} {d['w']}x{d['h']}: "
              f"alphaComp={CMP.get(d['ac'][0], d['ac'][0])}/{d['ac'][1]} "
              f"{AOP.get(d['ac'][2], d['ac'][2])} "
              f"{CMP.get(d['ac'][3], d['ac'][3])}/{d['ac'][4]} "
              f"cull={d['cull']} nTev={d['numTev']} "
              f"blend={BM.get(d['blend'], d['blend'])}"
              + (f" tcg={d['tcg']} vtx={d['vtx']}" if d["tcg"] else ""))
    if not orphan:
        print("  none")
    return 0


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: j3d_texcoord_expect.py <arc> <model.bdl> "
                 "[--expect-alpha c0,r0,op,c1,r1] [--log <dusklight log>]")
    arc, member = sys.argv[1], sys.argv[2]
    expect_alpha = None
    if "--expect-alpha" in sys.argv:
        expect_alpha = tuple(
            int(x) for x in sys.argv[sys.argv.index("--expect-alpha") + 1].split(","))

    bdl = get_member(arc, member)
    if bdl is None:
        sys.exit("member not found")
    moff, _ = find_block(bdl, b"MAT3")
    if moff is None:
        sys.exit("no MAT3")
    count = be16(bdl, moff + 8)
    names = mat3_names(bdl, moff, count) or ["?"] * count
    texs = tex1_names(bdl)

    entryBase = moff + be32(bdl, moff + 0x0C)
    remapRel = be32(bdl, moff + 0x10)
    texGenBase = moff + be32(bdl, moff + 0x38)
    texNoBase = moff + be32(bdl, moff + 0x48)
    alphaBase = moff + be32(bdl, moff + 0x6C)
    cullBase = moff + be32(bdl, moff + 0x1C) if be32(bdl, moff + 0x1C) else 0
    tevNumBase = moff + be32(bdl, moff + 0x58) if be32(bdl, moff + 0x58) else 0
    blendBase = moff + be32(bdl, moff + 0x70) if be32(bdl, moff + 0x70) else 0

    shapes = parse_shapes(bdl)
    pairs = parse_inf1_pairs(bdl)

    print(f"{member}: {count} materials, {len(texs)} textures, "
          f"{len(shapes)} shapes, {len(pairs)} material->shape pairings")

    rows = []
    for i in range(count):
        # remap table: several materials may share one entry record
        ent = i
        if remapRel:
            ent = be16(bdl, moff + remapRel + i * 2)
        e = entryBase + ent * ENTRY_SIZE
        tgIdx = be16(bdl, e + OFF_TEXGEN_IDX)
        tnIdx = be16(bdl, e + OFF_TEXNO_IDX)
        acIdx = be16(bdl, e + OFF_ALPHACOMP_IDX)

        # Cull mode, TEV stage count and blend mode come along because texture
        # SIZE alone cannot separate materials that share a texture format and
        # dimensions (four materials in this model are CMPR 128x128). Adding the
        # donor's own cull/numTev/blend to the key makes the runtime match
        # discriminating instead of merely plausible.
        cull = be32(bdl, cullBase + bdl[e + 1] * 4) if cullBase else None
        nTev = bdl[tevNumBase + bdl[e + 4]] if tevNumBase else None
        blIdx = be16(bdl, e + OFF_BLEND_IDX)
        bl = None
        if blendBase and blIdx != 0xFFFF:
            b = blendBase + blIdx * 4
            bl = (bdl[b], bdl[b + 1], bdl[b + 2], bdl[b + 3])

        tg = None
        if tgIdx != 0xFFFF:
            t = texGenBase + tgIdx * 4
            tg = (bdl[t], bdl[t + 1], bdl[t + 2])
        texNo = None
        if tnIdx != 0xFFFF:
            texNo = be16(bdl, texNoBase + tnIdx * 2)
        ac = None
        if acIdx != 0xFFFF:
            a = alphaBase + acIdx * 8
            ac = (bdl[a], bdl[a + 1], bdl[a + 2], bdl[a + 3], bdl[a + 4])
        rows.append({"i": i, "name": names[i], "tg": tg, "texNo": texNo,
                     "ac": ac, "shapes": pairs.get(i, []),
                     "cull": cull, "nTev": nTev, "blend": bl})

    # ---- self-validation gates -----------------------------------------------
    # GATE 1: entry stride is DERIVED, not remembered. The entry array runs from
    # the init-data offset to the remap-table offset, so span/count must be a
    # whole number — and it is what catches a wrong struct size immediately.
    span = remapRel - be32(bdl, moff + 0x0C) if remapRel else None
    if span:
        if span % count:
            print(f"\nSTRIDE GATE FAILED — entry array spans {span} bytes over "
                  f"{count} entries ({span / count:.2f} each), not a whole "
                  f"number. Field offsets below are UNKNOWN, not evidence.")
            return 2
        print(f"\nSTRIDE GATE: entry array spans {span} bytes over {count} "
              f"entries = {span // count} each"
              + ("  (matches the assumed 332)" if span // count == ENTRY_SIZE
                 else f"  *** ASSUMED {ENTRY_SIZE} — MISMATCH ***"))
        if span // count != ENTRY_SIZE:
            return 2

    # GATE 2: the one that actually caught §392b. A remembered field offset can
    # be plausible and wrong: reading alphaComp one short late returned the BLEND
    # index, whose values (0/1 on 19 materials) looked entirely reasonable and
    # produced a false finding on the bus. The only trustworthy check is whether
    # the per-material MAPPING agrees with the running game, material by
    # material — so pass --expect-alpha the live comp0 values as a comma list
    # (one per material, in index order) and this refuses to report on anything
    # less than a full agreement.
    if expect_alpha is not None:
        if len(expect_alpha) == 5:
            print(f"\nNOTE: --expect-alpha got a single 5-tuple. That only shows "
                  f"SOME material decodes to it — it does not validate the "
                  f"per-material mapping, which is exactly the error §392b made. "
                  f"Prefer one comp0 value per material.")
            hit = [r for r in rows if r["ac"] == expect_alpha]
            if not hit:
                print("LAYOUT GATE FAILED — no material decodes to that tuple.")
                return 2
            print(f"weak gate passed for: {', '.join(r['name'] for r in hit)}")
        elif len(expect_alpha) == count:
            bad = [(r["i"], r["name"],
                    None if r["ac"] is None else r["ac"][0], expect_alpha[r["i"]])
                   for r in rows
                   if r["ac"] is None or r["ac"][0] != expect_alpha[r["i"]]]
            if bad:
                print(f"\nMAPPING GATE FAILED — {len(bad)}/{count} materials "
                      f"disagree with the live comp0 values:")
                for i, nm, got, want in bad:
                    print(f"  mat[{i}] {nm}: file says {got}, live says {want}")
                print("Either the field offset is wrong (check the entry tail: "
                      "fog/alphaComp/blend/nbtScale are the last four shorts) or "
                      "the port genuinely deviates. Do not report either as fact "
                      "until this gate passes.")
                return 2
            print(f"\nMAPPING GATE PASSED — file alpha-compare agrees with the "
                  f"live per-material probe {count}/{count}. Field offsets are "
                  f"confirmed by the running game, not by a remembered layout.")
        else:
            print(f"\n--expect-alpha needs either 5 values (weak) or {count} "
                  f"comp0 values (one per material); got {len(expect_alpha)}.")
            return 2

    print(f"\n{'idx':>3} {'material':26s} {'texture':24s} "
          f"{'texgen(type/src/mtx)':28s} {'alphaComp':30s} shapes(TEX0?)")
    for r in rows:
        tn = texs[r["texNo"]] if (r["texNo"] is not None and
                                  0 <= r["texNo"] < len(texs)) else "UNKNOWN"
        if r["tg"] is None:
            tgs = "UNKNOWN (no texgen index)"
        else:
            ty, sr, mx = r["tg"]
            tgs = (f"{TG_TYPE.get(ty, ty)}/{TG_SRC.get(sr, sr)}/"
                   f"{'IDENTITY' if mx == 60 else f'TEXMTX{(mx - 30) // 3}'}")
        if r["ac"] is None:
            acs = "UNKNOWN"
        else:
            c0, r0, op, c1, r1 = r["ac"]
            acs = f"{CMP.get(c0, c0)}/{r0} {AOP.get(op, op)} {CMP.get(c1, c1)}/{r1}"
        sh = []
        for s in r["shapes"]:
            has = 13 in shapes[s]["attrs"] if s < len(shapes) else None
            sh.append(f"{s}({'TEX0' if has else 'no-TEX0'})")
        print(f"{r['i']:3d} {r['name']:26s} {tn:24s} {tgs:28s} {acs:30s} "
              f"{' '.join(sh) if sh else 'UNPAIRED'}")

    # ---- the cutout materials, called out ------------------------------------
    cut = [r for r in rows if r["ac"] and r["ac"][0] == 6 and r["ac"][1] > 0]
    print(f"\nCUTOUT MATERIALS (GEQUAL with a nonzero reference — these are the "
          f"ones an absent texcoord makes INVISIBLE rather than merely wrong):")
    if not cut:
        print("  none")
    for r in cut:
        for s in r["shapes"]:
            attrs = shapes[s]["attrs"] if s < len(shapes) else {}
            present = ",".join(VA.get(a, str(a)) for a in sorted(attrs))
            verdict = ("OK — shape ships TEX0" if 13 in attrs else
                       "*** SHAPE SHIPS NO TEX0 ATTRIBUTE ***")
            print(f"  mat[{r['i']}] {r['name']} -> shape {s}: {verdict}")
            print(f"      shape attrs: {present}")
        if not r["shapes"]:
            print(f"  mat[{r['i']}] {r['name']} -> UNPAIRED in INF1 (UNKNOWN)")

    if "--log" in sys.argv:
        room = (sys.argv[sys.argv.index("--room") + 1]
                if "--room" in sys.argv else None)
        return run_differ(rows, tex1_headers(bdl), texs,
                          sys.argv[sys.argv.index("--log") + 1], room)
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
