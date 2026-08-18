#!/usr/bin/env python3
# ============================================================================
# lane_watch.py — THE FOUNDRY LANE MONITOR, AND ITS OWN PROOF OF LIFE.
#
# ---------------------------------------------------------------------------
# WHY THE PULSE FILE EXISTS. The outgoing Foundry's handoff (S4, S5) records
# two facts that together make a bare monitor worthless:
#
#   - "My monitor died and I did not notice. The user caught it."
#   - "`TaskList` DOES NOT SHOW MONITORS. It returned 'No tasks found' while
#      THREE monitors were demonstrably running and firing. You cannot
#      enumerate your own watchers."
#
# **So a dead monitor and a quiet monitor produce the identical observation:
# nothing.** That is the vacuous pass in its purest form - an absence of
# events that cannot come out the other way - and it is why the previous
# instance ran three at once without knowing.
#
# The fix is the lane's own rule applied to itself: every pass writes a
# TIMESTAMP to `monitor-pulse.json`, so silence becomes falsifiable. Ask
# `monitor_pulse.py` whether the watcher is alive; do not infer it from quiet.
#
# ---------------------------------------------------------------------------
# COVERAGE. Silence is not success, so this emits on FAILURE as loudly as on
# news: a gate going blind, an audit that crashes, an unreadable board, an
# unexpected exception. If this process fell over right now, the pulse would
# go stale and `monitor_pulse.py` would say DEAD.
#
# It also emits ONE LINE ON STARTUP. That is deliberate: it demonstrates the
# monitor can produce an event at all, before anything depends on it doing so.
#
# ---------------------------------------------------------------------------
# ASCII-ONLY SOURCE, DELIBERATELY. A bulk `-replace ' <middot> ' -> ' | '` over
# this file on 2026-08-15 hit AUDIT_RE below and turned the separator into a
# REGEX ALTERNATION, which would have made `audit_blind()` fail on every pass
# and this watcher blind to the very thing it exists to report. The same
# command's `Set-Content -Encoding UTF8` also prepended a BOM and the file
# stopped parsing entirely. **Both were invisible in the diff.** Non-ASCII
# punctuation in an instrument buys nothing and costs exactly this.
#
# Usage:  lane_watch.py [--interval SEC] [--audit-every N]
# ============================================================================
import hashlib
import json
import os
import re
import subprocess
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"
PULSE = HERE / "monitor-pulse.json"
PY = sys.executable

# ============================================================================
# ROWS THIS LANE MUST WAKE FOR - AND WHY THIS IS NOT A REGEX ANY MORE.
#
# This was `^- \[ \]\s*(FOUNDRY|ALL LANES|ALL)\b`, anchored on the FIRST
# addressee. **Every row where another lane is named first was invisible**:
# `HOUSING, FOUNDRY`, `HOUSING, HISTORY, FOUNDRY`, `HOUSING, INTEGRATOR,
# FOUNDRY` - three open rows, TWO OF THEM USER-ORDERED, silently unreported
# while the watcher stamped "quiet" 367 times. The user caught it; the pulse
# could not, because the watcher was alive and wrong rather than dead.
#
# **This is the identical defect the LIBRARIAN reported to me in
# `file_row.py` hours earlier** - `field.split("/")[0]`, first token only, so
# `HOUSING/ENGINE` counted as HOUSING alone. I answered that row at length
# about how the failure class generalizes and shipped it in my own watcher in
# the same session. Their fix was the right one and I did not apply it here:
# ONE TOKENIZER over the whole addressee field, splitting on BOTH `,` and `/`.
#
# A liveness check cannot catch this. `monitor_pulse` proves something is
# RUNNING; it cannot prove the thing is LOOKING IN THE RIGHT PLACE. **A silent
# watcher and a blind watcher produce the same output, which is the same
# equivalence the pulse file was built to break - one level up.**
# ============================================================================
LANE = "FOUNDRY"
COLLECTIVE = {"ALL", "ALL LANES"}
# ============================================================================
# BOTH STATES, NOT JUST `[ ]`. Fourth blindness in this watcher in one night,
# and the one that explains the user having to relay calls to me three times.
#
# **AN ANSWER TO ME FLIPS `[ ]` -> `[x]`, which drops the row OUT of an
# open-only set.** So an open-only watcher wakes reliably for my own filings
# and stays silent for every reply. Measured by History/Bridge on their own
# board: open-only 29 rows, both-states 89 - **60 answered rows invisible**.
# The Integrator adds that Housing files 24 of its 25 rows BORN `[x]`, so an
# open-only filter never sees that lane at all.
#
# I answered the Librarian's `split("/")[0]` row about how this failure class
# generalises, fixed the addressee anchor, fixed the truncation, and still had
# the state filter wrong. Each fix was real and none of them was the whole
# defect - "the instrument cannot see it" had three independent causes here.
# ============================================================================
ROW_OPEN = re.compile(r"^- \[[ xX]\]\s*(.+?)\s*\|", re.I)


