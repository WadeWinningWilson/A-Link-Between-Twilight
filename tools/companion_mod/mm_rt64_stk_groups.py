#!/usr/bin/env python3
"""Cross-reference MMN64HD rt64.json rice hashes with object_stk decomp names."""
from __future__ import annotations

import json
import re
import urllib.request
import xml.etree.ElementTree as ET
from collections import defaultdict
from pathlib import Path

RT64 = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm-mods\MMN64HD\rt64.json"
)
OUT = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd"
)
XML = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk\object_stk.xml"
)
PRIOR = {
    "0461217F": "skull_kid_skin (guess)",
    "033C6482": "skull_kid_head_skin (guess)",
    "021C1551": "skull_kid_beak (guess)",
    "01967F09": "skull_kid_shawl (guess)",
    "03B80924": "skull_kid_clothing_fringe (guess)",
    "00BB7F5C": "skull_kid_pants_and_wrists (guess)",
    "30323426": "skull_kid_eye (guess)",
    "00BF8A97": "skull_kid_hand (guess)",
}

DDS_PAT = re.compile(
    r"ZELDA MAJORA'S MASK#([0-9A-F]{8})#(\d+)#(\d+)(?:#([0-9A-F]{8}))?_(.+)\.dds$",
    re.I,
)


def load_stk_textures() -> list[dict]:
    root = ET.parse(XML).getroot()
    out = []
    for tex in root.iter("Texture"):
        out.append(
            {
                "symbol": tex.get("Name"),
                "out_name": tex.get("OutName"),
                "format": tex.get("Format"),
                "w": int(tex.get("Width")),
                "h": int(tex.get("Height")),
                "offset": int(tex.get("Offset"), 16),
            }
        )
    return out


