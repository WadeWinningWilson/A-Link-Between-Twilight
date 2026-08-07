#!/usr/bin/env python3
"""Extract A_mori room.dzr ACTR-family placements into interior_placements.csv.

WW DZR layout: u32 chunkHdrCount; then count×{tag[4], entryCount, dataOff};
entries at dataOff. ACTR/ACTn = 0x20, SCOB/SCOn = 0x24.
"""
from __future__ import annotations

import os
import struct
import sys
from collections import Counter
from pathlib import Path

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration"
)
ARC = MOD / "arcs" / "A_mori.arc"
CSV = MOD / "population" / "interior_placements.csv"
STAGE = "A_mori"


def be32(d: bytes, o: int) -> int:
    return struct.unpack_from(">I", d, o)[0]


# ============================================================================
# Yaz0. Some arcs ship compressed -- Mshokki, Okmono and Vshin in the current
# install. Without this they are not "arcs we chose to skip", they are arcs the
# census never opened while still reporting a clean result, which is the
# silent-undercount shape №31-C exists to forbid.
#
# Format: "Yaz0", u32 decompressed size, 8 reserved (header 0x10). Then groups
# of one control byte, MSB first: bit set = copy one literal byte; bit clear =
# back-reference, 2 bytes giving dist = (b0 & 0xF) << 8 | b1, plus 1, and
# count = b0 >> 4, plus 2 -- or, when that nibble is 0, a third byte holding
# count - 0x12. Copies may overlap, so this must copy byte-at-a-time.
# ============================================================================
def yaz0_decompress(src: bytes) -> bytes:
    size = be32(src, 4)
    out = bytearray(size)
    si, di = 0x10, 0
    code, bits = 0, 0
    while di < size:
        if bits == 0:
            code = src[si]
            si += 1
            bits = 8
        if code & 0x80:
            out[di] = src[si]
            si += 1
            di += 1
        else:
            b0, b1 = src[si], src[si + 1]
            si += 2
            dist = ((b0 & 0x0F) << 8 | b1) + 1
            n = b0 >> 4
            if n == 0:
                n = src[si] + 0x12
                si += 1
            else:
                n += 2
            start = di - dist
            for k in range(n):
                if di >= size:
                    break
                out[di] = out[start + k]
                di += 1
        code = (code << 1) & 0xFF
        bits -= 1
    return bytes(out)


def maybe_decompress(data: bytes) -> bytes:
    """Return RARC bytes, transparently decompressing a Yaz0 container."""
    if data[:4] == b"Yaz0":
        return yaz0_decompress(data)
    return data


def list_rarc(data: bytes):
    assert data[:4] == b"RARC"
    data_abs = 0x20 + be32(data, 0x0C)
    info = 0x20
    n = be32(data, info + 0x08)
    ent = info + be32(data, info + 0x0C)
    strs = info + be32(data, info + 0x14)
    out = []
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", data, e + 4)[0] & 0x1100 != 0x1100:
            continue
        name_off = struct.unpack_from(">H", data, e + 6)[0]
        end = data.index(b"\0", strs + name_off)
        name = data[strs + name_off : end].decode("ascii", "replace")
        doff, size = be32(data, e + 8), be32(data, e + 12)
        out.append((name, data_abs + doff, size))
    return out


