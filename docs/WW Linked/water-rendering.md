# WW water rendering — the system taxonomy (ALL islands)

**Why this doc:** "water" in WW is not one system — it is at least six, each with its own textures,
geometry, blend modes, and code home. Porting or debugging "waves" without naming WHICH system is
how effort gets misdirected. Applies to every island; lives outside the per-island folders.

**Sources:** community video breakdown (user-supplied, 2026-07-22 —
https://youtu.be/8NL9Cc05CYk + transcript) — marked **[video]** where it is our only source;
Housing's donor-code reads (bus §97b/§98) — marked **[verified]** with citations. Where both agree,
the video corroborates the code read. IVAN rule: video-only claims are analysis, not donor law —
verify in code before building on any of them.

---

## The taxonomy

| # | system | where seen | mechanism (summary) | our status |
|---|---|---|---|---|
| 1 | **Open-sea surface foam** (the "poster child") | open Great Sea | 4-bit grayscale tex, runtime PALETTE by weather/time; two scales LIGHTEN-blended; displacement-map scroll for wobble; **hand-darkened mip levels fade foam with distance** [video] | sea surface not ported (open ocean not needed for hosted islands) |
| 2 | **Whitecap foam panes** (kankyo wave packet) | open water near camera | ~300 pulsing camera-facing trapezoid QUADS; sin-phase gated; sea-color TEV lerp through usonami tex; wind-skewed, bank-rolled — **[verified]** `d_kankyo_wether/rain`, full recipe bus §98 | **§98 Engine BUILT** — white was missing `BG1_K0` (stashed in `plight_col[2]`); awaiting №31-B eyes |
| 3 | **Island grounding rings** (foam + shadow around islands) | major islands (Forest Haven etc.; missing on Great Fish) | two textures scrolling opposite directions; foam breathes via UV-cycling the top corners of a face strip; seams hand-placed [video] | not identified in donor code yet; likely island/sea model materials |
| 4 | **Beach crashing waves — THE Outset shoreline material** | Outset beach ("seen at the start of the game") | **NINE layers** [video]: 2 horizontal scrolls + vertical crash scroll (vertex-alpha faded); subtle light foam (back-and-forth); always-present airy foam (same tex, DARKEN blend); lagging pitch-black shadow layer shaping alpha + making sand look wet | **LOCATION CONFIRMED (Bridge 0.30.1):** Room44 `model1.bdl+btk` — 8 materials all `SC_01_mizu*`, 9 textures `Txa_nami_*/Txa_umi_*/Txa_sirokuro_a`, BTK 16 tracks/100fr scrolls, ALREADY MOUNTED. PARTIALLY VISIBLE in-game (user: 'wants to be there'). Fidelity pass live: Bridge dumping the 8 materials' TEV/blend/vertex-alpha reference; Engine diffs rendering against it (bus §101) |
| 5 | **Rivers** | Forest Haven etc. | two side-foam scrolls at different scales curving up walls; center wiggly shadow scroll; mirrored-UV joins at mid-river islands [video] | future spaces |
| 6 | **Waterfalls + ripples** | Forest Haven etc. | crest: two scrolling textures, DARKEN blend used as alpha mask; base ripples: center-out scrolling tex + additive second layer + hard alpha-clip at half-brightness + vertex-alpha edge fade [video] | future spaces |

---

## Cross-corroborations (video ↔ verified code)

- **The palette claim corroborates the §98 TEV read**: video says foam colors are "picked at
  runtime from a predefined palette, which changes based on weather and time of day" — exactly
  `dKy_get_seacolor(&amb,&dif)` + the TEV lerp `(dif → amb by TEXC)` found in `drawWave`
  [verified, `d_kankyo_rain.cpp:3281`]. The grayscale texture is a MIXING CONTROL, never drawn
  directly. Any port that binds the texture without the palette TEV renders white — observed
  in-game (bus §98).
- **`TXA_USONAMI_M`** = Forsaken Fortress (`MajyuE`) stage variant [verified §98] — the video's
  weather/palette note is the WHY behind per-stage variants existing.

## ⚠ Porting traps this doc exists to prevent

1. **NEVER regenerate mipmaps on WW water textures.** The mip levels are HAND-AUTHORED darker to
   fade foam with distance [video: "higher mip levels darken the foam until it's entirely black"] —
   an asset pipeline that rebuilds mips destroys the distance fade silently. №31-B shaping trap of
   the first order; the `_64mip` suffix on the usonami textures is load-bearing. (Housing: flag any
   pipeline stage that touches mips on `txa_*` textures.)
2. **Name the system before debugging.** "Waves look wrong" must resolve to a row above first —
   the §98 panes (system 2) and the beach material (system 4) are unrelated implementations that
   both read as "waves at the shore."
3. **Vertex alpha is load-bearing** in systems 4 and 6 (edge fades, wet-sand shaping). Mount/import
   stages that drop or renormalize vertex colors/alpha will silently flatten these materials.
4. Blend-mode vocabulary per system (video): LIGHTEN (1), DARKEN (4's airy foam, 6's crest mask),
   ADDITIVE + hard alpha clip (6's ripples). J3D expresses these via TEV/blend configs native to
   the BMD materials — if a mounted model's material renders wrong, compare against these intents.

## The open question this raises (system 4 — the one the user wants on Outset)

**Where does the 9-layer beach material live in the donor?** Hypothesis — **SUSPECTED, NOT
PROVEN:** `Room44`'s secondary model (`model1.bdl` + `model1.btk`), which our mount ALREADY renders
(`ext_bg0.ini` declares model2/model2_btk; bus §97c). Scrolling layers = BTK texture-SRT anims;
back-and-forth + crash motion = BTK too (or BRK/vertex anim); vertex-alpha fades ride the mesh.
**If true: Outset's beach waves may already be partially on screen or one material-fidelity check
away** — the question becomes whether our J3D path renders all nine layers faithfully (vertex
alpha, blend modes, all BTK tracks playing).
**Identification steps:** (1) dump `model1.bdl`'s material list + `model1.btk`'s track targets
(Bridge — material/tex names tell the story instantly); (2) user eyeball: is ANY shoreline motion
visible at the beach today? (3) compare against video footage timestamps.

**§99c Bridge dump (2026-07-22):** step (1) **done — hypothesis SUPPORTED.** All 8 materials are
`SC_01_mizu*`; TEX1 is the `Txa_nami_*` / `Txa_umi_*` family; BTK has 16 multi-keyframe bindings
@100f. Already mounted. Remaining: (2) eyeball + (3) fidelity if motion wrong — not a new system.

Cross-refs: bus §97-§98 (foam panes) · [shared-libraries.md](shared-libraries.md) (usonami
textures) · [islands/Outset/README](islands/Outset/README.md).

---

## Tier-3 source: gordonnl "The Ocean" (Medium) — **UNPROVEN, NOT-VANILLA RECREATION (user's label)**

https://medium.com/@gordonnl/the-ocean-170fdfd659f1 — a WebGL/Three.js from-scratch recreation.
**Lowest epistemic tier in this doc:** it describes the AUTHOR'S reimplementation, not WW's. Use
for intuition about system 1 behavior; **never port its constants or shader structure as donor
fact.**

**Claims about the ORIGINAL that CORROBORATE our verified reads:**
- "Ocean plane doesn't shift with boat movement; texture and swell shift across a static plane" +
  "Link doesn't move vertically; ocean and islands rise/fall instead" — matches **[verified]**
  `d_a_sea`'s camera-centered packet/cloth structure (bus §97b: `daSea_packet_c`, flat-inter,
  `CheckRoomChange`).
- "GameCube didn't support vertex shaders → wave math on CPU applied to vertices" — matches
  **[verified]** `daSea_calcWave` (`d_a_sea.cpp:349`) computing heights CPU-side.

**Recreation-only content (author's inventions — IVAN-hard: never portable):**
- 3-layer fragment approach (base blue + white-mask + dark offset lookup) — a cruder read of
  system 1 than the video's palette/displacement/mip account; where they disagree, prefer [video],
  and both yield to code.
- Compound-sine formulas WITH SPECIFIC CONSTANTS (e.g. `(sin(x·1.0+t·1.0)+sin(x·2.3+t·1.5)+
  sin(x·3.3+t·0.4))/3`) — the author's chosen numbers. WW's real wave math lives in
  `daSea_calcWave` + `daSea_WaveInfo` [verified, unread in detail]; if system 1/vertex swell is
  ever ported, the constants come from THERE, not from this article.
