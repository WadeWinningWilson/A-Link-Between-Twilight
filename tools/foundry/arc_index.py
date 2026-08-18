#!/usr/bin/env python3
"""arc_index.py — dump a RARC archive's entries IN ORDER, with their file ids.

WHY THIS EXISTS
---------------
A by-index resource fetch (`getObjRes(arc, N)`) depends on the archive's own
file table. On 2026-08-17 the `daLwood` port shipped an AUTHORED dzb index of 5
and crashed at boot; the arc says 7. The lesson filed with that crash was "the
generated header is one grep away" — but for `lwood` there IS no generated
header: the donor decomp's `d_a_lwood.h` is 34 lines with no index enum, and
`dRes_INDEX_LWOOD_DZB_ALWD_e` appears nowhere under `include/`.

So the durable rule is stronger and simpler:

    ** THE ARC IS THE AUTHORITY. A resource index is the file's position in
       its own RARC table. Read it; never infer it. **

This reads it, in seconds, with or without a header anywhere.

USAGE
    python arc_index.py <arc-path-or-object-name>
    python arc_index.py Lwood
    python arc_index.py "D:/XXXXXXX/Ex WW/files/res/Object/Lwood.arc"

OWNERSHIP: written by History/Bridge on the daLwood crash; handed to FOUNDRY as
the instruments lane. Extend or fold it into the oracle stack as you see fit.
"""
import os
import struct
import sys

EXTRACT = "D:/XXXXXXX/Ex WW/files/res/Object"


def yaz0(data):
    """Decompress Yaz0 if present; several object arcs ship compressed."""
    if data[:4] != b"Yaz0":
        return data
    out_size = struct.unpack_from(">I", data, 4)[0]
    out = bytearray(out_size)
    src, dst, code, bits = 16, 0, 0, 0
    while dst < out_size:
        if bits == 0:
            code = data[src]
            src += 1
            bits = 8
        if code & 0x80:
            out[dst] = data[src]
            src += 1
            dst += 1
        else:
            b1, b2 = data[src], data[src + 1]
            src += 2
            back = ((b1 & 0x0F) << 8 | b2) + 1
            n = (b1 >> 4) + 2
            if n == 2:
                n = data[src] + 0x12
                src += 1
            for _ in range(n):
                out[dst] = out[dst - back]
                dst += 1
        code <<= 1
        bits -= 1
    return bytes(out)


RES = "D:/XXXXXXX/Ex WW/files/res"
SEARCH_DIRS = ("Object", "Stage", "Particle", "Msg", "ActorDat")


def resolve(arg):
    """Exact match first; then a case-insensitive sweep of the res tree.

    NEVER fail silently on a near miss. The first draft of this tool searched
    only res/Object and returned a bare "not found" for `Knob00` — while
    `Knob.arc` sat right there. An index read off the wrong arc is the bug
    class this tool exists to prevent, so a resolver that shrugs at a near
    miss is worse than one that refuses: it invites the caller to guess.
    """
    if os.path.exists(arg):
        return arg
    for d in SEARCH_DIRS:
        for cand in (arg, arg + ".arc", arg.capitalize() + ".arc"):
            p = os.path.join(RES, d, cand)
            if os.path.exists(p):
                return p
    # No exact hit — collect near misses across the whole res tree and SHOW them.
    stem = os.path.splitext(os.path.basename(arg))[0].lower()
    near = []
    for root, _dirs, files in os.walk(RES):
        for f in files:
            if not f.lower().endswith(".arc"):
                continue
            fs = os.path.splitext(f)[0].lower()
            if stem.startswith(fs) or fs.startswith(stem) or stem in fs or fs in stem:
                near.append(os.path.join(root, f))
    if near:
        print("no exact match for %r - but these look close:" % arg)
        for p in sorted(near)[:12]:
            print("    %s" % p.replace("\\", "/"))
        print("\nre-run with one of the paths above. NOT guessing on your behalf:")
        print("a resource index read off the wrong arc is exactly the fault")
        print("this tool exists to catch.")
    return None


def dump(path):
    arc = yaz0(open(path, "rb").read())
    if arc[:4] != b"RARC":
        print("not a RARC archive: %r" % arc[:4])
        return 1

    info = 0x20
    file_off = struct.unpack_from(">I", arc, info + 0x0C)[0] + info
    file_count = struct.unpack_from(">I", arc, info + 0x10)[0]
    str_off = struct.unpack_from(">I", arc, info + 0x14)[0] + info

    print("%s" % os.path.basename(path))
    print("%d file-table entries\n" % file_count)
    print("  file_id  name                       size  note")

    # ------------------------------------------------------------------
    # BOUND EVERY READ. This tool was written on the back of a crash caused
    # by an unbounded table walk, and its own first draft walked past the
    # valid entries into garbage (file ids of 18766, sizes of 0xFFFFFF).
    # Shipping the defect I had just reviewed someone else for would have
    # been the joke of the week. Iteration caps limit ITERATIONS, not
    # ADDRESSES — every offset below is checked against len(arc) first.
    # ------------------------------------------------------------------
    for i in range(file_count):
        e = file_off + i * 0x14
        if e + 0x14 > len(arc):
            break
        fid = struct.unpack_from(">H", arc, e)[0]
        name_off = struct.unpack_from(">H", arc, e + 0x06)[0]
        s = str_off + name_off
        if s >= len(arc):
            continue
        end = arc.find(b"\0", s)
        if end < 0:
            continue
        name = arc[s:end].decode("ascii", "replace")
        if not name or any(not (32 <= ord(c) < 127) for c in name):
            continue
        if name in (".", ".."):
            continue
        dlen = struct.unpack_from(">I", arc, e + 0x0C)[0]
        # A data length larger than the whole archive is not a short read or a
        # quirk — it is proof we have walked off the real table into whatever
        # follows it. Stop, rather than print plausible-looking nonsense: a
        # garbage row with a printable name is exactly how an authored index
        # gets taken from a tool and trusted.
        if dlen > len(arc):
            break

        note = ""
        low = name.lower()
        if low.endswith((".bdl", ".bmd")):
            note = "<-- MODEL"
        elif low.endswith(".dzb"):
            note = "<-- DZB (collision)"
        elif low.endswith((".bck", ".bti", ".btp", ".brk")):
            note = ""
        # a directory node carries 0xFFFF rather than a real file id
        shown = "  dir  " if fid == 0xFFFF else "%7d" % fid
        print("  %s  %-24s %7d  %s" % (shown, name, dlen, note))
    return 0


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 2
    p = resolve(sys.argv[1])
    if p is None:
        # resolve() has already printed near misses if it found any. Do not
        # follow that with a line naming ONE directory — the search covers the
        # whole res tree now, and a message that contradicts the behaviour it
        # describes is how a reader concludes the tool cannot see something it
        # can. (Same stale-message class this tool's own banner was written on.)
        print("no arc matched %r anywhere under %s" % (sys.argv[1], RES))
        return 2
    return dump(p)


if __name__ == "__main__":
    sys.exit(main())
