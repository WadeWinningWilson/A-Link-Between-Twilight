#!/usr/bin/env python3
# ============================================================================
# tex_color_census.py — §390: identify textures BY THEIR PIXELS, not their names.
#
# WHY: names have already misled this campaign (I labelled the plants "flowers"
# off `Ohana_high1_v`, and the user corrected me: the plants are light tan bark
# with green leaves). A material's identity should be established from its
# decoded image content, so "which material is the plant" stops being a naming
# argument and becomes a measurement.
#
# Decodes every texture in a model's TEX1 (CMPR + RGB5A3 + I4/I8/IA4/IA8 as
# needed), then reports per texture:
#   * mean RGB and the dominant hue family (green / tan-brown / grey / other)
#   * %green pixels and %tan pixels -> a FOLIAGE score
#   * alpha content: opaque vs transparent pixel counts (cutout foliage has
#     both; a fully-opaque texture cannot be a cutout leaf card)
# CMPR decode mirrors the backend's own path (aurora texture_convert.cpp:418):
#   big-endian endpoints; when color1 <= color2, index 3 is ALPHA 0 punch-through.
#
# Usage: tex_color_census.py <arc> <model member>
# Read-only. numpy only.
# ============================================================================
import struct
import sys
from pathlib import Path

import numpy as np

sys.path.insert(0, str(Path(__file__).resolve().parents[1] /
                      "ww_crew_restoration_skeleton"))
from jstudio_stb import yaz0_dec, rarc_members

be16 = lambda d, o: struct.unpack_from(">H", d, o)[0]
be32 = lambda d, o: struct.unpack_from(">I", d, o)[0]


def get_member(arc, want):
    raw = Path(arc).read_bytes()
    if raw[:4] == b"Yaz0":
        raw = yaz0_dec(raw)
    for nm, blob in rarc_members(raw):
        if nm.lower() == want.lower():
            return yaz0_dec(blob) if blob[:4] == b"Yaz0" else blob
    return None


def find_block(bdl, tag):
    n = be32(bdl, 12)
    off = 0x20
    for _ in range(n):
        if bdl[off:off + 4] == tag:
            return off, be32(bdl, off + 4)
        off += be32(bdl, off + 4)
    return None, None


def expand5(v):
    return (v << 3) | (v >> 2)


def expand6(v):
    return (v << 2) | (v >> 4)


