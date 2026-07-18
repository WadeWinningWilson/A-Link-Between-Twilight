#!/usr/bin/env python3
# ============================================================================
# Extract MM Skull Kid's Majora's Mask geometry (head-local space) to JSON
# ============================================================================
# The mask is drawn in DmStk_PostLimbDraw in the HEAD limb's matrix with NO
# offset (Context A / mask-worn-on-face), so its verts are in head-local
# space. Parses the already-generated ZAPD-style C (object_stk_gen.c): reads
# the Vtx arrays, walks gSkullKidMajorasMask{Eyes,1,2}DL (gsSPVertex +
# gsSP1Triangle/gsSP2Triangles + gsDPLoadTextureBlock), and writes
# mask_geo.json for Blender to build a mesh hung off the head bone (identity),
# exactly as the game draws it.
# ============================================================================
from __future__ import annotations

import json
import re
from pathlib import Path

from mm_stk_gen_object_c import XML, Symbols

import sys

GEN = Path(r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin"
           r"\_work\object_stk\gen_c\object_stk_gen.c")
# DL sets: mask (worn), and the head DL (reference geometry already imported
# into Blender, used to derive the head-local -> Blender transform).
DL_SETS = {
    "mask": (["gSkullKidMajorasMaskEyesDL", "gSkullKidMajorasMask1DL",
              "gSkullKidMajorasMask2DL"], GEN.parent / "mask_geo.json"),
    "head": (["gSkullKidNormalHeadDL"], GEN.parent / "head_geo.json"),
}
SET = sys.argv[1] if len(sys.argv) > 1 else "mask"
MASK_DLS, OUT = DL_SETS[SET]

VTX_RE = re.compile(
    r"\{\{\s*\{\s*(-?\d+),\s*(-?\d+),\s*(-?\d+)\s*\},\s*-?\d+,"
    r"\s*\{\s*(-?\d+),\s*(-?\d+)\s*\}")


def parse_vtx_arrays(src: str) -> dict[str, list[tuple]]:
    out = {}
    for m in re.finditer(r"Vtx (gSkullKidVtx_[0-9A-Fa-f]+)\[\] = \{(.*?)\n\};",
                         src, re.DOTALL):
        name, body = m.group(1), m.group(2)
        out[name] = [(int(a), int(b), int(c), int(s), int(t))
                     for a, b, c, s, t in VTX_RE.findall(body)]
    return out


def dl_body(src: str, name: str) -> str:
    m = re.search(r"Gfx " + re.escape(name) + r"\[\] = \{(.*?)\n\};", src, re.DOTALL)
    if not m:
        raise SystemExit(f"DL {name} not found")
    return m.group(1)


def main() -> int:
    src = GEN.read_text(encoding="utf-8")
    vtx = parse_vtx_arrays(src)
    syms = Symbols(XML)
    texdim = {t["name"]: (t["w"], t["h"]) for t in syms.textures.values()}

    verts, tris = [], []
    for dl in MASK_DLS:
        slots = [None] * 64
        cur_tex = None
        for line in dl_body(src, dl).splitlines():
            line = line.strip()
            m = re.match(r"gsSPVertex\((?:&)?(gSkullKidVtx_[0-9A-Fa-f]+)"
                         r"(?:\[(\d+)\])?,\s*(\d+),\s*(\d+)\)", line)
            if m:
                arr, off, num, start = m.group(1), int(m.group(2) or 0), \
                    int(m.group(3)), int(m.group(4))
                for i in range(num):
                    slots[start + i] = vtx[arr][off + i]
                continue
            m = re.match(r"gsDPLoadTextureBlock\((gSkullKid\w+Tex)", line)
            if m:
                cur_tex = m.group(1).replace("gSkullKid", "").replace("Tex", "")
                cur_tex = re.sub(r"(?<!^)(?=[A-Z])", "_", cur_tex).lower()
                cur_tex = "gSkullKid" + m.group(1)[9:]  # keep symbol form
                cur_tex = m.group(1)
                continue
            m = re.match(r"gsSP1Triangle\((\d+),\s*(\d+),\s*(\d+)", line)
            if m:
                idxs = [int(x) for x in m.groups()]
                emit(verts, tris, slots, idxs, cur_tex, texdim)
                continue
            m = re.match(r"gsSP2Triangles\((\d+),\s*(\d+),\s*(\d+),\s*\d+,"
                         r"\s*(\d+),\s*(\d+),\s*(\d+)", line)
            if m:
                g = [int(x) for x in m.groups()]
                emit(verts, tris, slots, g[0:3], cur_tex, texdim)
                emit(verts, tris, slots, g[3:6], cur_tex, texdim)

    OUT.write_text(json.dumps({"verts": verts, "tris": tris}))
    xs = [v[0] for v in verts]; ys = [v[1] for v in verts]; zs = [v[2] for v in verts]
    print(f"mask: {len(verts)} verts, {len(tris)} tris")
    print("textures:", sorted({v[5] for v in verts if v[5]}))
    print(f"head-local bbox: x[{min(xs)},{max(xs)}] y[{min(ys)},{max(ys)}] z[{min(zs)},{max(zs)}]")
    print("wrote", OUT)
    return 0


def emit(verts, tris, slots, idxs, tex, texdim):
    if any(slots[i] is None for i in idxs):
        return
    w, h = texdim.get(tex, (32, 32))
    base = len(verts)
    for i in idxs:
        x, y, z, s, t = slots[i]
        verts.append((x, y, z, (s / 32.0) / w, 1.0 - (t / 32.0) / h, tex))
    tris.append((base, base + 1, base + 2))


if __name__ == "__main__":
    raise SystemExit(main())
