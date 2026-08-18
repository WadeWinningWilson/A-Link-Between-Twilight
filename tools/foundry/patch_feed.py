#!/usr/bin/env python
# ============================================================================
# patch_feed.py - THE PRELIMINARY PATCHER'S FEED, and its completeness oracle.
#
# WHAT THIS IS (user-ordered 2026-08-17, "start it").
#
# The patcher needs to know WHICH receiver-side changes to carry to a clean
# dusklight. Three artifacts already answer parts of that and none answers it
# alone:
#
#   fork-vs-origin diff  1577 source files  - the RAW MATERIAL (mostly not WW:
#                                             ALBW, HUD, outfits, build)
#   ww-ownership-map     165 files          - the SELECTOR (which are WW-owned,
#                                             adjudicated 162/162, replaces the
#                                             four older partitions)
#   seam tracker         122 rows           - the SEAM DETAIL (symbols, doorway,
#                                             destination) inside those files
#
# THE DIRECTION MATTERS AND IT IS THE EASY THING TO GET BACKWARDS. The diff is
# not a filter applied to the tracker - it is the candidate pool the MAP selects
# from. Filtering a 122-row set against a 1577-file set finds "everything
# present" and tells you nothing.
#
# ============================ THE ORACLE ====================================
# The redundancy pass is the cheap half. Run the same comparison BACKWARDS and
# it becomes the check the whole plan rests on:
#
#   BLIND SPOT = a file that is WW-relevant (filename convention OR a declared
#                KIT-DONOR banner) and CHANGED vs origin, but carries NO
#                ownership-map entry.
#
# Those are the changes a patch would silently omit. The proposal already
# records what that costs: "a patch compiled from an incomplete set boots a
# black screen, and we already know exactly what that looks like (the Outset
# warp: scene torn down, stage never registered, zero errors)."
#
# A patch that is merely REDUNDANT wastes bytes. A patch that is INCOMPLETE
# boots black with no error. Only one of those is worth an instrument.
#
# ---------------------------------------------------------------------------
# NEGATIVE CONTROL (--control). This tool's whole output on a healthy tree is
# "no blind spots", which is indistinguishable from a tool that cannot detect
# blind spots at all. `--control` plants a synthetic WW-named changed file with
# no map entry and REQUIRES the detector to flag it; the run fails if it does
# not. Foundry filed a false gate finding earlier today for exactly the missing
# half of this discipline - a check that can only produce silence must be
# proven able to produce noise before its silence is read as a result.
# ============================================================================
import argparse
import json
import re
import subprocess
import sys
from pathlib import Path

sys.stdout.reconfigure(encoding="utf-8", errors="replace")

ROOT = Path(__file__).resolve().parents[2]
MAP = ROOT / "docs/state/ww-staging/ww-ownership-map.json"
ROWS = ROOT / "docs/state/ww-staging/tracker/rows"

# Categories the patcher CARRIES. Taken from the map's own adjudicated verdicts
# (162/162, 2026-08-16) - never re-derived here. `albw*`, `instrument`,
# `not-ww` and `stale-path` are excluded BY RULING, not by this tool's opinion.
CARRY = {"ww-port", "shared-per-hunk", "fork-host-plumbing"}

# WW-relevance by filename - the SAME rules the never-push manifest uses, kept
# textually identical so the two instruments cannot silently disagree.
WW_NAME = re.compile(
    r"(^|/)(ja1_|evt1_|mdoext1_|d_a_ext_|d_ext_|d_ww_|ww_|d_a_ww_)[^/]*\.(cpp|h)$"
    r"|(^|/)[^/]*_ww(_|\.)[^/]*"
)


def sh(*args):
    return subprocess.run(args, cwd=ROOT, capture_output=True,
                          text=True, encoding="utf-8", errors="replace").stdout


def baseline(pin):
    """The origin commit the user's clean dusklight is built from.

    Defaults to merge-base, which isolates OUR changes only. A moving ref is
    accepted but warned about: a patch pinned to `origin/main` silently changes
    meaning when origin moves, and the test build it must apply to does not.
    """
    if pin:
        return pin.strip()
    return sh("git", "merge-base", "HEAD", "origin/main").strip()


def changed(base):
    out = sh("git", "diff", "--name-only", base, "HEAD", "--", "src/", "include/")
    return {l.strip() for l in out.splitlines() if l.strip()}


def has_donor_banner(rel):
    p = ROOT / rel
    if not p.is_file():
        return False
    try:
        head = p.read_text(encoding="utf-8-sig", errors="replace")[:4000]
    except Exception:
        return False
    m = re.search(r"KIT-DONOR:\s*(\S+)", head)
    return bool(m and m.group(1).lower() not in ("none", "n/a"))


