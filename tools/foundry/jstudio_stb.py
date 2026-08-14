#!/usr/bin/env python3
# ============================================================================
# jstudio_stb.py — faithful JStudio STB/FVB reader (Housing, ferry §172)
#
# Mirrors the donor decomp's JSystem/JStudio, layer for layer — it does NOT
# reinvent the format. Every parse rule cites its decomp source:
#
#   (a) STB header + typed block walk .... stb.cpp TParse::parseHeader_next
#       (0x80275768), stb-data.h THeader/TBlock/TBlock_object
#   (b) sequence + paragraph iteration ... stb-data-parse.cpp
#       TParse_TSequence::getData (0x80275A64), TParse_TParagraph::getData
#       (0x80275AB4), JGadget/binary.cpp parseVariableUInt_16_32_following
#       (0x802BFA48); sequence semantics: stb.cpp TObject::process_sequence_
#       (0x80274F88) + process_paragraph_reserved_ (0x802750DC)
#   (c) object data-paragraph decode ..... stb-data-parse.cpp
#       TParse_TParagraph_data::getData (0x80275B2C, gauDataSize table
#       stb-data.cpp), TParse_TParagraph_dataID (stb-data-parse.h);
#       paragraph-type naming: jstudio-object.cpp TObject_actor::do_paragraph
#       (0x8026F47C) and TObject_camera::do_paragraph (0x8026F9C4) —
#       type = (command << 5) | operation
#   (d) FVB function-value curves ........ fvb.cpp TObject::prepare
#       (0x802739AC) attribute paragraphs 0x10-0x16 + per-kind prepare_data_
#       (composite/constant/transition/list/list_parameter/hermite,
#       0x80273CB8-0x8027404C), fvb-data-parse.cpp TParse_TParagraph::getData
#
# Reads a raw .stb/.fvb, or extracts a member from a (optionally Yaz0'd)
# RARC .arc. Output: human-readable report (default) or --json.
#
# Read-only analysis tool. Never writes game data. №31-clean.
# ============================================================================
import json
import os
import struct
import sys
from pathlib import Path

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
bef32 = lambda d, o: struct.unpack_from(">f", d, o)[0]
align4 = lambda n: (n + 3) & ~3


# ---------------------------------------------------------------------------
# container helpers (Yaz0 + RARC member fetch; mirrors adapt_bdl_arcs.yaz0_dec)
# ---------------------------------------------------------------------------
def yaz0_dec(d):
    size = be32(d, 4)
    out = bytearray(size)
    si, di = 16, 0
    code, bits = 0, 0
    while di < size:
        if bits == 0:
            code = d[si]; si += 1; bits = 8
        if code & 0x80:
            out[di] = d[si]; si += 1; di += 1
        else:
            b1, b2 = d[si], d[si + 1]; si += 2
            dist = ((b1 & 0xF) << 8) | b2
            src = di - (dist + 1)
            n = b1 >> 4
            if n == 0:
                n = d[si] + 0x12
                si += 1
            else:
                n += 2
            for _ in range(n):
                out[di] = out[src]; di += 1; src += 1
                if di >= size:
                    break
        code <<= 1; bits -= 1
    return bytes(out)


def rarc_members(a):
    """Yield (name, bytes) for every file entry in a RARC image."""
    if a[:4] == b"Yaz0":
        a = yaz0_dec(a)
    if a[:4] != b"RARC":
        raise ValueError("not a RARC archive")
    data_abs = 0x20 + be32(a, 0x0C)
    info = 0x20
    n = be32(a, info + 8)
    ent = info + be32(a, info + 0xC)
    strs = info + be32(a, info + 0x14)
    for i in range(n):
        e = ent + i * 0x14
        attr = be16(a, e + 4)
        if attr & 0x1100 != 0x1100:  # files only
            continue
        no = be16(a, e + 6)
        end = a.index(b"\0", strs + no)
        name = a[strs + no:end].decode("ascii", "replace")
        off = be32(a, e + 8)
        size = be32(a, e + 12)
        yield name, a[data_abs + off: data_abs + off + size]