# ============================================================================
# NARROWED 2026-08-17 BY USER ORDER: "Only if lanes specifically request
# you/send to your lane, otherwise don't waste what precious context you
# have."
#
# COLLECTIVE broadcasts (`ALL LANES` / `ALL`) NO LONGER WAKE THIS LANE.
# Foundry sits at the top and dispenses work; it does not read along with
# every lane's traffic. A row must NAME `FOUNDRY` in its addressee field.
#
# The COLLECTIVE set is deliberately KEPT rather than deleted, so the change
# is one flag rather than a rewrite if the user ever reverses it.
# ============================================================================
WAKE_ON_BROADCAST = False


def addressed_to_me(line):
    m = ROW_OPEN.match(line.strip())
    if not m:
        return False
    tokens = [t.strip().upper() for t in re.split(r"[,/]", m.group(1))]
    if any(t == LANE for t in tokens):
        return True
    return WAKE_ON_BROADCAST and any(t in COLLECTIVE for t in tokens)

# The audit summary line. Matched loosely on purpose: the separator between
# the two counts is a non-ASCII middot in control.py's output, and pinning it
# here is what broke this once already. `.{0,4}` spans it without naming it.
AUDIT_RE = re.compile(r"(\d+) gate\(s\) demonstrated live.{0,4}?(\d+) BLIND")

INTERVAL = 180
AUDIT_EVERY = 10


def emit(msg):
    """One line = one notification. ASCII only; the smoke test under Git Bash
    mangled non-ASCII to `?`, and a garbled alert is a discounted alert."""
    print(msg.encode("ascii", "replace").decode("ascii"), flush=True)


def now_iso():
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def write_pulse(passes, last_event, note):
    """The whole point. A quiet monitor and a dead one differ HERE."""
    try:
        PULSE.write_text(json.dumps({
            "updated": now_iso(),
            "updated_epoch": int(time.time()),
            # PID AND INTERVAL ARE HERE TO EXPOSE DUPLICATES. Two watchers
            # share this one file and stomp each other's stamp; the only way
            # to see that from the outside is that the values FLIP. A stray
            # smoke-test watcher survived its `kill` on 2026-08-15 and ran
            # alongside the armed one - caught solely because the pulse read
            # `interval 2` when the armed monitor uses 180.
            "pid": os.getpid(),
            "passes": passes,
            "interval_sec": INTERVAL,
            "last_event": last_event,
            "note": note,
        }, indent=2) + "\n", encoding="utf-8")
    except OSError as e:
        emit("FOUNDRY-WATCH ** CANNOT WRITE PULSE ** %s - liveness is now "
             "unfalsifiable, treat this watcher as untrusted" % e)


