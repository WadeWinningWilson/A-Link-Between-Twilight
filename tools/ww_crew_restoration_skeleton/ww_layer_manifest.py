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
    lines = [
        "<!-- BEGIN GENERATED TIER-1 — do not hand-edit.",
        "     Regenerate: python tools/ww_crew_restoration_skeleton/ww_layer_manifest.py --emit-tier1",
        "     Basis: SOURCES from files.cmake (authoritative, the build is the only",
        "     authority); HEADERS from a directory scan (indicative only). Both",
        "     classify by filename convention, NOT by declared lineage — roadmap",
        "     step 10's provenance banners upgrade that. -->",
        "",
        f"**Sources ({len(ww)}) — generated from `files.cmake`:**",
        "",
    ]
    for path, reason in ww:
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
    body.append(f"# count: {len(ww)}")
    body.append("#")
    body.append("# BASIS: filename convention. This is a FLOOR, not the WW layer.")
    body.append("# It cannot see a donor port correctly named after the receiver file")
    body.append("# it replaces, and it cannot see a LEG (donor lines inside a")
    body.append("# receiver-owned TU) at all -- those are not excludable by dropping a")
    body.append("# file. Excluding these does NOT make the build WW-free; it removes")
    body.append("# the separable stacks. Partial by definition until roadmap step 19.")
    body.append("set(WW_LAYER_FILES")
    for path, _reason in sorted(ww):
        body.append(f"    {path}")
    body.append(")")
    out.write_text("\n".join(body) + "\n", encoding="utf-8")
    print(f"emitted {out.relative_to(REPO)} — {len(ww)} sources")
    print("  NOTE: a FLOOR. Legs are not excludable by file; see the header.")
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
        if splice_tier1(emit_tier1(ww)):
            print(f"Tier-1 regenerated: {len(ww)} sources, "
                  f"{len(ww_headers())} headers -> {STRIP_SET.relative_to(REPO)}")
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
