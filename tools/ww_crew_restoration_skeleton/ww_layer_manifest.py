#!/usr/bin/env python3
"""§488 roadmap step 2 — GENERATE the WW-layer manifest from files.cmake.

WHY THIS EXISTS
---------------
NEVER-PUSH-STRIP-SET.md's Tier-1 list was hand-maintained. It said 34 files while
the build carried ~46, and it had gone untouched since 2026-07-25. That is not
negligence — it is the guaranteed end state of a hand list guarding a boundary
that does not physically exist in the build. The fix is to stop maintaining a
list and start deriving one.

SCOPE, STATED HONESTLY (the §487 gate ruling applied to this instrument too)
---------------------------------------------------------------------------
v1 classifies by PATH/NAME CONVENTION. That is a heuristic, not lineage:
  * it can MISS a WW-owned TU that follows no convention
  * it can FLAG a receiver-owned TU whose name merely matches
Roadmap step 10 (provenance banners, History) replaces this with DECLARED
lineage read from each file's banner. Until then every report prints
`basis: filename-convention (NOT lineage)` so no reader can mistake the two.

USAGE
  python ww_layer_manifest.py                 # human report
  python ww_layer_manifest.py --json out.json # machine manifest
  python ww_layer_manifest.py --check         # exit 1 if Tier-1 disagrees
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
from pathlib import Path


def _sha256(p: Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()

REPO = Path(__file__).resolve().parents[2]
FILES_CMAKE = REPO / "files.cmake"
STRIP_SET = REPO / "docs" / "NEVER-PUSH-STRIP-SET.md"

# Ordered, most-specific first. Each rule carries WHY it marks WW ownership, so
# a future reader can challenge the rule rather than the verdict.
WW_RULES: list[tuple[str, str]] = [
    (r"(^|/)ja1_[^/]*\.cpp$",        "JAudio1 parallel donor stack (WW audio)"),
    (r"(^|/)evt1_[^/]*\.cpp$",       "JEvent1 parallel donor stack (WW events)"),
    (r"(^|/)mdoext1_[^/]*\.cpp$",    "MDoExt1 parallel donor stack (WW 3D-line)"),
    (r"(^|/)d_a_ext_[^/]*\.cpp$",    "WW-restoration actor"),
    (r"(^|/)d_ext_[^/]*\.cpp$",      "WW-restoration subsystem"),
    (r"(^|/)d_ww_[^/]*\.cpp$",       "WW-specific receiver subsystem"),
    (r"(^|/)ww_[^/]*\.cpp$",         "WW-specific receiver subsystem"),
    (r"(^|/)d_a_ww_[^/]*\.cpp$",     "WW-restoration actor"),
    # NOTE the `(_|\.)`: v1 was `_ww_[^/]*` which REQUIRED a trailing underscore,
    # so `d_kankyo_ww_sky.cpp` matched but `d_kankyo_ww.cpp` — a TU whose name
    # ENDS at `_ww` — did not. HousingTemp's step-5 verification caught that
    # omission (roster 46 -> 47). A convention rule must cover the end-of-name
    # case, not just the infix one.
    (r"(^|/)[^/]*_ww(_|\.)[^/]*\.?c?p?p?$", "WW leg inside a receiver-named TU"),
    (r"(^|/)d_albw_dialogue\.cpp$",  "shared ALBW/WW dialogue surface (§113 note)"),
]


def build_sources() -> list[str]:
    """Every .cpp the build actually compiles. The build is the only authority."""
    text = FILES_CMAKE.read_text(encoding="utf-8", errors="replace")
    # strip cmake comments so a commented-out source never counts as shipped
    text = re.sub(r"#.*", "", text)
    # HT-18 (§493) -- THE PATTERN MUST BE ANCHORED.
    #
    # `(src/...)` is unanchored, so on a libs line it matched the EMBEDDED
    # substring: `libs/JSystem/src/JAudio2/JASChannel.cpp` was stored as
    # `src/JAudio2/JASChannel.cpp`. All 246 libs/ TUs were stored truncated ->
    # they resolve to nothing on disk -> the whole JSystem population became
    # unscannable, so no JSystem TU could ever be declared or evidenced.
    # JASChannel.cpp silently left the roster the moment the census adopted this
    # parser, and the drop was MASKED because `evidenced` stayed at 59: d_door
    # was gained in the same swap that lost JASChannel. A stable count across a
    # classifier change is not evidence of a stable roster.
    #
    # Anchored on a non-path character so the match starts at the real path root,
    # and libs/ is captured explicitly rather than by accident.
    # (Edit by Foundry into Housing's tool, declared: the regression entered
    # through Foundry's adoption of this parser and blocks roadmap step 8.)
    return sorted(set(m.group(1) for m in re.finditer(
        r"(?<![A-Za-z0-9_./-])((?:src|libs)/[A-Za-z0-9_./-]+\.cpp)", text)))


def unresolved_sources() -> list[str]:
    """Roster paths that do not exist on disk.

    THE ONE CONTROL THAT WOULD HAVE CAUGHT ALL THREE DEFECTS this session
    (HousingTemp's observation, §493): their HT-17 substring error, Foundry's
    trailing-comment drop, and HT-18's truncation all produce paths that cannot
    be opened. A roster entry that does not resolve is not a roster entry.
    """
    root = FILES_CMAKE.parent
    return [s for s in build_sources() if not (root / s).is_file()]


# RECEIVER files whose NAME matches a WW rule but whose LINEAGE is TP's. Each
# needs positive evidence, not a hunch — a silent exclusion is as dishonest as a
# silent miss, so these are printed on every run.
#
# This list is the proof that filename convention has a hard ceiling:
# `d_kankyo_ww.cpp` (a real WW leg) and `d_a_e_ww.cpp` (a TP enemy) are
# indistinguishable by name. Roadmap step 10's provenance banners are what
# actually resolve this; until then, exclusions are hand-justified and visible.
EXCLUSIONS: dict[str, str] = {
    "src/d/actor/d_a_e_ww.cpp":
        "TP ENEMY, not WW: fpcNm_E_WW_e, includes f_op_actor_enemy.h and "
        "Z2AudioLib (TP's audio lib). Name merely ends in _ww.",
}


def classify(path: str) -> str | None:
    if path in EXCLUSIONS:
        return None
    for pattern, reason in WW_RULES:
        if re.search(pattern, path):
            return reason
    return None


# ============================================================================
# DECLARED-LINEAGE BASIS (§574, user-ruled 2026-08-07).
#
# WHY THE BASIS CHANGED. The filename basis was measured against declared
# lineage for the first time in §573, once step 10 had bannered every TU, and it
# is wrong in BOTH directions:
#
#   14 files matched the filename rules but declare KIT-DONOR: none -- our own
#      plumbing, including d_albw_dialogue.cpp (a DIFFERENT game's port). The
#      step-11 link test removed them and lost ALBW dialogue, the quest/bag
#      system, menus and letters with them.
#   22 files declare donor lineage and the filename rules MISS -- d_stage.cpp,
#      d_demo.cpp, f_op_msg_mng.cpp, d_particle.cpp, d_door.cpp and 17 actors.
#      Those carry donor content and were on NO never-push list.
#
# Because the errors point opposite ways, "over-inclusion is the safe direction"
# does not rescue the single roster: it is safe for NEVER-PUSH and harmful for a
# BUILD EXCLUSION, and one roster was serving both.
#
# WHAT THIS BASIS STILL CANNOT DO, stated so it is never overread:
#   - `mixed` TUs are donor lines INSIDE receiver-owned files. No file-level
#     roster can express that; dropping d_stage.cpp removes the receiver. They
#     are reported as LEG-STRIP REQUIRED and are step 19's, not step 11's.
#   - A banner is AUTHORED. A missing or wrong one is invisible here exactly as
#     a misleading filename was invisible before. The failure mode MOVES; it
#     does not vanish. The independent cross-check is the covenant gate's string
#     scan, which reads the binary rather than the roster.
# ============================================================================
DONOR_LINEAGES = ("native-port", "bridge-owed")   # excludable wholesale
LEG_LINEAGES = ("mixed",)                          # NOT file-excludable

# ============================================================================
# WHICH LAYER DOES THIS PLUMBING SERVE? (§576)
#
# THE CRITERION WAS WRONG, not merely incomplete. Excluding on donor lineage
# alone left the WW bridges in the build while removing the implementations they
# exist to call, producing 30 unresolved symbols whose referencing TUs were the
# bridges THEMSELVES (§575). The instinct was to add a "dependency rule". That
# is treating the symptom.
#
# Step 20 already ruled the end state: a PREBUILT PLUGIN, the tree splitting
# into pure-TP-receiver + WW plugin + the user's own data. So the build
# exclusion is not asking "does this TU contain donor code?" -- the covenant
# question, answered by lineage and owned by Tier-1. It is asking **"does this
# TU move to the plugin?"** A WW audio bridge moves to the plugin whether or not
# it contains a single donor line.
#
# Lineage cannot answer that: `host-plumbing` means "our code, no donor
# content", which is true of a WW audio bridge AND of an ALBW dialogue box. The
# layer a TU SERVES is a separate axis, and it is not derivable from lineage or
# from the filename -- which is why this is a reviewed table with reasons rather
# than another pattern match.
#
# DEFAULT IS KEEP. An unclassified host-plumbing TU stays in the build. If that
# is wrong the build FAILS TO LINK, which is loud; the opposite default would
# silently drop receiver functionality. Failing toward noise beats failing
# toward silence.
# ============================================================================
WW_SERVING_PLUMBING = {
    "src/d/d_ext_npc_doors.cpp": "WW door/knob binding — merged from WW event data",
    "src/d/d_ext_npc_mount.cpp": "WW mount stand-in + its ModelData cache",
    "src/d/d_ext_npc_population.cpp": "WW chunk -> population placement",
    # ABSENT FROM DUSKLIGHT MAIN entirely (no file, not in its files.cmake),
    # and every user of dNpc_c is a WW actor already excluded. Its s32 angle
    # helper is a WW function: dusklight main's c_lib.cpp IS decompiled and
    # carries only the s16 variants.
    "src/d/d_npc.cpp": "WW NPC framework; nothing dusklight-owned uses it",
    "src/d/d_ext_room_verify.cpp": "WW room-load verification",
    "src/d/d_ww_itemmdl_pc.cpp": "WW item models",
    # NOTE: ww_itemmdl_dispatch.cpp is deliberately NOT here. It is the
    # receiver-side standalone layer -- excluding it would defeat its
    # entire purpose, which is letting the host run with no WW layer.

    "src/d/d_ww_itemmdl_test.cpp": "WW item-model harness",
    "src/d/ext_evt/evt1_boundary.cpp": "boundary INTO the evt1 event stack",
    "src/d/ext_plugin/ww_import_gate.cpp": "the WW plugin's own load gate",
    # Moves to the plugin WITH the profiles it owns. Excluding it makes
    # `dWwProfileRegister_lookup` show up unresolved, and that is CORRECT and
    # informative rather than a regression: it is precisely the call that
    # becomes a hook on `fpcPf_Get` once the layer is a module.
    "src/d/ext_plugin/ww_profile_register.cpp": "WW profile ownership shim",
    # §634: WAS unreviewed-plumbing (KEPT), and that was wrong. Its lineage is
    # WW end to end -- the seam AND every Phase-2 chunk translator behind it --
    # so it moves with the layer. Keeping it forced the opposite error: a
    # translator needing a WW-impl function became a LEG, and the only way to
    # link was to stub the WW symbol, which hides legs rather than removing
    # them. Excluding the whole file leaves the receiver ONE boundary symbol,
    # `dExtWwRoom_loadRoomDzr`, whose default is literally the call
    # d_s_room.cpp made before the seam existed. Same shape as
    # ww_profile_register.cpp above.
    "src/d/ext_plugin/ww_room_loader.cpp": "WW room-load seam + its chunk translators",
    "src/d/ext_seq/ja1_bank.cpp": "JAudio1 bank bridge",
    "src/d/ext_seq/ja1_event_dump.cpp": "JAudio1 diagnostics",
    "src/d/ext_seq/ja1_native.cpp": "JAudio1 native entry points",
    "src/d/ww_jpa_bind.cpp": "binds the WW JPA archive reader",
}

# Host-plumbing that serves ANOTHER layer and must stay. Recorded with the
# file's own words, because both of these say so themselves and a reviewer
# should be able to check the claim without reading the whole TU.
NON_WW_PLUMBING = {
    "src/d/d_albw_dialogue.cpp": 'self-declared "NEW CODE — ALBW Port (Native '
                                 'Dialogue Box)" — a DIFFERENT port, not WW',
    "src/d/d_ext_mod_flags.cpp": 'self-declared "WW-agnostic" twice — general '
                                 'mod flag/quest infrastructure',
}


def declared_lineage(path: str) -> tuple[str | None, str | None]:
    """(KIT-LINEAGE, KIT-DONOR) as declared by the TU itself, or (None, None).

    Reads the WHOLE file: banners sit behind long headers (lines 38-50 in
    several TUs), and a head-only reader reported 5 files as unbannered in §573
    when all of them carry banners.
    """
    p = REPO / path
    if not p.is_file():
        return None, None
    t = p.read_text(encoding="utf-8-sig", errors="replace")
    m = re.search(r"KIT-LINEAGE:\s*([a-z\- ]+)", t)
    d = re.search(r"KIT-DONOR:\s*(\S+)", t)
    return (m.group(1).strip() if m else None), (d.group(1) if d else None)


def lineage_partition() -> dict[str, list[str]]:
    """Partition every BUILT source by declared lineage.

    Scans all build sources, not just filename matches -- that is the whole
    point: d_stage.cpp declares donor lineage and no filename rule sees it.
    """
    out: dict[str, list[str]] = {}
    for src in build_sources():
        lin, _donor = declared_lineage(src)
        out.setdefault(lin or "NO BANNER", []).append(src)
    return out


def excluded_hits() -> list[tuple[str, str]]:
    """Excluded files that WOULD have matched — shown so they stay reviewable."""
    out = []
    for path, why in EXCLUSIONS.items():
        if any(re.search(pat, path) for pat, _ in WW_RULES):
            out.append((path, why))
    return out


def _tier1_section() -> str:
    """ONLY the Tier-1 list, not the whole document.

    First version of this searched the entire file and reported four TUs as
    'listed' purely because the STALE banner's prose names them as examples --
    the instrument was reading its own commentary as data. Same defect family as
    the probe errors of §467/§473: a checker whose scope is wider than its claim.
    """
    if not STRIP_SET.exists():
        return ""
    doc = STRIP_SET.read_text(encoding="utf-8", errors="replace")
    start = doc.find("## Tier 1")
    if start < 0:
        return ""
    end = doc.find("## Tier 2", start)
    return doc[start:end if end > 0 else len(doc)]


def tier1_listed(path: str) -> bool:
    """Is this file's stem named inside the Tier-1 SECTION?"""
    return Path(path).stem in _tier1_section()


def ww_headers() -> list[tuple[str, str]]:
    """WW-owned headers, by the same conventions.

    WEAKER BASIS, stated as such: headers are not listed in files.cmake, so this
    is a DIRECTORY SCAN, not build-derived. A header that no TU includes still
    appears here. Sources are authoritative; headers are indicative.
    """
    out: list[tuple[str, str]] = []
    inc = REPO / "include"
    for p in sorted(inc.rglob("*.h")) + sorted(inc.rglob("*.hpp")):
        rel = p.relative_to(REPO).as_posix()
        probe = rel.replace(".h", ".cpp").replace(".hpp", ".cpp")
        for pattern, reason in WW_RULES:
            if re.search(pattern, probe):
                out.append((rel, reason))
                break
    return out


def emit_tier1(ww: list[tuple[str, str]]) -> str:
    """The Tier-1 block, GENERATED. Spliced between markers in the strip set."""
    hdrs = ww_headers()
    # UNION of both bases (§574). Never-push and build-exclusion want OPPOSITE
    # safety directions from the same data, and one roster was serving both:
    # over-inclusion is free here and fatal in the build, omission is
    # unrecoverable here and merely noisy there. So this list is the WIDEST
    # available -- filename matches PLUS anything declaring donor lineage --
    # while cmake/ww_layer_exclude.cmake takes the narrow, wholesale-safe set.
    #
    # This is what closes the real containment gap §573 found: 22 TUs declaring
    # donor lineage (d_stage.cpp, d_demo.cpp, f_op_msg_mng.cpp, d_particle.cpp,
    # d_door.cpp and 17 actors) were on NO never-push list, because no filename
    # rule sees a donor leg inside a receiver-named file.
    part = lineage_partition()
    by_fn = dict(ww)
    donor_declared = {p for lin in (*DONOR_LINEAGES, *LEG_LINEAGES)
                      for p in part.get(lin, [])}
    union = []
    for p in sorted(set(by_fn) | donor_declared):
        lin, donor = declared_lineage(p)
        if p in by_fn and p not in donor_declared:
            why = f"{by_fn[p]} (filename basis; declares `{lin or 'no banner'}`)"
        elif p in by_fn:
            why = f"KIT-LINEAGE `{lin}`, KIT-DONOR `{donor}` — filename agrees"
        else:
            why = (f"KIT-LINEAGE `{lin}` — **filename rules MISS this**; "
                   f"donor content in a receiver-named TU")
        union.append((p, why))
    lines = [
        "<!-- BEGIN GENERATED TIER-1 — do not hand-edit.",
        "     Regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-tier1",
        "     Basis: UNION of filename convention AND declared lineage (KIT-LINEAGE",
        "     banners), user-ruled 2026-08-07 after §573 measured the filename basis",
        "     wrong in BOTH directions. Widest set on purpose: for a never-push list",
        "     over-inclusion costs nothing and omission cannot be recalled.",
        "     SOURCES from files.cmake (authoritative); HEADERS from a directory scan",
        "     (indicative — a header no TU includes still appears).",
        "     NOT sufficient on its own: a `mixed` TU is donor lines inside a",
        "     receiver-owned file, so listing it here does not make the file",
        "     strippable — see the leg list in cmake/ww_layer_exclude.cmake. -->",
        "",
        f"**Sources ({len(union)}) — generated from `files.cmake`:**",
        "",
    ]
    for path, reason in union:
        lines.append(f"- `{path}` — {reason}")
    lines += ["", f"**Includes ({len(hdrs)}) — directory scan, indicative:**", ""]
    for path, reason in hdrs:
        lines.append(f"- `{path}` — {reason}")
    lines += ["", "<!-- END GENERATED TIER-1 -->"]
    return "\n".join(lines)


def splice_tier1(block: str) -> bool:
    """Replace the generated region in the strip set, preserving human prose."""
    doc = STRIP_SET.read_text(encoding="utf-8", errors="replace")
    begin, end = "<!-- BEGIN GENERATED TIER-1", "<!-- END GENERATED TIER-1 -->"
    if begin in doc and end in doc:
        head = doc[: doc.index(begin)]
        tail = doc[doc.index(end) + len(end):]
        STRIP_SET.write_text(head + block + tail, encoding="utf-8")
        return True
    # first splice: replace the hand list, keep the "Why each cluster" prose on
    anchor = "**Why each cluster is covenant surface:**"
    start = doc.find("**Includes (")
    if start < 0 or anchor not in doc:
        return False
    STRIP_SET.write_text(doc[:start] + block + "\n\n" + doc[doc.index(anchor):],
                         encoding="utf-8")
    return True


def emit_cmake(ww) -> int:
    """Step 11: generate the build-exclusion list CMake consumes.

    GENERATED, never curated. §488's whole point and the R5 lesson: a hand-kept
    exclusion list drifts from the tree the moment someone adds a TU, and a
    drifted exclusion list is worse than none because the build reports success
    while compiling in the thing it was meant to exclude.

    Filename-convention basis, stated in the artifact rather than assumed:
    this list is a FLOOR. It cannot see a donor-derived TU that is correctly
    named after the receiver file it replaces, and it cannot see a LEG at all —
    donor lines inside a receiver-owned file are not excludable by dropping the
    file. Step 11 is "partial by definition until 19" for exactly that reason,
    and the header says so where a reader will hit it.
    """
    import hashlib
    out = REPO / "cmake" / "ww_layer_exclude.cmake"
    out.parent.mkdir(parents=True, exist_ok=True)
    tool = Path(__file__).read_bytes()
    fc = (REPO / "files.cmake").read_bytes()
    body = []
    body.append("# WW-layer build exclusion — GENERATED, do not hand-edit.")
    body.append("# regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-cmake")
    body.append(f"# tool_sha256:      {hashlib.sha256(tool).hexdigest()}")
    body.append(f"# files_cmake_sha256: {hashlib.sha256(fc).hexdigest()}")
    part = lineage_partition()
    donor = {p for lin in DONOR_LINEAGES for p in part.get(lin, [])}
    plumbing = set(part.get("host-plumbing", []))
    # §576: exclude what MOVES TO THE PLUGIN = donor code + the plumbing that
    # serves the WW layer. Unclassified plumbing is KEPT and reported.
    ww_plumb = plumbing & set(WW_SERVING_PLUMBING)
    unreviewed = sorted(plumbing - set(WW_SERVING_PLUMBING) - set(NON_WW_PLUMBING))
    excl = sorted(donor | ww_plumb)
    legs = sorted({p for lin in LEG_LINEAGES for p in part.get(lin, [])})
    kept = sorted(plumbing & set(NON_WW_PLUMBING))
    fn_only = sorted({p for p, _ in ww} - set(excl) - set(legs))
    body.append(f"# count: {len(excl)}")
    body.append("#")
    body.append("# BASIS: WHAT MOVES TO THE PLUGIN (§576) = declared donor lineage")
    body.append("# (native-port + bridge-owed) PLUS host-plumbing that serves the WW")
    body.append("# layer. Step 20 ruled the end state is a prebuilt plugin, so the")
    body.append("# question here is NOT 'does this contain donor code' -- that is the")
    body.append("# covenant question, owned by Tier-1 -- but 'does this move out of")
    body.append("# dusklight.exe'. A WW audio bridge moves whether or not it contains")
    body.append("# a single donor line. Lineage cannot answer that: `host-plumbing`")
    body.append("# describes an ALBW dialogue box and a WW audio bridge alike.")
    body.append("#")
    body.append("# The earlier FILENAME basis was wrong in both directions (§573);")
    body.append("# lineage-only then left the bridges in while removing what they call,")
    body.append("# which is where 30 of the 61 remaining unresolved symbols came from.")
    if kept:
        body.append("#")
        body.append(f"# KEPT — host-plumbing serving ANOTHER layer ({len(kept)}):")
        for p in kept:
            body.append(f"#   {p}  --  {NON_WW_PLUMBING[p]}")
    if unreviewed:
        body.append("#")
        body.append(f"# UNREVIEWED host-plumbing ({len(unreviewed)}) — KEPT by default.")
        body.append("# If one of these is a WW bridge the build FAILS TO LINK, which is")
        body.append("# loud; the opposite default drops receiver code silently.")
        for p in unreviewed:
            body.append(f"#   {p}")
    body.append("#")
    body.append("# STILL PARTIAL, and for a reason no basis can fix: `mixed` TUs are")
    body.append("# donor lines INSIDE receiver-owned files. Dropping them would remove")
    body.append("# the receiver, so they are listed below as LEG-STRIP REQUIRED and")
    body.append("# left in the build. Excluding this set does NOT make the build")
    body.append("# WW-free -- it removes the separable stacks. Step 19 owns the rest.")
    body.append("#")
    body.append(f"# LEG-STRIP REQUIRED ({len(legs)} mixed TUs, NOT excluded here):")
    for p in legs:
        body.append(f"#   {p}")
    if fn_only:
        body.append("#")
        body.append(f"# Matched the OLD filename rules but declare no donor content ({len(fn_only)}),")
        body.append("# so they are no longer excluded. Listed so the change is auditable:")
        for p in fn_only:
            body.append(f"#   {p}")
    body.append("set(WW_LAYER_FILES")
    for path in excl:
        body.append(f"    {path}")
    body.append(")")
    out.write_text("\n".join(body) + "\n", encoding="utf-8")
    print(f"emitted {out.relative_to(REPO)} — {len(excl)} sources "
          f"(plugin-split basis: {len(donor)} donor + {len(ww_plumb)} WW-serving plumbing)")
    print(f"  LEG-STRIP REQUIRED: {len(legs)} mixed TUs left in the build (step 19)")
    print(f"  KEPT, serves another layer: {len(kept)}   UNREVIEWED plumbing: {len(unreviewed)}")
    print(f"  dropped from the old filename list: {len(fn_only)}")
    print("  NOTE: still a FLOOR. Legs are not excludable by file; see the header.")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--json", metavar="OUT")
    ap.add_argument("--check", action="store_true")
    ap.add_argument("--emit-cmake", action="store_true",
                    help="generate cmake/ww_layer_exclude.cmake for step 11")
    ap.add_argument("--emit-tier1", action="store_true",
                    help="regenerate the Tier-1 block inside NEVER-PUSH-STRIP-SET.md")
    args = ap.parse_args()

    sources = build_sources()
    ww = [(p, r) for p in sources if (r := classify(p))]

    if args.emit_cmake:
        return emit_cmake(ww)

    if args.emit_tier1:
        block = emit_tier1(ww)
        if splice_tier1(block):
            # Count what was WRITTEN, not what was passed in: this printed
            # len(ww)=48 while writing a 70-entry union, which is a tool
            # misreporting its own output -- the exact defect this campaign
            # keeps finding in other instruments.
            n_src = block.count("\n- `src/") + block.count("\n- `libs/")
            part = lineage_partition()
            missed = len([p for lin in (*DONOR_LINEAGES, *LEG_LINEAGES)
                          for p in part.get(lin, []) if p not in dict(ww)])
            print(f"Tier-1 regenerated: {n_src} sources (UNION basis), "
                  f"{len(ww_headers())} headers -> {STRIP_SET.relative_to(REPO)}")
            print(f"  of those, {missed} declare donor lineage that the filename "
                  f"rules MISS -- previously on no never-push list")
            return 0
        print("ERROR: could not locate the Tier-1 region to splice.")
        return 2

    missing = [p for p, _ in ww if not tier1_listed(p)]

    print("WW-LAYER MANIFEST — generated from files.cmake")
    print(f"  basis: filename-convention (NOT lineage; see step 10)")
    print(f"  build sources: {len(sources)}   WW-owned: {len(ww)}   "
          f"absent from Tier-1: {len(missing)}")
    print()
    for path, reason in ww:
        mark = " " if tier1_listed(path) else "!"
        print(f"  {mark} {path:<48} {reason}")
    if missing:
        print()
        print(f"  '!' = compiled into the exe but NOT named in "
              f"NEVER-PUSH-STRIP-SET.md Tier-1 ({len(missing)} files)")

    ex = excluded_hits()
    if ex:
        print()
        print("  EXCLUDED (name matches a WW rule, lineage is the receiver's):")
        for path, why in ex:
            print(f"    - {path}\n        {why}")

    if args.json:
        # PROVENANCE (Foundry's finding, accepted): without this an emitted
        # artifact is indistinguishable from a hand snapshot, and an auditor is
        # right to treat it as one. Hashes make it REPRODUCIBLE rather than
        # merely timestamped — re-run the tool against the same files.cmake and
        # you must get the same manifest, which is what makes it evidence.
        tool = Path(__file__).resolve()
        Path(args.json).write_text(
            json.dumps(
                {
                    "generated_by": tool.relative_to(REPO).as_posix(),
                    "regenerate": (
                        "python tools/ww_crew_restoration_skeleton/"
                        "ww_layer_manifest.py --json docs/state/ww-layer-manifest.json"
                    ),
                    "tool_sha256": _sha256(tool),
                    "input": FILES_CMAKE.relative_to(REPO).as_posix(),
                    "input_sha256": _sha256(FILES_CMAKE),
                    "basis": "filename-convention (NOT lineage)",
                    "basis_note": (
                        "Sources are build-derived (authoritative). This answers "
                        "'what is WW-SHAPED', not 'what is WW-DERIVED' — roadmap "
                        "step 10's provenance banners upgrade it to lineage."
                    ),
                    "build_source_count": len(sources),
                    "ww_layer_count": len(ww),
                    "ww_layer": [{"path": p, "reason": r,
                                  "tier1_listed": tier1_listed(p)} for p, r in ww],
                },
                indent=2,
            ),
            encoding="utf-8",
        )
        print(f"\n  wrote {args.json}")

    if args.check and missing:
        print(f"\nCHECK FAILED: {len(missing)} WW-owned TU(s) not in Tier-1.")
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
