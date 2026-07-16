#!/usr/bin/env python3
"""Import MM Skull Kid SuperBMD DAEs into Blender via localhost:9876."""
from __future__ import annotations

import json
import socket
import sys
from pathlib import Path

HOST = "127.0.0.1"
PORT = 9876

DAE_BODY = Path(
    r"C:\Users\xxxxx\AppData\Roaming\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_29.dae"
)
BLEND_OUT = Path(
    r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend"
)


def blender_call(msg_type: str, params: dict | None = None, timeout: float = 120.0) -> dict:
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
                if len(buf) > 2_000_000:
                    raise


def main() -> int:
    dae = str(DAE_BODY).replace("\\", "\\\\")
    blend = str(BLEND_OUT).replace("\\", "\\\\")
    code = f"""
import bpy
import json
import os

dae = r"{dae}"
blend_out = r"{blend}"

if not os.path.isfile(dae):
    print("ERR|missing dae: " + dae)
else:
    for obj in list(bpy.data.objects):
        if obj.type in ("MESH", "ARMATURE", "EMPTY"):
            bpy.data.objects.remove(obj, do_unlink=True)

    bpy.ops.wm.collada_import(filepath=dae)

    armatures = [o for o in bpy.data.objects if o.type == "ARMATURE"]
    meshes = [o for o in bpy.data.objects if o.type == "MESH"]
    materials = sorted({{m.name for o in meshes for m in o.data.materials if m}})

    info = {{
        "dae": dae,
        "armatures": [a.name for a in armatures],
        "meshes": [m.name for m in meshes],
        "materials": materials,
        "mesh_count": len(meshes),
        "bone_count": len(armatures[0].data.bones) if armatures else 0,
    }}

    os.makedirs(os.path.dirname(blend_out), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=blend_out)
    info["blend"] = blend_out
    print("IMPORT_JSON|" + json.dumps(info))
"""
    print("=== get_scene_info (before) ===")
    print(json.dumps(blender_call("get_scene_info"), indent=2))

    print("=== import body DAE ===")
    resp = blender_call("execute_code", {"code": code})
    print(json.dumps(resp, indent=2))

    inner = resp.get("result", {}).get("result", "")
    if "IMPORT_JSON|" in inner:
        print("=== parsed ===")
        print(inner.split("IMPORT_JSON|", 1)[1].strip())

    print("=== get_scene_info (after) ===")
    print(json.dumps(blender_call("get_scene_info"), indent=2))
    return 0


if __name__ == "__main__":
    sys.exit(main())
