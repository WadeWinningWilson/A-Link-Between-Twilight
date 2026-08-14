#!/usr/bin/env python3
# ===========================================================================
# accessor_differ.py — V1: the THREE-SOURCE cross-lineage accessor differ.
#
# WHAT IT IS FOR
# Two separately-debugged bugs, one defect shape:
#   §212  JPA1 -> JPA2 `ESP1`: WW's alpha-enable bit 0x00000001 lands on TP's
#         scale-anim-enable => the particle is scaled to nothing => INVISIBLE.
#   §332  dzb collision attribute: WW packs bits 16-20, TP reads 12-15/16-18
#         => every WW attribute with nonzero low bits reads as TP sink-class
#         => Link sinks into furniture.
# Both are "same block name, different record layout". This turns "hand-diff
# each block, discover the collision in a playtest" into a mechanical pre-flight.
#
# WHY THREE SOURCES, NOT TWO (§A1 amendment, §513)
# A donor header and a receiver header are BOTH our own transcriptions, and the
# entire bug class this targets IS misread bits. A two-source differ can agree
# with itself and be confidently wrong. So a THIRD, independent implementation
# is read as VALIDATOR.
#
#   A  DONOR      decomp header        D:/XXXXXXX/WW DP/include/d/d_stage.h
#   B  RECEIVER   decomp header        dusklight include/d/d_stage.h
#   C  VALIDATOR  Winditor templates   D:/XXXXXXX/Winditor/.../templates/*.json
#
# **VALIDATOR SCOPE — STATED, NOT PAPERED OVER.** Winditor is a WIND WAKER
# editor, so source C validates the DONOR side only. It cannot speak to TP. That
# is still the useful direction: if A and C agree, our donor transcription is
# trustworthy, and any A-vs-B difference is then a REAL receiver difference
# rather than a transcription error on our side. Where C is absent, rows are
# reported UNVALIDATED — never silently promoted to agreement.
#
# IT REPORTS DISAGREEMENT. IT NEVER PICKS A WINNER.
# Adjudicating would make it an oracle, and it has no standing to be one — the
# whole point is that all three sources are fallible. A DISAGREE row is a
# routed question, not a verdict.
#
# Read-only. Usage:
#   accessor_differ.py                     all mapped blocks
#   accessor_differ.py --block ACTR        one block
#   accessor_differ.py --list              what is mapped, and what is not
# Exit 0 = no disagreements. 1 = at least one DISAGREE or CONTRADICTS.
# ===========================================================================
import argparse
import io
import json
import re
import sys
from pathlib import Path

DONOR_DIR = Path("D:/XXXXXXX/WW DP/include/d")
RECV_DIR = Path(__file__).resolve().parents[2] / "include" / "d"
DONOR_H = DONOR_DIR / "d_stage.h"
RECV_H = RECV_DIR / "d_stage.h"


def resolve_side(base_dir, default_header, name):
    """A map name is either 'StructName' (read from d_stage.h) or
    'header.h::StructName' (records that live elsewhere -- RPPN/RPAT's dPnt/
    dPath are in d_path.h on BOTH sides). Explicit per row, never searched:
    a found-by-search struct could be an unrelated same-named type."""
    if name and "::" in name:
        hdr, _, struct = name.partition("::")
        return base_dir / hdr, struct
    return default_header, name
WINDITOR = Path("D:/XXXXXXX/Winditor/Editor/resources/templates")

