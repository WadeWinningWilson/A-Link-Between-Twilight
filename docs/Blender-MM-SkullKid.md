# Blender — MM Skull Kid reskin (v1 textures only)

**Purpose:** Asset-side handoff for retexturing TP `E_PM` (Skull Kid) via Blender **4.2 LTS** + SuperBMD. v1 = **swap PNGs on existing materials** — do **not** move bones, edit mesh topology, or apply object transforms unless you plan to run [`BMD-Reskin-Tool.md`](BMD-Reskin-Tool.md).

**Drive Blender from AI:** [`Blender-Socket-Bridge.md`](Blender-Socket-Bridge.md) (`localhost:9876`, `tools/blender_socket/probe_blender.py`).

**Deploy / mod layout:** [`../companion_mods/MM-SkullKid-Reskin/BUILD.md`](../companion_mods/MM-SkullKid-Reskin/BUILD.md).

---

## Session state (2026-07-16)

| Item | Value |
|------|--------|
| Blender | **4.2.22 LTS** |
| Socket | **127.0.0.1:9876** — Variant A (`get_scene_info` / `execute_code`) verified |
| Working `.blend` | `D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend` |
| **Side-by-side (step 1)** | **BROKEN — see [`MM-SkullKid-SideBySide-HANDOFF.md`](MM-SkullKid-SideBySide-HANDOFF.md)** |
| MM mesh export (broken) | `tools/companion_mod/mm_stk_f3dex2_mesh.py` — do not use until replaced |
| Source DAE (body) | `%AppData%\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_29.dae` |
| **Vanilla PNG baseline** | `...\MM\textures_vanilla\` — **use this for Blender preview** (256×256 head, 64×128 leaf, 16×16 eye) |
| **MM-painted refs** | `...\MM\textures_mm\` — same filenames, same dimensions, MM look painted onto TP UVs |
| Import script | `tools/blender_socket/mm_skullkid_import.py` |
| Fix broken/missing preview | `python tools/blender_socket/mm_skullkid_blender_textures.py` |

**Do not use TP HD GZ2 DDS** as MM Skull Kid reference — that is Twilight Princess art on a different UV layout. An earlier pipeline pass overwrote PNGs with those files and left `m1pm_head` at **0×0** (purple) in Blender.

**Scene after import (body only):**

| Object | Type | Notes |
|--------|------|--------|
| `skeleton_root` | Armature | **27 bones** — keep name for SuperBMD |
| `mesh-0` | Mesh | shape 0 |
| `mesh-1` | Mesh | shape 1 |
| `mesh-2` | Mesh | shape 2 |

**Materials:**

| Blender material | Vanilla texture | PNG (SuperBMD extract) |
|------------------|-----------------|-------------------------|
| `m0SC_pm_eye` | `pm_eye.1` / `.2` / `.3` | `pm_eye.1.png`, `pm_eye.2.png`, `pm_eye.3.png` |
| `m1pm_head` | `pm_tex01` | `pm_tex01.png` |
| `m2pm_leaf` | `pm_leaf01` | `pm_leaf01.png` |

Reference PNGs + JSON: `%AppData%\TwilitRealm\Dusklight\bmd_export\superbmd_verify\`

Lamp / trumpet (optional v1 parts) — separate SuperBMD DAEs in the same folder:

| Part | DAE | Material JSON | Texture |
|------|-----|---------------|---------|
| Lamp 0x1E | `E_PM_30_lamp.dae` | `E_PM_30_lamp_materials.json` | `pm_lamp01.png` |
| Trumpet 0x1F | `E_PM_31_trumpet.dae` | `E_PM_31_trumpet_materials.json` | `pm_trumpet01.png` |

Import each into its own `.blend` (or separate collections) using the same Collada import steps below.

---

## Prerequisites (user)

1. Blender **4.2 LTS** open with **Blender MCP** addon → server **Started** on port **9876**.
2. In-game exports done (`Debug → BMD Export (dev)`) with endian restore (see BUILD.md Step 0).
3. SuperBMD already produced DAE + materials JSON + PNGs (see `superbmd_verify` folder above).
4. MM HD reference textures (Zelda64Recompiled pack) on disk for painting source.

Verify socket:

```powershell
python tools/blender_socket/probe_blender.py
```

Re-import body if the scene is empty:

```powershell
python tools/blender_socket/mm_skullkid_import.py
```

**Side-by-side (step 1 — NOT WORKING):** Previous attempt produced corrupted MM geometry. Read **[`MM-SkullKid-SideBySide-HANDOFF.md`](MM-SkullKid-SideBySide-HANDOFF.md)** before retrying. Do **not** run `mm_skullkid_side_by_side.py` or trust `mesh_export/MM_SkullKid_body.obj`.

---

## v1 texture swap (manual — user-friendly)

> User is Blender-inexperienced: use **Shading** workspace, not weight paint or Edit Mode.

1. Open `MM_SkullKid_v1.blend` (or re-run import script).
2. Select a mesh (`mesh-0` … `mesh-2`) → **Material Properties** (red sphere icon) → pick e.g. `m1pm_head`.
3. Switch workspace to **Shading** (top tabs). Ensure **Material Preview** or **Rendered** so you see textures.
4. In the **Shader Editor**, find the **Image Texture** node wired to **Base Color** (Collada import usually builds Principled BSDF + Image).
5. **Open** → select your new PNG (same UV layout — swap pixels only, do not change UVs for v1).
6. Repeat for `m2pm_leaf`, `m0SC_pm_eye` (eye may use multiple images / material slots — match vanilla naming).
7. **File → Save** before export.

**Do not:**

- Rename `skeleton_root`
- Apply object transforms (Object → Apply) unless you know the SuperBMD rotation recipe
- Enter Edit Mode and move vertices
- Re-parent meshes to a different armature

---

## v1 texture swap (scripted — AI / Claude via socket)

Paste via `execute_code` (see [`Blender-Socket-Bridge.md`](Blender-Socket-Bridge.md)) or save as a Text block in Blender:

```python
import bpy
import os

