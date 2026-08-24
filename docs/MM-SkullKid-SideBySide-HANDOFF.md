# MM Skull Kid — Side-by-Side Blender (HANDOFF)

**Status: FAILED / DO NOT USE current MM mesh export**

Previous agent attempted “step 1”: MM `object_stk` mesh + materials beside TP `E_PM` in Blender. **User confirmed the result is corrupted nonsense** — broken geometry and wrong materials. This doc records what was tried, what failed, and what to do instead.

---

## Goal (unchanged)

1. **Side-by-side in Blender:** TP Skull Kid (`E_PM_29`) with TP UVs/textures **next to** MM Skull Kid (`object_stk`) with MM UVs/textures — for visual region mapping before painting TP atlases.
2. **End product (later):** Textures-only reskin on TP BMD — not a mesh port.

---

## What actually works (keep using)

| Asset / tool | Path / command | Notes |
|--------------|----------------|-------|
| TP body DAE | `%AppData%\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_29.dae` | SuperBMD export; armature `skeleton_root`, meshes `mesh-0..2` |
| TP vanilla PNGs | `<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\textures_vanilla\` | `pm_tex01`, `pm_leaf01`, `pm_eye.*` |
| Blender blend (TP only) | `<decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend` | **May contain bad MM import + duplicate TP objects — clean or revert** |
| TP import script | `tools/blender_socket/mm_skullkid_import.py` | Socket `:9876` |
| TP texture relink | `tools/blender_socket/mm_skullkid_blender_textures.py` | Vanilla vs `--mm` |
| Nerrel source PNGs (18) | `companion_mods/MM-SkullKid-Reskin/_work/mm_refs_hd/_nerrel_skull_kid/` | Authoritative MM **color** reference; sync via `mm_sync_nerrel_skull_kid.py` |
| MMN64HD dump | `companion_mods/MM-SkullKid-Reskin/_work/mm_refs_hd/` | Hash filenames; use Nerrel folder for Skull Kid colors, not blind hash guesses |
| object_stk XML | `companion_mods/MM-SkullKid-Reskin/_work/object_stk/object_stk.xml` | From zeldaret/mm; DL/texture offsets |
| MM ROM (US 1.0) | Recomp bundle `Legend of Zelda, The - Majora's Mask (USA).z64` | md5 `f46493eaa0628827dbd6ad3ecd8d65d6` |
| Research docs | `docs/Blender-MM-SkullKid.md` | TP workflow; side-by-side section added prematurely |

---

## What was built (BROKEN — do not trust)

| File | Purpose | Problem |
|------|---------|---------|
| `tools/companion_mod/mm_stk_f3dex2_mesh.py` | ROM → OBJ + N64 PNGs | **Custom F3DEX2 parser is wrong** |
| `.../object_stk/mesh_export/MM_SkullKid_body.obj` | MM mesh | **Most vertices are `0 0 0`**; exploded/degenerate tris |
| `.../mesh_export/textures_n64/*.png` | N64 textures | pigment64 decode may be OK in isolation, but **UVs on mesh are garbage so preview is meaningless** |
| `tools/blender_socket/mm_skullkid_side_by_side.py` | Blender import both | Imported the broken OBJ; **reported success without visual check** |

Evidence (OBJ snippet): torso group is almost all zeros with occasional stray coords — not a valid Skull Kid.

```text
v 0.000000 0.000000 0.000000   # repeated hundreds of times
v 92.580000 -84.520000 104.810000   # sparse outliers
```

Manifest claims 247 tris but many parts have **0 tris** (pelvis, right upper arm, right hand).

---

## Honest error log (previous agent)

1. **Invented a F3DEX2 mesh parser instead of using proven tooling** (Fast64, ZAPD, libgfxd). MM uses **F3DEX2** opcodes (`G_VTX=0x01`, `G_TRI1=0x05`, `G_DL=0xDE`) — not the same as legacy F3D the first parser scripts assumed.

2. **`G_VTX` / triangle index decoding is wrong.** Vertex buffer never filled → draw calls read index 0 → positions stay `(0,0,0)`.

3. **`G_DL` branch targets guessed** (`w1 & 0xFFFF`) instead of proper segmented addressing after `G_MOVEWORD` segment setup. Some branches hit valid sub-DLs by accident; most geometry missing or wrong.

4. **No skeleton.** Limb translations were placeholders `(0,0,0)`; all DL parts stacked at origin. Never parsed Flex skeleton / limb table from ROM or decomp.

5. **Materials assigned by hand-wavy map** (`DL_TEXTURE_HINTS`: torso → shawl, etc.) — **not** from verified `SETTIMG` → texture offset → DL binding. Earlier `_parse_stk_dls.py` showed SETTIMG scan incomplete on branched DLs.

