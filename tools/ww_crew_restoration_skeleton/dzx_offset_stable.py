#!/usr/bin/env python3
# ============================================================================
# dzx_offset_stable.py — the P1-law editor for .dzs/.dzr blobs (§3.2 refit)
#
# THE LAW (stage-data-bake-pitfalls P1): never re-layout a dz* file wholesale.
# Chunks with FILE-ABSOLUTE interior pointers (RTBL's 3-level tables, RPAT ->
# RPPN, PATH -> PPNT) corrupt silently when anything moves. The only safe
# operations are:
#   (1) EXTEND an existing chunk: relocate THAT CHUNK's entry array to the
#       file end (old entries + new), patch only its 8 header bytes. The old
#       region becomes dead bytes; nothing else moves. (plyr_append pattern.)
#   (2) ADD a new chunk tag: the header directory at +0x4 must grow by 12
#       bytes, which would shift data — so instead RELOCATE the chunk whose
#       data starts immediately after the directory (only if it is on the
#       MOVABLE whitelist) to the file end, then place the new header entry
#       in the freed space. (revt_bake v2 pattern, generalized.)
# Both verify: byte-diff proves every region not explicitly claimed is
# untouched. Refuses (hard stop) rather than guesses.
#
# MOVABLE = flat entry arrays / index-linked chunks only. POINTERED chunks
# are never relocated and never silently crossed.
# ============================================================================
import struct

be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]

# flat entry arrays or index-linked payloads — safe to relocate wholesale
MOVABLE = {
    "FILI", "PLYR", "SCLS", "SOND", "LGTV", "LBNK", "EVNT", "2DMA", "MULT",
    "STAG", "EnvR", "Colo", "Pale", "Virt", "RCAM", "RARO", "REVT",
    "ACTR", "SCOB", "DOOR", "TGDR", "TGOB", "TGSC",
} | {f"ACT{c}" for c in "0123456789ab"} | {f"SCO{c}" for c in "0123456789ab"}

# interior FILE-ABSOLUTE pointers — never move, never rebuild
POINTERED = {"RTBL", "RPAT", "RPPN", "PATH", "PPNT"}


class DzxError(SystemExit):
    pass


def chunk_table(dz):
    n = be32(dz, 0)
    out = []
    for i in range(n):
        tag = dz[4 + i * 12:8 + i * 12].decode("ascii", "replace")
        cnt, off = struct.unpack_from(">II", dz, 8 + i * 12)
        out.append([tag, cnt, off, 8 + i * 12])  # header cnt field at idx 3
    return out


def extend_chunk(dz, tag, new_entries, entry_size):
    """Case 1: append entries to an EXISTING chunk, offset-stable.

    Returns (new_blob, claims) where claims = list of (start, end, why) byte
    ranges the edit is allowed to differ in — everything else must byte-match.
    """
    dz = bytearray(dz)
    table = chunk_table(dz)
    hit = [c for c in table if c[0] == tag]
    if not hit:
        raise DzxError(f"extend_chunk: no {tag} chunk (use add_chunk)")
    if tag in POINTERED:
        raise DzxError(f"extend_chunk: {tag} is POINTERED — hard stop (P1)")
    tag_, cnt, off, hdr = hit[0]
    old = bytes(dz[off:off + cnt * entry_size])
    add = b"".join(new_entries)
    new_off = len(dz)
    out = bytearray(dz)
    out += old + add
    while len(out) % 4:
        out.append(0)
    struct.pack_into(">II", out, hdr, cnt + len(new_entries), new_off)
    claims = [(hdr, hdr + 8, f"{tag} header (count+offset)"),
              (len(dz), len(out), f"{tag} relocated+extended entries")]
    _verify_untouched(dz, out, claims)
    return bytes(out), claims


def add_chunk(dz, tag, entries, entry_size):
    """Case 2: add a NEW chunk tag, offset-stable (directory growth via
    relocation of the first-after-directory movable chunk)."""
    dz = bytearray(dz)
    if tag in POINTERED:
        raise DzxError(f"add_chunk: {tag} is POINTERED — hard stop (P1)")
    table = chunk_table(dz)
    if any(c[0] == tag for c in table):
        raise DzxError(f"add_chunk: {tag} already exists (use extend_chunk)")
    n = len(table)
    dir_end = 4 + n * 12
    # who owns the bytes right after the directory?
    owner = min((c for c in table if c[2] >= dir_end), key=lambda c: c[2],
                default=None)
    if owner is None:
        raise DzxError("add_chunk: no chunk data after directory — layout?")
    otag, ocnt, ooff, ohdr = owner
    if ooff - dir_end >= 12:
        # enough existing slack — no relocation needed
        freed = dir_end
        out = bytearray(dz)
        claims = []
    else:
        if otag not in MOVABLE:
            raise DzxError(f"add_chunk: chunk after directory is {otag} "
                           "(not movable) — hard stop rather than shift (P1)")
        osz = _entry_size_of(otag, ocnt, dz, table)
        blob = bytes(dz[ooff:ooff + osz])
        out = bytearray(dz)
        reloc_off = len(out)
        out += blob
        while len(out) % 4:
            out.append(0)
        struct.pack_into(">I", out, ohdr + 4, reloc_off)
        freed = dir_end
        claims = [(ohdr + 4, ohdr + 8, f"{otag} relocated offset"),
                  (ooff, ooff + 12, "freed region reused for new header"),
                  (len(dz), len(out), f"{otag} relocated data")]
    # new header entry into freed space; new entries at the end
    data_off = len(out)
    out += b"".join(entries)
    while len(out) % 4:
        out.append(0)
    out[freed:freed + 4] = tag.encode("ascii").ljust(4)
    struct.pack_into(">II", out, freed + 4, len(entries), data_off)
    struct.pack_into(">I", out, 0, n + 1)
    claims += [(0, 4, "chunk count"),
               (freed, freed + 12, "new chunk header"),
               (data_off, len(out), f"{tag} new entries")]
    _verify_untouched(dz, out, claims)
    return bytes(out), claims


def _entry_size_of(tag, cnt, dz, table):
    """Size of a chunk's data region: gap to the next-higher chunk offset
    (or file end). Only used for MOVABLE chunks (flat arrays)."""
    off = next(c[2] for c in table if c[0] == tag)
    higher = [c[2] for c in table if c[2] > off]
    end = min(higher) if higher else len(dz)
    return end - off


def _verify_untouched(old, new, claims):
    """Byte-diff: every offset outside the claimed ranges must match."""
    lim = min(len(old), len(new))
    for i in range(lim):
        if old[i] != new[i] and not any(a <= i < b for a, b, _ in claims):
            raise DzxError(f"P1 VIOLATION at +0x{i:X}: byte changed outside "
                           f"claimed regions — refusing")