# Example: point head material at a new PNG
NEW_HEAD = r"D:\path\to\your\skullkid_head.png"  # user supplies MM HD paint

mat = bpy.data.materials.get("m1pm_head")
if mat and mat.use_nodes:
    for node in mat.node_tree.nodes:
        if node.type == "TEX_IMAGE":
            node.image = bpy.data.images.load(NEW_HEAD, check_existing=True)
            print("Updated", mat.name, "->", NEW_HEAD)
            break
```

List image nodes on all Skull Kid materials:

```python
import bpy, json
out = {}
for m in ("m0SC_pm_eye", "m1pm_head", "m2pm_leaf"):
    mat = bpy.data.materials.get(m)
    if not mat or not mat.use_nodes:
        continue
    out[m] = [n.image.name for n in mat.node_tree.nodes if n.type == "TEX_IMAGE" and n.image]
print(json.dumps(out, indent=2))
```

---

## Export Collada (Blender → SuperBMD)

**Known issue (Blender 4.2):** Collada export from an edited blend can drop the head texture from `library_images` and break Assimp. For v1 testing, keep the **SuperBMD-exported** `E_PM_29.dae` as the mesh/skeleton source and only swap PNGs on disk (see pipeline script below).

1. **File → Export → Collada (.dae)** (when export is fixed / validated)
2. Settings (Blender 4.2):
   - **Include → Limit to: Selected Objects** — OFF (export all meshes + armature)
   - **Geometry → Apply Modifiers** — ON
   - **Geometry → Triangulate** — ON (safe for SuperBMD)
   - **Armature** — ON, **Deformed Bones Only** typically OFF (export full rest skeleton)
3. Suggested output:  
   `D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\superbmd_out\body\E_PM_29_edited.dae`
4. Confirm exported armature object is still named **`skeleton_root`**.

Scripted export (socket):

```python
import bpy
out = r"D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\superbmd_out\body\E_PM_29_edited.dae"
bpy.ops.wm.collada_export(filepath=out, apply_modifiers=True, triangulate=True)
print("Exported", out)
```

---

## SuperBMD — DAE back to BMD

SuperBMD: `D:\Extractuibs\Extractions 6.5\ALBW Blender workflow\SuperBMD_2.5.0(1)\SuperBMD.exe`

**Automated (recommended for v1 test):**

```powershell
python tools/blender_socket/mm_skullkid_texture_pipeline.py
```

This converts TP HD reference DDS → PNG, updates the SuperBMD DAE texture folder, patches materials JSON (`pm_tex01:1` → `pm_tex01:0`), builds `E_PM_29.bmd`, and deploys to `model_replacements/MM-SkullKid-Reskin/`.

**Manual body build:**

```powershell
$sb = 'D:\Extractuibs\Extractions 6.5\ALBW Blender workflow\SuperBMD_2.5.0(1)\SuperBMD.exe'
$verify = "$env:APPDATA\TwilitRealm\Dusklight\bmd_export\superbmd_verify"
& $sb "$verify\E_PM_29.dae" '...\superbmd_out\body\E_PM_29.bmd' `
  -m "$verify\E_PM_29_EDITABLE_materials_patched.json" `
  --texfolder $verify