def open_rows():
    """Open CALLS rows addressed to this lane -> {hash: first 150 chars}."""
    if not CALLS.is_file():
        return None
    rows = {}
    for ln in CALLS.read_text(encoding="utf-8", errors="replace").splitlines():
        if addressed_to_me(ln):
            key = hashlib.sha1(ln.strip().encode("utf-8",
                                                 "replace")).hexdigest()[:12]
            # ============================================================
            # 600, NOT 150. The Integrator diagnosed this from the outside:
            # "your watcher truncates like Housing's did - read the FILE,
            # not this row". A 5,677-character roadmap arrived as its own
            # title and I skipped it, because the excerpt ended before the
            # ask did. **The row FIRED correctly; the notification was
            # useless.** Detection and legibility are separate failures and
            # this instrument had both in one night.
            # A truncated alert still gets read as the whole alert, so the
            # cut is now marked rather than silent.
            # ============================================================
            s = ln.strip()
            rows[key] = s[:600] + (
                "  ...[TRUNCATED %d more chars - READ THE ROW IN CALLS.md]"
                % (len(s) - 600) if len(s) > 600 else "")
    return rows


def audit_blind():
    """(blind_count, detail) from control.py audit. None on failure to run."""
    try:
        r = subprocess.run([PY, str(HERE / "control.py"), "audit"],
                           capture_output=True, text=True, encoding="utf-8",
                           errors="replace", timeout=600, cwd=str(REPO))
    except Exception as e:
        return None, "audit could not run: %s" % e
    out = (r.stdout or "") + (r.stderr or "")
    m = AUDIT_RE.search(out)
    if not m:
        return None, "audit output unparseable (tool changed or crashed)"
    names = re.findall(r"\[BLIND\] (\w+)", out)
    return int(m.group(2)), ", ".join(names)


USAGE = """lane_watch.py — the Foundry lane monitor, with a proof of life.

  lane_watch.py [--interval SEC] [--audit-every N] [--pulse PATH]

  --interval SEC     board check cadence (default 180)
  --audit-every N    run control.py audit every N passes (default 10)
  --pulse PATH       this lane's pulse file (default tools/foundry/
                     monitor-pulse.json). ONE PULSE FILE PER LANE.

Check liveness with `monitor_pulse.py [PATH]`; record the task ID in
MONITOR-REGISTRY.md. Running this ARMS A WATCHER — it does not exit."""


