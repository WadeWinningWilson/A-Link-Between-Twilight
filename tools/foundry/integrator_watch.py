#!/usr/bin/env python3
# ============================================================================
# integrator_watch.py — the INTEGRATOR's trigger surface, with a SEEN LIST.
#
# WHY THIS EXISTS (2026-08-16): my previous monitor re-emitted every UNCHECKED
# row naming INTEGRATOR on every pass. That is correct for a row nobody has
# touched and WRONG for a row I have already answered — and I cannot always
# answer by checking the box, because a row addressed to `ALL LANES` (or to
# HISTORY, HOUSING and me together) is not mine to close. Checking it would
# clear the trigger for lanes that still owe a ruling.
#
# So I answer those with a PARALLEL row carrying my stamp, the original stays
# open by design, and the old monitor re-fired it forever. It cost a turn each
# time and produced no new information — the same "the instrument cannot tell
# handled from unhandled" defect this session has been cataloguing, pointed at
# my own attention rather than at data.
#
# THE RULE: emit a row ONCE. Persist what has been emitted across restarts, so
# a monitor restart does not replay the whole board. A row is re-emitted only
# if its TEXT changes (an edited row is a new fact).
#
# Also watched: BUILD-QUEUE.md changes (a build may be due) and new boot logs
# (read + symbolicate immediately, per the standing order).
#
# Read-only w.r.t. the board — this NEVER writes CALLS.md.
# ============================================================================
import hashlib
import io
import json
import os
import sys
import time
from pathlib import Path

REPO = Path(__file__).resolve().parents[1].parent
CALLS = REPO / "docs/state/ww-staging/CALLS.md"
QUEUE = REPO / "docs/state/ww-staging/BUILD-QUEUE.md"
LOGS = Path(os.environ.get("APPDATA", "")) / "TwilitRealm/Dusklight/logs"
STATE = Path(__file__).resolve().parent / "integrator_watch_seen.json"
PENDING = Path(__file__).resolve().parent / "integrator_pending.txt"
# ============================================================================
# THE INBOX — fourth monitor lesson of 2026-08-16, user-caught like the other
# three: a fired event is a ONE-SHOT line in a task stream; if it lands while
# the Integrator is mid-turn it scrolls away and "missed once" becomes
# "missed forever". The pending-file nag never has this problem because it
# REPEATS until the artifact is cleared. So fresh rows now ALSO append to
# this inbox file, and the pass loop nags with the inbox COUNT every pass
# until the Integrator empties it after reading. Delivery stops being an
# event and becomes a STATE.
# ============================================================================
INBOX = Path(__file__).resolve().parent / "integrator_inbox.md"

# I am HOUSING SECURITY + HOUSINGTEMP + INTEGRATOR (user, 2026-08-17).
# This matched only INTEGRATOR, so a row addressed to "HOUSING SECURITY"
# WITHOUT "ALL LANES" would never have reached me — my other two lanes were
# unaddressable by their own names. Note the exact tokens: the parser splits
# on "/" and ",", so "HOUSING/ENGINE" yields a bare "HOUSING" that must NOT
# match here — Housing/Engine is a different lane and its rows are not mine.
MINE = ("INTEGRATOR", "HOUSING SECURITY", "HOUSINGTEMP", "ALL LANES", "ALL")
INTERVAL = 20


# ENCODING: the board is full of §, ·, —, →. On a cp1252 console those raise
# UnicodeEncodeError inside emit() — and because emit() runs BEFORE the row is
# marked seen, the same row retried every pass forever ("PASS FAILED" loop,
# observed 2026-08-16). Reconfigure to UTF-8 with replacement, and keep a
# belt-and-braces ASCII fallback so a single odd character can never again
# wedge the trigger surface. A watcher that cannot emit is a watcher that lies.
try:
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
except Exception:
    pass



# ============================================================================
# EXIT-ON-EVENT — copied from the DECODER lane's watcher on the user's order,
# after four failed delivery mechanisms in one day. THE INSIGHT IS THE WHOLE
# FIX: a background task that never exits emits into a stream nobody reads;
# a task that EXITS produces a harness TASK-COMPLETION NOTIFICATION, and that
# notification re-invokes the agent. Delivery stops depending on the agent
# happening to look. Every other channel I built (pulse, inbox, seen-list)
# was refining an emission nobody was listening to.
# ============================================================================
EXIT_ON_EVENT = "--exit-on-event" in sys.argv
_fired = {"hit": False}


