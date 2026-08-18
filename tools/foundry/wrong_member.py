#!/usr/bin/env python3
# =============================================================================
# wrong_member.py - find WRONG STRUCT MEMBER reads (a correctness bug class)
# =============================================================================
# The highest-value defects this decomp campaign produced were not row counts -
# they were functions that read the WRONG FIELD and matched everywhere else:
#
#   so::lookBack          shape_angle.y (0x20E) where the donor reads
#                         current.angle.y (0x206)
#   p2::cutJumpStart      children[].field_0x3c where the donor reads
#   + cutJumpToGoalStart  field_0x30 - two ADJACENT cXyz. field_0x7e8 is the
#                         jump DESTINATION, so the fallback jump went to the
#                         wrong configured point.
#
# Signature: ONE row where target and build share the same mnemonic and the
# same registers and differ ONLY in the displacement. A register allocator
# never changes which byte you load, so that is a SOURCE fact.
#
# THREE FILTERS, all of which I needed - the unfiltered version was mostly noise
# (14 hits, of which 13 were pool or stack position):
#   1. r1 is the STACK POINTER, never a struct. Those rows are stack-slot
#      ordering - a different, non-correctness bucket.
#   2. A shared delta across several rows is a pool/stack SHIFT, not a member
#      bug. aj1::_create had four rows all +0xc: a missing pool object.
#   3. DECISIVE: resolve both displacements in .rodata. If the two sides hold
#      the SAME VALUE it is literal-pool POSITION - the same constant at a
#      different offset. so::_draw looked like a clean lone hit and was exactly
#      this: both sides held 0x44480000 (800.0f).
#
# Usage:  python tools/foundry/wrong_member.py [TU ...]
# =============================================================================
import io
import json
import os
import re
import struct
import subprocess
import sys
from collections import Counter

OBJDIFF = os.environ.get('WWDP_OBJDIFF', 'D:/XXXXXXX/tools/objdiff-cli.exe')
OBJDUMP = os.environ.get('WWDP_OBJDUMP', 'build/binutils/powerpc-eabi-objdump.exe')
TARGET = os.environ.get('WWDP_TARGET', 'GZLE01')
DEFAULT = ['d_a_npc_so', 'd_a_npc_ob1', 'd_a_npc_p2', 'd_a_npc_aj1']
MEMREF = re.compile(r'^(\w+)\s+(\S+),\s*(-?0x[0-9a-fA-F]+|-?\d+)\((\w+)\)$')
TMP = os.environ.get('TEMP') or '/tmp'


def residuals(tu):
    rep = os.path.join(TMP, 'wm_report.json')
    subprocess.run([OBJDIFF, 'report', 'generate', '-p', '.', '-o', rep],
                   capture_output=True)
    if not os.path.exists(rep):
        return []
    for u in json.load(open(rep))['units']:
        if u['name'].split('/')[-1] == tu:
            return [f['name'] for f in u.get('functions', [])
                    if float(f.get('fuzzy_match_percent', 0)) < 100.0]
    return []


def rows(unit, sym):
    out = os.path.join(TMP, 'wm_diff.json')
    if os.path.exists(out):
        os.remove(out)
    subprocess.run([OBJDIFF, 'diff', '-p', '.', '-u', unit, '-o', out, sym],
                   capture_output=True)
    if not os.path.exists(out):
        return []
    d = json.load(open(out))

    def side(s):
        m = [x for x in d[s]['symbols'] if x.get('name') == sym]
        return m[0]['instructions'] if m else []

    left, right, res = side('left'), side('right'), []
    for i in range(max(len(left), len(right))):
        a = left[i] if i < len(left) else {}
        b = right[i] if i < len(right) else {}
        if (a.get('diff_kind') in (None, '', 'none')
                and b.get('diff_kind') in (None, '', 'none')):
            continue
        res.append((a.get('instruction', {}).get('formatted', ''),
                    b.get('instruction', {}).get('formatted', '')))
    return res


