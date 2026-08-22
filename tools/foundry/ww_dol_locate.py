#!/usr/bin/env python3
# ============================================================================
# ww_dol_locate.py -- find DOL-RESIDENT donor assets in the USER'S OWN main.dol
# by EXACT CONTENT MATCH, and emit the offsets a runtime server would need.
#
# WHY THIS EXISTS (docs/state/ww-staging/FINDING-vegetation-assets-are-DOL-
# resident.md): the donor's vegetation managers reference 186 KB of textures,
# display lists and geometry that are COMPILED INTO THE EXECUTABLE, not stored
# in any arc. Zero-bake forbids pasting those bytes into plugin source, so the
# port must READ THEM FROM THE USER'S DISC -- the same rule the arc-served
# assets already follow.
#
# WHY CONTENT MATCH RATHER THAN THE DEBUG MAP: the maps give addresses for the
# DEBUG build (frameworkD.map), and the user's disc is RETAIL. Those addresses
# do not transfer. But the decompilation's asset headers carry the actual BYTE
# VALUES, so the bytes themselves are the search key -- no symbol table needed,
# and a hit is self-verifying because it is an exact match of the whole blob.
#
# WHAT A HIT PROVES: that this exact byte sequence exists at that offset in the
# user's own executable. WHAT IT DOES NOT PROVE: that the runtime consumer
# reads it from there (the DOL is relocated at load; a served table must be
# addressed the way the game addresses it, which is a separate question).
#
# Usage:
#   python ww_dol_locate.py <main.dol> <asset_header.h> [more.h ...]
#   python ww_dol_locate.py <main.dol> --vegetation     # the 3 managers' set
# ============================================================================
import os
import re
import struct
import sys

DECOMP = r'D:\XXXXXXX\WW DP'
ASSETS = os.path.join(DECOMP, 'build', 'D44J01', 'include', 'assets')
MANAGERS = ('src/d/d_grass.cpp', 'src/d/d_flower.cpp', 'src/d/d_tree.cpp')

# A header may hold several arrays; each becomes its own searchable blob so a
# partial match cannot masquerade as a whole-file hit.
# The type may be MULTI-WORD ("unsigned char l_K_kusa_00TEX[] ATTRIBUTE_ALIGN(32)"),
# which a single-token type pattern silently skips -- that bug made the first
# run report 4 hits and 44 "skipped", hiding every texture and display list.
ARRAY_RE = re.compile(
    r'(?:^|\n)\s*(?:static\s+)?(?:const\s+)?(?:[\w:]+\s+){1,3}?(\w+)\s*\[\]\s*(?:ATTRIBUTE_ALIGN\(\d+\)\s*)?=\s*\{(.*?)\n\}\s*;',
    re.S)
HEX_RE = re.compile(r'0[xX]([0-9a-fA-F]{1,2})\b')
FLOAT_RE = re.compile(r'-?\d*\.\d+f?|-?\d+\.f?|-?\d+f')


def blobs(path):
    """Yield (array_name, bytes) for every byte-array in an asset header.

    Only arrays that are ENTIRELY hex byte literals are returned -- a float
    table (positions, texcoords) is real data but is not byte-addressable this
    way, and silently coercing it would invent a blob that does not exist.
    """
    txt = open(path, encoding='utf-8', errors='replace').read()
    out = []
    for name, body in ARRAY_RE.findall(txt):
        toks = [t for t in re.split(r'[\s,{}]+', body) if t]
        if not toks:
            continue
        if all(HEX_RE.fullmatch(t) for t in toks):
            out.append((name, bytes(int(t, 16) for t in toks)))
            continue
        # FLOAT TABLES (positions, texcoords, colours as cXy/cXyz/f32) are real
        # DOL-resident data too -- skipping them the way the first pass did
        # left 21 of 48 headers unaccounted for and made the coverage look
        # worse than it is. GameCube is big-endian, so pack >f.
        fl = [t for t in toks if FLOAT_RE.fullmatch(t)]
        if fl and len(fl) == len(toks):
            try:
                out.append((name, struct.pack('>%df' % len(fl),
                                              *[float(t.rstrip('f')) for t in fl])))
            except (ValueError, struct.error):
                pass
    return out


def veg_headers():
    """Return sorted header paths, and a header->owning-unit map.

    THE UNIT MATTERS: symbol names are NOT unique across the donor's asset
    headers. `l_pos` appears 3x, `l_texCoord` 3x, `l_color` 2x -- once per
    manager, each a DIFFERENT table at a different offset. Keyed on the bare
    symbol, a lookup would hand the grass manager the flower's vertex table and
    look like it worked. The owning donor unit disambiguates them.
    """
    names = {}
    for rel in MANAGERS:
        p = os.path.join(DECOMP, rel)
        if not os.path.exists(p):
            continue
        unit = os.path.splitext(os.path.basename(rel))[0]
        txt = open(p, encoding='utf-8', errors='replace').read()
        for n in re.findall(r'assets/([A-Za-z0-9_]+\.h)', txt):
            names.setdefault(n, set()).add(unit)
    paths = sorted(os.path.join(ASSETS, n) for n in names
                   if os.path.exists(os.path.join(ASSETS, n)))
    owner = {}
    for n, units in names.items():
        # A header included by more than one manager is recorded as shared
        # rather than silently attributed to whichever was seen first.
        owner[n] = sorted(units)[0] if len(units) == 1 else '+'.join(sorted(units))
    return paths, owner


