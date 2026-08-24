#!/usr/bin/env python3
"""Extract object_stk N64 textures and resolve MMN64HD HD replacements."""
from __future__ import annotations

import json
import re
import struct
import sys
import urllib.request
import xml.etree.ElementTree as ET
import zipfile
from pathlib import Path

try:
    from PIL import Image
    import texture2ddecoder as t2d
except ImportError:
    raise SystemExit("pip install Pillow texture2ddecoder")

ROM = Path(
    r"D:\Extractuibs\Extractions 6.5\MM64 shiz\Zelda64Recompiled-v1.2.2-Windows\roms\Legend of Zelda, The - Majora's Mask (USA).z64"
)
RTZ = Path(
    r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm-mods\MMN64HD\mm_MMN64HD.rtz"
)
OUT_MM = Path(r"<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\textures_mm_src")
OUT_TP = Path(r"<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\textures_mm")
VANILLA = Path(r"<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\textures_vanilla")

# object_stk virtual start (US MM) from zeldaret/mm dmadata
OBJECT_STK_ROM = 0x00C03350


def rgba16_to_rgba(data: bytes, w: int, h: int) -> bytes:
    out = bytearray()
    for i in range(0, w * h * 2, 2):
        px = struct.unpack_from(">H", data, i)[0]
        r = ((px >> 11) & 0x1F) * 255 // 31
        g = ((px >> 6) & 0x1F) * 255 // 31
        b = ((px >> 1) & 0x1F) * 255 // 31
        a = 255 if (px & 1) else 0
        out += bytes((r, g, b, a))
    return bytes(out)


def ia16_to_rgba(data: bytes, w: int, h: int) -> bytes:
    out = bytearray()
    for i in range(0, w * h * 2, 2):
        px = struct.unpack_from(">H", data, i)[0]
        i8 = ((px >> 8) & 0xFF)
        a8 = (px & 0xFF)
        out += bytes((i8, i8, i8, a8))
    return bytes(out)


def decode_n64_tex(fmt: str, data: bytes, w: int, h: int) -> Image.Image:
    if fmt == "rgba16":
        return Image.frombytes("RGBA", (w, h), rgba16_to_rgba(data, w, h))
    if fmt == "ia16":
        return Image.frombytes("RGBA", (w, h), ia16_to_rgba(data, w, h))
    raise ValueError(fmt)


def tex_byte_size(fmt: str, w: int, h: int) -> int:
    if fmt == "rgba16":
        return w * h * 2
    if fmt == "ia16":
        return w * h * 2
    raise ValueError(fmt)


def dds_to_image(data: bytes) -> Image.Image:
    if data[:4] != b"DDS ":
        raise ValueError("not dds")
    h = struct.unpack_from("<I", data, 12)[0]
    w = struct.unpack_from("<I", data, 16)[0]
    fourcc = data[84:88]
    if fourcc == b"DX10":
        dxgi = struct.unpack_from("<I", data, 128)[0]
        payload = data[148:]
        if dxgi == 98:
            rgba = t2d.decode_bc7(payload, w, h)
        elif dxgi == 71:
            rgba = t2d.decode_bc1(payload, w, h)
        elif dxgi in (73, 74):
            rgba = t2d.decode_bc3(payload, w, h)
        else:
            raise ValueError(f"dxgi {dxgi}")
    elif fourcc == b"DXT1":
        rgba = t2d.decode_bc1(data[128:], w, h)
    elif fourcc in (b"DXT3", b"DXT5"):
        rgba = t2d.decode_bc3(data[128:], w, h)
    else:
        raise ValueError(f"fourcc {fourcc!r}")
    return Image.frombytes("RGBA", (w, h), bytes(rgba))