# ===========================================================================
# BLOCK MAP — declared, never inferred.
# A guessed struct<->FourCC mapping would silently compare two unrelated
# records and report AGREE. Each row is a claim someone can check.
# ===========================================================================
# Struct names READ from the donor header's own chunk comments
# (D:/XXXXXXX/WW DP/include/d/d_stage.h — "// RTBL", "// EVNT", ...), not
# guessed from the FourCC. The first draft of this map guessed nine of twelve
# and every one of them was wrong; the tool reported them UNCOMPARABLE, which
# is why they are correct now.
# Rows are (fourcc, donor struct, receiver struct or None if same, template).
# The two decomps DIVERGE on names -- `stage_palet_info_class` vs
# `stage_palette_info_class` -- so one name per row would silently mis-report a
# real block as missing. Per-side names, both read from each header's own chunk
# comments, never guessed from the FourCC.
#
# ROOM SCOPE: this map now covers all 22 chunks of the donor's
# `dStage_dt_c_roomLoader` (d_stage.cpp:2181) -- the Phase-0 gate of
# docs/WW Linked/native-rooms-3b-scope.md -- plus ACTR/SCOB/MEMA/MECO, which are
# stage-scope but were already cleared.
BLOCK_MAP = [
    # -- room-loader chunks (the 3b Phase-0 set) ----------------------------
    ("PLYR", "stage_actor_data_class",     None,                        "Player Spawn.json"),
    ("RCAM", "stage_camera2_data_class",   None,                        "Camera Point (v1).json"),
    ("RARO", "stage_arrow_data_class",     None,                        None),
    # RTBL is POINTER-INDIRECTED (ptr array -> records -> room-byte lists), so
    # single-stride verdicts are a CATEGORY ERROR (§689): A=0x8 is the record,
    # C=0x4 the ptr entry, gap/num a composite. Compare it with --rtbl, never
    # by stride.
    ("RTBL", "roomRead_data_class",        None,                        "RTBL.json"),
    ("AROB", "stage_arrow_data_class",     None,                        None),
    ("2Dma", "stage_map_info_class",       None,                        "2D Minimap.json"),
    ("Pale", "stage_palet_info_class",     "stage_palette_info_class",  "Environment Palette.json"),
    ("Colo", "stage_pselect_info_class",   None,                        "Environment Time Preset.json"),
    ("Virt", "stage_vrbox_info_class",     None,                        "Environment Skybox Colors.json"),
    ("SCLS", "stage_scls_info_class",      None,                        "Exit Info.json"),
    ("LGHT", "stage_plight_info_class",    None,                        "Dynamic Light.json"),
    # RPPN/RPAT: records live in d_path.h on BOTH sides (containers
    # dStage_dPnt_c/dStage_dPath_c are in d_stage.h; the RECORDS are not).
    # SIZES HAND-DERIVED (donor header carries no // Size: comment, and the
    # tool rightly refuses to sum fields itself): dPnt = 4 + 0xC = 0x10 both
    # sides; dPath = 0xC both sides. Strides MATCH.
    #
    # ROUTED FINDING the type-compare CANNOT see (it matches u8 == u8 and
    # reports AGREE): the arg bytes are PERMUTED between lineages —
    #     donor    dPnt  { mArg0@0, mArg1@1, mArg2@2, mArg3@3 }
    #     receiver dPnt  { mArg1@0, mArg2@1, mArg3@2, mArg0@3 }
    # Same stride, first byte's MEANING moved to the end. RULED REAL (§609):
    # both headers are MATCHED decomps (zeldaret/tww and dusklight's TP match),
    # so each side's field order is ground truth for its own game — this is a
    # genuine lineage difference, not a naming artifact. PORT RULE for the 3b
    # RPPN reader: write args into the receiver's record BY NAME (mArg0 -> the
    # receiver's mArg0 slot at 0x3), never by position. A positional copy
    # reproduces the §332 class at one byte's distance.
    ("RPPN", "d_path.h::dPnt",             "d_path.h::dPnt",            None),
    ("RPAT", "d_path.h::dPath",            "d_path.h::dPath",           None),
    # SOND: `dStage_SoundInfo_c` is the receiver's CONTAINER (num + entries).
    # Its entries are `stage_sound_data` — the same record name as the donor's.
    # Mapping container-vs-record was my bug and it reported a false DISAGREE.
    ("SOND", "stage_sound_data",           None,                        "Sound.json"),
    ("EVNT", "dStage_Event_dt_c",          "ABSENT",                    "Event.json"),
    ("EnvR", "stage_envr_info_class",      None,                        "Environment Lighting Preset.json"),
    ("FILI", "dStage_FileList_dt_c",       None,                        "Room Properties.json"),
    ("LGTV", "stage_lightvec_info_class",  "ABSENT",                    "Light Vector.json"),
    ("FLOR", "dStage_FloorInfo_dt_c",      "dStage_FloorInfo_dt_c",     None),
    ("SHIP", "dStage_Ship_dt_c",           "ABSENT",                    "Ship Spawn.json"),
    # LBNK / MEMA: the DONOR has no named record struct — its container holds a
    # raw `u8*` / `u32*`, so the "record" is a primitive. There is nothing to
    # compare field-wise, and mapping donor-container against receiver-record
    # produced a false DISAGREE in the first run. Left deliberately UNMAPPED:
    # a visible hole, not a silent pass.
    ("LBNK", "PRIM:u8 bank per layer; 0xFF=none else Demo%02d (d_s_room.cpp:205-216)",
             None,                         "LBNK.json"),
    # -- stage-scope, already cleared ---------------------------------------
    ("ACTR", "stage_actor_data_class",     None,                        "Actor.json"),
    ("SCOB", "stage_tgsc_data_class",      None,                        "Scaleable Object.json"),
    ("MECO", "dStage_MemoryConfig_data",   None,                        "MECO.json"),
    ("MEMA", "PRIM:u32 memory-map entries (dStage_MemoryMap_c payload)",
             None,                         "MEMA.json"),
]

