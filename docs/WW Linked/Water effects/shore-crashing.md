# Shore crashing waves — beach material (system 4)

Per-effect doc for the Outset beach 9-layer crashing-wave material. Part of the `Water effects/`
set; taxonomy + shared traps in the parent [../water-rendering.md](../water-rendering.md).
Siblings: [waves.md](waves.md) (system 2, ACCEPTED), great-sea (system 1, future).

## STATUS: §128 compositing fix shipped — USER VERIFY crash swoosh (2026-07-25)
Engine's fidelity attempt on the beach material produced **white water at the shoreline** — the
9-layer crash did not resolve; the shore surface reads as flat white/blank (see user screenshot,
2026-07-25). This is a fidelity FAILURE, not the accepted system-2 panes (those are correct — waves.md).

## What we know (bus §99–§103)
- **LOCATION CONFIRMED:** `Room44` `model1.bdl` + `model1.btk`, ALREADY MOUNTED via `ext_bg0.ini`
  (`model2`/`model2_btk`). 8 materials all `SC_01_mizu*`; textures `Txa_nami_*`/`Txa_umi_*`/
  `Txa_sirokuro_a` (the grayscale control); BTK 16 tracks × 100f scrolls.
- **Reference sheet EXISTS (Bridge 0.30.2):** `reports/model1_mizu_mat_ref.md` + `_tev_stages.csv`
  — 7/8 SRCALPHA blend (mat0 konst stub), 7/8 vertex-alpha `matSrc=VTX` w/ VTX1 Color0, XLU
  z-update=0, tex slots at MAT3+0x84.