def load_stk_textures(rom: bytes) -> dict[str, Image.Image]:
    xml = urllib.request.urlopen(
        "https://raw.githubusercontent.com/zeldaret/mm/main/assets/xml/objects/object_stk.xml"
    ).read()
    root = ET.fromstring(xml)
    out: dict[str, Image.Image] = {}
    for tex in root.iter("Texture"):
        name = tex.get("OutName")
        fmt = tex.get("Format")
        w = int(tex.get("Width"))
        h = int(tex.get("Height"))
        off = int(tex.get("Offset"), 16)
        rom_off = OBJECT_STK_ROM + off
        size = tex_byte_size(fmt, w, h)
        blob = rom[rom_off : rom_off + size]
        out[name] = decode_n64_tex(fmt, blob, w, h)
    return out


def build_rice_index(rtz: zipfile.ZipFile) -> dict[str, str]:
    idx: dict[str, str] = {}
    for n in rtz.namelist():
        if not n.endswith(".dds"):
            continue
        m = re.search(r"#([0-9A-Fa-f]{8})#", n)
        if m:
            idx[m.group(1).lower()] = n
    return idx


def rice_hash_for_png(img: Image.Image) -> str:
    """Approximate Rice ciByRGBA hash used in MMN64HD filenames (palette index path)."""
    # Rice hash is over TMEM bytes; for ci8/rgba16 exports MMHD uses ciByRGBA suffix.
    # Use RT64 rt64.json rice key prefix match via rt64 database instead.
    raise NotImplementedError


def lookup_hd(rt64: dict, rice_idx: dict, z: zipfile.ZipFile, rice_key: str) -> Image.Image | None:
    path = None
    for t in rt64["textures"]:
        rice = t.get("hashes", {}).get("rice", "")
        if rice.lower().startswith(rice_key.lower()):
            path = t.get("path") or rice_idx.get(rice.split("#")[0].lower())
            if path and not path.endswith(".dds"):
                # autoPath rice naming
                for n in z.namelist():
                    if rice.split("#")[0].upper() in n.upper():
                        path = n
                        break
            break
    if not path:
        return None
    if not path.endswith(".dds"):
        cand = rice_idx.get(rice_key.lower())
        path = cand or path
    if path not in z.namelist():
        # try auto filename from rice hash only
        path = rice_idx.get(rice_key.lower())
    if not path:
        return None
    return dds_to_image(z.read(path))


def find_hd_by_rice_prefix(z: zipfile.ZipFile, rt64: dict, prefix: str) -> str | None:
    prefix = prefix.lower()
    for t in rt64["textures"]:
        rice = t.get("hashes", {}).get("rice", "")
        if rice.split("#")[0].lower() != prefix:
            continue
        p = t.get("path", "")
        if p and p in z.namelist():
            return p
        if p and not p.endswith(".dds"):
            for n in z.namelist():
                if p in n:
                    return n
    for n in z.namelist():
        if prefix in n.lower() and n.endswith(".dds"):
            return n
    return None


def paste_center(base: Image.Image, src: Image.Image, box: tuple[int, int, int, int]) -> None:
    x0, y0, x1, y1 = box
    bw, bh = x1 - x0, y1 - y0
    s = src.convert("RGBA").resize((bw, bh), Image.Resampling.LANCZOS)
    base.paste(s, (x0, y0), s)


