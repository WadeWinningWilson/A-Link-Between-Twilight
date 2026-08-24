#!/usr/bin/env python3
"""List G_DL branch targets from Skull Kid skeleton limb DLs."""
from __future__ import annotations

import struct
import sys
import xml.etree.ElementTree as ET
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mm_extract_object_stk import DMADATA_OFF, ROM, load_blob, object_stk_vrom, parse_dmadata

XML = Path(
    r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk\object_stk.xml"
)
G_DL = 0xDE

LIMB_DLS = [
    ("TORSO", "gSkullKidTorsoDL", 0xF6F0),
    ("PELVIS", "gSkullKidPelvisDL", 0x10C60),
    ("HAT_BRIM", "gSkullKidHatBrimDL", 0xFA78),
    ("HAT_RINGS", "gSkullKidHatRingsDL", 0xFBD0),
    ("HAT_NARROW", "gSkullKidHatNarrowSectionDL", 0xFCE8),
    ("HAT_TOP", "gSkullKidHatTopDL", 0xFDF8),
    ("R_UARM", "gSkullKidRightUpperArmDL", 0xFF30),
    ("R_FARM", "gSkullKidRightForearmDL", 0x10230),
    ("R_HAND", "gSkullKidRightHandDL", 0x10428),
    ("L_UARM", "gSkullKidLeftUpperArmDL", 0x10560),
    ("L_FARM", "gSkullKidLeftForearmDL", 0x10868),
    ("L_HAND", "gSkullKidLeftHandAndFluteDL", 0x10A50),
    ("L_THIGH", "gSkullKidLeftThighDL", 0x10E30),
    ("L_SHIN", "gSkullKidLeftShinDL", 0x11080),
    ("L_FOOT", "gSkullKidLeftFootDL", 0x111A8),
    ("R_THIGH", "gSkullKidRightThighDL", 0x112F0),
    ("R_SHIN", "gSkullKidRightShinDL", 0x11560),
    ("R_FOOT", "gSkullKidRightFootDL", 0x11688),
]

root = ET.parse(XML).getroot()
dls = {el.get("Name"): int(el.get("Offset"), 16) for el in root.iter("DList")}
textures = {int(el.get("Offset"), 16): el.get("OutName") for el in root.iter("Texture")}
inv_dl = {v: k for k, v in dls.items()}

rom = ROM.read_bytes()
blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))

# next DL end heuristic
sorted_dl_offs = sorted(dls.values())


def dl_end(start: int) -> int:
    for o in sorted_dl_offs:
        if o > start:
            return o
    return 0x1D530


def gdl_targets(start: int, end: int) -> list[tuple[int, int]]:
    out = []
    off = start
    while off + 8 <= end:
        w0, w1 = struct.unpack_from(">II", blob, off)
        op = (w0 >> 24) & 0xFF
        if op == G_DL:
            out.append((off, w1))
        elif op == 0xDF:
            break
        off += 8
    return out


print("Limb DL -> G_DL branches (w1 -> resolved if possible)")
for limb, name, start in LIMB_DLS:
    end = dl_end(start)
    branches = gdl_targets(start, end)
    print(f"\n{limb} {name} ({start:#x})")
    for pos, w1 in branches:
        hits = []
        for cand in (w1, w1 & 0xFFFFFF, w1 & 0xFFFF, (w1 & 0xFFFFFF) - 0x6000000):
            if cand in inv_dl:
                hits.append(f"DL {inv_dl[cand]}")
            if cand in textures:
                hits.append(f"TEX {textures[cand]}")
            if 0 < cand < len(blob) and cand not in inv_dl and cand not in textures:
                hits.append(f"off@{cand:#x}")
        label = ", ".join(hits) if hits else f"unresolved {w1:#x}"
        print(f"  {pos:#x} -> {label}")
