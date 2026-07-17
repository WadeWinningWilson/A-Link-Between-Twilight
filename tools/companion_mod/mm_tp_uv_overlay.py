#!/usr/bin/env python3
# ============================================================================
# TP Skull Kid (E_PM) atlas UV overlays, color-coded by body region
# ============================================================================
# Input:  _work/tp_uv_groups.json  (exported from Blender: per-face UVs +
#         dominant bone group, meshes mesh-0/1/2)
#         textures_vanilla/pm_tex01.png, pm_leaf01.png, pm_eye.1.png
# Output: _work/region_map/pm_tex01_regions.png (etc.) at 4x, with legend.
#
# Purpose: step-2 region mapping for the MM reskin -- shows which part of
# each TP atlas is worn by which body part, to pair against MM textures.
# ============================================================================
from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw

WORK = Path(r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work")
TEX_DIR = Path(r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\textures_vanilla")
OUT = WORK / "region_map"
SCALE = 4

# bone -> region
REGIONS = {
    "head": "head",
    "hat1": "hat", "hat2": "hat", "hat3": "hat", "hat4": "hat",
    "backbone": "torso", "waist": "torso", "center": "torso",
    "skirt": "skirt",
    "necklace1": "necklace", "necklace2": "necklace",
    "cloak1": "cloak", "cloak2": "cloak",
    "shoulderL": "arms", "armL1": "arms", "armL2": "arms",
    "shoulderR": "arms", "armR1": "arms", "armR2": "arms",
    "handL": "hands", "handR": "hands",
    "legL1": "legs", "legL2": "legs", "legR1": "legs", "legR2": "legs",
    "footL": "feet", "footR": "feet",
}
COLORS = {
    "head":     (255, 60, 60),
    "hat":      (255, 160, 0),
    "torso":    (255, 240, 0),
    "skirt":    (0, 220, 90),
    "necklace": (0, 230, 230),
    "arms":     (60, 120, 255),
    "hands":    (255, 0, 255),
    "legs":     (170, 90, 255),
    "feet":     (255, 130, 180),
    "cloak":    (0, 150, 130),
    "none":     (128, 128, 128),
}

MESH_TEX = {
    "mesh-0": "pm_tex01.png",
    "mesh-1": "pm_leaf01.png",
    "mesh-2": "pm_eye.1.png",
}


def render(mesh_name: str, faces: list[dict]) -> None:
    tex = Image.open(TEX_DIR / MESH_TEX[mesh_name]).convert("RGBA")
    w, h = tex.width * SCALE, tex.height * SCALE
    base = tex.resize((w, h), Image.NEAREST)

    fill = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    dfill = ImageDraw.Draw(fill)
    outline = Image.new("RGBA", (w, h), (0, 0, 0, 0))
    dout = ImageDraw.Draw(outline)

    used_regions = set()
    for f in faces:
        region = REGIONS.get(f["g"], "none")
        used_regions.add(region)
        c = COLORS[region]
        pts = [(u * w, (1.0 - v) * h) for u, v in f["uv"]]
        dfill.polygon(pts, fill=c + (70,))
        dout.polygon(pts, outline=c + (255,))

    img = Image.alpha_composite(base, fill)
    img = Image.alpha_composite(img, outline)

    # legend strip
    legend_h = 22 * len(used_regions) + 10
    canvas = Image.new("RGBA", (w + 170, max(h, legend_h)), (30, 30, 30, 255))
    canvas.paste(img, (0, 0))
    dleg = ImageDraw.Draw(canvas)
    y = 8
    for region in sorted(used_regions):
        dleg.rectangle([w + 12, y, w + 30, y + 14], fill=COLORS[region] + (255,))
        dleg.text((w + 38, y + 2), region, fill=(255, 255, 255, 255))
        y += 22

    out_name = MESH_TEX[mesh_name].replace(".png", "").replace(".1", "") + "_regions.png"
    canvas.save(OUT / out_name)
    print(f"{mesh_name} ({MESH_TEX[mesh_name]}): {len(faces)} faces, "
          f"regions: {sorted(used_regions)} -> {out_name}")


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    data = json.loads((WORK / "tp_uv_groups.json").read_text())
    for mesh_name, faces in data.items():
        render(mesh_name, faces)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
