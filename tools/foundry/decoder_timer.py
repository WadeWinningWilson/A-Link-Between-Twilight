#!/usr/bin/env python3
# ============================================================================
# decoder_timer.py - the DECODER lane's WORK-CONTINUATION timer, formalized.
#
# Prior sessions ran this as an inline/ad-hoc loop and never committed it -
# a gap against the lane's own recorded rule ("record the current version of
# your monitors" - decoder_watch.py's header, 2026-08-16 stand-down rule).
# integrator_timer.py, engine_timer.py, and foundry_timer.py were all copied
# FROM this lane's timer behavior but the source was never itself in tree.
# This file closes that gap: same shape, DECODER's own always-unconditional
# charter (DECODER-BRIEFING.md S3b.1), no plate file to read because this
# lane's queue is its own progress-campaign anchor (ym1/yw1/etc-progress.md),
# not a checklist file.
#
#   ARM (MONITOR TOOL, NOT BASH):
#       python -u tools/foundry/decoder_timer.py --interval 30
#
# ============================================================================
# WHY MONITOR TOOL, NOT BASH (MONITOR-REGISTRY.md line 592, 2026-08-17 ~11:05Z)
# ============================================================================
# "Root cause of silent timers bakhgc61g/bv7q596b4: plain background Bash
#  only notifies on EXIT - the heartbeat MUST be created with the Monitor
#  tool (stdout lines = events)." A background Bash job that loops forever
# notifies exactly once, at exit - never, for a forever loop. Every stdout
# line here is an event; that is the whole delivery mechanism.
#
# Verify liveness by the PULSE FILE's mtime, never by the arm command's exit
# code - a dead job and a live one both return 0.
#
# ============================================================================
# UNCONDITIONAL BY CHARTER, NOT BY OVERSIGHT
# ============================================================================
# DECODER-BRIEFING.md S3b.1: "If the user has not responded/prompted in the
# interval, the lane continues decoding - same effort, or relocated." The
# decode campaign (ym1 -> yw1 -> ... ) is chosen to never run dry the way a
# lane plate can; "quiet" here means "check the board, then keep decoding
# the open campaign's NEXT queue," not "nothing to do." So this timer never
# self-disarms - it always ticks QUIET-CONTINUE, unlike the plate-gated
# lane timers copied from it.
# ============================================================================
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
import argparse
import json
import os
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
PULSE = os.path.join(ROOT, "tools", "foundry", "monitor-pulse-decoder-timer.json")


def pulse(interval, tick):
    try:
        with open(PULSE, "w", encoding="utf-8") as fh:
            json.dump({
                "lane": "DECODER",
                "kind": "work-continuation-timer",
                "ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "pid": os.getpid(),
                "interval_s": interval,
                "tick": tick,
                "state": "QUIET-CONTINUE",
            }, fh, indent=2)
    except OSError:
        pass  # a pulse-write failure must never kill the heartbeat


def main():
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("--interval", type=int, default=30,
                    help="seconds between ticks (default 30, per charter)")
    ap.add_argument("--max-ticks", type=int, default=0, help="0 = unbounded")
    args = ap.parse_args()

    # --help must not arm. argparse exits before reaching here, on purpose -
    # registry defect #4 ("--help armed a live watcher, left a false-ALIVE
    # stamp") applies to this file too.

    tick = 0
    print("DECODER-TIMER armed interval=%ds (unconditional, no plate)"
          % args.interval, flush=True)
    while True:
        tick += 1
        pulse(args.interval, tick)
        print("DECODER-TICK %d - QUIET-CONTINUE: resume the open campaign's "
              "NEXT queue; re-check CALLS.md for anything addressed here "
              "since the last tick." % tick, flush=True)

        if args.max_ticks and tick >= args.max_ticks:
            print("DECODER-TIMER max-ticks reached, exiting.", flush=True)
            return 0
        time.sleep(args.interval)


if __name__ == "__main__":
    sys.exit(main())
