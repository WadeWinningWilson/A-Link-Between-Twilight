#!/usr/bin/env python3
# ============================================================================
# census_axis_d.py — AXIS D, decomp completeness (census spec §3).
#
# MEASURE: per subsystem, how many donor TUs are genuinely decompiled vs
# stubbed/NonMatching, PLUS a stub roster with a reachability note.
#
# WHY A ROSTER AND NOT A PERCENTAGE (spec §3): a 95%-complete subsystem whose
# missing 5% is the one function our content calls is strictly worse than an
# 80%-complete one whose gaps sit in unused paths. The number cannot express
# that; the roster can. Wholesale import silently brings the undecompiled
# remainder in as stubs, and **stubs do not fail at compile time — they fail
# when reached**. That is the dormant-landmine class, the most expensive failure
# mode this campaign has.
#
# ---------------------------------------------------------------------------
# THE VALIDITY SET IS ENUMERATED FROM THE DONOR TREE, NOT FROM MEMORY
#
# Spec §9 requires exactly this, citing the CP-register set that omitted
# 0xB0-0xBF. The obvious implementation -- grep the donor sources for
# `NONMATCHING` -- was measured first and is WRONG: only 8 files in the whole
# donor tree carry that marker, which cannot describe a ~72%-decompiled project.
#
# The authority is the donor's own build description, `configure.py`, which
# declares every object's status:
#     Object(Matching,       "path")   linked; byte-matches
#     Object(NonMatching,    "path")   NOT linked; not decompiled
#     Object(Equivalent,     "path")   linked only with --non-matching
#     Object(MatchingFor(v), "path")   matches on some versions only
#     Object(EquivalentFor(v), "path")
#
# ---------------------------------------------------------------------------
# SHELF LIFE, RECORDED SO NOBODY BANKS THIS
#
# Per the user's 2026-08-06 direction, the project will re-base onto the
# finished decomp. Axis D's content IS "what is stubbed today" -- exactly what
# upstream completion erases. C, W, P and B are structural and survive a
# re-base; D does not. Build it (the instrument must exist), run it when a port
# is imminent, do not bank its output as a standing fact.
#
# ---------------------------------------------------------------------------
# REACHABILITY IS REPORTED UNKNOWN, DELIBERATELY
#
# Spec §3 wants "is the stub on a path our content will actually hit?". Deciding
# that needs a donor-side symbol graph this census does not build, and guessing
# it would put a false ALL-CLEAR in front of the dormant-landmine class. So each
# stub carries reachability: UNKNOWN with the reason, per №31-C.
# ============================================================================
import re
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
import census_axis_c as C  # noqa: E402

DONOR = Path(r"D:\XXXXXXX\WW DP")
# Our build target, from the sanctioned dump identified in §332.
OUR_VERSION = "GZLE01"
# Anchored, not a substring test. `OUR_VERSION in status` was the same shape
# as HT-17 (unanchored src/) and HT-18 (embedded path) -- harmless with
# quoted fixed-width IDs, but that class has bitten this instrument twice,
# so it is closed rather than reasoned about.
VERSION_TOKEN = re.compile(r"[\"']" + OUR_VERSION + r"[\"']")
# D2: the status token may itself contain a parenthesised argument list --
# `MatchingFor("GZLJ01", "GZLE01", "GZLP01")`. The old lazy pattern stopped
# at the first comma INSIDE those parens, so 66 of 678 objects were dropped,
# all of them MatchingFor. Every percentage rested on an authority missing
# ~10% of its objects.
# Actor objects are declared through the ActorRel helper, whose object path
# is an f-string -- invisible to a literal-path parser. 416 of them.
ACTOR_REL = re.compile(r"ActorRel\(\s*([A-Za-z_]\w*(?:\([^)]*\))?)\s*,\s*[\"']([^\"']+)[\"']")
OBJECT = re.compile(r"Object\(\s*([A-Za-z_]\w*(?:\([^)]*\))?)\s*,\s*[\"']([^\"']+)[\"']")


