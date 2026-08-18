#!/usr/bin/env python3
"""Derive the WW name -> object-arc map from DONOR SOURCE, mechanically.

WHY THIS EXISTS (Foundry's census, ruling item 2): the generic-prop catch-all's
first real step is `resLoad(<arc>)`, and it has NO INPUT. `dStage_objectNameInf`
is `{ char name[8]; s16 procname; s8 argument; s8 gbaName }` — there is NO ARC
FIELD anywhere in the receiver's name table. Without this map the catch-all
reaches 15% of Outset instead of 39%.

WHY IT IS DISCOVERY AND NOT AUTHORING (the invariant this whole design rests
on): every row is READ OUT OF THE DONOR'S OWN SOURCE — the `m_arcname` statics
each actor declares, and the string literals it hands to `dComIfG_getObjectRes`.
Nothing here is typed from knowledge. Re-running the script re-derives the map;
if the donor says something different, the map changes with it. Three authored
constants were wrong out of three attempts tonight, which is the receipt behind
that rule.

SCALES WITH PROCS, NOT NAMES — the census's central finding. One actor TU maps
to one proc; the many names sharing that proc all resolve through it.

OUTPUT: TSV on stdout (proc-source-stem, arc-name, evidence-kind, source-line)
plus a summary to stderr. Deliberately NOT a C header: the consumer decides its
own storage, and a generated header invites hand-editing, which would turn a
derived map back into an authored one.

USAGE:
    python tools/foundry/ww_name_to_arc.py [--donor "D:/XXXXXXX/WW DP"] [--tsv out.tsv]
"""

import argparse
import os
import re
import sys

# `static const char M_arcname[N] = "Foo";` and its `mArcName`/`arcName` kin.
RE_ARCNAME_STATIC = re.compile(
    r'\b(?:M_arcname|m_arcname|mArcName|arcName)\s*(?:\[[^\]]*\])?\s*=\s*"([^"]+)"'
)
# `dComIfG_getObjectRes("Foo", ...)` / `dComIfGp_getObjectRes("Foo", ...)`.
RE_GETOBJRES_LITERAL = re.compile(
    r'\bdComIfG[a-zA-Z_]*_getObjectRes\s*\(\s*"([^"]+)"'
)
# `attr().arcName` style tables: `{"Foo", ...}` inside an obvious arc table is
# NOT harvested — too loose to be discovery rather than guessing. Recorded here
# so the omission is deliberate and visible rather than an oversight.


def scan_tu(path):
    """Return (arc_name, kind, lineno) evidence rows for one donor TU."""
    out = []
    try:
        with open(path, "r", encoding="utf-8", errors="replace") as fh:
            for lineno, line in enumerate(fh, 1):
                # Skip obvious comment lines: a commented-out arc name is not
                # evidence of what the actor loads.
                stripped = line.lstrip()
                if stripped.startswith("//") or stripped.startswith("*"):
                    continue
                for m in RE_ARCNAME_STATIC.finditer(line):
                    out.append((m.group(1), "m_arcname", lineno))
                for m in RE_GETOBJRES_LITERAL.finditer(line):
                    out.append((m.group(1), "getObjectRes", lineno))
    except OSError as exc:
        print(f"  ! unreadable: {path}: {exc}", file=sys.stderr)
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--donor", default=r"D:/XXXXXXX/WW DP",
                    help="donor decomp root (contains src/d/actor)")
    ap.add_argument("--tsv", help="write TSV here instead of stdout")
    args = ap.parse_args()

    actor_dir = os.path.join(args.donor, "src", "d", "actor")
    if not os.path.isdir(actor_dir):
        print(f"donor actor dir not found: {actor_dir}", file=sys.stderr)
        print("REFUSING to emit a partial map — an incomplete map that looks "
              "complete is the failure this script exists to avoid.",
              file=sys.stderr)
        return 2

    rows = []
    tus_scanned = 0
    tus_with_arc = 0
    for entry in sorted(os.listdir(actor_dir)):
        if not entry.endswith(".cpp"):
            continue
        tus_scanned += 1
        stem = entry[:-4]
        found = scan_tu(os.path.join(actor_dir, entry))
        if not found:
            continue
        tus_with_arc += 1
        seen = set()
        for arc, kind, lineno in found:
            key = (stem, arc)
            if key in seen:
                continue
            seen.add(key)
            rows.append((stem, arc, kind, lineno))

    out = open(args.tsv, "w", encoding="utf-8", newline="") if args.tsv else sys.stdout
    try:
        print("# DERIVED FROM DONOR SOURCE — do not hand-edit; re-run to refresh.",
              file=out)
        print("# tu\tarc\tevidence\tline", file=out)
        for stem, arc, kind, lineno in rows:
            print(f"{stem}\t{arc}\t{kind}\t{lineno}", file=out)
    finally:
        if args.tsv:
            out.close()

    arcs = {r[1] for r in rows}
    multi = {}
    for stem, arc, _, _ in rows:
        multi.setdefault(stem, set()).add(arc)
    many = {k: v for k, v in multi.items() if len(v) > 1}

    print(f"scanned {tus_scanned} donor actor TUs", file=sys.stderr)
    print(f"  {tus_with_arc} declare at least one arc", file=sys.stderr)
    print(f"  {len(rows)} (tu, arc) pairs · {len(arcs)} distinct arcs", file=sys.stderr)
    print(f"  {len(many)} TUs name MORE THAN ONE arc — these need the "
          f"consumer to pick by type/param, not by first-match:", file=sys.stderr)
    for k in sorted(many)[:12]:
        print(f"    {k}: {', '.join(sorted(many[k]))}", file=sys.stderr)
    if len(many) > 12:
        print(f"    ... and {len(many) - 12} more", file=sys.stderr)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