# ============================================================================
# B2b -- CHUNK COVERAGE (user ruled 2026-08-07: "all 15 to be WW owned").
#
# HOW THESE SIZES WERE ESTABLISHED, because a wrong stride silently produces
# plausible garbage rather than an error:
#
#   1. MEASURED from the data. Every chunk header carries (count, dataOffset),
#      so entry size = (next chunk's offset - this offset) / count, taken only
#      where the division is EXACT, across all 83 live arcs.
#   2. CONTROLLED. The method had to reproduce the three sizes already known --
#      ACTR 0x20 (11/11 arcs), SCOB 0x24 (9/9), SCLS 0x0C (13/13). It did.
#   3. CORROBORATED against the donor's own structs in d_stage.h, which WIN on
#      conflict.
#
# The control earned itself immediately: STAG measured 0x14 but
# `stage_stag_info_class` is 0x20. The measurement is unreliable for the LAST
# chunk in a file, where the span runs to EOF rather than to the next chunk --
# so single-sample tags are only trusted when the donor agrees.
# ============================================================================
# Each row is (size, layout, evidence). LAYOUT is the part that matters most:
# the parser previously assumed every chunk used ACTR's record shape, which is
# true for exactly one of these fifteen. The donor structs settle it.
#
#   "actr"  name[8] @0x00, params @0x08, pos @0x0C, rot @0x18
#   "sond"  name[8] @0x00, pos @0x08 (Vec), flags @0x14 -- no params, no rot
#   "raro"  pos @0x00, angle @0x0C (csXyz) -- NO name field at all
#   "meta"  no world position. Room/lighting/camera/environment tables.
#
# "meta" chunks are counted and reported, never emitted as placement rows:
# interior_placements.csv has x/y/z columns and a palette table has no
# coordinates to put in them. Inventing some would be worse than omitting them.
_CHUNK_SIZES = {
    # Route/path/light-bank chunks. These were NOT among the fifteen B2b was
    # scoped to; the census surfaced them, which is what the census is for.
    "RPPN": (0x10, "rppn", "dPnt (d_path.h) 4x u8 arg + cXyz @0x04"),
    "RPAT": (0x0C, "meta", "dPath (d_path.h); m_points is a file offset on disk"),
    "LBNK": (0x01, "meta", "dStage_Lbnk_c m_entries is u8*"),
    "SCLS": (0x0C, "meta", "stage_scls_info_class 0xC + measured 13/13 (exit table)"),
    "FILI": (0x08, "meta", "measured 12/12 arcs, unanimous"),
    "LGTV": (0x1C, "meta", "stage_lightvec_info_class 0x1C + measured 7/7"),
    "RCAM": (0x14, "meta", "stage_camera2_data_class 0x14 + measured 8/8"),
    "SOND": (0x1C, "sond", "stage_sound_data 0x1C + measured 6/6"),
    "RARO": (0x14, "raro", "stage_arrow_data_class 0x14 + measured 7/8"),
    # SHIP carries cXyz m_pos at 0x00 -- it IS a placement. Previously labelled
    # "meta" only because the donor struct had not been located yet.
    "SHIP": (0x10, "ship", "dStage_Ship_dt_c 0x10: cXyz @0x00, s16 angle @0x0C"),
    # --- formerly HELD (§560), decoded once the donor structs were located ----
    # They were missed by a grep for `stage_event_*` / `stage_mult_*`; the donor
    # names them `dStage_*_dt_c`. The hold was still right: RTBL's measured 0x0E
    # was WRONG (donor says 0x08), which is precisely the single-sample failure
    # the STAG control predicted, and no stride measurement could have revealed
    # that RTBL is two-level.
    "EVNT": (0x18, "meta", "dStage_Event_dt_c 0x18: char mName[15] @0x04; no position"),
    "MULT": (0x0C, "meta", "dStage_Mult_info 0xC: room-placement 2D transform, not an actor"),
    "RTBL": (0x08, "meta", "roomRead_data_class 0x08 -- OVERRIDES measured 0x0E; "
                           "m_rooms @0x04 is a file offset to a variable-length list"),
    "EnvR": (0x08, "meta", "stage_envr_info_class 0x8 + measured"),
    "Colo": (0x0C, "meta", "stage_pselect_info_class 0xC + measured"),
    "Pale": (0x2C, "meta", "stage_palet_info_class 0x2C + measured"),
    "Virt": (0x24, "meta", "stage_vrbox_info_class 0x24 + measured"),
    "STAG": (0x20, "meta", "stage_stag_info_class 0x20 -- DONOR OVERRIDES measured 0x14"),
    "2DMA": (0x38, "meta", "stage_map_info_class 0x38 (donor only; no exact measurement)"),
}

def entry_size(tag: str) -> int | None:
    if tag.startswith("ACT"):
        return 0x20
    if tag.startswith("SCO"):
        return 0x24
    if tag in ("TGOB", "TGSC", "TGDR", "Door", "DOOR", "TRES", "PLYR"):
        return 0x24 if tag in ("TGSC", "TGDR", "SCOB", "Door", "DOOR") else 0x20
    hit = _CHUNK_SIZES.get(tag)
    if hit is not None:
        return hit[0]
    return None


def entry_layout(tag: str) -> str:
    if tag.startswith("ACT") or tag.startswith("SCO"):
        return "actr"
    if tag in ("TGOB", "TGSC", "TGDR", "Door", "DOOR", "TRES", "PLYR"):
        return "actr"
    hit = _CHUNK_SIZES.get(tag)
    return hit[1] if hit is not None else "unknown"


