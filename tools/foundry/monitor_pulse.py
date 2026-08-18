#!/usr/bin/env python3
# ============================================================================
# monitor_pulse.py — IS MY WATCHER ALIVE, OR HAS IT BEEN DEAD FOR AN HOUR?
#
# ---------------------------------------------------------------------------
# THE QUESTION NO OTHER TOOL IN THIS REPO CAN ANSWER. `TaskList` does not show
# monitors — it reported "No tasks found" while three were firing (handoff §4).
# **You cannot enumerate your own watchers**, so the only evidence a monitor is
# alive is evidence IT wrote. `lane_watch.py` stamps `monitor-pulse.json`
# every pass; this reads the age of that stamp.
#
# Without it, a dead monitor and a quiet board are the same observation, and
# the previous Foundry instance lost a monitor for hours on exactly that.
#
# THE VERDICTS, deliberately not binary:
#   ALIVE    stamped within 2 intervals — it is running
#   LATE     within 6 intervals — a slow pass or a long gate audit, watch it
#   DEAD     older than 6 intervals — treat every quiet minute since as UNKNOWN
#   ABSENT   no pulse file — no watcher has EVER run here (not "clean")
#
# **DEAD does not mean nothing happened. It means nothing was watching**, and
# the difference is the entire point (№31-C: absent evidence is reported,
# never assumed).
#
# Usage:  monitor_pulse.py [<pulse.json>]
# Exit 0 = ALIVE · 1 = LATE/DEAD/ABSENT · 2 = unreadable.
# ============================================================================
import json
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
import time
from pathlib import Path

HERE = Path(__file__).resolve().parent
DEFAULT = HERE / "monitor-pulse.json"

# A pid that cannot exist, for the self-test. Windows pids are small multiples
# of 4; POSIX pids are bounded well below this.
IMPOSSIBLE_PID = 0x7FFFFFFE


