#!/usr/bin/env python3
"""Validate dmadata candidates for object_stk using known asset layout from object_stk.xml."""
import struct
from pathlib import Path

from mm_extract_object_stk import (
    DMADATA_OFF,
    ROM,
    load_object_stk_blob,
    parse_dmadata,
    rgba16_to_rgba,
)

# Known layout anchors from object_stk.xml (US decomp)
SKIN_OFF = 0x123D8
EYE_OFF = 0x1A5A0
SKEL_OFF = 0x13328
WALK_ANIM_OFF = 0x1D3D0


def score_blob(blob: bytes) -> dict:
    out = {"len": len(blob)}
    if len(blob) <= SKEL_OFF + 8:
        return out
    # Flex skeleton: first u8 often limb-related; gSkullKidSkel has 22 limbs (0x16)
    skel = blob[SKEL_OFF : SKEL_OFF + 16]
    out["skel_head"] = skel.hex()
    out["skel_limb_byte0"] = skel[0]
    # Walk anim at end: should exist within file
    out["tail_room"] = len(blob) - WALK_ANIM_OFF
    # Decode eye 8x8 — valid MM eye has limited palette, not random noise
    if len(blob) >= EYE_OFF + 128:
        eye_raw = blob[EYE_OFF : EYE_OFF + 128]
        out["eye_unique"] = len(set(eye_raw))
        rgba = rgba16_to_rgba(eye_raw, 8, 8)
        # count non-transparent pixels
        opaque = sum(1 for i in range(0, len(rgba), 4) if rgba[i + 3] > 0)
        out["eye_opaque_px"] = opaque
    if len(blob) >= SKIN_OFF + 512:
        skin = blob[SKIN_OFF : SKIN_OFF + 512]
        out["skin_unique"] = len(set(skin))
    return out


def main() -> None:
    rom = ROM.read_bytes()
    entries = parse_dmadata(rom, DMADATA_OFF)
    candidates = []
    for e in entries:
        sz = e["v1"] - e["v0"]
        if sz < 0x1C000 or sz > 0x21000:
            continue
        try:
            blob = load_object_stk_blob(rom, entries, e["v0"])
        except Exception:
            continue
        s = score_blob(blob)
        s["vrom"] = e["v0"]
        s["size"] = sz
        candidates.append(s)

    # object_stk should have ~22 limbs in skeleton, tail room for walk anim, eye not noise
    print(f"{'vrom':>12} {'size':>8} {'limb0':>5} {'eye_u':>5} {'eye_op':>6} {'skin_u':>6} {'tail':>5}")
    for s in sorted(candidates, key=lambda x: x["vrom"]):
        print(
            f"{s['vrom']:#12x} {s['size']:#10x} "
            f"{s.get('skel_limb_byte0', -1):5} "
            f"{s.get('eye_unique', 0):5} "
            f"{s.get('eye_opaque_px', 0):6} "
            f"{s.get('skin_unique', 0):6} "
            f"{s.get('tail_room', 0):5}"
        )


if __name__ == "__main__":
    main()