def main():
    if len(sys.argv) < 3:
        print(__doc__ or 'usage: ww_dol_locate.py <main.dol> <header.h|--vegetation>')
        return 2
    dol = open(sys.argv[1], 'rb').read()
    print('DOL: %s (%d bytes)' % (sys.argv[1], len(dol)))

    if sys.argv[2] == '--vegetation':
        headers, owner = veg_headers()
        print('vegetation asset headers: %d' % len(headers))
    else:
        headers = sys.argv[2:]
        owner = {}

    # --emit writes a C table instead of a report. The table holds OFFSETS AND
    # SIZES ONLY -- metadata about where bytes live on the user's disc, never
    # the bytes. Generated rather than hand-typed so it cannot drift from the
    # measurement that produced it.
    emit = '--emit' in sys.argv
    if emit:
        headers = [h for h in headers if h != '--emit']
        rows = []

    hits = misses = skipped = 0
    hit_bytes = 0
    skipped_names = []
    for h in headers:
        try:
            bl = blobs(h)
        except Exception as e:                                  # noqa: BLE001
            print('  %-40s PARSE ERROR %s' % (os.path.basename(h), e))
            continue
        if not bl:
            # NAME IT. This used to bump a counter and print "skipped N
            # (non-byte or tiny tables)", which read as benign and was not:
            # l_matDL and l_Vmori_matDL -- material display lists the grass
            # draw path calls EVERY FRAME -- were hiding in that count. Their
            # headers hold LOAD_BP_REG(...) MACRO INVOCATIONS, not bytes, so
            # content matching cannot find them at all. A summary that hides a
            # required asset behind a reassuring word is the same defect as a
            # gate whose silence is only as wide as its scope.
            skipped += 1
            skipped_names.append(os.path.basename(h) + ' (no byte/float array -- macro-built?)')
            continue
        for name, data in bl:
            if len(data) < 16:
                skipped += 1
                skipped_names.append('%s::%s (%d B, below the 16-byte search floor)'
                                     % (os.path.basename(h), name, len(data)))
                continue
            off = dol.find(data)
            if off >= 0:
                dup = dol.find(data, off + 1)
                uniq = dup < 0
                tag = 'HIT' if uniq else 'HIT (also @0x%X -- NOT UNIQUE)' % dup
                if emit:
                    # A non-unique blob is recorded with its uniqueness flag
                    # rather than dropped: the consumer decides whether an
                    # ambiguous match is good enough, and a silently-omitted
                    # row reads as "this asset does not exist".
                    base = os.path.basename(h)
                    unit = owner.get(base, '?')
                    rows.append((unit, name, off, len(data), uniq, base))
                else:
                    print('  %-38s %7d B  0x%06X  %s' % (name, len(data), off, tag))
                hits += 1
                hit_bytes += len(data)
            else:
                if not emit:
                    print('  %-38s %7d B  --------  MISS' % (name, len(data)))
                misses += 1
    if emit:
        # The disc ID is part of the table's identity: these are file offsets
        # into ONE build's executable. A JP or EU disc has different ones, and
        # serving this table against another build would hand the game the
        # wrong bytes silently -- so the ID is recorded and the consumer gates
        # on it.
        disc = 'UNKNOWN'
        try:
            with open(os.path.join(os.path.dirname(sys.argv[1]), 'boot.bin'), 'rb') as bf:
                disc = bf.read(6).decode('ascii', 'replace')
        except OSError:
            pass
        print('// GENERATED by tools/foundry/ww_dol_locate.py --emit -- DO NOT HAND-EDIT.')
        print('// Offsets and sizes ONLY: metadata about where bytes live in the')
        print('// user\'s own main.dol. No donor content is reproduced here.')
        print('// Disc build: %s   blobs: %d   total: %d bytes' % (disc, len(rows), hit_bytes))
        print('#define WW_DOL_TABLE_DISC_ID "%s"' % disc)
        print('// Keyed on UNIT + SYMBOL: symbol names are NOT unique across these')
        print('// headers (l_pos x3, l_texCoord x3, l_color x2 -- one per manager,')
        print('// each a different table). A bare-symbol key would silently serve')
        print('// the wrong vertex data to a manager that would draw it happily.')
        print('static const WwDolAsset kWwDolAssets[] = {')
        for unit, name, off, ln, uniq, src in sorted(rows):
            print('    { "%s", "%s", 0x%06Xu, %6uu, %d },  // %s'
                  % (unit, name, off, ln, 1 if uniq else 0, src))
        print('};')
        return 0

    print('\nhit %d / miss %d / skipped %d' % (hits, misses, skipped))
    if skipped_names:
        print('SKIPPED, NAMED -- a bare count hid two REQUIRED assets once')
        print('(l_matDL / l_Vmori_matDL, called every frame by the grass draw):')
        for n in skipped_names:
            print('   %s' % n)
    print('located %d bytes (%.1f KB) inside the user\'s own executable' % (hit_bytes, hit_bytes / 1024))
    if misses:
        print('A MISS is meaningful: the decomp headers are built from D44J01 (JP).')
        print('A miss means that blob differs between that build and this disc --')
        print('it does NOT mean the data is absent, and it must not be read as one.')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