def emit(msg):
    try:
        sys.stdout.write(msg + "\n")
    except Exception:
        sys.stdout.write(msg.encode("ascii", "replace").decode("ascii") + "\n")
    sys.stdout.flush()
    if msg.startswith("ACT NOW"):
        # RECORD *WHY* IT WOKE, not just that it did.
        # A pass exited with the exit-on-event banner and NO visible ACT NOW
        # line, which made a one-turn cost into an investigation. An
        # instrument that cannot say which trigger fired forces the reader to
        # re-derive it from absence - the same "silence as evidence" trap that
        # produced three false findings on 2026-08-17. The reason is now
        # carried to the exit line, so a spurious wake explains itself.
        _fired["hit"] = True
        _fired.setdefault("why", []).append(msg[:70])


# ========================================================================
# emit_standing - prints, but does NOT trip the exit-on-event delivery
# ========================================================================
# My own pending items are TRUE on every pass by construction: they stay
# until I do the work. Routing them through emit() made the watcher exit
# immediately, every time, forever - so the one signal that means "another
# lane needs you" was drowned by a signal that means "you still have a
# to-do list". A channel that fires unconditionally carries no information.
# Standing reminders still PRINT (I read them when a real event wakes me);
# only genuinely NEW external state - lane rows, boot logs, queue changes -
# is allowed to spend a wake-up.
# ========================================================================
def emit_standing(msg):
    try:
        sys.stdout.write(msg + chr(10))
    except Exception:
        sys.stdout.write(
            msg.encode("ascii", "replace").decode("ascii") + chr(10))
    sys.stdout.flush()


def load_state():
    try:
        return json.loads(io.open(STATE, encoding="utf-8").read())
    except Exception:
        return {"rows": [], "queue": "", "logs": []}


def save_state(st):
    # Bound the seen list so the file cannot grow without limit; 4000 rows is
    # far past the board's size and keeps restarts cheap.
    st["rows"] = st["rows"][-4000:]
    try:
        io.open(STATE, "w", encoding="utf-8").write(json.dumps(st))
    except Exception:
        pass                      # a state write failure must never kill the watch


def open_rows():
    """{sha: text} for every OPEN row addressed to this lane.

    Addressee is the segment before the first `|`, so `HOUSING/ENGINE` and
    `FOUNDRY, INTEGRATOR, ALL LANES` both parse — the split-on-`/` bug that
    made a sibling watcher miss rows for weeks is not repeated here.
    """
    try:
        text = io.open(CALLS, encoding="utf-8", errors="replace").read()
    except Exception:
        return None
    out = {}
    for line in text.splitlines():
        if not line.startswith("- [ ]"):
            continue
        head = line[5:].split("|", 1)[0]
        lanes = [p.strip().upper() for p in head.replace("/", ",").split(",")]
        if not any(m in lanes for m in MINE):
            continue
        # SKIP ROWS I AUTHORED — not rows ADDRESSED to me. The old watcher's
        # suppression conflated the two and hid two Foundry replies until the
        # user caught it. Authorship is the second-to-last `|` field carrying
        # my stamp; a peer REPLYING to me writes their own stamp there, so
        # their answer still fires. Narrow rule, deliberately.
        parts = line.split("|")
        # AUTHOR SUPPRESSION MUST TRACK MY LANE IDENTITY, NOT ONE NAME.
        # I am HOUSING SECURITY + HOUSINGTEMP + INTEGRATOR (user, 2026-08-17).
        # This matched only "Integrator," — so the moment I began stamping
        # rulings "Housing Security, ..." MY OWN ROWS FIRED BACK AT ME as
        # unanswered calls. That is the re-delivery I logged as an unrooted
        # defect two turns ago: it was not the seen-list, it was my stamp
        # changing out from under a hardcoded name. A suppression keyed to
        # one identity silently breaks when the identity is re-assigned.
        if len(parts) > 2:
            stamp = parts[-2]
            if any(a in stamp for a in ("Integrator,", "Housing Security,",
                                        "HousingTemp,")):
                continue
        out[hashlib.sha1(line.encode("utf-8")).hexdigest()] = line
    return out


