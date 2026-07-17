#!/usr/bin/env python3
"""Sync Nerrel Skull Kid source PNGs from MM-Reloaded GitHub + cross-ref local dump."""
from __future__ import annotations

import json
import re
import shutil
import urllib.request
import xml.etree.ElementTree as ET
from pathlib import Path

from PIL import Image

GITHUB_API = (
    "https://api.github.com/repos/GhostlyDark/MM-Reloaded/contents/"
    "ZELDA%20MAJORA'S%20MASK/Nerrel/Characters/Skull%20Kid"
)
GITHUB_WEB = (
    "https://github.com/GhostlyDark/MM-Reloaded/tree/master/"
    "ZELDA%20MAJORA'S%20MASK/Nerrel/Characters/Skull%20Kid"
)
OUT = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd"
)
NERREL_DIR = OUT / "_nerrel_skull_kid"
MANIFEST_LOCAL = OUT / "manifest_all.json"
XML = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk\object_stk.xml"
)
SCALES = (8, 16, 32, 64)
HASH_RE = re.compile(r"#([0-9A-F]{8})#", re.I)


def fetch_listing() -> list[dict]:
    req = urllib.request.Request(
        GITHUB_API,
        headers={"User-Agent": "dusklight-mm-skullkid-sync"},
    )
    return json.loads(urllib.request.urlopen(req, timeout=120).read())


def download_png(entry: dict, dest: Path) -> None:
    url = entry["download_url"]
    dest.write_bytes(urllib.request.urlopen(url, timeout=120).read())


def stk_size_matches(w: int, h: int) -> list[str]:
    root = ET.parse(XML).getroot()
    hits = []
    for tex in root.iter("Texture"):
        tw, th = int(tex.get("Width")), int(tex.get("Height"))
        for s in SCALES:
            if w == tw * s and h == th * s:
                hits.append(f"{tex.get('OutName')} ({tw}x{th} @{s}x)")
    return hits


def find_local_variants(hash8: str, by_hash_prefix: dict[str, list[dict]]) -> list[dict]:
    return by_hash_prefix.get(hash8.upper(), [])


def main() -> int:
    NERREL_DIR.mkdir(parents=True, exist_ok=True)
    listing = fetch_listing()
    png_entries = [e for e in listing if e["name"].endswith(".png")]
    print(f"Nerrel Skull Kid sources: {len(png_entries)}")

    by_hash_prefix: dict[str, list[dict]] = {}
    if MANIFEST_LOCAL.is_file():
        for e in json.loads(MANIFEST_LOCAL.read_text(encoding="utf-8")):
            m = HASH_RE.search(e.get("png", ""))
            if m:
                by_hash_prefix.setdefault(m.group(1).upper(), []).append(e)

    manifest: list[dict] = []
    copied_local = 0
    downloaded = 0

    for entry in sorted(png_entries, key=lambda x: x["name"]):
        name = entry["name"]
        dest = NERREL_DIR / name
        if not dest.is_file():
            download_png(entry, dest)
            downloaded += 1

        im = Image.open(dest)
        hash8 = HASH_RE.search(name).group(1).upper() if HASH_RE.search(name) else ""
        local = find_local_variants(hash8, by_hash_prefix)
        stk = stk_size_matches(im.width, im.height)

        # Prefer matching local RTZ decode in flat dump (for side-by-side)
        local_png_path = OUT / name
        if not local_png_path.is_file() and local:
            src_name = local[0]["png"]
            for candidate in [OUT / src_name, *OUT.rglob(src_name)]:
                if candidate.is_file():
                    shutil.copy2(candidate, local_png_path)
                    copied_local += 1
                    break

        manifest.append(
            {
                "png": name,
                "hash8": hash8,
                "size": [im.width, im.height],
                "github_bytes": entry.get("size"),
                "github_sha": entry.get("sha"),
                "local_rtz_variants": [
                    {
                        "png": v["png"],
                        "size": v["size"],
                        "dds_path": v.get("dds_path"),
                    }
                    for v in local
                ],
                "object_stk_size_match": stk,
            }
        )

    (NERREL_DIR / "nerrel_skull_kid.json").write_text(
        json.dumps(manifest, indent=2), encoding="utf-8"
    )

    lines = [
        "# Nerrel Skull Kid sources (MM-Reloaded)",
        "",
        f"Authoritative Skull Kid texture set from Nerrel's repo — **{len(png_entries)} PNGs**.",
        f"Synced from [{GITHUB_WEB}]({GITHUB_WEB}).",
        "",
        "These are **artist source PNGs** (correct colors), not RTZ BC7 decodes.",
        "Hash filenames match MMN64HD Rice keys; folder name `Skull Kid` is the label.",
        "",
        "| PNG | Size | object_stk size match (heuristic) | Local RTZ variants |",
        "|-----|------|-----------------------------------|--------------------|",
    ]
    for m in manifest:
        stk = "; ".join(m["object_stk_size_match"][:2]) or "—"
        if len(m["object_stk_size_match"]) > 2:
            stk += f" (+{len(m['object_stk_size_match']) - 2})"
        loc = str(len(m["local_rtz_variants"]))
        lines.append(f"| `{m['png']}` | {m['size'][0]}×{m['size'][1]} | {stk} | {loc} |")

    lines += [
        "",
        "## Notes",
        "",
        "- Prior `_groups/` picks used **different hashes** — none overlap this Nerrel set.",
        "- Use these PNGs as color reference when painting TP UV atlases.",
        "- See `nerrel_skull_kid.json` for full variant lists from local RTZ decode.",
        "",
    ]
    (NERREL_DIR / "README.md").write_text("\n".join(lines), encoding="utf-8")

    print(f"Downloaded {downloaded} new; {len(png_entries)} total in {NERREL_DIR}")
    print(f"Wrote {NERREL_DIR / 'README.md'}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
