#!/usr/bin/env python3
"""Relink MM Skull Kid Blender materials to PNGs on disk (fixes 0x0 / purple previews).

Default: vanilla SuperBMD extracts (correct TP UV baseline).
Pass --mm when MM reference PNGs exist in textures_mm/.
"""
from __future__ import annotations

import argparse
import json
import socket
import sys
from pathlib import Path

HOST, PORT = "127.0.0.1", 9876
ROOT = Path(r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM")
BLEND = ROOT / "MM_SkullKid_v1.blend"
VANILLA = ROOT / "textures_vanilla"
MM_REF = ROOT / "textures_mm"

MAT_MAP = {
    "m1pm_head": "pm_tex01.png",
    "m2pm_leaf": "pm_leaf01.png",
    "m0SC_pm_eye": "pm_eye.1.png",
}


def blender_call(code: str) -> str:
    payload = {"type": "execute_code", "params": {"code": code}}
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(120)
        s.connect((HOST, PORT))
        s.sendall(json.dumps(payload).encode())
        buf = b""
        while True:
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
            try:
                resp = json.loads(buf.decode())
                break
            except json.JSONDecodeError:
                pass
    return resp.get("result", {}).get("result", "")


def apply_textures(tex_dir: Path) -> None:
    tex_dir_s = str(tex_dir).replace("\\", "\\\\")
    blend_s = str(BLEND).replace("\\", "\\\\")
    mat_map = json.dumps(MAT_MAP)
    code = f"""
import bpy, json, os

mat_map = json.loads('''{mat_map}''')
tex_dir = r"{tex_dir_s}"
blend = r"{blend_s}"

if bpy.data.filepath != blend and os.path.isfile(blend):
    bpy.ops.wm.open_mainfile(filepath=blend)

def relink(mat_name, png_name):
    mat = bpy.data.materials.get(mat_name)
    if not mat or not mat.use_nodes:
        return {{"ok": False, "reason": "no material/nodes"}}
    path = os.path.join(tex_dir, png_name)
    if not os.path.isfile(path):
        return {{"ok": False, "reason": "missing file", "path": path}}
    # Drop stale datablocks (e.g. pm_tex01 at 0x0 after file swap).
    for img in list(bpy.data.images):
        if img.name in (png_name, png_name + ".001", os.path.splitext(png_name)[0] + ".png.001"):
            bpy.data.images.remove(img)
    img = bpy.data.images.load(path, check_existing=False)
    img.name = png_name
    hooked = False
    for node in mat.node_tree.nodes:
        if node.type == "TEX_IMAGE":
            node.image = img
            hooked = True
    img.reload()
    return {{"ok": hooked, "size": list(img.size), "path": path}}

report = {{k: relink(k, v) for k, v in mat_map.items()}}
bpy.ops.wm.save_as_mainfile(filepath=blend)
print("RELINK|" + json.dumps(report))
"""
    out = blender_call(code)
    print(out)
    if "RELINK|" not in out:
        raise RuntimeError("Blender relink failed — is MCP server running on 9876?")


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--mm", action="store_true", help="use textures_mm/ instead of textures_vanilla/")
    args = ap.parse_args()
    tex_dir = MM_REF if args.mm else VANILLA
    if not tex_dir.is_dir():
        print(f"Missing texture folder: {tex_dir}", file=sys.stderr)
        return 1
    missing = [v for v in MAT_MAP.values() if not (tex_dir / v).is_file()]
    if missing:
        print(f"Missing PNGs in {tex_dir}: {missing}", file=sys.stderr)
        return 1
    print(f"Relinking Blender materials from {tex_dir}")
    apply_textures(tex_dir)
    return 0


if __name__ == "__main__":
    sys.exit(main())
