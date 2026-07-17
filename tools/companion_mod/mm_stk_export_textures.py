#!/usr/bin/env python3
# ============================================================================
# MM object_stk texture export (correct blob) -> PNGs for region mapping
# ============================================================================
# Decodes all XML-named textures from the fingerprint-validated object_stk
# blob (see mm_stk_gen_object_c.py -- the old n64/ and textures_n64/ dumps
# came from the wrong dmadata file + broken Yaz0 and are garbage).
#
# Output: _work/object_stk/gen_c/textures_png/<gName>.png (+ @4x nearest)
# ============================================================================
from __future__ import annotations

import struct
import sys
from pathlib import Path

from PIL import Image

sys.path.insert(0, str(Path(__file__).resolve().parent))
from mm_stk_gen_object_c import ROM, WORK, XML, Symbols, extract_object_stk


def decode_rgba16(data: bytes, w: int, h: int) -> Image.Image:
    img = Image.new("RGBA", (w, h))
    px = img.load()
    for i in range(w * h):
        v = struct.unpack_from(">H", data, i * 2)[0]
        r = ((v >> 11) & 0x1F) * 255 // 31
        g = ((v >> 6) & 0x1F) * 255 // 31
        b = ((v >> 1) & 0x1F) * 255 // 31
        a = 255 if v & 1 else 0
        px[i % w, i // w] = (r, g, b, a)
    return img


def decode_ia16(data: bytes, w: int, h: int) -> Image.Image:
    img = Image.new("RGBA", (w, h))
    px = img.load()
    for i in range(w * h):
        lum, a = data[i * 2], data[i * 2 + 1]
        px[i % w, i // w] = (lum, lum, lum, a)
    return img


def main() -> int:
    rom = ROM.read_bytes()
    blob, _ = extract_object_stk(rom)
    syms = Symbols(XML)

    out_dir = WORK / "gen_c" / "textures_png"
    out_dir.mkdir(parents=True, exist_ok=True)

    for off, t in sorted(syms.textures.items()):
        data = blob[off : off + t["size"]]
        if t["size"] == t["w"] * t["h"] * 2 and t["name"] != "gSkullKidFluteTex":
            # rgba16 unless the XML said ia16 (only the flute is ia16)
            img = decode_rgba16(data, t["w"], t["h"])
        else:
            img = decode_ia16(data, t["w"], t["h"])
        img.save(out_dir / f"{t['name']}.png")
        big = img.resize((t["w"] * 4, t["h"] * 4), Image.NEAREST)
        big.save(out_dir / f"{t['name']}@4x.png")
        print(f"{t['name']:44s} {t['w']}x{t['h']}")

    print(f"\nwrote {len(syms.textures)} textures to {out_dir}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
