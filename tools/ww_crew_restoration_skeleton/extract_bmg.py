"""Extract the game's message archive (BMG) to an id-keyed TSV.

The flat dialogue dump we shipped first had no message ids and kept raw escape
bytes, so lines could not be attributed to a speaker and control-code residue
reached the font renderer. This reads the real archive instead:

    Msg/bmgres.arc -> zel_00.bmg
        INF1  entry table (fixed-size records: string offset + attributes)
        DAT1  string pool

Escape sequences are `0x1A <len> <payload...>` where <len> counts the whole
sequence. They carry colour/pause/furigana/player-name markers. We decode the
ones with a text meaning and drop the rest, so downstream text is clean.

Usage: python extract_bmg.py <bmgres.arc> <out.tsv>

Output is TSV: msg_id <TAB> text  (newlines escaped as \\n).
Never write output into the repo — it is game text.
"""
import io
import os
import struct
import sys

from adapt_bdl_arcs import be32, yaz0_dec


def rarc_members(d):
    data_abs = 0x20 + be32(d, 0x0C)
    info = 0x20
    n = be32(d, info + 0x08)
    ent = info + be32(d, info + 0x0C)
    strs = info + be32(d, info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", d, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", d, e + 6)[0]
        end = d.index(b"\0", strs + no)
        name = d[strs + no:end].decode("ascii", "replace")
        off, size = be32(d, e + 8), be32(d, e + 12)
        yield name, bytes(d[data_abs + off:data_abs + off + size])


def decode_text(dat, off):
    """Read one null-terminated record, decoding 0x1A escapes."""
    out = []
    i = off
    while i < len(dat):
        b = dat[i]
        if b == 0x00:
            break
        if b == 0x1A:
            # 0x1A <len> ... ; len covers the whole sequence
            ln = dat[i + 1] if i + 1 < len(dat) else 2
            if ln < 2:
                ln = 2
            seq = dat[i:i + ln]
            out.append(escape_meaning(seq))
            i += ln
            continue
        if b == 0x0A:
            out.append("\n")
            i += 1
            continue
        out.append(chr(b) if 32 <= b < 127 else "")
        i += 1
    return "".join(out)


def escape_meaning(seq):
    """Map an escape sequence to plain text (or '' when purely presentational).

    DONOR-VERIFIED (№261, WW DP src/d/d_mesg.cpp tag switch): the 3 bytes after
    <len> form the tag param; param & 0xFF0000 selects the group:
      group 0x00, case 0  -> dComIfGs_getPlayerName()   (THE name insert)
      group 0x00, 1-9     -> pauses/waits               (no text)
      group 0x00, 10-29   -> button/icon glyphs         (no text here)
      group 0x01          -> mDoAud_messageSePlay sound (no text)
      group 0x02          -> camera tag                 (no text)
      group 0xFF          -> colour change              (no text)
    The old mapping (01 00 -> {player}) mislabeled SOUNDS as the name and threw
    the real name escape (1a 05 00 00 00) away.
    """
    if len(seq) >= 5 and seq[2] == 0x00 and seq[3] == 0x00 and seq[4] == 0x00:
        return "{player}"      # player-name insert (donor tag case 0)
    return ""                  # sound / pause / colour / camera / glyph: no text


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    src, dst = sys.argv[1], sys.argv[2]

    d = bytearray(open(src, "rb").read())
    if bytes(d[:4]) == b"Yaz0":
        d = yaz0_dec(d)

    bmg = None
    for name, member in rarc_members(d):
        if name.lower().endswith(".bmg"):
            bmg = member
            break
    if bmg is None:
        sys.exit(f"{src}: no .bmg member")

    if bmg[:8] != b"MESGbmg1":
        sys.exit(f"{src}: not a BMG ({bmg[:8]!r})")

    sec_count = struct.unpack_from(">I", bmg, 0x0C)[0]
    inf_off = dat_off = None
    off = 0x20
    for _ in range(sec_count):
        tag = bmg[off:off + 4]
        size = struct.unpack_from(">I", bmg, off + 4)[0]
        if tag == b"INF1":
            inf_off = off
        elif tag == b"DAT1":
            dat_off = off
        off += size
    if inf_off is None or dat_off is None:
        sys.exit("missing INF1/DAT1")

    count, entry_size = struct.unpack_from(">HH", bmg, inf_off + 8)
    dat = bytes(bmg[dat_off + 8: dat_off + struct.unpack_from(">I", bmg, dat_off + 4)[0]])

    rows = []
    for i in range(count):
        e = inf_off + 0x10 + i * entry_size
        str_off = struct.unpack_from(">I", bmg, e)[0]
        msg_id = struct.unpack_from(">H", bmg, e + 4)[0]
        if str_off >= len(dat):
            continue
        text = decode_text(dat, str_off)
        rows.append((msg_id, i, text))

    with io.open(dst, "w", encoding="utf-8", newline="\n") as f:
        f.write("msg_id\tindex\ttext\n")
        for msg_id, idx, text in rows:
            f.write(f"{msg_id}\t{idx}\t{text.replace(chr(10), '\\n')}\n")

    print(f"{count} entries (entry_size={entry_size}) -> {dst}")


if __name__ == "__main__":
    main()
