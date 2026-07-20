#!/usr/bin/env python3
"""№107: build mod-side STG_00.arc for F_DL01 (room 44) and F_DL02 (room 0).

History authored R44_00.arc / R00_00.arc. This writes the stage shells so native
room streaming can request those rooms.

Sparse RTBL is legitimate (TP R_SP01: MULT rooms 0,1,2,4,5,7 with RTBL n=8).
F_DL01 needs RTBL n=45 (index by room id) + MULT n=1 (room 44) + room44.dzs.
F_DL02 needs RTBL/MULT n=1 + room0.dzs.

Writes AppData mod-side only (never commit WW arcs).
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

# Reuse №102-safe RARC / RTBL helpers.
import grow_rdl01_stg as g

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
TEMPLATE = MOD / "files" / "res" / "Stage" / "R_DL01" / "STG_00.arc"


def build_rtbl_sparse(rtbl_count: int, rtbl_abs_off: int, active: set[int]) -> bytes:
    """RTBL indexed by room id; only `active` rooms set ChkBg (0x80)."""
    ptr_array_size = 4 * rtbl_count
    data_base = rtbl_abs_off + ptr_array_size
    rooms_base = data_base + 8 * rtbl_count

    ptrs = bytearray()
    datas = bytearray()
    room_bytes = bytearray()
    for i in range(rtbl_count):
        data_off = data_base + i * 8
        rooms_off = rooms_base + i
        ptrs += g.be32(data_off)
        datas += bytes([1, 0, 0, 0]) + g.be32(rooms_off)
        if i in active:
            room_bytes.append(0xC0 | (i & 0x3F))
        else:
            # Present slot, no BG/room-scene create.
            room_bytes.append(i & 0x3F)
    return bytes(ptrs) + bytes(datas) + bytes(room_bytes)


def build_mult_sparse(rooms: list[int]) -> bytes:
    mult = bytearray()
    for room in rooms:
        mult += struct.pack(">ffhBB", 0.0, 0.0, 0, room & 0xFF, 0)
    return bytes(mult)


def rebuild_stage_dzs_sparse(old_stage: bytes, rooms: list[int]) -> bytes:
    active = set(rooms)
    rtbl_count = max(rooms) + 1
    mult_count = len(rooms)
    chunks = g.parse_dzs_chunks(old_stage)

    ordered_meta: list[tuple[str, int, int]] = []
    for tag, ent, payload in chunks:
        if tag == "RTBL":
            ordered_meta.append((tag, rtbl_count, g.rtbl_payload_size(rtbl_count)))
        elif tag == "MULT":
            ordered_meta.append((tag, mult_count, 12 * mult_count))
        else:
            ordered_meta.append((tag, ent, len(payload)))

    data_start = 4 + len(ordered_meta) * 12
    cursor = data_start
    final_meta: list[tuple[str, int, int, int]] = []
    for tag, ent, plen in ordered_meta:
        while cursor % 4:
            cursor += 1
        final_meta.append((tag, ent, cursor, plen))
        cursor += plen

    other_payload = {tag: payload for tag, _ent, payload in chunks}
    payloads: dict[str, bytes] = {}
    for tag, ent, off, plen in final_meta:
        if tag == "RTBL":
            payloads[tag] = build_rtbl_sparse(rtbl_count, off, active)
            assert len(payloads[tag]) == plen
        elif tag == "MULT":
            payloads[tag] = build_mult_sparse(rooms)
            assert len(payloads[tag]) == plen
        else:
            payloads[tag] = other_payload[tag]

    out = bytearray()
    out += g.be32(len(final_meta))
    for tag, ent, off, _plen in final_meta:
        out += tag.encode("ascii") + g.be32(ent) + g.be32(off)
    assert len(out) == data_start
    for tag, ent, off, _plen in final_meta:
        while len(out) < off:
            out.append(0)
        assert len(out) == off, (tag, hex(len(out)), hex(off))
        out += payloads[tag]

    g.assert_rtbl_pointers(bytes(out), rtbl_count)
    return bytes(out)


def build_host_stg(stage_name: str, rooms: list[int]) -> Path:
    if not TEMPLATE.is_file():
        raise SystemExit(f"missing template STG: {TEMPLATE}")
    raw = TEMPLATE.read_bytes()
    members = dict(g.list_rarc_files(raw))
    if "stage.dzs" not in members or "room0.dzs" not in members:
        raise SystemExit(f"template missing stage/room0.dzs: {sorted(members)}")

    fili = None
    for tag, _ent, payload in g.parse_dzs_chunks(members["room0.dzs"]):
        if tag == "FILI":
            fili = payload
            break
    if fili is None:
        raise SystemExit("template room0.dzs has no FILI")

    new_stage = rebuild_stage_dzs_sparse(members["stage.dzs"], rooms)
    files: list[tuple[str, bytes]] = []
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
    for room in sorted(set(rooms)):
        files.append((f"room{room}.dzs", g.build_room_dzs(fili)))
    files.append(("stage.dzs", new_stage))

    out_bytes = g.pack_rarc(files)
    check = dict(g.list_rarc_files(out_bytes))
    for room in rooms:
        key = f"room{room}.dzs"
        if key not in check:
            raise SystemExit(f"pack missing {key}; have {sorted(check)}")

    st = check["stage.dzs"]
    n = struct.unpack(">I", st[:4])[0]
    tags: dict[str, int] = {}
    for i in range(n):
        o = 4 + i * 12
        tag = st[o : o + 4].decode("ascii")
        ent, _off = struct.unpack(">II", st[o + 4 : o + 12])
        tags[tag] = ent
    rtbl_n = max(rooms) + 1
    if tags.get("RTBL") != rtbl_n or tags.get("MULT") != len(rooms):
        raise SystemExit(f"RTBL/MULT mismatch want RTBL={rtbl_n} MULT={len(rooms)} got {tags}")

    out_dir = MOD / "files" / "res" / "Stage" / stage_name
    out_dir.mkdir(parents=True, exist_ok=True)
    out_path = out_dir / "STG_00.arc"
    if out_path.is_file():
        bak = out_path.with_suffix(out_path.suffix + ".pre107-bak")
        if not bak.is_file():
            bak.write_bytes(out_path.read_bytes())
            print("backup", bak)
    out_path.write_bytes(out_bytes)
    print(
        f"wrote {out_path} bytes={len(out_bytes)} RTBL={tags['RTBL']} "
        f"MULT={tags['MULT']} rooms={rooms}"
    )
    return out_path


def main() -> int:
    # №107: Outset exterior = sea sector room 44; A_mori forest = room 0.
    build_host_stg("F_DL01", [44])
    build_host_stg("F_DL02", [0])
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
