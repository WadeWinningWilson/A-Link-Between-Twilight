#!/usr/bin/env python3
# ============================================================================
# layer_census.py — HOW BIG IS THE RECEIVER'S WW LAYER? Answer: name a scope
# first, because the question has no scope-free answer.
#
# THE LIVE FAILURE THIS ANSWERS (CALLS row, tale §933): three counts of "the
# receiver layer" are on the record and cannot be reconciled — 34 Tier-1 files
# (strip set, 2026-07-25) · ~79 .cpp / 51 .h (Librarian) · 49 .cpp / 44 .h
# (Integrator, four dirs, scope stated). All may be correct over different
# sets. Measuring it myself produced TWO MORE (38 .cpp / 37 .h by filename
# marker; 102 .cpp carrying a KIT-LINEAGE tag) — which settles nothing and
# proves everything: **a count without a declared scope is not a measurement.**
# That is exactly what the inventory's mandatory SCOPE field exists to stop,
# and it was happening inside the thing being inventoried.
#
# WHAT THIS DOES: reports the layer under EVERY declared scope at once, with
# each scope's definition and origin, then RECONCILES them pairwise — so the
# numbers stop competing and start explaining each other.
#
# THE AUTHORITY QUESTION, stated not dodged: the scopes below are all
# TREE-SIDE inferences. The one authoritative answer is RUNTIME — the plugin's
# boot manifest (§924 landed the profile half; the arc-roster half is owed).
# When that exists it becomes scope RUNTIME and the tree-side scopes become
# cross-checks against it, not competitors.
#
# Usage: layer_census.py [--files <scope>]   list the members of one scope
# Read-only. Exit 0.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
STRIP = REPO / "docs" / "NEVER-PUSH-STRIP-SET.md"
RE_PATH = re.compile(r"((?:src|include)/[\w/\.-]+\.(?:cpp|h))")


def scope_strip_set():
    """Files named in the never-push strip set (Tier-1 + whatever else it names)."""
    if not STRIP.is_file():
        return set(), "MISSING docs/NEVER-PUSH-STRIP-SET.md"
    txt = STRIP.read_text(encoding="utf-8", errors="replace")
    return {p for p in RE_PATH.findall(txt)}, "paths named in NEVER-PUSH-STRIP-SET.md"


def scope_filename():
    out = set()
    for pat, base in (("src/**/*.cpp", "src"), ("include/**/*.h", "include")):
        for p in REPO.glob(pat):
            if any(k in p.name for k in ("_ww_", "ww_", "_ext_", "ext_")):
                out.add(p.relative_to(REPO).as_posix())
    return out, "filename carries ww_/_ww_/ext_/_ext_"


def scope_ext_dirs():
    out = set()
    for d in ("src/d/ext_plugin", "src/d/ext_evt"):
        for p in (REPO / d).glob("*.*"):
            if p.suffix in (".cpp", ".h"):
                out.add(p.relative_to(REPO).as_posix())
    return out, "everything under src/d/ext_plugin + src/d/ext_evt"


def scope_lineage():
    out = set()
    for p in list(REPO.glob("src/**/*.cpp")) + list(REPO.glob("include/**/*.h")):
        try:
            head = p.read_text(encoding="utf-8", errors="replace")[:4000]
        except OSError:
            continue
        if "KIT-LINEAGE" in head:
            out.add(p.relative_to(REPO).as_posix())
    return out, "TU declares a // KIT-LINEAGE tag (§426, grep-exact)"


SCOPES = [
    ("STRIP-SET", scope_strip_set),
    ("FILENAME", scope_filename),
    ("EXT-DIRS", scope_ext_dirs),
    ("LINEAGE-DECLARED", scope_lineage),
]


def main():
    want = None
    if "--files" in sys.argv:
        i = sys.argv.index("--files")
        if i + 1 < len(sys.argv):
            want = sys.argv[i + 1].upper()

    sets = {}
    print("RECEIVER WW-LAYER CENSUS — every count carries its SCOPE or it is not a count")
    print("=" * 78)
    for name, fn in SCOPES:
        members, why = fn()
        sets[name] = members
        cpp = sum(1 for m in members if m.endswith(".cpp"))
        h = sum(1 for m in members if m.endswith(".h"))
        exists = sum(1 for m in members if (REPO / m).is_file())
        print("\n%-18s %d file(s): %d .cpp · %d .h" % (name, len(members), cpp, h))
        print("    definition: %s" % why)
        if exists != len(members):
            print("    NOTE: %d of %d no longer exist in the tree (stale scope entries)"
                  % (len(members) - exists, len(members)))
        if want == name:
            for m in sorted(members):
                print("      %s %s" % ("+" if (REPO / m).is_file() else "-", m))

    print("\n" + "=" * 78)
    print("RECONCILIATION — why the numbers differ (pairwise, both directions)")
    names = [n for n, _ in SCOPES]
    for i, a in enumerate(names):
        for b in names[i + 1:]:
            only_a = sets[a] - sets[b]
            only_b = sets[b] - sets[a]
            print("\n  %s vs %s: shared %d · only-%s %d · only-%s %d"
                  % (a, b, len(sets[a] & sets[b]), a, len(only_a), b, len(only_b)))
            for m in sorted(only_a)[:3]:
                print("      only %s: %s" % (a, m))
            for m in sorted(only_b)[:3]:
                print("      only %s: %s" % (b, m))

    print("\n" + "=" * 78)
    print("AUTHORITY: all four scopes above are TREE-SIDE INFERENCES. The")
    print("authoritative answer is RUNTIME — the plugin's boot manifest (§924")
    print("landed the profile half; the arc-roster half is owed). When it exists,")
    print("these become CROSS-CHECKS against it, not competing truths.")
    print("A count quoted without its scope tag cannot be reconciled with any other.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
