# HANDOFF — Armogohma phase-3 reveal: re-export the reveal BMD (orientation fix)

> **✅ RESOLVED (2026-07-08) — this handoff is historical.** The orientation fix below was applied
> (upright, playtest-confirmed). Two further reveal-model bugs surfaced and were also fixed: the
> eye/lids snapping to the dorsal socket (code anchor `b_gm_revealAnchorMouthJoint`) and the legs
> shattering under animation (Blender-round-trip re-derived the bone frames → fixed with the
> **[BMD reskin tool](BMD-Reskin-Tool.md)**). Current status + all three fixes:
> **[Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md) §12.** Still open: eye white-orb
> (env/emboss material) + ~+3% eye shrink.

**To the next AI instance.** The Armogohma "phase-3 reveal" feature is code-complete and builds; the fight plays correctly. The ONE remaining blocker is that the custom reveal model (`B_gm_37.bmd`) is baked **180° inverted** and renders **upside-down / under the floor** the moment it swaps in. Your job: **re-export the model corrected via the Blender MCP (port 9876), drop it in, re-enable the swap, rebuild, and confirm in-game.** Every detail you need is below.

---

## 1. What the feature is (scope)
Refined Armogohma fight ("Boss Refinements" ON + Custom Models folder enabled + `B_gm_37.bmd` present). After the **2nd** Dominion-Rod statue hit (vanilla is 3), the boss's model **swaps to a revealed "single-eye" model** (eye baked at the mouth, face-plate removed) and enters a floor **phase 3**: get-up → dash-chase with a 5s/5s eye blink cycle → a hit-counter that triggers an 8s stagger (5 hits) or a 10s tracking-laser (>3 hits) → HP drains (defended ÷2) to ≤5% → the vanilla disappear cutscene → E_GM floor eye. All gated so vanilla players are unaffected. Full design: `docs/Boss-Fights-RefinedGohma.md` §10–§12. Blender/model details: `memory/armogohma_phase3_blender.md`.

## 2. The bug (confirmed, precise)
- **In-game symptom:** phase 3 runs perfectly on the *vanilla* model (reorients + dashes upright — playtest-confirmed). The instant the **reveal model** swaps in, it renders **upside-down and sunk under the floor**.
- **Root cause (confirmed via Blender MCP):** the reveal scene's objects (meshes + `skeleton_root`) carry a **+90°X `matrix_world`** (euler x = +1.5708). The SuperBMD export recipe *also* passes `--rotate`. SuperBMD bakes the node transform **AND** applies `--rotate` → **two 90° rotations = a 180° flip** in-game. (Blender world-space is upright: `world_root` Z=0, `body` Z=297, `eye` Z=235; local verts are already Y-up, body local +Y=611.)

## 3. The Blender scene (as inspected 2026-07-08)
Objects: `skeleton_root` (ARMATURE, 66 bones), `mesh-0` (abdomen fur), `mesh-1` (eyeball, loc z≈−29), `mesh-2` (shell/legs/eyelids). Every object's `matrix_world` = +90°X:
```
(1, 0,  0, 0)
(0, 0, -1, 0)
(0, 1,  0, 0)
(0, 0,  0, 1)
```
**Confirm the scene is the Gohma reveal model before touching anything** (`get_scene_info`); a different project was briefly loaded earlier.

## 4. THE FIX — step by step (Blender MCP + SuperBMD)
**Goal: remove one of the two rotations.** Apply the +90°X object transform so `matrix_world` becomes identity (world Z-up geometry baked in), then export + `--rotate` = exactly one rotation.

1. **Verify scene** — `get_scene_info`; confirm `skeleton_root`, `mesh-0/1/2` present. Snapshot each object's `matrix_world` and WORLD-space bounds (you'll re-check them after apply to prove the bind is intact).
2. **Apply the object transform to the whole rig together** (armature + all 3 meshes selected, so the skin stays consistent):
   ```python
   import bpy
   bpy.ops.object.select_all(action='DESELECT')
   for n in ["skeleton_root","mesh-0","mesh-1","mesh-2"]:
       bpy.data.objects[n].select_set(True)
   bpy.context.view_layer.objects.active = bpy.data.objects["skeleton_root"]
   bpy.ops.object.transform_apply(location=False, rotation=True, scale=False)
   ```