def tracker_tus():
    """Every TU named by a seam row, so seam coverage is measured not assumed."""
    tus = set()
    for r in ROWS.glob("*.md"):
        for m in re.finditer(r"^tu:\s*(\S+)", r.read_text(encoding="utf-8",
                                                          errors="replace"), re.M):
            tus.add(m.group(1))
    return tus


INC = re.compile(r'#include\s+"([^"]+\.inc)"')


def inc_closure(carried):
    """CARRY is TU-scoped; the build is not.

    A carried .cpp with `#include "ww_item_data.inc"` does not compile without
    that .inc, and the ownership map has ZERO .inc entries (0 of 165) while 31
    .inc files differ from origin. Shipping map entries alone therefore emits a
    patch that cannot build - and this is NOT a map defect: .inc is simply
    outside its TU-scoped domain. The closure belongs here, at the feed, where
    the build's own dependency is the authority.
    """
    need = {}
    for rel in carried:
        p = ROOT / rel
        if p.suffix != ".cpp" or not p.is_file():
            continue
        for m in INC.finditer(p.read_text(encoding="utf-8", errors="replace")):
            need.setdefault(m.group(1), []).append(rel)
    return need


def manifest_exclusions():
    """The never-push manifest's evidence-backed non-WW list, read live.

    Kept as a live read rather than a copy: two instruments holding two copies
    of one list is how they drift into disagreeing, which is the very defect
    this feed exists to surface.
    """
    try:
        import importlib.util
        spec = importlib.util.spec_from_file_location(
            "wlm", ROOT / "tools/ww_crew_restoration_skeleton/ww_layer_manifest.py")
        m = importlib.util.module_from_spec(spec)
        spec.loader.exec_module(m)
        return dict(m.EXCLUSIONS)
    except Exception:
        return {}


def run(base, extra_changed=frozenset()):
    doc = json.loads(MAP.read_text(encoding="utf-8"))
    files = doc["files"]
    diff = changed(base) | set(extra_changed)
    seams = tracker_tus()

    carry, wrong_cat, stale, blind, noseam = [], [], [], [], []

    for rel, e in files.items():
        cat = e.get("category", "")
        if rel not in diff:
            stale.append((rel, cat))
            continue
        if cat in CARRY:
            carry.append((rel, cat, e.get("route", "?")))
            if rel.endswith(".cpp") and rel not in seams:
                noseam.append(rel)
        else:
            wrong_cat.append((rel, cat))

    incs = inc_closure([f for f, _c, _r in carry])

    # THE ORACLE: WW-relevant AND changed AND unmapped.
    #
    # .inc is EXCLUDED from blind-spot scoring on purpose. It is a CATEGORY the
    # map does not cover (0 of 165 entries), not an omission in it - and the
    # difference matters: scoring a whole uncovered category as 31 blind spots
    # would bury the real ones in noise. .inc is handled by inc_closure above.
    # (Same discipline that kept d_a_demo00 honest: check whether a gap is a
    # category before filing it as an omission.)
    excl = manifest_exclusions()
    for rel in sorted(diff):
        if rel in files or rel.endswith(".inc"):
            continue
        # A file the never-push manifest EXCLUDES carries POSITIVE EVIDENCE of
        # receiver lineage (d_a_e_ww.cpp: fpcNm_E_WW_e, f_op_actor_enemy.h,
        # Z2AudioLib). WW_NAME is filename-only and cannot see that. Scoring it
        # blind would demand a map entry for a file two instruments have now
        # agreed is TP - Housing Security ruled exactly this on 2026-08-17.
        if rel in excl:
            continue
        if WW_NAME.search(rel) or has_donor_banner(rel):
            blind.append(rel)

    # CROSS-INSTRUMENT CONFLICT - a different failure from a blind spot, and it
    # must not be reported as one. The never-push manifest EXCLUDES a file with
    # positive evidence while the ownership map CARRIES it (or its header). Two
    # adjudicated instruments disagreeing about one file means at least one
    # verdict is wrong, and neither tool can tell which on its own.
    conflict = []
    for ex, why in manifest_exclusions().items():
        stem = re.sub(r"\.(cpp|h)$", "", ex).split("/")[-1]
        for rel, e in files.items():
            if not re.sub(r"\.(cpp|h)$", "", rel).endswith(stem):
                continue
            # ONLY a CARRIED category is a conflict. Once the map agrees the
            # file is not-ww, the two instruments CONCUR and reporting that as
            # a conflict is the instrument failing to notice it was answered -
            # exactly the stale-artifact failure the 2026-08-17 stand-down row
            # names as its first lesson.
            if e.get("category") in CARRY:
                conflict.append((ex, rel, e.get("verdict", "?"), why))

    return dict(base=base, diff=diff, carry=carry, wrong_cat=wrong_cat,
                stale=stale, blind=blind, noseam=noseam, incs=incs,
                conflict=conflict)


