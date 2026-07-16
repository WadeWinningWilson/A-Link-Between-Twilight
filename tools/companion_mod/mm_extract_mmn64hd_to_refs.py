#!/usr/bin/env python3
"""Dump MMN64HD Character DDS → PNG into mm_refs_hd using original hash filenames.

Proven decode: texture2ddecoder (BC7/DXT) — same path as the clean mm_refs_hd images.
Optionally flags Skull Kid candidates via object_stk native dimensions × HD scale.
"""
from __future__ import annotations

import argparse
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

RTZ = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm-mods\MMN64HD\mm_MMN64HD.rtz"
)
OUT = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd"
)
XML_URL = "https://raw.githubusercontent.com/zeldaret/mm/main/assets/xml/objects/object_stk.xml"
SCALES = (8, 16, 32, 64)

# Prior hash paths from first clean decode pass (labels were guesses; pixels were good)
PRIOR_GUESSES = {
    "skull_kid_skin": "Characters/ZELDA MAJORA'S MASK#0461217F#2#1#EFF1C638_ciByRGBA.dds",
    "skull_kid_head_skin": "Characters/ZELDA MAJORA'S MASK#033C6482#2#1#2470C512_ciByRGBA.dds",
    "skull_kid_beak": "Characters/ZELDA MAJORA'S MASK#021C1551#2#1#1F7C0964_ciByRGBA.dds",
    "skull_kid_shawl": "Characters/ZELDA MAJORA'S MASK#01967F09#2#1#130FC1D9_ciByRGBA.dds",
    "skull_kid_clothing_fringe": "Characters/ZELDA MAJORA'S MASK#03B80924#0#2_all.dds",
    "skull_kid_pants_and_wrists": "Characters/ZELDA MAJORA'S MASK#00BB7F5C#4#1_all.dds",
    "skull_kid_eye": "Characters/ZELDA MAJORA'S MASK#30323426#2#1#61A28012_ciByRGBA.dds",
    "skull_kid_hand": "Characters/ZELDA MAJORA'S MASK#00BF8A97#0#2_all.dds",
}