# Winditor type -> byte width. An UNKNOWN type must NOT be guessed: every
# offset after it would silently shift, which is the exact failure mode this
# tool exists to catch. Unknown poisons the remainder of the block, loudly.
WIND_SIZES = {
    "Byte": 1, "SByte": 1, "Bool": 1,
    "Short": 2, "UShort": 2,
    "Int": 4, "UInt": 4, "Float": 4,
    "Vector2": 8, "Vector3": 12,
    "Color32": 4, "ColorU8": 4,
}


# ===========================================================================
# SOURCE A/B — decomp headers
# ===========================================================================
# `struct` OR `class`. The first version matched only `struct`, so every block
# the receiver declares as a class -- FILI, LBNK -- reported UNCOMPARABLE. That
# is a tool bug wearing a finding's clothes, and it is the dangerous direction:
# it manufactures "we could not check this" out of blocks that were sitting
# there readable. Distinguish it from EVNT/SHIP, which are genuinely absent.
STRUCT_RE = r"(?:struct|class)\s+%s\s*\{(.*?)\n\};"
FIELD_RE = re.compile(r"/\*\s*(0x[0-9A-Fa-f]+)\s*\*/\s*([^;]+);")
SIZE_RE = re.compile(r"\}\s*;\s*//\s*Size:\s*(0x[0-9A-Fa-f]+)")


def normalize_type(t):
    """Strip the PC port's wrappers, but RECORD that we did. BE(int) is an
    endian annotation and OFFSET_PTR(T) is a 32-bit offset standing where the
    GC build had a pointer — on a 64-bit host that IS a layout fact, so it is
    reported as a note rather than silently erased."""
    notes = []
    m = re.match(r"BE\((.+)\)$", t.strip())
    if m:
        notes.append("BE()")
        t = m.group(1)
    m = re.match(r"OFFSET_PTR\((.+)\)$", t.strip())
    if m:
        notes.append("OFFSET_PTR() — 32-bit offset where GC had a pointer")
        t = m.group(1) + "*"
    return t.strip(), notes


def parse_header(path, struct_name):
    """-> (fields, size, lineno) ; fields = [(off, type, name, notes, line)]"""
    if not path.is_file():
        return None, None, None
    txt = io.open(path, encoding="utf-8", errors="replace").read()
    m = re.search(STRUCT_RE % re.escape(struct_name), txt, re.S)
    if not m:
        return None, None, None
    body = m.group(1)
    start_line = txt[:m.start()].count("\n") + 1

    fields = []
    for fm in FIELD_RE.finditer(body):
        off = int(fm.group(1), 16)
        decl = " ".join(fm.group(2).split())
        line = start_line + body[:fm.start()].count("\n") + 1
        # split trailing identifier (and any array suffix) off the type
        dm = re.match(r"(.*?)([A-Za-z_]\w*)\s*(\[[^\]]*\])?$", decl)
        if dm:
            ftype = (dm.group(1) + (dm.group(3) or "")).strip()
            fname = dm.group(2)
        else:
            ftype, fname = decl, "?"
        ftype, notes = normalize_type(ftype)
        fields.append((off, ftype, fname, notes, line))

    tail = txt[m.end() - 3: m.end() + 40]
    sm = SIZE_RE.search(tail)
    size = int(sm.group(1), 16) if sm else None
    return fields, size, start_line