def main():
    global INTERVAL, AUDIT_EVERY, PULSE

    # ====================================================================
    # `--help` MUST NOT ARM. Added 2026-08-15 because it did. Another lane
    # ran `lane_watch.py --help` to discover the options; there was no help
    # handling, so main() ran, armed a watcher, and STAMPED THIS LANE'S
    # PULSE FILE with its own pid and passes=0. `timeout` then killed it.
    #
    # **That leaves a fresh stamp written by a DEAD process, so the pulse
    # reads ALIVE when nothing is watching** — a false ALIVE in the very
    # instrument built to make silence falsifiable. The vacuous pass, in
    # the tool against vacuous passes.
    #
    # Two fixes: this one removes the cause, and `monitor_pulse.py` now
    # verifies the stamping pid is still alive, which removes the harm for
    # any other way a stale stamp gets written.
    # ====================================================================
    if "--help" in sys.argv or "-h" in sys.argv:
        print(USAGE)
        return 0
    if "--interval" in sys.argv:
        INTERVAL = int(sys.argv[sys.argv.index("--interval") + 1])
    if "--audit-every" in sys.argv:
        AUDIT_EVERY = int(sys.argv[sys.argv.index("--audit-every") + 1])
    # ====================================================================
    # ONE PULSE FILE PER LANE — `--pulse <path>`. Added 2026-08-15 after the
    # LIBRARIAN caught the flaw in my design: I hardcoded a single shared
    # pulse, and a second lane's watcher stamping it would overwrite this
    # one's pid and interval on EVERY pass. The duplicate detector would
    # then fire continuously, and a correctly-armed pair would be
    # indistinguishable from a stray.
    # **A permanent false positive destroys the signal as thoroughly as a
    # permanent false negative** — the same lesson as a gate that reports
    # DIVERGENT for everything. The shared-file stomp test still holds
    # WITHIN a lane, which is where duplicates actually happen.
    # Caught by another lane, not by me. That keeps being the pattern.
    # ====================================================================
    if "--pulse" in sys.argv:
        PULSE = Path(sys.argv[sys.argv.index("--pulse") + 1])

    seen = open_rows()
    if seen is None:
        emit("FOUNDRY-WATCH ** CALLS.md NOT READABLE at %s ** - the call "
             "surface is my only trigger; this watcher is blind until fixed"
             % CALLS)
        seen = {}

    # STARTUP EVENT: proves the stream works before anything relies on it.
    emit("FOUNDRY-WATCH ARMED %s | watching %d open row(s) for FOUNDRY/ALL "
         "LANES | gate audit every %d passes (%ds) | pulse: %s"
         % (now_iso(), len(seen), AUDIT_EVERY, INTERVAL * AUDIT_EVERY,
            PULSE.name))
    write_pulse(0, "armed", "startup")

    passes = 0
    last_blind = 0
    while True:
        try:
            time.sleep(INTERVAL)
            passes += 1
            event = None

            rows = open_rows()
            if rows is None:
                event = "CALLS.md unreadable"
                emit("FOUNDRY-WATCH ** CALLS.md UNREADABLE ** my only trigger "
                     "surface is gone")
            else:
                fresh = [v for k, v in rows.items() if k not in seen]
                if fresh:
                    event = "%d new row(s)" % len(fresh)
                    for v in fresh[:4]:
                        emit("FOUNDRY-WATCH NEW CALLS ROW | %s" % v)
                    if len(fresh) > 4:
                        emit("FOUNDRY-WATCH (+%d more new row(s))"
                             % (len(fresh) - 4))
                seen = rows

            if passes % AUDIT_EVERY == 0:
                blind, detail = audit_blind()
                if blind is None:
                    event = event or "audit failed"
                    emit("FOUNDRY-WATCH ** GATE AUDIT FAILED ** %s - an audit "
                         "that cannot run is not a clean audit" % detail)
                elif blind > 0:
                    event = event or "%d blind gate(s)" % blind
                    emit("FOUNDRY-WATCH ** %d GATE(S) WENT BLIND ** %s - every "
                         "pass from those gates is worthless until fixed"
                         % (blind, detail))
                elif last_blind > 0:
                    event = event or "gates recovered"
                    emit("FOUNDRY-WATCH gates recovered - 0 blind")
                last_blind = blind or 0

            write_pulse(passes, event or "quiet", "running")

            # ================================================================
            # EXIT-ON-EVENT (defect 6, 2026-08-16): under a harness that
            # notifies on TASK COMPLETION, a forever-loop watcher writing to
            # an output file is a DIARY - liveness true, coverage true,
            # DELIVERY zero (the three-properties section; History's watcher
            # ran 56 passes into an unread file, and this one then ran ~30
            # new rows into one THE SAME DAY). Exiting is what interrupts:
            # the process ends, the harness notifies, the lane reads the
            # event and re-arms. --exit-on-event is opt-in so terminal users
            # who watch the stream keep the long-running form.
            # ================================================================
            if event and "--exit-on-event" in sys.argv:
                emit("FOUNDRY-WATCH EXITING ON EVENT (%s) after %d pass(es) - "
                     "delivery via task completion; RE-ARM after reading"
                     % (event, passes))
                write_pulse(passes, "exited-on-event: %s" % event, "delivered")
                return 0

        except KeyboardInterrupt:
            emit("FOUNDRY-WATCH stopped (interrupt) after %d pass(es)" % passes)
            write_pulse(passes, "stopped", "interrupt")
            return 0
        except Exception as e:
            # NEVER die silently - a silent death is indistinguishable from a
            # quiet board, which is the whole failure this file exists for.
            emit("FOUNDRY-WATCH ** WATCHER EXCEPTION ** %s: %s"
                 % (type(e).__name__, e))
            write_pulse(passes, "exception: %s" % type(e).__name__, "degraded")
            time.sleep(INTERVAL)


if __name__ == "__main__":
    sys.exit(main())
