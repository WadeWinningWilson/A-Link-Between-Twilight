#!/usr/bin/env python3
"""№116: bake authored sky colours into the sky dome's TEV registers.

WHY THIS EXISTS (and why it is a stopgap, not the real fix):

`adapt_bdl_arcs.normalize_tevregs` promotes each material's 50%-gray TEV
placeholder to white. For characters and terrain that is correct - the donor
runtime overwrites those registers every frame with live light colours, our
mount does not, and gray would render everything at half brightness.

The sky dome is the one case where that transform is wrong. For `vr_*` models
the TEV register IS the sky colour, so whitening the placeholder produces a
pure white dome - which is exactly what shipped.

The engine-correct fix is to drive these registers per frame from
`g_env_light.vrbox_*_col` the way `daVrbox_color_set` does for the native sky
model (see `d_a_vrbox.cpp`). That is engine work and it is what enables
time-of-day. Until it lands, this bakes the authored CLEAR-DAY values straight
into the files so the dome reads correctly instead of white.

Colours are Nintendo's own, read out of the donor chain rather than eyeballed:
room 44 -> palette set 0 (clear) -> band 2 (noon) -> its Virt record.

Static by construction: the sky will not shift with time of day until the
per-frame driver exists. Declared, not silent.

RETIRED 2026-08-07 — DO NOT RUN AS PART OF THE PIPELINE.
  §418's runtime colour feed supersedes this. Proven by test, not argument:
  arcs/WwSky.arc reverted to its un-baked .prewhite-bak state and the dome
  rendered CORRECTLY. The dome renders through TevKColor, which the feed
  drives per frame from g_env_light; this step wrote TevColor, which never
  reached it once the feed existed. Kept for the record and re-runnable if
  the feed is ever gated off.

Usage:
  bake_wwsky_colors.py            (no arguments)

  Inputs   : <mod>/arcs/WwSky.arc — must already exist AND must already have been
             through adapt_bdl_arcs; this step edits its TEV registers in place.
  Outputs  : <mod>/arcs/WwSky.arc — rewritten IN PLACE, backup written to
             WwSky.arc.prewhite-bak before the first change.
  Idempotent: yes — it re-reads and rewrites the same registers. The backup is
             taken every run, so a second run overwrites the first backup with
             already-baked bytes; the pre-bake state survives only the first run.
  Order    : AFTER adapt_bdl_arcs.py. This step exists to UNDO that step's
             normalize_tevregs for vr_* materials: for the sky dome the TEV
             register IS the sky colour, so whitening the 50%-gray placeholder
             produced the white dome that shipped.
             ANTI-EDGE — adapt_bdl_arcs.py must NEVER run after this one. It
             would re-whiten the dome, and nothing errors; the sky just goes
             white again. Same shape as the shell/grow pair: "A before B" cannot
             express "and never A again", and --from-step is where it bites.
"""
from __future__ import annotations

import os
import struct
import sys
from pathlib import Path

from adapt_bdl_arcs import be32, yaz0_dec

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
ARC = MOD / "arcs" / "WwSky.arc"

# Filled from the donor chain by convert_lighting's tables (room 44, set 0,
# band 2 = clear noon). Each dome model takes the band it actually represents.
SKY_COLOURS = {
    "vr_sky.bdl": (80, 120, 255),          # sky_col
    "vr_uso_umi.bdl": (80, 120, 255),      # horizon sea band
    "vr_kasumi_mae.bdl": (163, 210, 255),  # near haze
    "vr_back_cloud.bdl": (255, 255, 255),  # clouds are authored white
}


def patch_mat3(buf: bytearray, rgb: tuple[int, int, int]) -> int:
    """Set TEV register 0 (the colour the dome actually renders with)."""
    off = 0x20
    for _ in range(be32(buf, 12)):
        tag = bytes(buf[off : off + 4])
        ssize = be32(buf, off + 4)
        if tag == b"MAT3":
            offs = [struct.unpack_from(">i", buf, off + 0x0C + i * 4)[0] for i in range(30)]
            t = offs[17]
            if t <= 0:
                return 0
            nxt = min((x for x in offs if x > t), default=ssize)
            entries = list(range(off + t, off + nxt - 7, 8))
            if not entries:
                return 0
            # Register 0 only. Register 1 is the authored (0,0,0,0) black used
            # as the subtractive term - overwriting it flattens the gradient.
            r, g, b, a = struct.unpack_from(">hhhh", buf, entries[0])
            struct.pack_into(">hhhh", buf, entries[0], rgb[0], rgb[1], rgb[2], a)
            return 1
        off += ssize
    return 0


def main() -> int:
    if not ARC.is_file():
        raise SystemExit(f"missing {ARC}")
    raw = bytearray(ARC.read_bytes())
    if bytes(raw[:4]) == b"Yaz0":
        raise SystemExit("arc is compressed; expected the adapted (plain) form")

    data_abs = 0x20 + be32(raw, 0x0C)
    info = 0x20
    n = be32(raw, info + 8)
    ent = info + be32(raw, info + 0x0C)
    strs = info + be32(raw, info + 0x14)

    patched = 0
    for i in range(n):
        e = ent + i * 0x14
        if struct.unpack_from(">H", raw, e + 4)[0] & 0x1100 != 0x1100:
            continue
        no = struct.unpack_from(">H", raw, e + 6)[0]
        name = bytes(raw[strs + no : raw.index(b"\0", strs + no)]).decode("ascii", "replace")
        if name not in SKY_COLOURS:
            continue
        off, size = be32(raw, e + 8), be32(raw, e + 12)
        member = bytearray(raw[data_abs + off : data_abs + off + size])
        rgb = SKY_COLOURS[name]
        if patch_mat3(member, rgb):
            raw[data_abs + off : data_abs + off + size] = member
            patched += 1
            print(f"  {name:22} TEV reg0 -> {rgb}")

    if patched:
        bak = ARC.with_suffix(ARC.suffix + ".prewhite-bak")
        if not bak.is_file():
            bak.write_bytes(ARC.read_bytes())
            print(f"  backup -> {bak.name}")
        ARC.write_bytes(bytes(raw))
    print(f"{patched}/{len(SKY_COLOURS)} sky models coloured -> {ARC}")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
