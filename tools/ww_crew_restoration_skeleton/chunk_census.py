#!/usr/bin/env python3
# ============================================================================
# chunk_census.py -- measure DZR/DZS chunk-type coverage across the LIVE arcs.
#
# WHY THIS EXISTS
# docs/state/ww-b2-chunk-coverage.txt carried the authoritative "which chunk
# types are unhandled" figures, and had NO GENERATOR -- a grep of tools/ for its
# vocabulary returned nothing. It was produced ad-hoc and could not be
# re-measured, so every later claim about coverage cited a number nobody could
# reproduce. That is the same defect HT-28 caught one level up: a verdict that
# cannot name its input is not reproducible.
#
# This measures it from the arcs, every time, and classifies each type by what
# we can actually DO with it rather than by a single handled/unhandled bit:
#
#   UNSIZED    entry_size is None. We cannot even walk the records. Genuinely
#              unhandled -- this is the set B2b-1 should target.
#   SIZED      stride known, records walkable, but no field decode: counted,
#              never emitted. Room/lighting/camera/environment tables.
#   PLACED     fields decoded and emitted as placement rows.
#
# The distinction matters because "handled" collapsed all three and made a
# counted-but-not-emitted table indistinguishable from an undecodable one.
#
# Read-only with respect to the arcs. Usage:
#   chunk_census.py            report
#   chunk_census.py --emit     also rewrite docs/state/ww-b2-chunk-coverage.txt
# ============================================================================
from __future__ import annotations

import os
import pathlib
import sys
from collections import defaultdict

REPO = pathlib.Path(__file__).resolve().parents[2]
EXTRACTOR = REPO / "tools/ww_crew_restoration_skeleton/extract_amori_census.py"
DOC = REPO / "docs/state/ww-b2-chunk-coverage.txt"
LIVE = pathlib.Path(os.path.expandvars("%APPDATA%")) / (
    "TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration")


# ============================================================================
# DISPOSITIONS (§565). Knowing a record stride is the PRECONDITION for handling
# a chunk, not handling it -- so a predicate that only asks "is it sized?"
# reports 43/43 while 15 types produce nothing. That is the same defect caught
# in WWB:B2 and WWB:B4: keyed to the first property achieved rather than to the
# item's scope.
#
# So every type that is NOT placed must carry an explicit disposition and a
# REASON. Only non-placed types appear here: placement types are derived from
# the extractor's own layout table, so adding one needs no edit here, while
# anything that stops short of placement has to say why.
#
# A chunk type appearing in a new arc with no entry below is UNCLASSIFIED and
# fails the check. That is the tightening property -- silence is not consent.
#
#   DELIVERED     handled to completion, by a NAMED tool other than the
#                 placement extractor.
#   NO-PLACEMENT  decoded, but structurally carries no world position. There is
#                 no honest cell for it in a CSV with x/y/z columns.
#   DEFERRED      decodable and content-bearing, but nothing consumes it yet.
#                 Real owed work; the reason names what is missing.
# ============================================================================
DISPOSITION = {
    # --- delivered elsewhere -------------------------------------------------
    "SCLS": ("DELIVERED", "ww_bridge `scls` -> population/door_bindings.ini "
                          "(968 bindings). Exits are not placements: they have "
                          "no world position, so PLACED was never the target."),
    # --- decoded, nothing to place ------------------------------------------
    "Pale": ("NO-PLACEMENT", "stage_palet_info_class: lighting palette"),
    "Colo": ("NO-PLACEMENT", "stage_pselect_info_class: palette selector"),
    "EnvR": ("NO-PLACEMENT", "stage_envr_info_class: environment table"),
    "Virt": ("NO-PLACEMENT", "stage_vrbox_info_class: skybox colours"),
    "LGTV": ("NO-PLACEMENT", "stage_lightvec_info_class: light vector"),
    "LBNK": ("NO-PLACEMENT", "dStage_Lbnk_c: per-room light bank indices"),
    "FILI": ("NO-PLACEMENT", "dStage_FileList_dt_c: room param + sea level"),
    "2DMA": ("NO-PLACEMENT", "stage_map_info_class: 2D map layout floats"),
    "STAG": ("NO-PLACEMENT", "stage_stag_info_class: stage-wide parameters"),
    # --- owed ----------------------------------------------------------------
    "RPAT": ("DELIVERED", "B2d: extracted to population/paths.csv, and the "
                          "grouping carried on each RPPN row as `p<path>.<ord>`. "
                          "Not PLACED because dPath has no world position -- the "
                          "same rule as every NO-PLACEMENT type. Association "
                          "verified before building: m_points is relative to the "
                          "RPPN chunk base, 67/67 resolve, 8/8 stages partition "
                          "their points exactly (§566)."),
    "RCAM": ("DEFERRED", "stage_camera2_data_class: camera definitions keyed to "
                         "a RARO arrow index. No consumer asks for them."),
    "EVNT": ("DEFERRED", "dStage_Event_dt_c: event names, decoded §562. "
                         "No consumer asks for them."),
    "MULT": ("DEFERRED", "dStage_Mult_info: room-placement transforms (the "
                         "room graph), decoded §562. No consumer."),
    "RTBL": ("DEFERRED", "roomRead_data_class: room adjacency, decoded §562. "
                         "Two-level; the room lists are read but unused."),
}


