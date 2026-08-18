#!/usr/bin/env python3
# ============================================================================
# queue_verdict_lint.py - NO SYNTAX PASS MAY SIT ON AN OPEN QUEUE ROW DRESSED
#                         AS A COMPILE VERDICT.
#
# ---------------------------------------------------------------------------
# WHY THIS EXISTS, counted rather than argued: the /Zs-cited-as-verification
# class has THREE specimens (Housing's own count, CALLS row "Q2 ACTIONED"
# 2026-08-16): (1) the control.py first case receipt - four LNK2005s reached
# the Integrator THROUGH A GREEN GATE stamped by /Zs; (2) the registry.cpp
# two-lane latent link error this session; (3) five open queue rows stamped
# "cl /Zs EXIT=0" the same afternoon their author wrote "/Zs proves syntax
# only" into their own memory. **Knowing a thing is not the same as having a
# rule about it** - and a rule nobody checks mechanically is a note. Third
# specimen = the registration threshold this lane put on the record earlier
# the same day (scope-shadow row).
#
# THE RULE (Librarian, DN-12 framing, 2026-08-16): /Zs may never be CITED as
# a compile gate. It never links. A syntax pass may appear on a row as
# CONTEXT, but only alongside an explicit UNVERIFIED marker - never as the
# verdict.
#
# WHAT THIS LINTS: OPEN rows ("- [ ]") of BUILD-QUEUE.md whose text cites a
# syntax pass as if it verified (/Zs plus EXIT=0 / clean / pass / verified)
# WITHOUT the word UNVERIFIED in the same row. Closed rows are deliberately
# out of scope - rewriting the record of what was claimed is worse than the
# wrong claim (Housing's own boundary, kept here).
#
# Usage:  queue_verdict_lint.py [path]      lint (default: the live queue)
#         queue_verdict_lint.py --selftest  NEGATIVE CONTROL - must go red
# Exit 0 = clean · 1 = violations · 2 = queue file missing (UNKNOWN, not clean)
# ============================================================================
import re
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
QUEUE = REPO / "docs" / "state" / "ww-staging" / "BUILD-QUEUE.md"

RE_OPEN = re.compile(r"^\s*-\s*\[\s\]")
RE_ZS = re.compile(r"/Zs", re.I)
RE_VERDICT = re.compile(r"EXIT\s*=?\s*0|\bclean\b|\bpass(?:es|ed)?\b|"
                        r"\bverified\b|\bcompiles?\b", re.I)
RE_HONEST = re.compile(r"UNVERIFIED", re.I)


def lint(path):
    if not path.is_file():
        print("REFUSED - no queue at %s. A missing queue is UNKNOWN, not "
              "clean." % path)
        return 2
    bad = []
    for n, line in enumerate(path.read_text(encoding="utf-8",
                                            errors="replace").splitlines(), 1):
        if not RE_OPEN.match(line):
            continue
        if RE_ZS.search(line) and RE_VERDICT.search(line) \
                and not RE_HONEST.search(line):
            bad.append((n, line.strip()[:120]))
    if bad:
        print("%d OPEN row(s) cite a /Zs syntax pass as a verdict with no "
              "UNVERIFIED marker (DN-12: /Zs is never a compile gate - it "
              "never links):" % len(bad))
        for n, text in bad:
            print("  line %d: %s" % (n, text))
        print("Fix: add 'COMPILE STATUS: UNVERIFIED' to the row, or remove "
              "the verdict wording. The syntax pass may stay as context.")
        return 1
    print("queue clean: no open row dresses a /Zs syntax pass as a verdict")
    return 0


def selftest():
    """NEGATIVE CONTROL - a rigged queue row MUST be flagged, and the honest
    form MUST pass. A lint that cannot go red certifies nothing."""
    import tempfile
    d = Path(tempfile.mkdtemp())
    bad = d / "q_bad.md"
    bad.write_text(
        "- [ ] HOUSING | two-axis manifest | All three TUs pass "
        "cl /Zs /std:c++20 EXIT=0 | FILES: registry.cpp\n",
        encoding="utf-8")
    good = d / "q_good.md"
    good.write_text(
        "- [ ] HOUSING | two-axis manifest | cl /Zs EXIT=0 (shape only) - "
        "COMPILE STATUS: UNVERIFIED | FILES: registry.cpp\n"
        "- [x] HOUSING | closed row keeps its historical /Zs EXIT=0 stamp\n",
        encoding="utf-8")
    print("SELFTEST - the rigged row MUST flag, the honest one MUST pass\n")
    ok1 = lint(bad) == 1
    print("  [%-42s] %s" % ("verdict-dressed /Zs on open row -> RED",
                            "DETECTED" if ok1 else "**BLIND**"))
    ok2 = lint(good) == 0
    print("  [%-42s] %s" % ("UNVERIFIED-marked + closed rows -> pass",
                            "DETECTED" if ok2 else "**BLIND**"))
    ok3 = lint(d / "missing.md") == 2
    print("  [%-42s] %s" % ("missing queue -> REFUSED, not clean",
                            "DETECTED" if ok3 else "**BLIND**"))
    blind = [ok1, ok2, ok3].count(False)
    print("\n%s" % ("CONTROL OK - all cases DETECTED" if not blind
                    else "** %d BLIND CASE(S) **" % blind))
    return 1 if blind else 0


def main():
    if "--selftest" in sys.argv[1:]:
        return selftest()
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else QUEUE
    return lint(path)


if __name__ == "__main__":
    sys.exit(main())
