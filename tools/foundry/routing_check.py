#!/usr/bin/env python3
# ============================================================================
# routing_check.py — did my WHOSE-TURN routings actually become CALLS rows?
#
# WHY IT EXISTS: I proposed CALLS.md ("a WHOSE-TURN mention is narrative; the
# CALLS row is what triggers a lane") and then twice routed work to other
# lanes in prose only — caught both times by the user, not by me. The author
# of a protocol being its worst offender is a mechanism problem, not a
# discipline problem: the ferry FELT like delivery because it read like
# delivery.
#
# WHAT IT DOES: reads the last N sections of a bus, extracts every lane named
# in a WHOSE TURN / Turns block, and reports which of those lanes have NO
# open CALLS row. Fast, read-only, run it right after any ferry.
#
# HONEST LIMIT: it matches on LANE NAME, not on the ask. A lane with an open
# row about something else reads as covered. It answers "did anyone get
# told?", not "did the right thing get told" — the second is judgment, and
# this tool does not pretend otherwise (№31-C: state the limit, don't imply
# a coverage it cannot prove).
#
# Usage: routing_check.py [--sections N] [--lane FOUNDRY]
# Exit 0 all routed lanes have rows · 1 gaps found · 2 could-not-run.
# ============================================================================
import re
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
BUSES = {
    "tale": REPO / "docs" / "state" / "ww-tale-dmesg-live-state.md",
    "interconnected": REPO / "docs" / "WW Linked" / "ww-bridge-tool-interconnected.md",
}
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"

# Lanes that can be called. "user" is excluded deliberately: the user is not
# triggered by a file, and filing rows at them would be noise.
LANES = ["HISTORY/ENGINE", "HISTORY/BRIDGE", "HOUSING/ENGINE", "HOUSINGTEMP",
         "INTEGRATOR", "LIBRARIAN", "FOUNDRY", "BRIDGE", "HISTORY", "HOUSING",
         "ENGINE", "ALL LANES", "ALL"]
RE_TURNS = re.compile(r"(?:WHOSE TURN|\*\*Turns\.?\*\*)(.{0,1400})", re.S | re.I)


def main():
    n = 3
    if "--sections" in sys.argv:
        i = sys.argv.index("--sections")
        if i + 1 < len(sys.argv):
            n = int(sys.argv[i + 1])
    only = None
    if "--lane" in sys.argv:
        i = sys.argv.index("--lane")
        if i + 1 < len(sys.argv):
            only = sys.argv[i + 1].upper()

    if not CALLS.is_file():
        print("CALLS.md missing — cannot check (UNKNOWN, not clean)")
        return 2
    calls = CALLS.read_text(encoding="utf-8", errors="replace")
    open_rows = re.findall(r"(?m)^- \[ \] ([A-Z/ ]+?)\s*\|", calls)
    open_lanes = {r.strip().upper() for r in open_rows}

    routed = {}
    for bus, path in BUSES.items():
        if not path.is_file():
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        heads = [m.start() for m in re.finditer(r"(?m)^## ", text)]
        for start in heads[-n:]:
            end = text.find("\n## ", start + 3)
            sec = text[start:end if end > 0 else len(text)]
            title = sec.splitlines()[0][:70]
            if only and only not in title.upper():
                continue
            m = RE_TURNS.search(sec)
            if not m:
                continue
            block = m.group(1)
            for lane in LANES:
                # the lane must be addressed, not merely mentioned in prose
                if re.search(r"\*\*%s\*\*|^%s\b|· *%s\b" % (re.escape(lane),
                                                            re.escape(lane),
                                                            re.escape(lane)),
                             block, re.M | re.I):
                    routed.setdefault(lane.upper(), set()).add("%s: %s" % (bus, title))

    # collapse: a lane covered by a broader open row (HISTORY vs HISTORY/ENGINE)
    def covered(lane):
        return any(lane == o or lane in o or o in lane for o in open_lanes)

    gaps = {l: s for l, s in routed.items() if not covered(l) and l != "FOUNDRY"}
    print("routing check — last %d section(s) per bus" % n)
    print("  lanes routed to: %s" % (", ".join(sorted(routed)) or "(none)"))
    print("  lanes with an open CALLS row: %s" % (", ".join(sorted(open_lanes)) or "(none)"))
    if gaps:
        print("\n  [NO ROW] routed in prose but no open CALLS row:")
        for lane, secs in sorted(gaps.items()):
            print("    %-16s from %s" % (lane, "; ".join(sorted(secs))[:100]))
        print("\n  -> file a row, or the routing triggers nobody.")
    else:
        print("\n  every routed lane has an open row (name-level check).")
    print("\n  LIMIT: name-level only — an open row about something ELSE reads as "
          "covered.\n  This answers 'was anyone told?', not 'was the right thing told?'.")
    return 1 if gaps else 0


if __name__ == "__main__":
    sys.exit(main())
