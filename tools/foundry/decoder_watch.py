#!/usr/bin/env python3
# ============================================================================
# decoder_watch.py - the DECODER lane monitor. Recorded in-tree at arm time
# per the 2026-08-16 stand-down rule ("record the current version of your
# monitors"); a scratchpad watcher does not survive the session and re-arming
# from memory silently rebuilds v1.
#
#   ARM:  python -u tools/foundry/decoder_watch.py --exit-on-event
#   Then IMMEDIATELY: record the task ID in MONITOR-REGISTRY.md, verify with
#   `monitor_pulse.py tools/foundry/monitor-pulse-decoder.json` (expect ALIVE,
#   new pid), and run `watcher_census.py` (expect this watcher as a leaf).
#
# CHARTER (DECODER-BRIEFING.md S3b.1): interval defaults to 30 s - with a 30 s
# loop, quiet is never ambiguous for more than a minute. The pulse carries
# that interval so Foundry's registry can tell alive-and-working from dead.
# TWO MECHANISMS, DELIBERATELY DISTINCT: this WATCHER delivers inbound
# interrupts (exit-on-event -> harness task-completion notification); the
# TIMER that drives work continuation is the lane itself, not this process.
#
# CARRIES THE ESTATE'S RECORDED REPAIRS, by defect site:
#   1. ONE TOKENIZER over the whole addressee field (`,` and `/`) - the
#      first-addressee anchor blinded Foundry to `HOUSING, FOUNDRY` rows.
#   2. BOTH `[ ]` and `[x]` states - answers flip the box and drop out of an
#      open-only set (History/Bridge measured 60 invisible answered rows).
#      Keyed on the full line hash, so the flip itself surfaces as an event.
#   3. 600-char excerpts, cut marked - a truncated alert reads as whole.
#   4. `--help` must not arm (it did once, and left a false-ALIVE stamp).
#   5. ONE PULSE FILE PER LANE - monitor-pulse-decoder.json, nobody else's.
#   6. EXIT-ON-EVENT - a forever-loop watcher under this harness is a diary:
#      liveness true, coverage true, DELIVERY zero. Exiting is what notifies.
#      Liveness, coverage, and delivery are THREE properties; re-arm after
#      every delivery.
#
# SELF-SUPPRESSION, with the Librarian's exception: rows whose trailing
# source field names DECODER are this lane's own filings and do not wake it -
# UNLESS the addressee field also names DECODER, because a row addressed to
# me and answered by someone else is exactly what I must not miss.
#
# WHAT IT CANNOT SEE (blind spots recorded, not just coverage): replies that
# never touch CALLS.md; the re-arm window (rows landing between stop and
# re-arm are swallowed into the new baseline - enumerate the board by hand
# after any re-arm); anything outside CALLS.md.
#
# ASCII-ONLY SOURCE AND OUTPUT, per the 2026-08-15 middot/BOM incident.
# ============================================================================
import hashlib
import os
import json
import re
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"
PULSE = HERE / "monitor-pulse-decoder.json"

LANE = "DECODER"
COLLECTIVE = {"ALL", "ALL LANES"}
INTERVAL = 30

ROW = re.compile(r"^- \[[ xX]\]\s*(.+?)\s*\|", re.I)
# Trailing source field: `... | DECODER | 2026-08-17` (date optional).
SELF_TAIL = re.compile(r"\|\s*DECODER\s*(\|[^|]*)?$", re.I)


def addressed_to_me(line):
    m = ROW.match(line.strip())
    if not m:
        return False
    tokens = [t.strip().upper() for t in re.split(r"[,/]", m.group(1))]
    return any(t == LANE or t in COLLECTIVE for t in tokens)


def is_own_filing(line):
    m = ROW.match(line.strip())
    if not m:
        return False
    tokens = [t.strip().upper() for t in re.split(r"[,/]", m.group(1))]
    addressed_here = any(t == LANE for t in tokens)
    return bool(SELF_TAIL.search(line.strip())) and not addressed_here


def emit(msg):
    print(msg.encode("ascii", "replace").decode("ascii"), flush=True)


