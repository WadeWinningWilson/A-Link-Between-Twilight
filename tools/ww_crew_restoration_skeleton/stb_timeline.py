#!/usr/bin/env python3
# ============================================================================
# stb_timeline.py — §298 golden-trace: STB control/suspend/message timeline
#
# Answers History's four questions for a storyboard:
#   1. every suspend on the CONTROL track (frame + amount)
#   2. that suspend's release mechanism (later track op vs message-linked)
#   3. every MESSAGE op (frame + message code)
#   4. any branch commands before a given frame window
#
# Every rule transcribed from the RECEIVER's own processor (not guessed):
#   * sequence opcodes ......... TObject::process_sequence_
#       (libs/JSystem/src/JStudio/JStudio/stb.cpp:190):
#       0=end, 1=flag-op, 2=wait N frames, 3=branch (signed-24 offset,
#       target = addr_of_this_word + offset, getSequence_offset stb.h:89),
#       4=suspend(signed-24 amount), 0x80=paragraph block
#   * sequence word layout ..... TParse_TSequence::getData
#       (stb-data-parse.cpp:16): u32 BE, type=b31..24, param=b23..0;
#       type<=0x7F -> next=+4; else content=+4, next=+4+align4(param)
#   * paragraph layout ......... TParse_TParagraph::getData (:36) +
#       parseVariableUInt_16_32_following (JGadget/binary.cpp:18):
#       u16 head; bit15 clear -> size=u16, type=next u16 (4-byte head);
#       bit15 set -> size=((head&0x7FFF)<<16)|u16, type=u32 (8-byte head)
#   * paragraph type decode .... TObject_*::do_paragraph
#       (jstudio-object.cpp): group = type>>5, operation = type&0x1F;
#       MESSAGE: group 0x42, op 0x19 = setMessageCode(u32 content)
#       (donor JStudio_JMessage/object-message.cpp adaptor_do_MESSAGE)
#   * control block identity ... TParse::parseBlock_object (stb.cpp:439):
#       block type == BLOCK_NONE (-1) -> TObject_control; content layout
#       identical to TBlock_object (flag, idSize, id, then sequence)
#   * message release contract . donor d_mesg.cpp:2112 dMesg_closeProc ->
#       getControl()->unsuspend(1) fires ONLY when the box CLOSES
#
# Read-only analysis tool. Never writes game data. №31-clean.
# Usage: python stb_timeline.py <file.arc|file.stb> [member.stb] [--window N]
# ============================================================================
import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
from jstudio_stb import yaz0_dec, rarc_members  # container helpers (§172 reader)

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]
align4 = lambda n: (n + 3) & ~3


def s24(v):
    # gu32Mask_TSequence_value_signExpansion (stb-data.h): sign-extend 24->32
    return v - 0x1000000 if v & 0x800000 else v


