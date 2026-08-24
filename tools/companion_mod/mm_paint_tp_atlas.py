#!/usr/bin/env python3
# ============================================================================
# MM Skull Kid reskin -- programmatic v1 paint of TP E_PM atlases
# ============================================================================
# Composites MM art (Nerrel HD sources, downscaled) into the TP atlas UV
# islands, region by region, using the per-face UV/bone data exported from
# Blender (tp_uv_groups.json). This is the iterate-fast first pass: uniform
# tiling per region, no hand art. Preview by relinking the TP materials in
# MM_SkullKid_v1.blend to the output PNGs.
#
# Face variants (user request):
#   A: wood + Nerrel beak composited at the nose/mouth zone
#   B: wood + TP's vanilla mouth kept (carved grin on wooden face)
#
# Output: _work/paint_v1/pm_tex01_mm_A.png / _B.png, pm_leaf01_mm.png,
#         pm_eye_mm.png   (vanilla files are never touched)
# ============================================================================
from __future__ import annotations

import json
from pathlib import Path

from PIL import Image, ImageDraw, ImageOps

WORK = Path(r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work")
NERREL = WORK / "mm_refs_hd" / "_nerrel_skull_kid"
TEX_DIR = Path(r"<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\textures_vanilla")
# vanilla dump from the original BMD (has the blink flipbook frames .2/.3)
VERIFY_TEX_DIR = Path(
    r"%USERPROFILE%\AppData\Roaming\TwilitRealm\Dusklight\bmd_export\superbmd_verify")
OUT = WORK / "paint_v1"

SS = 4  # supersample factor for mask rasterization

# Nerrel hash -> texture identity (visually verified vs correct N64 decode)
NERREL_HASH = {
    "shawl": "479DC968",
    "skin": "FFE3907D",
    "head_skin": "E1E354AD",
    "beak": "8BFE216A",
    "pants": "FDF0F922",
    "fringe": "95CBFCFB",
    "shoe": "3A9973DD",
    "eye": "B8ACC11C",
    "hand": "B60C3FDB",
    "ring": "FD378C4F",
}

# bone -> region (matches mm_tp_uv_overlay.py)
REGIONS = {
    "head": "head",
    "hat1": "hat", "hat2": "hat", "hat3": "hat", "hat4": "hat",
    "backbone": "torso", "waist": "torso", "center": "torso", "skirt": "torso",
    "necklace1": "necklace", "necklace2": "necklace",
    "shoulderL": "arms", "armL1": "arms", "armL2": "arms",
    "shoulderR": "arms", "armR1": "arms", "armR2": "arms",
    "handL": "hands", "handR": "hands",
    "legL1": "legs_upper", "legR1": "legs_upper",
    "legL2": "legs_lower", "legR2": "legs_lower",
    "footL": "feet", "footR": "feet",
}

# region -> (source key, tile width in atlas px, rotate90)
# v3 (user feedback): arms = orange fringe the whole way (no skin
# transition); legs split by bone -- green pants thigh, wood shin down to
# the shoes; torso handled specially (flat ring-ground orange + one ring
# band mid-torso, "like a belt") instead of tiling rings everywhere.
# v12: arms fully orange fringe (user preference over the reference-faithful
# wood forearm — v11's arms_upper/arms_lower split was reverted); cuff +
# wrap still painted at the wrist end by arm_wrist_paint.
PAINT = {
    "arms":       ("fringe", 28, False),
    "legs_upper": ("pants", 32, False),
    "legs_lower": ("skin", 28, False),
    "feet":       ("shoe", 40, False),
}

# Edge-fringe regions: solid body color inside, fringed TIPS only along the
# island edges (v5 user note: brim shows tips at rim only; necklace shows
# shawl tips at edge, purer green elsewhere).  region -> (source, edge px,
# tip tile width)
EDGE_FRINGE = {
    "brim": ("fringe", 4, 24),
}

# Necklace: LIGHT green strand pattern all over (v14 user note -- the dark
# green radial center of the shawl was landing mid-island). Crop only the
# bright strand band of the Nerrel shawl and flatten over ITS OWN average.
NECK_SHAWL_CROP = (150, 390, 360, 500)

# User hand-painted strokes (extracted by diff vs the generated atlas) are
# re-applied LAST on variant A every run, so manual work survives
# regeneration. Delete the file to drop the overrides.
USER_STROKES = WORK / "pm_tex01_user_strokes.png"

# Hands: TP's hand UVs are ONE connected island (x35-109, y96-128): four
# horizontal finger bars stacked in the center, palm/back lobes wrapping
# the left/right edges. Positional mapping from the N64 reference (green
# glove mitt, white swirl on both palm and back, tan wood fingers):
HAND_FINGER_RECT = (58, 97, 96, 122)   # finger bars only -> tan wood (v9:
                                       # glove extends to the knuckles)
HAND_SWIRL_CROP = (4, 12, 15, 25)      # swirl region inside the 16x32 tex
# v13 (matched-view comparison): thumb is BARE WOOD (fingerless glove) --
# thumb faces identified in 3D (protrude toward -Y beyond this threshold)
# and painted per-polygon from tp_hand_faces.json; the swirl is centered on
# the actual back-of-hand / palm UV zones (face normals +Z / -Z).
HAND_FACES_JSON = WORK / "tp_hand_faces.json"
THUMB_Y_MAX = -3.0
SWIRL_ZONE_FILL = 0.85  # swirl rect as fraction of the zone bbox
HAND_WRAP_CROP = (0, 26, 16, 32)       # white striped wrist wrap rows

# v10 (full MM hand reference set): each arm strip ends at the wrist in a
# jagged yellow-green cuff (the PantsAndWrists texture -- "wrists" is
# literal) followed by the white striped wrap, then the glove. Painted at
# one end of each arm island; flip per-island if it lands on the shoulder.
ARM_CUFF_W = 10
ARM_WRAP_W = 6
ARM_WRIST_AT_RIGHT = True  # wrist end of the arm strips in UV space

# Regions painted as flat ring-ground + ONE ring band at the middle
# ("like a belt"): torso and the hat cone.  region -> band tile width px
BELTS = {"torso": 48, "hat": 32}

# The head group is split by UV island: the island containing the teeth is
# the face (wood + beak); the remaining head islands are the hat BRIM,
# which reads as clothing fringe on MM.
FACE_SEED = (110, 62)  # center of TP's teeth cluster
# Face fill (matched to the N64 head): dense vertical wood strands -> the
# light body of HeadSkin tiled FACE_COLS times across the face width, one
# span vertically (no vertical repeats = no phantom shadow bands). A thin
# dark band from the texture top sits only at the hat/head junction.
FACE_COLS = 3
FACE_SHADOW_FRAC = 0.10  # top shadow band as fraction of face island height
# Beak pixels darker than this luminance are keyed out so the face wood
# shows instead of the beak rectangle's black edges.
BEAK_LUMA_KEY = 30
# v15: the beak is now real GEOMETRY (TP_Beak object grafted from the MM
# head mesh) -- the painted decal is off so the face wood lines continue
# underneath. Set True to restore the painted-beak variant.
PAINTED_BEAK = False

# Approximate TP mouth zone inside the face island, in 256-atlas px
# (v2: shrunk — v1 rects blanketed the whole visible face front)
MOUTH_RECT = (84, 60, 128, 82)
# Beak placement for variant A: nose/mouth patch only. The stored MM beak
# texture is one HALF (mirrored in-game) -- paint_body_atlas composes the
# full beak, so this rect is ~2:1 wide. Positioned on TP's actual carved
# mouth (teeth pixels cluster at x 85-135, y 49-76 in the vanilla atlas).
BEAK_RECT = (86, 51, 134, 75)
# Torso belt: ring band width in atlas px
BELT_TILE_W = 48


def nerrel_img(key: str) -> Image.Image:
    path = next(NERREL.glob(f"*{NERREL_HASH[key]}*.png"))
    return Image.open(path).convert("RGBA")


def flatten_opaque(img: Image.Image) -> Image.Image:
    """Composite over the image's own average opaque color so transparent
    fringes (shawl, fringe strips) tile without white/transparent holes."""
    px = img.load()
    r = g = b = n = 0
    for y in range(0, img.height, 4):
        for x in range(0, img.width, 4):
            pr, pg, pb, pa = px[x, y]
            if pa > 200:
                r += pr; g += pg; b += pb; n += 1
    avg = (r // max(n, 1), g // max(n, 1), b // max(n, 1), 255)
    bg = Image.new("RGBA", img.size, avg)
    return Image.alpha_composite(bg, img)


def region_masks(faces: list[dict], w: int, h: int) -> dict[str, Image.Image]:
    layers: dict[str, Image.Image] = {}
    draws: dict[str, ImageDraw.ImageDraw] = {}
    for f in faces:
        region = REGIONS.get(f["g"])
        if region is None:
            continue
        if region not in layers:
            layers[region] = Image.new("L", (w * SS, h * SS), 0)
            draws[region] = ImageDraw.Draw(layers[region])
        pts = [(u * w * SS, (1.0 - v) * h * SS) for u, v in f["uv"]]
        draws[region].polygon(pts, fill=255)
    # grow 1px (at atlas res) to avoid seams, then downsample
    out = {}
    for region, layer in layers.items():
        small = layer.resize((w, h), Image.BOX)
        grown = small.filter(__import__("PIL.ImageFilter", fromlist=["MaxFilter"]).MaxFilter(3))
        out[region] = grown
    return out


def tiled(source: Image.Image, tile_w: int, w: int, h: int, rot: bool) -> Image.Image:
    src = flatten_opaque(source)
    if rot:
        src = src.rotate(90, expand=True)
    tile_h = max(1, round(tile_w * src.height / src.width))
    tile = src.resize((tile_w, tile_h), Image.LANCZOS)
    layer = Image.new("RGBA", (w, h))
    for y in range(0, h, tile_h):
        for x in range(0, w, tile_w):
            layer.paste(tile, (x, y))
    return layer


def belt_layer(mask: Image.Image, w: int, h: int, tile_w: int) -> Image.Image:
    """Flat ring-ground orange with ONE ring band across the region middle."""
    flat = flatten_opaque(nerrel_img("ring"))
    ground = flat.getpixel((2, 2))  # red ground corner of the ring texture
    layer = Image.new("RGBA", (w, h), ground)
    bbox = mask.getbbox()
    if bbox is None:
        return layer
    tile_h = max(1, round(tile_w * flat.height / flat.width))
    belt = flat.resize((tile_w, tile_h), Image.LANCZOS)
    yc = (bbox[1] + bbox[3]) // 2 - tile_h // 2
    for x in range(0, w, tile_w):
        layer.paste(belt, (x, yc))
    return layer


def full_beak() -> Image.Image:
    """The MM beak texture is a mirrored half -- compose the full beak.
    The mirror seam is the half's LEFT edge ([flipped | original] joins the
    two halves at center). Crop the top green face-edge strip."""
    half = nerrel_img("beak")
    full = Image.new("RGBA", (half.width * 2, half.height))
    full.paste(half.transpose(Image.FLIP_LEFT_RIGHT), (0, 0))
    full.paste(half, (half.width, 0))
    # crop 30%: the green face-edge strip read as a stray "shadow" mid-face
    return full.crop((0, round(full.height * 0.30), full.width, full.height))


def erode(mask: Image.Image, n: int) -> Image.Image:
    from PIL import ImageFilter
    out = mask
    for _ in range(n):
        out = out.filter(ImageFilter.MinFilter(3))
    return out


def components(mask: Image.Image) -> list[tuple[int, Image.Image]]:
    """All 4-connected components of a binary mask as (area, mask) pairs."""
    from collections import deque

    w, h = mask.size
    px = mask.load()
    seen = [[False] * h for _ in range(w)]
    comps = []
    for sy in range(h):
        for sx in range(w):
            if not px[sx, sy] or seen[sx][sy]:
                continue
            comp = Image.new("L", mask.size, 0)
            cp = comp.load()
            q = deque([(sx, sy)])
            seen[sx][sy] = True
            area = 0
            while q:
                x, y = q.popleft()
                cp[x, y] = 255
                area += 1
                for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
                    if 0 <= nx < w and 0 <= ny < h and px[nx, ny] and not seen[nx][ny]:
                        seen[nx][ny] = True
                        q.append((nx, ny))
            comps.append((area, comp))
    return comps


def body_color(flat: Image.Image) -> tuple[int, int, int, int]:
    """Average color of the solid upper part of a fringe-style texture."""
    region = flat.crop((0, round(flat.height * 0.1), flat.width, round(flat.height * 0.45)))
    px = region.load()
    r = g = b = n = 0
    for y in range(region.height):
        for x in range(region.width):
            pr, pg, pb, _ = px[x, y]
            r += pr; g += pg; b += pb; n += 1
    return (r // n, g // n, b // n, 255)


def edge_fringe_paint(out: Image.Image, mask: Image.Image, src_key: str,
                      edge_px: int, tile_w: int) -> None:
    """Solid body color inside the region; fringe tips only along edges."""
    from PIL import ImageChops

    w, h = out.size
    flat = flatten_opaque(nerrel_img(src_key))
    out.paste(Image.new("RGBA", (w, h), body_color(flat)), (0, 0), mask)

    edge = ImageChops.subtract(mask, erode(mask, edge_px))
    tips = flat.crop((0, round(flat.height * 0.55), flat.width, flat.height))
    tile_h = max(1, round(tile_w * tips.height / tips.width))
    tips_layer = Image.new("RGBA", (w, h))
    tile = tips.resize((tile_w, tile_h), Image.LANCZOS)
    for y in range(0, h, tile_h):
        for x in range(0, w, tile_w):
            tips_layer.paste(tile, (x, y))
    out.paste(tips_layer, (0, 0), edge)


def necklace_paint(out: Image.Image, mask: Image.Image) -> None:
    """LIGHT-green strand pattern across the whole necklace (v14): the
    bright strand band of the shawl, flattened over its OWN average color
    so no dark radial-center green appears anywhere in the region."""
    w, h = out.size
    strands = nerrel_img("shawl").crop(NECK_SHAWL_CROP)

    # average of the crop's own opaque pixels = light strand green
    px = strands.load()
    r = g = b_ = n = 0
    for y in range(0, strands.height, 3):
        for x in range(0, strands.width, 3):
            pr, pg, pb, pa = px[x, y]
            if pa > 200:
                r += pr; g += pg; b_ += pb; n += 1
    light = (r // max(n, 1), g // max(n, 1), b_ // max(n, 1), 255)

    strands = Image.alpha_composite(Image.new("RGBA", strands.size, light), strands)
    for _, comp in components(mask):
        cb = comp.getbbox()
        layer = Image.new("RGBA", (w, h))
        layer.paste(strands.resize((cb[2] - cb[0], cb[3] - cb[1]), Image.LANCZOS), cb[:2])
        out.paste(layer, (0, 0), comp)


def faces_uv_mask(faces: list[dict], w: int, h: int) -> Image.Image:
    """Rasterize face UV polygons into a binary mask (supersampled)."""
    from PIL import ImageDraw as _ID
    layer = Image.new("L", (w * SS, h * SS), 0)
    d = _ID.Draw(layer)
    for f in faces:
        d.polygon([(u * w * SS, (1.0 - v) * h * SS) for u, v in f["uv"]], fill=255)
    from PIL import ImageFilter as _IF
    return layer.resize((w, h), Image.BOX).filter(_IF.MaxFilter(3))


def faces_uv_bbox(faces: list[dict], w: int, h: int) -> tuple[int, int, int, int]:
    us = [u for f in faces for u, v in f["uv"]]
    vs = [v for f in faces for u, v in f["uv"]]
    return (round(min(us) * w), round((1 - max(vs)) * h),
            round(max(us) * w), round((1 - min(vs)) * h))


def hand_paint(out: Image.Image, mask: Image.Image) -> None:
    """v13: glove base, tan finger bars, swirl CENTERED on the real
    back-of-hand and palm UV zones, thumb painted bare wood per-polygon
    (MM's glove is fingerless -- the thumb is uncovered)."""
    import json as _json

    w, h = out.size
    flat = flatten_opaque(nerrel_img("hand"))
    sx, sy = flat.width / 16, flat.height / 32

    def crop16(r):
        return flat.crop((round(r[0] * sx), round(r[1] * sy),
                          round(r[2] * sx), round(r[3] * sy)))

    glove_green = crop16((2, 16, 7, 22))       # plain glove area, no swirl
    swirl = crop16(HAND_SWIRL_CROP)
    fingers_wood = crop16((0, 0, 16, 6))       # wood strip at texture top

    # base: whole hand region in plain glove green
    px = glove_green.resize((8, 8), Image.BOX).resize((2, 2), Image.BOX)
    avg = px.getpixel((0, 0))
    out.paste(Image.new("RGBA", (w, h), avg), (0, 0), mask)

    def paste_rect(src, rect, resample=Image.LANCZOS):
        rect = (max(rect[0], 0), max(rect[1], 0), min(rect[2], w), min(rect[3], h))
        img = src.resize((rect[2] - rect[0], rect[3] - rect[1]), resample)
        out.paste(img, rect[:2], mask.crop(rect))

    paste_rect(fingers_wood, HAND_FINGER_RECT)

    faces = _json.loads(HAND_FACES_JSON.read_text())
    faces = [f for f in faces if f["side"] == "L"]  # both hands share UVs
    thumb = [f for f in faces if f["c"][1] < THUMB_Y_MAX]
    back = [f for f in faces if f["n"][2] > 0.5 and f["c"][1] >= THUMB_Y_MAX]
    palm = [f for f in faces if f["n"][2] < -0.5 and f["c"][1] >= THUMB_Y_MAX]

    # swirl centered on each zone's UV bbox
    for zone in (back, palm):
        if not zone:
            continue
        b = faces_uv_bbox(zone, w, h)
        cx, cy = (b[0] + b[2]) / 2, (b[1] + b[3]) / 2
        sw = max(8, round((b[2] - b[0]) * SWIRL_ZONE_FILL))
        sh = max(8, round((b[3] - b[1]) * SWIRL_ZONE_FILL))
        paste_rect(swirl, (round(cx - sw / 2), round(cy - sh / 2),
                           round(cx + sw / 2), round(cy + sh / 2)), Image.NEAREST)

    # thumb last: bare wood overrides glove/swirl
    thumb_mask = faces_uv_mask(thumb, w, h)
    wood_layer = tiled(fingers_wood, 30, w, h, False)
    out.paste(wood_layer, (0, 0), thumb_mask)


def arm_wrist_paint(out: Image.Image, mask: Image.Image) -> None:
    """Cuff (PantsAndWrists) + white striped wrap at each arm's wrist end."""
    hand_flat = flatten_opaque(nerrel_img("hand"))
    sx, sy = hand_flat.width / 16, hand_flat.height / 32
    wrap = hand_flat.crop((round(HAND_WRAP_CROP[0] * sx), round(HAND_WRAP_CROP[1] * sy),
                           round(HAND_WRAP_CROP[2] * sx), round(HAND_WRAP_CROP[3] * sy)))
    cuff = flatten_opaque(nerrel_img("pants"))
    for _, comp in components(mask):
        b = comp.getbbox()
        bh = b[3] - b[1]
        if ARM_WRIST_AT_RIGHT:
            wrap_rect = (b[2] - ARM_WRAP_W, b[1], b[2], b[3])
            cuff_rect = (b[2] - ARM_WRAP_W - ARM_CUFF_W, b[1], b[2] - ARM_WRAP_W, b[3])
        else:
            wrap_rect = (b[0], b[1], b[0] + ARM_WRAP_W, b[3])
            cuff_rect = (b[0] + ARM_WRAP_W, b[1], b[0] + ARM_WRAP_W + ARM_CUFF_W, b[3])
        # wrap stripes run across the wrist: rotate the horizontal strip
        wrap_img = wrap.rotate(90, expand=True).resize(
            (wrap_rect[2] - wrap_rect[0], bh), Image.LANCZOS)
        cuff_img = cuff.resize((cuff_rect[2] - cuff_rect[0], bh), Image.LANCZOS)
        out.paste(cuff_img, cuff_rect[:2], comp.crop(cuff_rect))
        out.paste(wrap_img, wrap_rect[:2], comp.crop(wrap_rect))


def component_containing(mask: Image.Image, seed: tuple[int, int]) -> Image.Image:
    """4-connected component of a binary mask containing seed (BFS)."""
    from collections import deque

    w, h = mask.size
    px = mask.load()
    out = Image.new("L", mask.size, 0)
    op = out.load()
    if not px[seed]:
        return out
    q = deque([seed])
    op[seed] = 255
    while q:
        x, y = q.popleft()
        for nx, ny in ((x - 1, y), (x + 1, y), (x, y - 1), (x, y + 1)):
            if 0 <= nx < w and 0 <= ny < h and px[nx, ny] and not op[nx, ny]:
                op[nx, ny] = 255
                q.append((nx, ny))
    return out


def paint_body_atlas(faces: list[dict]) -> tuple[Image.Image, Image.Image]:
    base = Image.open(TEX_DIR / "pm_tex01.png").convert("RGBA")
    w, h = base.size
    masks = region_masks(faces, w, h)

    out = base.copy()
    for region, (src_key, tile_w, rot) in PAINT.items():
        if region not in masks:
            continue
        layer = tiled(nerrel_img(src_key), tile_w, w, h, rot)
        out.paste(layer, (0, 0), masks[region])
    for region, tile_w in BELTS.items():
        if region in masks:
            out.paste(belt_layer(masks[region], w, h, tile_w), (0, 0), masks[region])

    # head split: face island (wood) vs brim islands (edge-fringed)
    face_mask = component_containing(masks["head"], FACE_SEED)
    brim_mask = masks["head"].copy()
    brim_mask.paste(0, (0, 0), face_mask)
    # face fill: thin hat-junction shadow band + dense vertical wood strands
    face_flat = flatten_opaque(nerrel_img("head_skin"))
    sw, sh = face_flat.size
    dark_strip = face_flat.crop((0, 0, sw, round(sh * 0.22)))
    light_body = face_flat.crop((0, round(sh * 0.28), sw, sh))
    fb = face_mask.getbbox()
    fw, fh = fb[2] - fb[0], fb[3] - fb[1]
    band_h = max(2, round(fh * FACE_SHADOW_FRAC))
    col_w = -(-fw // FACE_COLS)  # ceil division
    column = Image.new("RGBA", (col_w, fh))
    column.paste(dark_strip.resize((col_w, band_h), Image.LANCZOS), (0, 0))
    column.paste(light_body.resize((col_w, fh - band_h), Image.LANCZOS), (0, band_h))
    face_layer = Image.new("RGBA", (w, h))
    for i in range(FACE_COLS):
        face_layer.paste(column, (fb[0] + i * col_w, fb[1]))
    out.paste(face_layer, (0, 0), face_mask)
    edge_fringe_paint(out, brim_mask, *EDGE_FRINGE["brim"])
    if "necklace" in masks:
        necklace_paint(out, masks["necklace"])
    if "hands" in masks:
        hand_paint(out, masks["hands"])
    if "arms" in masks:
        arm_wrist_paint(out, masks["arms"])
    masks["head"] = face_mask  # beak/mouth composites clip to the face only

    # Variant B: restore TP's vanilla carved mouth on the wooden face
    variant_b = out.copy()
    mouth = base.crop(MOUTH_RECT)
    face_mask = masks["head"].crop(MOUTH_RECT)
    variant_b.paste(mouth, MOUTH_RECT[:2], face_mask)

    # Variant A: painted beak decal over the mouth zone -- superseded by the
    # TP_Beak geometry graft (PAINTED_BEAK False leaves continuous wood).
    variant_a = out.copy()
    if PAINTED_BEAK:
        beak = full_beak().resize(
            (BEAK_RECT[2] - BEAK_RECT[0], BEAK_RECT[3] - BEAK_RECT[1]), Image.LANCZOS
        )
        bpx = beak.load()
        luma_mask = Image.new("L", beak.size, 0)
        lpx = luma_mask.load()
        for y in range(beak.height):
            for x in range(beak.width):
                r, g, b, a = bpx[x, y]
                if a > 128 and (r + g + b) / 3 > BEAK_LUMA_KEY:
                    lpx[x, y] = 255
        face_mask_a = masks["head"].crop(BEAK_RECT)
        from PIL import ImageChops
        combined = ImageChops.multiply(luma_mask, face_mask_a)
        variant_a.paste(beak, BEAK_RECT[:2], combined)

    # re-apply the user's hand-painted strokes last (survives regeneration)
    if USER_STROKES.is_file():
        strokes = Image.open(USER_STROKES).convert("RGBA")
        variant_a = Image.alpha_composite(variant_a, strokes)

    return variant_a, variant_b


def paint_leaf(faces: list[dict]) -> Image.Image:
    """Recolor the TP leaf cape toward the MM shawl straw palette.
    v15 (final): the crown-feather faces (head bone group) KEEP the vanilla
    leaf art -- user-approved look -- via a UV mask composite."""
    base = Image.open(TEX_DIR / "pm_leaf01.png").convert("RGBA")
    gray = ImageOps.grayscale(base)
    # straw gradient: deep red-brown shadows -> orange -> straw yellow
    colored = ImageOps.colorize(gray, black=(96, 34, 12), mid=(204, 106, 22),
                                white=(233, 200, 74))
    out = colored.convert("RGBA")
    out.putalpha(base.getchannel("A"))

    crown = [f for f in faces if f["g"] == "head"]
    if crown:
        w, h = base.size
        mask = faces_uv_mask(crown, w, h)
        out.paste(base, (0, 0), mask)
    return out


def paint_eye(frame: str = "pm_eye.1.png") -> Image.Image:
    """MM eye colors under the vanilla frame's alpha (works for the blink
    flipbook frames pm_eye.2/.3 too -- the closing-lid alpha shapes are
    preserved, only the colors go MM)."""
    base = Image.open(VERIFY_TEX_DIR / frame if not (TEX_DIR / frame).is_file()
                      else TEX_DIR / frame).convert("RGBA")
    eye = nerrel_img("eye").resize(base.size, Image.LANCZOS)
    eye.putalpha(base.getchannel("A"))
    return eye


def main() -> int:
    OUT.mkdir(parents=True, exist_ok=True)
    data = json.loads((WORK / "tp_uv_groups.json").read_text())

    va, vb = paint_body_atlas(data["mesh-0"])
    va.save(OUT / "pm_tex01_mm_A.png")
    vb.save(OUT / "pm_tex01_mm_B.png")
    paint_leaf(data["mesh-1"]).save(OUT / "pm_leaf01_mm.png")
    paint_eye().save(OUT / "pm_eye_mm.png")
    paint_eye("pm_eye.2.png").save(OUT / "pm_eye2_mm.png")
    paint_eye("pm_eye.3.png").save(OUT / "pm_eye3_mm.png")

    for p in sorted(OUT.glob("*.png")):
        print("wrote", p.name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