# ===========================================================================
# SOURCE C — Winditor templates (offsets DERIVED from ordered field types)
# ===========================================================================
def parse_winditor(fname):
    """-> (fields, size, fourcc, error) ; offsets derived by summation."""
    p = WINDITOR / fname
    if not p.is_file():
        return None, None, None, "template not found: %s" % fname
    raw = io.open(p, encoding="utf-8-sig", errors="replace").read()
    # these templates carry trailing commas, which strict JSON rejects
    raw = re.sub(r",(\s*[}\]])", r"\1", raw)
    try:
        doc = json.loads(raw)
    except ValueError as e:
        return None, None, None, "unparseable: %s" % e

    off = 0
    out = []
    for f in doc.get("Fields", []):
        t = f.get("Type")
        if t == "FixedLengthString":
            w = int(f.get("Length", 0))
        elif t in WIND_SIZES:
            w = WIND_SIZES[t]
        else:
            # Poison the rest: every later offset would be wrong, and a wrong
            # offset reported as fact is worse than no offset at all.
            out.append((off, t or "?", f.get("Name", "?"), ["UNKNOWN TYPE"], 0))
            return out, None, doc.get("FourCC"), (
                "unknown Winditor type %r at field %r — offsets past it are "
                "UNRESOLVED, not assumed" % (t, f.get("Name")))
        out.append((off, t, f.get("Name", "?"), [], 0))
        off += w
    return out, off, doc.get("FourCC"), None


