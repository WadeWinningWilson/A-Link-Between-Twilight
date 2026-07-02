# Blender WW Items — asset re-rig / extraction handoff

**Purpose:** Dedicated guide + working doc for a **Blender-focused chat** assisting the Dusklight (Twilight Princess) mod's **Wind Waker item** work. The in-game code can't add skeleton joints or split baked geometry — those are **external asset tasks**. This doc covers extracting, re-rigging, and repacking WW `itemmdl` meshes so they work as Link skins.

> **⚠️ READ FIRST — who you're helping:** the user is **familiar with Blender but VERY inexperienced.** Explain every step concretely — *where* the panel/button is, the exact clicks, what a term means (armature, weight paint, edit mode). Do **not** assume they know the Python console, rigging, or the outliner. Prefer giving **ready-to-run `bpy` scripts** (paste into the Scripting tab) for deterministic steps, and slow click-by-click for the manual ones. When in doubt, over-explain.

**Related (game-side) docs — for context, not required reading:** [`wind-waker-item-work.md`](wind-waker-item-work.md) (the mod's item pipeline, all 21 meshes, indices) and [`Interconnected Chats/Wind Curs-Wind Clau.md`](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) (implementation log). **This doc is the ASSET side.**

---

## Environment

- **Blender:** user has **5.x**. Workflow is identical to older versions, **BUT the Collada (`.dae`) importer/exporter** — which the BMD converter needs — has been deprioritized in new Blender:
  - Enable it: **Edit → Preferences → Add-ons → search "Collada" →** tick Import-Export.
  - If 5.x's Collada is flaky (a common modding complaint), install **Blender 3.6 LTS** *alongside* 5.x and use it **only as the DAE import/export bridge** (convert there, edit wherever). SuperBMD only cares about the `.dae`, not which Blender wrote it.
  - **Do not** rely on direct "BMD importer" Blender add-ons — they lag major versions and likely don't support 5.x. Use standalone **SuperBMD** for the BMD↔DAE step (version-independent).
- **Tools:**
  - **SuperBMD** — command-line `BMD/BDL ↔ DAE`. The core converter.
  - **GCFT** (GameCube File Tools) — unpack/repack `.arc` (RARC) archives.
  - **BMDView2** — quick visual check of a `.bmd`/`.bdl`'s joints, materials, weights (sanity-check before/after).
- **AI in Blender (recommended):** the open-source **`blender-mcp`** server connects Claude directly to a running Blender instance (inspect the scene + run `bpy`). If the user sets it up, the Blender chat can **drive operations directly** instead of only describing them — highest leverage. Otherwise, hand the user `bpy` scripts to paste. (Note the honest limit: AI is great at scriptable/repeatable steps — conversion, bone reindex, weight transfer, export — and weak at fine weight-painting. For rigid iron boots that barely flex, that weakness barely matters.)

**The loop:** `unpack arc → SuperBMD BMD→DAE → edit in Blender → SuperBMD DAE→BMD → repack arc → drop in res/Object/ → test in the in-game Item Viewer.` The mod's **Item Viewer** (Editor → ALBW → Wind Waker Item Viewer) spins any `itemmdl` index instantly, so it's the fast iteration surface.

---

## Goals (priority order)

1. **Iron Boots re-rig (ACTIVE BLOCKER)** — make `vboot` work as a worn boot on Link's feet.
2. **Bow: extract the WW arrow** from `vbow` → a standalone mesh (to skin TP's arrow projectile).
3. *(Later)* Re-rig held items for real flex; extract **King Bulblin axe** (`RB_ONO` from `E_rdb`) to a light arc.

---

## Key technical facts (the Blender chat MUST know these)

- **`vboot`** (WW iron boots, `itemmdl` BDL index **0xE**): **2 joints** — `ROOT_VBOOT`(0) + `VBOOT_MODEL`(1). It is a **standing PAIR of boots in one model**, weighted to the single `VBOOT_MODEL` joint, **no foot rig**. Origin sits at the pair's ground.
- **`al_bootsH`** (TP's iron boots, inside Link's clothes arc): a **SINGLE boot** mesh that the game **instances twice** (Left + Right). The game drives each instance's **joints 1 / 2 / 3** from Link's foot bones every frame:
  - **Left boot** ← Link bones `0x13 / 0x14 / 0x15` (ankle / foot / toe)
  - **Right boot** ← Link bones `0x18 / 0x19 / 0x1A`
  - Because it attaches to **bones**, it works on ANY Link body (native, **sumo**, etc.) with zero per-outfit code — the bones exist on all of them. It does **not** contain the legs (those are Link's body model); it's the boot + shin cup.
- **THE GAME ADDRESSES JOINTS BY INDEX** (`setAnmMtx(1,2,3)`), not by name. So a re-rigged `vboot` must match `al_bootsH`'s **joint index ORDER**: index 1 = ankle, 2 = foot, 3 = toe. Names help humans; **indices are what must line up.**
- **`vbow`** (WW bow, BDL **0xF**): **2 shapes / 2 materials / 2 joints**. `Vbow_v` = body + gold arrow **shaft**; `SC_Vbow_v` = silver arrow **head + fletching + ink outline**. The **nocked arrow's geometry is split across BOTH shapes** — that's why it can't be separated in code.
- **Material naming convention:** body material `<Name>_v`, second-pass/ink material `SC_<Name>_v`. The mod's renderer auto-applies its cel recipe to any **`SC_`-prefixed** material. **Keep these material names** on any edited/new mesh so it colors correctly in-game.
- Handy indices (from `itemmdl.h`): `vboot`=0xE, `vbow`=0xF, `vhook`=0x14 (hookshot), `vhamm`=0x12 (skull hammer). Full list in `wind-waker-item-work.md`.

---

## Task 1 — Iron Boots re-rig  ← the current blocker

### Where it stands (the struggle)
The mod already loads `vboot` onto Link's boot slot, but `vboot` is a **standing pair with no foot rig**, so:
- Putting it on *both* boot instances = **two pairs (four boots)**.
- Pinning it to the ankle bone floated it up to **hip/knee** and rotated it **~90° / facing sideways** (the ankle bone carries the foot's local rotation).
- The current code workaround draws **one** pair at Link's base transform — upright and single now, but **too low on Y** and static (won't follow each foot).

### The fix (in Blender)
Turn `vboot` into a **SINGLE boot** rigged to **match `al_bootsH` exactly** (same bone count, order, and index positions). Then the game's existing two-instance foot rig drives it per-foot, at the right height, articulated — and the mod deletes its special-case code. **`al_bootsH` is your reference rig; copy its skeleton.**

### Steps
1. **Extract meshes** (GCFT):
   - Unpack `itemmdl.arc` → get `vboot.bdl`.
   - Find `al_bootsH.bmd` in Link's clothes/model arc (search the extracted `Object/` tree for the filename) → unpack that arc.
2. **Convert to DAE** (SuperBMD, command line):
   - `SuperBMD.exe vboot.bdl vboot.dae`
   - `SuperBMD.exe al_bootsH.bmd al_bootsH.dae`
3. **Import both** into Blender: **File → Import → Collada (.dae)**. Do vboot and al_bootsH.
4. **Compare the skeletons** — select each armature and run **Script 1** (below). Note `al_bootsH`'s bone names, order, and hierarchy — that's the **target**.
5. **Split vboot's pair** into two boots — select the vboot mesh, run **Script 3** (separate by loose parts), then delete one boot half (keep a single boot). *(If the pair is one connected mesh, do it manually in Edit Mode: hover a boot, press `L` to select linked, `P → Selection` to separate.)*
6. **Re-rig the single boot to `al_bootsH`'s skeleton:**
   - Delete vboot's own armature.
   - Select the vboot boot mesh, then shift-select `al_bootsH`'s armature, **Ctrl+P → Armature Deform → With Automatic Weights**.
   - Clean up weights if needed (toe verts → toe bone, etc.) in Weight Paint mode.
7. **Match position/scale** — move/scale the vboot boot so it sits exactly where `al_bootsH`'s boot sits (use `al_bootsH` as an on-screen reference). This is what fixes the Y-height at the source.
8. **Export** the vboot boot + its (al_bootsH-matching) armature: run **Script 4** (or File → Export → Collada, "Selection Only"). Then `SuperBMD.exe vboot_new.dae vboot.bdl`.
9. **Repack** into `itemmdl.arc` (GCFT), drop in `res/Object/`.
10. **Game-side (tell the code chat):** if the bones now match `al_bootsH`, the mod can **delete the `s_albwWwBootsSkinned` special-case** in `d_a_alink.cpp` — the vanilla `setAnmMtx(1/2/3)` rig then drives the re-rigged vboot per-foot, correct height, articulated.

**Verify before repacking (BMDView2):** joint **index order** matches `al_bootsH` (index 1 = ankle…); the boot's forward axis matches; weights look sane.

---

## Task 2 — Extract the WW arrow from the bow

**Why:** the WW bow's nocked arrow is baked into `vbow`'s two shapes (`Vbow_v` shaft + `SC_Vbow_v` head/fletch), so the game can't move "just the arrow" onto TP's arrow projectile. Blender can isolate it.

**Steps:** convert `vbow.bdl → vbow.dae`, import; in Edit Mode select only the **arrow** geometry (shaft from `Vbow_v`, head+fletch from `SC_Vbow_v` — use material-based selection: in Edit Mode, in the Material panel pick a material and **Select**), separate (`P`), join the two arrow parts into one mesh, keep the materials (`SC_` naming), export `warrow.bmd`. The mod then skins TP's arrow actor with it.

---

## `bpy` scripts — paste into Blender's Scripting tab

**How to run a script (for the inexperienced user):**
1. Top of the Blender window, click the **"Scripting"** workspace tab.
2. In the text editor panel, click **New**.
3. Paste the script, then press the **▶ (Run Script)** button (or `Alt+P` with the cursor over the editor).
4. See output in the **System Console**: **Window → Toggle System Console** (open it *before* running).

> Note: `bpy` operator names occasionally change between Blender versions. If a line errors, tell the Blender chat the exact error and it'll adjust. These target Blender 4.x/5.x with Collada enabled.

**Script 1 — list an armature's bones + order (for comparing vboot vs al_bootsH):**
```python
import bpy
arm = bpy.context.object   # click the ARMATURE in the outliner first
if arm and arm.type == 'ARMATURE':
    print("=== Armature:", arm.name, "===")
    for i, b in enumerate(arm.data.bones):
        parent = b.parent.name if b.parent else "(root)"
        print(f"[{i}] {b.name}  parent={parent}")
else:
    print("Select an ARMATURE object first (click it in the Outliner).")
```

**Script 2 — rename bones to match a target (edit the mapping):**
```python
import bpy
arm = bpy.context.object
rename = {                 # OLD name : NEW name  — set these to match al_bootsH
    "VBOOT_MODEL": "foot",
    # "ROOT_VBOOT": "root",
}
if arm and arm.type == 'ARMATURE':
    for b in arm.data.bones:
        if b.name in rename:
            b.name = rename[b.name]
    print("Renamed:", rename)
else:
    print("Select the ARMATURE first.")
```

**Script 3 — separate a mesh into loose parts (split the boot pair):**
```python
import bpy
# Select the vboot MESH object first, then run.
bpy.ops.object.mode_set(mode='EDIT')
bpy.ops.mesh.select_all(action='SELECT')
bpy.ops.mesh.separate(type='LOOSE')
bpy.ops.object.mode_set(mode='OBJECT')
print("Separated into:", [o.name for o in bpy.context.selected_objects])
```

**Script 4 — export the selected objects to DAE:**
```python
import bpy
# Select the mesh(es) + armature you want to export first.
out = r"C:\Users\xxxxx\Documents\ww_export\vboot_new.dae"   # <-- edit path (folder must exist)
bpy.ops.wm.collada_export(filepath=out, selected=True, apply_modifiers=True,
                          include_armatures=True)
print("Exported ->", out)
```

---

## Fresh Blender-chat opener (paste)

> You are assisting with Blender asset work for a Twilight Princess mod (Dusklight) that reuses Wind Waker item meshes. **I'm familiar with Blender but very inexperienced — explain every step concretely (where panels/buttons are), and give me ready-to-run scripts for the Scripting tab.** Read `docs/Blender-WW-Items.md` first — it has the goals, the tools (SuperBMD, Blender 5.x + Collada caveat, GCFT), the key facts (vboot = 2-joint standing pair; al_bootsH = single boot rigged to foot bones 0x13–0x1A at joint indices 1/2/3; game addresses joints by index), and the current blocker. Start with **Task 1 — Iron Boots re-rig**: help me extract `vboot` + `al_bootsH`, compare their skeletons, and re-rig vboot into a single boot matching al_bootsH's joint layout. If I set up `blender-mcp`, drive Blender directly.

---

## Status / handoff notes

- **Game side is done & committed** for bow + hookshot held skins and the get-item viewer (see the two related docs). The **iron-boots game code is on hold** (a stiff single-pair swap exists, uncommitted / too low on Y) pending this re-rigged `vboot` asset.
- Once the re-rigged `vboot.bdl` exists, the code chat removes the `s_albwWwBootsSkinned` special-case and the vanilla boot rig drives it.
- Keep `SC_`-prefixed material names on any edited mesh so the in-game cel renderer applies automatically.
