#!/usr/bin/env python3
# ============================================================================
# monitor_census.py - IS EACH LANE'S WATCHER ACTUALLY ALIVE?
#
# THE FAILURE THIS EXISTS FOR: **a pulse file keeps its mtime after the writer
# dies.** A watcher that died an hour ago leaves behind a file that still says
# "last wrote at <time>", and every check that asks only "is the pulse recent?"
# reports ALIVE on a corpse. That is worse than no check, because a dead
# watcher does not look like an outage - **it looks like quiet.** The lane
# stops seeing rows addressed to it and nobody, including the lane, notices.
#
# THE FIX, and the whole point of this tool: **verify the PID against the live
# process table, not the pulse.** A recent pulse is necessary and NOT
# sufficient. Both signals are printed side by side and the verdict requires
# BOTH, so a disagreement between them is visible rather than resolved
# silently in favour of the cheerful one.
#
# THE HALF-DEAD PAIR, which is why this runs every wake and not on suspicion:
# most lanes run TWO watchers - a board watcher (sees rows addressed to them)
# and a continuation timer (wakes them between calls). When only ONE dies the
# lane keeps behaving almost normally, so the outage produces no symptom the
# lane can feel. Observed live on 2026-08-22: Decoder's board watcher dead 80
# min (pid 29600) while its timer ticked; Engine's timer dead 175 min (pid
# 34496) while its board watcher ran. Neither lane had reported anything wrong.
#
# WHAT IT WILL NOT DO: re-arm anything. A watcher is a lane's own agent
# process and cannot be spawned from outside it. This tool produces evidence;
# the owning lane does the re-arming. (Usual cause of a death: launching the
# watcher with a shell ampersand inside a Bash call - it dies when that call
# returns. Use run_in_background.)
#
# Usage:
#   monitor_census.py             census all pulses
#   monitor_census.py --stale N   minutes before a pulse counts stale (default 12)
#   monitor_census.py --control   prove the check can report DOWN, not only ALIVE
# Exit 0 all alive - 1 at least one DOWN - 2 the census itself could not run.
# ============================================================================
import json
import os
import subprocess
import sys
import time
from pathlib import Path

if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")

HERE = Path(__file__).resolve().parent
DEFAULT_STALE_MIN = 12.0


def live_pids():
    """The real process table. None means WE COULD NOT TELL -- which is
    reported as unknown, never silently as alive. A census that cannot see
    the process table has lost the only signal that distinguishes it from
    the pulse-only check it replaces."""
    try:
        out = subprocess.run(
            ["powershell", "-NoProfile", "-Command",
             "Get-Process | Select-Object -Expand Id"],
            capture_output=True, text=True, timeout=60).stdout
        pids = {int(x) for x in out.split() if x.strip().isdigit()}
        return pids or None
    except Exception:
        return None


STATE = HERE / ".monitor_census_state.json"


def load_state():
    """Last census's (pid, mtime) per pulse. Absent/corrupt reads as empty --
    a missing history must degrade to UNCERTAIN, never to a confident verdict."""
    try:
        return json.loads(STATE.read_text(encoding="utf-8"))
    except Exception:
        return {}


def save_state(state):
    try:
        STATE.write_text(json.dumps(state, indent=1), encoding="utf-8")
    except OSError:
        pass          # history is an optimisation; losing it weakens, not breaks


