#!/usr/bin/env python3
# ============================================================================
# pool_align.py - IS THE STRING/RODATA POOL PLACEMENT THE SAME AS THE TARGET'S?
#
# WRITTEN BY DECODER, ADOPTED BY FOUNDRY 2026-08-18 (CALLS row 437). Decoder
# generalised the walk that cracked `so` - where remaining diff rows turned out
# to be a placement artifact FOUR separate times - and placed it in this tree
# rather than lose it in a scratch directory, asking explicitly that it be
# ADOPTED rather than assumed. **Adopted. The parsing logic below is theirs and
# is unchanged.** Two cautions they baked in from their own errors survive:
# it compares VALUES not `(offset,size)` (sizes agree trivially when every
# object is a 4-byte float, which produced a false ALIGN), and it is READ-ONLY.
#
# -- WHAT FOUNDRY ADDED, AND WHY IT WAS THE URGENT PART -----------------------
# Decoder's headline result is a NEGATIVE one: `ob1` and `p2` pools ALIGN, so
# the placement well that paid ~36 exact functions on `so` is DRY. **That
# conclusion rests entirely on this tool printing "ALIGN"** - and it feeds a
# PENDING USER RULING on p2.
#
# This lane's standing rule, earned by publishing a push-gate hole that did not
# exist: **a check that can only produce silence must be PROVEN ABLE TO PRODUCE
# NOISE before its silence is read as a result.** An ALIGN from an instrument
# that cannot report divergence is indistinguishable from an ALIGN from one
# that can.
#
# So the two comparisons are lifted into pure functions - `cmp_strings` and
# `cmp_rodata`, called by the same main path as before - and `--control` feeds
# them synthetic pools with a PLANTED divergence in each, requiring both be
# caught. **The dry-well finding is admissible once this passes. It was not
# before, through no fault of the measurement itself.**
#
# Usage:
#   pool_align.py <tu>        - run against the decomp workspace (cwd = decomp)
#   pool_align.py --control   - prove it can report divergence, then exit
# Exit 0 ran / control passed - 1 CONTROL FAILED, do not trust an ALIGN.
# ============================================================================
import re, struct, subprocess, sys


# ============================================================================
# FOUNDRY: Decoder's comparison logic, lifted VERBATIM into callable form so a
# control can exercise it. Behaviour is unchanged; only the wrapper is new.
# Each returns True when a DIVERGENCE was reported - which is what the control
# asserts, and what an "ALIGN" is the absence of.
# ============================================================================
def cmp_strings(tstr, mstr):
    print('STRINGS: target %d | mine %d' % (len(tstr), len(mstr)))
    bad = False
    for k in range(max(len(tstr), len(mstr))):
        a = tstr[k] if k < len(tstr) else None
        b2 = mstr[k] if k < len(mstr) else None
        if a != b2:
            print('  DIVERGE at %d:' % k)
            for j in range(max(0, k - 2), min(max(len(tstr), len(mstr)), k + 6)):
                print('    T %-26s | M %s' % (tstr[j] if j < len(tstr) else '',
                                              mstr[j] if j < len(mstr) else ''))
            bad = True
            break
    if not bad:
        print('  strings ALIGN')
    return bad


def cmp_rodata(tgt, w):
    ok = 0
    for t in tgt:
        off, sz, vals = t
        mv = [w.get(off + 4 * k, '????') for k in range(len(vals))]
        if mv != vals:
            print('RODATA: aligns %d/%d objects; FIRST DIVERGENCE 0x%04X'
                  % (ok, len(tgt), off))
            idx = tgt.index(t)
            for t2 in tgt[max(0, idx - 2):idx + 5]:
                o = t2[0]
                print('   0x%04X T %-20s M %s'
                      % (o, ' '.join(t2[2])[:20],
                         ' '.join(w.get(o + 4 * k, '????')
                                  for k in range(len(t2[2])))[:20]))
            return True
        ok += 1
    print('RODATA: FULLY ALIGNED (%d objects)' % ok)
    return False


