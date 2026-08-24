#!/usr/bin/env python3
"""Move Skull Kid priority PNGs into decomp group folders (hash filenames unchanged)."""
from __future__ import annotations

import json
import shutil
from pathlib import Path

OUT = Path(
    r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd"
)
GROUPS_ROOT = OUT / "_groups"

# object_stk.xml categories (OutName lists for README / future expansion)
GROUP_SLOTS: dict[str, list[str]] = {
    "body_worn": [
        "skull_kid_clothing_fringe",
        "skull_kid_ring",
        "skull_kid_pants_and_wrists",
        "skull_kid_skin",
        "skull_kid_shawl",
        "skull_kid_shoe",
    ],
    "head_face": [
        "skull_kid_oot_style_mouth",
        "skull_kid_hand",
        "skull_kid_eye",
        "skull_kid_beak",
        "skull_kid_head_skin",
    ],
    "majoras_mask": [
        "skull_kid_majoras_mask_eye",
        "skull_kid_unused_majoras_mask_eye",
        "skull_kid_majoras_mask_face",
        "skull_kid_majoras_mask_spikes_1",
        "skull_kid_majoras_mask_spikes_2",
        "skull_kid_majoras_mask_spikes_3",
        "skull_kid_majoras_mask_spikes_4",
        "skull_kid_majoras_mask_back",
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

SLOT_GROUP = {
    slot: group for group, slots in GROUP_SLOTS.items() for slot in slots
}

# Prior clean-decode hashes → slot folder (labels still unverified)
PRIOR_SLOT_BY_PNG: dict[str, str] = {
    "ZELDA MAJORA'S MASK#0461217F#2#1#EFF1C638_ciByRGBA.png": "skull_kid_skin",
    "ZELDA MAJORA'S MASK#033C6482#2#1#2470C512_ciByRGBA.png": "skull_kid_head_skin",
    "ZELDA MAJORA'S MASK#021C1551#2#1#1F7C0964_ciByRGBA.png": "skull_kid_beak",
    "ZELDA MAJORA'S MASK#03B80924#0#2_all.png": "skull_kid_clothing_fringe",
    "ZELDA MAJORA'S MASK#00BB7F5C#4#1_all.png": "skull_kid_pants_and_wrists",
    "ZELDA MAJORA'S MASK#30323426#2#1#61A28012_ciByRGBA.png": "skull_kid_eye",
    "ZELDA MAJORA'S MASK#00BF8A97#0#2_all.png": "skull_kid_hand",
}

# Decomp size correction: was labeled shawl; native 32x64 @64x = mask face
OVERRIDE_SLOT: dict[str, tuple[str, str, str]] = {
    "ZELDA MAJORA'S MASK#01967F09#2#1#130FC1D9_ciByRGBA.png": (
        "majoras_mask",
        "skull_kid_majoras_mask_face",
        "Prior pass labeled shawl; decomp 32x64@64x -> majoras_mask_face.",
    ),
}

# Tier-2 unique Character sizes → best slot guess
TIER2_SLOT: dict[str, str] = {
    "ZELDA MAJORA'S MASK#882AF9BF#2#1#52C62400_ciByRGBA.png": "skull_kid_shawl",
    "ZELDA MAJORA'S MASK#8464E76C#0#2_all.png": "skull_kid_link_mask_hat",
    "ZELDA MAJORA'S MASK#99B05AE1#0#2_all.png": "skull_kid_clothing_fringe",
}


def move_into_group(png_name: str, group: str, slot: str, note: str = "") -> bool:
    src = OUT / png_name
    if not src.is_file():
        # already moved, or sitting under _groups from a prior run
        for hit in GROUPS_ROOT.rglob(png_name):
            return True
        return False

    dest_dir = GROUPS_ROOT / group / slot
    dest_dir.mkdir(parents=True, exist_ok=True)
    dest = dest_dir / png_name
    if dest.exists():
        src.unlink()
        return True
    shutil.move(str(src), str(dest))
    if note:
        readme = dest_dir / "NOTE.txt"
        if not readme.exists():
            readme.write_text(note + "\n", encoding="utf-8")
    return True


def write_readmes() -> None:
    GROUPS_ROOT.mkdir(exist_ok=True)
    lines = [
        "# Skull Kid texture groups (object_stk decomp)",
        "",
        "Hash-named PNGs moved here for review. Filenames are unchanged.",
        "Full dump remains in the parent `mm_refs_hd/` folder.",
        "",
        "| Folder | object_stk slots |",
        "|--------|------------------|",
    ]
    for group, slots in GROUP_SLOTS.items():
        lines.append(f"| `{group}/` | {', '.join(f'`{s}`' for s in slots)} |")
    lines += [
        "",
        "Subfolders use decomp **OutName** (e.g. `head_face/skull_kid_eye/`).",
        "Prefer `_ciByRGBA` / `#2#1#` files when a slot has hue variants.",
        "",
    ]
    (GROUPS_ROOT / "README.md").write_text("\n".join(lines), encoding="utf-8")

    for group, slots in GROUP_SLOTS.items():
        gdir = GROUPS_ROOT / group
        gdir.mkdir(exist_ok=True)
        body = [
            f"# {group}",
            "",
            "Slots from zeldaret `object_stk.xml`:",
            "",
        ]
        for s in slots:
            sub = gdir / s
            marker = " *(has PNG)*" if sub.is_dir() and any(sub.glob("*.png")) else ""
            body.append(f"- `{s}`{marker}")
        body.append("")
        (gdir / "README.md").write_text("\n".join(body), encoding="utf-8")


def main() -> int:
    moved = 0
    missing = []

    for png, slot in PRIOR_SLOT_BY_PNG.items():
        if png in OVERRIDE_SLOT:
            group, slot, note = OVERRIDE_SLOT[png]
        else:
            group = SLOT_GROUP[slot]
            note = ""
        if move_into_group(png, group, slot, note):
            moved += 1
        else:
            missing.append(png)

    for png, (group, slot, note) in OVERRIDE_SLOT.items():
        if png in PRIOR_SLOT_BY_PNG:
            continue
        if move_into_group(png, group, slot, note):
            moved += 1
        else:
            missing.append(png)

    for png, slot in TIER2_SLOT.items():
        group = SLOT_GROUP.get(slot, "link_mask_unused")
        if slot.startswith("skull_kid_link"):
            group = "link_mask_unused"
        if move_into_group(png, group, slot, "Tier-2 unique Character size (size heuristic)."):
            moved += 1
        else:
            missing.append(png)

    # Remaining tier-2 priority → review bucket
    review_dir = GROUPS_ROOT / "_review_unique_sizes"
    priority_path = OUT / "skull_kid_priority.json"
    if priority_path.is_file():
        for entry in json.loads(priority_path.read_text(encoding="utf-8")):
            png = entry["png"]
            if png in PRIOR_SLOT_BY_PNG or png in TIER2_SLOT:
                continue
            if entry.get("priority_tier") != 2:
                continue
            src = OUT / png
            if not src.is_file():
                continue
            review_dir.mkdir(parents=True, exist_ok=True)
            dest = review_dir / png
            if not dest.exists():
                shutil.move(str(src), str(dest))
            moved += 1

    write_readmes()
    print(f"Grouped {moved} PNGs under {GROUPS_ROOT}")
    if missing:
        print("Not found in root (may already be grouped):", len(missing))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