def pid_alive(pid):
    """True / False / None (cannot tell on this platform).

    **NEVER `os.kill(pid, 0)` ON WINDOWS.** Python maps os.kill to
    TerminateProcess there, so the textbook liveness probe would KILL the
    watcher it is asking about — a check that destroys what it measures.
    """
    if pid is None or pid <= 0:
        return None
    if sys.platform == "win32":
        try:
            import ctypes
            from ctypes import wintypes
            PROCESS_QUERY_LIMITED_INFORMATION = 0x1000
            STILL_ACTIVE = 259
            ERROR_ACCESS_DENIED = 5
            k32 = ctypes.WinDLL("kernel32", use_last_error=True)
            h = k32.OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, False, pid)
            if not h:
                # Access denied means it EXISTS but is not ours to query.
                # Reporting that as dead would be a false DEAD.
                return True if ctypes.get_last_error() == ERROR_ACCESS_DENIED \
                    else False
            try:
                code = wintypes.DWORD()
                if not k32.GetExitCodeProcess(h, ctypes.byref(code)):
                    return None
                return code.value == STILL_ACTIVE
            finally:
                k32.CloseHandle(h)
        except Exception:
            return None
    try:
        import os
        os.kill(pid, 0)          # safe on POSIX only
        return True
    except ProcessLookupError:
        return False
    except PermissionError:
        return True
    except Exception:
        return None


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    pulse = Path(args[0]) if args else DEFAULT

    # ====================================================================
    # `--selftest` IS THIS TOOL'S NEGATIVE CONTROL, and it has to be
    # GENERATED rather than checked in. The failure it guards against is a
    # FRESH stamp from a DEAD process; a static fixture would go stale and
    # then read DEAD for the wrong reason — passing the control while
    # proving nothing about the pid check. So the fixture is written with
    # the current timestamp, every run.
    # ====================================================================
    # `--selftest-alive` IS THE OTHER DIRECTION, and it is not optional.
    # The two DEAD controls together still cannot catch a tool STUCK on
    # DEAD — that would pass both and report a healthy watcher as dead
    # forever, which is a false alarm that trains you to ignore the alarm.
    # A gate needs a control in every direction it can rule; this lane
    # shipped a sig_diff that could only be proven in one.
    if "--selftest" in sys.argv or "--selftest-alive" in sys.argv:
        import os
        import tempfile
        alive_mode = "--selftest-alive" in sys.argv
        pulse = Path(tempfile.gettempdir()) / (
            "foundry_pulse_selftest_%s.json" % ("alive" if alive_mode
                                                else "dead"))
        pulse.write_text(json.dumps({
            "updated": time.strftime("%Y-%m-%dT%H:%M:%S+00:00",
                                     time.gmtime()),
            "updated_epoch": int(time.time()),   # FRESH in both modes
            # the only difference: a writer that exists, or one that cannot
            "pid": os.getpid() if alive_mode else IMPOSSIBLE_PID,
            "passes": 0,
            "interval_sec": 180,
            "last_event": "selftest",
            "note": "generated control",
        }) + "\n", encoding="utf-8")
        if alive_mode:
            print("SELFTEST-ALIVE — fresh stamp written by pid %d, this very "
                  "process." % os.getpid())
            print("A correct verdict is ALIVE. DEAD here means the pid check "
                  "rejects live writers.\n")
        else:
            print("SELFTEST — fresh stamp (age 0 s) written by pid %d, which "
                  "cannot exist." % IMPOSSIBLE_PID)
            print("Age alone would say ALIVE. A correct verdict is DEAD.\n")

    print("MONITOR PULSE — %s" % pulse.name)
    if not pulse.is_file():
        print("  [ABSENT] no pulse file at %s" % pulse)
        print("  **No watcher has ever stamped here.** That is UNKNOWN, not")
        print("  clean — nothing has been watching this lane's surfaces.")
        return 1

    try:
        d = json.loads(pulse.read_text(encoding="utf-8", errors="replace"))
    except (OSError, ValueError) as e:
        print("  UNREADABLE: %s" % e)
        return 2

    stamped = int(d.get("updated_epoch", 0))
    interval = int(d.get("interval_sec", 180)) or 180
    age = int(time.time()) - stamped

    if age <= interval * 2:
        verdict, rc = "ALIVE", 0
    elif age <= interval * 6:
        verdict, rc = "LATE", 1
    else:
        verdict, rc = "DEAD", 1

    # ====================================================================
    # A FRESH STAMP IS NOT A LIVE WATCHER. Added 2026-08-15 after a lane ran
    # `lane_watch.py --help`, which (having no help handling) armed, stamped
    # this file, and was then killed by `timeout`. The stamp was seconds old
    # and the writer was dead — **this tool would have said ALIVE while
    # nothing was watching.** Recency was an observation that could not come
    # out the other way, which is the exact defect the estate spent a night
    # on, committed inside the instrument built to detect it.
    #
    # So verify the stamping process still exists. NEVER `os.kill(pid, 0)`
    # on Windows: Python maps os.kill to TerminateProcess there, so the
    # "liveness probe" would KILL the watcher it is checking.
    # ====================================================================
    pid = d.get("pid")
    if verdict in ("ALIVE", "LATE") and isinstance(pid, int):
        alive = pid_alive(pid)
        if alive is False:
            print("  [DEAD] stamp is %d s old but PID %d IS GONE" % (age, pid))
            print("  **A fresh stamp from a dead process is not liveness.**")
            print("  Something stamped this file and exited (a `--help` run, a")
            print("  crash, a killed smoke test). Nothing is watching.")
            print("  Re-arm, and treat the quiet since as UNOBSERVED.")
            return 1
        if alive is None:
            print("  NOTE: could not verify PID %d on this platform; the")
            print("  verdict below rests on stamp age alone." % pid)

    print("  [%s] last stamp %s (%d s ago; interval %d s)"
          % (verdict, d.get("updated", "?"), age, interval))
    print("  pid %s · interval %d s · passes: %s · last event: %s · state: %s"
          % (d.get("pid", "?"), interval, d.get("passes", "?"),
             d.get("last_event", "?"), d.get("note", "?")))
    print("  **Cross-check pid and interval against MONITOR-REGISTRY.md.**")
    print("  Watchers on ONE lane share that lane's pulse file and stomp each")
    print("  other's stamp, so a value that does not match the registry row")
    print("  means a DUPLICATE is running — exactly how the stray of")
    print("  2026-08-15 was caught. One pulse file PER LANE")
    print("  (`lane_watch.py --pulse <path>`): a second lane stamping a shared")
    print("  file would overwrite pid and interval every pass and this check")
    print("  would fire forever, which is no signal at all.")
    if verdict == "DEAD":
        print("  **Everything quiet in the last %d s is UNKNOWN, not clear.**"
              % age)
        print("  Re-arm the watcher, then treat the gap as unobserved.")
    elif verdict == "LATE":
        print("  A gate audit pass can legitimately run long. If the next")
        print("  check is still LATE or worse, treat it as DEAD.")
    return rc


if __name__ == "__main__":
    sys.exit(main())