def donor_status():
    """path -> declared status, from the donor's own configure.py."""
    cfg = DONOR / "configure.py"
    if not cfg.is_file():
        return None
    txt = cfg.read_text(encoding="utf-8", errors="replace")
    out = {}
    pairs = [(m.group(1), m.group(2)) for m in OBJECT.finditer(txt)]
    # ActorRel(status, "d_a_name") -> object "d/actor/d_a_name.cpp"
    pairs += [(m.group(1), "d/actor/" + m.group(2) + ".cpp")
              for m in ACTOR_REL.finditer(txt)]
    for status, path in pairs:
        status, path = status.strip(), path.strip()
        # D2 version guard: MatchingFor/EquivalentFor match only on the listed
        # versions. Counting them as matching WITHOUT checking ours was an
        # absent guard, not a violated one -- HousingTemp verified every current
        # entry includes GZLE01. Absent guards become violated ones later.
        if status.startswith("MatchingFor"):
            status = ("MatchingFor" if VERSION_TOKEN.search(status)
                      else "MatchingFor-OTHER-VERSION")
        elif status.startswith("EquivalentFor"):
            status = ("EquivalentFor" if VERSION_TOKEN.search(status)
                      else "EquivalentFor-OTHER-VERSION")
        out[path] = status
    return out


def _donor_candidates(rel, status_map):
    """Donor objects this receiver TU plausibly derives from.

    Two evidence routes, both explicit:
      * BASENAME match against the donor object list (a direct port keeps the
        donor name -- that is a standing project rule);
      * a donor path CITED in the file itself.
    A TU with neither is UNMAPPED, never guessed at."""
    hits, why = [], None
    stem = Path(rel).name
    for dpath in status_map:
        if Path(dpath).name == stem:
            hits.append(dpath)
            why = "basename match"
    if hits:
        return hits, why
    p = C.REPO / rel
    if p.is_file():
        txt = p.read_text(encoding="utf-8", errors="replace")
        for dpath in status_map:
            if Path(dpath).name in txt:
                hits.append(dpath)
                why = "donor path cited in file"
    return hits, why


STUBBY = {"NonMatching", "Equivalent", "EquivalentFor",
          "MatchingFor-OTHER-VERSION", "EquivalentFor-OTHER-VERSION"}


def run(roster):
    status_map = donor_status()
    if status_map is None:
        return {"__error__": "donor configure.py not readable; Axis D is "
                            "UNKNOWN, not clean"}

    subs = {}
    for rel in roster:
        subs.setdefault(C.group_of(rel), []).append(rel)

    out = {}
    for sub, files in sorted(subs.items()):
        mapped, unmapped, stubs, counts = {}, [], [], {}
        for rel in files:
            cands, why = _donor_candidates(rel, status_map)
            if not cands:
                unmapped.append(rel)
                continue
            for d in cands:
                st = status_map[d]
                counts[st] = counts.get(st, 0) + 1
                mapped.setdefault(rel, []).append((d, st, why))
                if st in STUBBY:
                    stubs.append({
                        "receiver_tu": rel, "donor_object": d, "status": st,
                        "evidence": why,
                        "reachability": "UNKNOWN — needs a donor-side symbol "
                                        "graph this census does not build; "
                                        "guessing would put a false all-clear "
                                        "in front of the dormant-stub class",
                    })
        total = sum(counts.values())
        matching = counts.get("Matching", 0) + counts.get("MatchingFor", 0)
        # D3: a percentage computed over MAPPED TUs only, printed without its
        # denominator, reads as a statement about the subsystem. "JAudio1 90%"
        # meant 90% of the 20 that mapped, of 22; "WW engine legs 0.0%" was ONE
        # mapped TU that happens to be a stub, which reads catastrophic when the
        # subsystem is mostly UNMEASURED. Coverage now travels with every figure
        # and an unmapped TU is a reported fact, not an absence.
        coverage = round(100.0 * len(mapped) / len(files), 1) if files else None
        out[sub] = {
            "files": files,
            "mapped_tus": len(mapped),
            "unmapped_count": len(unmapped),
            "unmapped_tus": unmapped,
            "coverage_pct": coverage,
            "status_counts": counts,
            "decomp_pct": round(100.0 * matching / total, 1) if total else None,
            "decomp_pct_basis": (f"{matching}/{total} donor objects, over "
                                 f"{len(mapped)}/{len(files)} TUs mapped"),
            "stubs": stubs,
        }
    return out
