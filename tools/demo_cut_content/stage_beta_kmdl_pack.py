#!/usr/bin/env python3
"""Stage the proven beta-Link Kmdl pack as a Custom Models Layer-A overlay.

Proof input (community restore, already adapted for gameplay skeleton):
  D:/XXXXXXX/beta link/Kmdl.arc

Output (default):
  <out>/Beta Link (Demo Restore)/files/res/Object/Kmdl.arc

Drop/copy that folder into the game's model_replacements/ directory and enable
it under Custom Models. Layer A overlays the whole arc — no ISO repack.

This does NOT re-adapt Demo01_00 BMDs; it mounts the already-working pack.
Use scan_demo_leftovers.py to re-mine raw demo sources when adapting further.
"""
from __future__ import annotations

import argparse
import hashlib
import shutil
import sys
from pathlib import Path


def sha256(path: Path) -> str:
    h = hashlib.sha256()
    with path.open("rb") as f:
        for chunk in iter(lambda: f.read(1 << 20), b""):
            h.update(chunk)
    return h.hexdigest()


def main() -> int:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--beta-kmdl",
        type=Path,
        default=Path(r"D:/XXXXXXX/beta link/Kmdl.arc"),
    )
    ap.add_argument(
        "--out",
        type=Path,
        default=Path(__file__).resolve().parent / "out" / "packs",
    )
    ap.add_argument(
        "--pack-name",
        default="Beta Link (Demo Restore)",
    )
    args = ap.parse_args()

    if not args.beta_kmdl.is_file():
        print(f"ERROR: missing beta Kmdl: {args.beta_kmdl}", file=sys.stderr)
        return 1

    pack_root = args.out / args.pack_name
    dest = pack_root / "files" / "res" / "Object" / "Kmdl.arc"
    dest.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(args.beta_kmdl, dest)

    ini = pack_root / "modinfo.ini"
    if not ini.exists():
        ini.write_text(
            "[mod]\n"
            "name=Beta Link (Demo Restore)\n"
            "author=community restore (adapted from Demo01_00 / Demo04_01)\n"
            "description=Gameplay-adapted beta Hero's Clothes from demo leftovers. "
            "Source proof: D:/XXXXXXX/beta link/Kmdl.arc. "
            "Raw demo BMDs are NOT drop-in — see tools/demo_cut_content/README.md.\n",
            encoding="utf-8",
        )

    digest = sha256(dest)
    print(f"Staged: {dest}")
    print(f"Size:   {dest.stat().st_size}")
    print(f"SHA256: {digest}")
    print(f"Pack:   {pack_root}")
    print("Next: copy pack folder into <config>/model_replacements/ and enable it.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
