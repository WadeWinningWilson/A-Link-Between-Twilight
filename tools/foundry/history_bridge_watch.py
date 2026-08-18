#!/usr/bin/env python3
# ============================================================================
# history_bridge_watch.py — the HISTORY/BRIDGE lane monitor, v3.
#
# RECORDED INTO THE TREE 2026-08-16 on the user's stand-down order:
#   "record the current version of your monitors so that they may perform at
#    the same quality tomorrow."
#
# Until now this ran from the SESSION SCRATCHPAD — a temp directory that does
# not survive the session. Tomorrow's instance would have re-armed from memory
# and rebuilt v1, silently losing both repairs below. THAT is why the artifact
# and not a description had to be recorded.
#
#   ARM:  python -u tools/foundry/history_bridge_watch.py
#   Register the task ID in tools/foundry/MONITOR-REGISTRY.md. An unrecorded
#   monitor is an unkillable one; `TaskList` cannot enumerate monitors.
#
# ---------------------------------------------------------------------------
# WHAT IT TRIGGERS ON
#   · a CALLS row whose ADDRESSEE FIELD names HISTORY, BRIDGE, ALL LANES or ALL
#     — in BOTH `[ ]` and `[x]` states
#   · a STATE CHANGE on such a row: `[ ]`->`[x]` is AN ANSWER TO ME, the single
#     most important event this lane receives
#   · CALLS.md going missing, an unwritable pulse, any unexpected exception
#
# WHAT IT DELIBERATELY IGNORES
#   · rows THIS LANE AUTHORED (trailing source field carries "History/Bridge")
#     — UNLESS the addressee names HISTORY or BRIDGE, because a row addressed
#     to me answered by someone else is exactly what I must not miss
#
# ⚠ WHAT IT CANNOT SEE — record the blind spots, not just the coverage:
#   · a reply that never touches CALLS.md. The Integrator answered a lane once
#     via a registry.cpp edit plus a BUILD-QUEUE revision and this class of
#     watcher is blind to it. It does NOT watch BUILD-QUEUE.md or source.
#   · the RE-ARM WINDOW. It re-baselines on arm, so a row landing between a
#     stop and the next arm is swallowed into the new baseline. The Integrator's
#     stronger pattern is to snapshot the open-row set BEFORE stopping and
#     baseline from that file. Not implemented here; enumerate the board by
#     hand after any re-arm and say which method you used.
#   · anything outside CALLS.md entirely.
#
# ---------------------------------------------------------------------------
# DEFECTS REPAIRED TONIGHT — the part that is silently lost on a rebuild:
#
#  (1) v1 HAD NO PROOF OF LIFE. Between events its silence was not falsifiable:
#      a dead watcher and a quiet board produce identical output. FIX: stamp a
#      pulse file EVERY pass; `monitor_pulse.py <path>` reads its age.
#      LIMIT, learned from Foundry the same night: the pulse proves the watcher
#      is RUNNING, never that it is LOOKING IN THE RIGHT PLACE. Theirs stamped
#      `quiet` 367 times while blind. A liveness check is not a coverage check.
#
#  (2) v2 FILTERED ON `^- [ ]` ONLY, AND THAT WAS EXACTLY BACKWARDS.
#      An ANSWER to one of my rows flips `[ ]`->`[x]` and DROPS OUT of an
#      open-only set, while my own new filings appear as open rows. So v2 woke
#      me reliably for my own writing and stayed SILENT for every reply.
#      MEASURED BEFORE AND AFTER on this lane's own board: 29 rows -> 89.
#      **60 answered rows were invisible.** FIX: track both states, key rows on
#      addressee+body (survives the checkbox flip), and emit on STATE CHANGE.
#
#  (3) Self-suppression added with the Librarian's exception intact — see
#      "WHAT IT DELIBERATELY IGNORES". Without it this lane wakes itself on
#      every row it files, which is noise that trains you to skim.
#
#  NOT A DEFECT THIS LANE HAD, recorded because three other lanes did: the
#  addressee match here has ALWAYS used the WHOLE field up to the first `|`,
#  not the first token. First-addressee anchoring made another lane's watcher
#  blind for its entire run (13 -> 34 rows on the fix). Rows arrive as
#  "HOUSING, HISTORY" and "HOUSING / ALL LANES"; anchoring on the first name
#  misses them.
# ============================================================================
import hashlib
import json
import os
import re
import sys
import time
from datetime import datetime, timezone
from pathlib import Path