def walk_track(data, seq_off, seq_end, name):
    """Walk one object's sequence stream exactly as TObject::forward would,
    accumulating frames from wait ops.  Returns event list."""
    events = []
    frame = 0
    off = seq_off
    seen = set()
    while off < seq_end:
        if off in seen:  # authored backward branch = loop; stop after 1 pass
            events.append((frame, "LOOP-REVISIT", off, None))
            break
        seen.add(off)
        head = be32(data, off)
        typ = head >> 24
        param = head & 0xFFFFFF
        nxt = off + 4 if typ <= 0x7F else off + 4 + align4(param)
        if typ == 0:
            events.append((frame, "END", off, None))
            break
        elif typ == 1:
            events.append((frame, "FLAG-OP", off, param))
        elif typ == 2:
            frame += param
            events.append((frame, "WAIT", off, param))
        elif typ == 3:
            tgt = off + s24(param)  # getSequence_offset: base = this word
            events.append((frame, "BRANCH", off, (s24(param), tgt)))
            nxt = tgt
        elif typ == 4:
            events.append((frame, "SUSPEND", off, s24(param)))
        elif typ == 0x80:
            # paragraph block: content=+4, byte length=param
            p = off + 4
            pend = p + param
            while p < pend:
                h = be16(data, p)
                if h & 0x8000:
                    size = ((h & 0x7FFF) << 16) | be16(data, p + 2)
                    ptype = be32(data, p + 4)
                    c = p + 8
                else:
                    size = h
                    ptype = be16(data, p + 2)
                    c = p + 4
                group, oper = ptype >> 5, ptype & 0x1F
                if group == 0x42 and oper == 0x19 and size == 4:
                    events.append((frame, "MESSAGE", p, be32(data, c)))
                else:
                    raw = data[c:c + min(size, 16)].hex()
                    events.append(
                        (frame, "PARA", p, (hex(ptype), group, oper, size, raw)))
                p = c + align4(size) if size else c
        else:
            events.append((frame, "UNKNOWN-%02x" % typ, off, param))
        off = nxt
    return events


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    window = 198
    for a in sys.argv[1:]:
        if a.startswith("--window"):
            window = int(a.split("=", 1)[1])
    path = Path(args[0])
    raw = path.read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    if raw[:4] == b"RARC":
        member = args[1] if len(args) > 1 else "tale.stb"
        found = None
        for nm, blob in rarc_members(raw):
            if nm.lower() == member.lower():
                found = blob
                break
        if found is None:
            sys.exit("member not found: " + member)
        raw = found
        if raw[:4] == b"Yaz0":
            raw = yaz0_dec(raw)
    assert raw[:4] == b"STB\0", "not an STB"
    nblocks = be32(raw, 0x0C)
    off = 0x20  # THeader size (stb-data.h)

    print("== %s: %d bytes, %d blocks ==" % (args[-1] if len(args) > 1 else path.name,
                                             len(raw), nblocks))
    all_events = {}
    for _ in range(nblocks):
        bsize = be32(raw, off)
        btype = be32(raw, off + 4)
        # object-shaped content (control block included; stb.cpp:439)
        idsize = be16(raw, off + 0xA)
        oid = raw[off + 0xC:off + 0xC + idsize].split(b"\0")[0].decode(
            "ascii", "replace")
        seq = off + 0xC + align4(idsize)
        tb = struct.pack(">I", btype)
        tag = ("CONTROL" if btype == 0xFFFFFFFF else
               tb.decode("ascii") if all(0x20 <= c < 0x7F for c in tb) else
               "0x%08X" % btype)
        name = "%s '%s'" % (tag, oid or "(anon)")
        all_events[name] = walk_track(raw, seq, off + bsize, name)
        off += bsize

    # -------- report --------
    for name, evs in all_events.items():
        interesting = [e for e in evs if e[1] != "WAIT" or True]
        print("\n[%s]" % name)
        for frame, kind, at, val in evs:
            if kind == "WAIT":
                continue  # frame column already carries the accumulation
            print("  @%5d  %-10s (file+0x%04x)  %s" % (frame, kind, at,
                                                       val if val is not None else ""))

    # -------- §298 answers --------
    print("\n" + "=" * 60)
    print("§298 ANSWERS (window = frame %d)" % window)
    print("=" * 60)
    sus = [(n, f, v) for n, evs in all_events.items()
           for f, k, _, v in evs if k == "SUSPEND"]
    print("1. suspend ops (track / frame / amount):")
    for n, f, v in sus or [("-", "-", "none authored")]:
        print("   %-22s @%s  suspend(%s)" % (n, f, v))
    br = [(n, f, v) for n, evs in all_events.items()
          for f, k, _, v in evs if k == "BRANCH" and isinstance(f, int) and f <= window]
    print("2. branch ops at/before frame %d:" % window)
    for n, f, v in br or [("-", "-", "none")]:
        print("   %-22s @%s  offset %s -> file+0x%x" % (n, f, v[0], v[1])
              if isinstance(v, tuple) else "   none")
    msg = [(f, v) for n, evs in all_events.items() if "JMSG" in n or "message" in n
           for f, k, _, v in evs if k == "MESSAGE"]
    print("3. MESSAGE ops (frame / message code):")
    for f, v in msg:
        print("   @%5d  setMessageCode(0x%04x = %d)" % (f, v, v))


if __name__ == "__main__":
    main()