def census(stale_min=DEFAULT_STALE_MIN, pulses=None, prev=None, remember=True):
    now = time.time()
    live = live_pids()
    prev = load_state() if prev is None else prev
    # How long since the PREVIOUS census -- the window over which "nothing
    # moved" is being judged. None on a first-ever run.
    prev_at = prev.get("_census_at") if isinstance(prev, dict) else None
    gap_min = (now - prev_at) / 60.0 if prev_at else None
    state = {}
    rows = []
    src = pulses if pulses is not None else HERE.glob("monitor-pulse*.json")
    for f in sorted(src):
        age = (now - f.stat().st_mtime) / 60.0
        pid, err = None, None
        try:
            j = json.loads(f.read_text(encoding="utf-8", errors="replace"))
            pid = j.get("pid") or j.get("PID")
        except Exception as e:
            err = str(e)[:40]
        if live is None:
            pidstate, pid_ok = "process table UNREADABLE", None
        elif pid is None:
            pidstate, pid_ok = "NO PID RECORDED", None
        elif int(pid) in live:
            pidstate, pid_ok = "pid %s LIVE" % pid, True
        else:
            pidstate, pid_ok = "pid %s DEAD" % pid, False
        fresh = age < stale_min
        # ====================================================================
        # THE PER-TICK WRITER, and the inverse error it nearly caused.
        #
        # Not every watcher is one long-lived process. Some are a TIMER that
        # spawns a SHORT-LIVED WRITER each tick: the writer stamps the pulse
        # and exits, so by the time any census looks, the recorded pid is
        # ALREADY GONE. Pulse fresh + pid dead therefore has TWO readings --
        # a healthy per-tick watcher, or a watcher that just died -- and
        # calling it DOWN is the exact INVERSE of the false-ALIVE bug this
        # tool exists to kill. Caught live on 2026-08-22: Engine's two pulses
        # read "pid DEAD" three minutes apart with DIFFERENT pids each time
        # (34496->22968, 34332->30660). **A corpse cannot change its pid.**
        #
        # RESOLUTION, and it needs no guessing: a pid or mtime that MOVED
        # since the previous census is positive proof something is running.
        # When nothing has moved we say UNCERTAIN and name what resolves it,
        # rather than picking whichever verdict reads more decisive.
        # ====================================================================
        prior = (prev or {}).get(f.name) or {}
        if not isinstance(prior, dict):
            prior = {}
        moved = (prior and (str(prior.get("pid")) != str(pid)
                            or prior.get("mtime", 0) < f.stat().st_mtime))
        # SILENCE ONLY MEANS SOMETHING IF WE WATCHED LONG ENOUGH. "Nothing
        # moved" across a 45-SECOND gap says nothing about a watcher whose
        # tick is five minutes -- concluding DOWN from it would be inventing
        # a death out of a short look. The observation gap must be at least
        # the staleness threshold before absence of change is evidence.
        watched_long_enough = gap_min is not None and gap_min >= stale_min
        if pid_ok is None:
            verdict = "UNKNOWN"          # never guess ALIVE
        elif not fresh:
            verdict = "*** DOWN ***"     # stale pulse is decisive on its own
        elif pid_ok:
            verdict = "ALIVE"
        elif moved:
            verdict = "ALIVE (per-tick)"  # pid/mtime moved -> a writer is running
        elif not prior:
            verdict = "UNCERTAIN"        # first sighting; nothing to compare to
        elif watched_long_enough:
            verdict = "*** DOWN ***"     # silent across a meaningful window
        else:
            verdict = "UNCERTAIN"        # dead pid, but we have not watched long enough
        rows.append((f.name, age, pidstate, fresh, verdict, err))
        state[f.name] = {"pid": pid, "mtime": f.stat().st_mtime}
    state["_census_at"] = now
    if remember:
        save_state(state)
    return rows, live, gap_min


def report(rows, live, stale_min, gap_min=None):
    print("MONITOR CENSUS - pulse age AND pid. A recent pulse is NOT proof of life;")
    print("a pulse file keeps its mtime after the writer dies.")
    if live is None:
        print("  !! process table unreadable - every verdict below is UNKNOWN, not ALIVE.")
    if gap_min is None:
        print("  (first census - no prior sample, so a fresh pulse with a dead pid")
        print("   reads UNCERTAIN, not DOWN. Run again to resolve it.)")
    else:
        print("  (silence judged over a %.1f min window; needs >=%.0f min to count)"
              % (gap_min, stale_min))
    print()
    for name, age, pidstate, fresh, verdict, err in rows:
        flag = "" if fresh else "  <- pulse stale (>%.0fm)" % stale_min
        print("  %-34s pulse %7.1f min  %-26s %s%s"
              % (name, age, pidstate, verdict, flag))
        if err:
            print("      (pulse unparseable: %s)" % err)
    down = [r for r in rows if not r[4].startswith("ALIVE")]
    print()
    if not down:
        print("  ALL %d WATCHERS ALIVE (pid-verified)." % len(rows))
        return 0
    print("  *** %d OF %d NOT CONFIRMED ALIVE ***" % (len(down), len(rows)))
    print("  The OWNING LANE re-arms its own watcher - a watcher is that lane's")
    print("  agent process and cannot be spawned from here. Use run_in_background:")
    print("  a watcher launched with a shell ampersand inside a Bash call dies")
    print("  when the call returns, which is the usual cause.")
    return 1