# ---------------------------------------------------------------------------
# (b) JGadget::binary::parseVariableUInt_16_32_following — exact mirror
#     16-bit form: [u16 size][u16 type], next=+4
#     32-bit form (first u16 high bit): size=((w0<<16)&0x7fff0000)|w1,
#     type=u32@+4, next=+8
# ---------------------------------------------------------------------------
def parse_variable_uint_16_32(d, o):
    w0 = be16(d, o)
    if (w0 & 0x8000) == 0:
        return w0, be16(d, o + 2), o + 4          # size, type, content_off
    size = ((w0 << 16) & 0x7FFF0000) | be16(d, o + 2)
    return size, be32(d, o + 4), o + 8


# ---------------------------------------------------------------------------
# (c) naming tables — jstudio-object.cpp dispatch: type=(command<<5)|operation
# ---------------------------------------------------------------------------
# TObject_actor::do_paragraph (0x8026F47C)
ACTOR_CMDS = {
    9: "TRANSLATION_X", 10: "TRANSLATION_Y", 11: "TRANSLATION_Z",
    12: "TRANSLATION_XYZ",
    13: "ROTATION_X", 14: "ROTATION_Y", 15: "ROTATION_Z",
    16: "ROTATION_XYZ",
    17: "SCALING_X", 18: "SCALING_Y", 19: "SCALING_Z",
    20: "SCALING_XYZ",
    48: "PARENT", 49: "PARENT_NODE", 50: "PARENT_ENABLE",
    51: "RELATION", 52: "RELATION_NODE", 53: "RELATION_ENABLE",
    57: "SHAPE", 58: "ANIMATION", 59: "ANIMATION_FRAME",
    67: "ANIMATION_MODE",
    75: "TEXTURE_ANIMATION_FRAME",
    76: "TEXTURE_ANIMATION", 78: "TEXTURE_ANIMATION_MODE",
}
# TObject_camera::do_paragraph (0x8026F9C4). vv6-vv9 single-value channels:
# decomp names not in the dispatch itself — left as vv-index (candidates:
# roll/fov/near/far family). POSITION/TARGET/DISTANCE arrays are named.
CAMERA_CMDS = {
    21: "POSITION_X", 22: "POSITION_Y", 23: "POSITION_Z",
    24: "POSITION_XYZ",
    25: "TARGET_X", 26: "TARGET_Y", 27: "TARGET_Z",
    28: "TARGET_POSITION_XYZ",
    38: "vv7", 39: "vv6", 40: "vv8", 41: "vv9",
    42: "DISTANCE_NEAR_FAR",
    48: "PARENT", 49: "PARENT_NODE", 50: "PARENT_ENABLE",
}
# jstudio-object.cpp adaptor_setVariableValue (0x8026EB18)
OPERATIONS = {
    1: "VOID", 2: "IMMEDIATE", 3: "TIME",
    0x10: "FVR_NAME", 0x11: "op11", 0x12: "FVR_INDEX",
    0x18: "op18", 0x19: "op19",  # seen in shipped STBs; dispatch passes through
}
BLOCK_NAMES = {
    b"JACT": "actor", b"JCMR": "camera", b"JSND": "sound", b"JMSG": "message",
    b"JABL": "ambientlight", b"JFOG": "fog", b"JLIT": "light",
    b"JPTC": "particle",
}
# stb-data.cpp gauDataSize_TEParagraph_data
DATA_SIZES = [0x0, 0x1, 0x2, 0x4, 0x8, 0x10, 0x20, 0x40]

FVB_KINDS = {1: "composite", 2: "constant", 3: "transition", 4: "list",
             5: "list_parameter", 6: "hermite"}  # fvb.cpp TFactory::create
FVB_ATTRS = {0x10: "refer_id", 0x11: "refer_index", 0x12: "range",
             0x13: "progress", 0x14: "adjust", 0x15: "outside",
             0x16: "interpolate"}  # fvb.cpp TObject::prepare


def cmd_name(block_type, ptype):
    cmd, op = ptype >> 5, ptype & 0x1F
    table = ACTOR_CMDS if block_type == b"JACT" else (
        CAMERA_CMDS if block_type == b"JCMR" else {})
    return table.get(cmd, f"cmd{cmd}"), OPERATIONS.get(op, f"op{op:#x}"), cmd, op