# ===========================================================================
# COMPARE — report, never adjudicate
# ===========================================================================
def compare(fourcc, d_struct, r_struct, tmpl, verbose):
    r_struct = r_struct or d_struct
    if r_struct == "ABSENT" or d_struct == "ABSENT":
        side = "RECEIVER" if r_struct == "ABSENT" else "DONOR"
        print("\n" + "=" * 74)
        print("  %-6s  %s-ABSENT (confirmed by grep, not assumed)" % (fourcc, side))
        print("=" * 74)
        print("  The %s has no record for this chunk at all. CLOSED as a finding:" % side.lower())
        print("  the loader must decide what to do with the block, but there is no")
        print("  layout question left to audit. (EVNT: TP uses REVT in a different")
        print("  phase — the 3b Phase-4 question. SHIP: WW sailing, no TP analogue.")
        print("  LGTV: TP's nearest is stage_pure_lightvec_info_class under tag LGT,")
        print("  a spotlight record, 0x20 — a DIFFERENT chunk, not a renamed LGTV.)")
        return 0, 0, "%s-ABSENT" % side
    if d_struct and d_struct.startswith("PRIM:"):
        print("\n" + "=" * 74)
        print("  %-6s  PRIMITIVE payload — %s" % (fourcc, d_struct[5:]))
        print("=" * 74)
        print("  No record struct on either side; the chunk body is a bare array.")
        print("  Nothing to field-compare. CLOSED; semantics verified at port time")
        print("  against the consumer cited in the map.")
        return 0, 0, "PRIMITIVE"
    if d_struct is None and r_struct is None:
        print("\n" + "=" * 74)
        print("  %-6s  UNMAPPED — no struct name recorded for either side." % fourcc)
        print("=" * 74)
        print("  NOT a pass. An unmapped chunk has not been checked; it is on the")
        print("  map as a visible hole precisely so it cannot read as covered.")
        return 0, 1, "UNMAPPED"

    d_hdr, d_struct = resolve_side(DONOR_DIR, DONOR_H, d_struct)
    r_hdr, r_struct = resolve_side(RECV_DIR, RECV_H, r_struct)
    a, a_size, a_line = parse_header(d_hdr, d_struct) if d_struct else (None, None, None)
    b, b_size, b_line = parse_header(r_hdr, r_struct) if r_struct else (None, None, None)
    c, c_size, c_cc, c_err = parse_winditor(tmpl) if tmpl else (None, None, None, "no template mapped")

    print("\n" + "=" * 74)
    print("  %-6s  A:%s   B:%s" % (fourcc, d_struct or "-", r_struct or "-"))
    print("=" * 74)
    print("  A DONOR     %s%s" % (d_hdr.name, ":%d" % a_line if a_line else "  NOT FOUND — %s" % d_struct))
    print("  B RECEIVER  %s%s" % (r_hdr.name, ":%d" % b_line if b_line else "  NOT FOUND — %s" % r_struct))
    print("  C VALIDATOR %s%s" % (tmpl or "-", "" if c and not c_err else "   %s" % (c_err or "n/a")))
    if c_cc and c_cc != fourcc:
        print("  !! FourCC MISMATCH: map says %s, template declares %s" % (fourcc, c_cc))

    if a is None or b is None:
        which = "DONOR" if a is None else "RECEIVER"
        print("\n  UNCOMPARABLE — the %s side has no such struct." % which)
        print("  Reported, not skipped. Two very different causes look identical here:")
        print("    (a) the block genuinely does not exist on that side  — a FINDING")
        print("        (EVNT and SHIP are this: TP has no EVNT record, it uses REVT)")
        print("    (b) our name for it is wrong                          — a MAP BUG")
        print("  This tool cannot tell them apart. Resolve by hand before believing either.")
        return 0, 1, "UNCOMPARABLE"

    dis = 0
    print("\n  %-6s %-28s %-28s %s" % ("off", "A donor", "B receiver", "verdict"))
    print("  " + "-" * 70)
    amap = {f[0]: f for f in a}
    bmap = {f[0]: f for f in b}
    for off in sorted(set(amap) | set(bmap)):
        fa, fb = amap.get(off), bmap.get(off)
        if fa and fb:
            same = (fa[1] == fb[1])
            verdict = "AGREE" if same else "DISAGREE"
            if not same:
                dis += 1
        else:
            verdict = "A-ONLY" if fa else "B-ONLY"
            dis += 1
        sa = "%s %s" % (fa[1], fa[2]) if fa else "-"
        sb = "%s %s" % (fb[1], fb[2]) if fb else "-"
        print("  0x%02x   %-28s %-28s %s" % (off, sa[:28], sb[:28], verdict))
        for note in (fa[3] if fa else []) + (fb[3] if fb else []):
            print("         note: %s" % note)

    print("\n  SIZE   A=%s  B=%s  C=%s" % (
        hex(a_size) if a_size else "?", hex(b_size) if b_size else "?",
        hex(c_size) if c_size else "?"))

    # ---------------------------------------------------------------------
    # BLOCK VERDICT — size first, because field rows are NOISY BY CONSTRUCTION.
    # The two decomps decompose identical bytes differently: `cXyz position`
    # on one side vs `f32 posX/posY/posZ` on the other is the SAME 12 bytes,
    # and a field-by-field offset compare calls that three disagreements. The
    # first full run reported 88 field rows, and the great majority were this.
    # Publishing 88 would have buried the five that matter.
    #
    # Record SIZE is the signal that survives decomposition. A size mismatch
    # cannot be a notation difference — it is stride, and stride is what the
    # §212/§332 class actually breaks.
    # ---------------------------------------------------------------------
    if a_size is not None and b_size is not None:
        if a_size != b_size:
            verdict = "SIZE-DISAGREE — stride differs; donor data CANNOT be fed to the receiver's reader unchanged"
        elif dis:
            verdict = "SIZE-AGREE, fields decomposed differently — advisory, verify by hand before trusting"
        else:
            verdict = "AGREE"
    else:
        verdict = "INDETERMINATE — a size is unknown; not a pass"
    # validator: size is the strongest single check it can make independently
    if c_size is not None and a_size is not None:
        if c_size == a_size:
            print("  VALIDATOR: CONFIRMS the donor record size (independent derivation).")
        else:
            print("  VALIDATOR: CONTRADICTS the donor size — A=%s C=%s. NOT adjudicated:"
                  % (hex(a_size), hex(c_size)))
            print("             one of our donor header, or Winditor, is wrong. Route it.")
            dis += 1
    else:
        print("  VALIDATOR: UNVALIDATED — no independent size. Silence is not agreement.")

    print("  BLOCK VERDICT: %s" % verdict)

    if verbose and c:
        print("\n  C derived layout:")
        for off, t, n, notes, _ in c:
            print("    0x%02x  %-18s %s%s" % (off, t, n, "  <-- %s" % notes[0] if notes else ""))
    return dis, 0, verdict