6. **Claimed “step 1 complete” from script stdout** (triangle counts, socket JSON) **without opening Blender Material Preview** or inspecting OBJ vertices.

7. **Side-by-side script re-imported TP DAE** on existing scene → duplicate objects (`mesh-0.001`, `skeleton_root.001`). Fix attempted but blend may already be dirty.

8. **Conflated texture pipelines:** `object_stk/n64/` (older ROM extract, user/docs say corrupt) vs `mesh_export/textures_n64/` (pigment64 in new script). Neither helps if mesh/UVs are broken.

9. **Did not complete the right path:** ZAPD extract `object_stk.c` → Fast64 `object.oot_import_skeleton` with mm decomp path — cloned Fast64 to `tools/vendor/fast64` but never wired it; no ZAPD build (no MSVC/cmake on machine).

10. **Documentation updated as if working** (`docs/Blender-MM-SkullKid.md` session table) before user validation.

---

## MM vs TP facts (still valid research)

- MM: ~20 N64 textures, many DLs, **no cape mesh** — shawl on torso/hat; head/eyes drawn in `DmStk_PostLimbDraw` (`z_dm_stk.c`).
- TP: 3 atlases (`pm_tex01`, `pm_leaf01`, `pm_eye.*`); **mesh-1 is TP-only leaf cape**.
- Reskin target: paint MM look onto **TP UV atlases**, not 1:1 file rename.
- Use **`_nerrel_skull_kid/`** for color reference, not `_groups/` hash guesses.

---

## Recommended path forward (for next agent)

### Option A — Fast64 + mm decomp (preferred)

1. User sets up [zeldaret/mm](https://github.com/zeldaret/mm) with baserom → `make` extracts `assets/objects/object_stk/object_stk.c`.
2. Install Fast64 in Blender 4.2 (`tools/vendor/fast64` already cloned in repo).
3. Enable MM features; set decomp path; import `gSkullKidSkel` via **Import Skeleton** (`object.oot_import_skeleton`).
4. Place in collection `MM_SkullKid`; TP stays in `TP_SkullKid`. **Verify visually before saving.**

### Option B — ZAPD only first

1. Build ZAPDTR (`tools/vendor/ZAPDTR`) on Windows (VS + libpng) or WSL.
2. `ZAPD e -i object_stk.xml -o ... -b <baserom>` → inspect generated C/Gfx.
3. Then Option A or port Fast64 `parseF3D` **inside Blender** with extracted C, not raw ROM hacks.

### Option C — Fix custom parser (not recommended unless A/B blocked)

- Port `fast64_internal/f3d/f3d_parser.py` `parseF3DBinary` with `segmentData={6:0}` **in Blender bpy context**, not standalone half-port.
- Must handle `G_MOVEWORD` segment registers, F3DEX2 `G_VTX`/`G_TRI1` packing, and Flex skeleton from extracted C or verified limb table offset.

### Verification checklist (mandatory)

- [ ] MM object looks like Skull Kid in Material Preview (not spike ball / collapsed origin).
- [ ] TP object unchanged; no `.001` duplicates.
- [ ] MM materials show correct N64 or Nerrel art **on the right mesh regions** (hat rings, shawl, face).
- [ ] Compare scale/facing; align for side-by-side (manual offset/rotation OK).
- [ ] User sign-off before updating docs.

---

## Key paths (quick copy)

```
Repo: %USERPROFILE%\Documents\dusklight

TP DAE:     %AppData%\TwilitRealm\Dusklight\bmd_export\superbmd_verify\E_PM_29.dae
Blend:      <decomp-root>\Ex TP\Blender workflow\Custom porting work\MM\MM_SkullKid_v1.blend
Nerrel:     companion_mods\MM-SkullKid-Reskin\_work\mm_refs_hd\_nerrel_skull_kid\
object_stk: companion_mods\MM-SkullKid-Reskin\_work\object_stk\
BROKEN OBJ: companion_mods\MM-SkullKid-Reskin\_work\object_stk\mesh_export\

Fast64:     tools\vendor\fast64\
ZAPDTR:     tools\vendor\ZAPDTR\

Blender socket: 127.0.0.1:9876
Probe: python tools/blender_socket/probe_blender.py
```

---

## Scripts to delete or rewrite

| Script | Action |
|--------|--------|
| `mm_stk_f3dex2_mesh.py` | **Rewrite or replace** with Fast64/ZAPD path; do not patch blindly |
| `mm_skullkid_side_by_side.py` | **Rewrite** after valid MM mesh exists; never import broken OBJ |
| `_parse_stk_dls.py`, `_parse_stk_gdl.py` | Research only; SETTIMG incomplete |

---

## Related transcripts

Cursor chat: `49528311-4989-4953-a7b2-81a28170d34c` (full MM Skull Kid reskin thread)

---

## Paste block for new Claude chat

See user message — full handoff block copied below in chat response.