def main() -> None:
    rt = json.loads(RT64.read_text(encoding="utf-8"))
    by_rice: dict[str, dict] = {}
    char_paths: list[dict] = []
    for entry in rt["textures"]:
        rice = entry["hashes"]["rice"].lower()
        by_rice[rice] = entry
        path = entry["path"]
        if path.startswith("Characters/") or "Characters" in path:
            char_paths.append(entry)

    print(f"rt64 textures: {len(rt['textures'])}, Character paths: {len(char_paths)}")

    # Match priority PNG names via rice key
    print("\n=== Prior hash -> rt64.json path ===")
    for h, label in PRIOR.items():
        matches = [
            (k, v["path"])
            for k, v in by_rice.items()
            if k.startswith(h.lower() + "#")
        ]
        print(f"{h} ({label})")
        for k, p in sorted(matches):
            print(f"  rice={k} -> {p}")

    # Filename suffix stats for Characters
    suffix_groups: dict[str, int] = defaultdict(int)
    for k, v in by_rice.items():
        p = v["path"].replace("\\", "/")
        if p.startswith("Characters/"):
            base = p.split("/")[-1]
        else:
            # autoPath rice: filename is only in the .rtz zip, keyed by rice hash
            continue
        m = DDS_PAT.search(base)
        if m:
            suffix_groups[m.group(5)] += 1

    # Count from actual Character DDS in manifest if available
    manifest_path = OUT / "manifest_all.json"
    if manifest_path.is_file():
        manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
        suffix_groups = defaultdict(int)
        for e in manifest:
            if not e["dds_path"].startswith("Characters/"):
                continue
            base = e["dds_path"].split("/")[-1]
            m = DDS_PAT.search(base)
            if m:
                suffix_groups[m.group(5)] += 1

    print("\n=== MMN64HD suffix meaning (Character folder) ===")
    for suf, count in sorted(suffix_groups.items(), key=lambda x: -x[1]):
        print(f"  _{suf}: {count} textures")

    # Group object_stk by decomp section (using XML order + comments)
    stk = load_stk_textures()
    groups = {
        "body_worn": [
            "skull_kid_clothing_fringe",
            "skull_kid_ring",
            "skull_kid_pants_and_wrists",
            "skull_kid_skin",
            "skull_kid_shawl",
            "skull_kid_shoe",
        ],
        "head_face": [
            "skull_kid_eye",
            "skull_kid_beak",
            "skull_kid_head_skin",
            "skull_kid_hand",
            "skull_kid_oot_style_mouth",
        ],
        "majoras_mask": [
            "skull_kid_majoras_mask_eye",
            "skull_kid_majoras_mask_face",
            "skull_kid_majoras_mask_spikes_1",
            "skull_kid_majoras_mask_spikes_2",
            "skull_kid_majoras_mask_spikes_3",
            "skull_kid_majoras_mask_spikes_4",
            "skull_kid_majoras_mask_back",
            "skull_kid_unused_majoras_mask_eye",
        ],
        "link_mask_unused": [
            "skull_kid_link_mask_eye",
            "skull_kid_link_mask_nose",
            "skull_kid_link_mask_mouth",
            "skull_kid_link_mask_hair",
            "skull_kid_link_mask_hat",
            "skull_kid_link_mask_ear",
        ],
        "props": [
            "skull_kid_ocarina_of_time",
            "skull_kid_flute",
            "skull_kid_flute_hole",
        ],
        "unused_dupes": [
            "skull_kid_unused_eye",
            "skull_kid_unused_eye2",
        ],
    }

    lines = [
        "# object_stk texture groups (zeldaret decomp)",
        "",
        "From `object_stk.xml` — **OutName** is the only stable label on the N64 side.",
        "MMN64HD filenames use **Rice hash** (`0461217F#2#1`) + variant suffix.",
        "",
        "## Hue / color variant clue (MMN64HD pack)",
        "",
        "| Suffix | Count (Characters) | Likely meaning |",
        "|--------|-------------------|----------------|",
        f"| `_ciByRGBA` | {suffix_groups.get('ciByRGBA', 0)} | CI/RGBA16 with palette applied — **prefer for correct hue** |",
        f"| `_all` | {suffix_groups.get('all', 0)} | Full RGBA / alternate mip slot — **often blue-shifted or wrong** |",
        "",
        "Middle numbers encode N64 load params (RT64 Rice key): `#2#1#` pairs with `_ciByRGBA`; `#0#2#` / `#4#1#` pairs with `_all`.",
        "When one ROM tex has both variants, open the `_ciByRGBA` file first.",
        "",
        "## Decomp groups (for manual matching by native size)",
        "",
    ]

    scales = (8, 16, 32, 64)
    for gname, names in groups.items():
        lines.append(f"### {gname}")
        lines.append("")
        lines.append("| OutName | Symbol | Format | Native | HD @32x |")
        lines.append("|---------|--------|--------|--------|---------|")
        for t in stk:
            if t["out_name"] not in names:
                continue
            hd = f"{t['w']*32}x{t['h']*32}"
            lines.append(
                f"| `{t['out_name']}` | `{t['symbol']}` | {t['format']} | {t['w']}x{t['h']} | {hd} |"
            )
        lines.append("")

    lines += [
        "## rt64.json",
        "",
        f"Extracted from `mm_MMN64HD.rtz` — maps `rice` hash → DDS path inside pack.",
        f"**{len(by_rice)}** entries total; use hash prefix to find all variants of one ROM texture.",
        "",
        "### Prior hashes in rt64.json",
        "",
    ]
    for h, label in PRIOR.items():
        matches = sorted(
            (k, v["path"]) for k, v in by_rice.items() if k.startswith(h.lower() + "#")
        )
        lines.append(f"**`{h}`** ({label})")
        for k, p in matches:
            lines.append(f"- `{k}` → `{p}`")
        lines.append("")

    out_md = OUT / "OBJECT_STK_GROUPS.md"
    out_md.write_text("\n".join(lines), encoding="utf-8")
    print(f"\nWrote {out_md}")


if __name__ == "__main__":
    main()
