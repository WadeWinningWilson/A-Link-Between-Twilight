#!/usr/bin/env python3
# ============================================================================
# watcher_census.py - WHICH PROCESSES ARE ACTUALLY WATCHERS, AND WHICH ARE
# THE HARNESS SHELLS AROUND THEM?
#
# ---------------------------------------------------------------------------
# WHY THIS REPLACES THE ONE-LINER. The duplicate scan in MONITOR-REGISTRY.md
# went through two wrong versions in five minutes, both mine:
#
#   1. filtered `Name='python.exe'` + `-like '*lane_watch*'` - could not see
#      the Librarian's `.sh` or the Integrator's inline bash, so it would
#      have reported "no duplicates" with two other watchers running;
#   2. widened, then narrowed again with `-and $_.Name -notmatch 'bash'` to
#      cut wrapper noise - which dropped the Integrator's bash watcher.
#
# **Both times the filter was narrower than the claim it supported.** The
# widened version then drew the opposite complaint from the lanes, and it is
# the dangerous one: *"3-4 hits per watcher and all but one are harness
# wrappers - read it wrong and you will kill a healthy watcher."*
#
# **A raw process list that a human must interpret correctly under time
# pressure is not an instrument; it is a trap with documentation.** Each
# Monitor task appears as a chain - `bash -c` wrapper, sometimes a second
# shell, then the real process - and killing the wrapper is how you take down
# a watcher you meant to keep.
#
# ---------------------------------------------------------------------------
# HOW IT DISCRIMINATES, without hardcoding interpreter names (the mistake
# twice over): **a matching process that is the PARENT of another matching
# process is a WRAPPER. The leaves are the watchers.** That is structural, so
# it holds for `bash -> bash -> python`, for `bash -> sh`, and for the
# Integrator's inline bash loop, which is a leaf and therefore a watcher.
#
# It then cross-checks the leaves against the pid column of
# MONITOR-REGISTRY.md and reports THREE failure directions, because any one
# of them alone is a half-check:
#   UNREGISTERED  running but in no registry row -> a stray, the --old defect
#   MISSING       registry row whose pid is not running -> a dead watcher
#                 whose silence you would otherwise read as a quiet board
#   MATCHED       both agree
#
# Report-only. It never kills anything: naming what is safe to kill is the
# job; doing it is the operator's.
#
# Usage:  watcher_census.py [--pattern REGEX] [--selftest]
# Exit 0 = every leaf matched a registry row - 1 = a discrepancy - 2 = could
# not enumerate.
# ============================================================================
import csv
import re
import subprocess
import sys
if hasattr(sys.stdout, "reconfigure"):
    sys.stdout.reconfigure(encoding="utf-8", errors="replace")
from pathlib import Path

HERE = Path(__file__).resolve().parent
REGISTRY = HERE / "MONITOR-REGISTRY.md"

# What a watcher's command line looks like. Matched in PYTHON, never inside
# the enumeration command - that is what kept the scan's own process out of
# its own results, which the one-liner could not manage.
DEFAULT_PATTERN = r"lane_watch|watch\.sh|_watch\.py|monitor-pulse"

CONTROL_MARKER = "WATCHER_CENSUS_CONTROL_lane_watch"


def enumerate_processes():
    """[(pid, ppid, name, cmdline)] for every process, or None."""
    ps = ("Get-CimInstance Win32_Process | "
          "Select-Object ProcessId,ParentProcessId,Name,CommandLine | "
          "ConvertTo-Csv -NoTypeInformation")
    try:
        r = subprocess.run(
            ["powershell", "-NoProfile", "-NonInteractive", "-Command", ps],
            capture_output=True, text=True, encoding="utf-8",
            errors="replace", timeout=120)
    except Exception as e:
        print("  COULD NOT ENUMERATE PROCESSES: %s" % e)
        return None
    if r.returncode != 0 and not r.stdout.strip():
        print("  COULD NOT ENUMERATE PROCESSES (exit %d)" % r.returncode)
        return None
    out = []
    for row in csv.DictReader(r.stdout.splitlines()):
        try:
            pid = int(row.get("ProcessId") or 0)
            ppid = int(row.get("ParentProcessId") or 0)
        except ValueError:
            continue
        out.append((pid, ppid, row.get("Name") or "",
                    (row.get("CommandLine") or "").replace("\n", " ")))
    return out