def report(r, quiet=False):
    if not quiet:
        print("PATCH FEED - reconciliation against %s" % r["base"][:12])
        print("  changed vs origin (src/include) : %d" % len(r["diff"]))
        print("  ownership map CARRY -> patch    : %d" % len(r["carry"]))
        print("  map entries excluded by ruling  : %d  (albw / instrument / not-ww)"
              % len(r["wrong_cat"]))
        print("  map entries NOT in the diff     : %d  <- stale map rows"
              % len(r["stale"]))
        print("  CARRY .cpp with no seam row     : %d  (informational)"
              % len(r["noseam"]))
        print("  .inc pulled in by CARRIED TUs   : %d  <- MUST ship with the patch"
              % len(r["incs"]))
        for rel, cat in r["stale"]:
            print("      STALE: %s  [%s]" % (rel, cat))
        for inc, by in sorted(r["incs"].items()):
            print("      INC:   %s  (needed by %s)" % (inc, by[0]))
        if r["conflict"]:
            print()
            print("  *** CROSS-INSTRUMENT CONFLICT : %d ***" % len(r["conflict"]))
            for ex, rel, verdict, why in r["conflict"]:
                print("      manifest EXCLUDES %s" % ex)
                print("      but the map CARRIES %s as %s" % (rel, verdict))
                print("      manifest's evidence: %s" % why[:120])
        print()
    print("  *** BLIND SPOTS (WW-relevant, changed, UNMAPPED) : %d ***"
          % len(r["blind"]))
    for rel in r["blind"]:
        print("      BLIND: %s" % rel)
    if not r["blind"]:
        print("      none - every WW-relevant changed file is adjudicated.")
    return 1 if r["blind"] else 0


def control(base):
    """Prove the detector can go RED before its GREEN is believed."""
    planted = "src/d/d_ww_control_specimen.cpp"
    print("NEGATIVE CONTROL - planting a WW-named changed file with no map entry")
    print("  specimen: %s" % planted)
    r = run(base, extra_changed={planted})
    if planted not in r["blind"]:
        print("  *** CONTROL FAILED *** the detector did NOT flag the specimen.")
        print("  Its silence on a real tree therefore means nothing. NOT USABLE.")
        return 1
    print("  CONTROL PASSED - specimen flagged; the detector produces noise,")
    print("  so its silence on the real tree is a result and not an artifact.")
    return 0


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("--base", help="pinned origin commit (default: merge-base "
                                   "HEAD origin/main). PIN IT for a real patch.")
    ap.add_argument("--control", action="store_true",
                    help="run the negative control and exit")
    ap.add_argument("--emit", metavar="PATH",
                    help="write the CARRY set (the patcher's input) as JSON")
    a = ap.parse_args()

    base = baseline(a.base)
    if not base:
        print("no baseline: is origin/main fetched?")
        return 2
    if not a.base:
        print("NOTE: baseline derived from a MOVING ref (origin/main). Pin an "
              "explicit commit with --base for any patch that will be TESTED, "
              "or the patch and the build it applies to can drift apart.\n")

    if a.control:
        return control(base)

    r = run(base)
    rc = report(r)
    if a.emit:
        # The emitted set is the PATCHER'S INPUT, so it must be buildable, not
        # merely adjudicated: map entries alone omit the .inc their own TUs
        # include. Closure members are tagged so the patcher can tell an
        # adjudicated carry from a build-forced one.
        payload = {
            "baseline": base,
            "baseline_is_pinned": bool(a.base),
            "carry": [{"file": f, "category": c, "route": ro, "why": "ownership-map"}
                      for f, c, ro in r["carry"]]
            + [{"file": i, "category": "inc-closure", "route": "BUILD",
                "why": "included by " + by[0]} for i, by in sorted(r["incs"].items())],
            "unresolved": {"blind_spots": r["blind"],
                           "cross_instrument_conflict":
                               [list(c) for c in r["conflict"]],
                           "stale_map_rows": [f for f, _ in r["stale"]]},
        }
        Path(a.emit).write_text(json.dumps(payload, indent=1), encoding="utf-8")
        print("\n  CARRY set written -> %s" % a.emit)
        print("     %d adjudicated + %d inc-closure = %d files"
              % (len(r["carry"]), len(r["incs"]), len(r["carry"]) + len(r["incs"])))
        if r["blind"] or r["conflict"]:
            print("     UNRESOLVED items are recorded IN the file - a patcher")
            print("     reading it cannot mistake this for a clean set.")
    return rc


if __name__ == "__main__":
    sys.exit(main())