def control():
    """Plant a divergence in each comparison and require BOTH be reported.

    The rodata specimen is deliberately Decoder's own trap: SAME offsets, SAME
    sizes, one VALUE changed. A tool comparing `(offset,size)` would call that
    aligned - which is the false ALIGN they hit and fixed."""
    print('NEGATIVE CONTROL - planting a divergence in each comparison')
    print('-- strings --')
    s_clean = cmp_strings(['a', 'b', 'c'], ['a', 'b', 'c'])
    s_dirty = cmp_strings(['a', 'b', 'c'], ['a', 'b', 'XX'])
    print('-- rodata: same offsets and sizes, one VALUE changed --')
    tgt = [[0x00, 4, ['3F800000']], [0x04, 4, ['40000000']]]
    r_clean = cmp_rodata(tgt, {0x00: '3F800000', 0x04: '40000000'})
    r_dirty = cmp_rodata(tgt, {0x00: '3F800000', 0x04: 'DEADBEEF'})
    ok = (not s_clean) and s_dirty and (not r_clean) and r_dirty
    print()
    if ok:
        print('  CONTROL PASSED - clean pools report ALIGN, planted divergences')
        print('  are CAUGHT in both comparisons. An ALIGN from this tool is now')
        print('  a RESULT rather than an untested silence.')
        return 0
    print('  *** CONTROL FAILED (s_clean=%s s_dirty=%s r_clean=%s r_dirty=%s)'
          % (s_clean, s_dirty, r_clean, r_dirty))
    print('      DO NOT trust an ALIGN from this build. ***')
    return 1


if '--control' in sys.argv:
    sys.exit(control())

# ============================================================================
# DECODER'S ORIGINAL BODY BELOW, UNCHANGED except that the two comparison
# blocks now call the functions above instead of inlining them.
# ============================================================================
tu = sys.argv[1]                      # e.g. d_a_npc_ob1
asm = 'build/GZLE01/%s/asm/d/actor/%s.s' % (tu, tu)
obj = 'build/GZLE01/src/d/actor/%s.o' % tu
OD = './build/binutils/powerpc-eabi-objdump.exe'


def f2h(t):
    t = t.strip()
    if t.startswith('0x'):
        return t[2:].upper().zfill(8)
    try:
        return '%08X' % struct.unpack('>I', struct.pack('>f', float(t)))[0]
    except Exception:
        return t


src = open(asm, encoding='utf-8', errors='ignore').read()
lines = src.splitlines()

# ---- string pool ----
i = src.find('.obj "@stringBase0"')
tstr = []
if i >= 0:
    seg = src[i:src.find('.endobj "@stringBase0"', i)]
    for ln in seg.splitlines():
        ln = ln.strip()
        if ln.startswith('.string'):
            v = ln[len('.string'):].strip()
            if v.startswith('"') and v.endswith('"'):
                tstr.append(v[1:-1])

dump = subprocess.run([OD, '-s', '-j', '.rodata', obj], capture_output=True, text=True).stdout
by, w = {}, {}
for ln in dump.splitlines():
    # objdump -s pads the hex area to a fixed 35 cols, then TWO spaces, then ASCII.
    # The final line of a section is often a PARTIAL word (e.g. '3100'); matching
    # only 8-hex-digit groups silently drops those bytes and truncates the last
    # string in the pool. Take the hex area positionally instead.
    m = re.match(r'\s*([0-9a-f]{4,8}) (.{1,35}?)(?:  |$)', ln)
    if m:
        b = int(m.group(1), 16)
        h = ''.join(m.group(2).split())
        if not re.fullmatch(r'[0-9a-f]*', h):
            continue
        for k in range(len(h) // 2):
            by[b + k] = int(h[2 * k:2 * k + 2], 16)
        for k in range(len(h) // 8):
            w[b + 4 * k] = h[8 * k:8 * k + 8].upper()

tab = subprocess.run([OD, '-t', '-j', '.rodata', obj], capture_output=True, text=True).stdout
base = size = None
for ln in tab.splitlines():
    if '@stringBase0' in ln:
        p = ln.split()
        base, size = int(p[0], 16), int(p[-2], 16)
mstr = []
if base is not None:
    raw = bytes(by.get(base + k, 0) for k in range(size))
    mstr = [x.decode('latin1') for x in raw.split(b'\x00') if x]

diverged_s = cmp_strings(tstr, mstr)

# ---- rodata values ----
start = next((k for k, l in enumerate(lines) if l.strip() == '.rodata'), None)
if start is None:
    print('RODATA: no .rodata in target asm'); sys.exit()
tgt, cur = [], None
for ln in lines[start:]:
    if ln.strip().startswith('.section') and 'rodata' not in ln:
        break
    m = re.match(r'# \.rodata:0x([0-9A-Fa-f]+) \| .* size: 0x([0-9A-Fa-f]+)', ln)
    if m:
        cur = [int(m.group(1), 16), int(m.group(2), 16), []]
        tgt.append(cur)
        continue
    if cur is not None:
        m2 = re.match(r'\s*\.(4byte|float)\s+(.*)', ln)
        if m2:
            cur[2].append(f2h(m2.group(2)))
tgt = [t for t in tgt if t[1] and t[2]]
diverged_r = cmp_rodata(tgt, w)
