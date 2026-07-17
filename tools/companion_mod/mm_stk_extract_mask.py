#!/usr/bin/env python3
# ============================================================================
# Extract MM Skull Kid's Majora's Mask geometry (head-local space) to JSON
# ============================================================================
# The mask is drawn in DmStk_PostLimbDraw in the HEAD limb's matrix with NO
# offset (Context A / mask-worn-on-face), so its vertices are authored in
# head-local space. This walks gSkullKidMajorasMask{Eyes,1,2}DL with pygfxd,
# collects per-triangle verts (pos + uv) tagged by their bound texture, and
# writes _work/object_stk/gen_c/mask_geo.json for Blender to build a mesh
# that we then hang off the head bone (identity), exactly like the game.
# ============================================================================
from __future__ import annotations

import json
import struct
from pathlib import Path

from pygfxd import (
    GfxdEndian, gfxd_endian, gfxd_execute, gfxd_f3dex2, gfxd_input_buffer,
    gfxd_macro_fn, gfxd_output_buffer, gfxd_target, gfxd_tri1_callback,
    gfxd_tri2_callback, gfxd_vtx_callback, gfxd_timg_callback, gfxd_macro_dflt,
)

from mm_stk_gen_object_c import ROM, XML, Symbols, extract_object_stk, dl_slice

MASK_DLS = ["gSkullKidMajorasMaskEyesDL", "gSkullKidMajorasMask1DL",
            "gSkullKidMajorasMask2DL"]
OUT = Path(r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin"
           r"\_work\object_stk\gen_c\mask_geo.json")
SCALE = 1.0  # keep raw N64 units; Blender side scales to match the import


class MaskWalker:
    def __init__(self, blob, syms):
        self.blob = blob
        self.syms = syms
        self.vbuf = [None] * 64          # loaded vertex slots (pos, uv)
        self.cur_tex = None
        self.tris = []                   # (i0,i1,i2, tex) as resolved verts
        self.verts = []                  # flat [(x,y,z,u,v,tex)]

    def vtx_cb(self, seg, num, start):
        addr = seg & 0xFFFFFF
        for i in range(num):
            o = addr + i * 16
            x, y, z = struct.unpack_from(">hhh", self.blob, o)
            u, v = struct.unpack_from(">hh", self.blob, o + 8)
            self.vbuf[start + i] = (x, y, z, u / 32.0, v / 32.0)
        return 1

    def timg_cb(self, timg, fmt, siz, width, height, pal):
        name = self.syms.texture_for_addr(timg & 0xFFFFFF)
        self.cur_tex = name or self.cur_tex
        return 1

    def _emit(self, a, b, c):
        for idx in (a, b, c):
            vt = self.vbuf[idx]
            if vt is None:
                return
        base = len(self.verts)
        for idx in (a, b, c):
            x, y, z, u, v = self.vbuf[idx]
            self.verts.append((x, y, z, u, v, self.cur_tex))
        self.tris.append((base, base + 1, base + 2))

    def tri1_cb(self, a, b, c):
        self._emit(a, b, c)
        return 1

    def tri2_cb(self, a, b, c, d, e, f):
        self._emit(a, b, c)
        self._emit(d, e, f)
        return 1


def main() -> int:
    rom = ROM.read_bytes()
    blob, vrom = extract_object_stk(rom)
    syms = Symbols(XML)

    walker = MaskWalker(blob, syms)
    gfxd_target(gfxd_f3dex2)
    gfxd_endian(GfxdEndian.big, 4)
    gfxd_vtx_callback(walker.vtx_cb)
    gfxd_timg_callback(walker.timg_cb)
    gfxd_tri1_callback(walker.tri1_cb)
    gfxd_tri2_callback(walker.tri2_cb)
    gfxd_macro_fn(gfxd_macro_dflt)

    dl_offsets = {n: off for off, n in syms.dlists.items()}
    for name in MASK_DLS:
        off = dl_offsets[name]
        data = dl_slice(blob, off)
        gfxd_input_buffer(data)
        gfxd_output_buffer(bytes(len(data) * 64))
        if gfxd_execute() != 0:
            raise SystemExit(f"gfxd failed on {name}")

    OUT.write_text(json.dumps({
        "verts": walker.verts, "tris": walker.tris,
        "textures": sorted({v[5] for v in walker.verts if v[5]}),
    }))
    print(f"mask: {len(walker.verts)} verts, {len(walker.tris)} tris")
    print("textures:", sorted({v[5] for v in walker.verts if v[5]}))
    xs = [v[0] for v in walker.verts]; ys = [v[1] for v in walker.verts]; zs = [v[2] for v in walker.verts]
    print(f"head-local bbox: x[{min(xs)},{max(xs)}] y[{min(ys)},{max(ys)}] z[{min(zs)},{max(zs)}]")
    print("wrote", OUT)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