def rodata_target(path):
    """offset -> 4-byte word, parsed from the target .s section comments."""
    out, sec, off = {}, None, None
    if not os.path.exists(path):
        return out
    for line in io.open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'# \.(\w+):(0x[0-9A-Fa-f]+)', line)
        if m:
            sec, off = m.group(1), int(m.group(2), 16)
            continue
        if sec != 'rodata' or off is None:
            continue
        m = re.match(r'\s*\.4byte (0x[0-9A-Fa-f]+)', line)
        if m:
            out[off] = '%08x' % int(m.group(1), 0)
            off += 4
            continue
        m = re.match(r'\s*\.float (\S+)', line)
        if m:
            out[off] = struct.pack('>f', float(m.group(1))).hex()
            off += 4
    return out


def rodata_mine(obj):
    """offset -> 4-byte word, from objdump -s on the built object."""
    out = {}
    if not os.path.exists(obj):
        return out
    exe = os.path.abspath(OBJDUMP) if os.path.exists(OBJDUMP) else OBJDUMP
    try:
        dump = subprocess.run([exe, '-s', '-j', '.rodata', obj],
                              capture_output=True, text=True).stdout
    except OSError:
        return out
    for line in dump.split('\n'):
        m = re.match(r'^\s*([0-9a-f]+)\s+((?:[0-9a-f]{8}\s+){1,4})', line)
        if not m:
            continue
        base = int(m.group(1), 16)
        for i, word in enumerate(m.group(2).split()):
            out[base + i * 4] = word.lower()
    return out


def main(tus):
    cand = []
    for tu in tus:
        unit = '%s/d/actor/%s' % (tu, tu)
        for fn in residuals(tu):
            rs = rows(unit, fn)
            for t, m in rs:
                mt, mm = MEMREF.match(t.strip()), MEMREF.match(m.strip())
                if not (mt and mm):
                    continue
                same_shape = (mt.group(1) == mm.group(1)
                              and mt.group(2) == mm.group(2)
                              and mt.group(4) == mm.group(4))
                if not (same_shape and mt.group(3) != mm.group(3)):
                    continue
                if mt.group(4) == 'r1':
                    continue
                cand.append((tu, fn, len(rs), t.strip(), m.strip(),
                             int(mt.group(3), 0) - int(mm.group(3), 0)))

    per_fn = Counter((c[0], c[1], c[5]) for c in cand)
    pools, hits, suppressed = {}, [], 0
    for tu, fn, n, t, m, delta in cand:
        if per_fn[(tu, fn, delta)] > 1:
            suppressed += 1
            continue
        if tu not in pools:
            pools[tu] = (
                rodata_target('build/%s/%s/asm/d/actor/%s.s' % (TARGET, tu, tu)),
                rodata_mine('build/%s/src/d/actor/%s.o' % (TARGET, tu)))
        tgt_pool, my_pool = pools[tu]
        t_off = int(MEMREF.match(t).group(3), 0)
        m_off = int(MEMREF.match(m).group(3), 0)
        if t_off in tgt_pool and m_off in my_pool and tgt_pool[t_off] == my_pool[m_off]:
            suppressed += 1
            continue
        hits.append((n, tu, fn, t, m))

    if suppressed:
        print('(%d row(s) suppressed: shared delta, or both sides hold the same '
              '.rodata value -> pool/stack position, not a member bug)' % suppressed)
    if not hits:
        print('No wrong-member candidates.')
        return 0
    hits.sort()
    print('%-14s %-42s %s' % ('TU', 'FUNCTION', 'target   |   mine'))
    for n, tu, fn, t, m in hits:
        print('%-14s %-42s %s   |   %s   [%d row(s) in fn]'
              % (tu, fn[:42], t, m, n))
    print('')
    print('%d candidate wrong-member read(s). Check the struct: ADJACENT '
          'members of the same type are the usual cause.' % len(hits))
    print('')
    print('READ THE ROW-COUNT COLUMN AS A CONFIDENCE SCORE. objdiff pairs '
          'instructions BY POSITION, so in a function that already has '
          'insertions or deletions the two sides can be MISPAIRED and a '
          '"candidate" is then meaningless. A 1-row function is trustworthy '
          '(both real bugs found this way were 1-row); a 10+-row function '
          'needs the pairing confirmed by eye before you touch anything.')
    return 1


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:] or DEFAULT))
