import re, struct, subprocess, sys

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

print('STRINGS: target %d | mine %d' % (len(tstr), len(mstr)))
bad = False
for k in range(max(len(tstr), len(mstr))):
    a = tstr[k] if k < len(tstr) else None
    b2 = mstr[k] if k < len(mstr) else None
    if a != b2:
        print('  DIVERGE at %d:' % k)
        for j in range(max(0, k - 2), min(max(len(tstr), len(mstr)), k + 6)):
            print('    T %-26s | M %s' % (tstr[j] if j < len(tstr) else '', mstr[j] if j < len(mstr) else ''))
        bad = True
        break
if not bad:
    print('  strings ALIGN')

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
ok = 0
for t in tgt:
    off, sz, vals = t
    mv = [w.get(off + 4 * k, '????') for k in range(len(vals))]
    if mv != vals:
        print('RODATA: aligns %d/%d objects; FIRST DIVERGENCE 0x%04X' % (ok, len(tgt), off))
        idx = tgt.index(t)
        for t2 in tgt[max(0, idx - 2):idx + 5]:
            o = t2[0]
            print('   0x%04X T %-20s M %s' % (o, ' '.join(t2[2])[:20],
                                              ' '.join(w.get(o + 4 * k, '????') for k in range(len(t2[2])))[:20]))
        break
    ok += 1
else:
    print('RODATA: FULLY ALIGNED (%d objects)' % ok)
