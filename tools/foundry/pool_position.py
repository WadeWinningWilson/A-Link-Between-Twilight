"""Classify a TU's residual functions: LOGIC-EXACT (every diff row is only a
constant-pool displacement) vs REAL (anything else).

Rationale: ko1 is being written out of order, so its .rodata pool ordering does
not yet match the target's. Functions whose ONLY disagreement is 'T lfs f0,
0x140(r31) | M lfs f0, 0x34(r31)' are already correct C++ and will close for
free when the TU is complete. Counting them separately keeps the exact-count
from understating how much of the TU is actually done - and, more importantly,
stops a later sweep re-opening them as defects.
"""
import json
import os
import re
import subprocess
import sys

OBJDIFF = 'D:/XXXXXXX/tools/objdiff-cli.exe'
TU = sys.argv[1] if len(sys.argv) > 1 else 'd_a_npc_ko1'
TMP = 'C:/Users/xxxxx/AppData/Local/Temp'
# a pool reference: <mnem> <reg>, <disp>(<base>)
POOL = re.compile(r'^(lfs|lfd|lwz|lha|lhz|lbz)\s+(\S+),\s*(-?0x[0-9a-f]+)\((r\d+)\)$')
# A string-pool reference is 'addi rA, rB, <offset-into-the-string-blob>'.
# Same shape of artifact as a float-pool load: the offset is TU-completion
# state, not a defect. event_actionInit's only residual row is exactly this.
SPOOL = re.compile(r'^(addi)\s+(\S+),\s*(r\d+),\s*(-?0x[0-9a-f]+)$')


def residuals():
    rep = os.path.join(TMP, 'po_rep.json')
    subprocess.run([OBJDIFF, 'report', 'generate', '-p', '.', '-o', rep],
                   capture_output=True)
    for u in json.load(open(rep))['units']:
        if u['name'].split('/')[-1] == TU:
            return [(f['name'], float(f.get('fuzzy_match_percent', 0)))
                    for f in u.get('functions', [])
                    if float(f.get('fuzzy_match_percent', 0)) < 100.0]
    return []


def rows(sym):
    out = os.path.join(TMP, 'po_diff.json')
    if os.path.exists(out):
        os.remove(out)
    subprocess.run([OBJDIFF, 'diff', '-p', '.', '-u',
                    '%s/d/actor/%s' % (TU, TU), '-o', out, sym],
                   capture_output=True)
    if not os.path.exists(out):
        return None
    d = json.load(open(out))

    def side(s):
        m = [x for x in d[s]['symbols'] if x.get('name') == sym]
        return m[0]['instructions'] if m else []

    l, r = side('left'), side('right')

    def fmt(x):
        return x.get('instruction', {}).get('formatted', '')

    def changed(x):
        return x.get('diff_kind') not in (None, '', 'none')

    # objdiff ALREADY knows which differences are cosmetic - branch targets are
    # absolute addresses and pool symbols are renamed, both of which differ on
    # every single row. Selecting rows by raw string inequality throws that
    # knowledge away and reports every function as broken. Select by diff_kind,
    # exactly as the trusted per-function differ does.
    diffs = []
    for i in range(max(len(l), len(r))):
        a = l[i] if i < len(l) else {}
        b = r[i] if i < len(r) else {}
        if changed(a) or changed(b):
            diffs.append((fmt(a), fmt(b)))

    if not diffs:
        return diffs, False
    for a, b in diffs:
        ma, mb = POOL.match(a), POOL.match(b)
        if ma and mb:
            # same mnemonic, same dest reg, same base reg: ONLY the pool
            # displacement differs, which is TU-completion state, not a defect
            if (ma.group(1), ma.group(2), ma.group(4)) != (mb.group(1), mb.group(2), mb.group(4)):
                return diffs, False
            continue
        sa, sb = SPOOL.match(a), SPOOL.match(b)
        if sa and sb:
            if (sa.group(2), sa.group(3)) != (sb.group(2), sb.group(3)):
                return diffs, False
            continue
        return diffs, False
    return diffs, True


pool_only, real = [], []
for sym, pct in residuals():
    got = rows(sym)
    if not got:
        continue
    diffs, is_pool = got
    (pool_only if is_pool else real).append((sym, len(diffs), pct))

print('%s residuals: %d LOGIC-EXACT (pool position only), %d REAL\n'
      % (TU, len(pool_only), len(real)))
print('POOL-POSITION ONLY - the body is correct; the offsets are TU-state.')
print('NOTE: NOT guaranteed to close by themselves. They cannot be fixed by')
print('editing the function, but a delta still present at TU completion means')
print('a real pool-ORDERING defect - so re-check these at the end.')
for s, n, p in sorted(pool_only, key=lambda x: -x[1]):
    print('  %-46s %2d rows  %.4f%%' % (s[:46], n, p))
print('')
print('REAL - a defect, or simply an unwritten stub:')
for s, n, p in sorted(real, key=lambda x: -x[1])[:25]:
    print('  %-46s %2d rows  %.4f%%' % (s[:46], n, p))
if len(real) > 25:
    print('  ... and %d more' % (len(real) - 25))
