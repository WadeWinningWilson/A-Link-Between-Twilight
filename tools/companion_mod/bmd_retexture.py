#!/usr/bin/env python3
"""Replace TEX1 image payloads in a J3D2 BMD while keeping names/formats/layout intact."""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

try:
    from PIL import Image
except ImportError:
    raise SystemExit("pip install Pillow")

FMT_BPP = {0: 0.5, 1: 1, 2: 1, 3: 2, 4: 2, 5: 2, 6: 4, 8: 0.5, 9: 1, 10: 2, 14: 0.5}
FMT_NAME = {
    0: "I4",
    1: "I8",
    2: "IA4",
    3: "IA8",
    4: "RGB565",
    5: "RGB5A3",
    6: "RGBA32",
    8: "C4",
    9: "C8",
    10: "C14X2",
    14: "CMPR",
}


def u16(b: bytes, o: int) -> int:
    return struct.unpack_from(">H", b, o)[0]


def u32(b: bytes, o: int) -> int:
    return struct.unpack_from(">I", b, o)[0]


def sections(buf: bytes) -> tuple[dict[str, tuple[int, int]], list[str]]:
    n = u32(buf, 0x0C)
    sec: dict[str, tuple[int, int]] = {}
    order: list[str] = []
    o = 0x20
    for _ in range(n):
        tag = buf[o : o + 4].decode("ascii", "replace")
        sz = u32(buf, o + 4)
        sec[tag] = (o, sz)
        order.append(tag)
        o += sz
    return sec, order


def parse_tex1(buf: bytearray, base: int) -> list[dict]:
    n = u16(buf, base + 8)
    hdr_off = base + u32(buf, base + 0x0C)
    str_off = base + u32(buf, base + 0x10)
    sc = u16(buf, str_off)
    names: list[str] = []
    for i in range(sc):
        so = u16(buf, str_off + 4 + i * 4 + 2)
        e = buf.index(0, str_off + so)
        names.append(buf[str_off + so : e].decode("ascii"))

    textures: list[dict] = []
    for i in range(n):
        h = hdr_off + i * 0x20
        fmt = buf[h]
        w = u16(buf, h + 2)
        ht = u16(buf, h + 4)
        data_rel = u32(buf, h + 0x1C)
        data_abs = h + data_rel
        bpp = FMT_BPP[fmt]
        size = int(w * ht * bpp)
        textures.append(
            {
                "name": names[i] if i < len(names) else f"tex{i}",
                "header_off": h,
                "data_abs": data_abs,
                "size": size,
                "fmt": fmt,
                "w": w,
                "h": ht,
            }
        )
    return textures


def rgb565(r: int, g: int, b: int) -> int:
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)


def encode_rgb565(img: Image.Image) -> bytes:
    img = img.convert("RGB")
    out = bytearray()
    for y in range(img.height):
        for x in range(img.width):
            r, g, b = img.getpixel((x, y))
            v = rgb565(r, g, b)
            out += struct.pack(">H", v)
    return bytes(out)


def encode_rgba32(img: Image.Image) -> bytes:
    img = img.convert("RGBA")
    out = bytearray()
    for y in range(img.height):
        for x in range(img.width):
            out += bytes(img.getpixel((x, y)))
    return bytes(out)


def encode_rgb5a3(img: Image.Image) -> bytes:
    img = img.convert("RGBA")
    out = bytearray()
    for y in range(img.height):
        for x in range(img.width):
            r, g, b, a = img.getpixel((x, y))
            if a < 224:
                # ARGB3444
                v = ((a >> 4) << 12) | ((r >> 4) << 8) | ((g >> 4) << 4) | (b >> 4)
            else:
                v = 0x8000 | rgb565(r, g, b)
            out += struct.pack(">H", v)
    return bytes(out)


def encode_cmpr(img: Image.Image) -> bytes:
    """Minimal DXT1/CMPR encoder (4x4 blocks). Good enough for pipeline tests."""
    img = img.convert("RGBA")
    w, h = img.size
    out = bytearray()
    for by in range(0, h, 4):
        for bx in range(0, w, 4):
            pixels = []
            for dy in range(4):
                for dx in range(4):
                    x = min(bx + dx, w - 1)
                    y = min(by + dy, h - 1)
                    pixels.append(img.getpixel((x, y)))
            rs = sum(p[0] for p in pixels) // 16
            gs = sum(p[1] for p in pixels) // 16
            bs = sum(p[2] for p in pixels) // 16
            c0 = rgb565(rs, gs, bs)
            c1 = rgb565(min(255, rs + 32), min(255, gs + 32), min(255, bs + 32))
            out += struct.pack("<HH", c0, c1)
            bits = 0
            for i, (r, g, b, a) in enumerate(pixels):
                d0 = (r - rs) ** 2 + (g - gs) ** 2 + (b - bs) ** 2
                d1 = (r - min(255, rs + 32)) ** 2 + (g - min(255, gs + 32)) ** 2 + (b - min(255, bs + 32)) ** 2
                bits |= (0 if d0 <= d1 else 1) << (i * 2)
            out += struct.pack("<I", bits)
    return bytes(out)


def encode_image(img: Image.Image, fmt: int, w: int, h: int) -> bytes:
    if img.size != (w, h):
        img = img.resize((w, h), Image.Resampling.LANCZOS)
    if fmt == 4:
        return encode_rgb565(img)
    if fmt == 5:
        return encode_rgb5a3(img)
    if fmt == 6:
        return encode_rgba32(img)
    if fmt == 14:
        return encode_cmpr(img)
    raise ValueError(f"unsupported GX format {FMT_NAME.get(fmt, fmt)}")


def retexture_bmd(src: Path, dst: Path, replacements: dict[str, Path]) -> None:
    buf = bytearray(src.read_bytes())
    sec, _ = sections(buf)
    if "TEX1" not in sec:
        raise ValueError("no TEX1")
    tex_base, _ = sec["TEX1"]
    textures = parse_tex1(buf, tex_base)
    for tex in textures:
        name = tex["name"]
        if name not in replacements:
            continue
        png = replacements[name]
        img = Image.open(png)
        encoded = encode_image(img, tex["fmt"], tex["w"], tex["h"])
        if len(encoded) != tex["size"]:
            raise ValueError(
                f"{name}: encoded {len(encoded)} bytes != slot {tex['size']} "
                f"({tex['w']}x{tex['h']} {FMT_NAME.get(tex['fmt'], tex['fmt'])})"
            )
        start = tex["data_abs"]
        buf[start : start + tex["size"]] = encoded
        print(f"  patched {name} <- {png.name}")
    dst.write_bytes(buf)


def main() -> int:
    ap = argparse.ArgumentParser(description="Swap TEX1 payloads in a J3D2 BMD")
    ap.add_argument("--src", required=True, type=Path)
    ap.add_argument("--dst", required=True, type=Path)
    ap.add_argument("--map", nargs="+", required=True, help="name=png pairs")
    args = ap.parse_args()
    mapping: dict[str, Path] = {}
    for item in args.map:
        name, path = item.split("=", 1)
        mapping[name] = Path(path)
    print(f"retexture {args.src} -> {args.dst}")
    retexture_bmd(args.src, args.dst, mapping)
    print("done")
    return 0


if __name__ == "__main__":
    sys.exit(main())
