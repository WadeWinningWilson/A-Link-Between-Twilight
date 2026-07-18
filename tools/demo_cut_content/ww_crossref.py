"""Cross-index every RARC member on the TP disc against a SHA pool of the whole
WW extraction. Finds byte-identical WW assets anywhere in TP (incl. Demo*.arc),
plus name-matched near-misses for 'reworked from WW' candidates."""
import hashlib
import os
import struct
import sys
from collections import defaultdict

TP_ROOTS = [r"D:\XXXXXXX\Ex TP\files\res\Object"]
WW_ROOTS = [r"D:\XXXXXXX\Ex WW\files\res\Object", r"D:\XXXXXXX\Ex WW\files\res\Stage"]

def yaz0_dec(d):
    size = struct.unpack_from(">I", d, 4)[0]
    out = bytearray(size)
    si, di = 16, 0
    code, bits = 0, 0
    while di < size:
        if bits == 0:
            code = d[si]; si += 1; bits = 8
        if code & 0x80:
            out[di] = d[si]; si += 1; di += 1
        else:
            b1, b2 = d[si], d[si + 1]; si += 2
            dist = ((b1 & 0xF) << 8) | b2
            copy_src = di - (dist + 1)
            n = b1 >> 4
            if n == 0:
                n = d[si] + 0x12; si += 1
            else:
                n += 2
            for _ in range(n):
                out[di] = out[copy_src]; di += 1; copy_src += 1
                if di >= size:
                    break
        code <<= 1; bits -= 1
    return bytes(out)

def rarc_members(path):
    """Yield (member_name, sha1, size) for each file member of a RARC archive."""
    with open(path, "rb") as f:
        d = f.read()
    if d[:4] == b"Yaz0":
        try:
            d = yaz0_dec(d)
        except Exception:
            return
    if d[:4] != b"RARC":
        return
    be32 = lambda o: struct.unpack_from(">I", d, o)[0]
    be16 = lambda o: struct.unpack_from(">H", d, o)[0]
    data_abs = 0x20 + be32(0x0C)
    info = 0x20
    num_entries = be32(info + 0x08)
    entries_abs = info + be32(info + 0x0C)
    str_abs = info + be32(info + 0x14)
    for i in range(num_entries):
        e = entries_abs + i * 0x14
        typ = be16(e + 4)
        if typ & 0x1100 != 0x1100:  # files only
            continue
        name_off = be16(e + 6)
        off = be32(e + 8)
        size = be32(e + 12)
        end = d.index(b"\0", str_abs + name_off)
        name = d[str_abs + name_off:end].decode("ascii", "replace")
        blob = d[data_abs + off : data_abs + off + size]
        yield name, hashlib.sha1(blob).hexdigest(), size

def walk_arcs(roots):
    for root in roots:
        for dirpath, _dirs, files in os.walk(root):
            for fn in files:
                if fn.lower().endswith(".arc"):
                    yield os.path.join(dirpath, fn)

# --- Build WW SHA pool (members of all arcs + loose files like .bdl/.bti) ---
ww_sha = {}                    # sha -> "arc/member" (first seen)
ww_names = defaultdict(list)   # lowercase member name -> [(arc, sha, size)]
n_ww_arcs = 0
for path in walk_arcs(WW_ROOTS):
    arc = os.path.basename(path)
    try:
        for name, sha, size in rarc_members(path):
            ww_sha.setdefault(sha, f"{arc}/{name}")
            ww_names[name.lower()].append((arc, sha, size))
        n_ww_arcs += 1
    except Exception as ex:
        print(f"WW parse fail {arc}: {ex}", file=sys.stderr)

print(f"WW pool: {n_ww_arcs} arcs, {len(ww_sha)} unique member SHAs", file=sys.stderr)

# --- Sweep TP arcs ---
KNOWN = {"itemmdl.arc", "Always.arc", "Dalways.arc"}  # already documented
ident_by_arc = defaultdict(list)
name_near = []
n_tp_arcs = 0
for path in walk_arcs(TP_ROOTS):
    arc = os.path.basename(path)
    try:
        for name, sha, size in rarc_members(path):
            if size == 0:
                continue
            if sha in ww_sha:
                ident_by_arc[arc].append((name, size, ww_sha[sha]))
            elif name.lower() in ww_names and name.lower().endswith((".bmd", ".bdl", ".bck", ".btk", ".brk", ".bti")):
                # same distinctive name, different bytes: rework candidate
                warc, wsha, wsize = ww_names[name.lower()][0]
                if abs(size - wsize) <= max(size, wsize) * 0.5:
                    name_near.append((arc, name, size, warc, wsize))
        n_tp_arcs += 1
    except Exception as ex:
        print(f"TP parse fail {arc}: {ex}", file=sys.stderr)

print(f"TP swept: {n_tp_arcs} arcs", file=sys.stderr)

print("\n=== BYTE-IDENTICAL WW members inside TP arcs ===")
for arc in sorted(ident_by_arc):
    tag = "  [known]" if arc in KNOWN else ""
    print(f"\n-- {arc}{tag} ({len(ident_by_arc[arc])} identical member(s))")
    for name, size, wwsrc in sorted(ident_by_arc[arc])[:40]:
        print(f"   {name}  ({size} B)  == WW {wwsrc}")

print(f"\n=== NAME-MATCH near-misses (same member name, different bytes; model/anim/tex only) === ({len(name_near)})")
seen = set()
for arc, name, size, warc, wsize in sorted(name_near)[:120]:
    key = (arc, name)
    if key in seen:
        continue
    seen.add(key)
    print(f"   {arc}/{name} ({size} B)  vs WW {warc} ({wsize} B)")
