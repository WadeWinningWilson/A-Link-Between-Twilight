#!/usr/bin/env python3
"""№120: extract the donor's vegetation assets into a mod-folder pack.

The grass / tree / flower systems are NOT actor overlays - they are packet
systems compiled into the donor's main executable, with their textures and
display lists as static arrays. `framework.map` names every one of them with an
exact address and size, so this is extraction, not reverse-engineering.

COVENANT: these bytes may never enter the repo. A port that did
`#include "assets/l_K_kusa_00TEX.h"` would compile donor art straight into the
shipped binary. So they are written to the mod folder and loaded at runtime,
the same separation every other donor asset uses.

Address -> file offset goes through the executable header (7 text + 11 data
sections, each with file offset / load address / size) rather than trusting the
map's first column, which is section-relative.

Output: <mod>/assets/veg/<symbol>.bin + veg_manifest.ini describing each blob.
"""
from __future__ import annotations

import io
import os
import re
import struct
import sys
from pathlib import Path

MOD = (
    Path(os.environ["APPDATA"])
    / "TwilitRealm"
    / "Dusklight"
    / "model_replacements"
    / "WW-Crew-Restoration"
)
DOL = Path("D:/XXXXXXX/Ex WW/sys/main.dol")
MAP = Path("D:/XXXXXXX/Ex WW/files/maps/framework.map")
WANT_OBJS = ("d_grass.o", "d_tree.o", "d_flower.o")

# Texture dimensions the packet code declares alongside each array. Recorded
# here so the port does not have to re-derive them (all CMPR/RGBA5A3 tiles).
TEX_DIMS = {
    "l_K_kusa_00TEX": (64, 128),
    "l_Txa_ob_kusa_aTEX": (64, 64),
    "l_Txa_swood_aTEX": (64, 64),
    "l_Txa_kage_32TEX": (32, 32),
    "l_Txq_bessou_hanaTEX": (64, 128),
    "l_Txo_ob_flower_white_64x64TEX": (64, 64),
    "l_Txo_ob_flower_pink_64x64TEX": (64, 64),
}


def dol_sections(d: bytes):
    """Yield (file_off, load_addr, size) for every populated section."""
    for i in range(18):
        off = struct.unpack_from(">I", d, i * 4)[0]
        addr = struct.unpack_from(">I", d, 0x48 + i * 4)[0]
        size = struct.unpack_from(">I", d, 0x90 + i * 4)[0]
        if off and size:
            yield off, addr, size


def addr_to_off(secs, addr: int, size: int):
    for off, load, ssize in secs:
        if load <= addr and addr + size <= load + ssize:
            return off + (addr - load)
    return None


def main() -> int:
    if not DOL.is_file():
        raise SystemExit(f"missing {DOL}")
    if not MAP.is_file():
        raise SystemExit(f"missing {MAP}")

    dol = DOL.read_bytes()
    secs = list(dol_sections(dol))

    rx = re.compile(
        r"\s*([0-9a-f]{8})\s+([0-9a-f]{6})\s+([0-9a-f]{8})\s+\d+\s+(\S+)\s+(\S+\.o)"
    )
    syms = []
    for line in io.open(MAP, encoding="utf-8", errors="replace"):
        m = rx.match(line)
        if not m:
            continue
        _soff, size, va, sym, obj = m.groups()
        # Vertex arrays are bound via GFSetArray at draw time and are just as
        # required as the DLs. They share names across objects (l_pos exists in
        # d_chain/d_grass/d_tree), which the source-prefixed key handles.
        if obj in WANT_OBJS and re.search(
            r"TEX|DL[0-9]*$|^l_(pos|color|texCoord)$|^l_Vmori_(pos|color|texCoord)$", sym
        ):
            syms.append((obj, sym, int(va, 16), int(size, 16)))

    out_dir = MOD / "assets" / "veg"
    out_dir.mkdir(parents=True, exist_ok=True)

    lines = [
        "# №120 vegetation asset pack (extracted, mod-side only).",
        "# Loaded at runtime by the ported grass/tree/flower packets.",
        "# size= is authoritative; w/h present for textures only.",
        "",
    ]
    written = total = 0
    for obj, sym, va, size in sorted(syms):
        foff = addr_to_off(secs, va, size)
        if foff is None:
            print(f"  !! {sym}: addr 0x{va:08X} outside every section - SKIPPED")
            continue
        blob = dol[foff : foff + size]
        if len(blob) != size:
            print(f"  !! {sym}: short read - SKIPPED")
            continue
        # l_matDL exists in ALL THREE objects - unprefixed names collide
        # and silently overwrite each other. Key every blob by its source.
        key = f"{obj[:-2]}__{sym}"
        (out_dir / f"{key}.bin").write_bytes(blob)
        written += 1
        total += size
        lines.append(f"[{key}]")
        lines.append(f"source={obj[:-2]}")
        lines.append(f"symbol={sym}")
        lines.append(f"size={size}")
        if sym in TEX_DIMS:
            w, h = TEX_DIMS[sym]
            lines.append(f"width={w}")
            lines.append(f"height={h}")
        lines.append("")
        print(f"  {sym:34} {size:>5} bytes  (from {obj})")

    io.open(out_dir / "veg_manifest.ini", "w", encoding="utf-8", newline="\n").write(
        "\n".join(lines)
    )
    print(f"\n{written}/{len(syms)} blobs -> {out_dir}  ({total} bytes, {total/1024:.1f} KB)")
    return 0


if __name__ == "__main__":
    sys.stdout.reconfigure(encoding="utf-8")
    raise SystemExit(main())