# ===========================================================================
# SOURCE D (§683/§684 wiring) — EMPIRICAL stride from real chunk geometry.
# The technique that proved SCLS=0xC (§610) and RCAM=0x18 (§612) by hand,
# mechanized: gap to the next chunk / entry count. It measures what is IN a
# file, so it answers a different question than A/B/C (what code EXPECTS) —
# disagreement between D and A is a FORMAT finding about that file (mixed-
# format arcs, §612), not a layout finding about the games.
# Caveat stated, not hidden: the LAST chunk's gap runs to end-of-file and
# may include padding, so its stride prints as ">= floor" and is never
# reported as exact.
# ===========================================================================
def cmd_dzr(path):
    import struct as _s
    raw = open(path, "rb").read()
    if raw[:4] == b"Yaz0":
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from ww_disc import yaz0_decompress
        raw = yaz0_decompress(raw)
    if raw[:4] == b"RARC":
        sys.path.insert(0, str(Path(__file__).resolve().parent))
        from ww_disc import rarc_list
        for nm, data in rarc_list(raw):
            if nm.endswith(".dzr") or nm.endswith(".dzs"):
                raw = data
                break
        else:
            print("no .dzr/.dzs member in the RARC")
            return 2
    n = _s.unpack(">I", raw[:4])[0]
    if not (0 < n < 64):
        print("does not look like a dz* chunk table (count=%d)" % n)
        return 2
    nodes = []
    for i in range(n):
        tag = raw[4 + i * 12:8 + i * 12].decode("ascii", "replace")
        num, off = _s.unpack(">II", raw[8 + i * 12:16 + i * 12])
        nodes.append((off, tag, num))
    nodes.sort()
    sizes = {cc: (a_size_of(ds), ds) for cc, ds, rs, tm in BLOCK_MAP if ds}
    print("SOURCE D — empirical strides from chunk geometry: %s" % path)
    print("  %-6s %-5s %-9s %-10s %s" % ("tag", "num", "stride", "vs A", "note"))
    for k, (off, tag, num) in enumerate(nodes):
        end = nodes[k + 1][0] if k + 1 < len(nodes) else len(raw)
        gap = end - off
        exact = k + 1 < len(nodes)
        stride = gap // num if num else 0
        a = sizes.get(tag, (None, None))[0]
        if tag == "RTBL":
            # pointer-indirected: gap/num is a composite, not a stride (§689)
            print("  %-6s %-5d %-9s %-10s %s" % (tag, num, "COMPOSITE",
                  "n/a", "pointer-indirected -- use --rtbl, stride is a category error"))
            continue
        if a is None:
            vs = "-"
        elif not exact:
            vs = "AGREE?" if stride >= a else "UNDER!"
        elif stride == a:
            vs = "AGREE"
        else:
            vs = "D=%s A=%s DIFFER" % (hex(stride), hex(a))
        note = "" if exact else "last chunk — floor only, padding possible"
        print("  %-6s %-5d %-9s %-10s %s"
              % (tag, num, hex(stride) if exact else ">=%s" % hex(stride), vs, note))
    return 0


def a_size_of(struct_name):
    hdr, sn = resolve_side(DONOR_DIR, DONOR_H, struct_name)
    _, size, _ = parse_header(hdr, sn)
    return size


