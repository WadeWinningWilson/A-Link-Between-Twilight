# Blender WW Items — asset re-rig / extraction handoff

**Purpose:** Dedicated guide + working doc for a **Blender-focused chat** assisting the Dusklight (Twilight Princess) mod's **Wind Waker item** work. The in-game code can't add skeleton joints or split baked geometry — those are **external asset tasks**. This doc covers extracting, re-rigging, and repacking WW `itemmdl` meshes so they work as Link skins.

> **⚠️ READ FIRST — who you're helping:** the user is **familiar with Blender but VERY inexperienced.** Explain every step concretely — *where* the panel/button is, the exact clicks, what a term means (armature, weight paint, edit mode). Do **not** assume they know the Python console, rigging, or the outliner. Prefer giving **ready-to-run `bpy` scripts** (paste into the Scripting tab) for deterministic steps, and slow click-by-click for the manual ones. When in doubt, over-explain.

**Related (game-side) docs — for context, not required reading:** [`wind-waker-item-work.md`](wind-waker-item-work.md) (the mod's item pipeline, all 21 meshes, indices) and [`Interconnected Chats/Wind Curs-Wind Clau.md`](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) (implementation log). **This doc is the ASSET side.**

---

## ▶ NEW CHAT — START HERE (2026-07-08): WW Iron Boots — geometry re-rig (Option B)

**Where we are:** The WW iron boots now **load, don't crash, and render in full color** on Link's feet in-game. The ONLY remaining problem is **geometry**: worn, the boot mesh **explodes into scattered shards** — even though it looks perfect standalone in the get-item viewer. This is a **skinning/weighting bug in the re-rig ASSET.** The game side is solved and must not be touched.

**Your job (asset-only, Blender + SuperBMD — NO game code):** redo the `vboot` re-rig so its skin EXACTLY matches `al_bootsH`'s structure, then redeploy the BMD. **No game rebuild** — Layer-B loads loose BMDs at runtime.

**Why it mangles (root cause, confirmed by diffing the two DAEs):** the game drives boot joints 1/2/3 → Link's ankle/foot/toe (`setAnmMtx(1,2,3)` ← bones `0x13/0x14/0x15`) and pins joint 0 (`world_root`) at Link's base. The deployed re-rig does NOT match `al_bootsH`:

| | `al_bootsH` (TARGET) | current re-rig (BROKEN) |
|---|---|---|
| mesh | **3 separate rigid pieces** | 1 single mesh |
| skin per piece | each piece → **exactly 1 joint** (HA=1 / HB=2 / HC=3), `count="1"` | one mesh weighted to **4 joints incl `world_root`**, `count="4"` |
| `bind_shape_matrix` | identity | **+90° X** (`0 7.5e-8 1 / 0 -1 7.5e-8`) — Blender Z-up leak |
| node above `world_root` | identity | **−90° X** (Blender armature) |

⇒ verts weighted to `world_root` stay up at Link's waist while the HA/HB/HC verts fly down to the foot bones → radial explosion; the +90° `bind_shape` skew rotates the mesh relative to the joints.

**Option B spec — replicate `al_bootsH` exactly:**
1. **3 rigid segments** — each mesh region bound **100% to ONE joint**, matching al_bootsH's ankle/foot/toe split (region → HA=1 / HB=2 / HC=3). Reuse al_bootsH's own vertex→segment assignment as the map (nearest-segment / KDTree region-match — see the "Weighting = region-match" discovery in Task 1). **Single influence per vertex, no blending.**
2. **NO `world_root` (joint 0) weighting** — al_bootsH weights nothing to it.
3. **Identity `bind_shape_matrix`** — bake all transforms (`transform_apply`) so the mesh is in al_bootsH's Y-up space with no residual ±90°. Verify the exported DAE's `<bind_shape_matrix>` is identity AND the node above `world_root` is identity (compare against `al_bootsh.dae`).
4. Keep joint **index order** 1/2/3 = ankle/foot/toe and material names `SC_boot` / `boot`.

**CRITICAL — convert without re-crashing the game (these bit us this session):**
- Build the BMD **WITH** `-m vboot_materials.json -x vboot_tex_headers.json` (both in `…\DAE files\`). **Building WITHOUT `-m` yields a degenerate MAT3 whose material node[0] is NULL → the engine NULL-crashes on load** (`mDoExt_J3DModel__create` derefs `getMaterialNodePointer(0)`). The material JSON is mandatory. **Export `.bmd`, not `.bdl`** for the Layer-B loose file.
- Textures: the DAE's `library_images` must list all three PNGs (`V_boot`, `ZAtoon`, `V_hamm_spc`) — SuperBMD only embeds textures referenced there. (`ZAtoon`/`V_hamm_spc` are WW-shared toon/spec maps, not in the original `vboot.bmd`; they were added to `vboot_new.dae`.) Confirm with `Mapped V_boot→0 / ZAtoon→1 / V_hamm_spc→3` in the convert output.
- **Deploy (no game rebuild):** copy the one BMD to **four** names in `%AppData%\Roaming\TwilitRealm\Dusklight\model_replacements\Wind Waker Skins\`: `Kmdl_13.bmd`, `Bmdl_12.bmd`, `Zmdl_12.bmd`, `Mmdl_13.bmd`. Relaunch → enable **Wind Waker Skins** → equip heavy boots → **walk through a room transition** (the reuse path the heap fix protects; must not crash).

**Game side is DONE — do NOT modify (uncommitted at time of writing):**
- `custom_assets.cpp` — **GameHeap pin** around `loaderBasicBmd` in `try_load` (`mDoExt_getGameHeap()`): the loose model's `mMaterialNodePointer` array must live on a heap that survives Link's clothes-heap `mpArcHeap->freeAll()` each rebuild, or it dangles → NULL-crash on reuse. **This is the core Layer-B lifetime fix for ALL custom models.** Plus a material-node validation guard (rejects a bad BMD → vanilla fallback instead of crash).
- `d_a_alink.cpp` + `d_a_alink_wolf.inc` — `s_albwWwBootsSkinned` set true when `try_load` returns the WW boot, gating a **get-item cel-lighting branch** in `modelDraw` (`settingTevStruct(0)` + `setWwBowActorAmbient` + `applyBowMaterialAmbientOnly`, no MAJI). Uses the normal `modelEntryDL` — deliberately NOT the held-bow `modelUpdateDL` + SC scope, which corrupts the two boots' **shared** model data.

**Cross-refs:** game-side heap/crash detail → [`Custom-Model-API-Work.md`](Custom-Model-API-Work.md); item pipeline + the get-item lighting recipe this borrowed → [`wind-waker-item-work.md`](wind-waker-item-work.md) (Layer A/B PROVEN PATH); implementation log → [`Interconnected Chats/Wind Curs-Wind Clau.md`](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md).

**Files:** blend `<decomp-root>\Ex TP\Blender workflow\vboot_rerig_DONE.blend`; DAEs + `vboot_materials.json` + `vboot_tex_headers.json` + the 3 PNGs in `…\DAE files\`; the current (broken) deployed BMD was `vboot_new.dae` → `vboot_tex.bmd`. Compare against `al_bootsh.dae` / `al_bootsh_hierarchy.json` (the target structure).

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

1. **Iron Boots re-rig — ✅ ASSET DONE (2026-07-02).** `vboot` re-rigged to al_bootsh's exact skeleton; textured `vboot.bdl` built & validated. See [Task 1 — DONE](#task-1--iron-boots-re-rig---the-current-blocker) below.
   - **⚠️ DELIVERY = Custom Model API, Layer B (game hook DONE 2026-07-06).** The old code-injection swap is removed; the boot load now calls `custom_assets::try_load(mArcName, al_bootsH_index)` with a vanilla fallback, driven by the **vanilla foot rig** (zero special-casing). See [`Custom-Model-API-Work.md`](Custom-Model-API-Work.md). **What's left is 100% asset-side:**
     1. **Export the re-rig as a `.bmd`** (SuperBMD, *not* `.bdl`), matching `al_bootsH`'s materials/textures. Verify in BMDView2: **4 joints** `WORLD_ROOT`(0)/`AL_BOOTSHA`(1)/`B`(2)/`C`(3), 2 mats, **clean material blocks** (a corrupt build makes Link skip — no crash, but no boots).
     2. **Copy it under 4 names** into `%AppData%/TwilitRealm/Dusklight/model_replacements/Wind Waker Skins/` (`al_bootsH` is the same mesh+skeleton in every clothes arc, so one BMD → four names):
        - `Kmdl_13.bmd` (default green / Hero) · `Bmdl_12.bmd` (casual) · `Zmdl_12.bmd` (Zora armor) · `Mmdl_13.bmd` (Magic armor)
     3. **Enable "Wind Waker Skins"** in Editor → ALBW → Custom Models, wear an outfit, equip heavy boots → WW boots on the feet via the vanilla rig. (Applies on the next clothes rebuild — outfit switch / area reload.)
2. **Bow: extract the WW arrow** from `vbow` → a standalone mesh (to skin TP's arrow projectile).
3. *(Later)* Re-rig held items for real flex; extract **King Bulblin axe** (`RB_ONO` from `E_rdb`) to a light arc.

> **Design intent — TP↔WW toggle.** The whole reason to match al_bootsh's skeleton exactly (joints/indices 1/2/3) is so the re-rigged `vboot` is a **drop-in** the boot slot can load *instead of* al_bootsh, driven by the **same** vanilla foot rig. That makes a clean **player toggle: Iron Boots (TP) ↔ Iron Boots (WW)** — a pure model-swap, like the Cap Wear selector. The old stiff single-joint hack couldn't toggle cleanly; this can.

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

> **⚠️ STATUS CORRECTION (2026-07-08): the re-rig below is NOT correct.** It loads/colors/doesn't-crash in-game, but the skin **mangles when worn** (explodes into shards) because it does not match `al_bootsH`'s structure — a single 4-joint blended mesh with a +90° `bind_shape` and `world_root` weighting, vs al_bootsH's 3 rigid single-joint pieces. **The real next step is the [▶ NEW CHAT — START HERE](#-new-chat--start-here-2026-07-08-ww-iron-boots--geometry-re-rig-option-b) block at the top (Option B).** The section below is retained as the build history + the correct "Weighting = region-match" insight to reuse.

### ⚠️ Asset built 2026-07-02 — but skin structure is WRONG (see status correction above)

**Deliverable:** `<decomp-root>\Ex TP\Blender workflow\DAE files\vboot.bdl` (re-rigged, textured, BDL format). **Blend:** `<decomp-root>\Ex TP\Blender workflow\vboot_rerig_DONE.blend`. Validated by round-trip: **4 joints** (0=root, 1/2/3 = al_bootsHA/B/C — matches al_bootsh indices), **2 materials** (`SC_boot`, `boot`), textures embedded.

**How it was actually driven — Claude ⇄ Blender socket bridge (NO Desktop MCP needed).** The Cowork/epitaxy build of Claude Desktop **ignores `claude_desktop_config.json` mcpServers** — no hammer ever appears, and editing that file is futile (the app rewrites it on launch; even a read-only lock didn't surface a hammer). **Bypass:** the `blender-mcp` addon runs a plain socket server on **`localhost:9876`**. A short Python client (`socket` → send `{"type":"execute_code","params":{"code":...}}` / `get_scene_info` / `get_viewport_screenshot`) drives Blender directly from a terminal — read scene, run `bpy`, screenshot to a PNG and view it. This is the reliable path; don't chase the Desktop hammer.

**Key discoveries (a re-rig chat MUST know these — they invalidate the naive plan below):**
- **al_bootsh's bones are runtime-driven / degenerate in the file.** All 4 report `head=(0,0,0) tail=(0,0,1)` — the game injects each bone's real transform at runtime from Link's foot bones. ⇒ **"parent with Automatic Weights" does NOT work** (no positioned bones to weight against). BUT the **mesh** is the real assembled boot in model space (segments A/B/C are spatially adjacent: A X[5,42], B X[-18,12], C center), so the mesh **is** a valid alignment reference.
- **Weighting = region-match, not auto.** Clone al_bootsh's armature, then for each `vboot` vertex assign it (rigid, 100%) to whichever al_bootsh segment (A/B/C → bone 1/2/3) is **nearest** (KDTree over al_bootsh verts labeled by segment). Result mirrored al_bootsh's proportions (A=366, B=149, C=43 verts).
- **Orientation/scale:** `vboot` long axis = **Y**, al_bootsh long axis = **X** → rotate **90° about Z**; **uniform** scale ≈ **1.64** (longest-axis match — do NOT non-uniform-scale, the WW boot is just chunkier and will overhang; that's fine, it must still read as WW). Then translate so bbox centers coincide.
- **The pair split:** `vboot` is a standing PAIR, split by **material** (each of its 2 meshes spans both feet, symmetric across X=0), NOT by side. Cut at **X=0** (clean gap confirmed) and keep one side → single boot. (Loose-parts separate would fragment straps.)

**SuperBMD / export gotchas (each cost a failed convert):**
- Armature MUST be named exactly **`skeleton_root`** or SuperBMD errors "No Skeleton found."
- Use the **`--bdl`** flag (or `superbmd_createbdl.bat`) — default output is `.bmd`; `itemmdl` items are **BDL** (they live in `bdlm/`).
- **Clean geometry first**: `mesh.delete_loose` + `mesh.dissolve_degenerate`, else "a face … has less than 3 vertices (loose vertex or edge)."
- **Rename materials back to `SC_boot` / `boot`** — Blender/SuperBMD import prefixes them `m0…`/`m1…`; the cel-shader keys on an **`SC_` prefix at the START**, so `m0SC_boot` would NOT match. (Kept: both link `V_boot.png`.)
- **UV-JOIN TRAP (cost the "beige boot"):** joining the 2 vboot meshes gave the joined object **two UV layers** (`meshId0-tex0`, `meshId1-tex0`); each face has valid coords in only ONE, so faces sampling the empty layer land at (0,0) = flat beige. **Fix:** merge into one layer — for `material_index==1` faces copy their UVs from layer-2 into layer-1, then delete layer-2 (single `tex0`). This bug is in the BDL too, not just the viewport — re-export after fixing.

**Baked transform before export** (`transform_apply` all) so SuperBMD exports from identity. Removed the stale old-armature modifier (parent-clear leaves the modifier behind → double-armature).

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
out = r"%USERPROFILE%\Documents\ww_export\vboot_new.dae"   # <-- edit path (folder must exist)
bpy.ops.wm.collada_export(filepath=out, selected=True, apply_modifiers=True,
                          include_armatures=True)
print("Exported ->", out)
```

---

## Fresh Blender-chat opener (paste)

> You are assisting with Blender asset work for a Twilight Princess mod (Dusklight) that reuses Wind Waker item meshes. **I'm familiar with Blender but very inexperienced — explain every step concretely (where panels/buttons are), and give me ready-to-run scripts for the Scripting tab.** Read `docs/Blender-WW-Items.md` first — it has the goals, the tools (SuperBMD, Blender 5.x + Collada caveat, GCFT), the key facts (vboot = 2-joint standing pair; al_bootsH = single boot rigged to foot bones 0x13–0x1A at joint indices 1/2/3; game addresses joints by index), and the current blocker. Start with **Task 1 — Iron Boots re-rig**: help me extract `vboot` + `al_bootsH`, compare their skeletons, and re-rig vboot into a single boot matching al_bootsH's joint layout. If I set up `blender-mcp`, drive Blender directly.

---

## Status / handoff notes

- **Iron boots ASSET = ✅ DONE (2026-07-02).** Re-rigged textured `vboot.bdl` built & validated (see [Task 1 — DONE](#-done--asset-built--validated-2026-07-02)). **Remaining, game-side:** (1) repack `vboot.bdl` into `itemmdl.arc` (GCFT), drop in `res/Object/`, wipe `dawn_cache.db*`/`pipeline_cache.db*`; (2) test in the **Item Viewer** (index `0xE`) = mesh/texture check; (3) test **on Link's foot** = rig check; (4) code chat **removes the `s_albwWwBootsSkinned` rig-gating** so vanilla `setAnmMtx(1/2/3)` drives it — and gate load-vboot-vs-al_bootsh behind the **TP↔WW toggle** flag.
- **Game side is done & committed** for bow + hookshot held skins and the get-item viewer (see the two related docs).
- Keep `SC_`-prefixed material names on any edited mesh so the in-game cel renderer applies automatically. **Strip SuperBMD's `m0`/`m1` import prefixes before export** (they defeat the `SC_`-at-start check).
- **Tooling:** SuperBMD `2.5.0` at `D:\Extractuibs\Extractions 6.5\ALBW Blender workflow\SuperBMD_2.5.0(1)`; Blender **4.2 LTS** (5.x dropped Collada — needed for the DAE bridge); drive Blender via the **`localhost:9876` socket**, not the Desktop MCP hammer.
