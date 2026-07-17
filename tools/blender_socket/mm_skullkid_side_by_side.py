#!/usr/bin/env python3
"""Import TP + MM Skull Kid side-by-side into Blender (collections + materials).

Step 1 of the MM reskin workflow: visual comparison with each game's own UVs/textures.

Prerequisites:
  - Blender 4.2 LTS + MCP socket on localhost:9876
  - TP DAE already exported (E_PM_29.dae)
  - MM mesh OBJ from: python tools/companion_mod/mm_stk_f3dex2_mesh.py

Usage:
  python tools/blender_socket/mm_skullkid_side_by_side.py
  python tools/blender_socket/mm_skullkid_side_by_side.py --no-export  # skip OBJ regen
"""
from __future__ import annotations

import json
import socket
import subprocess
import sys
from pathlib import Path

HOST = "127.0.0.1"
PORT = 9876

TP_DAE = Path(
    r"C:\Users\xxxxx\AppData\Roaming\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_29.dae"
)
BLEND_OUT = Path(
    r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend"
)
MM_MESH_DIR = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\object_stk\mesh_export"
)
MM_OBJ = MM_MESH_DIR / "MM_SkullKid_body.obj"
MM_TEX_N64 = MM_MESH_DIR / "textures_n64"
NERREL_DIR = Path(
    r"c:\Users\xxxxx\Documents\dusklight\companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd\_nerrel_skull_kid"
)
TP_TEX_VANILLA = Path(
    r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\textures_vanilla"
)

# Best-effort DL group -> object_stk OutName (N64 textures in textures_n64/)
DL_TEXTURE_HINTS: dict[str, str] = {
    "gSkullKidTorsoDL": "skull_kid_shawl",
    "gSkullKidPelvisDL": "skull_kid_pants_and_wrists",
    "gSkullKidHatBrimDL": "skull_kid_shawl",
    "gSkullKidHatRingsDL": "skull_kid_ring",
    "gSkullKidHatNarrowSectionDL": "skull_kid_shawl",
    "gSkullKidHatTopDL": "skull_kid_shawl",
    "gSkullKidLeftUpperArmDL": "skull_kid_skin",
    "gSkullKidLeftForearmDL": "skull_kid_skin",
    "gSkullKidLeftHandAndFluteDL": "skull_kid_hand",
    "gSkullKidRightUpperArmDL": "skull_kid_skin",
    "gSkullKidRightForearmDL": "skull_kid_skin",
    "gSkullKidRightHandDL": "skull_kid_hand",
    "gSkullKidLeftThighDL": "skull_kid_pants_and_wrists",
    "gSkullKidLeftShinDL": "skull_kid_pants_and_wrists",
    "gSkullKidLeftFootDL": "skull_kid_shoe",
    "gSkullKidRightThighDL": "skull_kid_pants_and_wrists",
    "gSkullKidRightShinDL": "skull_kid_pants_and_wrists",
    "gSkullKidRightFootDL": "skull_kid_shoe",
    "gSkullKidNormalHeadDL": "skull_kid_head_skin",
    "gSkullKidNormalEyesDL": "skull_kid_eye",
}


def blender_call(msg_type: str, params: dict | None = None, timeout: float = 180.0) -> dict:
    payload: dict = {"type": msg_type}
    if params:
        payload["params"] = params
    data = json.dumps(payload).encode("utf-8")
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.settimeout(timeout)
        s.connect((HOST, PORT))
        s.sendall(data)
        buf = b""
        while True:
            chunk = s.recv(65536)
            if not chunk:
                break
            buf += chunk
            try:
                return json.loads(buf.decode("utf-8"))
            except json.JSONDecodeError:
                if len(buf) > 4_000_000:
                    raise


def run_mesh_export() -> None:
    script = Path(__file__).resolve().parents[1] / "companion_mod" / "mm_stk_f3dex2_mesh.py"
    subprocess.run([sys.executable, str(script)], check=True)