# ============================================================================
# B2d -- RPAT path headers, and the association to their RPPN points.
#
# THE DEFECT THIS CLOSES. RPPN waypoints were emitted and RPAT discarded, so
# what shipped was 474 positionally-correct but semantically ORPHANED points:
# no way to say which route a point belongs to, in what order, or whether the
# route closes. That is worse than an absent feature -- a consumer reading the
# CSV today gets data that looks whole and is not.
#
# THE ASSOCIATION, VERIFIED BEFORE BUILDING (Foundry flagged their own ruling
# as inference, correctly). `dPath.m_points` at 0x08 is NOT an absolute file
# offset -- checking it as one puts 38 of 67 outside the RPPN chunk. It is an
# offset RELATIVE TO THE RPPN CHUNK BASE, so index = m_points / 0x10. With that
# reading, across all 8 stages carrying both chunks:
#
#   67/67 pointers resolve to a valid RPPN index with the run inside the chunk
#   67/67 equal the sequential running offset
#    8/8  stages: the RPAT runs cover every RPPN index EXACTLY -- a perfect
#         partition, nothing orphaned and nothing claimed twice
#   sum(m_num) == RPPN count exactly, per stage and in total (474)
# ============================================================================
def read_paths(blob: bytes):
    """Returns (paths, pnt_path).

    paths    : list of (path_id, num, next_id, arg0, closed)
    pnt_path : RPPN index -> (path_id, ordinal within that path)

    Empty when the stage carries no RPAT/RPPN pair, so callers need no special
    case for the 5 stages that have neither.
    """
    hdr_n = be32(blob, 0)
    chunks = {}
    for i in range(hdr_n):
        o = 4 + i * 12
        if o + 12 > len(blob):
            break
        chunks[blob[o:o + 4].decode("ascii", "replace")] = (
            be32(blob, o + 4), be32(blob, o + 8))
    if "RPAT" not in chunks or "RPPN" not in chunks:
        return [], {}
    pat_n, pat_off = chunks["RPAT"]
    ppn_n, _ppn_off = chunks["RPPN"]
    paths, pnt_path = [], {}
    for k in range(pat_n):
        e = pat_off + k * 0x0C
        if e + 0x0C > len(blob):
            break
        num, next_id = struct.unpack_from(">HH", blob, e)
        arg0, closed = blob[e + 4], blob[e + 5]
        first = struct.unpack_from(">I", blob, e + 8)[0] // 0x10
        # Refuse to invent membership if the run does not fit. Silence here
        # would reproduce the orphaning this exists to fix, one level in.
        if first + num > ppn_n:
            continue
        paths.append((k, num, next_id, arg0, closed))
        for j in range(num):
            pnt_path[first + j] = (k, j)
    return paths, pnt_path


def parse_dzr(blob: bytes):
    """Returns (placement_rows, census, paths).

    census covers EVERY chunk in the file including ones we cannot decode, so
    that "not extracted" is visible in the output rather than being an absence
    nobody can see. A chunk that silently vanishes reads as a chunk that is not
    there.
    """
    hdr_n = be32(blob, 0)
    rows = []
    paths, pnt_path = read_paths(blob)
    census = []
    for i in range(hdr_n):
        o = 4 + i * 12
        tag = blob[o : o + 4].decode("ascii", "replace")
        count = be32(blob, o + 4)
        data_off = be32(blob, o + 8)
        es = entry_size(tag)
        layout = entry_layout(tag)
        census.append((tag, count, es, layout))
        if es is None or count <= 0:
            continue
        layer = "-"
        # Layer suffixes are DIGITS and LOWERCASE letters (ACT0-9, ACTa, ACTb,
        # SCO0-3, SCOb). This set was previously uppercase hex, which had two
        # effects, NOW CORRECTED -- the `layer` column changes for some rows and
        # that is expected, not a coverage change:
        #   - `SCOB`, the BASE chunk, was labelled "layer B". It has no layer.
        #   - `ACTa`/`ACTb`, which ARE layers, were labelled "-".
        # Cosmetic either way: the extractor gates on the chunk tag and never on
        # this field, so no placement was included or excluded because of it.
        if len(tag) == 4 and tag[3] in "0123456789abcdef" and tag[:3] in ("ACT", "SCO"):
            layer = tag[3]
        if layout == "meta" or layout == "unknown":
            continue  # counted above; has no coordinates to report
        for k in range(count):
            e = data_off + k * es
            if e + es > len(blob):
                break
            if layout == "actr":
                name = blob[e : e + 8].split(b"\0", 1)[0].decode("ascii", "replace").strip()
                if not name:
                    continue
                params = be32(blob, e + 8)
                x, y, z = struct.unpack_from(">fff", blob, e + 0xC)
                rx, ry, rz = struct.unpack_from(">hhh", blob, e + 0x18)
            elif layout == "sond":
                name = blob[e : e + 8].split(b"\0", 1)[0].decode("ascii", "replace").strip()
                if not name:
                    continue
                params = 0
                x, y, z = struct.unpack_from(">fff", blob, e + 0x08)
                rx = ry = rz = 0
            elif layout == "ship":
                # Ship spawn point: position, then a heading. No name field.
                name = f"ship{k}"
                params = 0
                x, y, z = struct.unpack_from(">fff", blob, e + 0x00)
                rx, rz = 0, 0
                (ry,) = struct.unpack_from(">h", blob, e + 0x0C)
            elif layout == "rppn":
                # Route waypoint. Four u8 args, then the position.
                # B2d: carry the RPAT grouping in the name -- `p<path>.<ord>` --
                # so a consumer can recover which route the point belongs to and
                # its order within it. `pnt<k>` alone was the orphaned form.
                # A point with NO path is named so explicitly rather than being
                # given a plausible-looking default.
                owner = pnt_path.get(k)
                name = f"p{owner[0]}.{owner[1]}" if owner else f"orphan{k}"
                params = be32(blob, e + 0x00)
                x, y, z = struct.unpack_from(">fff", blob, e + 0x04)
                rx = ry = rz = 0
            elif layout == "raro":
                # No name field in the record. Index it, rather than leaving the
                # column blank and letting it look like a decode failure.
                name = f"arrow{k}"
                params = 0
                x, y, z = struct.unpack_from(">fff", blob, e + 0x00)
                rx, ry, rz = struct.unpack_from(">hhh", blob, e + 0x0C)
            else:
                continue
            rows.append((tag, layer, name, params, x, y, z, rx, ry, rz))
    return rows, census, paths


