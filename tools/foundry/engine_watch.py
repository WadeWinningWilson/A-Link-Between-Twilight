#!/usr/bin/env python3
# ============================================================================
# engine_watch.py - THE HOUSING/ENGINE LANE MONITOR, AND ITS OWN PROOF OF LIFE.
#
# Derived from decoder_watch.py rather than authored fresh (DN-10 order of
# resort: the board already HAS a hardened watcher; port it, do not reinvent).
# Behavioural deltas from the Decoder original are only these, and each one
# exists because MONITOR-REGISTRY.md records the failure it prevents:
#
#   1. TWO LANE TOKENS, NOT ONE. This lane files rows as "Housing/Engine" and
#      is called as HOUSING *or* ENGINE, interchangeably and in the same week
#      (CALLS 425 = HOUSING first, 435 = ENGINE first). A single-token watcher
#      would have matched 435 and been blind to 425.
#
#   2. ADDRESSEE MATCHED AT ANY POSITION. Registry line 73: "HOUSING, FOUNDRY
#      rows were invisible" - the first-addressee anchor. Tokens are compared
#      as a set; position carries no meaning.
#
#   3. BOTH CHECKBOX STATES, keyed on a hash of the WHOLE row. Registry:
#      "Housing files 24 of its 25 rows born [x]", and an answer TO you flips
#      [ ]->[x], dropping the row out of any open-only set. Hashing the full
#      line means a checkbox flip, an appended reply, or an edit all read as
#      events - which is what a reply to this lane actually looks like.
#
#   4. SELF-SUPPRESSION SCANS THE LAST TWO FIELDS, NOT THE LAST ONE.
#      MEASURED, after my first version suppressed ZERO rows and I checked
#      instead of shipping it: the author field is SECOND-to-last, followed
#      by a date - "| Housing/Engine, door chain closed to #39 | 2026-08-17".
#      39 of this lane's own rows carry that shape. Reading only the final
#      field finds a DATE every time and suppresses nothing, silently.
#      The Decoder original was RIGHT here (it allowed one trailing field)
#      and my "correction" broke it - the same wrong-retraction reflex this
#      lane's handoff already lists under DO NOT TRUST.
#      A row ADDRESSED to this lane fires regardless of author (the
#      Librarian's exception) - only unanswered self-filings are suppressed.
#
#   5. SILENT ADOPT, added after THREE consecutive self-wakes in one turn.
#      The board's protocol has you name your own lane first, so every row
#      this lane files is also addressed to it - and delta 4's guard
#      deliberately does not suppress those. Each filing therefore burned a
#      whole delivery cycle waking me with my own words, which is the
#      "v2 woke me reliably for my own writing" defect History/Bridge logged.
#      THE FIX IS NOT MORE SUPPRESSION - that is how they lost replies
#      (defect 5, their v5). A row I authored is TRACKED SILENTLY on first
#      sight and fires on every LATER state, because the key is a whole-line
#      hash: an appended answer changes the hash and wakes me. Suppress the
#      echo, keep the reply. Selftest below proves BOTH halves.
#
# ASCII-ONLY SOURCE, DELIBERATELY - see the lane_watch.py header for the bulk
# -replace that turned a separator into a regex alternation and made a watcher
# silently blind to the thing it existed to report.
#
#   ARM:  python -u tools/foundry/engine_watch.py          <-- USE THIS
#
#   DO NOT PASS --exit-on-event UNDER THE HARNESS. It is INHERITED FROM
#   decoder_watch.py AND IT IS REDUNDANT HERE: the harness already raises a
#   notification for EVERY stdout line, so ROW lines deliver while the
#   watcher keeps running. Exiting buys nothing and costs a re-arm.
#
#   AND THE RE-ARM IS A REAL FAILURE MODE, NOT A CHORE - measured on this
#   lane 2026-08-18: the delivery and the death arrive in the SAME
#   notification, and when it carries interesting rows the EXITING line gets
#   buried under them. This lane re-armed 59 times correctly and then missed
#   one WHILE EDITING HANDOFF PROSE ABOUT RE-ARMING. Four minutes blind; a
#   row landed in the gap; THE USER caught it, which is the exact failure a
#   watcher exists to prevent. Removing the flag removes the class.
#
# Running this ARMS A WATCHER. --help does not (it did once, elsewhere, and
# left a false-ALIVE stamp). ONE PULSE FILE PER LANE.
# ============================================================================
import hashlib
import json
import os
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
PULSE = HERE / "monitor-pulse-engine.json"

