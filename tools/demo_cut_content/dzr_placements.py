"""Count retail stage placements of given 8-char actor names by scanning DZR/DZS
members inside every stage arc (Yaz0-aware). Names in ACTR-family chunks are
8-byte null-padded, so `name\0` byte-search with an alignment sanity pass is a
reliable presence test; counts are occurrences of the padded pattern.
Usage: python dzr_placements.py E_ms E_gs [control...]"""
import os
import struct
import sys

STAGE_ROOT = r"D:\XXXXXXX\Ex TP\files\res\Stage"

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
            src = di - (dist + 1)
            n = b1 >> 4
            if n == 0:
                n = d[si] + 0x12; si += 1
            else:
                n += 2
            for _ in range(n):
                out[di] = out[src]; di += 1; src += 1
                if di >= size:
                    break
        code <<= 1; bits -= 1
    return bytes(out)

def rarc_members(path):
    with open(path, "rb") as f:
        d = f.read()
    if d[:4] == b"Yaz0":
        d = yaz0_dec(d)
    if d[:4] != b"RARC":
        return
    be32 = lambda o: struct.unpack_from(">I", d, o)[0]
    be16 = lambda o: struct.unpack_from(">H", d, o)[0]
    data_abs = 0x20 + be32(0x0C)
    info = 0x20
    n = be32(info + 0x08)
    ent = info + be32(info + 0x0C)
    strs = info + be32(info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        if be16(e + 4) & 0x1100 != 0x1100:
            continue
        name_off = be16(e + 6)
        off, size = be32(e + 8), be32(e + 12)
        end = d.index(b"\0", strs + name_off)
        name = d[strs + name_off:end].decode("ascii", "replace")
        yield name, d[data_abs + off:data_abs + off + size]

# ============================================================
# Main sweep — guarded so helper functions are importable
# without re-running the scan (earlier runs polluted importer
# output because this block executed at module level).
# ============================================================
def main():
    targets = sys.argv[1:] or ["E_ms", "E_gs", "E_oc"]
    pats = {t: (t.encode("ascii") + b"\0" * (8 - len(t))) for t in targets}
    hits = {t: {} for t in targets}

    for stage in sorted(os.listdir(STAGE_ROOT)):
        sdir = os.path.join(STAGE_ROOT, stage)
        if not os.path.isdir(sdir):
            continue
        for fn in sorted(os.listdir(sdir)):
            if not fn.lower().endswith(".arc"):
                continue
            try:
                for mname, blob in rarc_members(os.path.join(sdir, fn)):
                    if not mname.lower().endswith((".dzr", ".dzs")):
                        continue
                    for t, pat in pats.items():
                        c = blob.count(pat)
                        if c:
                            hits[t].setdefault(stage, []).append(f"{fn}/{mname} x{c}")
            except Exception as ex:
                print(f"parse fail {stage}/{fn}: {ex}", file=sys.stderr)

    for t in targets:
        total = sum(int(e.rsplit("x", 1)[1]) for v in hits[t].values() for e in v)
        print(f"\n== {t}: {total} placement pattern hit(s) across {len(hits[t])} stage(s)")
        for stage, entries in sorted(hits[t].items()):
            print(f"   {stage}: {', '.join(entries)}")

if __name__ == "__main__":
    main()
