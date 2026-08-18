#!/usr/bin/env python3
# ============================================================================
# row_doorway.py - B4: ROW ACCURACY, NOT JUST ROW EXISTENCE.
#
# Re-runs the doorway test for every ACTIVE tracker row: does each declared
# `doorway:` class still match what the two images actually say about the
# row's `symbols:`? The amendment's case for this is exact: "a row whose
# EXISTS / GATED / ABSENT classification silently changed underneath it" -
# an origin update moves a symbol, the row stays green, and the first sign
# is a runtime absence months later.
#
# BUDGET RULE (roadmap B4, verbatim): one manifest load PER IMAGE, never N
# tool invocations. Both manifests load once; every row is N dict lookups.
#
# THE MEASURABLE INVARIANT PER CLASS - stated so the verdicts mean one thing:
#   EXISTS            every symbol resolves on VANILLA (>=1; MULTI flagged -
#                     bind mangled, the setStageRes lesson)
#   GATED             symbols ABSENT from vanilla's manifest (compiled out);
#                     manifest alone cannot tell GATED from ABSENT - that
#                     distinction is the row author's, only ABSENCE is checked
#   ABSENT-hookable / ABSENT-unhookable
#                     symbols ABSENT from vanilla
#   (every class)     symbols must resolve on OUR FORK - a row citing code
#                     the fork no longer has is STALE, not green
#
# Verdicts: OK · STALE (measurement contradicts the declaration - requires
# re-affirmation, per the amendment) · UNKNOWN (an image is unavailable -
# reported, never assumed either way, No.31-C).
#
# Usage: row_doorway.py [--rows <dir>] [--selftest]
# Exit 0 = all OK · 1 = any STALE · 2 = UNKNOWN / cannot run.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
ROWS = REPO / "docs" / "state" / "ww-staging" / "tracker" / "rows"
OWN_EXE = REPO / "build" / "windows-msvc-relwithdebinfo" / "dusklight.exe"
UP_EXE = Path("C:/Users/xxxxx/Documents/dusklight-main/build/"
              "windows-msvc-relwithdebinfo/dusklight.exe")

# match broadly, validate against the domain (§P3) - a novel doorway value
# surfaces as STALE-UNKNOWN-CLASS, never as a silent skip
DOORWAY_DOMAIN = {"EXISTS", "GATED", "ABSENT-hookable", "ABSENT-unhookable"}
RE_FIELD = re.compile(r"^(id|symbols|doorway|retired):\s*(.+?)\s*$", re.M)


def load_rows(rows_dir):
    out = []
    for p in sorted(Path(rows_dir).glob("*.md")):
        f = dict(RE_FIELD.findall(p.read_text(encoding="utf-8-sig",
                                              errors="replace")))
        if f.get("retired"):
            continue                    # RETIRED rows are history, not claims
        syms = [s.strip() for s in f.get("symbols", "").split(",") if s.strip()]
        out.append((p.name, f.get("id", "?"), f.get("doorway", "?"), syms))
    return out


def manifest(img):
    """{name: count} or None (unavailable - UNKNOWN, not empty)."""
    if not Path(img).is_file():
        return None
    sys.path.insert(0, str(HERE))
    import symbol_manifest as SM
    occ = SM.occurrences(str(img))
    if not occ:
        return None
    counts = {}
    for name in occ:
        counts[name] = counts.get(name, 0) + 1
    return counts


def check_rows(rows, own, up):
    stale = unknown = 0
    for fname, rid, doorway, syms in rows:
        problems = []
        if doorway not in DOORWAY_DOMAIN:
            problems.append("doorway %r not in declared domain" % doorway)
        for s in syms:
            n_own = None if own is None else sum(
                c for k, c in own.items() if k == s or k.endswith("::" + s))
            n_up = None if up is None else sum(
                c for k, c in up.items() if k == s or k.endswith("::" + s))
            if n_own is None or n_up is None:
                unknown += 1
                problems.append("%s: an image is UNAVAILABLE - UNKNOWN" % s)
                continue
            if n_own == 0:
                problems.append("%s: gone from OUR FORK - the row cites code "
                                "the tree no longer has" % s)
            if doorway == "EXISTS":
                if n_up == 0:
                    problems.append("%s: declared EXISTS but MISSING on "
                                    "vanilla - class flipped under the row" % s)
                elif n_up > 1:
                    problems.append("%s: EXISTS but MULTI (%d) on vanilla - "
                                    "bind mangled (setStageRes lesson)"
                                    % (s, n_up))
            elif doorway in DOORWAY_DOMAIN and n_up > 0:
                problems.append("%s: declared %s but PRESENT on vanilla (%d) "
                                "- class flipped under the row"
                                % (s, doorway, n_up))
        if problems:
            stale += 1
            print("  [STALE] %-14s %s" % (rid, fname))
            for pr in problems:
                print("          %s" % pr)
        else:
            print("  [ OK  ] %-14s %s  (%s, %d symbol(s))"
                  % (rid, fname, doorway, len(syms)))
    return stale, unknown


def main():
    rows_dir = ROWS
    if "--rows" in sys.argv:
        rows_dir = Path(sys.argv[sys.argv.index("--rows") + 1])

    if "--selftest" in sys.argv:
        # ================================================================
        # THE CONTROL - fixture rows whose declarations are REQUIRED to be
        # contradicted by the real images, plus one required-OK. Poisons
        # symbols the metric actually reads (the tier2-fixture lesson: an
        # out-of-scope poison validates nothing while looking rigorous).
        # ================================================================
        import tempfile
        d = Path(tempfile.mkdtemp())
        (d / "ok.md").write_text(
            "id: sok\nsymbols: dStage_searchName\ndoorway: EXISTS\n",
            encoding="utf-8")
        (d / "flip1.md").write_text(
            "id: sflip1\nsymbols: zzz_not_a_symbol_anywhere\n"
            "doorway: EXISTS\n", encoding="utf-8")
        (d / "flip2.md").write_text(
            "id: sflip2\nsymbols: dStage_searchName\n"
            "doorway: ABSENT-unhookable\n", encoding="utf-8")
        print("SELFTEST - one row must pass, two must come back STALE:")
        rows_dir = d

    rows = load_rows(rows_dir)
    if not rows:
        print("no active rows under %s - nothing to check (not a pass: "
              "an empty store checks nothing)" % rows_dir)
        return 0

    own, up = manifest(OWN_EXE), manifest(UP_EXE)
    print("ROW DOORWAY CHECK - %d active row(s); fork manifest %s - vanilla "
          "manifest %s"
          % (len(rows),
             "loaded" if own else "UNAVAILABLE",
             "loaded" if up else "UNAVAILABLE"))
    stale, unknown = check_rows(rows, own, up)

    if "--selftest" in sys.argv:
        fired = stale == 2 and unknown == 0
        print("\nSELFTEST %s - %d/2 required STALEs fired, required-OK %s"
              % ("FIRED" if fired else "**FAILED**", stale,
                 "held" if stale == 2 else "unverified"))
        return 0 if fired else 1

    print("\n%d OK - %d STALE - %d UNKNOWN lookup(s)"
          % (len(rows) - stale, stale, unknown))
    if stale:
        print("STALE rows need RE-AFFIRMATION (one confirmation, not a "
              "re-derivation) before the build passes - amendment §5a.")
    return 2 if unknown else (1 if stale else 0)


if __name__ == "__main__":
    sys.exit(main())
