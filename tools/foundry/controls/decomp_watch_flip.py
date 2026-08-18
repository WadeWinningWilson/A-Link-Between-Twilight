#!/usr/bin/env python3
# ============================================================================
# CONTROL FIXTURE — decomp_watch MUST detect an upstream flip.
#
# THE THING IT PROVES CAN FAIL: a TU that was NONMATCHING in the pinned
# baseline and is Matching in the live ledger is the ONE event decomp_watch
# exists to catch — it is what discharges a reconstruction debt. If the tool
# reports "no movement" on that, then "our own decoding is owed and will be
# replaced" is a sentence with nothing behind it, and every clean run it has
# ever printed is worthless.
#
# WHY A FIXTURE AND NOT A LIVE RUN: the live ledger only flips when the donor
# project actually lands a match, which may be months. A control that cannot
# be fired on demand is not a control. This builds its own baseline in a temp
# directory and never touches the live tracker or the pinned baseline.
#
# Prints CONTROL OK + DETECTED when the tool behaves; anything else is a red
# meta-gate.
# ============================================================================
import io
import json
import sys
import tempfile
from pathlib import Path

HERE = Path(__file__).resolve().parent
FOUNDRY = HERE.parent
sys.path.insert(0, str(FOUNDRY))

import decomp_watch as DW  # noqa: E402

BASE = {
    "d/d_fixture_a.cpp": "NONMATCHING",   # flips -> must be caught
    "d/d_fixture_b.cpp": "NONMATCHING",   # stays -> must NOT be reported
    "d/d_fixture_c.cpp": "MATCHED",       # already done -> not a NEW gain
}
LIVE = {
    "d/d_fixture_a.cpp": "MATCHED",
    "d/d_fixture_b.cpp": "NONMATCHING",
    "d/d_fixture_c.cpp": "MATCHED",
}


def main():
    tmp = Path(tempfile.mkdtemp(prefix="decomp_watch_ctl_"))
    baseline = tmp / "_decomp_baseline.json"
    io.open(baseline, "w", encoding="utf-8").write(
        json.dumps({"entries": BASE}))

    # Point the tool at the fixture: its own module globals, no live paths.
    DW.BASELINE = baseline
    DW.ROWS = tmp / "rows"          # empty: the join must not be what detects it
    DW.ledger = lambda: dict(LIVE)

    rc = DW.cmd_check()

    gained_expected = ["d/d_fixture_a.cpp"]
    gained_actual = [tu for tu, st in LIVE.items()
                     if DW.LEDGER_TO_AXIS.get(st) == "DECOMPILED"
                     and DW.LEDGER_TO_AXIS.get(BASE.get(tu)) != "DECOMPILED"]

    ok = (rc == 1) and (gained_actual == gained_expected)
    print()
    print("  expected NEWLY DECOMPILED : %s" % gained_expected)
    print("  tool reported             : %s" % gained_actual)
    print("  exit code (1 = action owed): %d" % rc)
    if ok:
        print("CONTROL OK - DETECTED the flip and returned non-zero")
        return 0
    print("CONTROL FAILED - decomp_watch did NOT detect an upstream flip; "
          "every clean run it has printed is uncertified")
    return 1


if __name__ == "__main__":
    sys.exit(main())