HERE = Path(__file__).resolve().parent
REPO = HERE.parents[1]
CALLS = REPO / "docs" / "state" / "ww-staging" / "CALLS.md"
PULSE = HERE / "history-bridge-pulse.json"
PLATE = HERE / "history-bridge-plate.md"
INTERVAL = 50
# Timed self-wake while the plate has open items. USER-SET 2026-08-16: 100s.
# My first cut was 900s and the user's correction was blunt — "way too long".
# They are right about the cost model: a wake that finds nothing costs one cheap
# check, while a plate item sitting idle costs REAL TIME, and those are not
# symmetric. Bias toward waking.
#
# ⚠ INTERVAL IS COUPLED AND WAS THE ACTUAL BUG RISK HERE: the wake check runs
# only after each scan, so PLATE_WAKE_SEC can never resolve finer than INTERVAL.
# Setting 100 while INTERVAL stayed 180 would have SILENTLY produced a 180s wake
# and reported 100 in the banner — the config lying about itself. INTERVAL is now
# 50 so 100s lands on pass 2. Keep INTERVAL <= PLATE_WAKE_SEC/2.
PLATE_WAKE_SEC = 100

ROW = re.compile(r"^- \[([ xX])\]([^|]*)\|")
MINE = re.compile(r"\b(HISTORY|BRIDGE|ALL LANES|ALL|DECODER)\b", re.I)
# ---------------------------------------------------------------------------
# (6) SELF-SUPPRESSION WAS A SUBSTRING SEARCH OVER A FIXED TAIL WINDOW, AND IT
# WAS WRONG IN BOTH DIRECTIONS. Measured over all 361 rows on the live board:
#
#   ·  9 FALSE POSITIVES - rows authored by FOUNDRY and HOUSING SECURITY whose
#      text merely MENTIONS "History/Bridge" (e.g. "Foundry, withdrawing the
#      node-0 comparand - History/Bridge's box and INF signature are correct").
#      These were SUPPRESSED AS MINE. **A row ABOUT this lane, written BY
#      another lane, is exactly the row that must never be swallowed** - and
#      being about me is what made it look like it was from me.
#   · 28 FALSE NEGATIVES - rows this lane genuinely authored, MISSED because
#      `s[-260:]` never reached the source field on a long row. Those woke me
#      on my own filings, the precise noise self-suppression exists to stop.
#
# ROOT: authorship is a FIELD, not a substring. The row format is
# `- [ ] LANES | BODY | SOURCE, note | DATE`, so the author is the LEADING NAME
# of the second-to-last pipe field. Anchor there and both error classes vanish.
# An unparseable/absent source field yields NOT-MINE, which errs toward
# DELIVERING - the correct failure direction for a monitor.
# ---------------------------------------------------------------------------
AUTHORED = re.compile(r"^\s*History\s*/\s*Bridge\b", re.I)

# ---------------------------------------------------------------------------
# (5) THE BROADCAST BLIND SPOT — found by the USER, not by this watcher, and not
# by its pulse (which said ALIVE, and ALIVE was true; that is defect 4's lesson
# repeating in a new place). v4 delivered ONLY on a lane field naming
# HISTORY/BRIDGE, treating ALL / ALL LANES as informational. That was CORRECT
# when eight lanes broadcast constantly and exiting on every broadcast would
# have restored the noise self-suppression exists to stop.
#
# IT STOPPED BEING CORRECT WHEN EVERY OTHER LANE RETIRED. DECODER is now the
# sole active counterpart and this lane is its designated reviewer — and Decoder
# addresses its rows `[ALL LANES, DECODER]`. So its ob1 milestone, its 12/12
# claim AND its 12/12 retraction were all detected, logged, and NOT DELIVERED.
# The one row that woke this lane was the single one that happened to name
# HISTORY explicitly. The reviewer was structurally blind to the reviewee.
#
# FIX: a lane field naming DECODER now forces delivery too. Self-suppression
# still applies unchanged — rows THIS lane authors carry History/Bridge in their
# source field, so answering my own Decoder-addressed row does not wake me.
#
# THE GENERAL LESSON, which is the part worth keeping: A DELIVERY RULE ENCODES
# AN ASSUMPTION ABOUT WHO IS TALKING. When the roster changed, the rule silently
# became a filter against exactly the traffic it existed to catch. Re-read the
# addressee assumptions whenever the lane roster changes.
# ---------------------------------------------------------------------------
FROM_DECODER = re.compile(r"\bDECODER\b", re.I)


