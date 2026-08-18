#!/usr/bin/env python3
# ============================================================================
# drain_worksheet.py - GENERATE the tracker-drain worksheet: every
#                      LEGACY-UNROWED TU, one line each, with its live
#                      KIT-LINEAGE value, ready for History/Bridge to
#                      adjudicate into rows.
#
# ---------------------------------------------------------------------------
# WHY GENERATED: the drain list is 90+ entries and changes every time a row
# lands - a hand-kept copy is wrong the day after it is written (the
# tool_index lesson, verbatim). This reuses ww_ratchet's own scope and split
# logic via import, so the worksheet can never disagree with the ratchet's
# ROWED/LEGACY readout - one enumerator, not two (the calls_rotate lesson).
#
# WHY IT EXISTS (user order 2026-08-16): "the tracker that has all necessary
# fork code traced for potential future patches: the crux that the tracker
# was established upon" - rowing had been accreting opportunistically; the
# order makes it a driven campaign. This sheet makes the remaining work
# ENUMERABLE so the per-row cost is adjudication, not discovery.
#
# Usage:  drain_worksheet.py            print
#         drain_worksheet.py --write    write docs/state/ww-staging/DRAIN-WORKSHEET.md
# ============================================================================
import importlib.util
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
OUT = REPO / "docs" / "state" / "ww-staging" / "DRAIN-WORKSHEET.md"


def _mod(name):
    spec = importlib.util.spec_from_file_location(name, HERE / (name + ".py"))
    m = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(m)
    return m


def main():
    rat = _mod("ww_ratchet")
    rs = _mod("row_store")
    _, names = rat.lineage_tus()
    fl_n, floor_files = rat.terminal_floor(names)
    floor_set = {Path(f).as_posix() for f in (floor_files or [])}

    covered = set()
    rows_dir = rat.ROWS_DIR
    if rows_dir.is_dir():
        for p in rows_dir.glob("*.md"):
            f = rs.parse_row(p)
            if f.get("tu"):
                covered.add(Path(f["tu"]).as_posix())

    legacy, rowed = [], []
    for n in names:
        pn = Path(n).as_posix()
        if pn in floor_set:
            continue
        (rowed if pn in covered else legacy).append(pn)

    lines = [
        "# TRACKER DRAIN WORKSHEET (GENERATED - do not hand-edit)",
        "",
        "> Regenerate: `python tools/foundry/drain_worksheet.py --write`.",
        "> One line per LEGACY-UNROWED TU; it leaves this sheet the day a",
        "> tracker row carries its `tu:` field. Scope and split come from",
        "> `ww_ratchet` ITSELF (imported, not copied) so this sheet cannot",
        "> disagree with the drain readout. `lineage` is parsed LIVE from",
        "> each TU's own KIT-LINEAGE header - never stored (fifth-roster).",
        ">",
        "> **USER ORDER 2026-08-16: the drain is a driven campaign, not an",
        "> accretion.** Done = this file lists ZERO TUs.",
        "",
        "Standing state: **ROWED %d · LEGACY-UNROWED %d · floor %d** (of %d)"
        % (len(rowed), len(legacy), fl_n or 0, len(names)),
        "",
        "## LEGACY-UNROWED - the campaign",
        "",
        "| # | TU | lineage (live) |",
        "|---|---|---|",
    ]
    for i, n in enumerate(legacy, 1):
        lines.append("| %d | `%s` | %s |" % (i, n, rs.lineage_of(n)))
    lines += [
        "",
        "## ROWED - for the record",
        "",
    ]
    for n in rowed:
        lines.append("- `%s`" % n)
    lines += [
        "",
        "## Floor (never migrates - adjudicated ownership map)",
        "",
    ]
    for n in sorted(floor_set):
        lines.append("- `%s`" % n)
    text = "\n".join(lines) + "\n"
    if "--write" in sys.argv:
        OUT.write_text(text, encoding="utf-8", newline="\n")
        print("wrote %s (%d legacy · %d rowed · %d floor)"
              % (OUT.relative_to(REPO), len(legacy), len(rowed),
                 len(floor_set)))
        return 0
    print(text)
    return 0


if __name__ == "__main__":
    sys.exit(main())
