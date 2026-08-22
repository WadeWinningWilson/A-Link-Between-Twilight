#!/usr/bin/env python3
# ============================================================================
# ww_dzs_kankyo.py -- OFFLINE ORACLE for the WW stage kankyo chunks.
#
# WHY THIS EXISTS: the "WW grass and flowers are black" defect was being
# treated as boot-blocked -- read the runtime receipts, see what the overlay
# wrote. But "what colour does the donor's own stage data carry" is a DATA
# question, and the data is on the user's disc. This answers it without a run.
#
# It reads the SAME BYTES THE PLUGIN READS, deliberately: the Pale entry
# layout below is transcribed from `WwPalet` in src/ww/d/d_kankyo.cpp:48-66
# (static_assert sizeof == 0x2C, "donor stage_palet_info_class"), and the
# chunk tags from the same file's kTagPale/kTagVirt/kTagEnvR/kTagColo. If the
# plugin's struct is wrong, this tool is wrong in the SAME direction -- which
# is a limit worth naming rather than a claim of independence.
#
# Usage: python ww_dzs_kankyo.py <Stage.arc|Room*.arc> [...]
# ============================================================================
import struct
import sys
import os


def yaz0(src):
    out_size = struct.unpack_from('>I', src, 4)[0]
    out = bytearray()
    p, code, bits = 16, 0, 0
    while len(out) < out_size:
        if bits == 0:
            code = src[p]
            p += 1
            bits = 8
        if code & 0x80:
            out.append(src[p])
            p += 1
        else:
            b1, b2 = src[p], src[p + 1]
            p += 2
            dist = ((b1 & 0x0F) << 8) | b2
            start = len(out) - (dist + 1)
            n = b1 >> 4
            if n == 0:
                n = src[p] + 0x12
                p += 1
            else:
                n += 2
            for i in range(n):
                out.append(out[start + i])
        code <<= 1
        bits -= 1
    return bytes(out)


def rarc_files(data):
    entry_off = struct.unpack_from('>I', data, 0x2C)[0] + 0x20
    num_entries = struct.unpack_from('>I', data, 0x28)[0]
    data_off = struct.unpack_from('>I', data, 0x0C)[0] + 0x20
    str_off = struct.unpack_from('>I', data, 0x34)[0] + 0x20
    out = []
    for i in range(num_entries):
        e = entry_off + i * 0x14
        if data[e + 0x04] & 0x02:
            continue
        nameoff = struct.unpack_from('>H', data, e + 0x06)[0]
        off = struct.unpack_from('>I', data, e + 0x08)[0]
        size = struct.unpack_from('>I', data, e + 0x0C)[0]
        end = data.find(b'\x00', str_off + nameoff)
        out.append((data[str_off + nameoff:end].decode('shift_jis', 'replace'),
                    data_off + off, size))
    return out


# WwPalet, src/ww/d/d_kankyo.cpp:48. color_RGB_class is 3 bytes; the field
# ORDER is what decides which slot the BG draw consumes, so it is spelled out
# rather than indexed by arithmetic.
PALE_FIELDS = [('actor_c0', 0), ('actor_k0', 3),
               ('bg0_c0', 6), ('bg0_k0', 9),
               ('bg1_c0', 12), ('bg1_k0', 15),
               ('bg2_c0', 18), ('bg2_k0', 21),
               ('bg3_c0', 24), ('bg3_k0', 27),
               ('fog', 30)]
PALE_SIZE = 0x2C


def chunks(dzs):
    n = struct.unpack_from('>I', dzs, 0)[0]
    if n <= 0 or n > 512:
        return []
    out = []
    for i in range(n):
        p = 4 + i * 12
        tag = dzs[p:p + 4].decode('ascii', 'replace')
        cnt = struct.unpack_from('>I', dzs, p + 4)[0]
        off = struct.unpack_from('>I', dzs, p + 8)[0]
        out.append((tag, cnt, off))
    return out


def report(path):
    data = open(path, 'rb').read()
    if data[:4] == b'Yaz0':
        data = yaz0(data)
    if data[:4] != b'RARC':
        print('%s: not RARC' % path)
        return
    dzs_files = [f for f in rarc_files(data)
                 if f[0].lower().endswith(('.dzs', '.dzr'))]
    if not dzs_files:
        print('%s: no .dzs/.dzr inside' % path)
        return
    for name, off, size in dzs_files:
        dzs = data[off:off + size]
        cl = chunks(dzs)
        tags = ', '.join('%s x%d' % (t, c) for t, c, _ in cl)
        print('\n=== %s :: %s (%d bytes) ===' % (os.path.basename(path), name, size))
        print('chunks: %s' % (tags or 'NONE'))

        for tag, cnt, off2 in cl:
            if tag != 'Pale':
                continue
            print('Pale: %d entries @ 0x%X' % (cnt, off2))
            for i in range(cnt):
                base = off2 + i * PALE_SIZE
                if base + PALE_SIZE > len(dzs):
                    print('  [%d] TRUNCATED -- entry runs past the chunk' % i)
                    continue
                vals = []
                for fname, fo in PALE_FIELDS:
                    r, g, b = dzs[base + fo], dzs[base + fo + 1], dzs[base + fo + 2]
                    vals.append('%s=(%d,%d,%d)' % (fname, r, g, b))
                virt = dzs[base + 33]
                allzero = all(dzs[base + fo + k] == 0
                              for _, fo in PALE_FIELDS[:10] for k in range(3))
                flag = '  <-- ALL BG/ACTOR SLOTS ZERO' if allzero else ''
                print('  [%d] %s virtIdx=%d%s' % (i, ' '.join(vals), virt, flag))


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__ or 'usage: ww_dzs_kankyo.py <arc> [...]')
        raise SystemExit(2)
    for a in sys.argv[1:]:
        report(a)