# v1 body slots from object_stk (for narrowing size heuristics)
BODY_SLOTS = {
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

LEGACY_RENAMED = (
    "skull_kid_skin.png",
    "skull_kid_head_skin.png",
    "skull_kid_beak.png",
    "skull_kid_shawl.png",
    "skull_kid_clothing_fringe.png",
    "skull_kid_pants_and_wrists.png",
    "skull_kid_eye.png",
    "skull_kid_hand.png",
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


def png_name_from_dds_path(entry: str) -> str:
    """Keep hash identity from RTZ path; only strip folder + .dds."""
    base = entry
    if base.startswith("Characters/"):
        base = base[len("Characters/") :]
    base = base.replace(".dds", ".png")
    base = re.sub(r'[<>:"|?*]', "_", base)
    return base


def load_stk_sizes() -> list[dict]:
    xml = urllib.request.urlopen(XML_URL, timeout=60).read()
    root = ET.fromstring(xml)
    out = []
    for tex in root.iter("Texture"):
        out.append(
            {
                "out_name": tex.get("OutName"),
                "symbol": tex.get("Name"),
                "w": int(tex.get("Width")),
                "h": int(tex.get("Height")),
            }
        )
    return out


def match_stk(w: int, h: int, stk: list[dict]) -> list[str]:
    hits = []
    for t in stk:
        tw, th = t["w"], t["h"]
        for s in SCALES:
            if w == tw * s and h == th * s:
                hits.append(f"{t['out_name']} ({tw}x{th} @{s}x)")
    return hits


def collect_entries(z: zipfile.ZipFile, prefix: str, include_root: bool) -> list[str]:
    entries: set[str] = set()
    if prefix:
        entries.update(
            n for n in z.namelist() if n.startswith(prefix) and n.endswith(".dds")
        )
    if include_root:
        entries.update(
            n
            for n in z.namelist()
            if n.endswith(".dds")
            and "/" not in n
            and n.startswith("ZELDA MAJORA'S MASK#")
        )
    return sorted(entries)


def archive_legacy_renamed(out: Path) -> None:
    legacy = out / "_legacy_renamed"
    moved = 0
    for name in LEGACY_RENAMED:
        src = out / name
        if not src.is_file():
            continue
        legacy.mkdir(exist_ok=True)
        dst = legacy / name
        if dst.exists():
            src.unlink()
        else:
            src.rename(dst)
        moved += 1
    if moved:
        print(f"Moved {moved} old skull_kid_*.png renames -> {legacy}")


def build_priority(manifest: list[dict], char_count_by_size: dict[tuple[int, int], int]) -> list[dict]:
    """High-confidence Skull Kid refs: prior clean decode + rare Character sizes."""
    priority: list[dict] = []
    seen: set[str] = set()

    def add(entry: dict, reason: str, tier: int) -> None:
        if entry["png"] in seen:
            return
        seen.add(entry["png"])
        priority.append({**entry, "priority_reason": reason, "priority_tier": tier})

    for e in manifest:
        if e["prior_guess"]:
            add(e, "prior clean decode (labels are guesses)", 1)

    for e in manifest:
        if not e["dds_path"].startswith("Characters/"):
            continue
        w, h = e["size"]
        if char_count_by_size.get((w, h), 0) == 1:
            body_hits = [
                h.split(" (")[0]
                for h in e["stk_size_match"]
                if h.split(" (")[0] in BODY_SLOTS
            ]
            slot = ", ".join(dict.fromkeys(body_hits)) if body_hits else "unique Character size"
            add(e, f"only Character PNG at {w}x{h} ({slot})", 2)

    return sorted(priority, key=lambda x: (x["priority_tier"], x["png"]))


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--rtz", type=Path, default=RTZ)
    ap.add_argument("--out", type=Path, default=OUT)
    ap.add_argument("--prefix", default="Characters/")
    ap.add_argument(
        "--include-root",
        action="store_true",
        help="Also decode root-level ZELDA hash DDS (environment/objects; not Characters/)",
    )
    args = ap.parse_args()

    if not args.rtz.is_file():
        print("Missing", args.rtz, file=sys.stderr)
        return 1

    args.out.mkdir(parents=True, exist_ok=True)
    archive_legacy_renamed(args.out)
    stk = load_stk_sizes()

    z = zipfile.ZipFile(args.rtz)
    entries = collect_entries(z, args.prefix, args.include_root)
    scope = "Characters + root ZELDA" if args.include_root else args.prefix or "all"
    print(f"Decoding {len(entries)} DDS ({scope}) -> {args.out}")

    manifest = []
    candidates: list[dict] = []
    ok = skip = 0

    for dds_path in entries:
        png_name = png_name_from_dds_path(dds_path)
        out_path = args.out / png_name
        try:
            img = dds_to_image(z.read(dds_path))
            img.save(out_path)
            ok += 1
        except Exception as exc:
            skip += 1
            continue

        stk_hits = match_stk(img.width, img.height, stk)
        prior = [k for k, v in PRIOR_GUESSES.items() if v == dds_path]
        entry = {
            "png": png_name,
            "dds_path": dds_path,
            "size": [img.width, img.height],
            "prior_guess": prior,
            "stk_size_match": stk_hits,
        }
        manifest.append(entry)
        if stk_hits or prior:
            candidates.append(entry)

    char_count_by_size: dict[tuple[int, int], int] = {}
    for e in manifest:
        if e["dds_path"].startswith("Characters/"):
            key = tuple(e["size"])
            char_count_by_size[key] = char_count_by_size.get(key, 0) + 1

    priority = build_priority(manifest, char_count_by_size)

    (args.out / "manifest_all.json").write_text(json.dumps(manifest, indent=2), encoding="utf-8")
    (args.out / "skull_kid_candidates.json").write_text(json.dumps(candidates, indent=2), encoding="utf-8")
    (args.out / "skull_kid_priority.json").write_text(json.dumps(priority, indent=2), encoding="utf-8")
    (args.out / "mapping.json").write_text(
        json.dumps(PRIOR_GUESSES, indent=2), encoding="utf-8"
    )

    pri_lines = [
        "# Skull Kid priority refs (MMN64HD → hash filenames)",
        "",
        "Start here for manual review. **Tier 1** = prior clean BC7 decode (slot names were guesses).",
        "**Tier 2** = only Character PNG at that HD size (stronger than the 1278-entry size table).",
        "",
        f"Full Character dump: **{sum(1 for e in manifest if e['dds_path'].startswith('Characters/'))}** PNGs.",
        f"Total in folder after this run: **{ok}** decoded (+ skipped {skip}).",
        "",
        "| Tier | PNG | Size | Note |",
        "|------|-----|------|------|",
    ]
    for p in priority:
        pg = ", ".join(p["prior_guess"]) if p["prior_guess"] else "—"
        note = p["priority_reason"]
        if pg != "—":
            note = f"{pg}; {note}"
        pri_lines.append(
            f"| {p['priority_tier']} | `{p['png']}` | {p['size'][0]}×{p['size'][1]} | {note} |"
        )
    (args.out / "SKULL_KID_PRIORITY.md").write_text("\n".join(pri_lines), encoding="utf-8")

    lines = [
        "# MMN64HD dump (hash filenames)",
        "",
        f"Decoded **{ok}** PNGs from `{args.rtz.name}` into this folder.",
        f"Skipped **{skip}** (unsupported DDS). Scope: `{scope}`.",
        "",
        "Filenames match RTZ hash paths (no `skull_kid_*` rename).",
        "See **`SKULL_KID_PRIORITY.md`** first (~10 high-signal refs), then this table.",
        "",
        "## Skull Kid candidates (broad)",
        "",
        "Flagged if HD size matches `object_stk` native size × {8,16,32,64}, and/or prior clean decode.",
        f"**{len(candidates)}** entries — most Character PNGs match some stk size; use priority list instead.",
        "",
        "| PNG | Size | Prior guess | object_stk size match |",
        "|-----|------|-------------|------------------------|",
    ]
    for c in sorted(candidates, key=lambda x: (not x["prior_guess"], x["png"])):
        pg = ", ".join(c["prior_guess"]) if c["prior_guess"] else "—"
        sm = "; ".join(c["stk_size_match"][:3]) if c["stk_size_match"] else "—"
        if len(c["stk_size_match"]) > 3:
            sm += f" (+{len(c['stk_size_match']) - 3})"
        lines.append(f"| `{c['png']}` | {c['size'][0]}×{c['size'][1]} | {pg} | {sm} |")

    lines += [
        "",
        "## Prior guess hash paths (pixels good; labels unverified)",
        "",
    ]
    for label, dds in PRIOR_GUESSES.items():
        lines.append(f"- **{label}** → `{png_name_from_dds_path(dds)}`")

    (args.out / "SKULL_KID_CANDIDATES.md").write_text("\n".join(lines), encoding="utf-8")
    print(f"Done: {ok} PNGs, {len(priority)} priority / {len(candidates)} broad candidates")
    print(f"See {args.out / 'SKULL_KID_PRIORITY.md'}")

    # Move priority Skull Kid refs into decomp group folders
    group_script = Path(__file__).resolve().parent / "mm_refs_hd_group_folders.py"
    if group_script.is_file():
        import subprocess

        subprocess.run([sys.executable, str(group_script)], check=False)

    return 0


if __name__ == "__main__":
    sys.exit(main())
