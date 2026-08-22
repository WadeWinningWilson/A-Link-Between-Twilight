#!/usr/bin/env python3
# ============================================================================
# kit_batch_rate.py - DOES THE KIT'S LEARNING LOOP COMPOUND? (E3's instrument)
#
# ROADMAP-E1-E6: "Batch 1 was five of six failing. If batch five is two of
# six, the loop compounds. If it is still five of six, the harvested rules do
# not generalise - and right now nobody would be able to tell which." This is
# the telling-which.
#
# It is a LEDGER + TREND REPORTER, not a build driver: History records each
# first-compile attempt at the moment it happens (pass or fail, and for fails
# THE CAUSE - "five-of-six is systematic, not idiosyncratic; recurring causes
# are exactly what codemod rules should absorb"). The report then says, per
# batch, what the first-compile rate did and which causes recur.
#
# FIRST-compile means FIRST: an attempt after fixing is a different datum.
# Record the initial result and stop - re-recording post-fix successes as
# passes is how a learning metric flatters itself into uselessness.
#
# Store: tools/foundry/kit-batch-log.jsonl (append-only, one JSON per line).
# Batch 1 is seeded as an AGGREGATE (the roadmap's 5/6, per-actor detail not
# recorded at the time) and is labelled as such rather than backfilled.
#
# Usage:
#   kit_batch_rate.py add --batch 2 --actor d_a_obj_X --result fail --cause "missing shim header"
#   kit_batch_rate.py add --batch 2 --actor d_a_obj_Y --result pass
#   kit_batch_rate.py report
#   kit_batch_rate.py --control
# Exit 0 ok - 1 control failed - 2 bad input.
# ============================================================================
import argparse
import json
import sys
import time
from collections import Counter, defaultdict
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

LOG = Path(__file__).resolve().parent / "kit-batch-log.jsonl"

SEED = {"batch": 1, "actor": "AGGREGATE(6 actors, Otble harvest era)",
        "result": "aggregate", "passes": 1, "fails": 5,
        "cause": "per-actor causes not recorded at the time; roadmap E3 baseline",
        "ts": "2026-08-21T00:00:00Z", "seed": True}


def load():
    rows = []
    if LOG.exists():
        for line in LOG.read_text(encoding="utf-8").splitlines():
            if line.strip():
                rows.append(json.loads(line))
    return rows


def trend(rows):
    """batch -> (passes, fails, cause Counter). Pure, for the control."""
    b = defaultdict(lambda: [0, 0, Counter()])
    for r in rows:
        k = b[r["batch"]]
        if r.get("result") == "aggregate":
            k[0] += r["passes"]; k[1] += r["fails"]
        elif r["result"] == "pass":
            k[0] += 1
        else:
            k[1] += 1
            if r.get("cause"):
                k[2][r["cause"]] += 1
    return dict(sorted(b.items()))


def report(rows):
    t = trend(rows)
    print("KIT FIRST-COMPILE TREND - the E3 gate is this table existing")
    prev = None
    for batch, (p, f, causes) in t.items():
        n = p + f
        rate = 100.0 * p / n if n else 0
        arrow = ""
        if prev is not None:
            arrow = "  IMPROVING" if rate > prev else ("  FLAT/WORSE - rules not generalising" if n else "")
        print("  batch %-3s  %d/%d first-compile (%.0f%%)%s" % (batch, p, n, rate, arrow))
        for c, k in causes.most_common():
            print("        cause x%d: %s" % (k, c))
        prev = rate
    if len(t) < 2:
        print("  (one batch: no trend yet - the metric exists, the line needs batch 2)")


def control():
    rows = [SEED,
            {"batch": 2, "actor": "a", "result": "fail", "cause": "missing shim"},
            {"batch": 2, "actor": "b", "result": "fail", "cause": "missing shim"},
            {"batch": 2, "actor": "c", "result": "pass"}]
    t = trend(rows)
    ok = (t[1][1] == 5 and t[2][0] == 1 and t[2][1] == 2
          and t[2][2]["missing shim"] == 2)
    print("NEGATIVE CONTROL - seeded 1/6 baseline + planted batch with a recurring cause")
    report(rows)
    print("  CONTROL %s" % ("PASSED - aggregate seed, per-actor rows and cause "
                            "recurrence all counted." if ok else "*** FAILED ***"))
    return 0 if ok else 1


def main():
    if "--control" in sys.argv:
        return control()
    ap = argparse.ArgumentParser()
    sub = ap.add_subparsers(dest="cmd")
    a = sub.add_parser("add")
    a.add_argument("--batch", type=int, required=True)
    a.add_argument("--actor", required=True)
    a.add_argument("--result", choices=["pass", "fail"], required=True)
    a.add_argument("--cause", default="")
    sub.add_parser("report")
    args = ap.parse_args()
    if args.cmd == "add":
        if args.result == "fail" and not args.cause:
            print("a FAIL with no cause is exactly the datum E3 says is useless - name it")
            return 2
        rows = load()
        if not rows:
            LOG.write_text(json.dumps(SEED) + "\n", encoding="utf-8")
        rec = {"batch": args.batch, "actor": args.actor, "result": args.result,
               "cause": args.cause, "ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime())}
        with LOG.open("a", encoding="utf-8") as fh:
            fh.write(json.dumps(rec) + "\n")
        back = load()[-1]
        print("RECORDED (verified by re-read): %s" % json.dumps(back))
        return 0
    rows = load()
    if not rows:
        LOG.write_text(json.dumps(SEED) + "\n", encoding="utf-8")
        rows = [SEED]
    report(rows)
    return 0


if __name__ == "__main__":
    sys.exit(main())
