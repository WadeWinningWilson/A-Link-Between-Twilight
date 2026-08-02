# noclip.website — scope review & usage rules ([reference-implementation] tier)

**What it is:** Jasper's (§142 video author) open-source "digital museum" — an in-browser WebGL
renderer of game worlds, including full Wind Waker maps. **The site itself is a canvas app (not
readable by fetch); its SOURCE is public:** `github.com/magcius/noclip.website`,
`src/ZeldaWindWaker/` — **TypeScript ports of the same decomp files we work from** (`d_kankyo.ts`,
`d_kankyo_wether.ts`, `d_a_sea.ts`, `d_particle.ts`), preserving donor function names.

**User's Outset link:** `noclip.website/#zww/Room44.arc` — Room44 rendered with materials, palettes,
and (site UI) a time-of-day control.

## Epistemic tier — carefully measured (user's ask)
**[reference-implementation]** — BETWEEN the decomp and a fan recreation:
- NOT vanilla bytes: it's a reimplementation; its constants/simplifications are Jasper's choices.
- BUT far above [recreation] tier: ported from the decomp with donor names, and **visually verified
  at scale** (it renders WW correctly for thousands of users).
- **Rules:** (1) decomp remains law — never port a noclip value without checking the donor;
  (2) noclip's power is DISAMBIGUATION: where the decomp is `/* Nonmatching */` or ambiguous,
  noclip shows a proven-working interpretation; (3) the SITE is a live visual reference — better
  than static screenshots for eye-acceptance.

## What it gives us, concretely
1. **LIVE VISUAL REFERENCE:** open Room44, set time of day, compare against our build side-by-side —
   the acceptance reference for water color, foam, palettes, at ANY time (beats single screenshots;
   e.g. Ferry 3's "dusk shifts automatically" is directly checkable against noclip's slider).
2. **REFERENCE IMPLEMENTATION for our exact open threads:**
   - **§143 CROSS-CHECK DONE (2026-07-26):** noclip `setLightTevColorType` = `C0 ← tevStr.colorC0`,
     `K0 ← tevStr.colorK0`, NO toon swap; `setLight_bg` = the same 4-way palette blend
     (`palePrevA/B × paleCurrA/B × blendPaleAB`). **Ferry 3's mapping is now TRIPLE-sourced**
     (decomp + Jasper video + running implementation). Bonus corroboration: BG1 fog =
     `vrUsoUmiCol` — the `uso_umi` fog Engine already wired (§98).
   - Future: `d_a_sea.ts` = a working system-1 ocean implementation (when/if the far-ocean hole
     is filled); `d_kankyo_wether.ts` = wind streaks/weather reference.
3. **Nonmatching-function fallback:** decomp `/* Nonmatching */` functions (wave_move, drawWave,
   messageSePlay…) often have working noclip counterparts — a disambiguation source ranked above
   guessing, below matched decomp.

Cross-refs: lighting-palette-reference.md (§142 video, same author) · bus §143 (the ferry this
cross-checked) · water-rendering.md tier ladder (this tier slots between [verified] and [video]).

---

## FULL SCOPE INVENTORY (§145, 2026-07-26) — what noclip's WW source covers

22 TS files. The renderer half of WW is remarkably complete; the gameplay half is absent.

**HAS (working, readable implementations):**
| file | maps to our threads |
|---|---|
| **`d_demo.ts`** (19KB) | **STB cutscene PLAYBACK** — a working player for the same .stb format History decoded by hand (awake.stb, JMSG/JSND/JACT). Reference for demo semantics, camera cuts, actor tracks. |
| **`Grass.ts`** | the kusa/flower system — reference for grass SWAY (§130 HUNT 1) |
| **`d_wood.ts`** (30KB) | **the swood tree system** — Outset's unported P1 wood item has a working reference |
| `d_kankyo.ts` / `d_kankyo_wether.ts` | palettes (§143-verified) / weather + windline (§130 HUNT 2) |
| `d_a_sea.ts` (31KB) | system-1 ocean (the far-ocean hole, §140) |
| `d_a.ts` (304KB!) + `LegacyActor.ts` + `d_bg.ts` | prop/NPC/placement RENDERING — how each placed actor type presents |
| `d_stage.ts` / `d_resorce.ts` / `d_particle.ts` | stage/room loading semantics, resource system, JPA particles |

**DOES NOT HAVE:** gameplay logic, AI/pathing (user correctly excluded), collision response,
JAudio synthesis, save/flags, combat — the systems where our fork does original receiver work.

**Net for the user's question:** "instant read on every cutscene" = *almost* — the cutscene DATA
still lives in the game files (our extraction), but `d_demo.ts` is a WORKING interpretation of the
STB format — every track type's playback semantics, readable. Same for wind (yes), props (yes,
extensively). The rule stands: [reference-implementation] — disambiguate and cross-check against
it; the decomp remains law.

---

## FULL CAPABILITY ANALYSIS (§152, 2026-07-26 — source-verified vs Main.ts)

User-found, source-confirmed capabilities beyond the §145 inventory:
| capability | implementation | use for us |
|---|---|---|
| **LAYER VIEWING** (room + demo/cutscene layers) | `WindWakerLayer` class + Scenarios UI panel; per-layer visibility bits (`objectLayerVisible`) | **History: inspect each ACT layer's placements in isolation** — visual census per layer; demo-layer views show what a cutscene's room-state looks like. (Corrects fast-track Limit №1.) |
| **Render hacks** | checkboxes: Enable Vertex Colors / Enable Textures / Enable Objects / Wireframe | **Debug isolation on REFERENCE data:** e.g. toggle vertex colors on Room44's water to SEE what VTX alpha contributes (trap #3 verification by eye); textures-off shows raw material color (the palette layers isolated) |
| **Time of day** | `TimeOfDayPanel` slider driving `g_env_light.curTime` (t*360) + optional real-clock dynamic mode | already our §143/§147 acceptance instrument — confirmed it drives the REAL ported kankyo (curTime→palette blend), not a shader fake |
| Rooms panel | per-room visibility | multi-room stages: isolate a room's contribution |

**Draw-fidelity scope (the user's "what does it take into account" question, answered honestly):**
noclip implements the GX TEV pipeline in shaders (konst/registers/stages — high material fidelity;
that's why our §143 checks against it were meaningful), the kankyo palette system natively
(d_kankyo.ts), BTK/BRK/BCK animation, and JPA particles. It does NOT emulate: GC framebuffer
tricks exactly (EFB copies partial), Z-precision quirks, performance characteristics (its draw
batching is modern-GPU — NEVER use noclip as an FPS/perf reference, cf. §151's aurora immediate-
mode cost), audio, or game logic. **Rule refined: presentation semantics = trustworthy reference;
performance + activation logic = never.**
