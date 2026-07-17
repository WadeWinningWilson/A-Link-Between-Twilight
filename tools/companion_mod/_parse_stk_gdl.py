#!/usr/bin/env python3
"""Follow G_DL (0xDE) branches in object_stk limb DLs and collect SETTIMG targets."""
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

G_ENDDL = 0xDF
G_DL = 0xDE
G_SETTIMG = 0xFD
G_RDPHALF_1 = 0xE4  # also used for texture in some chains
G_RDPHALF_2 = 0xF3  # LOADBLOCK


def load_assets():
    root = ET.parse(XML).getroot()
    dls = {el.get("Name"): int(el.get("Offset"), 16) for el in root.iter("DList")}
    textures = {
        int(el.get("Offset"), 16): el.get("OutName")
        for el in root.iter("Texture")
    }
    return dls, textures


def read_gfx(blob: bytes, off: int) -> tuple[int, int, int]:
    w0, w1 = struct.unpack_from(">II", blob, off)
    return w0, w1, (w0 >> 24) & 0xFF


def resolve_addr(w1: int, textures: dict[int, str]) -> str:
    # segment 6
    if (w1 & 0xFF000000) == 0x06000000:
        off = w1 & 0xFFFFFF
        return textures.get(off, f"off_{off:#x}")
    # bare file offset (pre-relocation)
    if w1 in textures:
        return textures[w1]
    low = w1 & 0xFFFFFF
    if low in textures:
        return textures[low]
    return f"raw_{w1:#010x}"


def walk_dl(blob: bytes, start: int, textures: dict[int, str], seen: set[int], depth: int = 0) -> list[str]:
    texs: list[str] = []
    off = start
    while off + 8 <= len(blob) and off not in seen:
        seen.add(off)
        w0, w1, op = read_gfx(blob, off)
        if op == G_SETTIMG:
            texs.append(resolve_addr(w1, textures))
        elif op == G_DL:
            # branch target often in w1; also try w0 low bits
            for cand in (w1, w1 & 0xFFFFFF, w0 & 0xFFFF):
                if 0 < cand < len(blob):
                    texs.extend(walk_dl(blob, cand, textures, seen, depth + 1))
                    break
        elif op == G_ENDDL:
            break
        off += 8
    return texs


def main() -> None:
    dls, textures = load_assets()
    rom = ROM.read_bytes()
    blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))

    targets = [
        "gSkullKidTorsoDL",
        "gSkullKidPelvisDL",
        "gSkullKidHatBrimDL",
        "gSkullKidHatRingsDL",
        "gSkullKidHatNarrowSectionDL",
        "gSkullKidHatTopDL",
        "gSkullKidNormalHeadDL",
        "gSkullKidNormalEyesDL",
        "gSkullKidLaughingHeadDL",
        "gSkullKidLaughingEyesDL",
        "gSkullKidRightUpperArmDL",
        "gSkullKidRightForearmDL",
        "gSkullKidRightHandDL",
        "gSkullKidLeftUpperArmDL",
        "gSkullKidLeftForearmDL",
        "gSkullKidLeftHandAndFluteDL",
        "gSkullKidLeftThighDL",
        "gSkullKidLeftShinDL",
        "gSkullKidLeftFootDL",
        "gSkullKidRightThighDL",
        "gSkullKidRightShinDL",
        "gSkullKidRightFootDL",
        "gSkullKidOpenLeftHandDL",
        "gSkullKidFluteHoldingLeftHandDL",
        "gSkullKidFluteDL",
        "gSkullKidOcarinaOfTimeDL",
        "gSkullKidMajorasMask1DL",
        "gSkullKidMajorasMask2DL",
        "gSkullKidMajorasMaskEyesDL",
        "gSkullKidLinkMask1DL",
        "gSkullKidLinkMask2DL",
        "gSkullKidLinkMask3DL",
    ]

    print("=== G_DL walk texture hits ===")
    for name in targets:
        if name not in dls:
            continue
        start = dls[name]
        seen: set[int] = set()
        texs = walk_dl(blob, start, textures, seen)
        # dedupe preserve order
        uniq = []
        for t in texs:
            if t not in uniq:
                uniq.append(t)
        print(f"{name} @ {start:#x}: {uniq if uniq else ['(none found)']}")


def disasm_materials() -> None:
    rom = ROM.read_bytes()
    blob = load_blob(rom, parse_dmadata(rom, DMADATA_OFF), *object_stk_vrom(rom))
    _, textures = load_assets()
    inv = {v: k for k, v in textures.items()}
    G_DL = 0xDE
    G_SETTIMG = 0xFD
    G_ENDDL = 0xDF
    FMT = {0: "RGBA", 1: "YUV", 2: "CI", 3: "IA", 4: "I"}

    def name_for(w1: int) -> str:
        for key in (w1, w1 & 0xFFFFFF):
            if key in textures:
                return textures[key]
        if (w1 & 0xFF000000) == 0x06000000:
            off = w1 & 0xFFFFFF
            return textures.get(off, f"off_{off:#x}")
        return f"raw_{w1:#x}"

    def scan(start: int, label: str, max_cmds: int = 50, depth: int = 0) -> None:
        print(f"{'  '*depth}--- {label} @ {start:#x} ---")
        off = start
        n = 0
        while off + 8 <= len(blob) and n < max_cmds:
            w0, w1 = struct.unpack_from(">II", blob, off)
            op = (w0 >> 24) & 0xFF
            if op == G_SETTIMG:
                fmt = FMT.get((w0 >> 21) & 7, "?")
                siz = (w0 >> 19) & 3
                width = (w0 & 0xFFF) + 1
                print(
                    f"{'  '*depth}  {off:#x} SETTIMG {fmt} siz={siz} w={width} "
                    f"addr={w1:#x} -> {name_for(w1)}"
                )
            elif op == G_DL and depth < 2:
                print(f"{'  '*depth}  {off:#x} G_DL -> {w1:#x}")
                for cand in (w1, w1 & 0xFFFFFF):
                    if 0 < cand < len(blob) and cand != start:
                        scan(cand, f"branch@{off:#x}", 20, depth + 1)
                        break
            elif op == G_ENDDL:
                print(f"{'  '*depth}  {off:#x} ENDDL")
                break
            off += 8
            n += 1

    targets = [
        (0xF6F0, "Torso"),
        (0xFA78, "HatBrim"),
        (0xFBD0, "HatRings"),
        (0xFCE8, "HatNarrow"),
        (0xFDF8, "HatTop"),
        (0x10C60, "Pelvis"),
        (0xFF30, "RUpperArm"),
        (0x10560, "LUpperArm"),
        (0x10E30, "LThigh"),
        (0x112F0, "RThigh"),
        (0x111A8, "LFoot"),
        (0xA5C0, "NormalHead"),
        (0xA530, "NormalEyes"),
        (0x6BB0, "Mask1"),
        (0x79F0, "Mask2"),
        (0x5870, "MaskEyes"),
        (0x16620, "Flute"),
        (0xCAD0, "Ocarina"),
        (0xB4B2, "Torso branch target"),
        (0x7001, "HatBrim branch target"),
    ]
    print("=== Material SETTIMG scan ===")
    for start, label in targets:
        scan(start, label)
        print()


if __name__ == "__main__":
    import sys as _sys

    if len(_sys.argv) > 1 and _sys.argv[1] == "--materials":
        disasm_materials()
    else:
        main()