def main() -> int:
    import argparse

    ap = argparse.ArgumentParser()
    ap.add_argument("--no-export", action="store_true", help="Do not regenerate MM OBJ")
    ap.add_argument("--mm-textures", choices=("n64", "nerrel"), default="n64")
    args = ap.parse_args()

    if not args.no_export:
        print("=== Exporting MM mesh from ROM ===")
        run_mesh_export()

    if not MM_OBJ.is_file():
        print("MM OBJ missing:", MM_OBJ, file=sys.stderr)
        return 1

    tex_dir = MM_TEX_N64 if args.mm_textures == "n64" else NERREL_DIR

    dae = str(TP_DAE).replace("\\", "\\\\")
    blend = str(BLEND_OUT).replace("\\", "\\\\")
    mm_obj = str(MM_OBJ).replace("\\", "\\\\")
    tex_dir_s = str(tex_dir).replace("\\", "\\\\")
    tp_tex = str(TP_TEX_VANILLA).replace("\\", "\\\\")
    mm_tex_mode = args.mm_textures

    code = f"""
import bpy
import json
import os

dae = r"{dae}"
mm_obj = r"{mm_obj}"
blend_out = r"{blend}"
mm_tex_dir = r"{tex_dir_s}"
tp_tex_dir = r"{tp_tex}"
mm_tex_mode = "{mm_tex_mode}"

def ensure_collection(name):
    col = bpy.data.collections.get(name)
    if col is None:
        col = bpy.data.collections.new(name)
        bpy.context.scene.collection.children.link(col)
    return col

def move_to_collection(obj, col):
    for c in list(obj.users_collection):
        c.objects.unlink(obj)
    col.objects.link(obj)

def relink_image_nodes(mat, png_path):
    if not mat or not mat.use_nodes or not os.path.isfile(png_path):
        return
    for node in mat.node_tree.nodes:
        if node.type == "TEX_IMAGE":
            node.image = bpy.data.images.load(png_path, check_existing=True)
            node.interpolation = "Closest"
            return

def make_principled_mat(name, png_path):
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
        mat.use_nodes = True
    nt = mat.node_tree
    if not any(n.type == "TEX_IMAGE" for n in nt.nodes):
        nt.nodes.clear()
        out = nt.nodes.new("ShaderNodeOutputMaterial")
        bsdf = nt.nodes.new("ShaderNodeBsdfPrincipled")
        tex = nt.nodes.new("ShaderNodeTexImage")
        tex.interpolation = "Closest"
        nt.links.new(tex.outputs["Color"], bsdf.inputs["Base Color"])
        nt.links.new(bsdf.outputs["BSDF"], out.inputs["Surface"])
    relink_image_nodes(mat, png_path)
    return mat

root_col = ensure_collection("SkullKid_Compare")
tp_col = ensure_collection("TP_SkullKid")
mm_col = ensure_collection("MM_SkullKid")
if tp_col.name not in [c.name for c in root_col.children]:
    root_col.children.link(tp_col)
if mm_col.name not in [c.name for c in root_col.children]:
    root_col.children.link(mm_col)

# --- TP: reuse existing E_PM import if present ---
tp_col = ensure_collection("TP_SkullKid")
existing_tp = [
    o
    for o in bpy.data.objects
    if o.name in ("skeleton_root", "mesh-0", "mesh-1", "mesh-2")
    or o.name.startswith(("skeleton_root.", "mesh-0.", "mesh-1.", "mesh-2."))
]
if existing_tp:
    for obj in existing_tp:
        move_to_collection(obj, tp_col)
elif os.path.isfile(dae):
    bpy.ops.wm.collada_import(filepath=dae)
    for obj in bpy.context.selected_objects:
        move_to_collection(obj, tp_col)
for obj in tp_col.objects:
    obj.location.x = -1.5

for mat_name, png_name in (
    ("m1pm_head", "pm_tex01.png"),
    ("m2pm_leaf", "pm_leaf01.png"),
    ("m0SC_pm_eye", "pm_eye.1.png"),
):
    relink_image_nodes(
        bpy.data.materials.get(mat_name),
        os.path.join(tp_tex_dir, png_name),
    )

# --- MM OBJ ---
for obj in list(mm_col.objects):
    bpy.data.objects.remove(obj, do_unlink=True)

if os.path.isfile(mm_obj):
    before = set(bpy.data.objects)
    bpy.ops.wm.obj_import(filepath=mm_obj)
    imported = [o for o in bpy.data.objects if o not in before]
    for obj in imported:
        move_to_collection(obj, mm_col)
        obj.location.x = 1.5
        obj.rotation_euler[0] = 1.5708
        if obj.type != "MESH":
            continue
        for slot in obj.material_slots:
            mat = slot.material
            if mat is None:
                continue
            out_name = mat.name
            if mm_tex_mode == "nerrel":
                png_path = ""
                for fn in os.listdir(mm_tex_dir):
                    if fn.lower().endswith(".png") and out_name.replace("_", "")[:6] in fn.replace("_", "").lower():
                        png_path = os.path.join(mm_tex_dir, fn)
                        break
            else:
                png_path = os.path.join(mm_tex_dir, out_name + ".png")
            if png_path and os.path.isfile(png_path):
                make_principled_mat(mat.name, png_path)

info = {{
    "blend": blend_out,
    "tp_meshes": [o.name for o in tp_col.objects if o.type == "MESH"],
    "tp_armatures": [o.name for o in tp_col.objects if o.type == "ARMATURE"],
    "mm_meshes": [o.name for o in mm_col.objects if o.type == "MESH"],
    "mm_materials": sorted({{s.material.name for o in mm_col.objects if o.type == "MESH" for s in o.material_slots if s.material}}),
    "mm_texture_mode": mm_tex_mode,
}}
os.makedirs(os.path.dirname(blend_out), exist_ok=True)
bpy.ops.wm.save_as_mainfile(filepath=blend_out)
print("SIDE_BY_SIDE_JSON|" + json.dumps(info))
"""

    print("=== Probing Blender socket ===")
    try:
        print(json.dumps(blender_call("get_scene_info"), indent=2))
    except OSError as e:
        print("Blender socket not available:", e, file=sys.stderr)
        print("Open Blender 4.2, start MCP server on port 9876, then re-run.")
        return 1

    print("=== Side-by-side import ===")
    resp = blender_call("execute_code", {"code": code})
    print(json.dumps(resp, indent=2))
    inner = resp.get("result", {}).get("result", "")
    if "SIDE_BY_SIDE_JSON|" in inner:
        print("=== Result ===")
        print(inner.split("SIDE_BY_SIDE_JSON|", 1)[1].strip())
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
