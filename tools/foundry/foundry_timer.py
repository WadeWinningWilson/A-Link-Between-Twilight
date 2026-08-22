#!/usr/bin/env python3
# ============================================================================
# foundry_timer.py - the FOUNDRY lane's WORK-CONTINUATION timer.
#
# FOUNDRY copy of the Integrator timer, on the user's order 2026-08-21
# ("make a timer similar to decoder's that wakes you up to do your own passes").
# Lineage: Decoder -> Integrator (2026-08-17) ("copy decoder's
# timer so that you can retrigger yourself so that any work on your plate
# starts without the user prompting").
#
#   ARM (MONITOR TOOL, NOT BASH):
#       python -u tools/foundry/foundry_timer.py --interval 180
#
# ============================================================================
# THE ONE DEFECT THIS FILE EXISTS TO NOT REPEAT
# ============================================================================
# MONITOR-REGISTRY.md line 592, recorded 2026-08-17 ~11:05Z:
#
#   "Root cause of silent timers bakhgc61g/bv7q596b4: plain background Bash
#    only notifies on EXIT - the heartbeat MUST be created with the Monitor
#    tool (stdout lines = events)."
#
# TWO Decoder timers were armed, reported success, and delivered NOTHING. A
# background Bash job that ticks forever notifies exactly once, at exit, which
# for a forever-loop is never. THIS SCRIPT MUST BE ARMED WITH THE MONITOR TOOL.
# Every stdout line is an event; that is the whole delivery mechanism.
#
# And per the estate's standing rule: verify liveness by the PULSE FILE's
# mtime, never by the arm command's exit code. A dead job and a live one both
# return 0.
#
# ============================================================================
# THE REFINEMENT DECODER'S TIMER DOES NOT CARRY
# ============================================================================
# Decoder's timer ticks UNCONDITIONALLY. History/Bridge's row in the registry
# records why that is not quite right:
#
#   "plate EMPTY, so the timed self-wake is DISARMED by design ... a monitor
#    that wakes with nothing to do is the one failure a self-waking design
#    cannot tune its way out of."
#
# So this timer reads a PLATE FILE and stops itself when the plate is empty.
# A tick that wakes the lane with nothing to do trains the lane to ignore
# ticks, and an ignored heartbeat is worse than no heartbeat.
#
# The plate is a plain checklist: lines starting "- [ ]" are OPEN, "- [x]" are
# DONE. It is the lane's own file; the harness task list is not readable from
# here, which is exactly why the plate is written down instead of assumed.
# ============================================================================
#
# ============================================================================
# KNOWN GAP, found by using it (2026-08-17, tick 28)
# ============================================================================
# This timer disarms on an EMPTY plate. It does NOT detect a plate whose items
# are all BLOCKED — on the user, on a boot, on a decision that is not this
# lane's to take. At tick 28 every open item was blocked and it kept firing,
# which is precisely the "wakes the lane with nothing to do" failure the
# History/Bridge refinement above was written to prevent. The refinement was
# right and my implementation of it was too narrow: I checked for ZERO items
# instead of zero ACTIONABLE items.
#
# The fix is a blocked-marker the plate can carry (e.g. a "- [~]" state, or a
# "BLOCKED:" prefix) that this reader treats as not-actionable, disarming when
# open == blocked. Deliberately NOT implemented here: changing the plate format
# mid-session would silently reclassify every existing row, and this file's
# whole purpose is to not do that class of thing quietly.
# ============================================================================
import sys
# ============================================================================
# CP1252 GUARD, added 2026-08-21 after this timer DIED ON ITS OWN PLATE: a
# U+2194 arrow in a plate item reached print() on a cp1252 console and the
# heartbeat crashed at tick 4 -- the EXACT failure file_row.py already
# documents ("the work happened, the report died"). The plate is DATA; the
# timer must survive whatever bytes the plate carries.
# ============================================================================
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
import argparse
import json
import os
import sys
import time

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
DEFAULT_PLATE = os.path.join(ROOT, "tools", "foundry", "FOUNDRY-PLATE.md")
PULSE = os.path.join(ROOT, "tools", "foundry", "monitor-pulse-foundry-timer.json")


def read_plate(path):
    """Return (open_items, done_count). Missing plate is NOT treated as empty:
    a vanished file must look like a fault, not like 'nothing to do'."""
    if not os.path.exists(path):
        return None, 0
    open_items, done = [], 0
    with open(path, "r", encoding="utf-8", errors="replace") as fh:
        for line in fh:
            s = line.strip()
            if s.startswith("- [ ]"):
                open_items.append(s[5:].strip())
            elif s.startswith("- [x]") or s.startswith("- [X]"):
                done += 1
    return open_items, done


def pulse(interval, tick, state, top):
    try:
        with open(PULSE, "w", encoding="utf-8") as fh:
            json.dump({
                "lane": "FOUNDRY",
                "kind": "work-continuation-timer",
                "ts": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
                "pid": os.getpid(),
                "interval_s": interval,
                "tick": tick,
                "state": state,
                "top": top,
            }, fh, indent=2)
    except OSError:
        pass  # a pulse-write failure must never kill the heartbeat


def main():
    ap = argparse.ArgumentParser(add_help=True)
    ap.add_argument("--interval", type=int, default=180,
                    help="seconds between ticks (default 180)")
    ap.add_argument("--plate", default=DEFAULT_PLATE)
    ap.add_argument("--max-ticks", type=int, default=0, help="0 = unbounded")
    args = ap.parse_args()

    # --help must not arm. argparse exits before reaching here, which is the
    # point: registry defect #4, "--help armed a live watcher, and left a
    # false-ALIVE stamp".

    tick = 0
    print("FOUNDRY-TIMER armed interval=%ds plate=%s"
          % (args.interval, os.path.relpath(args.plate, ROOT)), flush=True)
    while True:
        tick += 1
        open_items, done = read_plate(args.plate)

        if open_items is None:
            pulse(args.interval, tick, "PLATE-MISSING", None)
            print("FOUNDRY-TICK %d - PLATE FILE MISSING (%s). Not treating as empty; "
                  "this is a fault, not quiet." % (tick, args.plate), flush=True)
        elif not open_items:
            pulse(args.interval, tick, "EMPTY-DISARMED", None)
            print("FOUNDRY-TIMER disarming: plate EMPTY after %d done. A timer that "
                  "wakes the lane with nothing to do is the one failure this design "
                  "cannot tune its way out of. Re-add a plate item and re-arm."
                  % done, flush=True)
            return 0
        else:
            pulse(args.interval, tick, "OPEN", open_items[0])
            print("FOUNDRY-TICK %d - %d open, %d done. NEXT: %s"
                  % (tick, len(open_items), done, open_items[0]), flush=True)

        if args.max_ticks and tick >= args.max_ticks:
            print("FOUNDRY-TIMER max-ticks reached, exiting.", flush=True)
            return 0
        time.sleep(args.interval)


if __name__ == "__main__":
    sys.exit(main())
