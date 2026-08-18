#!/usr/bin/env python3
# ============================================================================
# census_join.py — name the plugin's census pointers from the loader's own
# create lines, in one pass.
#
# WHY THIS EXISTS (2026-08-16): reading runs 143410/144259 meant hand-joining
# `create_census` / `create_born_dead` / `drawq_*` pointers against
# `fpcBs_Create` lines — three times in one session, and the first attempt
# silently matched NOTHING because the two sides print pointers differently:
#   plugin  %p (MSVC):  00000187D49CD1E0   (zero-padded, uppercase, no 0x)
#   loader          :  0x187d49cd1e0      (0x-prefixed, lowercase, trimmed)
# A join that normalizes both sides is 20 lines; a human doing it under time
# pressure is a format bug factory. This is the 20 lines.
#
# JOIN SHAPE: `fpcBs_Create: pid=N profname=X ...` is immediately followed by
# `fpcBs_Create OK proc=0x...` — the pair is (profname, proc). The plugin
# events carry proc/tag pointers. Normalize to int, join, report:
#   loopers (census first_loopers)  -> names
#   born-dead (create_born_dead)    -> names
#   transitions (create_status)     -> names + from/to
#   drawq admissions/cuts           -> tag ptrs are NOT proc ptrs; reported
#                                      unresolved rather than mis-joined.
# Read-only. Usage: census_join.py <boot-log> [--selftest <boot-log>]
# ============================================================================
import io
import re
import sys


def norm(p):
    """Pointer string of either format -> int, or None."""
    try:
        return int(p, 16)
    except (ValueError, TypeError):
        return None


def load(path):
    text = io.open(path, encoding="utf-8", errors="replace").read()

    # (profname, proc) pairs from the loader's paired lines.
    names = {}
    pending = None
    for line in text.splitlines():
        m = re.search(r"fpcBs_Create: pid=(\d+) profname=(\S+)", line)
        if m:
            pending = (int(m.group(1)), m.group(2))
            continue
        m = re.search(r"fpcBs_Create OK proc=(0x[0-9a-f]+)", line)
        if m and pending is not None:
            names[norm(m.group(1))] = pending
            pending = None
    return text, names


def name_of(names, ptr):
    v = norm(ptr)
    if v in names:
        pid, prof = names[v]
        return "%s(pid=%d)" % (prof, pid)
    return "UNNAMED(%s)" % ptr


def main():
    if len(sys.argv) < 2:
        print(__doc__ or "usage: census_join.py <boot-log>")
        return 2
    path = sys.argv[-1]
    text, names = load(path)
    print("loader create pairs: %d" % len(names))

    # Loopers out of the last census line.
    m = None
    for m in re.finditer(r'"first_loopers":"([0-9A-Fa-f,]+)"', text):
        pass
    if m:
        print("\nINIT-LOOPERS (never drawable):")
        for p in m.group(1).split(","):
            print("  %s  ->  %s" % (p, name_of(names, p)))

    born = re.findall(r'"ev":"create_born_dead".*?"proc":"([0-9A-Fa-f]+)"', text)
    if born:
        print("\nBORN-DEAD (ERROR on first create; silently deleted):")
        for p in born:
            print("  %s  ->  %s" % (p, name_of(names, p)))

    trans = re.findall(
        r'"ev":"create_status","proc":"([0-9A-Fa-f]+)","from":(\d+),"to":(\d+)', text)
    if trans:
        print("\nTRANSITIONS:")
        for p, a, b in trans:
            print("  %s  %s->%s  %s" % (p, a, b, name_of(names, p)))

    drawn = re.findall(r'"ev":"dw_exec","ww":1,"distinct":\d+,"proc":"([0-9A-Fa-f]+)"', text)
    if drawn:
        print("\nWW PROCS ACTUALLY DRAWN:")
        for p in sorted(set(drawn)):
            print("  %s  ->  %s" % (p, name_of(names, p)))

    wipes = len(re.findall(r'"ev":"drawq_wipe"', text))
    cuts = len(re.findall(r'"ev":"drawq_cut".*?"ww_admitted_tag":1', text))
    print("\nremoval side: %d wipe(s), %d cut(s) of WW-admitted tags "
          "(tag ptrs are draw tags, not proc ptrs - no join attempted)" % (wipes, cuts))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