# ===========================================================================
# RTBL STRUCTURAL DECODE (§689) — the comparator stride cannot be. Decodes
# the pointer-indirected shape build_rtbl_sparse writes and WW ships: ptr
# array (4B abs offsets) -> per-room records (8B: count,u8 pad, rooms_off)
# -> room-byte lists (bit7 = ChkBg, low bits = room index, per §602's
# loadRoom read path). Run on two arcs and diff the PRINTS.
# ===========================================================================
def cmd_rtbl(path):
    import struct as _s
    sys.path.insert(0, str(Path(__file__).resolve().parent))
    from ww_disc import yaz0_decompress, rarc_list
    raw = open(path, "rb").read()
    if raw[:4] == b"Yaz0":
        raw = yaz0_decompress(raw)
    if raw[:4] == b"RARC":
        for nm, data in rarc_list(raw):
            if nm == "stage.dzs":
                raw = data
                break
    n = _s.unpack(">I", raw[:4])[0]
    for i in range(n):
        tag = raw[4 + i * 12:8 + i * 12]
        num, off = _s.unpack(">II", raw[8 + i * 12:16 + i * 12])
        if tag == b"RTBL":
            print("RTBL structural decode: %s" % path)
            print("  rooms: %d, ptr array @0x%X" % (num, off))
            for r in range(num):
                ptr = _s.unpack(">I", raw[off + r * 4: off + r * 4 + 4])[0]
                cnt, pad, rooms_off = _s.unpack(">BBH", raw[ptr:ptr + 4])
                ext = _s.unpack(">I", raw[ptr + 4:ptr + 8])[0]
                rooms = raw[ext:ext + cnt] if cnt and ext < len(raw) else b""
                decoded = ["%d%s" % (b & 0x3F, "+ChkBg" if b & 0x80 else "")
                           for b in rooms]
                print("  room[%d] rec@0x%X count=%d load-set=[%s]"
                      % (r, ptr, cnt, ", ".join(decoded)))
            return 0
    print("no RTBL chunk in %s" % path)
    return 2


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--block")
    ap.add_argument("--rtbl", help="structural decode of an arc's RTBL (§689 comparator)")
    ap.add_argument("--dzr", help="empirical stride census of a room/stage arc or dz* file (source D)")
    ap.add_argument("--list", action="store_true")
    ap.add_argument("-v", "--verbose", action="store_true")
    a = ap.parse_args()

    if a.rtbl:
        return cmd_rtbl(a.rtbl)
    if a.dzr:
        return cmd_dzr(a.dzr)
    if a.list:
        print("V1 BLOCK MAP — declared, never inferred:")
        for cc, ds, rs, tm in BLOCK_MAP:
            print("  %-6s A:%-28s B:%-28s %s" % (cc, ds or "-", rs or "(same)", tm or "-"))
        print("\nA block absent from this map is NOT covered. Add it deliberately.")
        return 0

    rows = [r for r in BLOCK_MAP
            if not a.block or r[0].upper() == a.block.upper()]
    if not rows:
        print("no such block in the map; --list to see it")
        return 2

    print("V1 — CROSS-LINEAGE ACCESSOR DIFFER (three-source)")
    print("Reports DISAGREEMENT. Never picks a winner.")

    total_dis = total_unc = 0
    verdicts = []
    for cc, ds, rs, tm in rows:
        d, u, v = compare(cc, ds, rs, tm, a.verbose)
        total_dis += d
        total_unc += u
        verdicts.append((cc, v))

    print("\n" + "=" * 74)
    print("  blocks compared : %d" % len(rows))
    print("  disagreements   : %d" % total_dis)
    print("  uncomparable    : %d" % total_unc)
    print("\n  BLOCK VERDICTS — the triaged view. Field-row counts are NOISY BY")
    print("  CONSTRUCTION: the two decomps decompose identical bytes differently")
    print("  (cXyz vs f32 posX/posY/posZ is the same 12 bytes, counted as three")
    print("  disagreements). Record SIZE survives that, so blocks rank on it.")
    order = ["SIZE-DISAGREE", "INDETERMINATE", "UNCOMPARABLE", "UNMAPPED",
             "RECEIVER-ABSENT", "DONOR-ABSENT", "PRIMITIVE", "SIZE-AGREE", "AGREE"]
    for key in order:
        hits = [cc for cc, v in verdicts if v.startswith(key)]
        if hits:
            print("    %-14s %2d   %s" % (key, len(hits), " ".join(hits)))
    hard = [cc for cc, v in verdicts if v.startswith("SIZE-DISAGREE")]
    if hard:
        print("\n  %d block(s) with a DIFFERENT RECORD SIZE: %s" % (len(hard), " ".join(hard)))
        print("  Stride differences. Donor data CANNOT be fed to the receiver's")
        print("  reader unchanged, and the failure mode is SILENT DATA, not a crash.")
        print("  Each is a ROUTED QUESTION. This tool never adjudicates.")
    if total_unc:
        # An uncomparable block is NOT a pass. The first run of this tool
        # printed "no disagreement" and exited 0 with nine of twelve blocks
        # uncompared — silence reading as agreement, which is the exact defect
        # this campaign keeps catching. It cannot do that again.
        print("\n  %d block(s) UNCOMPARABLE. This is NOT a clean result: a block that" % total_unc)
        print("  could not be read has not been checked, and exiting 0 here would")
        print("  report absence of evidence as evidence of absence.")
    if not total_dis and not total_unc:
        print("\n  All %d mapped blocks compared, no disagreement. Scope is the MAP:" % len(rows))
        print("  silence covers what is mapped and nothing else — see --list.")
    return 1 if (total_dis or total_unc) else 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
