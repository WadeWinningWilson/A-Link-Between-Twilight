#!/usr/bin/env python3
"""MM Skull Kid v1 texture pipeline.

Blender preview: vanilla TP UV baseline (textures_vanilla/) or MM-painted refs
(textures_mm/). In-game deploy uses bmd_retexture (swap TEX1 payloads only) —
NOT a full SuperBMD BMD rebuild.
"""
from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

ROOT = Path(r"<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM")
TEX_VANILLA = ROOT / "textures_vanilla"
TEX_MM = ROOT / "textures_mm"
VERIFY = Path(r"%USERPROFILE%\AppData\Roaming\TwilitRealm\Dusklight\bmd_export\superbmd_verify")
DEPLOY = Path(
    r"%USERPROFILE%\AppData\Roaming\TwilitRealm\Dusklight\model_replacements\MM-SkullKid-Reskin"
)
MODINFO = Path(r"%USERPROFILE%\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\modinfo.ini")
SRC_BMD = Path(
    r"%USERPROFILE%\AppData\Roaming\TwilitRealm\Dusklight\bmd_export\E_PM_29_EDITABLE.bmd"
)
RETEXTURE = Path(__file__).resolve().parents[1] / "companion_mod" / "bmd_retexture.py"
BLENDER_TEX = Path(__file__).resolve().parent / "mm_skullkid_blender_textures.py"
SUPERBMD = Path(
    r"D:\Extractuibs\Extractions 6.5\ALBW Blender workflow\SuperBMD_2.5.0(1)\SuperBMD.exe"
)

SLOTS = ("pm_tex01.png", "pm_leaf01.png", "pm_eye.1.png")


def extract_vanilla_pngs() -> None:
    TEX_VANILLA.mkdir(parents=True, exist_ok=True)
    dae = TEX_VANILLA / "E_PM_29.dae"
    cmd = [str(SUPERBMD), str(SRC_BMD), str(dae), "--texfolder", str(TEX_VANILLA)]
    proc = subprocess.run(cmd, capture_output=True, text=True)
    if proc.returncode != 0:
        raise RuntimeError("SuperBMD vanilla extract failed")
    for name in SLOTS:
        src = TEX_VANILLA / name
        if src.is_file():
            shutil.copy2(src, VERIFY / name)
        else:
            raise FileNotFoundError(src)


def relink_blender(use_mm: bool) -> None:
    args = [sys.executable, str(BLENDER_TEX)]
    if use_mm:
        args.append("--mm")
    subprocess.run(args, check=True)


def deploy_retexture(tex_dir: Path) -> None:
    mapping = []
    for png in SLOTS:
        slot = png.replace(".png", "")
        mapping.append(f"{slot}={tex_dir / png}")
    cmd = [
        sys.executable,
        str(RETEXTURE),
        "--src",
        str(SRC_BMD),
        "--dst",
        str(DEPLOY / "E_PM_29.bmd"),
        "--map",
        *mapping,
    ]
    subprocess.run(cmd, check=True)
    DEPLOY.mkdir(parents=True, exist_ok=True)
    if MODINFO.is_file():
        shutil.copy2(MODINFO, DEPLOY / "modinfo.ini")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--mm", action="store_true", help="use textures_mm/ (MM-painted refs)")
    ap.add_argument("--blender-only", action="store_true")
    ap.add_argument("--deploy", action="store_true", help="write E_PM_29.bmd via bmd_retexture")
    args = ap.parse_args()

    tex_dir = TEX_MM if args.mm else TEX_VANILLA
    if not tex_dir.is_dir() or not all((tex_dir / n).is_file() for n in SLOTS):
        print("=== Extract vanilla TP UV baseline ===")
        extract_vanilla_pngs()
        if not args.mm:
            tex_dir = TEX_VANILLA

    print(f"=== Blender preview from {tex_dir.name}/ ===")
    try:
        relink_blender(args.mm)
    except subprocess.CalledProcessError as exc:
        print("Blender relink failed (is MCP on 9876?):", exc)

    if args.blender_only:
        return 0

    if args.deploy:
        if args.mm:
            print("=== Deploy retexture (MM PNGs) ===")
            deploy_retexture(tex_dir)
        else:
            print("Skip deploy for vanilla baseline (nothing to change in-game).")
        return 0

    print("Done. Paint MM refs into textures_mm/ at vanilla sizes, then:")
    print("  python tools/blender_socket/mm_skullkid_texture_pipeline.py --mm")
    print("  python tools/blender_socket/mm_skullkid_texture_pipeline.py --mm --deploy")
    return 0


if __name__ == "__main__":
    sys.exit(main())