# ---------------------------------------------------------------------------
# (c) TParse_TParagraph_data::getData (0x80275B2C) — exact mirror
# ---------------------------------------------------------------------------
def parse_paragraph_data(d, o):
    b = d[o]
    out = {"status": b & ~0x8, "entryCount": 0, "entrySize": 0, "entries": []}
    if b == 0:
        return out
    p = o + 1
    count = 1
    if b & 8:
        count = d[p]; p += 1
    out["entryCount"] = count
    esize = DATA_SIZES[b & 7]
    out["entrySize"] = esize
    if esize:
        for i in range(count):
            out["entries"].append(d[p + i * esize: p + (i + 1) * esize].hex())
    return out


# TParse_TParagraph_dataID (stb-data-parse.h): u16 _0, u16 id_size, id, align4
def parse_data_id(d, o, size):
    id_size = be16(d, o + 2)
    ident = d[o + 4: o + 4 + id_size]
    coff = o + 4 + align4(id_size)
    txt = ident.rstrip(b"\0")
    name = (txt.decode("ascii") if txt and all(0x20 <= c < 0x7F for c in txt)
            else "0x" + ident.hex())
    return name, d[coff: o + size]


# ---------------------------------------------------------------------------
# (b) sequence walk — TObject::process_sequence_ (0x80274F88)
# ---------------------------------------------------------------------------
def walk_sequence(d, start, end, block_type):
    """Yield event dicts for one object's sequence bytes [start, end)."""
    o = start
    while o < end:
        head = be32(d, o)
        typ, param = head >> 24, head & 0xFFFFFF
        if typ == 0:
            yield {"seq": "END", "off": o}
            return
        if typ == 1:
            yield {"seq": "FLAG_OP", "off": o, "param": param}; o += 4
        elif typ == 2:
            yield {"seq": "WAIT", "off": o, "frames": param}; o += 4
        elif typ == 3:
            v = param | 0xFF000000 if param & 0x800000 else param
            yield {"seq": "JUMP", "off": o,
                   "target": v - (1 << 32) if v >> 31 else v}
            o += 4
        elif typ == 4:
            yield {"seq": "SUSPEND", "off": o, "frames": param}; o += 4
        elif typ >= 0x80:
            content, nxt = o + 4, o + 4 + param
            yield from walk_paragraphs(d, content, nxt, block_type)
            o = nxt
        else:
            yield {"seq": f"UNKNOWN_{typ:#x}", "off": o}; o += 4