def now_iso():
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def write_pulse(passes, last_event, note):
    try:
        PULSE.write_text(json.dumps({
            "updated": now_iso(),
            "updated_epoch": int(time.time()),
            "pid": os.getpid(),
            "lane": LANE,
            "passes": passes,
            "interval_sec": INTERVAL,
            "last_event": last_event,
            "note": note,
        }, indent=2) + "\n", encoding="utf-8")
    except OSError as e:
        emit("DECODER-WATCH ** CANNOT WRITE PULSE ** %s - liveness is now "
             "unfalsifiable, treat this watcher as untrusted" % e)


def board_rows():
    """CALLS rows addressed to this lane, both states -> {hash: excerpt}."""
    if not CALLS.is_file():
        return None
    rows = {}
    for ln in CALLS.read_text(encoding="utf-8", errors="replace").splitlines():
        if addressed_to_me(ln) and not is_own_filing(ln):
            s = ln.strip()
            key = hashlib.sha1(s.encode("utf-8", "replace")).hexdigest()[:12]
            rows[key] = s[:600] + (
                "  ...[TRUNCATED %d more chars - READ THE ROW IN CALLS.md]"
                % (len(s) - 600) if len(s) > 600 else "")
    return rows


USAGE = """decoder_watch.py - the Decoder lane monitor, with a proof of life.

  decoder_watch.py [--interval SEC] [--pulse PATH] [--exit-on-event]

  --interval SEC     board check cadence (default 30, per DECODER-BRIEFING)
  --pulse PATH       pulse file (default tools/foundry/
                     monitor-pulse-decoder.json). ONE PULSE FILE PER LANE.
  --exit-on-event    exit when an event fires so the harness notifies
                     (the delivery mechanism; re-arm after reading)

Check liveness with `monitor_pulse.py <pulse path>`; record the task ID in
MONITOR-REGISTRY.md. Running this ARMS A WATCHER - it does not exit."""


def main():
    global INTERVAL, PULSE
    if "--help" in sys.argv or "-h" in sys.argv:
        print(USAGE)
        return 0
    if "--interval" in sys.argv:
        INTERVAL = int(sys.argv[sys.argv.index("--interval") + 1])
    if "--pulse" in sys.argv:
        PULSE = Path(sys.argv[sys.argv.index("--pulse") + 1])

    seen = board_rows()
    if seen is None:
        emit("DECODER-WATCH ** CALLS.md NOT READABLE at %s ** - the call "
             "surface is my only trigger; this watcher is blind until fixed"
             % CALLS)
        seen = {}

    emit("DECODER-WATCH ARMED %s | watching %d row(s) (both states) for "
         "DECODER/ALL LANES | interval %ds | pulse: %s"
         % (now_iso(), len(seen), INTERVAL, PULSE.name))
    write_pulse(0, "armed", "startup")

    passes = 0
    while True:
        try:
            time.sleep(INTERVAL)
            passes += 1
            event = None

            rows = board_rows()
            if rows is None:
                event = "CALLS.md unreadable"
                emit("DECODER-WATCH ** CALLS.md UNREADABLE ** my only trigger "
                     "surface is gone")
            else:
                fresh = [v for k, v in rows.items() if k not in seen]
                if fresh:
                    event = "%d new/changed row(s)" % len(fresh)
                    for v in fresh[:4]:
                        emit("DECODER-WATCH ROW | %s" % v)
                    if len(fresh) > 4:
                        emit("DECODER-WATCH (+%d more row(s))"
                             % (len(fresh) - 4))
                seen = rows

            write_pulse(passes, event or "quiet", "running")

            if event and "--exit-on-event" in sys.argv:
                emit("DECODER-WATCH EXITING ON EVENT (%s) after %d pass(es) - "
                     "delivery via task completion; RE-ARM after reading"
                     % (event, passes))
                write_pulse(passes, "exited-on-event: %s" % event, "delivered")
                return 0

        except KeyboardInterrupt:
            emit("DECODER-WATCH stopped (interrupt) after %d pass(es)" % passes)
            write_pulse(passes, "stopped", "interrupt")
            return 0
        except Exception as e:
            emit("DECODER-WATCH ** WATCHER EXCEPTION ** %s: %s"
                 % (type(e).__name__, e))
            write_pulse(passes, "exception: %s" % type(e).__name__, "degraded")
            time.sleep(INTERVAL)


if __name__ == "__main__":
    sys.exit(main())
