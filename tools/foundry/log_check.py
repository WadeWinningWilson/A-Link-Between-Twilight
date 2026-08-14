#!/usr/bin/env python3
# ===========================================================================
# log_check.py — §695 #2: the playtest verdict table, mechanized.
#
# Reads the PLAYTEST CARD (docs/state/ww-staging/PLAYTEST-CARD.md) as its
# marker registry — one artifact, two uses — and diffs the NEWEST game log
# against it. Three verdicts, and the middle one is the point:
#
#   PASS      MARKER found (and FAIL literal, if declared, absent)
#   UNTESTED  MARKER absent — the run never exercised this check. NEVER green:
#             "no line" and "broken detector" look identical (§593), and a
#             check nobody ran must not read as a check that passed (§615's
#             RCAM warning, mechanized).
#   FAIL      declared FAIL literal present — outranks a found MARKER.
#
# Usage:  log_check.py [--log <path>] [--selftest]
# Exit: 0 all PASS/UNTESTED with zero FAIL · 1 any FAIL · 2 card/log missing.
# ===========================================================================
import glob
import io
import os
import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CARD = REPO / "docs" / "state" / "ww-staging" / "PLAYTEST-CARD.md"
LOGDIR = os.path.join(os.environ.get("APPDATA", ""), "TwilitRealm", "Dusklight", "logs")

LINE = re.compile(r"^- CHECK(!?)\s+(\S+)\s*\|\s*(?:PATH:(\S+)\s*\|\s*)?([^|]+?)\s*\|\s*([^|]+?)\s*\|\s*"
                  r"MARKER:\s*(.+?)(?:\s*\|\s*AFTER:\s*(.+?))?(?:\s*\|\s*(EXPECT:\s*UNTESTED))?(?:\s*\|\s*FAIL:\s*(.+))?$")


def parse_card(path):
    checks = []
    for ln in io.open(path, encoding="utf-8", errors="replace"):
        m = LINE.match(ln.strip())
        if m:
            req, cid, path, lane, what, marker, after, expect, fail = m.groups()
            checks.append((cid, req == "!", (path or "-").strip(), lane.strip(),
                           what.strip(), marker.strip(),
                           after.strip() if after else None,
                           expect is not None,
                           fail.strip() if fail else None))
    return checks


def newest_log():
    logs = sorted(glob.glob(os.path.join(LOGDIR, "*.log")),
                  key=os.path.getmtime, reverse=True)
    return logs[0] if logs else None


def run(card_path, log_path):
    checks = parse_card(card_path)
    if not checks:
        print("card has no CHECK lines: %s" % card_path)
        return 2
    text = io.open(log_path, encoding="utf-8", errors="replace").read()
    print("log_check — card: %s" % card_path)
    print("            log : %s\n" % log_path)
    n_fail = n_pass = n_unt = 0
    req_unt = []
    for cid, req, path, lane, what, marker, after, expected, fail in checks:
        # ====================================================================
        # AFTER: sequence semantics (pass-9 false-PASS fix). PASS requires the
        # MARKER to occur at a position LATER than the AFTER literal's last
        # occurrence — "B after A", not "B anywhere". A marker that measures
        # a state transition must be ordered against the state it leaves, or
        # presence from an earlier visit reads as the transition (scls-tale).
        # ====================================================================
        if fail and fail in text:
            verdict = "FAIL"
            n_fail += 1
        elif marker in text and (after is None or
                                 text.rfind(marker) > text.rfind(after) >= 0):
            verdict = "PASS"
            n_pass += 1
        else:
            # ================================================================
            # HT-32 residue: EXPECT: UNTESTED marks a DELIBERATELY pending
            # check (no entry path yet, branch awaits data). It prints PENDING
            # so an intended gap is not noise -- and it can never satisfy a
            # required mark: a check both required and expected-pending is a
            # card contradiction, reported as such.
            # ================================================================
            if expected:
                verdict = "PENDING"
                if req:
                    verdict = "CARD-BUG"
                    print("  [CARD-BUG ] %s is CHECK! and EXPECT:UNTESTED at once"
                          " -- required and deliberately-pending contradict" % cid)
                    req_unt.append(cid)
            else:
                verdict = "UNTESTED"
                n_unt += 1
                if req:
                    req_unt.append(cid)
        print("  [%-8s] %-6s %-16s %-14s %s" % (verdict, path,
              cid + ("!" if req else ""), lane, what[:46]))
    print("\n  %d PASS · %d UNTESTED · %d FAIL" % (n_pass, n_unt, n_fail))
    if n_unt:
        print("  UNTESTED is not green: those checks were never exercised this run.")
    # ========================================================================
    # HT-29: the CONTRACT layer. A REQUIRED check (- CHECK!) that is UNTESTED
    # makes the RUN unverified -- exit 3, never 0. Row-level truth without
    # contract-level truth is how two green cards shipped on unplayable
    # sessions (pass 9/10). exit: 0 verified / 1 FAIL / 2 missing inputs /
    # 3 required-check untested (SESSION NOT VERIFIED).
    # ========================================================================
    if n_fail:
        return 1
    if req_unt:
        print("  SESSION NOT VERIFIED (exit 3): required check(s) untested: %s"
              % ", ".join(req_unt))
        return 3
    return 0


def selftest():
    # §695 #4 applied to this tool on day one: prove all three verdicts can
    # fire, including that FAIL outranks a found MARKER.
    import tempfile
    card = ("- CHECK a | L | t | MARKER: alpha\n"
            "- CHECK b | L | t | MARKER: beta\n"
            "- CHECK c | L | t | MARKER: gamma | FAIL: gamma-err\n")
    log = "alpha ... gamma ... gamma-err\n"
    d = tempfile.mkdtemp()
    cp, lp = os.path.join(d, "c.md"), os.path.join(d, "l.log")
    io.open(cp, "w", encoding="utf-8").write(card)
    io.open(lp, "w", encoding="utf-8").write(log)
    rc = run(cp, lp)
    ok = (rc == 1)  # one FAIL expected
    print("\n  selftest: expected PASS/UNTESTED/FAIL one each, exit 1 -> %s"
          % ("OK" if ok else "BROKEN"))
    return 0 if ok else 1


def main():
    if "--selftest" in sys.argv:
        return selftest()
    log = None
    if "--log" in sys.argv:
        log = sys.argv[sys.argv.index("--log") + 1]
    else:
        log = newest_log()
    if not CARD.is_file():
        print("no playtest card at %s" % CARD)
        return 2
    if not log or not os.path.isfile(log):
        print("no game log found in %s" % LOGDIR)
        return 2
    return run(CARD, log)


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