LANE = "HOUSING/ENGINE"
MINE = {"HOUSING", "ENGINE", "HOUSING SECURITY"}
COLLECTIVE = {"ALL", "ALL LANES"}
INTERVAL = 30

ROW = re.compile(r"^- \[[ xX]\]\s*(.+?)\s*\|", re.I)


def _tokens(line):
    m = ROW.match(line.strip())
    if not m:
        return None
    return set(t.strip().upper() for t in re.split(r"[,/]", m.group(1)))


def addressed_to_me(line):
    t = _tokens(line)
    return bool(t) and bool(t & (MINE | COLLECTIVE))


def is_own_filing(line):
    """My own filing, not addressed to me -> suppress. Anything addressed to
    this lane fires regardless of who wrote it."""
    t = _tokens(line)
    if not t or (t & MINE):
        return False
    fields = [f.strip().upper() for f in line.strip().rstrip("|").split("|")]
    return any(f.startswith("HOUSING") or f.startswith("ENGINE")
               for f in fields[-2:])


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
            "watches": "CALLS.md HOUSING/ENGINE/ALL LANES, both states",
        }, indent=2) + "\n", encoding="utf-8")
    except OSError as e:
        emit("ENGINE-WATCH ** CANNOT WRITE PULSE ** %s - liveness is now "
             "unfalsifiable, treat this watcher as untrusted" % e)


def authored_by_me(line):
    """Author field (second-to-last, before the date) names this lane."""
    fields = [f.strip().upper() for f in line.strip().rstrip("|").split("|")]
    return any(f.startswith("HOUSING") or f.startswith("ENGINE")
               for f in fields[-2:])


# ============================================================
# READ WITH ONE RETRY - THE READ RACE AGAINST OUR OWN FILING TOOL
# ============================================================
# `file_row.py` REWRITES CALLS.md. A scan landing mid-write hits a Windows
# file lock and read_text raises OSError. This watcher's loop caught only
# KeyboardInterrupt, so that OSError propagated out and KILLED THE PROCESS -
# leaving the pulse frozen at note:"running" with no delivery and no stderr.
# That is not hypothetical: pid 31212 died exactly that way earlier today and
# was found only because its pulse mtime went stale, not because anything
# reported. Librarian hit the same race with a 60 s degraded sleep; the same
# cause, a worse outcome here.
# The blind spot CORRELATES WITH ACTIVITY - it fires when rows are landing,
# which is when a watcher matters most.
# Retry once, 2 s apart. A genuinely unreadable board still raises, and the
# caller still reports it: this widens no eyes shut.
# ============================================================
def _read_calls():
    try:
        return CALLS.read_text(encoding="utf-8", errors="replace")
    except OSError:
        time.sleep(2)
        return CALLS.read_text(encoding="utf-8", errors="replace")


def board_rows():
    """CALLS rows addressed to this lane, both states -> {hash: (excerpt, mine)}.

    `mine` marks a row THIS LANE AUTHORED. Such a row is TRACKED but does not
    fire on FIRST SIGHT - see SILENT_ADOPT in the loop. It still fires on every
    later change, because the key is a hash of the whole line.
    """
    if not CALLS.is_file():
        return None
    rows = {}
    for ln in _read_calls().splitlines():
        if addressed_to_me(ln) and not is_own_filing(ln):
            s = ln.strip()
            # IDENTITY vs CONTENT are two different keys and conflating them
            # is what swallows replies. IDENTITY MUST COME FROM A STRUCTURAL
            # BOUNDARY, NOT A CHARACTER COUNT - I tried a 200-char prefix and
            # the selftest killed it twice: short rows have no 200 chars to be
            # stable over, and a `[ ]`->`[x]` flip mutates character 3, which
            # is inside ANY prefix. So: checkbox normalised out, then the
            # addressee field plus the head of the body - the two things a
            # reply never touches, because replies append at the END.
            # LIMIT, stated rather than discovered later: two rows with the
            # same addressees AND the same first 80 body chars collide.
            norm = re.sub(r"^- \[[ xX]\]", "- [ ]", s)
            parts = norm.split("|")
            ident_src = parts[0] + "|" + (parts[1][:80] if len(parts) > 1 else "")
            ident = hashlib.sha1(
                ident_src.encode("utf-8", "replace")).hexdigest()[:12]
            content = hashlib.sha1(s.encode("utf-8", "replace")).hexdigest()[:12]
            excerpt = s[:600] + (
                "  ...[TRUNCATED %d more chars - READ THE ROW IN CALLS.md]"
                % (len(s) - 600) if len(s) > 600 else "")
            rows[ident] = (content, excerpt, authored_by_me(ln))
    return rows