def _load_extractor():
    """Reuse the extractor's own tables, so the census cannot drift from it."""
    ns: dict = {}
    exec(compile(EXTRACTOR.read_text(encoding="utf-8"), str(EXTRACTOR), "exec"), ns)
    return ns


def scan():
    ns = _load_extractor()
    be32, list_rarc = ns["be32"], ns["list_rarc"]
    maybe_decompress = ns["maybe_decompress"]
    entry_size, entry_layout = ns["entry_size"], ns["entry_layout"]

    entries: dict[str, int] = defaultdict(int)
    arcs_with: dict[str, set] = defaultdict(set)
    stages = 0
    # Arcs we could NOT examine. Previously these were swallowed by a bare
    # `continue`, so 3 Yaz0 arcs and 1 malformed one were skipped while the
    # census still reported a clean result -- a silent undercount that would
    # let B2c pass while blind to arcs it claims to cover. An unexaminable arc
    # must be visible to the caller, not absorbed (№31-C).
    skipped: list[tuple[str, str]] = []

    arc_dirs = [LIVE / "arcs", LIVE / "arcs_lib"]
    files = [p for d in arc_dirs if d.is_dir() for p in sorted(d.glob("*.arc"))]
    for path in files:
        try:
            data = maybe_decompress(path.read_bytes())
            members = list(list_rarc(data))
        except Exception as exc:
            skipped.append((path.name, f"{type(exc).__name__}: {exc}"))
            continue
        if not members:
            # An EMPTY archive is not an unexamined one. Hitobj.arc is a valid
            # 192-byte RARC whose single node holds only the "." / ".." pair --
            # it parsed correctly and contains nothing. Counting that as a skip
            # would force B2c to UNKNOWN forever over an arc with nothing in it,
            # which is the mirror of the undercount: refusing a verdict we are
            # actually entitled to. Only a PARSE failure blocks.
            if data[:4] != b"RARC":
                skipped.append((path.name, "not RARC and not Yaz0"))
            continue
        for name, off, size in members:
            if not name.endswith((".dzr", ".dzs")):
                continue
            blob = data[off:off + size]
            if len(blob) < 8:
                continue
            try:
                n = be32(blob, 0)
            except Exception:
                continue
            # A plausible chunk-table length. A bad offset yields a huge count.
            if n <= 0 or n > 200 or 4 + n * 12 > len(blob):
                continue
            stages += 1
            for i in range(n):
                o = 4 + i * 12
                tag = blob[o:o + 4].decode("ascii", "replace")
                if not tag.isprintable():
                    continue
                entries[tag] += be32(blob, o + 4)
                arcs_with[tag].add(path.name)

    rows = []
    for tag in entries:
        es = entry_size(tag)
        layout = entry_layout(tag)
        if es is None:
            tier = "UNSIZED"
        elif layout in ("meta", "unknown"):
            # Not placed -> it owes an explicit disposition. No entry means
            # UNCLASSIFIED, which fails: a type nobody has ruled on must not
            # inherit a passing state by default.
            tier = DISPOSITION.get(tag, ("UNCLASSIFIED", ""))[0]
        else:
            tier = "PLACED"
        rows.append((tag, entries[tag], len(arcs_with[tag]), es, tier))
    rows.sort(key=lambda r: (-r[1], r[0]))
    return rows, len(files), stages, skipped


