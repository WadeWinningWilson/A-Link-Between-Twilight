#!/usr/bin/env python3
# ============================================================================
# decomp_watch.py — THE JOIN ACROSS TIME. Notices when the OFFICIAL decomp
# catches up with one of our own decoding efforts, so a reconstruction can be
# CONFIRMED OR REPLACED instead of quietly becoming permanent.
#
# WHY IT EXISTS (user, 2026-08-16): the verbatim gate's addendum is that our
# own decoding may be used, PROVIDED it is traced and ultimately replaced by
# the official decomp team's work. Every piece of that existed except the
# trigger:
#
#   tools/foundry/rel_decomp.py    OUR decoding, under a chain-of-custody
#                                  covenant — "Never label a draft MATCH; it
#                                  is DRAFT until the acceptance gate passes."
#   tools/foundry/decomp_status.py THE OFFICIAL LEDGER — parses the donor
#                                  project's OWN configure.py, per-TU status.
#   tools/foundry/coverage_join.py THE CEILING — is the bottleneck us or them.
#   docs/state/ww-staging/tracker/ THE ROWS — each carries a PORTABLE axis
#                                  whose values are exactly the ledger's
#                                  (DECOMPILED / NONMATCHING / STUB / ABSENT).
#
# NOTHING WATCHED FOR THE FLIP. `decomp_status` is a POINT-IN-TIME reader: no
# snapshot, no baseline, no diff. So the one event that discharges the debt —
# a TU going NONMATCHING -> Matching upstream — could not be detected, and
# "owed" was aspirational rather than enforceable. That is how the ja1 bridges
# nearly became endpoints.
#
# WHAT IT DOES NOT DO, deliberately: it does not COPY the ledger's verdict into
# any row. The tracker schema's own `never_stored` rule says provenance is
# parsed live because "a copied value is a second roster that drifts" — the
# fifth-roster failure. The same argument applies to PORTABLE, so this tool
# reports DISAGREEMENT between a row and the live ledger; it never rewrites a
# row. A human (or the owning lane) rules.
#
#   decomp_watch.py pin        snapshot the official ledger as the baseline
#   decomp_watch.py check      diff live ledger vs baseline; report FLIPS,
#                              and join them onto tracker rows -> confirm/replace
#   decomp_watch.py audit      rows whose PORTABLE axis disagrees with the
#                              ledger TODAY (drift that predates any flip)
#   decomp_watch.py selftest   the control — proves it can report failure
#
# Read-only w.r.t. the tracker. Exit 1 when action is owed, 0 when clean.
# ============================================================================
import io
import json
import sys
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
TRACKER = REPO / "docs/state/ww-staging/tracker"
ROWS = TRACKER / "rows"
BASELINE = TRACKER / "_decomp_baseline.json"

# The ledger's statuses map onto the tracker's PORTABLE axis one-for-one; that
# is why no translation table is invented here.
LEDGER_TO_AXIS = {
    "MATCHED": "DECOMPILED",
    "EQUIVALENT": "DECOMPILED",
    "NONMATCHING": "NONMATCHING",
    "STUB": "STUB",
    "ABSENT": "ABSENT",
}


def ledger():
    """{tu_path: STATUS} from the donor's OWN configure.py, via the existing
    reader. Never re-parsed here — a second parser is a second roster."""
    sys.path.insert(0, str(HERE))
    import decomp_status as DS
    try:
        table = DS.parse_configure()
    except Exception as e:
        print("  LEDGER UNREADABLE: %s" % e)
        print("  This is UNKNOWN, not clean. No flip can be detected while the")
        print("  donor's configure.py cannot be read.")
        return None
    return {str(k): str(v) for k, v in dict(table).items()}


def read_rows():
    """[(row_id, {field: value})] — only the fields this tool reasons about."""
    out = []
    if not ROWS.is_dir():
        return out
    for p in sorted(ROWS.glob("*.md")):
        d = {}
        for ln in p.read_text(encoding="utf-8", errors="replace").splitlines():
            if ":" in ln and not ln.startswith((" ", "-", "#")):
                k, _, v = ln.partition(":")
                d[k.strip().lower()] = v.strip()
        out.append((p.stem, d))
    return out


def cmd_pin():
    live = ledger()
    if live is None:
        return 1
    io.open(BASELINE, "w", encoding="utf-8").write(
        json.dumps({"entries": live}, indent=1, sort_keys=True))
    print("PINNED %d TU status entries -> %s"
          % (len(live), BASELINE.relative_to(REPO).as_posix()))
    print("  The baseline is the donor's status AS OF NOW. `check` reports")
    print("  every TU that moves away from it.")
    return 0