def plate_open():
    """Open `- [ ]` items on THIS LANE'S OWN plate (never CALLS work).

    THE POINT (user, 2026-08-16): *"stop having me having to push your button."*
    A watcher that only fires on INCOMING rows leaves the lane's OWN backlog
    parked forever whenever the board is quiet - and a quiet board is exactly
    when there is time to burn it down. So an open plate arms a TIMED wake.

    AN EMPTY PLATE DISARMS THE TIMER COMPLETELY. That is deliberate and it is the
    kill switch: this is the one failure mode of a self-waking monitor that
    cannot be fixed by tuning the interval, because a watcher that wakes with
    nothing to do trains you to ignore it - and then it is useless on the pass
    that matters. Ticking items off is what stops the waking.

    WRAPPED ITEMS ARE JOINED, and that is not a nicety: the first version of this
    function read only the line carrying `- [ ]` and truncated every item at its
    first wrap - reintroducing, in a brand-new parser, the exact multi-line defect
    fixed in scan() the same day. A woken lane that is shown half an item has to
    go open the file anyway, which is the failure the wake exists to prevent.
    """
    if not PLATE.is_file():
        return []
    out = []
    cur = None
    for ln in PLATE.read_text(encoding="utf-8", errors="replace").splitlines():
        s = ln.strip()
        if s.startswith("- [ ]"):
            if cur:
                out.append(cur)
            cur = s[5:].strip()
        elif cur is not None:
            # continuation: indented prose under the item, up to the next item
            if s and not s.startswith(("- [", "#", "*")):
                cur += " " + s
            else:
                out.append(cur)
                cur = None
    if cur:
        out.append(cur)
    return [" ".join(i.split())[:220] for i in out]


def emit(msg):
    # ASCII only: non-ASCII returned '?' under Git Bash and a garbled alert is
    # a discounted alert. (Another lane's watcher WEDGED on an unencodable
    # character — it threw inside emit and retried the same row forever while
    # reporting "still armed".)
    print(msg.encode("ascii", "replace").decode("ascii"), flush=True)


def now_iso():
    return datetime.now(timezone.utc).isoformat(timespec="seconds")


def write_pulse(passes, last_event, note):
    try:
        PULSE.write_text(json.dumps({
            "updated": now_iso(), "updated_epoch": int(time.time()),
            "pid": os.getpid(), "passes": passes,
            "interval_sec": INTERVAL, "last_event": last_event, "note": note,
        }, indent=2) + "\n", encoding="utf-8")
    except OSError as e:
        emit("HISTORY-WATCH ** CANNOT WRITE PULSE ** %s - liveness is now "
             "unfalsifiable, treat this watcher as untrusted" % e)