def control():
    """Prove the census can say DOWN. A check that has only ever printed ALIVE
    is indistinguishable from a check that CANNOT print anything else -- which
    is precisely the pulse-only bug this tool replaces, so asserting the fix
    without this control would repeat it."""
    import tempfile
    tmp = Path(tempfile.mkdtemp(prefix="census_control_"))
    # (1) fresh pulse, pid that cannot be live, NOTHING MOVED since last look
    #     -- the false-ALIVE case this tool was built for.
    corpse = tmp / "monitor-pulse-CONTROL-corpse.json"
    corpse.write_text(json.dumps({"pid": 999999999, "note": "fresh pulse, dead pid"}))
    # (2) fresh pulse, OUR pid -- must read ALIVE.
    alive = tmp / "monitor-pulse-CONTROL-alive.json"
    alive.write_text(json.dumps({"pid": os.getpid(), "note": "fresh pulse, live pid"}))
    # (3) fresh pulse, dead pid, but the pid MOVED since last look -- a per-tick
    #     writer. Must read ALIVE, or this tool reports healthy watchers dead:
    #     the INVERSE error, and just as blinding.
    tick = tmp / "monitor-pulse-CONTROL-pertick.json"
    tick.write_text(json.dumps({"pid": 999999998, "note": "per-tick writer"}))
    prev = {
        corpse.name: {"pid": 999999999, "mtime": corpse.stat().st_mtime},
        alive.name: {"pid": os.getpid(), "mtime": alive.stat().st_mtime},
        tick.name: {"pid": 999999997, "mtime": tick.stat().st_mtime},  # different pid
        # Pretend the previous census was an hour ago, so "nothing moved" is
        # being judged over a window long enough to MEAN something. Without
        # this the corpse case correctly reads UNCERTAIN and the control
        # would be testing the guard rather than the verdict.
        "_census_at": time.time() - 3600,
    }
    try:
        rows, live, _gap = census(DEFAULT_STALE_MIN, pulses=sorted(tmp.glob("*.json")),
                                  prev=prev, remember=False)
        got = {r[0]: r[4] for r in rows}
        c1 = got.get(corpse.name) == "*** DOWN ***"
        c2 = got.get(alive.name) == "ALIVE"
        c3 = got.get(tick.name) == "ALIVE (per-tick)"
        # (4) THE SAME INPUTS AS THE CORPSE, only the observation WINDOW is
        #     short. Must NOT read DOWN. This is the case that proves the
        #     window is load-bearing rather than decorative: without it the
        #     tool would invent a death out of a 45-second look at a watcher
        #     whose tick is five minutes.
        short = dict(prev)
        short["_census_at"] = time.time() - 30
        rows4, _l4, _g4 = census(DEFAULT_STALE_MIN, pulses=[corpse],
                                 prev=short, remember=False)
        c4 = rows4[0][4] == "UNCERTAIN"
        print("CONTROL - the verdict must swing ALL FOUR ways, not just the loud one")
        print("  fresh / dead pid / unchanged -> DOWN            %s"
              % ("PASS" if c1 else "*** FAIL *** got %r" % got.get(corpse.name)))
        print("  fresh / live pid             -> ALIVE           %s"
              % ("PASS" if c2 else "*** FAIL *** got %r" % got.get(alive.name)))
        print("  fresh / dead pid / pid MOVED -> ALIVE(per-tick) %s"
              % ("PASS" if c3 else "*** FAIL *** got %r" % got.get(tick.name)))
        print("  same inputs as row 1, SHORT window   -> UNCERTAIN      %s"
              % ("PASS" if c4 else "*** FAIL *** got %r" % rows4[0][4]))
        ok = c1 and c2 and c3 and c4
        print("  CONTROL %s" % ("PASSED - corpse dead, process alive, per-tick writer "
                                "alive, and short looks refuse to convict."
                                if ok else "*** FAILED ***"))
        return 0 if ok else 1
    finally:
        for f in tmp.glob("*"):
            f.unlink()
        tmp.rmdir()


def main():
    if "--control" in sys.argv:
        return control()
    stale = DEFAULT_STALE_MIN
    if "--stale" in sys.argv:
        try:
            stale = float(sys.argv[sys.argv.index("--stale") + 1])
        except (IndexError, ValueError):
            print("*** FAILED *** --stale needs a number of minutes")
            return 2
    rows, live, gap_min = census(stale)
    if not rows:
        print("*** FAILED *** no monitor-pulse*.json found in %s" % HERE)
        return 2
    return report(rows, live, stale, gap_min)


if __name__ == "__main__":
    sys.exit(main())
