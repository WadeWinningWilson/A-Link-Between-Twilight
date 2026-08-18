"""Snapshot a TU's set of EXACT function names, and diff it against the previous
snapshot. Answers "what did my last change actually do?" by NAME, not by count.

Why this exists: a TU's exact COUNT can stay flat while one function goes green
and another silently breaks. That happened three times in one session on
d_a_npc_ko1 - widening a return type fixed iniTexPttrnAnm and broke CreateHeap,
net zero, invisible in the total and invisible in the per-function score of the
thing I had just edited. Comparing NAME SETS names the casualty immediately.

Usage:
    exact_delta.py <tu>            diff against the saved snapshot, then update it
    exact_delta.py <tu> --init     just write the snapshot (no diff)

Run it from the decomp workspace root.
"""
import json
import os
import subprocess
import sys

OBJDIFF = 'D:/XXXXXXX/tools/objdiff-cli.exe'
SNAP_DIR = os.path.join(os.path.expanduser('~'), '.wwdp_exact_snapshots')


def exact_set(tu):
    rep = os.path.join(SNAP_DIR, '_report.json')
    subprocess.run([OBJDIFF, 'report', 'generate', '-p', '.', '-o', rep],
                   capture_output=True)
    for u in json.load(open(rep))['units']:
        if u['name'].split('/')[-1] == tu:
            return set(f['name'] for f in u.get('functions', [])
                       if float(f.get('fuzzy_match_percent', 0)) == 100.0)
    return set()


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    tu = sys.argv[1]
    init = '--init' in sys.argv
    if not os.path.isdir(SNAP_DIR):
        os.makedirs(SNAP_DIR)
    snap = os.path.join(SNAP_DIR, tu + '.txt')
    now = exact_set(tu)

    if init or not os.path.exists(snap):
        open(snap, 'w').write('\n'.join(sorted(now)))
        print('%s: snapshot written, %d exact' % (tu, len(now)))
        return 0

    prev = set(x for x in open(snap).read().split('\n') if x)
    lost = sorted(prev - now)
    gained = sorted(now - prev)

    print('%s: %d -> %d exact' % (tu, len(prev), len(now)))
    if gained:
        print('  GAINED (%d):' % len(gained))
        for g in gained:
            print('    + %s' % g)
    if lost:
        print('  LOST (%d)  <-- REGRESSIONS, check these before committing:' % len(lost))
        for l in lost:
            print('    - %s' % l)
    if not lost and not gained:
        print('  no change to the exact set')
    # a flat count with churn underneath is the case this tool exists for
    if lost and gained and len(lost) == len(gained):
        print('  NOTE: equal numbers lost and gained - the TOTAL would have looked')
        print('        unchanged. This is exactly the case that hides a regression.')

    open(snap, 'w').write('\n'.join(sorted(now)))
    return 0


if __name__ == '__main__':
    sys.exit(main())
