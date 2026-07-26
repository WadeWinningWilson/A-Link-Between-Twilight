# WW lighting & color-palette system (ALL islands) — [video] tier, decomp-compare pending

**Source:** Jasper/noclip WW cel-shading breakdown (youtube mnxs6CR6Zrk, user-supplied 2026-07-26).
**Tier: [video]** — technically credible (noclip.website author renders these games), but every
claim below **must be compared against the decomp before building on it** (user instruction).
Island-generic: this is the game-wide lighting/color system.

## The claims (video), with our known decomp anchors

### 1. THE PALETTE SYSTEM — the big one (bears on §141 water + §130 grass VFX + Link lighting)
- **Hand-authored color palettes for SIX times of day × weather** (sunny/stormy; some islands add
  snow). Covers: island lighting, **"the tinting of the ocean waves"**, sky, night windows —
  everything.
- **Decomp anchors we already hold:** `dKy_tevstr_c` (per-room tev colors — grass VFX reads its
  mColorK0, §130); `g_env_light` + the BG1 channel (§100: noon BG1_C0 white / BG1_K0 (9,99,224) —
  the sea palette the panes lerp); the №113 palette conversion (our pipeline; twice dropped K0 —
  §100/§112); `dKy_get_seacolor`. **Compare-target: d_kankyo.cpp's time-of-day palette tables and
  the per-channel (BG0..BG3) draw-time register writes.**
- **DIRECT COROLLARY for §141:** the video independently corroborates that the water surface tint
  is written AT RUNTIME from this palette (six-daytime authored colors), not from the BDL bake —
  matching the Ferry-1 result (authored bake ≠ vanilla look).

### 2. CHARACTER CEL-SHADING — three steps (bears on the parked Link lighting item)
1. **Vertex lighting** (normal·light angle, per vertex);
2. **"Toonification": the `ZAtoon` texture** — a 256-wide lookup ramp: vertex-light value is the X
   coordinate; <~120 → pure black, >~136 → pure white, sharp gradient between. The cel edge IS this
   texture. (Decomp anchor: the toon tex binding in dKy/d_a_player draw paths — compare pending.)
3. **Palette tinting** — the black/white output is re-colored to the scene palette's
   highlight/shadow pair (same six-daytime system as above).
- Eyes/mouth: never lit (always full-bright).

### 3. SINGLE-LIGHT RULE (art decision, not hardware)
Most objects respect ONE light at a time (nearest; with a small switch animation) despite GC
supporting 8. Keeps silhouettes readable. (Anchor: the light-influence selection in d_kankyo —
`mLightInfluence`/plight selection; compare pending.)

### 4. RULE-BASED EXCEPTIONS (not palette entries)
- Sun-stare (first person): darkens island palette / brightens sky by view-angle to sun.
- Lightning strike: replaces every highlight/shadow with blue — a hard-coded rule.
- (These are engine rules layered OVER the palette — relevant if storm/lightning ever ports.)

## What this changes for live threads
- **§141 (main water dark):** corroborated — the fix is the runtime palette write path, not more
  register pinning. Research target: which palette entry + which draw path writes the sea-surface
  KONST at draw (d_kankyo BG-channel setTev writes).
- **№113 pipeline risk sharpened:** the palette is per-time-of-day ARRAYS. Our converter has already
  dropped single entries twice (K0, §100/§112). When comparing against the decomp, audit what №113
  carries vs what the donor tables hold — partial palette conversion would explain "right at noon,
  wrong at other times" classes of bug before they're ever reported.
- **Link lighting (parked):** the ZAtoon 3-step is the donor spec for whenever that unparks.

Cross-refs: bus §141 (water runtime layer) · §100/§112 (K0 stash incidents) · §130 (grass VFX K0) ·
grass-effects.md · water-rendering.md · the NEUTRAL-AMBIENT recipe (cookbook).
