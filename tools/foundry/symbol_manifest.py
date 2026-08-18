#!/usr/bin/env python3
# ============================================================================
# symbol_manifest.py — read the EMBEDDED symbol manifest out of a built
# dusklight image, so "needs resolve()" can become "resolves" or "MISSING".
#
# WHY THIS EXISTS
# §524's conformance run compared EXPORT SURFACES (.def files). That answers
# what a plugin can LINK, and says nothing about what resolve() can find --
# resolve() reads a manifest embedded in the linked image, not the .def. So the
# ten genuine (c)-set symbols were classified "needs resolve()" on both builds
# with no evidence either could actually resolve them. This closes that.
#
# FORMAT, mirrored from src/dusk/mods/manifest.cpp (the reader, not a guess)
#   Header 72 bytes: magic "SYMGEN\0\0" · version u32 (2) · compression u32
#   (0=None 1=Zstd) · uncompressedLen u64 · compressedLen u64 · buildIdLen u32
#   · buildId[32] · entryCount u32
#   Payload (after decompression): [Entry * entryCount][string table]
#   Entry 24 bytes: hash u64 · rva u64 · nameOff u32 · flags u32
#
# LOCATING IT. The SYMDBHDR descriptor stores its magic as a u64 and is patched
# post-link, so scanning for the descriptor finds the source string first. This
# scans for the manifest magic directly and VALIDATES each hit against the
# header's own invariants (version==2, known compression, sane entry count) --
# the first hit in a real image is the `kMagic` constant in .rdata, not the
# manifest, so accepting hit #1 would parse garbage.
#
# Read-only. Usage: symbol_manifest.py <image.exe> [--find NAME ...] [--stats]
# ============================================================================
import struct
import sys
from pathlib import Path

MAGIC = b"SYMGEN\x00\x00"
HDR = "<8sIIQQI32sI"
HDR_SIZE = 72
ENTRY = struct.Struct("<QQII")

FLAG_CODE = 1 << 0
FLAG_DATA = 1 << 1
FLAG_LOCAL = 1 << 2
FLAG_MULTI_NAME = 1 << 3
FLAG_DISPLAY = 1 << 6


def _unzstd(payload, size):
    """Decompress, preferring the STDLIB.

    §530/§532: two independent lanes could not reproduce this tool's real-data
    numbers because `zstandard` was missing in both environments. Declaring the
    dependency (§531) did not fix that — it documented it. By this campaign's
    own standard a number two lanes cannot reproduce is not yet a finding, so
    the dependency is REMOVED where possible rather than merely announced.

    Python 3.14 ships zstd in the standard library (PEP 784), so on a current
    interpreter this needs no install at all. `zstandard` stays as a fallback
    for older ones."""
    try:
        from compression import zstd as _z          # Python 3.14+ stdlib
        return _z.decompress(payload)
    except ImportError:
        pass
    try:
        import zstandard
        return zstandard.ZstdDecompressor().decompress(
            payload, max_output_size=size)
    except ImportError:
        raise SystemExit(
            "this manifest is zstd-compressed and no zstd backend is available."
            + chr(10) + "  EASIEST: run this under Python 3.14+, which has zstd "
                        "in the stdlib (no install)."
            + chr(10) + "  otherwise: python -m pip install -r "
                        "tools/foundry/requirements.txt")


def flag_names(f):
    out = []
    for bit, nm in ((FLAG_CODE, "CODE"), (FLAG_DATA, "DATA"),
                    (FLAG_LOCAL, "LOCAL"), (FLAG_MULTI_NAME, "MULTI_NAME"),
                    (FLAG_DISPLAY, "DISPLAY")):
        if f & bit:
            out.append(nm)
    return "|".join(out) or f"0x{f:x}"


def load(image):
    """name -> (rva, flags). Returns None if the image carries no manifest.

    None is NOT an empty dict, and callers must not conflate them: an image
    built without code mods has NO by-name resolution at all, which is a
    different claim from "this name is absent" (№31-C, and the exact case
    HousingTemp's C4 controls)."""
    data = Path(image).read_bytes()
    off = -1
    while True:
        off = data.find(MAGIC, off + 1)
        if off < 0:
            return None
        try:
            h = struct.unpack_from(HDR, data, off)
        except struct.error:
            continue
        ver, comp, unc, cmp_, _bidlen, _bid, count = h[1:]
        if ver == 2 and comp in (0, 1) and 0 < count < 5_000_000 \
                and 0 < cmp_ <= len(data) - off:
            break

    payload = data[off + HDR_SIZE: off + HDR_SIZE + cmp_]
    if comp == 1:
        payload = _unzstd(payload, unc)
    if len(payload) != unc:
        raise ValueError(f"manifest decompressed to {len(payload)}, expected {unc}")

    # RAW COUNTS, exposed because the dict below LOSES them (Integrator audit,
    # tale §959): `out` is keyed by NAME, so duplicate names collapse — 5,076
    # of them in our image, 5,317 in vanilla. That made the reported size
    # 279,447 against the runtime's 294,750 (delta 15,303), a systematic gap
    # that looked like a parser bug and was actually a MISLABEL: the dict size
    # is UNIQUE NAMES, never entries. Callers that want the runtime's number
    # must read `load_counts`, and callers that hook BY NAME must know that
    # 5k names are ambiguous.
    ents_end = count * ENTRY.size
    strings = payload[ents_end:]
    out = {}
    for i in range(count):
        _hash, rva, name_off, flags = ENTRY.unpack_from(payload, i * ENTRY.size)
        end = strings.find(b"\x00", name_off)
        name = strings[name_off:end].decode("utf-8", "replace")
        out[name] = (rva, flags)
    load.last_entry_count = count          # header truth, before collapsing
    load.last_unique_names = len(out)
    return out


