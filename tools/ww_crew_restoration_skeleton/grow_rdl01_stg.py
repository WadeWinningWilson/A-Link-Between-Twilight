#!/usr/bin/env python3
"""№97/№102: grow R_DL01 STG_00.arc to 6 rooms (room0..5 + RTBL/MULT).

History already delivered R01_00..R05_00.arc + ext_bg2..6 manifests.
This expands the stage shell so native room streaming can request them.

Writes AppData mod-side only (never commit WW arcs):
  .../WW-Crew-Restoration/files/res/Stage/R_DL01/STG_00.arc

Reference shape: TP R_SP01 uses RTBL n=8 / MULT n=6; we use RTBL=MULT=6 for
contiguous rooms 0..5 (room = bgN − 1).

№102: RTBL pointer / m_rooms offsets are FILE-ABSOLUTE (dzs base + param_3),
never relative to the RTBL payload. Payload-relative 0x18/0x20… land in the
chunk-header directory ("MULT"/"RCAM") and the play scene creates no rooms.

Usage:
  grow_rdl01_stg.py               (no arguments — ROOM_COUNT = 6 is a constant)

  Inputs   : <mod>/files/res/Stage/R_DL01/STG_00.arc — must ALREADY EXIST (built
             by build_rdl01_shell.py); read for stage.dzs and room0.dzs's FILI
  Outputs  : the SAME arc, rewritten in place, RTBL = MULT = 6 (rooms 0..5)
             backups .pre97-bak and .pre102-bak, each written once
  Idempotent: yes in shape — stage.dzs is REBUILT to 6 rooms from room0's FILI on
             every run rather than extended, so a second run converges on the
             same arc instead of growing it to 12.
  Order    : after build_rdl01_shell.py; before anything that expects rooms 1-5.
             Independent of build_fdl_host_stg.py — neither reads the other's
             output, so a recipe may run them in either order or in parallel.
  R1 note  : both backups are WRITE-ONCE, so they hold the arc as it stood on the
             FIRST run — the pre-grow shell — not the previous run. Do not treat
             .pre97-bak as an undo for the most recent execution; it is a
             pre-№97 snapshot and nothing else.
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

ROOM_COUNT = 6
MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
STG_PATH = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"


def be32(n: int) -> bytes:
    return struct.pack(">I", n & 0xFFFFFFFF)


def yaz0_decompress(data: bytes) -> bytes:
    if data[:4] != b"Yaz0":
        return data
    size = struct.unpack(">I", data[4:8])[0]
    src = memoryview(data)[16:]
    dst = bytearray()
    i = 0
    while len(dst) < size:
        code = src[i]
        i += 1
        for bit in range(8):
            if len(dst) >= size:
                break
            if code & (0x80 >> bit):
                dst.append(src[i])
                i += 1
            else:
                b1 = src[i]
                b2 = src[i + 1]
                i += 2
                dist = ((b1 & 0x0F) << 8) | b2
                copy = b1 >> 4
                if copy == 0:
                    copy = src[i] + 0x12
                    i += 1
                else:
                    copy += 2
                for _ in range(copy):
                    dst.append(dst[-dist - 1])
    return bytes(dst)


def list_rarc_files(data: bytes) -> list[tuple[str, bytes]]:
    data = yaz0_decompress(data)
    assert data[:4] == b"RARC"
    data_abs = 0x20 + struct.unpack(">I", data[0x0C:0x10])[0]
    info = 0x20
    n = struct.unpack(">I", data[info + 8 : info + 12])[0]
    ent = info + struct.unpack(">I", data[info + 12 : info + 16])[0]
    strs = info + struct.unpack(">I", data[info + 20 : info + 24])[0]
    out: list[tuple[str, bytes]] = []
    for i in range(n):
        e = ent + i * 0x14
        flags = struct.unpack(">H", data[e + 4 : e + 6])[0]
        if flags & 0x1100 != 0x1100:
            continue
        name_off = struct.unpack(">H", data[e + 6 : e + 8])[0]
        name = data[strs + name_off : data.index(0, strs + name_off)].decode("ascii")
        doff = struct.unpack(">I", data[e + 8 : e + 12])[0]
        size = struct.unpack(">I", data[e + 12 : e + 16])[0]
        out.append((name, data[data_abs + doff : data_abs + doff + size]))
    return out


def parse_dzs_chunks(blob: bytes) -> list[tuple[str, int, bytes]]:
    n = struct.unpack(">I", blob[:4])[0]
    chunks = []
    for i in range(n):
        o = 4 + i * 12
        tag = blob[o : o + 4].decode("ascii")
        ent, off = struct.unpack(">II", blob[o + 4 : o + 12])
        # Data extent: until next chunk data or EOF — use next offset sorted.
        chunks.append((tag, ent, off))
    # Resolve payloads by sorted offsets
    offs = sorted({off for _, _, off in chunks})
    off_end = {offs[i]: (offs[i + 1] if i + 1 < len(offs) else len(blob)) for i in range(len(offs))}
    out = []
    for tag, ent, off in chunks:
        out.append((tag, ent, blob[off : off_end[off]]))
    return out


def build_room_dzs(fili_payload: bytes) -> bytes:
    """Minimal roomN.dzs: FILI n=1 (same interior FILI as room0)."""
    # header count=1, node FILI, data at 0x10
    hdr = be32(1) + b"FILI" + be32(1) + be32(0x10)
    # pad header to 0x10 then FILI data
    assert len(hdr) == 0x10
    return hdr + fili_payload


def rtbl_payload_size(room_count: int) -> int:
    # [N × u32 ptr][N × 8-byte roomRead_data][N × u8 room index]
    return 4 * room_count + 8 * room_count + room_count


def build_rtbl_data(room_count: int, rtbl_abs_off: int) -> bytes:
    """RTBL payload at file offset rtbl_abs_off.

    Layout (TP R_SP01 shape; record size is 8, not 12):
      [N × u32]           file-absolute pointers to roomRead_data
      [N × 8]             roomRead_data {num,f1,f2,pad, m_rooms abs}
      [N × u8]            load-room index bytes (0x80|room for ChkBg)
    """
    ptr_array_size = 4 * room_count
    data_base = rtbl_abs_off + ptr_array_size
    rooms_base = data_base + 8 * room_count

    ptrs = bytearray()
    datas = bytearray()
    room_bytes = bytearray()
    for i in range(room_count):
        data_off = data_base + i * 8
        rooms_off = rooms_base + i
        ptrs += be32(data_off)
        # roomRead_data: num=1, f1=0, f2=0, pad=0, m_rooms=rooms_off (file abs)
        datas += bytes([1, 0, 0, 0]) + be32(rooms_off)
        # 0x80 = ChkBg (create ROOM_SCENE); low 6 bits = room index
        room_bytes.append(0xC0 | (i & 0x3F))

    return bytes(ptrs) + bytes(datas) + bytes(room_bytes)


def build_mult_data(room_count: int) -> bytes:
    # MULT: N × Mult_info {f32 x, f32 y, s16 angle, u8 room, u8 pad}
    mult = bytearray()
    for i in range(room_count):
        mult += struct.pack(">ffhBB", 0.0, 0.0, 0, i, 0)
    return bytes(mult)


def assert_rtbl_pointers(stage: bytes, room_count: int) -> None:
    """№102: refuse if any RTBL pointer lands in the chunk-header directory."""
    n = struct.unpack(">I", stage[:4])[0]
    chunk_dir_end = 4 + n * 12
    rtbl_off = None
    for i in range(n):
        o = 4 + i * 12
        tag = stage[o : o + 4].decode("ascii")
        ent, off = struct.unpack(">II", stage[o + 4 : o + 12])
        if tag == "RTBL":
            if ent != room_count:
                raise SystemExit(f"RTBL n={ent} expected {room_count}")
            rtbl_off = off
            break
    if rtbl_off is None:
        raise SystemExit("stage.dzs missing RTBL")

    for i in range(room_count):
        ptr = struct.unpack(">I", stage[rtbl_off + i * 4 : rtbl_off + i * 4 + 4])[0]
        if ptr < chunk_dir_end:
            sample = stage[ptr : ptr + 4]
            raise SystemExit(
                f"№102 RTBL ptr[{i}]=0x{ptr:x} lands in chunk directory "
                f"(dir_end=0x{chunk_dir_end:x}) bytes={sample!r} — refuse write"
            )
        if ptr + 8 > len(stage):
            raise SystemExit(f"№102 RTBL ptr[{i}]=0x{ptr:x} OOB")
        rec = stage[ptr : ptr + 8]
        if rec[0] != 1:
            raise SystemExit(f"№102 RTBL record[{i}] num={rec[0]} (want 1) at 0x{ptr:x}")
        rooms_off = struct.unpack(">I", rec[4:8])[0]
        if rooms_off < chunk_dir_end or rooms_off >= len(stage):
            raise SystemExit(
                f"№102 RTBL m_rooms[{i}]=0x{rooms_off:x} invalid (dir_end=0x{chunk_dir_end:x})"
            )


def rebuild_stage_dzs(old_stage: bytes, room_count: int) -> bytes:
    chunks = parse_dzs_chunks(old_stage)

    # Preserve order; RTBL/MULT rebuilt with absolute offsets (№102).
    # First pass: sizes only (RTBL size known a priori).
    ordered_meta: list[tuple[str, int, int]] = []  # tag, ent, payload_len
    for tag, ent, payload in chunks:
        if tag == "RTBL":
            ordered_meta.append((tag, room_count, rtbl_payload_size(room_count)))
        elif tag == "MULT":
            ordered_meta.append((tag, room_count, 12 * room_count))
        else:
            ordered_meta.append((tag, ent, len(payload)))

    data_start = 4 + len(ordered_meta) * 12
    cursor = data_start
    final_meta: list[tuple[str, int, int, int]] = []  # tag, ent, off, plen
    for tag, ent, plen in ordered_meta:
        while cursor % 4:
            cursor += 1
        final_meta.append((tag, ent, cursor, plen))
        cursor += plen

    # Second pass: build payloads with known absolute RTBL offset.
    other_payload = {tag: payload for tag, _ent, payload in chunks}
    payloads: dict[str, bytes] = {}
    for tag, ent, off, plen in final_meta:
        if tag == "RTBL":
            payloads[tag] = build_rtbl_data(room_count, off)
            assert len(payloads[tag]) == plen
        elif tag == "MULT":
            payloads[tag] = build_mult_data(room_count)
            assert len(payloads[tag]) == plen
        else:
            payloads[tag] = other_payload[tag]

    out = bytearray()
    out += be32(len(final_meta))
    for tag, ent, off, _plen in final_meta:
        out += tag.encode("ascii") + be32(ent) + be32(off)
    assert len(out) == data_start
    for tag, ent, off, _plen in final_meta:
        while len(out) < off:
            out.append(0)
        assert len(out) == off, (tag, hex(len(out)), hex(off))
        out += payloads[tag]

    assert_rtbl_pointers(bytes(out), room_count)
    return bytes(out)


def jms_hash(name: str) -> int:
    h = 0
    for ch in name.encode("ascii"):
        h *= 3
        h += ch
        h &= 0xFFFF
    return h


def pack_rarc(files: list[tuple[str, bytes]]) -> bytes:
    """Minimal 3-node RARC: root / dat / dzs (same layout as R_SP300)."""
    dat_files = [(n, p) for n, p in files if not n.endswith(".dzs")]
    dzs_files = [(n, p) for n, p in files if n.endswith(".dzs")]

    # String table
    strings = bytearray(b"\x00")  # offset 0 = empty / root quirks
    str_off: dict[str, int] = {}

    def add_str(s: str) -> int:
        if s in str_off:
            return str_off[s]
        off = len(strings)
        strings.extend(s.encode("ascii") + b"\x00")
        str_off[s] = off
        return off

    add_str(".")
    add_str("..")
    add_str("dat")
    add_str("dzs")
    for n, _ in dat_files + dzs_files:
        add_str(n)

    # Directory nodes: ROOT, dat, dzs
    # Node: type[4], name_off u32, hash u16, file_count u16, first_file_index u32
    # File entries interleaved: dirs first then files per nintendo style of R_SP300:
    #   root: dat, dzs, ., ..
    #   dat:  <files>, ., ..
    #   dzs:  <files>, ., ..

    file_entries: list[tuple[int, int, int, int, int, bytes | None]] = []
    # (id, hash, flags_name, data_off_placeholder, size, payload|None)

    def add_dir_ref(name: str, node_index: int) -> None:
        # directory entry: flags 0x02, data_off = node index
        name_off = add_str(name)
        type_name = (0x02 << 24) | (name_off & 0xFFFFFF)
        file_entries.append((0xFFFF, jms_hash(name), type_name, node_index, 0x10, None))

    def add_file(name: str, payload: bytes) -> None:
        name_off = add_str(name)
        type_name = (0x11 << 24) | (name_off & 0xFFFFFF)
        file_entries.append((len([e for e in file_entries if e[5] is not None]), jms_hash(name), type_name, 0, len(payload), payload))

    # We'll assign first_file indices as we build
    # Root node files: dat, dzs, ., ..
    root_first = 0
    add_dir_ref("dat", 1)
    add_dir_ref("dzs", 2)
    add_dir_ref(".", 0)
    add_dir_ref("..", 0xFFFFFFFF)
    root_count = 4

    dat_first = len(file_entries)
    for n, p in dat_files:
        add_file(n, p)
    add_dir_ref(".", 1)
    add_dir_ref("..", 0)
    dat_count = 2 + len(dat_files)

    dzs_first = len(file_entries)
    for n, p in dzs_files:
        add_file(n, p)
    add_dir_ref(".", 2)
    add_dir_ref("..", 0)
    dzs_count = 2 + len(dzs_files)

    nodes = [
        (b"ROOT", add_str(""), root_count, root_first),
        (b"dat\x00", add_str("dat"), dat_count, dat_first),
        (b"dzs\x00", add_str("dzs"), dzs_count, dzs_first),
    ]
    # Fix ROOT name — use "ROOT" typed node name field (4 chars) + string ""
    nodes = [
        (b"ROOT", 0, root_count, root_first),
        (b"dat ", add_str("dat"), dat_count, dat_first),
        (b"dzs ", add_str("dzs"), dzs_count, dzs_first),
    ]

    # Align string table
    while len(strings) % 4:
        strings.append(0)

    # Layout: RARC hdr(0x20) + info(0x20) + nodes(0x10*3) + files(0x14*n) + strings + file data
    num_nodes = 3
    num_files = len(file_entries)
    info_size = 0x20
    node_off = info_size
    file_off = node_off + num_nodes * 0x10
    str_off_val = file_off + num_files * 0x14
    # header_len = 0x20 typically means info starts at 0x20
    header_len = 0x20
    str_len = len(strings)
    data_off_from_header = str_off_val + str_len
    while data_off_from_header % 32:
        data_off_from_header += 1
        # pad will be inserted in strings area or after
    # pad after strings
    str_pad = data_off_from_header - (str_off_val + str_len)

    # Build file data blob with 32-align
    data_blob = bytearray()
    file_data_meta: list[tuple[int, int]] = []  # off, size per file entry index
    for i, (fid, h, tn, doff, sz, payload) in enumerate(file_entries):
        if payload is None:
            file_data_meta.append((doff, sz))  # dir: keep node index in doff
            continue
        while len(data_blob) % 32:
            data_blob.append(0)
        off = len(data_blob)
        data_blob.extend(payload)
        file_data_meta.append((off, len(payload)))

    while len(data_blob) % 32:
        data_blob.append(0)

    # Assemble
    out = bytearray(b"RARC")
    # placeholders for sizes
    out += be32(0)  # file size
    out += be32(header_len)
    out += be32(data_off_from_header)  # data offset from header start (=0x20)
    out += be32(len(data_blob))
    out += be32(0)
    out += be32(0)
    out += be32(0)

    # info at 0x20
    assert len(out) == 0x20
    out += be32(num_nodes)
    out += be32(node_off)  # relative to info → 0x20 means nodes at 0x40
    out += be32(num_files)
    out += be32(file_off)
    out += be32(str_len)
    out += be32(str_off_val)
    out += be32(0x100)  # sync? unknown; R_SP300 has this-ish
    out += be32(0)

    # Fix: node_off/file_off/str_off are relative to info (0x20)
    # We wrote node_off = 0x20 which would put nodes at 0x40 — good if info is 0x20 bytes.
    # Rewrite info properly:
    out = bytearray(out[:0x20])
    node_off_rel = 0x20
    file_off_rel = node_off_rel + num_nodes * 0x10
    str_off_rel = file_off_rel + num_files * 0x14
    data_off_rel = str_off_rel + str_len + str_pad
    while data_off_rel % 32:
        data_off_rel += 1
        str_pad = data_off_rel - (str_off_rel + str_len)

    out += be32(num_nodes)
    out += be32(node_off_rel)
    out += be32(num_files)
    out += be32(file_off_rel)
    out += be32(str_len)
    out += be32(str_off_rel)
    out += be32(16)  # file data alignment hint used by some tools
    out += be32(0)

    # nodes
    assert len(out) == 0x20 + node_off_rel
    for typ4, name_off, count, first in [
        (b"ROOT", 0, root_count, root_first),
        (b"dat ", add_str("dat"), dat_count, dat_first),
        (b"dzs ", add_str("dzs"), dzs_count, dzs_first),
    ]:
        out += typ4[:4]
        out += be32(name_off)
        out += struct.pack(">HH", jms_hash(typ4[:3].decode("ascii", "replace").strip() or "ROOT"), count)
        # hash for ROOT/dat/dzs — use name string hash
        # overwrite hash with proper
        pass

    # Remake nodes cleanly
    out = out[: 0x20 + node_off_rel]
    node_defs = [
        (b"ROOT", 0, "ROOT", root_count, root_first),
        (b"dat ", add_str("dat"), "dat", dat_count, dat_first),
        (b"dzs ", add_str("dzs"), "dzs", dzs_count, dzs_first),
    ]
    for typ4, name_off, name, count, first in node_defs:
        out += typ4[:4]
        out += be32(name_off)
        out += struct.pack(">HH", jms_hash(name), count)
        out += be32(first)

    assert len(out) == 0x20 + file_off_rel
    # files
    for i, (fid, h, tn, doff, sz, payload) in enumerate(file_entries):
        meta_off, meta_sz = file_data_meta[i]
        if payload is None:
            out += struct.pack(">HHIII", 0xFFFF, h, tn, meta_off, 0x10)
            out += be32(0)
        else:
            out += struct.pack(">HHIII", fid, h, tn, meta_off, meta_sz)
            out += be32(0)

    assert len(out) == 0x20 + str_off_rel
    out += strings
    out += b"\x00" * str_pad
    while len(out) < 0x20 + data_off_rel:
        out.append(0)
    assert len(out) == 0x20 + data_off_rel
    out += data_blob

    # Patch header sizes
    struct.pack_into(">I", out, 4, len(out))
    struct.pack_into(">I", out, 8, header_len)
    struct.pack_into(">I", out, 12, data_off_rel)
    struct.pack_into(">I", out, 16, len(data_blob))
    return bytes(out)


def main() -> int:
    if not STG_PATH.is_file():
        print("missing", STG_PATH, file=sys.stderr)
        return 1

    raw = STG_PATH.read_bytes()
    members = dict(list_rarc_files(raw))
    if "stage.dzs" not in members or "room0.dzs" not in members:
        print("STG missing stage.dzs/room0.dzs", members.keys(), file=sys.stderr)
        return 1

    room0 = members["room0.dzs"]
    r0_chunks = parse_dzs_chunks(room0)
    fili = None
    for tag, ent, payload in r0_chunks:
        if tag == "FILI":
            fili = payload
            break
    if fili is None:
        print("room0.dzs has no FILI", file=sys.stderr)
        return 1

    new_stage = rebuild_stage_dzs(members["stage.dzs"], ROOM_COUNT)
    files: list[tuple[str, bytes]] = []
    # Keep dat/* members in stable order
    for name in (
        "colordata_tbl.dat",
        "pol_arg.dat",
        "pol_effcol.dat",
        "pol_effcol2.dat",
        "pol_efftbl.dat",
        "pol_efftbl2.dat",
        "pol_sound.dat",
    ):
        if name in members:
            files.append((name, members[name]))
    for i in range(ROOM_COUNT):
        files.append((f"room{i}.dzs", build_room_dzs(fili)))
    files.append(("stage.dzs", new_stage))

    out = pack_rarc(files)

    # Verify round-trip listing
    check = dict(list_rarc_files(out))
    for i in range(ROOM_COUNT):
        key = f"room{i}.dzs"
        if key not in check:
            print("pack failed; missing", key, "have", sorted(check), file=sys.stderr)
            return 1
    st = check["stage.dzs"]
    n = struct.unpack(">I", st[:4])[0]
    tags = {}
    for i in range(n):
        o = 4 + i * 12
        tag = st[o : o + 4].decode("ascii")
        ent, off = struct.unpack(">II", st[o + 4 : o + 12])
        tags[tag] = ent
    if tags.get("RTBL") != ROOM_COUNT or tags.get("MULT") != ROOM_COUNT:
        print("stage.dzs RTBL/MULT mismatch", tags, file=sys.stderr)
        return 1

    # Verify RTBL pointers before write (also runs inside rebuild; belt+suspenders).
    assert_rtbl_pointers(st, ROOM_COUNT)
    rtbl_off = None
    for i in range(n):
        o = 4 + i * 12
        if st[o : o + 4] == b"RTBL":
            rtbl_off = struct.unpack(">I", st[o + 8 : o + 12])[0]
            break
    assert rtbl_off is not None
    for i in range(ROOM_COUNT):
        ptr = struct.unpack(">I", st[rtbl_off + i * 4 : rtbl_off + i * 4 + 4])[0]
        print(f"  RTBL ptr[{i}]=0x{ptr:x} rec={st[ptr:ptr+8].hex()}")

    bak97 = STG_PATH.with_suffix(STG_PATH.suffix + ".pre97-bak")
    if not bak97.is_file():
        bak97.write_bytes(raw)
        print("backup", bak97)
    bak102 = STG_PATH.with_suffix(STG_PATH.suffix + ".pre102-bak")
    if not bak102.is_file():
        bak102.write_bytes(raw)
        print("backup", bak102)
    STG_PATH.write_bytes(out)
    print(
        f"wrote {STG_PATH} bytes={len(out)} RTBL={tags['RTBL']} MULT={tags['MULT']} "
        f"rooms={ROOM_COUNT} (№102 absolute RTBL ptrs)"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
