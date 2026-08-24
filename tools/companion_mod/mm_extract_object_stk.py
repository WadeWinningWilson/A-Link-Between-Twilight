#!/usr/bin/env python3
"""Extract MM object_stk textures from our ROM copy (not from an installed game).

Outputs MM-native names only (zeldaret OutName + symbol in manifest).
TP slot mapping (pm_tex01 / lamp / trumpet) is a separate manual art step.
"""
from __future__ import annotations

import argparse
import json
import struct
import sys
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

try:
    from PIL import Image
    import pigment64
except ImportError:
    raise SystemExit("pip install Pillow pigment64")

# US 1.0 retail — matches Recomp bundle ROM (md5 f46493eaa0628827dbd6ad3ecd8d65d6)
DMADATA_OFF = 0x001A500
OBJECT_STK_ID = 0x0192
# gObjectTable in .rodata (US 1.0): object_stk RomFile at index 0x192
GOBJECT_TABLE_ROM = 0x001D040

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
WORK = Path(r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk")
XML_URL = "https://raw.githubusercontent.com/zeldaret/mm/main/assets/xml/objects/object_stk.xml"

TEX_SIZE = {
    "rgba16": lambda w, h: w * h * 2,
    "ia16": lambda w, h: w * h * 2,
}


def yaz0_decompress(data: bytes) -> bytes:
    if data[:4] != b"Yaz0":
        return data
    dec_size = struct.unpack(">I", data[4:8])[0]
    out = bytearray(dec_size)
    src, dst = 16, 0
    while dst < dec_size:
        code = data[src]
        src += 1
        for bit in range(8):
            if dst >= dec_size:
                break
            if code & (0x80 >> bit):
                out[dst] = data[src]
                dst += 1
                src += 1
            else:
                b1, b2 = data[src], data[src + 1]
                src += 2
                dist = ((b1 & 0xF) << 8) | b2
                length = (b1 >> 4) + 2
                offset = dst - dist - 1
                for i in range(length):
                    if dst >= dec_size:
                        break
                    out[dst] = out[offset + i]
                    dst += 1
    return bytes(out)


def parse_dmadata(rom: bytes, table_off: int) -> list[dict]:
    entries = []
    off = table_off
    while off + 16 <= len(rom):
        v0, v1, p0, p1 = struct.unpack_from(">IIII", rom, off)
        if v0 == 0 and v1 == 0 and p0 == 0 and p1 == 0:
            break
        entries.append({"v0": v0, "v1": v1, "p0": p0, "p1": p1})
        off += 16
    return entries


def fetch_file(rom: bytes, entry: dict, cache: dict[int, bytes]) -> bytes | None:
    p0, p1 = entry["p0"], entry["p1"]
    if p0 == 0xFFFFFFFF:
        return None
    size = entry["v1"] - entry["v0"]
    if p1 == 0:
        return rom[p0 : p0 + size]
    if p0 not in cache:
        cache[p0] = yaz0_decompress(rom[p0:p1])
    return cache[p0]


def object_stk_vrom(rom: bytes) -> tuple[int, int]:
    off = GOBJECT_TABLE_ROM + OBJECT_STK_ID * 8
    v0, v1 = struct.unpack_from(">II", rom, off)
    if v1 <= v0:
        raise ValueError(f"invalid gObjectTable entry @ {off:#x}: {v0:#x}-{v1:#x}")
    return v0, v1 - v0


def load_blob(rom: bytes, entries: list[dict], vrom_start: int, vrom_size: int) -> bytes:
    cache: dict[int, bytes] = {}
    for e in entries:
        if e["v0"] == vrom_start:
            data = fetch_file(rom, e, cache)
            if data is None:
                break
            return data[:vrom_size]
    raise ValueError(f"vrom {vrom_start:#x} not in dmadata")


def load_texture_defs() -> list[dict]:
    xml = urllib.request.urlopen(XML_URL, timeout=60).read()
    root = ET.fromstring(xml)
    out = []
    for tex in root.iter("Texture"):
        out.append(
            {
                "symbol": tex.get("Name"),
                "out_name": tex.get("OutName"),
                "format": tex.get("Format"),
                "width": int(tex.get("Width")),
                "height": int(tex.get("Height")),
                "offset": int(tex.get("Offset"), 16),
            }
        )
    return out


def decode_tex(fmt: str, raw: bytes, w: int, h: int) -> Image.Image:
    # pigment64 handles N64 tiling/swizzle (linear read was the corruption source)
    png_bytes = pigment64.native_to_png(raw, fmt, w, h, b"")
    from io import BytesIO

    return Image.open(BytesIO(png_bytes)).convert("RGBA")


def write_review(manifest: list[dict], vrom: int, size: int, out_dir: Path) -> None:
    lines = [
        "# MM object_stk — texture review",
        "",
        "Source: **our ROM copy** in the Recomp bundle (decompressed via dmadata + Yaz0).",
        f"Verified `object_stk` vrom `{vrom:#x}` size `{size:#x}` from `gObjectTable[0x192]`.",
        "",
        "These are **MM-native** textures only. They do **not** map 1:1 to TP `E_PM` files.",
        "",
        "- MM Skull Kid has no TP lamp submesh; `pm_lamp01` is TP-only (ignore for body v1).",
        "- MM flute/ocarina textures exist in `object_stk` but TP grove NPC uses trumpet (`E_PM_31`) — optional later.",
        "- TP reskin = paint MM reference art onto **TP UV atlases** (`textures_vanilla/pm_*.png`), not rename/swap files.",
        "",
        "## What differed: mm_refs_hd vs object_stk/n64",
        "",
        "mm_refs_hd used MMN64HD .rtz (BC7/DXT DDS) + texture2ddecoder — never touched the ROM.",
        "object_stk/n64 reads N64 rgba16 from ROM — decode still broken; use mm_extract_mmn64hd_chars.py for HD browse.",
        "",
        "## Folders",
        "",
        "| Folder | Contents |",
        "|--------|----------|",
        "| `n64/` | Native resolution, `{out_name}.png` |",
        "| `n64_preview8x/` | 8× nearest-neighbor preview |",
        "| `hd/` | (optional) your MMN64HD picks, same `{out_name}.png` names |",
        "",
        "## Textures (body-relevant highlighted)",
        "",
        "| OutName | Symbol | Format | Size |",
        "|---------|--------|--------|------|",
    ]
    body = {
        "skull_kid_skin",
        "skull_kid_head_skin",
        "skull_kid_beak",
        "skull_kid_eye",
        "skull_kid_shawl",
        "skull_kid_clothing_fringe",
        "skull_kid_pants_and_wrists",
        "skull_kid_hand",
        "skull_kid_shoe",
        "skull_kid_ring",
    }
    for m in manifest:
        mark = "**" if m["out_name"] in body else ""
        lines.append(
            f"| {mark}`{m['out_name']}`{mark} | `{m['symbol']}` | {m['format']} | {m['width']}×{m['height']} |"
        )
    lines += [
        "",
        "## Next step after you confirm previews",
        "",
        "1. Mark which `out_name` entries look correct (or note wrong ones).",
        "2. Optionally drop MMN64HD PNGs into `hd/` using the **same filenames**.",
        "3. In Blender, paint those references onto TP templates in `textures_vanilla/` → save `textures_mm/`.",
        "4. Tell me when `textures_mm/` is ready → relink Blender + `bmd_retexture` deploy.",
        "",
    ]
    (out_dir / "TEXTURE_REVIEW.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--rom", type=Path, default=ROM)
    ap.add_argument("--out", type=Path, default=WORK)
    ap.add_argument("--preview-scale", type=int, default=8)
    args = ap.parse_args()

    if not args.rom.is_file():
        print("ROM not found:", args.rom, file=sys.stderr)
        return 1

    rom = args.rom.read_bytes()
    vrom, size = object_stk_vrom(rom)
    entries = parse_dmadata(rom, DMADATA_OFF)
    blob = load_blob(rom, entries, vrom, size)
    print(f"object_stk vrom {vrom:#x} size {size:#x} ({len(blob)} bytes decompressed)")

    n64_dir = args.out / "n64"
    prev_dir = args.out / "n64_preview8x"
    hd_dir = args.out / "hd"
    for d in (n64_dir, prev_dir, hd_dir):
        d.mkdir(parents=True, exist_ok=True)

    manifest = []
    for tex in load_texture_defs():
        fmt = tex["format"]
        w, h = tex["width"], tex["height"]
        off = tex["offset"]
        raw_size = TEX_SIZE[fmt](w, h)
        raw = blob[off : off + raw_size]
        img = decode_tex(fmt, raw, w, h)
        name = tex["out_name"]
        img.save(n64_dir / f"{name}.png")
        if args.preview_scale > 1:
            img.resize((w * args.preview_scale, h * args.preview_scale), Image.Resampling.NEAREST).save(
                prev_dir / f"{name}.png"
            )
        entry = {**tex, "file": f"{name}.png"}
        manifest.append(entry)
        print(f"  {tex['symbol']} -> {name}.png ({w}x{h} {fmt})")

    (args.out / "manifest.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    write_review(manifest, vrom, size, args.out)
    print(f"Done -> {args.out}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