def occurrences(image):
    """Counter of name -> how many RAW entries carry it.

    Why this exists (and it is the second time the same trap fired): the
    dict from load() COLLAPSES duplicates, so a query against it reports
    `RES_U32` as unique when the manifest holds 685 of them. A safety check
    built on the collapsed view gives a confident FALSE SAFE for exactly the
    names it was written to catch.
    """
    import collections
    import struct as _s
    data = Path(image).read_bytes()
    off = -1
    while True:
        off = data.find(MAGIC, off + 1)
        if off < 0:
            return None
        try:
            h = _s.unpack_from(HDR, data, off)
        except _s.error:
            continue
        ver, comp, unc, cmp_, _bl, _bid, count = h[1:]
        if ver == 2 and comp in (0, 1) and 0 < count < 5_000_000 \
                and 0 < cmp_ <= len(data) - off:
            break
    payload = data[off + HDR_SIZE: off + HDR_SIZE + cmp_]
    if comp == 1:
        payload = _unzstd(payload, unc)
    strings = payload[count * ENTRY.size:]
    seen = collections.Counter()
    for i in range(count):
        _h, _r, no, _f = ENTRY.unpack_from(payload, i * ENTRY.size)
        e = strings.find(b"\x00", no)
        seen[strings[no:e].decode("utf-8", "replace")] += 1
    return seen


def load_counts(image):
    """(entries, unique_names, duplicated_names) or None.

    entries          = the header's own count; what the RUNTIME logs.
    unique_names     = what by-name resolution can address.
    duplicated_names = names appearing more than once — the AMBIGUITY surface:
                       a hook installed on one of these binds by a name that
                       does not identify a single function."""
    import collections
    import struct as _s
    data = Path(image).read_bytes()
    off = -1
    while True:
        off = data.find(MAGIC, off + 1)
        if off < 0:
            return None
        try:
            h = _s.unpack_from(HDR, data, off)
        except _s.error:
            continue
        ver, comp, unc, cmp_, _bl, _bid, count = h[1:]
        if ver == 2 and comp in (0, 1) and 0 < count < 5_000_000 \
                and 0 < cmp_ <= len(data) - off:
            break
    payload = data[off + HDR_SIZE: off + HDR_SIZE + cmp_]
    if comp == 1:
        payload = _unzstd(payload, unc)
    strings = payload[count * ENTRY.size:]
    seen = collections.Counter()
    for i in range(count):
        _h, _r, no, _f = ENTRY.unpack_from(payload, i * ENTRY.size)
        e = strings.find(b"\x00", no)
        seen[strings[no:e].decode("utf-8", "replace")] += 1
    return count, len(seen), sum(1 for v in seen.values() if v > 1)


def main():
    argv = sys.argv[1:]
    if not argv:
        print(__doc__ or "usage: symbol_manifest.py <image.exe> [--find NAME ...]")
        return 2
    image = argv[0]
    table = load(image)
    print(f"SYMBOL MANIFEST — {image}")
    if table is None:
        print("  NO MANIFEST EMBEDDED — by-name resolution is unavailable in "
              "this image WHOLESALE. Not 'these names are missing': every name "
              "is unresolvable. Reported as UNKNOWN, never CLEAN (№31-C).")
        return 2
    print(f"  {len(table)} symbols\n")

    names = [a for a in argv[1:] if a != "--find"]
    if names:
        # Identifier-level fallback: the manifest stores decorated/qualified
        # names, while the census records a bare identifier. An exact miss is
        # therefore not yet a MISSING verdict -- a suffix/substring pass has to
        # run before that word is used.
        # SHAPE-A CHECK, ported from upstream_conformance (tale §966): --find
        # tested `n in table` — the COLLAPSED dict — so 685 entries sharing the
        # name `RES_U32` reported a plain RESOLVED with no ambiguity marker at
        # all. --find already caught shape B (one bare name matching many
        # QUALIFIED names) and was blind to shape A (many raw entries under one
        # IDENTICAL name), which is the very shape §960 found and fixed in its
        # sibling. A static gate that disagrees with the runtime binder in the
        # OPTIMISTIC direction is worse than no gate: the host's resolve()
        # returns MOD_CONFLICT for these, so --find was greener than reality.
        occ = occurrences(image) or {}
        for n in names:
            dup = occ.get(n, 0)
            if dup > 1:
                print(f"  AMBIGUOUS {n:18s} — {dup} raw entries share this exact "
                      f"name; by-name binding cannot pick one (runtime resolve() "
                      f"returns MOD_CONFLICT). Hook by address or a unique caller.")
                continue
            if n in table:
                rva, fl = table[n]
                print(f"  RESOLVED  {n:18s} rva=0x{rva:x} [{flag_names(fl)}]")
                continue
            cand = [k for k in table
                    if k == n or k.endswith("::" + n) or f"{n}@" in k
                    or f"?{n}@" in k or f"::{n}(" in k]
            if cand:
                k = min(cand, key=len)
                rva, fl = table[k]
                print(f"  RESOLVED* {n:18s} rva=0x{rva:x} [{flag_names(fl)}]  "
                      f"as {k[:60]}"
                      + (f"   (+{len(cand) - 1} more)" if len(cand) > 1 else ""))
            else:
                print(f"  MISSING   {n:18s} — no exact or qualified match")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