def main():
    st = load_state()
    seen = set(st.get("rows", []))
    rows = open_rows()
    emit("INTEGRATOR-WATCH ARMED | %d open row(s) name this lane | %d already "
         "answered and suppressed | seen-list: %s"
         % (len(rows or {}), len(seen), STATE.name))

    # Arming must not replay the board: everything open RIGHT NOW that I have
    # already worked is folded into the seen list on first run.
    if rows and not seen:
        seen |= set(rows.keys())
        st["rows"] = sorted(seen)
        save_state(st)
        emit("INTEGRATOR-WATCH baseline taken — %d existing row(s) suppressed; "
             "only genuinely NEW rows fire from here" % len(seen))

    try:
        st["queue"] = hashlib.sha1(
            io.open(QUEUE, "rb").read()).hexdigest() if QUEUE.exists() else ""
    except Exception:
        pass
    # ========================================================================
    # LOGS ARE NOT BASELINED ON RE-ARM — defect caught by the USER, twice in
    # one hour (2026-08-16: logs 153846 AND 154838 both arrived while the
    # watcher was down and were silently folded into "seen" by this very
    # block). A CALLS row existing at arm time was plausibly already handled;
    # a BOOT LOG existing at arm time was NOT — reading logs is exactly the
    # work this watcher exists to trigger. So: snapshot the directory only on
    # FIRST-EVER arm (no state on disk). On any re-arm, KEEP the persisted
    # list — logs that landed during the outage then fire immediately on the
    # first pass, which is the correct and previously-missing behavior.
    # ========================================================================
    if "logs" not in st:
        try:
            st["logs"] = sorted(p.name for p in LOGS.glob("*.log")) if LOGS.exists() else []
        except Exception:
            st["logs"] = []
    save_state(st)

    pulse = Path(__file__).resolve().parent / "monitor-pulse-integrator.json"
    # ====================================================================
    # PULSE FIRST, SLEEP LAST. The sleep used to lead the loop body, so a
    # freshly-armed watcher wrote NO pulse for a full interval and read as
    # DEAD to every other lane's tooling during exactly the window after a
    # re-arm - the window in which someone is most likely to check. Worse,
    # a watcher restarted more often than INTERVAL never pulsed at all.
    # Arming IS a liveness event; it publishes immediately.
    # ====================================================================
    first = True
    while True:
        if not first:
            time.sleep(INTERVAL)
        first = False
        # PULSE EVERY PASS — user-caught defect 2026-08-16: this watcher never
        # wrote the pulse convention file, so every OTHER lane's tooling read
        # the Integrator as 16 hours dead and deprioritized accordingly. A
        # watcher that is alive but looks dead gets routed around.
        try:
            io.open(pulse, "w", encoding="utf-8").write(
                json.dumps({"lane": "INTEGRATOR", "ts": time.time()}))
        except Exception:
            pass
        try:
            rows = open_rows()
            if rows is None:
                emit("INTEGRATOR-WATCH ** CALLS.md UNREADABLE ** the call "
                     "surface is my trigger; blind until fixed")
            else:
                fresh = [(k, v) for k, v in rows.items() if k not in seen]
                # EMIT EVERY FRESH ROW — user-caught defect, 2026-08-16, third
                # monitor lesson of the day: the old `fresh[:3]` emitted three
                # and marked the REST seen with only a "+N more" summary. When
                # active lanes land a burst (six rows in one gap, twice today),
                # rows 4..N were swallowed forever behind one line nobody
                # reads. A seen-mark without an emission is the delivery gap
                # again, this time inside my own instrument. Mark-seen still
                # precedes each emit (the wedge rule stands); the cap is gone.
                for k, v in fresh:
                    seen.add(k)
                    emit("ACT NOW - READ AND ANSWER THIS CALL IN THIS TURN: "
                         + v[:400])
                    try:            # inbox append — the persistent copy
                        io.open(INBOX, "a", encoding="utf-8").write(v + "\n")
                    except Exception:
                        pass
                if fresh:
                    st["rows"] = sorted(seen)
                    save_state(st)
            # INBOX NAG — repeats every pass until the Integrator clears the
            # file after reading. This is the line that makes a mid-turn miss
            # recoverable instead of permanent.
            try:
                if INBOX.exists():
                    ib = [l for l in io.open(INBOX, encoding="utf-8",
                                             errors="replace").read().splitlines()
                          if l.strip()]
                    if ib:
                        emit("ACT NOW - INBOX HOLDS %d UNREAD CALL(S) - read "
                             "tools/foundry/integrator_inbox.md and CLEAR it"
                             % len(ib))
            except Exception:
                pass

            # QUEUE: FIRE ON WORK ARRIVING, NOT ON ANY BYTE CHANGING.
            # Hashing the whole file meant MY OWN CLOSE of a built row re-fired
            # the trigger at me — the same "cannot tell my writes from theirs"
            # defect the CALLS side had, where answering a row was what made it
            # re-fire. A CLOSE lowers the open-row count; QUEUED WORK raises it.
            # Count open rows instead: the trigger's whole purpose is "there is
            # something to gate and maybe build", and a row I just closed is
            # neither. A noisy trigger is one that gets ignored, which is the
            # failure mode this estate keeps naming.
            if QUEUE.exists():
                try:
                    qtext = io.open(QUEUE, encoding="utf-8",
                                    errors="replace").read()
                    nopen = sum(1 for ln in qtext.splitlines()
                                if ln.startswith("- [ ]"))
                except Exception:
                    nopen = st.get("queue_open", 0)
                prev = st.get("queue_open")
                if prev is None:
                    st["queue_open"] = nopen
                    save_state(st)
                elif nopen > prev:
                    st["queue_open"] = nopen
                    save_state(st)
                    emit("ACT NOW - BUILD-QUEUE: %d new open row(s) (%d -> %d) "
                         "- re-gate this turn and build if due"
                         % (nopen - prev, prev, nopen))
                elif nopen != prev:
                    # A close or a reword. Track it silently; do not order work.
                    st["queue_open"] = nopen
                    save_state(st)

            # ================================================================
            # SELF-IDENTIFIED WORK IS NOW A TRIGGER, NOT A MEMORY.
            # User, 2026-08-16: "THAT IS A MEMORY YOU HAVE TO READ, NOT AN
            # ACTUAL MECHANISM DESIGNED TO TRIGGER YOU." Correct - a memory
            # file is passive and only helps if I happen to re-read it. This
            # makes my own identified next-action an ARTIFACT THE WATCHER
            # WATCHES: every non-empty line in `integrator_pending.txt` fires
            # an ACT NOW on EVERY pass until it is cleared.
            #
            # It cannot be satisfied by acknowledging - only by deleting the
            # line, which means doing the work. That is the difference between
            # a rule I must remember and a mechanism that remembers for me.
            # ================================================================
            if PENDING.exists():
                try:
                    items = [ln.strip() for ln in
                             io.open(PENDING, encoding="utf-8",
                                     errors="replace").read().splitlines()
                             if ln.strip() and not ln.strip().startswith("#")]
                except Exception:
                    items = []
                for it in items[:3]:
                    emit_standing("STANDING - WORK YOU IDENTIFIED AND HAVE "
                                  "NOT DONE: " + it[:300])
                if len(items) > 3:
                    emit_standing("STANDING - (+%d more pending item(s) in %s)"
                                  % (len(items) - 3, PENDING.name))

            if LOGS.exists():
                cur = sorted(p.name for p in LOGS.glob("*.log"))
                new = [n for n in cur if n not in st.get("logs", [])]
                if new:
                    st["logs"] = cur
                    save_state(st)
                    for n in new[-2:]:
                        emit("ACT NOW - NEW BOOT LOG, READ AND SYMBOLICATE IT "
                             "THIS TURN: " + n)
            # THE DELIVERY STEP: exit so the harness notifies. Everything
            # above is bookkeeping; this line is what reaches the agent.
            if EXIT_ON_EVENT and _fired["hit"]:
                why = _fired.get("why") or ["(NO TRIGGER RECORDED - spurious "
                                            "wake, report it)"]
                emit_standing("INTEGRATOR-WATCH exiting to notify "
                              "(--exit-on-event) | TRIGGERED BY: "
                              + " | ".join(w for w in why[:4]))
                return
        except Exception as e:      # a watcher that dies is a watcher that lies
            emit("INTEGRATOR-WATCH ** PASS FAILED ** %s — still armed" % e)


if __name__ == "__main__":
    main()