- **White-water suspects** (parent traps #3/#4): vertex alpha (VTX1 Color0) dropped/renormalized →
  layers flatten; wrong blend mode → grayscale draws as opaque white instead of DARKEN/LIGHTEN
  modulating sea color; §114 shape-rewrite compounding the material path. "White" specifically =
  the grayscale `Txa_sirokuro`/foam texture drawn WITHOUT its palette/blend modulation — same class
  as system-2's white-triangle bug, different material.

## Fix route — this is J3D MATERIAL FIDELITY, not new shore code
**Key fact (Housing code audit §124):** there is NO custom Engine shore handler. model1 mounts
GENERICALLY (`d_ext_npc_mount.cpp` model2/model2_btk) through the standard J3D path. **The 9-layer
effect is DATA baked into the donor BDL's own materials (`SC_01_mizu*`), not code to write.**
Decomp-faithful here = render the mounted BDL faithfully; the BDL produces the effect.
- **All-white ⇒ our J3D is not honoring the BDL's baked TEV/blend/palette.** Prime suspect: the §114
  merge rewrite of `J3DShape`/`J3DShapeDraw`/`J3DShapeMtx` (same rewrite warping Link), OR a
  material-draw gap in how the generic mount renders a secondary model's TEV.
- **Engine:** diff live render vs Bridge's `model1_mizu_mat_ref` — confirm the BDL's blend modes +
  vertex-alpha (VTX1 Color0) + TEV stages actually reach the GX draw. White = the material setup is
  dropped/overridden before draw, not a missing layer.
- User eye = acceptance (№31-B). (If Engine made an uncommitted shore change, it is not in the
  committed tree Housing audited — verify on the merge tree.)

## Resource landing (user posting a SHORE resource, 2026-07-25 — standby)
> _Incoming shore-crashing resource — paste/link here when posted. Epistemic tier on arrival._

---

## VISUAL TARGET + composing textures (user-supplied refs, 2026-07-25)

**Reference screenshot (what it must look like):** crisp WHITE foam crashing along the sand edge
over BLUE water — Link running the shoreline (WW Outset start). Target invariant: water BLUE, foam
WHITE.

**Our failure (user-corrected 2026-07-25): BOTH water AND foam render WHITE — total collapse.** Not
a palette SWAP (that would turn foam blue) — the grayscale→palette mapping is absent ENTIRELY, so
every texel draws flat white regardless of its 4-bit value. The palette/TEV that turns the grayscale
into blue-water-with-white-foam is not being applied at all.

**Composing grayscale textures (user-supplied):** the beach foam is authored as **grayscale**
(mostly black with white cellular/cobblestone foam at the crest, + a thin white crash-line variant).
These map directly to model1's `Txa_sirokuro_a` (black-white) + `Txa_nami_*` family (Bridge §99c).
Corroboration: the video's "grayscale ocean foam, colors picked at runtime from a palette" IS our
`Txa_sirokuro`/`Txa_nami` — **the grayscale is a MASK/CONTROL, never drawn as literal color.**
White water = the grayscale drawn without its palette+blend (same root class as system-2's
white-triangle, different material).

## The 9-layer breakdown — Outset shoreline (video, doogus §8NL9; now fully captured here)

Video's Outset-specific section ("the final material... seen at the start of the game... nine
layers"). [video] tier — analysis, verify against model1's actual materials before building:

1. **Crash scroll A+B** — two textures scrolling past each other HORIZONTALLY along the shore.
2. **Vertical crash scroll** — a simple vertical scroll makes them "crash" up the beach.
   - Layers 1–2 FADE via **vertex alpha** (same as the waterfall-ripple edge fade); also faded out
     along the edges where the wave strip ends. ("trouble combining island shadow with waves.")
3. **Light-colored foam** — subtle, moves BACK-AND-FORTH (not continuous scroll). Easy to miss.
4. **Ever-present receding foam** — same texture as the crash waves but **DARKEN blend** (not
   lighten) → airy appearance; most visible at ebb when crash waves recede.
5. **Lagging shadow** — pitch-BLACK, lags behind the wave; contributes to the ALPHA layer to shape
   it AND darkens the sand to look WET where the wave just crashed.

("Nine layers" per video; ~5 distinct mechanisms above — the crash pair + directions count toward
the 9.) Blend vocabulary: LIGHTEN (crash), DARKEN (ebb foam), vertex-alpha (edge/wet fades).

**Fix implication (reinforces the mat-ref diff):** white water ⇒ the material is NOT applying the
palette+blend that turns the grayscale into blue-water-with-white-foam. Engine's diff vs
`model1_mizu_mat_ref` must confirm: blend modes (LIGHTEN/DARKEN per layer), vertex-alpha binding
(the fades), and that the grayscale feeds a palette/TEV — not opaque white. This is the system-2
palette lesson applied to system-4's material.

---

## MOTION (the "swoosh in/out" crash) — SEPARATE from color (§128, 2026-07-25)

User (correctly): the crash MOTION is not addressed by the color-register work (§126/§127). Decomp is
the effect source, not the casual "swoosh" phrasing.

**§128 ROOT CAUSE (Engine, 2026-07-25):** BTK *frame* path was live (bind LOOP / `play` / `entry`),
but model1 was created with DifferedDL flags `0x11000084` — **no `0x1200` TexGen bit**.
`calcMaterial` updated SRT in CPU mats; `diff()` never patched texMtx into the GPU DL → frozen UVs
("playback verified, no swoosh"). Fix: slot1 create flags `0x11001284` (daBg / `d_a_bg_obj` parity);
BTK `entry` before `calc`; XLU z-update stays OFF; mat0 opaque z-write left ON (donor).

**Bridge §128b CLEAR (tool 0.35.0):** `albt bridge/reports/model1_btk_motion.md` (+ CSVs). Hermite
SRT decoded (LOOP/100f/16 bindings). Engine compositing fix shipped — **USER VERIFY** crash swoosh.

Color (§127) and motion (this) are independent; both must hold for full shore fidelity.

**Secondary corroboration (2nd video, env-artist 9zA2rJmn3Ws — [recreation] tier, LOW):** "a plane
all the way across the shore, the same wave texture twice OVERLAPPING, one scrolling each direction,
UVs scrolling down; when it backs up that's different (ebb)." Independently matches §128's UV-scroll
(BTK texture-SRT) mechanism + the doogus two-textures-scroll-opposite breakdown. Two casual observers
converge on the same mechanism → confidence up, but still verify against model1.btk's actual track
config (decomp). Rest of that video = asset-AUTHORING technique (trim sheets/UV), N/A to a port.