3. **Verify:** each object's `matrix_world` is now identity (or very close), and the **WORLD-space vertex bounds are UNCHANGED** vs step 1 (body still up at world +Z, eye mid, legs low). If bounds changed or the mesh visibly deformed → the bind broke; **undo** and use the fallback (§6).
4. **Delete Loose** on every mesh (edit mode → Select All → Mesh ▸ Delete Loose) — SuperBMD dies on faces with <3 verts. (Also re-check no stray verts were introduced.)
5. **Export DAE** (collada) — triangulated, include armature/deform bones, **Z-up**, selected objects. Use the same path/name the prior export used (a fresh `.dae` next to the working folder is fine).
6. **Run SuperBMD** (unchanged recipe, keep `--rotate`):
   ```
   "D:\Extractuibs\Extractions 6.5\ALBW Blender workflow\SuperBMD_2.5.0(1)\SuperBMD.exe" <new.dae> "D:\XXXXXXX\Ex TP\Blender workflow\Modified daebmds\B_gm_37.bmd" --mat "D:\XXXXXXX\Ex TP\Blender workflow\B-gmarc\Gohma output\goma_materials_edited.json" --rotate
   ```
   (`goma_materials_edited.json` already has body `goma2_body01:1`→`:0` and eye env/emboss redirected to `goma_eye01:0`. `--profile` prints then exits non-zero — harmless.)
7. **Verify the BMD:** 66 joints, 3 meshes/materials/textures, **eye material stays index 1** (eye-zoom BTK depends on it).
8. **Copy** the new BMD over:
   `%AppData%\TwilitRealm\Dusklight\model_replacements\Armogohma Custom\B_gm_37.bmd`

## 5. Re-enable + verify in-game
1. In `src/d/actor/d_a_b_gm.cpp`, flip the diagnostic define **back to 0**:
   `#define D_ALBW_ARMO_P3_DIAG_NOSWAP 0`  (currently `1` = keeps the vanilla model through phase 3).
2. Build: run `build_run.bat` (RelWithDebInfo) from the repo root.
3. **Wipe GPU caches** (mandatory after every build): delete `dawn_cache.db*` and `pipeline_cache.db*` in `%AppData%\TwilitRealm\Dusklight`.
4. Playtest: Boss Refinements ON + Custom Models folder "Armogohma Custom" enabled. Take Armogohma to the **2nd statue hit**. PASS = the revealed model stands **upright**, on the floor, eye at the **mouth (front)**, and is hittable head-on (HP bar drains during the eye-open windows and the 8s stagger).

## 6. Fallback if apply-transform breaks the skin
Don't touch the rig. Instead re-export with the collada exporter's **global orientation** options (`export_global_up_selection` / `export_global_forward_selection` / `apply_global_orientation`) to bake the desired up-axis, choosing values so that with `--rotate` the net is a single Z-up→Y-up rotation. Iterate: export → SuperBMD → drop → in-game check upright. (There is also a code-side last resort — a fixed compensating rotation on the reveal model's `baseTRMtx` in phase 3, floor-only — but it risks foot-IK quirks and mis-aims the forward offset; prefer fixing the asset.)

## 7. Current code state (all builds clean, UNCOMMITTED — do not commit unless asked)
- `src/dusk/custom_assets.cpp` — Layer-B load-once cache + leak diagnostic.
- `src/d/d_albw_boss.{h,cpp}` — `dAlbwBoss_armogohmaPhase3Damage()` (÷2 defense, `kAlbwArmogohmaPhase3DefenseDiv=2`, handoff at `kAlbwArmogohmaPhase3HandoffPct=5`).
- `src/d/actor/d_a_b_gm.cpp` — Stage 1 reveal build/swap + Stage 2 `ACTION_PHASE3` state machine (INTRO/DASH/VULN/LASER + single `s_gmPhase3HitCount`). `#define D_ALBW_ARMO_P3_DIAG_NOSWAP 1` **← flip to 0 after the model is fixed.** Heap `0xC800`.
- BMD currently in place is the OLD inverted one — your re-export overwrites it.
- After the reveal renders upright, re-validate open playtest items: HP-drain feel + ÷2 defense, the P3_VULN `WAIT` pose (may want a more "weak"/OoT iris-spin look), 60 fps timing of the 300/480/600/180 frame constants, and beam aim from the ground model.

**Do NOT commit or push unless the user asks.** Build guidelines: read `build-fps-guidelines` + commit-and-push docs first; wipe GPU caches after every build.