def scan():
    """{key: (state, addressee, excerpt, authored_by_me)} over BOTH states.

    MULTI-LINE ROWS (added 2026-08-16): rows are no longer always one physical
    line. `decomp_sync.py` — a MACHINE, not a lane — files rows carrying fenced
    transcripts spanning 21 lines, and that same change nearly made
    `calls_rotate.py` amputate 50 rows (Librarian). Detection was already
    correct here: the `- [ ]` and the addressee live on the first line, and
    continuation lines do not match ROW so they are never double-counted
    (verified against the 21-line row). LEGIBILITY was not: a 600-char excerpt
    of line one can miss the entire payload of a 21-line row.

    Detection and legibility are separate failures — Foundry's watcher fired
    correctly on a 5,677-char roadmap and emitted a useless notification. So a
    spanning row now says how many lines it spans and tells you to open the
    file, rather than looking like a short row that happened to end.
    """
    if not CALLS.is_file():
        return None
    lines = CALLS.read_text(encoding="utf-8", errors="replace").splitlines()
    rows = {}
    for i, ln in enumerate(lines):
        s = ln.strip()
        m = ROW.match(s)
        if not m or not MINE.search(m.group(2)):
            continue
        state = "x" if m.group(1).lower() == "x" else " "
        body = s[s.find("|") + 1:].strip()
        # how many physical lines does this row occupy?
        j = i + 1
        while j < len(lines) and not ROW.match(lines[j].strip()) and lines[j].strip():
            j += 1
        span = j - i
        excerpt = body[:600]
        if span > 1:
            excerpt += ("  ...[ROW SPANS %d PHYSICAL LINES — excerpt is line 1 only; "
                        "OPEN CALLS.md AT LINE %d]" % (span, i + 1))
        # key on addressee+body so a checkbox flip is a STATE CHANGE, not a new row
        key = hashlib.sha1((m.group(2).strip() + body[:90])
                           .encode("utf-8", "replace")).hexdigest()[:12]
        # authorship = LEADING NAME of the source field (see defect 6 above),
        # never a substring sweep over a tail window.
        #
        # (7) BUT THE FIELD'S *POSITION* IS NOT GUARANTEED, and this lane proved
        # it by waking itself. The canonical row is
        # `LANES | BODY | SOURCE | DATE` so the source sits at [-2] — but a row
        # filed WITHOUT the trailing date field has only 3 fields, and then [-2]
        # is the BODY and the source has slid to [-1]. I filed exactly such a
        # row, my own suppression missed it, and the watcher delivered my own
        # filing back to me — the precise noise suppression exists to stop.
        # FIX: anchor against BOTH tail fields. A well-formed row has the date
        # at [-1], which cannot match the anchor, so testing both adds no false
        # positives and removes the dependence on field COUNT.
        parts = s.split("|")
        tail = [p.strip() for p in parts[-2:]] if len(parts) >= 3 else []
        rows[key] = (state, m.group(2).strip(), excerpt,
                     any(AUTHORED.match(c) for c in tail))
    return rows


SEEN_DB = HERE / "history-bridge-seen.json"


def load_seen():
    """The baseline from the PREVIOUS arm, so a re-arm does not amnesty rows.

    ⚠ THIS IS THE DEFECT THAT MATTERED MOST AND I SHIPPED IT KNOWINGLY. The
    header has warned since v3 that "it re-baselines on arm, so a row landing
    between a stop and the next arm is swallowed", and I left it unimplemented
    because the window looked narrow.

    IT IS NOT NARROW. Handling a call means: read it, work it, file the answer,
    RE-ARM. So the watcher re-baselines after EVERY delivery — ~15 times in one
    session — and each re-arm silently forgives every row that landed while I
    was working. The window is not the gap between stop and start; it is THE
    ENTIRE WORKING TURN, which is exactly when other lanes are most likely to
    file. Caught when the user asked whether I was seeing my calls and a
    Housing row addressed to HISTORY turned out to be sitting unanswered.

    Persisting the row-state across arms closes it: a row seen as OPEN before
    the stop is still OPEN after the start, and still fires.
    """
    try:
        raw = json.loads(SEEN_DB.read_text(encoding="utf-8"))
        return {k: tuple(v) for k, v in raw.items()}
    except Exception:
        return None


def save_seen(rows):
    try:
        SEEN_DB.write_text(json.dumps({k: list(v) for k, v in rows.items()}),
                           encoding="utf-8")
    except OSError as e:
        emit("HISTORY-WATCH ** CANNOT PERSIST BASELINE ** %s — a re-arm will "
             "amnesty anything filed while this ran" % e)


