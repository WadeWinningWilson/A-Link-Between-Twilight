#!/usr/bin/env python3
# =============================================================================
# float_strictness.py - find NON-STRICT float comparisons you got wrong
# =============================================================================
# MWCC compiles a float `>=` as fcmpo + `cror eq, gt, eq` + branch: it must OR
# the gt and eq bits because IEEE unordered has to be handled. A plain `>`
# compiles to bgt/ble with NO cror at all. Same shape for `cror eq, lt, eq`
# = `<=`.
#
# So a cror-count difference is a SOURCE fact, not an allocation artifact: the
# donor used a non-strict comparison and you used a strict one (or vice versa).
# These are CORRECTNESS bugs - they change behaviour exactly at the boundary.
# On d_a_npc_so this found two, both at the water surface where the minigame's
# state machine lives, which no playtest would reliably surface.
#
# ⚠ WHY THIS COMPARES TOTALS AND NOT PER-FUNCTION COUNTS
# A per-function version of this check produced 18 hits across so/p2, of which
# every single p2 hit was FALSE - it flagged functions objdiff scores at 100%,
# which is impossible. The target .s and the built .o attribute inlined bodies
# and tail functions to different enclosing symbols (tail functions live in
# separate `.text unique` sections), so per-function attribution is not sound.
# The TU total is exact and trustworthy. Localise a real mismatch by hand with
# objdiff, not by trusting a per-function count.
#
# Usage:  python tools/foundry/float_strictness.py [TU ...]
# =============================================================================
import io, os, re, subprocess, sys

OBJDUMP = os.environ.get('WWDP_OBJDUMP', 'build/binutils/powerpc-eabi-objdump.exe')
TARGET  = os.environ.get('WWDP_TARGET', 'GZLE01')
DEFAULT = ['d_a_npc_so', 'd_a_npc_ob1', 'd_a_npc_p2', 'd_a_npc_aj1']


def count(tu):
    asm = 'build/%s/%s/asm/d/actor/%s.s' % (TARGET, tu, tu)
    obj = 'build/%s/src/d/actor/%s.o' % (TARGET, tu)
    if not (os.path.exists(asm) and os.path.exists(obj)):
        return None, None
    tgt = sum(1 for l in io.open(asm, encoding='utf-8', errors='replace') if 'cror' in l)
    exe = os.path.abspath(OBJDUMP) if os.path.exists(OBJDUMP) else OBJDUMP
    try:
        out = subprocess.run([exe, '-d', obj], capture_output=True, text=True).stdout
    except OSError:
        print('  (objdump not found: %s - set WWDP_OBJDUMP)' % exe)
        return None, None
    return tgt, sum(1 for l in out.split('\n') if 'cror' in l)


def main(tus):
    rc = 0
    for tu in tus:
        t, m = count(tu)
        if t is None:
            print('%-16s SKIP (no target .s or no built .o)' % tu)
            continue
        if t == m:
            print('%-16s OK        target=%-3d mine=%-3d' % (tu, t, m))
            continue
        rc = 1
        which = 'MISSING %d non-strict compare(s)' % (t - m) if t > m else \
                'EXTRA %d non-strict compare(s)' % (m - t)
        print('%-16s MISMATCH  target=%-3d mine=%-3d  %s' % (tu, t, m, which))
        print('%-16s           -> you wrote > or < where the donor has >= or <=' % '')
        print('%-16s              (or the reverse). This changes behaviour AT the' % '')
        print('%-16s              boundary. Localise with objdiff, function by' % '')
        print('%-16s              function - do NOT trust a per-function cror count.' % '')
    return rc


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:] or DEFAULT))