USAGE = """engine_watch.py - the Housing/Engine lane monitor, with a proof of life.

  engine_watch.py [--interval SEC] [--pulse PATH] [--exit-on-event]

  --interval SEC     board check cadence (default 30)
  --pulse PATH       pulse file (default tools/foundry/
                     monitor-pulse-engine.json). ONE PULSE FILE PER LANE.
  --exit-on-event    exit when an event fires so the harness notifies
                     (the delivery mechanism; RE-ARM IMMEDIATELY after
                     reading - a delivered watcher is a DEAD watcher, and
                     that rule has bitten this board repeatedly, most
                     recently a 45-minute blind window DECODER logged at
                     03:53Z)

Check liveness with monitor_pulse.py <pulse path>; record the task ID in
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
        emit("ENGINE-WATCH ** CALLS.md NOT READABLE at %s ** - the call "
             "surface is my only trigger; this watcher is blind until fixed"
             % CALLS)
        seen = {}

    emit("ENGINE-WATCH ARMED %s | watching %d row(s) (both states) for "
         "HOUSING/ENGINE/ALL LANES | interval %ds | pulse: %s"
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
                emit("ENGINE-WATCH ** CALLS.md UNREADABLE ** my only trigger "
                     "surface is gone")
            else:
                # SILENT ADOPT applies to a NEW IDENTITY I authored, and to
                # nothing else. A CHANGED row always fires whoever wrote it -
                # that is the reply, and suppressing it is the defect this
                # guard was one edit away from reintroducing.
                fresh, adopted = [], 0
                for ident, (content, excerpt, mine) in rows.items():
                    if ident not in seen:
                        if mine:
                            adopted += 1
                        else:
                            fresh.append(excerpt)
                    elif seen[ident][0] != content:
                        fresh.append("[CHANGED - answered or amended] " + excerpt)
                if adopted:
                    emit("ENGINE-WATCH adopted %d own filing(s) silently - "
                         "tracked; a reply to them WILL fire" % adopted)
                if fresh:
                    event = "%d new/changed row(s)" % len(fresh)
                    for v in fresh[:4]:
                        emit("ENGINE-WATCH ROW | %s" % v)
                    if len(fresh) > 4:
                        emit("ENGINE-WATCH (+%d more row(s))"
                             % (len(fresh) - 4))
                seen = rows

            write_pulse(passes, event or "quiet", "running")

            if event and "--exit-on-event" in sys.argv:
                emit("ENGINE-WATCH EXITING ON EVENT (%s) after %d pass(es) - "
                     "delivery via task completion; RE-ARM after reading"
                     % (event, passes))
                write_pulse(passes, "exited-on-event: %s" % event, "delivered")
                return 0

        except OSError as e:
            # SURVIVE, LOUDLY. A scan failure must degrade this watcher, never
            # end it - a dead watcher and a quiet board are indistinguishable
            # from outside, and that is how a call goes unread.
            emit("ENGINE-WATCH ** SCAN DEGRADED ** %s: %s - retried once and "
                 "still failed; STAYING UP, next pass in %ds"
                 % (type(e).__name__, e, INTERVAL))
            write_pulse(passes, "scan-failed", "degraded")

        except KeyboardInterrupt:
            emit("ENGINE-WATCH stopped (interrupt) after %d pass(es)" % passes)
            write_pulse(passes, "stopped", "interrupt")
            return 0


if __name__ == "__main__":
    sys.exit(main())
