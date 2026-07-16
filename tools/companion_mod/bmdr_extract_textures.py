#!/usr/bin/env python3
"""Extract BTI textures from an embedded TEX1 section inside a TP BMDR blob."""
from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

FMT_BPP = {0: 0.5, 1: 1, 2: 1, 3: 2, 4: 2, 5: 2, 6: 4, 14: 0.5}
GX_TO_RGBA = {
    0: "I4",
    1: "I8",
    2: "IA4",
    3: "IA8",
    4: "RGB565",
    5: "RGB5A3",
    6: "RGBA8",
    14: "CMPR",
}


def find_tex1(data: bytes) -> int:
    best = -1
    for i in range(len(data) - 0x30):
        if data[i : i + 4] != b"TEX1":
            continue
        sec_size = struct.unpack_from(">I", data, i + 4)[0]
        tex_count = struct.unpack_from(">H", data, i + 8)[0]
        if sec_size < 0x20 or i + sec_size > len(data):
            continue
        if tex_count == 0 or tex_count > 64:
            continue
        hdr_rel = struct.unpack_from(">I", data, i + 0x0C)[0]
        str_rel = struct.unpack_from(">I", data, i + 0x10)[0]
        if hdr_rel < 0x20 or str_rel < 0x20:
            continue
        if i + hdr_rel + tex_count * 0x20 > len(data):
            continue
        best = i
    if best < 0:
        raise ValueError("no plausible TEX1 section found")
    return best


def parse_tex1(data: bytes, base: int) -> list[dict]:
    tex_count = struct.unpack_from(">H", data, base + 8)[0]
    hdr_off = base + struct.unpack_from(">I", data, base + 0x0C)[0]
    str_off = base + struct.unpack_from(">I", data, base + 0x10)[0]
    name_count = struct.unpack_from(">H", data, str_off)[0]
    names: list[str] = []
    for i in range(name_count):
        name_off = struct.unpack_from(">H", data, str_off + 4 + i * 4 + 2)[0]
        end = data.index(0, str_off + name_off)
        names.append(data[str_off + name_off : end].decode("ascii", "replace"))

    out: list[dict] = []
    for i in range(tex_count):
        h = hdr_off + i * 0x20
        hdr = data[h : h + 0x20]
        fmt = hdr[0]
        w = struct.unpack_from(">H", hdr, 2)[0]
        ht = struct.unpack_from(">H", hdr, 4)[0]
        data_off = struct.unpack_from(">I", hdr, 0x1C)[0]
        img_off = h + data_off
        bpp = FMT_BPP.get(fmt)
        if bpp is None:
            raise ValueError(f"unsupported GX format {fmt} on texture {names[i]}")
        size = int(w * ht * bpp)
        img = data[img_off : img_off + size]
        out.append(
            dict(
                name=names[i],
                fmt=fmt,
                fmt_name=GX_TO_RGBA.get(fmt, f"GX{fmt}"),
                width=w,
                height=ht,
                header=hdr,
                image=img,
            )
        )
    return out


def decode_i8_to_rgba(img: bytes, w: int, h: int) -> bytes:
    px = bytearray(w * h * 4)
    for i, v in enumerate(img[: w * h]):
        px[i * 4 : i * 4 + 4] = bytes([v, v, v, 255])
    return bytes(px)


def decode_ia8_to_rgba(img: bytes, w: int, h: int) -> bytes:
    px = bytearray(w * h * 4)
    for i in range(w * h):
        intensity = img[i * 2]
        alpha = img[i * 2 + 1]
        px[i * 4 : i * 4 + 4] = bytes([intensity, intensity, intensity, alpha])
    return bytes(px)


def decode_rgb5a3_to_rgba(img: bytes, w: int, h: int) -> bytes:
    px = bytearray(w * h * 4)
    for i in range(w * h):
        half = struct.unpack_from(">H", img, i * 2)[0]
        if half & 0x8000:
            r = ((half >> 10) & 0x1F) * 255 // 31
            g = ((half >> 5) & 0x1F) * 255 // 31
            b = (half & 0x1F) * 255 // 31
            a = 255
        else:
            r = ((half >> 9) & 0xF) * 255 // 15
            g = ((half >> 4) & 0xF) * 255 // 15
            b = (half & 0xF) * 255 // 15
            a = ((half >> 12) & 0x7) * 255 // 7
        px[i * 4 : i * 4 + 4] = bytes([r, g, b, a])
    return bytes(px)


def write_png(path: Path, rgba: bytes, w: int, h: int) -> None:
    import zlib

    raw = bytearray()
    stride = w * 4
    for y in range(h):
        raw.append(0)
        start = y * stride
        raw.extend(rgba[start : start + stride])

    def chunk(tag: bytes, payload: bytes) -> bytes:
        crc = zlib.crc32(tag + payload) & 0xFFFFFFFF
        return struct.pack(">I", len(payload)) + tag + payload + struct.pack(">I", crc)

    ihdr = struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0)
    png = b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", ihdr) + chunk(b"IDAT", zlib.compress(bytes(raw), 9)) + chunk(b"IEND", b"")
    path.write_bytes(png)


def decode_texture(tex: dict) -> tuple[bytes, int, int]:
    fmt = tex["fmt"]
    w, h = tex["width"], tex["height"]
    img = tex["image"]
    if fmt == 1:
        return decode_i8_to_rgba(img, w, h), w, h
    if fmt == 3:
        return decode_ia8_to_rgba(img, w, h), w, h
    if fmt == 5:
        return decode_rgb5a3_to_rgba(img, w, h), w, h
    raise ValueError(f"PNG export not implemented for {tex['fmt_name']} ({tex['name']})")


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument("bmdr")
    ap.add_argument("--out", type=Path, required=True)
    ap.add_argument("--tex1-offset", type=lambda x: int(x, 0), default=None)
    args = ap.parse_args()

    data = Path(args.bmdr).read_bytes()
    tex1_off = args.tex1_offset if args.tex1_offset is not None else find_tex1(data)
    textures = parse_tex1(data, tex1_off)
    args.out.mkdir(parents=True, exist_ok=True)

    manifest = [f"TEX1 offset: 0x{tex1_off:X}", f"textures: {len(textures)}", ""]
    for tex in textures:
        manifest.append(
            f"{tex['name']}: {tex['fmt_name']} {tex['width']}x{tex['height']} ({len(tex['image'])} bytes raw)"
        )
        try:
            rgba, w, h = decode_texture(tex)
            png_path = args.out / f"{tex['name']}.png"
            write_png(png_path, rgba, w, h)
            manifest.append(f"  -> wrote {png_path.name}")
        except ValueError as exc:
            manifest.append(f"  -> skipped PNG ({exc})")
            raw_path = args.out / f"{tex['name']}.btiraw"
            raw_path.write_bytes(tex["header"] + tex["image"])
            manifest.append(f"  -> wrote raw {raw_path.name}")

    (args.out / "textures_manifest.txt").write_text("\n".join(manifest) + "\n", encoding="utf-8")
    print("\n".join(manifest))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