def compose_tp_textures(mm: dict[str, Image.Image], vanilla_paths: dict[str, Path]) -> None:
    """Paint MM sources onto TP vanilla UV atlases (v1 heuristic regions)."""
    OUT_TP.mkdir(parents=True, exist_ok=True)

    head = Image.open(vanilla_paths["pm_tex01.png"]).convert("RGBA")
    leaf = Image.open(vanilla_paths["pm_leaf01.png"]).convert("RGBA")
    eye = Image.open(vanilla_paths["pm_eye.1.png"]).convert("RGBA")

    skin = mm.get("skull_kid_skin") or mm.get("skull_kid_head_skin")
    head_skin = mm.get("skull_kid_head_skin") or skin
    shawl = mm.get("skull_kid_shawl")
    fringe = mm.get("skull_kid_clothing_fringe")
    beak = mm.get("skull_kid_beak")
    pants = mm.get("skull_kid_pants_and_wrists")
    mm_eye = mm.get("skull_kid_eye")

    # Regions tuned against vanilla pm_tex01 layout (256x256 CMPR atlas).
    if head_skin:
        paste_center(head, head_skin, (48, 24, 176, 152))
    if beak:
        paste_center(head, beak, (108, 120, 148, 160))
    if skin:
        paste_center(head, skin, (160, 40, 240, 120))
        paste_center(head, skin, (16, 140, 96, 220))
    if pants:
        paste_center(head, pants, (176, 160, 248, 232))
    if fringe:
        paste_center(head, fringe, (8, 8, 120, 40))

    if shawl:
        paste_center(leaf, shawl, (0, 0, 64, 128))
    if fringe:
        paste_center(leaf, fringe, (0, 96, 64, 128))

    if mm_eye:
        paste_center(eye, mm_eye, (0, 0, 16, 16))

    head.save(OUT_TP / "pm_tex01.png")
    leaf.save(OUT_TP / "pm_leaf01.png")
    eye.save(OUT_TP / "pm_eye.1.png")
    print(f"wrote {OUT_TP}")


def main() -> int:
    if not ROM.is_file():
        print("MM ROM missing:", ROM, file=sys.stderr)
        return 1
    if not RTZ.is_file():
        print("MMN64HD rtz missing:", RTZ, file=sys.stderr)
        return 1

    rom = ROM.read_bytes()
    print("Extracting vanilla MM object_stk textures...")
    n64 = load_stk_textures(rom)
    OUT_MM.mkdir(parents=True, exist_ok=True)
    for name, img in n64.items():
        img.save(OUT_MM / f"{name}.png")

    print("Loading MMN64HD index...")
    z = zipfile.ZipFile(RTZ)
    rt64 = json.loads(z.read("rt64.json"))
    rice_idx = build_rice_index(z)

    # Match HD replacements by scanning rt64 rice hashes against filename list.
    # Build map: n64 outname -> HD image by finding closest rice entry via file path search.
    mm_hd: dict[str, Image.Image] = {}
    for out_name, n64_img in n64.items():
        # brute: find HD dds with same aspect ratio within scale 8-64 and minimum color distance on downscale
        tw, th = n64_img.size
        best = None
        best_score = 1e18
        n64_small = n64_img.resize((tw, th), Image.Resampling.LANCZOS)
        n64_px = list(n64_small.getdata())
        for n in z.namelist():
            if not (n.startswith("Characters/") and n.endswith(".dds")):
                continue
            try:
                hd = dds_to_image(z.read(n))
            except Exception:
                continue
            if hd.width / tw != hd.height / th:
                continue
            scale = hd.width / tw
            if scale not in (8, 16, 32, 64):
                continue
            sample = hd.resize((tw, th), Image.Resampling.LANCZOS)
            score = sum(
                abs(a - b) for (a, b) in zip(n64_px, sample.getdata()) if a[3] > 0 or b[3] > 0
            )
            if score < best_score:
                best_score = score
                best = hd
        if best is not None:
            mm_hd[out_name] = best
            best.save(OUT_MM / f"{out_name}_hd.png")
            print(f"  HD match {out_name}: {best.size} score={best_score:.0f}")

    vanilla_paths = {
        "pm_tex01.png": VANILLA / "pm_tex01.png",
        "pm_leaf01.png": VANILLA / "pm_leaf01.png",
        "pm_eye.1.png": VANILLA / "pm_eye.1.png",
    }
    if not all(p.is_file() for p in vanilla_paths.values()):
        print("Missing vanilla TP templates in", VANILLA, file=sys.stderr)
        return 1

    print("Compositing TP UV atlases...")
    compose_tp_textures(mm_hd, vanilla_paths)
    return 0


if __name__ == "__main__":
    sys.exit(main())