def decode_cmpr(data, w, h):
    """RGBA8 array (h,w,4). Mirrors aurora BuildRGBA8FromCMPR."""
    out = np.zeros((h, w, 4), dtype=np.uint8)
    src = 0
    for yy in range(0, h, 8):
        for xx in range(0, w, 8):
            for yb in range(0, 8, 4):
                for xb in range(0, 8, 4):
                    if src + 8 > len(data):
                        return out
                    c1, c2 = be16(data, src), be16(data, src + 2)
                    src += 4
                    tbl = np.zeros((4, 4), dtype=np.int32)
                    for i, c in enumerate((c1, c2)):
                        tbl[i] = (expand5((c >> 11) & 0x1F),
                                  expand6((c >> 5) & 0x3F),
                                  expand5(c & 0x1F), 255)
                    if c1 > c2:
                        tbl[2] = [(2 * tbl[0][k] + tbl[1][k]) // 3 for k in range(3)] + [255]
                        tbl[3] = [(tbl[0][k] + 2 * tbl[1][k]) // 3 for k in range(3)] + [255]
                    else:
                        tbl[2] = [(tbl[0][k] + tbl[1][k]) // 2 for k in range(3)] + [255]
                        tbl[3] = [(tbl[0][k] + tbl[1][k]) // 2 for k in range(3)] + [0]
                    for y in range(4):
                        bits = data[src + y]
                        for x in range(4):
                            px, py = xx + xb + x, yy + yb + y
                            if px < w and py < h:
                                out[py, px] = tbl[(bits >> (6 - 2 * x)) & 3]
                    src += 4
    return out


def decode_rgb5a3(data, w, h):
    out = np.zeros((h, w, 4), dtype=np.uint8)
    i = 0
    for yy in range(0, h, 4):
        for xx in range(0, w, 4):
            for y in range(4):
                for x in range(4):
                    if i + 2 > len(data):
                        return out
                    v = be16(data, i)
                    i += 2
                    px, py = xx + x, yy + y
                    if px >= w or py >= h:
                        continue
                    if v & 0x8000:
                        out[py, px] = (expand5((v >> 10) & 0x1F),
                                       expand5((v >> 5) & 0x1F),
                                       expand5(v & 0x1F), 255)
                    else:
                        a = ((v >> 12) & 7) * 255 // 7
                        out[py, px] = (((v >> 8) & 0xF) * 17,
                                       ((v >> 4) & 0xF) * 17,
                                       (v & 0xF) * 17, a)
    return out


def classify(img):
    """Colour/alpha profile of an RGBA image + a foliage score."""
    rgb = img[:, :, :3].astype(np.int32)
    a = img[:, :, 3]
    vis = a >= 128
    n_vis = int(vis.sum())
    if n_vis == 0:
        return None
    r, g, b = rgb[:, :, 0][vis], rgb[:, :, 1][vis], rgb[:, :, 2][vis]
    green = (g > r + 12) & (g > b + 12)
    tan = (r > g) & (g > b) & (r > 90) & ((r - b) > 25)
    grey = (abs(r - g) < 14) & (abs(g - b) < 14)
    return {
        "mean": (int(r.mean()), int(g.mean()), int(b.mean())),
        "pct_green": 100.0 * green.mean(),
        "pct_tan": 100.0 * tan.mean(),
        "pct_grey": 100.0 * grey.mean(),
        "opaque": n_vis,
        "transparent": int((~vis).sum()),
    }


def main():
    if len(sys.argv) < 3:
        sys.exit("usage: tex_color_census.py <arc> <model.bdl>")
    bdl = get_member(sys.argv[1], sys.argv[2])
    if bdl is None:
        sys.exit("member not found")
    off, size = find_block(bdl, b"TEX1")
    cnt = be16(bdl, off + 8)
    hdr = be32(bdl, off + 0x0C)
    nameoff = be32(bdl, off + 0x10)
    base = off + nameoff
    num = be16(bdl, base)
    names, p = [], base + 4
    for _ in range(num):
        so = be16(bdl, p + 2)
        s = base + so
        names.append(bdl[s:bdl.index(b"\0", s)].decode("ascii", "replace"))
        p += 4

    FMT = {0: "I4", 1: "I8", 2: "IA4", 3: "IA8", 4: "RGB565", 5: "RGB5A3",
           6: "RGBA8", 14: "CMPR"}
    print(f"{sys.argv[2]}: {cnt} textures\n")
    print(f"{'idx':>3} {'texture':26s} {'fmt':7s} {'WxH':>9} {'meanRGB':>14} "
          f"{'%grn':>5} {'%tan':>5} {'opaq/transp':>13}  read")
    rows = []
    for i in range(cnt):
        h0 = off + hdr + i * 0x20
        fmt = bdl[h0]
        w, ht = be16(bdl, h0 + 2), be16(bdl, h0 + 4)
        dataOff = be32(bdl, h0 + 0x1C)
        data = bdl[h0 + dataOff:]
        img = None
        if fmt == 14:
            img = decode_cmpr(data, w, ht)
        elif fmt == 5:
            img = decode_rgb5a3(data, w, ht)
        if img is None:
            print(f"{i:3d} {names[i]:26s} {FMT.get(fmt, '?'):7s} {w:4d}x{ht:<4d} "
                  f"{'(decoder n/a)':>14}")
            continue
        c = classify(img)
        if c is None:
            print(f"{i:3d} {names[i]:26s} {FMT.get(fmt,'?'):7s} {w:4d}x{ht:<4d} "
                  f"{'fully transparent':>14}")
            continue
        foliage = c["pct_green"] >= 12 and c["transparent"] > 0
        barky = c["pct_tan"] >= 25
        read = ("PLANT-LIKE (green + cutout)" if foliage and barky else
                "foliage-ish (green + cutout)" if foliage else
                "bark/wood-ish (tan, opaque)" if barky and not c["transparent"] else
                "")
        print(f"{i:3d} {names[i]:26s} {FMT.get(fmt,'?'):7s} {w:4d}x{ht:<4d} "
              f"{str(c['mean']):>14} {c['pct_green']:5.1f} {c['pct_tan']:5.1f} "
              f"{c['opaque']:6d}/{c['transparent']:<6d}  {read}")
        rows.append((i, names[i], c, read))
    cand = [r for r in rows if "PLANT" in r[3] or "foliage" in r[3]]
    print("\nPLANT CANDIDATES BY PIXELS (user: light tan bark + green leaves):")
    for i, nm, c, read in cand:
        print(f"  tex[{i}] {nm}: mean {c['mean']}, green {c['pct_green']:.1f}%, "
              f"tan {c['pct_tan']:.1f}%, {c['transparent']} transparent px")
    if not cand:
        print("  none — no texture in this model has green+cutout content")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
