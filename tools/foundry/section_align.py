#!/usr/bin/env python3
# =============================================================================
# section_align.py - find statics compiled into the WRONG SECTION
# =============================================================================
# A static in the wrong section is INVISIBLE in a per-function instruction diff.
# It shows up somewhere else entirely, as a CONSTANT offset shift in the literal
# pool of unrelated functions. On d_a_npc_aj1 a single `static int` that should
# have been `static const int` put 0x28 bytes in .data instead of .rodata and
# cost six functions their match; adding `const` closed all six at once.
#
# Rule of thumb this instrument encodes:
#   CONSTANT offset delta shared by unrelated functions -> MISSING POOL OBJECT
#   VARYING  offset delta                               -> scheduling/allocation
#
# Usage:  python tools/foundry/section_align.py [TU ...]
#         (run from the decomp workspace; default TU set is the WW NPC lane)
# =============================================================================
import io, os, re, subprocess, sys

OBJDUMP = os.environ.get('WWDP_OBJDUMP', 'build/binutils/powerpc-eabi-objdump.exe')
TARGET  = os.environ.get('WWDP_TARGET', 'GZLE01')
DEFAULT = ['d_a_npc_so', 'd_a_npc_ob1', 'd_a_npc_p2', 'd_a_npc_aj1']


def target_sections(tu):
    """name -> {sections} as declared by the target .s '# .sect:0xNN' comments."""
    path = 'build/%s/%s/asm/d/actor/%s.s' % (TARGET, tu, tu)
    if not os.path.exists(path):
        return None
    out, sec = {}, None
    for line in io.open(path, encoding='utf-8', errors='replace'):
        m = re.match(r'# \.(\w+):0x', line)
        if m:
            sec = m.group(1)
            continue
        m = re.match(r'\.obj "?([\w$@.]+)"?,', line)
        if m and sec:
            out.setdefault(m.group(1), set()).add(sec)
    return out


def my_sections(tu):
    """name -> section, from objdump -t on the built object."""
    obj = 'build/%s/src/d/actor/%s.o' % (TARGET, tu)
    if not os.path.exists(obj):
        return None
    exe = os.path.abspath(OBJDUMP) if os.path.exists(OBJDUMP) else OBJDUMP
    try:
        res = subprocess.run([exe, '-t', obj], capture_output=True, text=True)
    except OSError:
        print('  (objdump not found: %s - set WWDP_OBJDUMP)' % exe)
        return None
    out = {}
    for line in res.stdout.split('\n'):
        m = re.match(r'^[0-9a-f]+ l\s+O \.(\w+)\s+[0-9a-f]+ (\S+)', line)
        if m:
            out[m.group(2)] = m.group(1)
    return out


def main(tus):
    rc = 0
    for tu in tus:
        tgt, mine = target_sections(tu), my_sections(tu)
        if tgt is None or mine is None:
            print('%-16s SKIP (no target .s or no built .o)' % tu)
            continue
        # MWCC appends a $NNNN uniquing counter that differs between builds;
        # match on the base name instead.
        base = lambda n: n.split('$')[0]
        by_base = {}
        for name, secs in tgt.items():
            by_base.setdefault(base(name), set()).update(secs)
        bad = [(n, s, by_base[base(n)]) for n, s in sorted(mine.items())
               if base(n) in by_base and s not in by_base[base(n)]]
        if not bad:
            print('%-16s CLEAN (%d statics checked)' % (tu, len(mine)))
            continue
        rc = 1
        print('%-16s %d MISPLACED' % (tu, len(bad)))
        for n, got, want in bad:
            print('      %-30s mine=.%-9s target=.%s' % (n, got, '/'.join(sorted(want))))
            print('      %-30s -> usually a missing `const` (non-const statics go to .data)' % '')
    return rc


if __name__ == '__main__':
    sys.exit(main(sys.argv[1:] or DEFAULT))
