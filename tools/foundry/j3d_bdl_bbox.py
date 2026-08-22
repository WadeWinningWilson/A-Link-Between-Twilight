"""Read the SHP1 per-shape bounding boxes out of a donor Lwood.arc, offline.

Independent of the running game: if these match what the receipt reported at
runtime, the bytes the plugin decoded came from this disc.

RARC container -> ALWD.bdl -> SHP1 section -> per-shape radius/min/max.
"""
import struct
import sys

path = sys.argv[1]
data = open(path, 'rb').read()
print('arc: %s  %d bytes  magic=%r' % (path, len(data), data[:4]))


def yaz0(src):
    """Yaz0 is RLE-with-backreferences; the arc on disc is compressed."""
    assert src[:4] == b'Yaz0'
    out_size = struct.unpack_from('>I', src, 4)[0]
    out = bytearray()
    p = 16
    code = 0
    bits = 0
    while len(out) < out_size:
        if bits == 0:
            code = src[p]
            p += 1
            bits = 8
        if code & 0x80:
            out.append(src[p])
            p += 1
        else:
            b1 = src[p]
            b2 = src[p + 1]
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


if data[:4] == b'Yaz0':
    data = yaz0(data)
    print('  Yaz0-decompressed -> %d bytes  magic=%r' % (len(data), data[:4]))

# ---- RARC ----------------------------------------------------------------
assert data[:4] == b'RARC', data[:4]
data_off = struct.unpack_from('>I', data, 0x0C)[0] + 0x20
num_nodes = struct.unpack_from('>I', data, 0x20)[0]
node_off = struct.unpack_from('>I', data, 0x24)[0] + 0x20
num_entries = struct.unpack_from('>I', data, 0x28)[0]
entry_off = struct.unpack_from('>I', data, 0x2C)[0] + 0x20
str_off = struct.unpack_from('>I', data, 0x34)[0] + 0x20

files = []
for i in range(num_entries):
    e = entry_off + i * 0x14
    nameoff = struct.unpack_from('>H', data, e + 0x06)[0]
    flags = data[e + 0x04]
    off = struct.unpack_from('>I', data, e + 0x08)[0]
    size = struct.unpack_from('>I', data, e + 0x0C)[0]
    end = data.find(b'\x00', str_off + nameoff)
    name = data[str_off + nameoff:end].decode('shift_jis', 'replace')
    isdir = (flags & 0x02) != 0
    if not isdir:
        files.append((name, data_off + off, size))

print('files in arc:')
for n, o, s in files:
    print('   %-20s %8d bytes' % (n, s))

bdl = [f for f in files if f[0].lower().endswith('.bdl')]
if not bdl:
    print('NO BDL FOUND')
    sys.exit(1)
name, off, size = bdl[0]
b = data[off:off + size]
print()
print('BDL: %s  %d bytes  magic=%r' % (name, size, b[:8]))

# ---- J3D sections --------------------------------------------------------
nsec = struct.unpack_from('>I', b, 0x0C)[0]
p = 0x20
shp1 = None
for _ in range(nsec):
    tag = b[p:p + 4]
    ssize = struct.unpack_from('>I', b, p + 4)[0]
    if tag == b'SHP1':
        shp1 = p
    p += ssize
if shp1 is None:
    print('NO SHP1')
    sys.exit(1)

shape_count = struct.unpack_from('>H', b, shp1 + 0x08)[0]
shape_off = struct.unpack_from('>I', b, shp1 + 0x0C)[0]
print('SHP1 at 0x%X  shapes=%d' % (shp1, shape_count))

lo = [1e30] * 3
hi = [-1e30] * 3
for i in range(shape_count):
    s = shp1 + shape_off + i * 0x28
    mtxgroups = struct.unpack_from('>H', b, s + 0x02)[0]
    radius = struct.unpack_from('>f', b, s + 0x10)[0]
    mn = struct.unpack_from('>3f', b, s + 0x14)
    mx = struct.unpack_from('>3f', b, s + 0x20)
    print('  shape %d: mtxGroups=%d radius=%.1f min=(%.1f, %.1f, %.1f) max=(%.1f, %.1f, %.1f)'
          % (i, mtxgroups, radius, mn[0], mn[1], mn[2], mx[0], mx[1], mx[2]))
    for a in range(3):
        lo[a] = min(lo[a], mn[a])
        hi[a] = max(hi[a], mx[a])

print()
print('UNION over shapes:')
print('   min = (%.1f, %.1f, %.1f)' % tuple(lo))
print('   max = (%.1f, %.1f, %.1f)' % tuple(hi))
print()
print('RUNTIME REPORTED: dec_lo=(-660.6, -40.0, -660.6)  dec_hi=(660.6, 881.3, 660.6)')
match = (abs(lo[0] + 660.6) < 1 and abs(lo[1] + 40.0) < 1 and abs(lo[2] + 660.6) < 1 and
         abs(hi[0] - 660.6) < 1 and abs(hi[1] - 881.3) < 1 and abs(hi[2] - 660.6) < 1)
print('MATCH: %s' % ('YES - the runtime decoded THIS disc\'s bytes'
                     if match else 'NO - the runtime model came from somewhere else'))
