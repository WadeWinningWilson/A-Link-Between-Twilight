#!/usr/bin/env python3
"""Disassemble material sub-DLs referenced by G_DL branches."""
from __future__ import annotations

import struct
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mm_extract_object_stk import DMADATA_OFF, ROM, load_blob, object_stk_vrom, parse_dmadata

TARGETS = [
    (0xB4B2, "Torso material"),
    (0x7001, "HatBrim material"),
    (0xE2F3, "LThigh material"),
    (0xF6F0, "Torso (full)"),
    (0xFA78, "HatBrim (full)"),
    (0xFBD0, "HatRings (full)"),
    (0xFCE8, "HatNarrow (full)"),
    (0xFDF8, "HatTop (full)"),
    (0xA5C0, "NormalHead"),
    (0xA530, "NormalEyes"),
]

OP = {
    0xDE: "G_DL",
    0xDF: "ENDDL",
    0xE4: "TEXTURE",
    0xE6: "SETOTHERMODE_L",
    0xE7: "MODIFYVTX",
    0xF2: "LOADTILE",
    0xF3: "LOADBLOCK",
    0xF5: "SETTILE",
    0xFD: "SETTIMG",
    0xFE: "SETCOMBINE",
    0x07: "MOVEWORD",
    0x08: "VTX",
    0x0C: "MODIFYVTX",
    0x04: "MOVEMEM",
    0xBC: "TRI1",
    0xB1: "TRI1",
}

FMT = {0: "RGBA", 1: "YUV", 2: "CI", 3: "IA", 4: "I"}
SIZ = {0: "4b", 1: "8b", 2: "16b", 3: "32b"}

rom = ROM.read_bytes()
blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))

for start, label in TARGETS:
    print(f"\n===== {label} @ {start:#x} =====")
    off = start
    for _ in range(60):
        if off + 8 > len(blob):
            break
        w0, w1 = struct.unpack_from(">II", blob, off)
        op = (w0 >> 24) & 0xFF
        name = OP.get(op, f"0x{op:02X}")
        extra = ""
        if op == 0xFD:
            fmt = FMT.get((w0 >> 21) & 7, "?")
            siz = SIZ.get((w0 >> 19) & 3, "?")
            width = (w0 & 0xFFF) + 1
            extra = f" fmt={fmt} siz={siz} w={width} img={w1:#x}"
        elif op == 0xF5:
            extra = f" tile={w1:#x}"
        elif op == 0xF3:
            extra = f" load={w1:#x}"
        elif op == 0xDE:
            extra = f" -> {w1:#x}"
        print(f"  {off:#06x}: {name}{extra}")
        off += 8
        if op == 0xDF:
            break
