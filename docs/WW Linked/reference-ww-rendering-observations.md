# Reference — WW Rendering, from external sources & emulator dumps

> **This doc is EXTERNAL reference material, not our implementation.** It collects observations
> from third parties and emulator inspection so we don't re-derive them. In-progress porting work
> lives in the *recipe* docs; keep the two separate. Every entry is tagged:
>
> - **✓ verified** — confirmed against our decomp (`D:\XXXXXXX\WW DP\src`) or our shipped assets.
> - **⚠ unverified** — plausible, recorded, not yet confirmed here.
> - **✗ contradicted** — our data disagrees; the correction is noted.

**Sources**
- Nathan Gordon, *Link's Expressions* — Wind Waker graphics analysis
  ([medium](https://medium.com/@gordonnl/links-expressions-eb7beae2c62c) /
  [windwaker.graphics](https://windwaker.graphics/links-expressions.html)).
- Ben Jones, *Twilight Princess Eyes Breakdown* ([benjones.us](https://www.benjones.us/twilight-princess-eyes-breakdown/)).
- A user's own Dolphin-emulator inspection (texture dumping, wireframe, freecam, per/vertex light) of a legally-owned GameCube copy.
- A WWHD (Wii U) model-hacking tutorial (Switch-Toolbox + Blender), for the HD asset names.

> **Legal note (recorded, not acted on):** the emulator observations came from inspecting a
> legally-owned copy — fine for study. **Do not redistribute dumped game textures or assets**,
> and nothing dumped goes into the receiver tree (the covenant already forbids this). This doc
> records *facts learned*, not assets.

---

## 1. Faces — the floating-plane system

**Claim (article + emulator):** eyes, eyebrows and pupils are flat textured planes raised off
the head; expressions are texture swaps; the pupil is masked in the shader (visible only in the
eye's white areas); pupil "look" direction is a UV slide.
**✓ verified** in `d_a_npc_ls1.cpp` + `ls.bdl` — see
[face-expression-recipe.md](face-expression-recipe.md) for the full mechanism.

**Claim (emulator, direct):** *"facial expressions are all done with floating alpha planes for
eyes and eyebrows (that also draw in front of the hair shader) but not the mouth — that is an
opaque texture on the actual model (not floating)."*

- **✓ Mouth is opaque-on-model.** Verified: `ls.bdl` material `SC_kuchi` draws in the **OPAQUE**
  pass, unlike the eyes. It was wrong to call it a floating plane; corrected in the recipe.
- **✓ Eyes are floating alpha planes.** `SC_eyeL/R` draw **BLEND/xlu**.
- **⚠/✗ Eyebrows floating.** On **Link** the observer saw brows as floating alpha planes. On our
  **Aryll** (`ls.bdl`) the brow materials `SC_mayuL/R` are **OPAQUE** (alpha-test cutout), not
  blend — still drawn over hair, but via cutout not blend. Likely **character-dependent**, or the
  observer's "alpha plane" covers alpha-test cutouts too. Not a contradiction of the mechanism,
  a nuance in how each face is authored.

**Claim (article):** Link's set = **7 eye shapes, 6 eyebrows, 9 mouths, 1 pupil.**
**⚠** our `Ls.arc` carries its own expression set (11 BTPs); exact per-feature counts not tallied.

**HD asset names (WWHD tutorial), for cross-reference only** — our GC assets differ:

| Feature | WWHD name | Our GC (`Ls`) |
|---|---|---|
| Eyes | `eyeh.1`–`eyeh.7` | `ls_eye` (frames via BTP) |
| Eyebrows | `mayuh.1`–`mayuh.6` | `ls_mayu` |
| Mouths | `mouthS3TC.1`–`mouthS3TC.9` | `ls_kuchi` |
| Pupil | `hitomi` | `ls_hitomi` |

**Tutorial guidance worth keeping:**
- *"Link's eyes and eyebrows display in front of the rest of the mesh — the textures you use
  must be transparent."* **✓** consistent with eyes = BLEND, brows = alpha-cutout.
- *"Be careful using colors with eyebrows and eyes — anything that isn't black or white shows up
  way brighter than it should."* **⚠** a TEV/lighting characteristic; plausible, not verified.

---

## 2. Other systems observed (non-facial — recorded, out of current scope)

All from the emulator inspection, **⚠ unverified** here (no decomp cross-check done — logged so a
future lane can confirm before relying on them):

- **Water follows the player.** The water plane is effectively attached to Link and moves with
  him on X/Z (not vertical). Explains why the sea never "runs out" around the camera.
- **Enemy cloth is dynamic, not baked.** The cloth strips on the boar/Moblin weapons are runtime
  cloth simulation, not precomputed physics.
- **Stars are additive rotating quads.** The night sky's stars are not a skybox or single
  texture — they're a stack of additive quads rotating over each other.
- **Waves come in two kinds by weather.** Both geometry waves and sprite (billboard) waves exist
  and are used under different weather states.

> These matter to later WW restoration work (sky, sea, enemies) but are **not** part of the
> facial/cutscene track. Verify each against the decomp before porting behaviour from them.

---

## 3. Emulator inspection method (for reproducing/extending)

Recorded because it's the tool that produced §1–§2 and can answer future questions:

- Dolphin GFX debug: **wireframe**, **per-vertex / per-pixel light** toggles, **freecam**,
  **texture dumping**.
- **Texture-dump caveat:** dumps are named by load-time hash, not original name, and a texture
  used in N scenes dumps N times. You cannot know you have them all without visiting every 3D
  scene, and de-duplication afterwards is manual. So a dump set is **evidence, not a complete
  asset index** — treat gaps as "not yet seen", not "doesn't exist".
