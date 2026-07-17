#!/usr/bin/env python3
"""Map object_stk texture offsets to display lists via segment-6 Gfx pointers."""
from __future__ import annotations

import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mm_extract_object_stk import DMADATA_OFF, ROM, load_blob, object_stk_vrom, parse_dmadata

XML = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk\object_stk.xml"
)

# F3DEX2 Gfx opcodes (byte 0 of w0, big-endian)
OP = {
    0x00: "SPNOOP",
    0x01: "MTX",
    0x02: "RESERVED0",
    0x03: "POPMTX",
    0x04: "MOVEMEM",
    0x05: "MOVEWORD",
    0x06: "MTXPROJECTION",
    0x07: "MOVEMEM",
    0x08: "VTX",
    0x09: "RESERVED1",
    0x0A: "VTX",
    0x0B: "VTX",
    0x0C: "MODIFYVTX",
    0x0D: "RESERVED2",
    0x0E: "RESERVED3",
    0x0F: "RESERVED4",
    0x10: "RESERVED5",
    0x11: "RESERVED6",
    0x12: "RESERVED7",
    0x13: "RESERVED8",
    0x14: "RESERVED9",
    0x15: "RESERVED10",
    0x16: "RESERVED11",
    0x17: "RESERVED12",
    0x18: "RESERVED13",
    0x19: "RESERVED14",
    0x1A: "RESERVED15",
    0x1B: "RESERVED16",
    0x1C: "RESERVED17",
    0x1D: "RESERVED18",
    0x1E: "RESERVED19",
    0x1F: "RESERVED20",
    0xDF: "ENDDL",
    0xE2: "SETGEOMETRYMODE",
    0xE3: "CLEARGEOMETRYMODE",
    0xE4: "TEXTURE",
    0xE5: "SETOTHERMODE_H",
    0xE6: "SETOTHERMODE_L",
    0xE7: "MODIFYVTX",
    0xE8: "RDPHALF_1",
    0xF2: "LOADTILE",
    0xF3: "LOADBLOCK",
    0xF5: "SETTILE",
    0xF6: "FILLRECT",
    0xF7: "FILLRECT",
    0xFB: "SETSCISSOR",
    0xFC: "SETPRIMDEPTH",
    0xFD: "SETTIMG",
    0xFE: "SETCOMBINE",
    0xFF: "SETENVCOLOR",
}


def load_assets():
    root = ET.parse(XML).getroot()
    dls = sorted(
        [(el.get("Name"), int(el.get("Offset"), 16)) for el in root.iter("DList")],
        key=lambda x: x[1],
    )
    textures = {
        int(el.get("Offset"), 16): (el.get("OutName"), el.get("Name"), el.get("Format"))
        for el in root.iter("Texture")
    }
    dl_starts = [o for _, o in dls]
    all_offs = sorted(set(dl_starts) | set(textures) | {0x1D530})
    ends = {}
    for i, (name, start) in enumerate(dls):
        # Next DL start, or file end for last DL
        ends[start] = dls[i + 1][1] if i + 1 < len(dls) else 0x1D530
    return dls, textures, ends


def dl_for_pos(dls, ends, pos):
    for name, start in reversed(dls):
        if start <= pos < ends[start]:
            return name
    return None


def scan_seg6_pointers(blob: bytes, textures: dict[int, tuple]):
    hits: dict[int, list[int]] = {t: [] for t in textures}
    for i in range(0, len(blob) - 3, 4):
        w = struct.unpack_from(">I", blob, i)[0]
        if (w & 0xFF000000) == 0x06000000:
            off = w & 0x00FFFFFF
            if off in hits:
                hits[off].append(i)
    return hits


def disasm_dl(blob: bytes, start: int, end: int) -> list[dict]:
    cmds = []
    off = start
    while off + 8 <= end and off + 8 <= len(blob):
        w0, w1 = struct.unpack_from(">II", blob, off)
        op = (w0 >> 24) & 0xFF
        entry = {"off": off, "op": op, "name": OP.get(op, f"0x{op:02X}"), "w0": w0, "w1": w1}
        if op == 0xFD:
            entry["timg"] = w1
            entry["tex_off"] = w1 & 0x00FFFFFF if (w1 & 0xFF000000) == 0x06000000 else None
        cmds.append(entry)
        off += 8
        if op == 0xDF:
            break
    return cmds


def main() -> None:
    dls, textures, ends = load_assets()
    rom = ROM.read_bytes()
    blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))

    hits = scan_seg6_pointers(blob, textures)
    print("=== Segment 6 texture pointers by OutName ===")
    for toff in sorted(textures):
        positions = hits[toff]
        if not positions:
            continue
        out_name, sym, fmt = textures[toff]
        dl_names = []
        seen = set()
        for pos in positions:
            dl = dl_for_pos(dls, ends, pos)
            if dl and dl not in seen:
                seen.add(dl)
                dl_names.append(dl)
        print(f"{out_name} ({sym}, {fmt}, {toff:#x}):")
        for dl in dl_names:
            print(f"  -> {dl}")
        print()

    # Full disasm for limb DLs with SETTIMG extraction
    limb_dls = [
        n
        for n, _ in dls
        if any(
            k in n
            for k in (
                "Torso",
                "Hat",
                "Head",
                "Arm",
                "Hand",
                "Foot",
                "Thigh",
                "Shin",
                "Pelvis",
                "Flute",
                "Ocarina",
                "MajorasMask",
                "Eyes",
                "Neck",
            )
        )
        and "LinkMask" not in n
        and "Unused" not in n
    ]

    print("=== SETTIMG (0xFD) per display list ===")
    dl_map = {n: s for n, s in dls}
    for name in limb_dls:
        start = dl_map[name]
        end = ends[start]
        cmds = disasm_dl(blob, start, end)
        timgs = [c for c in cmds if c["op"] == 0xFD]
        if not timgs:
            continue
        print(f"{name} ({start:#x}):")
        for c in timgs:
            tex = textures.get(c.get("tex_off", -1), ("?", "?", "?"))[0] if c.get("tex_off") else "?"
            print(f"  {c['off']:#x}: SETTIMG {c['w1']:#010x} -> {tex}")
        print()


if __name__ == "__main__":
    main()
