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

    ents_end = count * ENTRY.size
    strings = payload[ents_end:]
    out = {}
    for i in range(count):
        _hash, rva, name_off, flags = ENTRY.unpack_from(payload, i * ENTRY.size)
        end = strings.find(b"\x00", name_off)
        name = strings[name_off:end].decode("utf-8", "replace")
        out[name] = (rva, flags)
    return out


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
        for n in names:
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
