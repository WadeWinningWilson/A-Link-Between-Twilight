#!/usr/bin/env python3
"""Decode a JStudio storyboard (.stb) using the RECEIVER's own parse rules.

Every rule here is transcribed from receiver source, not guessed:

  * header/block layout  -> libs/JSystem/include/JSystem/JStudio/JStudio/stb-data.h
  * sequence walk        -> TParse_TSequence::getData      (stb-data-parse.cpp:12)
  * paragraph walk       -> TParse_TParagraph::getData     (stb-data-parse.cpp:36)
  * paragraph data       -> TParse_TParagraph_data::getData(stb-data-parse.cpp:52)
  * varint 16/32         -> parseVariableUInt_16_32_following (JGadget/binary.cpp:18)
  * data entry sizes     -> gauDataSize_TEParagraph_data   (stb-data.cpp:5)

Everything is big-endian. Usage:

    python decode_stb.py <file.stb>            # structure
    python decode_stb.py <file.stb> --verbose  # + raw paragraph payloads
"""
from __future__ import annotations

import struct
import sys
from pathlib import Path

# gauDataSize_TEParagraph_data (stb-data.cpp:5)
DATA_SIZE = [0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40]


def u16(b, o):
    return struct.unpack_from(">H", b, o)[0]


def u32(b, o):
    return struct.unpack_from(">I", b, o)[0]


def align4(n):
    return (n + 3) & ~3


def parse_varint(b, o):
    """parseVariableUInt_16_32_following -> (first, second, next_offset)."""
    v = u16(b, o)
    if (v & 0x8000) == 0:
        return v, u16(b, o + 2), o + 4
    v = ((v & 0x7FFF) << 16) | u16(b, o + 2)
    return v, u32(b, o + 4), o + 8


def parse_paragraph_data(b, o):
    """TParse_TParagraph_data::getData -> (status, count, size, content_off)."""
    if o >= len(b):
        return None
    r29 = b[o]
    status = r29 & ~0x8
    if r29 == 0:
        return status, 0, 0, None
    o += 1
    count = 1
    if r29 & 8:
        count = b[o]
        o += 1
    r27 = r29 & 7
    size = DATA_SIZE[r27] if r27 else 0
    return status, count, size, o


def walk_paragraphs(b, start, end, verbose):
    """A sequence entry's content is a chain of paragraphs."""
    out = []
    o = start
    while o < end:
        try:
            param, ptype, after = parse_varint(b, o)
        except struct.error:
            break
        if param == 0 and ptype == 0:
            break
        content = after if param else None
        nxt = after + align4(param) if param else after
        rec = {"type": ptype, "param": param, "off": o}
        if content is not None and content < len(b):
            pd = parse_paragraph_data(b, content)
            if pd:
                rec["status"], rec["count"], rec["esize"], coff = pd
                if coff is not None and rec["esize"]:
                    n = rec["esize"] * rec["count"]
                    rec["raw"] = b[coff:coff + n]
            if verbose:
                rec["bytes"] = b[content:content + min(param, 32)]
        out.append(rec)
        if nxt <= o:
            break
        o = nxt
    return out


def walk_sequence(b, start, end, verbose):
    """TParse_TSequence::getData."""
    out = []
    o = start
    while o + 4 <= end:
        head = u32(b, o)
        typ = head >> 24
        param = head & 0xFFFFFF
        if typ == 0:
            break
        nxt = o + 4
        content = None
        if typ > 0x7F:
            content = nxt
            nxt = nxt + param
        out.append({"type": typ, "param": param, "off": o, "content": content,
                    "paras": walk_paragraphs(b, content, min(content + param, end), verbose)
                    if content is not None else []})
        if nxt <= o:
            break
        o = nxt
    return out


def main():
    if len(sys.argv) < 2:
        print(__doc__)
        return 1
    path = Path(sys.argv[1])
    verbose = "--verbose" in sys.argv
    b = path.read_bytes()

    sig = b[0:4]
    print(f"file      : {path.name}  ({len(b)} B)")
    print(f"signature : {sig!r}")
    print(f"byte_order: 0x{u16(b, 4):04X}   version: {u16(b, 6)}")
    nblocks = u32(b, 0x0C)
    target = b[0x10:0x18].split(b'\0')[0].decode(errors="replace")
    print(f"blocks    : {nblocks}   target: {target} v{u16(b, 0x1E)}")
    print()

    o = 0x20
    for i in range(nblocks):
        if o + 8 > len(b):
            break
        size = u32(b, o)
        typ = b[o + 4:o + 8].decode(errors="replace")
        end = o + size
        line = f"[{i}] {typ}  size={size}"
        body = o + 8
        if typ in ("JACT", "JCMR", "JMSG", "JSND", "JLIT", "JFOG", "JPTC", "JABL"):
            flag = u16(b, body)
            idsz = u16(b, body + 2)
            ident = b[body + 4:body + 4 + idsz].split(b'\0')[0].decode(errors="replace")
            line += f"  flag=0x{flag:04X}  id='{ident}'"
            body = body + 4 + align4(idsz)
        print(line)
        for s in walk_sequence(b, body, end, verbose):
            print(f"      seq type=0x{s['type']:02X} param={s['param']}"
                  f" paras={len(s['paras'])}")
            for p in s["paras"]:
                extra = ""
                if "raw" in p:
                    r = p["raw"]
                    ints = [int.from_bytes(r[k:k + 4], "big", signed=True)
                            for k in range(0, len(r) - 3, 4)]
                    flts = []
                    for k in range(0, len(r) - 3, 4):
                        flts.append(round(struct.unpack(">f", r[k:k + 4])[0], 3))
                    extra = (f" count={p.get('count')} esize={p.get('esize')}"
                             f" ints={ints[:8]} f32={flts[:8]}")
                print(f"          para type=0x{p['type']:X} param={p['param']}"
                      f" status={p.get('status')}{extra}")
                if verbose and "bytes" in p:
                    print(f"              raw={p['bytes'].hex()}")
        o = end
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
