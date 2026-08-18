"""Decode a switch's jump table from target asm into ready-to-paste C++ cases.

Two facts live in a MWCC jump table and they are easy to conflate:
  * the TABLE gives the case VALUES  (index i  ->  value base + i)
  * the LABEL ADDRESSES give the SOURCE ORDER, because MWCC emits case bodies in
    the order they were written while the table is indexed by value.
Recover only the values and you get a switch that builds cleanly and scores
badly with nothing to point at. On d_a_npc_ko1::next_msgStatus the written order
starts 0xAF4, 0xAF5, 0xB59, 0xB5A and only reaches 0xAF6 as the twelfth group -
not something anyone reconstructs by reading values.

It also surfaces two shapes that are invisible when transcribing by eye:
indices sharing one label are FALL-THROUGH case lists, and a label whose body is
not a simple store is flagged rather than guessed at.

Usage:
    jump_table.py <tu> <mangled_symbol> [--base 0xAF4]

--base is the switch's low bound; without it the tool prints indices and you
supply the base from the 'subi rX, rY, <base>' ahead of the bounds check.
Run from the decomp workspace root.
"""
import io
import re
import sys

def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 2
    tu, sym = sys.argv[1], sys.argv[2]
    base = 0
    if '--base' in sys.argv:
        base = int(sys.argv[sys.argv.index('--base') + 1], 0)

    asm = 'build/GZLE01/%s/asm/d/actor/%s.s' % (tu, tu)
    L = io.open(asm, encoding='utf-8', errors='replace').read().split('\n')
    strip = lambda l: re.sub(r'/\*[^*]*\*/', '', l)

    body, on = [], False
    for l in L:
        if l.startswith('.fn %s' % sym):
            on = True
        if on:
            body.append(strip(l))
        if l.startswith('.endfn %s' % sym):
            break
    if not body:
        print('symbol not found: %s  (check the mangled name - a typo here')
        print('reports an EMPTY diff, which reads exactly like a perfect match)')
        return 1

    tbls = sorted(set(re.findall(r'"(@\d+)"@ha', '\n'.join(body))))
    # label -> its leading instructions
    lab, acts = None, {}
    for l in body:
        s = l.strip()
        if s.startswith('.L_') and s.endswith(':'):
            lab = s[:-1]
            acts[lab] = []
        elif lab and s:
            acts[lab].append(s)

    for t in tbls:
        idx = [k for k, l in enumerate(L) if l.startswith('.obj "%s"' % t)]
        if not idx:
            continue
        rels = []
        for l in L[idx[0] + 1:]:
            if l.startswith('.endobj'):
                break
            m = re.search(r',\s*(\.L_[0-9A-Fa-f]+)', l)
            if m:
                rels.append(m.group(1))
        if not rels:
            continue
        freq = {}
        for r in rels:
            freq[r] = freq.get(r, 0) + 1
        # the default is the most repeated label - but ONLY if it actually
        # repeats; when every entry is distinct there is no default and calling
        # one of them 'default' silently drops a real case.
        default = max(freq, key=lambda k: freq[k]) if max(freq.values()) > 1 else None

        bylab = {}
        for i, r in enumerate(rels):
            if r == default:
                continue
            bylab.setdefault(r, []).append(base + i)

        print('=== %s: %d entries, %d distinct bodies, default=%s'
              % (t, len(rels), len(bylab), default or 'NONE (every entry distinct)'))
        print('--- cases in SOURCE order (sorted by label address) ---')
        for lb in sorted(bylab):
            ins = acts.get(lb, [])
            txt = ' | '.join(ins[:3])
            m = re.match(r'li r0, (0x[0-9a-f]+)\s*\|\s*stw r0, 0x0\(r\d+\)', txt)
            vals = ' '.join('0x%X' % v for v in bylab[lb])
            if m:
                print('  case %s:  -> store 0x%X' % (vals, int(m.group(1), 16)))
            else:
                print('  case %s:  -> NOT a simple store, read it: %s' % (vals, txt[:80]))
            if len(bylab[lb]) > 1:
                print('        ^ %d values share this body: FALL-THROUGH case list'
                      % len(bylab[lb]))
    return 0


if __name__ == '__main__':
    sys.exit(main())