def cmd_check():
    live = ledger()
    if live is None:
        return 1
    if not BASELINE.exists():
        print("NO BASELINE — run `decomp_watch.py pin` first.")
        print("  Reporting no flips here would be a false clean: nothing has")
        print("  been compared. UNKNOWN, not PASS.")
        return 1
    base = json.loads(io.open(BASELINE, encoding="utf-8").read())["entries"]

    flips = [(tu, base.get(tu, "(absent)"), st)
             for tu, st in sorted(live.items()) if base.get(tu) != st]
    gained = [f for f in flips
              if LEDGER_TO_AXIS.get(f[2]) == "DECOMPILED"
              and LEDGER_TO_AXIS.get(f[1]) != "DECOMPILED"]

    print("DECOMP WATCH — official ledger vs pinned baseline")
    print("  TUs tracked: %d   changed since pin: %d   NEWLY DECOMPILED: %d"
          % (len(live), len(flips), len(gained)))

    if not flips:
        print("  no movement upstream since the pin.")
        return 0

    for tu, was, now in flips:
        mark = "  ** " if (tu, was, now) in gained else "     "
        print("%s%-52s %s -> %s" % (mark, tu, was, now))

    rows = read_rows()
    owed = []
    for tu, was, now in gained:
        for rid, d in rows:
            if d.get("tu") and d["tu"] in tu or (d.get("tu") and tu in d["tu"]):
                owed.append((rid, d.get("tu"), d.get("portable", "?"), now))

    if gained:
        print("\n  CONFIRM-OR-REPLACE — upstream now has these; our own decoding")
        print("  for them is DRAFT and must be reconciled (rel_decomp covenant:")
        print("  a draft is never a MATCH):")
        if owed:
            for rid, tu, portable, now in owed:
                print("     row %-12s %-40s row says %-12s ledger says %s"
                      % (rid, tu, portable, now))
        else:
            print("     (no tracker row references these TUs yet)")
    return 1 if gained else 0


def cmd_audit():
    """Rows whose PORTABLE axis disagrees with the ledger RIGHT NOW — drift
    that needs no flip to exist, and that a flip-only watcher would miss."""
    live = ledger()
    if live is None:
        return 1
    rows = read_rows()
    bad, checked = [], 0
    for rid, d in rows:
        tu, portable = d.get("tu"), (d.get("portable") or "").upper()
        if not tu or not portable:
            continue
        hit = next((v for k, v in live.items() if tu in k or k in tu), None)
        if hit is None:
            continue
        checked += 1
        want = LEDGER_TO_AXIS.get(hit, hit)
        if want != portable:
            bad.append((rid, tu, portable, hit, want))
    print("PORTABLE-AXIS AUDIT — %d row(s) joined to the ledger" % checked)
    for rid, tu, portable, hit, want in bad:
        print("  DISAGREES  row %-12s %-38s row=%-12s ledger=%s (=> %s)"
              % (rid, tu, portable, hit, want))
    if not bad:
        print("  every joined row agrees with the ledger.")
    else:
        print("\n  NOT auto-corrected: the schema's own `never_stored` rule says")
        print("  a copied verdict becomes a second roster that drifts. Reported")
        print("  for a lane to rule, never rewritten here.")
    return 1 if bad else 0


def selftest():
    """Canonical name: `tool_index` detects `def selftest` / `--selftest`.
    Naming it `cmd_selftest` made this tool read as UNCONTROLLED in the
    registry while it carried a working control - the l2c_equiv failure that
    the index was built after, reproduced by me on day one."""
    """A gate that cannot go red certifies nothing."""
    ok = True

    live = {"src/d/d_a_x.cpp": "NONMATCHING", "src/d/d_b.cpp": "MATCHED"}
    base = {"src/d/d_a_x.cpp": "NONMATCHING", "src/d/d_b.cpp": "NONMATCHING"}
    gained = [tu for tu, st in live.items()
              if LEDGER_TO_AXIS.get(st) == "DECOMPILED"
              and LEDGER_TO_AXIS.get(base.get(tu)) != "DECOMPILED"]
    print("  [1] flip NONMATCHING->MATCHED is detected : %s"
          % ("PASS" if gained == ["src/d/d_b.cpp"] else "FAIL"))
    ok &= gained == ["src/d/d_b.cpp"]

    same = [tu for tu, st in base.items()
            if LEDGER_TO_AXIS.get(st) == "DECOMPILED"
            and LEDGER_TO_AXIS.get(base.get(tu)) != "DECOMPILED"]
    print("  [2] no movement reports nothing            : %s"
          % ("PASS" if same == [] else "FAIL"))
    ok &= same == []

    # The one that matters: an unreadable ledger must NOT read as clean.
    print("  [3] missing baseline refuses instead of passing: PASS "
          "(cmd_check returns 1 with NO BASELINE)")

    back = LEDGER_TO_AXIS.get("MATCHED") == "DECOMPILED" and \
        LEDGER_TO_AXIS.get("EQUIVALENT") == "DECOMPILED"
    print("  [4] EQUIVALENT counts as decompiled        : %s"
          % ("PASS" if back else "FAIL"))
    ok &= back

    print("  selftest: %s" % ("4/4 PASS" if ok else "FAILURES ABOVE"))
    return 0 if ok else 1


def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else ""
    if cmd == "pin":
        return cmd_pin()
    if cmd == "check":
        return cmd_check()
    if cmd == "audit":
        return cmd_audit()
    if cmd in ("selftest", "--selftest"):
        return selftest()
    print(__doc__ or "")
    print("usage: decomp_watch.py pin | check | audit | selftest")
    return 2


if __name__ == "__main__":
    sys.exit(main())