def main() -> int:
    if not LIVE.is_dir():
        print(f"live install not found: {LIVE}", file=sys.stderr)
        return 1
    rows, n_arcs, n_stages, skipped = scan()
    def of(*t):
        return [r for r in rows if r[4] in t]

    print(f"CHUNK CENSUS — {n_arcs} arcs, {n_stages} DZR/DZS, "
          f"{len(rows)} distinct chunk types")
    for tier, note in (("PLACED", "extracted as placement rows"),
                       ("DELIVERED", "handled to completion by a named tool"),
                       ("NO-PLACEMENT", "decoded; carries no world position"),
                       ("DEFERRED", "decodable, content-bearing, no consumer"),
                       ("UNCLASSIFIED", "NOT RULED ON — fails the check"),
                       ("UNSIZED", "cannot walk the records")):
        print(f"  {tier:13s} {len(of(tier)):3d}   {note}")
    print()
    for tag, ent, arcs, es, tier in rows:
        size = f"0x{es:02X}" if es is not None else "   ?"
        print(f"  {tag:5s} entries={ent:6d}  arcs={arcs:3d}  size={size}  {tier}")

    owed = of("DEFERRED")
    if owed:
        print("\nDEFERRED — decodable but unconsumed. Each names what is missing:")
        for tag, *_ in owed:
            print(f"   {tag}: {DISPOSITION[tag][1]}")
    bad = of("UNCLASSIFIED", "UNSIZED")
    print()
    if bad:
        print("FAILS: " + ", ".join(f"{r[0]}({r[4]})" for r in bad))
    else:
        print("Every chunk type is placed, delivered, or recorded with a reason.")

    # Coverage of the COVERAGE. A census that cannot open an arc has not
    # measured it, and saying so is the difference between "no undecodable
    # types" and "no undecodable types among the arcs I could read".
    print()
    if skipped:
        print(f"NOT EXAMINED — {len(skipped)} arc(s). Coverage above is "
              f"INCOMPLETE and any verdict drawn from it is UNKNOWN, not clean:")
        for name, why in skipped:
            print(f"   {name:24s} {why}")
    else:
        print("NOT EXAMINED: none — every arc was opened and read.")

    if "--emit" in sys.argv:
        out = [
            "# B2 chunk coverage — MEASURED from the live arcs by chunk_census.py.",
            "# Regenerate: python tools/ww_crew_restoration_skeleton/chunk_census.py --emit",
            f"# distinct chunk types: {len(rows)}   "
            + "   ".join(f"{t}: {len(of(t))}" for t in
                         ("PLACED", "DELIVERED", "NO-PLACEMENT", "DEFERRED",
                          "UNCLASSIFIED", "UNSIZED")),
        ]
        for tag, ent, arcs, es, tier in rows:
            size = f"0x{es:02X}" if es is not None else "?"
            out.append(f"{tag:6s} entries={ent:6d} arcs={arcs:3d} size={size:5s} {tier}")
        DOC.write_text("\n".join(out) + "\n", encoding="utf-8")
        print(f"\nwrote {DOC}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