```

Notes:

- **`-m` / `--mat`** = input materials JSON. SuperBMD 2.4.9 instance names use `:0` for the first texture; exported JSON may say `:1` — patch before DAE→BMD (pipeline does this).
- Do **not** use `--outmat` on BMD→DAE (SuperBMD 2.4.9 bug — use default `<stem>_materials.json` beside DAE).
- Reference PNGs for testing: TP HD `GZ2` Skull Kid DDS converted to `pm_*.png` (not MM Nerrel `object_stk` — swap later).

Rename deploy: `E_PM_29.bmd` → `%AppData%\TwilitRealm\Dusklight\model_replacements\MM-SkullKid-Reskin\`

Enable **Editor → ALBW → Custom Models** → Sacred Grove playtest.

---

## Lamp / trumpet (optional)

Same loop per part; use matching materials JSON from `superbmd_verify`:

```powershell
& $sb $env:APPDATA\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_30_lamp.dae `
  '...\MM\superbmd_out\lamp\E_PM_30.bmd' `
  -m $env:APPDATA\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_30_lamp_materials.json
```

(Repeat for trumpet / `E_PM_31.bmd`.)

---

## Troubleshooting

| Symptom | Fix |
|---------|-----|
| Socket timeout | Start MCP server in Blender; `Test-NetConnection 127.0.0.1 -Port 9876` |
| SuperBMD "No Skeleton found" | Armature must be named **`skeleton_root`** |
| White / wrong materials in-game | Re-export with `-m` materials JSON; check `SC_` prefix on cel materials (`m0SC_pm_eye` is OK) |
| Limbs explode when animated | You edited the rig — run [`tools/bmd_reskin/bmd_reskin.py`](../tools/bmd_reskin/bmd_reskin.py) (v1 should avoid this) |
| Engine crash on load | BMD built without materials JSON — always pass `-m` |

---

## Handoff block (paste to Claude / new Cursor chat)

> **MM Skull Kid v1 — textures only.** Blender **4.2.22 LTS**, MCP socket **localhost:9876**. Read **`docs/Blender-Socket-Bridge.md`** + **`docs/Blender-MM-SkullKid.md`**. Working file: **`D:\XXXXXXX\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend`** (body imported: `skeleton_root` + 3 meshes, materials `m0SC_pm_eye` / `m1pm_head` / `m2pm_leaf`). Reference PNGs in **`%AppData%\TwilitRealm\Dusklight\bmd_export\superbmd_verify\`**. Task: swap Image Texture PNGs only → export DAE → SuperBMD with **`-m E_PM_29_EDITABLE_materials.json`** → deploy as **`E_PM_29.bmd`**. User is Blender-inexperienced — prefer **`execute_code`** scripts + Shading workspace instructions. Do not move bones or apply transforms. Lamp/trumpet optional — separate DAEs in same verify folder.

---

## Related

- [`Blender-Socket-Bridge.md`](Blender-Socket-Bridge.md) — socket protocol
- [`BMD-Reskin-Tool.md`](BMD-Reskin-Tool.md) — if rig was edited
- [`Custom-Model-API-Work.md`](Custom-Model-API-Work.md) — Layer B deploy
