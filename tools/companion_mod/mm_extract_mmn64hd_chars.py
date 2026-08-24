#!/usr/bin/env python3
"""Decode MMN64HD Character DDS from our local .rtz copy to PNGs.

Uses the same decode path as mm_refs_hd (BC7/DXT via texture2ddecoder) — NOT N64 ROM bytes.
Outputs keep the **original DDS path** in the filename so you can label them manually.
"""
from __future__ import annotations

import argparse
import hashlib
import re
import struct
import sys
import zipfile
from pathlib import Path

try:
    from PIL import Image
    import texture2ddecoder as t2d
except ImportError:
    raise SystemExit("pip install Pillow texture2ddecoder")

RTZ = Path(
    r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm-mods\MMN64HD\mm_MMN64HD.rtz"
)
OUT = Path(
    r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mmn64hd_chars"
)


def dds_to_image(data: bytes) -> Image.Image:
    if data[:4] != b"DDS ":
        raise ValueError("not DDS")
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
            raise ValueError(f"DXGI {dxgi}")
    elif fourcc == b"DXT1":
        rgba = t2d.decode_bc1(data[128:], w, h)
    elif fourcc in (b"DXT3", b"DXT5"):
        rgba = t2d.decode_bc3(data[128:], w, h)
    else:
        raise ValueError(f"fourcc {fourcc!r}")
    return Image.frombytes("RGBA", (w, h), bytes(rgba))


def safe_name(entry: str) -> str:
    # Characters/ZELDA MAJORA'S MASK#01967F09#2#1#130FC1D9_ciByRGBA.dds
    base = entry.replace("Characters/", "").replace("/", "_")
    base = re.sub(r"[^\w#@.-]+", "_", base)
    if len(base) > 180:
        base = hashlib.sha1(entry.encode()).hexdigest()[:16] + ".dds"
    return base.replace(".dds", ".png")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--rtz", type=Path, default=RTZ)
    ap.add_argument("--out", type=Path, default=OUT)
    ap.add_argument("--prefix", default="Characters/")
    ap.add_argument("--limit", type=int, default=0)
    args = ap.parse_args()

    if not args.rtz.is_file():
        print("Missing", args.rtz, file=sys.stderr)
        return 1

    args.out.mkdir(parents=True, exist_ok=True)
    index_lines = ["# MMN64HD Character textures (original hash filenames)", ""]

    z = zipfile.ZipFile(args.rtz)
    entries = [n for n in z.namelist() if n.startswith(args.prefix) and n.endswith(".dds")]
    entries.sort()
    if args.limit:
        entries = entries[: args.limit]

    ok = 0
    for n in entries:
        try:
            img = dds_to_image(z.read(n))
        except Exception as exc:
            print("skip", n, exc)
            continue
        out_name = safe_name(n)
        img.save(args.out / out_name)
        index_lines.append(f"- `{out_name}` ← `{n}` ({img.width}×{img.height})")
        ok += 1

    (args.out / "INDEX.md").write_text("\n".join(index_lines), encoding="utf-8")
    print(f"Wrote {ok} PNGs -> {args.out}")
    print("Browse here to pick Skull Kid refs; do NOT assume filename = object_stk OutName.")
    return 0


if __name__ == "__main__":
    sys.exit(main())