def main() -> int:
    if not ARC.is_file():
        print("missing", ARC, file=sys.stderr)
        return 1
    data = ARC.read_bytes()
    dzr = None
    for name, off, size in list_rarc(data):
        if name == "room.dzr":
            dzr = data[off : off + size]
            break
    if dzr is None:
        print("no room.dzr", file=sys.stderr)
        return 1
    rows, census, paths = parse_dzr(dzr)
    counts = Counter(r[2] for r in rows)
    print("rows", len(rows))
    print("name counts:", sorted(counts.items(), key=lambda kv: (-kv[1], kv[0])))
    for key in ("mo2", "Bk", "gmos", "kuro_t", "Zl1", "P1a", "P1b"):
        print(key, counts.get(key, 0))

    text = CSV.read_text(encoding="utf-8", errors="replace") if CSV.is_file() else ""
    # Drop prior A_mori block if re-running.
    lines = [ln for ln in text.splitlines() if not ln.startswith("A_mori,")]
    if not lines or not lines[0].startswith("stage,"):
        lines = ["stage,chunk,layer,name,params_hex,x,y,z,rx,ry,rz"] + [
            ln for ln in lines if ln and not ln.startswith("stage,")
        ]
    for chunk, layer, name, params, x, y, z, rx, ry, rz in rows:
        lines.append(
            f"{STAGE},{chunk},{layer},{name},{params:08x},{x:.1f},{y:.1f},{z:.1f},{rx},{ry},{rz}"
        )
    CSV.write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"wrote {len(rows)} A_mori rows -> {CSV}")

    # B2d: the path HEADERS. RPAT carries no world position, so it gets its own
    # file rather than being forced into a CSV with x/y/z columns -- the same
    # rule applied to every other NO-PLACEMENT chunk type.
    if paths:
        pf = CSV.parent / "paths.csv"
        prior = [ln for ln in (pf.read_text(encoding="utf-8", errors="replace").splitlines()
                               if pf.is_file() else []) if not ln.startswith(f"{STAGE},")]
        if not prior or not prior[0].startswith("stage,"):
            prior = ["stage,path_id,num_points,next_id,arg0,closed"] + [
                ln for ln in prior if ln and not ln.startswith("stage,")]
        for pid, num, nxt, arg0, closed in paths:
            prior.append(f"{STAGE},{pid},{num},0x{nxt:04X},{arg0},{closed}")
        pf.write_text("\n".join(prior) + "\n", encoding="utf-8")
        print(f"wrote {len(paths)} path header(s) -> {pf}")

    # B2b coverage report. Printed unconditionally so an undecoded chunk is a
    # visible line rather than a silent omission.
    print("\n-- chunk census (B2b) --")
    for tag, count, es, layout in census:
        size = f"0x{es:02X}" if es is not None else "  ?"
        note = {"meta": "no coordinates -- counted only",
                "unknown": "NOT DECODED"}.get(layout, "placements")
        print(f"   {tag:5s} n={count:<4d} size={size}  {layout:8s} {note}")
    undec = [c[0] for c in census if c[2] is None]
    if undec:
        print(f"   UNDECODED: {', '.join(sorted(set(undec)))}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