def walk_paragraphs(d, start, end, block_type):
    """process_sequence_ case 0x80 paragraph loop + reserved handling."""
    o = start
    while o < end:
        size, ptype, coff = parse_variable_uint_16_32(d, o)
        nxt = coff + align4(size) if size else coff
        if ptype <= 0xFF:
            # process_paragraph_reserved_ (0x802750DC)
            if ptype == 0x1:
                yield {"para": "FLAG_OP", "off": o, "value": be32(d, coff)}
            elif ptype == 0x2:
                yield {"para": "WAIT", "off": o, "frames": be32(d, coff)}
            elif ptype == 0x3:
                yield {"para": "SEQ_OFFSET", "off": o,
                       "value": struct.unpack_from(">i", d, coff)[0]}
            elif ptype == 0x80:
                yield {"para": "DATA", "off": o, "id": None,
                       "data": parse_paragraph_data(d, coff),
                       "raw": d[coff:coff + size].hex()}
            elif ptype == 0x81:
                ident, content = parse_data_id(d, coff, size)
                yield {"para": "DATA_ID", "off": o, "id": ident,
                       "data": parse_paragraph_data(content, 0)
                       if content else None,
                       "raw": content.hex()}
            elif ptype == 0x82:
                yield {"para": "NOP_82", "off": o}
            else:
                yield {"para": f"RESERVED_{ptype:#x}", "off": o,
                       "raw": d[coff:coff + size].hex()}
        else:
            name, opname, cmd, op = cmd_name(block_type, ptype)
            ev = {"para": name, "op": opname, "type": f"{ptype:#x}",
                  "cmd": cmd, "off": o, "size": size}
            if size == 4:
                ev["u32"] = be32(d, coff)
                ev["f32"] = round(bef32(d, coff), 6)
            elif size in (8, 12, 16) and size % 4 == 0:
                ev["f32s"] = [round(bef32(d, coff + i * 4), 6)
                              for i in range(size // 4)]
                ev["u32s"] = [f"{be32(d, coff + i * 4):#x}"
                              for i in range(size // 4)]
            elif size:
                ev["raw"] = d[coff:coff + size].hex()
            yield ev
        o = nxt


# ---------------------------------------------------------------------------
# (a) STB header + block walk — stb.cpp TParse (0x80275768)
# ---------------------------------------------------------------------------
def parse_stb(d):
    if d[:4] != b"STB\0":
        raise ValueError("bad signature (want 'STB\\0', got %r)" % d[:4])
    if be16(d, 4) != 0xFEFF:
        raise ValueError("bad byte order")
    version = be16(d, 6)
    if not (1 <= version <= 3):
        raise ValueError("unsupported version %d" % version)
    nblocks = be32(d, 0x0C)
    target = d[0x10:0x18].rstrip(b"\0").decode("ascii", "replace")
    out = {"signature": "STB", "version": version,
           "target": target, "target_version": be16(d, 0x1E),
           "block_count": nblocks, "objects": []}
    o = 0x20
    for _ in range(nblocks):
        size = be32(d, o)
        btype = d[o + 4:o + 8]
        if be32(d, o + 4) == 0xFFFFFFFF:      # BLOCK_NONE → control object
            obj = {"type": "control", "id": "(control)",
                   "events": list(walk_sequence(d, o + 8, o + size, None))}
        elif btype == b"JFVB":
            # embedded function-value bank: block content is a complete FVB
            # image (own THeader) — route through the layer-(d) parser.
            inner = parse_fvb(bytes(d[o + 8: o + size]))
            obj = {"type": "fvb", "fourcc": "JFVB", "id": "(curves)",
                   "curves": inner["curves"], "events": []}
        else:
            flag = be16(d, o + 8)
            id_size = be16(d, o + 10)
            ident = d[o + 12:o + 12 + id_size].rstrip(b"\0").decode(
                "ascii", "replace")
            content = o + 12 + align4(id_size)
            obj = {"type": BLOCK_NAMES.get(btype, btype.decode("ascii",
                                                               "replace")),
                   "fourcc": btype.decode("ascii", "replace"),
                   "id": ident, "flag": f"{flag:#x}",
                   "events": list(walk_sequence(d, content, o + size,
                                                btype))}
        out["objects"].append(obj)
        o += size
    return out


# ---------------------------------------------------------------------------
# (d) FVB — fvb.cpp TParse/TObject::prepare + per-kind prepare_data_
# ---------------------------------------------------------------------------
def parse_fvb_data(kind, d, o, size):
    """Mirror per-kind prepare_data_ (fvb.cpp 0x80273CB8-0x8027404C)."""
    if kind == 2:                    # constant: data_set(f32)
        return {"value": bef32(d, o)}
    if kind == 3:                    # transition: data_set(f32, f32)
        return {"first": bef32(d, o), "second": bef32(d, o + 4)}
    if kind == 4:                    # list: setInterval(_0); set(_8, _4)
        return {"interval_u32": be32(d, o), "interval_f32": bef32(d, o),
                "count": be32(d, o + 4),
                "values": [bef32(d, o + 8 + i * 4)
                           for i in range(min(be32(d, o + 4), 64))]}
    if kind == 5:                    # list_parameter: set(_4, _0)
        cnt = be32(d, o)
        return {"count": cnt,
                "values": [bef32(d, o + 4 + i * 4)
                           for i in range(min(cnt * 2, 128))]}
    if kind == 6:                    # hermite: set(_4, _0&0xFFFFFFF, _0>>28)
        head = be32(d, o)
        cnt, stride = head & 0xFFFFFFF, head >> 28
        keys = [round(bef32(d, o + 4 + i * 4), 6)
                for i in range(min(cnt * stride, 512))]
        return {"count": cnt, "stride": stride,
                "keys": [keys[i:i + stride]
                         for i in range(0, len(keys), stride)]}
    return {"raw": d[o:o + size].hex()}   # composite (1) / unknown


def parse_fvb(d):
    if d[:4] != b"FVB\0":
        raise ValueError("bad signature (want 'FVB\\0')")
    if be16(d, 4) != 0xFEFF:
        raise ValueError("bad byte order")
    nblocks = be32(d, 0x0C)
    out = {"signature": "FVB", "version": be16(d, 6),
           "block_count": nblocks, "curves": []}
    o = 0x10
    for _ in range(nblocks):
        size = be32(d, o)
        kind = be16(d, o + 4)
        id_size = be16(d, o + 6)
        ident = d[o + 8:o + 8 + id_size].rstrip(b"\0").decode("ascii",
                                                              "replace")
        p = o + 8 + align4(id_size)
        curve = {"kind": FVB_KINDS.get(kind, f"kind{kind}"), "id": ident,
                 "attributes": {}, "data": None}
        end = o + size
        while p < end:
            psize, ptype, coff = parse_variable_uint_16_32(d, p)
            if ptype == 0:
                break
            if ptype == 1:
                curve["data"] = parse_fvb_data(kind, d, coff, psize)
            elif ptype in FVB_ATTRS:
                if ptype == 0x12:
                    curve["attributes"]["range"] = [bef32(d, coff),
                                                    bef32(d, coff + 4)]
                elif ptype == 0x15:
                    curve["attributes"]["outside"] = [be16(d, coff),
                                                      be16(d, coff + 2)]
                else:
                    curve["attributes"][FVB_ATTRS[ptype]] = be32(d, coff)
            else:
                curve["attributes"][f"para_{ptype:#x}"] = d[
                    coff:coff + psize].hex()
            p = coff + align4(psize) if psize else coff
        out["curves"].append(curve)
        o += size
    return out


# ---------------------------------------------------------------------------
# report
# ---------------------------------------------------------------------------
def report_stb(parsed, verbose):
    lines = [f"STB v{parsed['version']} target={parsed['target']} "
             f"blocks={parsed['block_count']}"]
    for obj in parsed["objects"]:
        if obj["type"] == "fvb":
            lines.append(f"\n[fvb] {len(obj['curves'])} curves:")
            for c in obj["curves"]:
                d = c["data"] or {}
                lines.append(f"  {c['kind']} id='{c['id']}' "
                             f"attrs={c['attributes']} "
                             f"keys={d.get('count', d.get('value', '-'))} "
                             f"stride={d.get('stride', '-')}")
            continue
        lines.append(f"\n[{obj['type']}] id='{obj['id']}' "
                     f"flag={obj.get('flag', '-')} "
                     f"events={len(obj['events'])}")
        t = 0
        for ev in obj["events"]:
            if "seq" in ev:
                if ev["seq"] == "WAIT":
                    t += ev["frames"]
                    lines.append(f"  @{t:>5} WAIT {ev['frames']}")
                elif verbose or ev["seq"] != "END":
                    lines.append(f"  @{t:>5} {ev['seq']} "
                                 f"{ {k: v for k, v in ev.items() if k not in ('seq', 'off')} }")
            elif "para" in ev:
                kv = {k: v for k, v in ev.items()
                      if k not in ("para", "off", "cmd")}
                lines.append(f"  @{t:>5} {ev['para']} {kv}")
    return "\n".join(lines)


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    opts = [a for a in sys.argv[1:] if a.startswith("--")]
    if not args:
        print("usage: jstudio_stb.py <file.stb|file.fvb|file.arc> "
              "[member.stb] [--json[=out.json]] [--verbose]")
        return 1
    path = Path(args[0])
    raw = path.read_bytes()
    if raw[:4] in (b"Yaz0", b"RARC"):
        members = dict(rarc_members(raw))
        want = args[1] if len(args) > 1 else None
        stbs = {n: b for n, b in members.items()
                if n.endswith(".stb") or n.endswith(".fvb")}
        if want is None:
            if not stbs:
                print("no .stb/.fvb members; archive holds:",
                      ", ".join(members))
                return 1
            print("members:", ", ".join(stbs))
            want = next(iter(stbs))
            print("defaulting to first:", want)
        raw = members[want]
    parsed = parse_fvb(raw) if raw[:4] == b"FVB\0" else parse_stb(raw)
    jopt = next((o for o in opts if o.startswith("--json")), None)
    if jopt:
        text = json.dumps(parsed, indent=1)
        if "=" in jopt:
            Path(jopt.split("=", 1)[1]).write_text(text, encoding="utf-8")
            print("wrote", jopt.split("=", 1)[1])
        else:
            print(text)
    else:
        if parsed["signature"] == "STB":
            print(report_stb(parsed, "--verbose" in opts))
        else:
            print(json.dumps(parsed, indent=1))
    return 0


if __name__ == "__main__":
    sys.exit(main())