def main():
    seen = scan()
    if seen is None:
        emit("HISTORY-WATCH ** CALLS.md MISSING AT ARM TIME ** not watching")
        write_pulse(0, "armed-blind", "CALLS.md absent")
        return 2
    # Carry the PREVIOUS arm's state forward. Rows unchanged since then stay
    # baselined; anything filed while this lane was working is now a live event
    # on the first pass instead of being amnestied into the new baseline.
    prior = load_seen()
    if prior:
        # The baseline is the PRIOR state, restricted to rows that still exist.
        # It must NOT include the fresh scan: a row absent from the baseline is
        # exactly what makes it fire, so folding the current scan back in would
        # re-amnesty everything and leave this fix looking applied while doing
        # nothing. (First cut did precisely that.)
        current = seen
        seen = {k: v for k, v in prior.items() if k in current}
        arrived = [k for k in current if k not in seen]
        changed = [k for k in current if k in seen and current[k][0] != seen[k][0]]
        if arrived or changed:
            emit("HISTORY-WATCH  PRIOR BASELINE CARRIED — %d new row(s) and %d state "
                 "change(s) landed while this lane was working; they fire on pass 1 "
                 "instead of being amnestied by the re-arm" % (len(arrived), len(changed)))
    else:
        save_seen(seen)

    op = sum(1 for v in seen.values() if v[0] == " ")
    plate0 = plate_open()
    emit("HISTORY-WATCH v4 ARMED %s | %d row(s) naming HISTORY/BRIDGE/ALL "
         "(%d open, %d answered) | BOTH states | self-suppress on | %ds | pulse %s"
         % (now_iso(), len(seen), op, len(seen) - op, INTERVAL, PULSE.name))
    # Report the wake time the loop can ACTUALLY deliver, not the configured
    # one: it resolves in whole scans, so a config finer than INTERVAL would
    # otherwise be announced accurately and honoured falsely.
    eff = max(PLATE_WAKE_SEC, INTERVAL)
    emit("  DELIVERY: exits on an OPEN row naming HISTORY/BRIDGE *or DECODER* | SELF-WAKE: %s"
         % ("%d plate item(s), waking in ~%ds (scan %ds)" % (len(plate0), eff, INTERVAL)
            if plate0 else "DISARMED (plate empty - calls only)"))
    if PLATE_WAKE_SEC < INTERVAL:
        emit("  ** CONFIG WARNING ** PLATE_WAKE_SEC=%d < INTERVAL=%d — the wake "
             "cannot fire sooner than one scan; effective wake is %ds"
             % (PLATE_WAKE_SEC, INTERVAL, INTERVAL))
    write_pulse(0, "armed", "%d rows (%d open), %d plate" % (len(seen), op, len(plate0)))

    started = time.time()
    passes = 0
    while True:
        time.sleep(INTERVAL)
        passes += 1
        try:
            cur = scan()
            if cur is None:
                emit("HISTORY-WATCH ** CALLS.md MISSING ** BLIND, not quiet")
                write_pulse(passes, "calls-missing", "CALLS.md absent")
                continue
            events = 0
            to_me_events = []
            for k, (st, lane, body, mine_auth) in cur.items():
                if k not in seen:
                    to_me = re.search(r"\b(HISTORY|BRIDGE)\b", lane, re.I)
                    from_decoder = FROM_DECODER.search(lane)
                    if mine_auth and not to_me:
                        continue
                    emit("NEW [%s] >> [%s] %s" % (st, lane, body))
                    events += 1
                    # Only an OPEN row naming THIS LANE forces delivery. Broadcasts
                    # (ALL / ALL LANES) do not, and neither does an already-[x] row
                    # appearing for the first time - that is history landing on the
                    # board, not work arriving.
                    if (to_me or from_decoder) and not mine_auth and st == " ":
                        # (8) NAME THE TRIGGER, do not just count it. Housing
                        # Security's rule, adopted rather than agreed with: IF A
                        # TOOL TAKES AN ACTION ON A CONDITION, IT MUST BE ABLE TO
                        # NAME THE CONDITION. This banner used to print a COUNT,
                        # so every delivery required reading the surrounding
                        # output to infer what actually fired -- reasoning from
                        # context, which is the same inference-from-absence that
                        # produced three false findings on this board in a day.
                        why = "addressed to HISTORY/BRIDGE" if to_me else "authored/addressed DECODER"
                        to_me_events.append((lane, why, body[:110]))
                elif seen[k][0] != st:
                    emit("%s >> [%s] %s" % ("ANSWERED" if st == "x" else "RE-OPENED",
                                            lane, body[:400]))
                    events += 1
                    # DELIBERATELY DOES NOT FORCE DELIVERY. Its first live run
                    # exited on an ANSWERED flip that THIS LANE had caused seconds
                    # earlier (file_row.py answer on a row addressed to me, authored
                    # by Foundry - so the self-suppression check could not see it:
                    # AUTHORED reads the row's SOURCE field, and the source was
                    # Foundry's). Left as-is it wakes this lane every time it closes
                    # its own row - the exact noise self-suppression exists to stop.
                    # An ARRIVING OPEN ROW is work and must interrupt; a row going
                    # ANSWERED is work CLOSING and only needs to be logged. Both
                    # still appear in the output; only the first one exits.
            for k, (st, lane, body, _) in seen.items():
                if k not in cur:
                    emit("ROW GONE (edited/rotated) >> [%s] %s" % (lane, body[:200]))
                    events += 1
            seen = cur
            save_seen(seen)  # persist every pass, so a stop at ANY moment
                             # leaves the next arm an accurate baseline
            op = sum(1 for v in cur.values() if v[0] == " ")
            write_pulse(passes, ("%d event(s)" % events) if events else "quiet",
                        "%d rows (%d open)" % (len(cur), op))
            # ---- TIMED SELF-WAKE while this lane's own plate has open items ----
            plate = plate_open()
            if not to_me_events and plate and (time.time() - started) >= PLATE_WAKE_SEC:
                emit("HISTORY-WATCH ** WAKING ON MY OWN PLATE ** board quiet for "
                     "%dm; %d open item(s) - resume them, do not wait for a call:"
                     % (PLATE_WAKE_SEC // 60, len(plate)))
                for i, item in enumerate(plate, 1):
                    emit("   %d. %s" % (i, item))
                emit("   (tick items in %s; AN EMPTY PLATE STOPS THE TIMED WAKE)"
                     % PLATE.name)
                write_pulse(passes, "woke-on-plate", "%d open plate item(s)" % len(plate))
                return 0

            if to_me_events:
                # ---------------------------------------------------------------
                # (4) DETECTION IS NOT DELIVERY - THE DEFECT THAT COST A USER-ORDERED
                # PING. v3 detected correctly for 56 passes and delivered NOTHING:
                # a background task that NEVER EXITS produces no completion
                # notification, so every event it emitted sat in an output file
                # nobody read. The board waited on a row addressed to this lane
                # while the watcher "worked perfectly".
                #
                # The pulse could not have caught this either - it said ALIVE and
                # ALIVE was true. Liveness, coverage and DELIVERY are three
                # different properties and this lane has now been bitten by all
                # three (see defects 1 and 2 above).
                #
                # FIX: EXIT on a row addressed to HISTORY/BRIDGE specifically.
                # Exiting is what raises a notification in this harness, so the
                # watcher now interrupts rather than accumulates. Broadcast rows
                # (ALL / ALL LANES) do NOT exit - they are informational, and
                # exiting on them would restore the noise self-suppression exists
                # to prevent. RE-ARM after handling; the row set is re-baselined.
                # ---------------------------------------------------------------
                emit("HISTORY-WATCH ** EXITING TO DELIVER ** %d row(s). Re-arm "
                     "after handling." % len(to_me_events))
                for lane, why, excerpt in to_me_events:
                    emit("   TRIGGERED BY: [%s] (%s)" % (lane, why))
                    emit("      %s" % excerpt)
                if not to_me_events:
                    emit("   (NO TRIGGER RECORDED - spurious wake, report it)")
                write_pulse(passes, "exited-to-deliver",
                            "%d to-me row(s)" % len(to_me_events))
                return 0
        except Exception as e:
            emit("HISTORY-WATCH ** PASS FAILED ** %s: %s" % (type(e).__name__, e))
            write_pulse(passes, "pass-failed", "%s: %s" % (type(e).__name__, e))


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
    sys.exit(main())
