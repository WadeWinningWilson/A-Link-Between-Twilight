"""Dump a BDL's MAT3 TEV stages offline — no game, no engine.

Purpose: find out whether a donor material asks for the TEV alpha-compare ops
that stock aurora mistranslates into invalid WGSL (the fork's shader.cpp F1 fix
targets GX_TEV_COMP_*_GT / _EQ on the ALPHA path). If the material never uses
them, F1 is not this model's fix and the search moves elsewhere.

Usage: python j3d_bdl_tev.py <path-to.arc>
"""
import struct
import sys

TEV_OP = {
    0: 'GX_TEV_ADD',
    1: 'GX_TEV_SUB',
    8: 'GX_TEV_COMP_R8_GT',
    9: 'GX_TEV_COMP_R8_EQ',
    10: 'GX_TEV_COMP_GR16_GT',
    11: 'GX_TEV_COMP_GR16_EQ',
    12: 'GX_TEV_COMP_BGR24_GT',
    13: 'GX_TEV_COMP_BGR24_EQ',
    14: 'GX_TEV_COMP_RGB8_GT',   # == GX_TEV_COMP_A8_GT on the alpha path
    15: 'GX_TEV_COMP_RGB8_EQ',   # == GX_TEV_COMP_A8_EQ on the alpha path
}
# The ops the fork's shader.cpp added handling for; anything here on the ALPHA
# channel is what makes stock aurora emit invalid WGSL.
COMP_OPS = set(range(8, 16))


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


path = sys.argv[1]
data = open(path, 'rb').read()
if data[:4] == b'Yaz0':
    data = yaz0(data)
assert data[:4] == b'RARC'

bdl = [f for f in rarc_files(data) if f[0].lower().endswith(('.bdl', '.bmd'))]
name, off, size = bdl[0]
b = data[off:off + size]
print('BDL %s (%d bytes)' % (name, size))

# ---- locate MAT3 ---------------------------------------------------------
nsec = struct.unpack_from('>I', b, 0x0C)[0]
p = 0x20
mat3 = None
for _ in range(nsec):
    tag = b[p:p + 4]
    ssize = struct.unpack_from('>I', b, p + 4)[0]
    if tag == b'MAT3':
        mat3 = p
    p += ssize
if mat3 is None:
    print('NO MAT3')
    sys.exit(1)

mat_count = struct.unpack_from('>H', b, mat3 + 0x08)[0]
# MAT3 offset table: index 0x0C.. is a list of section offsets.
# TevStageInfo table is entry 18 (0-based) in the classic BMD/BDL layout.
# MAT3 offset table order (0-based): 16 TevOrderInfo, 17 TevColor,
# 18 TevKColor, 19 TevStageNum, 20 TevStageInfo, 21 TevSwapModeInfo, ...
# An earlier version used 18 and reported "0 stages" - which was the PARSER
# being wrong, not the model having none. Worth keeping: the tool refusing to
# print a table it could not bound is what stopped a false verdict.
TEV_STAGE_INFO_INDEX = 20
off_tev = struct.unpack_from('>I', b, mat3 + 0x0C + TEV_STAGE_INFO_INDEX * 4)[0]
off_tev_next = None
for k in range(TEV_STAGE_INFO_INDEX + 1, 30):
    v = struct.unpack_from('>I', b, mat3 + 0x0C + k * 4)[0]
    if v != 0:
        off_tev_next = v
        break
print('MAT3 at 0x%X  materials=%d  tevStage table at +0x%X' % (mat3, mat_count, off_tev))

if not off_tev or off_tev_next is None or off_tev_next <= off_tev:
    print('could not bound the TEV stage table; aborting rather than guessing')
    sys.exit(1)

count = (off_tev_next - off_tev) // 20   # TevStageInfo is 20 bytes
print('TEV stages in table: %d' % count)

# ---- TevOrderInfo (index 16): texCoordId / texMapId per stage --------------
# Stock aurora CHECK-fatals when a stage's texcoord or texmap is UNBOUND
# (GX_TEXCOORD_NULL 0xFF / GX_TEXMAP_NULL 0xFF); the fork replaced those hard
# CHECKs with tolerance. A material that binds neither is therefore fatal on
# stock and fine on the fork - and that is a MATERIAL property, so it can be
# translated at the consumption boundary instead of patching the engine.
off_ord = struct.unpack_from('>I', b, mat3 + 0x0C + 16 * 4)[0]
off_ord_next = None
for k in range(17, 30):
    v = struct.unpack_from('>I', b, mat3 + 0x0C + k * 4)[0]
    if v != 0:
        off_ord_next = v
        break
if off_ord and off_ord_next and off_ord_next > off_ord:
    ocount = (off_ord_next - off_ord) // 4   # TevOrderInfo is 4 bytes
    print()
    print('TevOrderInfo entries: %d' % ocount)
    unbound = []
    for i in range(ocount):
        s0 = mat3 + off_ord + i * 4
        tcoord, tmap, color = b[s0], b[s0 + 1], b[s0 + 2]
        flag = ''
        if tcoord == 0xFF:
            flag += '  <== texCoord UNBOUND'
        if tmap == 0xFF:
            flag += '  <== texMap UNBOUND'
        print('  order %2d: texCoordId=0x%02X texMapId=0x%02X colorChan=0x%02X%s'
              % (i, tcoord, tmap, color, flag))
        if tcoord == 0xFF or tmap == 0xFF:
            unbound.append(i)
    print()
    if unbound:
        print('*** %d TevOrder entr(ies) UNBOUND -> stock aurora CHECK-fatals here.' % len(unbound))
        print('    This IS a plugin-translatable material property.')
    else:
        print('    All TevOrder entries bound; the unbound-texmap CHECK is not the trigger.')
else:
    print()
    print('could not bound the TevOrder table; not guessing')

print()
found = []
for i in range(count):
    s = mat3 + off_tev + i * 20
    # TevStageInfo: pad, colorInA..D (4), colorOp, colorBias, colorScale,
    #               colorClamp, colorRegID, alphaInA..D (4), alphaOp,
    #               alphaBias, alphaScale, alphaClamp, alphaRegID, pad
    color_op = b[s + 5]
    alpha_op = b[s + 14]
    cflag = ' <== COMP op on COLOUR' if color_op in COMP_OPS else ''
    aflag = ' <== COMP op on ALPHA (this is what stock aurora mistranslates)' \
        if alpha_op in COMP_OPS else ''
    print('  stage %2d: colorOp=%-22s alphaOp=%-22s%s%s'
          % (i, TEV_OP.get(color_op, '0x%02X' % color_op),
             TEV_OP.get(alpha_op, '0x%02X' % alpha_op), cflag, aflag))
    if alpha_op in COMP_OPS:
        found.append((i, alpha_op))

print()
if found:
    print('VERDICT: %d stage(s) use a COMP op on the ALPHA channel -> F1 IS relevant here.' % len(found))
    for i, op in found:
        print('   stage %d: %s' % (i, TEV_OP.get(op, hex(op))))
else:
    print('VERDICT: NO alpha-channel COMP ops in this model. F1 is NOT this model\'s fix;')
    print('         the reason lwood fails on stock aurora lies elsewhere.')