# Filenames that identify a watcher in a registry row's prose.
RE_PAYLOAD = re.compile(r"[\w.-]+\.(?:py|sh)|monitor-pulse[\w.-]*\.json"
                        r"|[\w-]*pulse[\w-]*\.json")


def registry_rows():
    """[(task_id, pid|None, [payload tokens])]. Absent file -> None."""
    if not REGISTRY.is_file():
        return None
    rows = []
    for ln in REGISTRY.read_text(encoding="utf-8", errors="replace").splitlines():
        s = ln.strip()
        if not s.startswith("| `"):
            continue
        cells = [c.strip().strip("`") for c in s.strip("|").split("|")]
        task = cells[0] if cells else "?"
        pid = next((int(c) for c in cells[1:] if c.isdigit()), None)
        payloads = {t for t in RE_PAYLOAD.findall(s)
                    if not t.endswith("MONITOR-REGISTRY.md")}
        rows.append((task, pid, payloads))
    return rows


def main():
    pattern = DEFAULT_PATTERN
    if "--pattern" in sys.argv:
        pattern = sys.argv[sys.argv.index("--pattern") + 1]
    selftest = "--selftest" in sys.argv

    proc = None
    if selftest:
        # ================================================================
        # THE NEGATIVE CONTROL. A census that finds NOTHING looks exactly
        # like a clean estate - the vacuous pass again, and the most likely
        # way this tool fails (a PowerShell that will not run, a CSV shape
        # change). So spawn a process that IS a watcher by this tool's own
        # definition and require it to be found as a LEAF.
        # ================================================================
        proc = subprocess.Popen(
            [sys.executable, "-c", "import time; time.sleep(25)",
             CONTROL_MARKER])
        pattern = CONTROL_MARKER
        print("SELFTEST - spawned pid %d whose command line contains the "
              "watcher marker." % proc.pid)
        print("It MUST appear below as a WATCHER leaf. Finding nothing means "
              "this census is blind.\n")

    procs = enumerate_processes()
    if procs is None:
        if proc:
            proc.kill()
        return 2

    hits = [p for p in procs if re.search(pattern, p[3] or "", re.I)]
    hit_pids = {p[0] for p in hits}
    # A matching process that PARENTS another matching process is the shell
    # around it. Leaves are the real watchers.
    parents = {p[1] for p in hits if p[1] in hit_pids}
    leaves = [p for p in hits if p[0] not in parents]
    wrappers = [p for p in hits if p[0] in parents]

    print("WATCHER CENSUS - %d matching process(es): %d watcher(s), "
          "%d harness wrapper(s)" % (len(hits), len(leaves), len(wrappers)))
    print("  wrappers are the shells AROUND a watcher. **Killing one takes "
          "down the watcher inside it.**\n")

    reg = registry_rows()
    rc = 0

    # ====================================================================
    # MATCH THROUGH THE ANCESTRY, NOT JUST THE LEAF PID. First run of this
    # census flagged three of five leaves UNREGISTERED. **None was a stray.**
    # A lane records the pid its shell reports (`$$`) - the WRAPPER - while
    # this tool reports the leaf, so the two disagree by construction on
    # every watcher launched through a shell.
    # **In a tool whose stated job is "do not kill a healthy watcher", an
    # UNREGISTERED label on a healthy watcher is the worst output it can
    # produce** - and it is the same false-absence shape as everything else
    # caught today: the identifier was there, one level up, and the check
    # could not see that far. So walk the parent chain before ruling.
    # ====================================================================
    ppid_of = {p[0]: p[1] for p in procs}
    by_pid = {t: p for t, p, _ in (reg or []) if p}

    def registered_via(pid, cmd):
        """(task_id, how) - by pid, by ancestor pid, or by payload name."""
        if not reg:
            return None, None
        seen, cur, hops = set(), pid, 0
        while cur and cur not in seen and hops < 12:
            for task, rpid, _pay in reg:
                if rpid == cur:
                    return task, ("pid" if hops == 0
                                  else "its shell %d hop(s) up" % hops)
            seen.add(cur)
            cur = ppid_of.get(cur, 0)
            hops += 1
        # ================================================================
        # PID FAILED - FALL BACK TO THE PAYLOAD NAME, because for a
        # shell-launched watcher the pid CANNOT match. **Git Bash `$$`
        # returns an MSYS pid, not a Win32 ProcessId**, so a lane that
        # records `$$` writes a number from a different namespace and no
        # Win32 scan will ever find it. My first two passes read that as
        # three strays and said UNREGISTERED next to three healthy
        # watchers - in the tool whose one job is stopping you from
        # killing a healthy watcher.
        # The command line, unlike the pid, is namespace-free.
        # ================================================================
        low = (cmd or "").lower()
        for task, _rpid, payloads in reg:
            for p in payloads:
                if p.lower() in low:
                    return task, "payload %s (pid namespaces differ)" % p
        return None, None

    print("  WATCHERS (leaf processes - these are the real ones):")
    if not leaves:
        print("    (none) - if you expected watchers, this census is either")
        print("    blind or the estate is unwatched. Run --selftest to tell")
        print("    those apart; they look identical from here.")
        rc = 1
    unregistered = []
    for pid, ppid, name, cmd in sorted(leaves):
        if reg is None:
            tag = "registry unreadable - cannot rule"
        else:
            task, how = registered_via(pid, cmd)
            if task and how == "pid":
                tag = "registered as %s" % task
            elif task:
                tag = "registered as %s, matched by %s - NOT a stray" % (
                    task, how)
            else:
                tag = "UNREGISTERED"
                unregistered.append(pid)
                rc = 1
        print("    pid %-7d %-12s [%s]" % (pid, name, tag))
        print("      %s" % (re.sub(r"\s+", " ", cmd)[:150]))
    if unregistered:
        print("\n    ** %d leaf/leaves match NO registry row, directly or "
              "through a parent.** Confirm against MONITOR-REGISTRY.md before"
              % len(unregistered))
        print("    killing anything: an unregistered watcher is unkillable by")
        print("    TaskStop, but a mislabelled one is a healthy watcher you")
        print("    are about to take down.")

    if wrappers:
        print("\n  HARNESS WRAPPERS (do NOT kill these to stop a watcher - "
              "use TaskStop):")
        for pid, ppid, name, cmd in sorted(wrappers):
            print("    pid %-7d %-12s wraps a matching child" % (pid, name))

    # ====================================================================
    # THE CROSS-CHECK IS ONLY VALID UNDER THE FULL PATTERN. In `--selftest`
    # the pattern narrows to the control marker, so the four real watchers
    # do not match and every registry row came back "NO RUNNING PROCESS" -
    # **a four-line false alarm announcing that nobody is watching, printed
    # by a run that proved the opposite.** A false alarm is not a harmless
    # over-report: it is how an alarm gets ignored, and this one fired on
    # the most alarming condition the tool can report.
    # The narrowing was the point of the selftest; printing a verdict that
    # depends on the un-narrowed pattern was the bug.
    # ====================================================================
    if reg and not selftest and pattern == DEFAULT_PATTERN:
        running = {p[0] for p in leaves}
        missing = {pid: t for pid, t in reg.items() if pid not in running}
        if missing:
            rc = 1
            print("\n  ** REGISTRY ROWS WITH NO RUNNING PROCESS **")
            print("  A row here is a watcher you believe is running and is "
                  "not. Its silence is UNOBSERVED, not clear:")
            for pid, t in sorted(missing.items()):
                print("    pid %-7d row %s" % (pid, t))

    if proc:
        found = proc.pid in {p[0] for p in leaves}
        proc.kill()
        proc.wait(timeout=10)
        print("\n  SELFTEST: control pid %d %s"
              % (proc.pid, "FOUND as a leaf - census can see"
                 if found else "**NOT FOUND - CENSUS IS BLIND**"))
        return 0 if found else 1

    print()
    print("  %s" % ("every running watcher matches a registry row"
                    if rc == 0 else
                    "**DISCREPANCY - see above. An unregistered watcher is "
                    "unkillable; a missing one means nobody is watching.**"))
    return rc


if __name__ == "__main__":
    sys.exit(main())
