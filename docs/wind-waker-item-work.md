# Wind Waker item work — handoff & roadmap

**Purpose:** Track **WW-era leftovers on the TP disc** — primarily `itemmdl.arc` item view models, plus related cut-enemy / orphan-asset research (Moblin, Shadow Beast prototype). Start new chats with:

> Continue Wind Waker / cut-content work — read `docs/wind-waker-item-work.md` first.

**Two-chat handoff (Cursor ↔ Claude):** this feature runs as parallel chats — **Cursor (Wind Curs)** implements, **Claude (Wind Clau)** reviews. Both are handed off via [Interconnected Chats/Wind Curs-Wind Clau.md](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md):
> - **Implementation chat →** that doc's **▶ RESUME HERE**
> - **Review chat →** that doc's **▶ WIND CLAU HANDOFF**
>
> Trust **this doc + the git log** as the current technical state — a prior chat's summary may lag the latest color-tuning commits.

**Status (2026-07-01):** **Bow BOTH tracks working & committed.** Track A get-item — 4a authentic SC TEV replay + 4b K0 cap/output ceiling (`888f69392d` and prior). Track B **held skin** — vbow renders fully colored on Link's held bow (`game.wwItemmdlHeldBow`), committed `888f69392d`. **The bow is now a proven end-to-end reference — see [▶ PROVEN PATH](#-proven-path--bringing-a-ww-itemmdl-item-to-screen) below.** Next item: **Skull Hammer (`vhamm`)** — get-item generalization (no TP host for held). Cut-enemy lineage documented; beta Moblin **not found** on retail GZ2E01 decomp map.

---

## ▶ PROVEN PATH — bringing a WW `itemmdl` item to screen

**Reference implementation: `vbow` (Hero's Bow), both layers shipped.** Two presentation layers; the render core is shared and item-agnostic.

### Layer A — Get-item ("You got it!" spin)
1. **Heap/arc swap:** branch `daItemBase_c::CreateItemHeap` on the item id → `dWwItemmdl_getVbowModelData("itemmdl")` + `dWwItemmdl_patchModelForPc` (`d_a_itembase.cpp`).
2. **Demo draw override:** `daDitem_c::setTevStr` for the WW mesh = `settingTevStruct(0)` + `dWwItemmdl_setWwBowActorAmbient` + `applyBowMaterialAmbientOnly` (**no MAJI, no struct-14**); skip `dKy_efplight_set` + GETITEM beam particles (`d_a_demo_item.cpp`, `d_demo.cpp`).
3. **PC material realization:** 2N′ bake-once locked DL + **2B‴ persistent `MatDrawPostDl` callback** — per-material texgen + TEV order; SC gets the **4a full TEV/konst replay**; body ambient-only fill (`d_ww_itemmdl_pc.cpp`).
4. **Color tuning:** body ambient cap; SC **K0 cap** (→ matte silver) + **output ceiling** (bloom threshold). Live via Editor sliders.

### Layer B — Held skin (equipped in Link's hand) — THE TEMPLATE
1. **Arc residency:** `dWwItemmdl_tickHeldBowArcMount()` per play-tick (`d_s_play.cpp`) so the private itemmdl arc stays mounted (no get-item replay needed).
2. **Model swap:** in the item's `set*Model()` → `getVbowModelData` + `patchModelForPc` + **`initModel(data, 0x80000, 0x11000084)`** (mdlFlags `0x80000` matters) (`d_a_alink_bow.inc`).
3. **Draw override:** `daAlink_c::modelDraw` branch for the WW model — **skip MAJI**, `settingTevStruct(0)` + warm ambient + **`mDoExt_modelUpdateDL`** (not `modelEntryDL`) (`d_a_alink.cpp`).
4. **Scope stays alive to DRAIN:** `beginBowDrawScope` re-point each frame, `clearBowDrawScope` only on item-switch — the realization callback fires at draw-buffer drain, **not** during `modelUpdateDL`. (This was the last blocker.)
5. **Rig:** skip the item's aim/hold BCK when it targets a different skeleton (stiff but functional); expose a scale-% slider; glow reuses the shared K0 / output-ceiling sliders.

**Shared core (A3/A4 == B3/B4):** retained itemmdl arc + heap, the 2B‴ callback, struct-0/ambient lighting, and the SC realization are all **item-agnostic** once the item-id + material-name guards are widened.

### To add a NEW item
- **Get-item:** branch `CreateItemHeap` on the new id; point at the item's BDL index (`itemmdl.h`).
- **Held:** branch the item's `set*Model()` + `modelDraw` with the Layer-B recipe — **only if the item has a TP host** to equip (bow, boomerang, hookshot, boots, bombs, bottles, telescope→hawkeye). No-TP-analogue items (Skull Hammer, Deku Leaf, Sail, WW baton, Grappling Hook, Tingle Tuner, Picto Box, bags) are **get-item / viewer only** unless you write new procs.
- **Generalize the guards:** `isTevDumpMaterial` / `isVbowDrawMaterial` currently hardcode `Vbow_v` / `SC_Vbow_v`; widen to the new item's material names (or match any itemmdl material). Tune ambient/scale; wire BTK/BRK if present.

### Generalization progress (2026-07-01) — render core is now item-agnostic
- **`dWwItemmdl_getItemmdlModelData(u16 bdlIndex)`** — generic per-index loader off the same private arc, cached per-index; bow reuses its proven path. Entry point for every other item.
- **Material guards generalized:** `isTevDumpMaterial` accepts any material of a scoped itemmdl model; `isScVbowDrawMaterial` → any **`SC_`-prefixed** material (WW ink-pass convention). Bow behavior byte-identical.
- **Still bow-specific (next):** `daItemBase_c::CreateItemHeap` id-branch + `d_a_demo_item.cpp` `useWwItemmdlBow*` gate (both keyed to `dItemNo_BOW_e`); per-item color constants.

### Next item — Skull Hammer (`vhamm`)
- **BDL:** `dRes_INDEX_ITEMMDL_BDL_VHAMM_e` (`itemmdl.h`); TWW arc `Vhamm`; **no BTK/BRK**.
- **No TP item id** (TP has no Skull Hammer; iron ball is unrelated) → the get-item flow (keyed by `dItemNo`) and held-skin (needs a TP host) **cannot** trigger it. ⇒ **Test vehicle = a VIEWER / index spawn** using `dWwItemmdl_getItemmdlModelData(VHAMM)` + the realization draw scope, drawn at Link's feet. This is Phase 3 (ImGui itemmdl viewer) or a generalized demo-spawn that takes a BDL index instead of a `dItemNo`.
- **This is the general answer for the ~13 no-TP-analogue items** (hammer, Deku Leaf, Sail, WW baton, grappling hook, Tingle Tuner, Picto Box, bags): viewer-only until new procs exist. TP-analogue items (boomerang, hookshot, boots, bombs, bottles, telescope) can use the get-item + held paths.

### ✅ VIEWER RESULTS (2026-07-01) — all 21 meshes spin
`game.wwItemmdlViewerBdlIndex` + Replay drew **all 21** WW meshes through the proven get-item path — the generalized render core works across the whole set. Per-item color varies under the shared recipe:
- **Color-perfect, ≤ bow brightness:** **Iron Boots** (`vboot` 0xE), **Hookshot** (`vhook` 0x14) — correct, not over-bright.
- **Colored but angle-bright:** several swing nice-color ↔ washed-white at certain camera angles (view-lit on bright texels — per-item ceiling/ambient tune).
- **~1 off-color** item (TBD which).
⇒ render core proven for all 21; remaining work is **per-item** color tuning, not architecture.

### Next skin candidates (both TP-hosted → held works)
- **Iron Boots** `vboot` 0xE → `dItemNo_HVY_BOOTS_e`, and **Hookshot** `vhook` 0x14 → `dItemNo_HOOKSHOT_e` — already color-perfect in the viewer + have TP equip slots. Prime **held-skin** targets: apply the held template to their model setters (analog of `setBowModel`), per-item scale/offset.

### Related thread — King Bulblin axe as a Link weapon (research; see interconnected doc)
- **Enemy** King Bulblin = `E_rdb` (`0x1D5`); **weapon** `RB_ONO` (`dRes_INDEX_E_RDB_BMD_RB_ONO_e=0x55`, "ono"=axe, single-joint prop). Phase-1 reskin over Link's sword = feasible via the held template; authentic Bulblin swings not portable (32-bone RD rig ≠ Link). Full feasibility + integration options in [Wind Curs-Wind Clau.md](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md).

---

## Track A progress (2026-06-26 → 2026-07-01)

> **Canonical session log:** [Interconnected Chats/Wind Curs-Wind Clau.md](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) — **▶ RESUME HERE** for fresh chats.

| Milestone | Status |
|-----------|--------|
| `game.wwItemmdlGetItem` toggle + Editor replay button | Done |
| `itemmdl.arc` / `vbow.bdl` load on PC (`BDLM` + `bdl3` in retail TP `res/Object/`) | Done |
| Get-item demo spawns WW bow mesh (2 materials, 3 embedded textures) | Done |
| **2N′** bake-once locked DL + **2B‴** per-mat texgen/TEV post-`callDL` | Done |
| **struct-0** + ambient-only (no MAJI) — stable cel color + detail | Done (`72a2f01194` 4B, `6023333a8b` 4D) |
| Fixed warm ambient + per-mat caps + SC absolute ink | Done (4D/4E in `d_ww_itemmdl_pc.cpp`) |
| Skip efplight + skip demo GETITEM beam particles (WW bow only) | Done (4D/4E in `d_a_demo_item.cpp`, `d_demo.cpp`) |
| WW cel look vs reference (goldenrod body, cream tips, subtle ink) | **~85%** — **4a** SC TEV replay restored detail (root fix, generalizes). Two gaps left: SC caps read **white** (want matte **silver** — temper st[2] `+HALF`); body **too bright/flat** (cap `80→70`, opt. Vbow_v TEV replay for amber gradient) |
| BTK spin (`dRes_INDEX_ITEMMDL_BTK_VBOW_e`) | **Deferred** — after color sign-off |
| Track B held bow | **WORKING (fully colored, 2026-07-01)** — vbow skin on Link's held bow via `game.wwItemmdlHeldBow` + live scale %. **This is the reusable template for held versions of all 21 items** (see [Wind Curs-Wind Clau.md → ★ THE HELD TEMPLATE](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md)). Uncommitted; polish = double-arrow, scale tune, strip diagnostics |

**Git milestones (main):**

| Commit | Summary |
|--------|---------|
| `e0374888ed` | 2N′ geometry + Vbow_v texgen bind |
| `ace05ce875` | TWW draw recipe + persistent 2B mat hook |
| `72a2f01194` | 4B ambient-only stable cel |
| `6023333a8b` | 4D baseline (fixed warm amb, cap, SC ink, skip efplight) |
| `d4f7066637` | 4E polish (warmer/dim, darker ink, skip beams, SC A/B toggle) |

**Where the art lives:** All color data is **on-disc in TP** — `itemmdl.arc` → `vbow.bdl` ships with embedded textures (no external WW game files). Nothing is missing from the repo; Dusk loads the same retail archive as vanilla TP would, it was just never wired up.

**PC rendering notes (`src/d/d_ww_itemmdl_pc.cpp`, `src/d/actor/d_a_demo_item.cpp`):**

- `vbow.bdl` is a **binary display list** model (`bdl3`). Default loader used locked materials with null TEV blocks (baked WW cel math only) — fine on hardware, breaks under Aurora + TP get-item lighting.
- **2N′** `bakeLockedMaterialSharedDl()` once at parse; single `mDoExt_modelUpdateDL` per frame.
- **2B‴** persistent `MatDrawPostDlCallback` — per-mat texgen+TEV after `callDL` (`Vbow_v` body + `SC_Vbow_v` ink both draw).
- **Lighting:** struct **0** + `dWwItemmdl_setWwBowActorAmbient` + `dWwItemmdl_applyBowMaterialAmbientOnly` — **no MAJI**, **no struct 14**. Skip `dKy_efplight_set` for WW bow.
- **4E constants:** body `(105,78,48)` cap **80**; SC ink `(58,48,42)`; A/B toggle **`game.wwItemmdlBowScSuppress`** (Editor, default off).
- **Beams:** skip `ID_IT_JN_GETITEM_*` in `dDemo_particle_c::emitter_create` while WW bow demo item lives.
- Material names: **`Vbow_v`** = body (gold limbs + arrow shaft). **`SC_Vbow_v`** = **second cel pass carrying real geometry** — white/silver limb-tip caps + teal bands, silver arrowhead, string/nock art, back-half feather — **plus** ink lines. **Not** an optional outline: suppressing it amputates those parts (A/B proven 2026-07-01). Ship it **on**; de-bloom via **TEV/blend**, not ambient (dark SC ambient `(58,48,42)` does not tame the bloom — the 2B‴ hook replays texgen+TEV order only, not TEV color/konst/blend). See [Wind Curs-Wind Clau.md → `▶ SC_Vbow_v is a geometry+TEV pass`](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md).

**Debug log:** `Documents/dusklight/albw_ww_itemmdl_debug.txt`

**Related (do not duplicate here):**

| Doc | Overlap |
|-----|---------|
| [boss-fights-handoff.md](boss-fights-handoff.md) | Other on-disc unused assets (e.g. Armogohma crawl BCKs) — boss scope only |
| [combat-refinements-handoff.md](combat-refinements-handoff.md) | WW *mechanics* references (Hurricane Spin / `procCutRoll`) — not item meshes |
| [trailer-handoff.md](trailer-handoff.md) | Trailer PNGs; optional use of in-game `itemmdl` props for B-roll once viewer exists |
| [shield-ordon-quest-assets.md](shield-ordon-quest-assets.md) | TP shield quest asset IDs — different item pipeline |
| [build-fps-guidelines.md](build-fps-guidelines.md) | Build / launch hygiene before playtesting |
| [Interconnected Chats/Wind Curs-Wind Clau.md](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) | **Cursor ↔ Claude** implementation roadmap + session log |
| [Blender-WW-Items.md](Blender-WW-Items.md) | **Asset-side** (Blender) handoff — re-rig `vboot` for iron boots, extract WW arrow from `vbow`; SuperBMD/GCFT workflow + `bpy` scripts |
| [zeldaret/tww](https://github.com/zeldaret/tww) | WW decomp — Rosetta stone for `d_item_data` rows and item behavior (not linkable object code) |
| [zeldaret/ss](https://github.com/zeldaret/ss) | SS decomp — Wii-era lighting / `MA**` palette lineage; **not** WW ink or J3D item path (see SS section) |

---

## What `itemmdl` is

- **Archive:** `itemmdl.arc` on retail TP disc (TCRF unused models #547).
- **Contents:** 21 WW “view” item BDLs (`v*` prefix) + BTK/BRK on some entries.
- **Header:** `assets/GZ2E01/res/Object/itemmdl.h` (all regional copies match).
- **Retail TP code:** **Zero** `resLoad("itemmdl")` in `src/` — orphaned after TP moved held items to `ALANM` / item actors.

WW shipped the **same meshes** as **separate per-item arcs** (`Vbow`, `Vboot`, …), wired through `d_item_data` → get-item / field pickup only. Equipped gameplay used **`LINK` / `LKANM` rigs** (e.g. `LINK_BDL_BOW`), not the `v*` props. TP’s `itemmdl` bundle is a consolidated leftover of that presentation layer.

---

## All 21 meshes (`itemmdl.h`)

| Mesh | WW item | TP analogue (if any) |
|------|---------|----------------------|
| `vbage` | Spoils Bag | None (UI/spoils only) |
| `vbagf` | Bait Bag | Fishing bait (no bag item) |
| `vbagh` | Delivery Bag | Mail system |
| `vbinb` / `vbing` / `vbinr` / `vbinx` | Bottle variants | Bottle system |
| `vbomb` | Bomb | Bomb bag |
| `vboom` | Boomerang | `dItemNo_BOOMERANG_e` |
| `vboot` | Iron Boots | `dItemNo_HVY_BOOTS_e` |
| `vbow` | Hero’s Bow | `dItemNo_BOW_e` (0x43) |
| `vboxn` | Picto Box | None |
| `vchin` | Tingle Tuner | None |
| `vhamm` | Skull Hammer | Iron ball (different weapon) |
| `vho` | Sail | None |
| `vhook` | Hookshot | Hookshot / Clawshot |
| `vleaf` | Deku Leaf | None |
| `vmags` | Magic Armor | Magic armor (Link body BRK, not mesh) |
| `vrope` | Grappling Hook | None |
| `vtact` | Wind Waker baton | None |
| `vtele` | Telescope | Hawkeye |

**Usability summary:**

- **Get-item / field pickup / debug viewer:** All 21 — correct layer.
- **Held equip skin:** Poor fit (2-bone props vs `AL_BOW`-class rigs); gameplay still works but looks stiff.
- **Full WW behavior:** Requires new or ported procs from TWW decomp, not mesh alone.

---

## TWW decomp reference ([zeldaret/tww](https://github.com/zeldaret/tww))

Rosetta stone for how WW **intended** view-item presentation to work. TP’s `itemmdl.arc` is a bundled leftover of the same meshes; TWW never loads an `itemmdl` archive — it uses **one arc per item** (`Vbow`, `Vboot`, …) via `d_item_data::item_resource[]`.

### Bow get-item row (primary pilot)

TWW item id for bow is **`0x27`** (TP uses **`0x43`** — different enum layout).

| Field | TWW retail | TP `itemmdl` (Dusk swap) |
|-------|------------|---------------------------|
| Arc | `"Vbow"` | `"itemmdl"` |
| BDL index | `dRes_INDEX_VBOW_BDL_VBOW_e` = **0x4** | `dRes_INDEX_ITEMMDL_BDL_VBOW_e` = **0xF** |
| BTK (spin) | `dRes_INDEX_VBOW_BTK_VBOW_e` = **0x7** | `dRes_INDEX_ITEMMDL_BTK_VBOW_e` = **0x24** |
| BRK / BCK | none | none |
| Heap | `0x3000` | TP demo solid-heap path |
| `mUnknown` | `0x11000222` | — |

Sources: [tww `d_item_data.cpp` bow row](https://github.com/zeldaret/tww/blob/main/src/d/d_item_data.cpp), [tww `Vbow.h`](https://github.com/zeldaret/tww/blob/main/assets/GZLE01/res/Object/Vbow.h).

**Joints:** 2 — `ROOT_VBOW` + `VBOW_MODEL` — matches TP `itemmdl.h` `VBOW_JNT_*`.

**BTK:** TWW always attaches bow BTK for get-item spin. Dusk has BTK index wired in `d_a_itembase_static.cpp` but **not enabled** in `CreateItemHeap` yet — enable after color baseline is stable.

### Draw / lighting path (why PC colors break)

TWW demo items **do not** override tev setup. They inherit `daItemBase_c::DrawBase()` → `setTevStr()`:

- `g_env_light.settingTevStruct(TEV_TYPE_ACTOR, …)` — **`TEV_TYPE_ACTOR` = 0**
- `g_env_light.setLightTevColorType(mpModel, …)` — **not** `setLightTevColorType_MAJI`
- `dComIfGd_setListMaskOff()` in `setListStart()` — **not** TP’s dark list

TP `daDitem_c` overrides with `settingTevStruct(14, …)` + `setLightTevColorType_MAJI`. Struct 14 is get-item-specific (dark ambient ~24,24,24, view-matrix lighting) — built for TP `O_gD_bow`, not WW cel BDLs. This is the strongest decomp-backed explanation for **dark/muddy fill** and **white edge shards** on `SC_Vbow_v`.

TWW **never** unlocks materials or calls `diff()` for items — locked BDL + `mDoExt_modelUpdateDL` with create flags **`0, 0x11020203`**. BDLM loader on GC uses `loadBinaryDisplayList(..., 0x00002020)` (simpler than TP PC `0x59022010` + `DoBdlMaterialCalc`; hardware TEV matched baked DL without Aurora patching).

### Demo presentation (TWW vs TP)

TWW `daDitem_c` ([`d_a_demo_item.cpp`](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_demo_item.cpp)):

- **Billboard spin:** inverse view matrix + fixed X tilt (`0x12C0`); no TP `settingEffectLight` / joint AABB height.
- **Rotation:** `fopAcM_addAngleY` with `0x0111` per frame (same family as TP).
- **Particles:** camera-aligned get-item flash/halo/star (`m_effect_type[dItemNo_BOW_e] == 0`).
- **No** struct-14 tev override anywhere in demo item source.

Held bow on Link still uses **`LINK_BDL_BOW`** ([`d_a_player_bow.inc`](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_bow.inc)), never `Vbow` — same as TP Track B expectation.

### All 21 `itemmdl` meshes → TWW arcs

Use when extending beyond bow; indices in `itemmdl.h` are TP-bundle layout, not TWW per-arc layout.

| `itemmdl` BDL | WW item | TWW arc | BTK | BRK | Notes |
|---------------|---------|---------|-----|-----|-------|
| `vbage` | Spoils Bag | `VbagE` | — | — | |
| `vbagf` | Bait Bag | `VbagF` | — | — | |
| `vbagh` | Delivery Bag | `VbagH` | — | — | |
| `vbinb` | Bottle (blue) | `Vbin*` family | varies | — | TWW splits bottles by content/color |
| `vbing` | Bottle (green) | `Vbin*` family | varies | — | |
| `vbinr` | Bottle (red) | `Vbin*` family | varies | — | |
| `vbinx` | Bottle (special) | `VbinX`-style | varies | — | |
| `vbomb` | Bomb | `Vboml` (`VBOMM`) | yes | — | bomb **bag** mesh |
| `vboom` | Boomerang | **`Boom`** | — | — | not `Vboom` |
| `vboot` | Iron Boots | `Vboot` | yes | — | |
| **`vbow`** | **Hero's Bow** | **`Vbow`** | **yes** | — | **Track A pilot** |
| `vboxn` | Picto Box | `VboxN` | yes | — | |
| `vchin` | Tingle Tuner | `Vchin` | — | — | |
| `vhamm` | Skull Hammer | `Vhamm` | — | — | |
| `vho` | Sail | `Vho` | — | — | |
| `vhook` | Hookshot | `Vhook` | yes | — | |
| `vleaf` | Deku Leaf | `Vleaf` | — | — | |
| `vmags` | Magic Armor | `VmagS` | yes | **BRK** | only `itemmdl` entry with BRK in TP header |
| `vrope` | Grappling Hook | `Vrope` | yes | — | |
| `vtact` | Wind Waker | `Vtact` | — | — | |
| `vtele` | Telescope | **`Tele_00`** | yes | — | arc name ≠ mesh prefix |

### Color fix order (decomp-backed, bow first)

1. ~~**WW bow branch in `daDitem_c::setTevStr()`** — struct **0** instead of struct 14 + MAJI.~~ **Done.**
2. ~~**Ambient-only material fill** — fixed warm RGB + per-mat caps (no view-matrix lights).~~ **Done (4B→4E).**
3. ~~**Skip efplight + get-item beam particles** for WW bow evaluation.~~ **Done (4D/4E).**
4. **Tune** body/ink constants vs WW reference; **SC A/B** via `kWwBowSuppressScInkPassForDraw`.
5. **Then** enable `itemmdl` BTK index **0x24** for authentic spin.
6. **Extend** same tev path to other `v*` items when scaling beyond bow.
7. **Optional:** `setListMaskOff` (TWW mask-off list) — never tried on PC.

---

## Skyward Sword decomp reference ([zeldaret/ss](https://github.com/zeldaret/ss)) — scan 2026-06-26

**Purpose:** Phase 3+ **cel / scene-lighting lineage** reference — not a substitute for [TWW](#tww-decomp-reference-zeldarettww) on WW `itemmdl` behavior. Hypothesis: SS is an evolved Wii-era stack on the same TP/TWW lighting *ideas* (actor palettes, `MA**` material slots), but with **nw4r::g3d + EGG** instead of JSystem J3D, and a **watercolor / soft-shaded** look — not WW ink outlines.

**Local scan clone:** `c:\Users\xxxxx\Documents\ss-decomp-scan` (sparse checkout, external to dusklight repo). **Decomp progress:** ~25% code matched ([decomp.dev/ss](https://decomp.dev/ss)); many item-model helpers still live under `src/toBeSorted/`.

### Engine stack vs Dusk / TP

| Layer | TP / Dusk (Track A) | SS retail |
|-------|---------------------|-----------|
| Model API | JSystem **J3D** (`J3DModel`, `bdl3` BDLM) | **nw4r::g3d** (`ResMdl`, `brres` in OARC) |
| Runtime wrapper | `mDoExt_*`, locked binary DL | `m3d::smdl_c` + G3D scene callbacks |
| Scene / heap libs | JSystem + `mDo` | **EGG** (successor; see `src/egg/gfx/`) |
| Env lighting | `g_env_light` + `TevStruct` types (0, 14, …) | `dLightEnv_c` + per-actor **`ActorLighting`** |
| Item get mesh source | Orphan **`itemmdl.arc`** bundle on TP disc | **Per-item OARC** via `OarcManager::getMdlFromArc2` → `g3d/model.brres` |

`d_light_env.cpp` explicitly notes TP lineage (`vectle_calc` copied from TP `d_kankyo_rain`). Same *family*, different implementation.

### Item model architecture (parallel concept to TWW, not TP `itemmdl`)

SS never consolidated view items into one arc. Bow get meshes are separate resources:

| SS (`item_mdl_item.cpp`) | Notes |
|--------------------------|-------|
| `GetBowA` | Base bow get-item model |
| `GetBowB` / `GetBowC` | Iron / Sacred bow upgrades |
| `resFile` column in `sMdlConfig[]` | OARC basename (e.g. treasure-box table also references `GetBowA`) |

Routing mirrors TWW’s “one arc per item” more than TP’s bundled `itemmdl`:

- **`item_mdl_name.cpp`** — `ITEM_*` → `ITEMMDL_*` table (512 slots); same *role* as TWW `d_item_data` indices.
- **`d_a_item.cpp::createHeap()`** — picks a **`dItemMdl_*`** subclass by item id: `Put`, `Shield`, `Bottle`, `Tear`, `Firefly`, `Potion`, `Item`, `Rupee`, `LightFruit`, `StaminaFruit`.
- **`dItemMdlItem_c`** — generic get/put props (bow, sword, beetle, …); loads `nw4r::g3d::ResMdl` by name from `brres`.

No `SC_*` / ink-edge material split appears in the decompiled item_mdl sources — consistent with SS’s non–Wind-Waker aesthetic.

### Get-item vs field draw path

SS folds presentation into **`dAcItem_c`** (no TP-style separate `daDitem_c` actor):

| Mode | Set by | Draw |
|------|--------|------|
| Field / put | `DrawMode_Put` | `mpItem->fn_8002ECD0(&mMdl, **7**)` |
| Get-item demo | `DrawMode_Get` (+ `mMdlScaleType == 2`) | `fn_8002ECD0(&mMdl, **6**)` |

Get-demo states (`WaitGetDemo`, `GetDemo`, `WaitForcedGetDemo`, …) all call `mpMdl->setDrawMode(DrawMode_Get)` before `mpMdl->draw()` → `d_a_item.cpp` `draw()`.

**Not decompiled yet:** `dAcObjBase_c::fn_8002ECD0` (symbol `0x8002ECD0`, size `0x50`) and `drawModelType1` — the actual GX draw-list / lighting hook for types **6** and **7** is still opaque. Do **not** assume they map to TP `settingTevStruct(14)` or TWW struct **0**.

### TEV / lighting observations (what SS teaches Dusk)

**Decompiled and useful:**

- **`dScnCallback_c::ExecCallback_CALC_MAT`** — before draw, walks `ResMdl` materials and writes **`ActorLighting`** tev colors (`mTev0Color`, `mTev1Color`, `mTevK*`, …) into `ResMatTevColor`. Runtime per-material TEV injection, not baked-DL unlock.
- **`d_d3d.cpp::setRoomTevColors` / `unk2`** — room palette pushes **`MA00`–`MA03`** kColor / tevReg values from `dLightEnv_c::GetCurrentDefaultMcf()` into materials by **material name prefix** (`MA0x`). Same naming convention as WW/TP cel material slots.
- **`ActorPalette`** in `d_light_env.h` — scene ambient/diffuse + extended tev palette blocks; superset of TP `g_env_light` actor palette idea.
- **BG lighting codes** — `dBgS::GetLightingCode` feeds mass-object / actor lighting (poly attribute driven), separate from demo-item struct 14.

**Does *not* translate 1:1 to Aurora J3D Track A:**

- No `TEV_TYPE_ACTOR` / `setLightTevColorType_MAJI` API — different structs and callback timing.
- SS items use **G3D copied mat access** + scene callbacks; Dusk WW bow stays on **locked J3D binary DL** + `J3DMLF_DoBdlMaterialCalc` (see PC rendering notes above).
- SS soft shading ≠ WW `Vbow_v` + `SC_Vbow_v` two-pass ink.

### Relevance matrix (Dusk WW `itemmdl` Track A)

| Question | Best reference | SS scan answer |
|----------|----------------|----------------|
| Fix dark/muddy WW bow fill on get-item | **TWW** `daItemBase_c::setTevStr` (struct **0**) | SS uses different lighting objects; no struct-14 analogue found |
| Where did `MA**` material names come from? | SS + TWW + TP | SS `unk2` confirms continued `MA00`–`MA03` palette routing in Wii era |
| Should we port SS draw callback for items? | **No** for Phase 1 | Wrong engine; keep J3D locked-DL path + tev struct fix |
| Per-item arc layout / index tables | TWW primary; SS secondary | SS `item_mdl_name` + `sMdlConfig[]` good for *workflow* patterns, not TP indices |
| Ink outline pass | TWW `SC_*` materials | SS item_mdl code shows no equivalent edge pass |

### Key SS files to re-read as decomp grows

| Area | Path (in zeldaret/ss) |
|------|------------------------|
| Item actor + get-demo FSM | `src/d/a/d_a_item.cpp` |
| Model class vtable | `include/toBeSorted/item_mdl.h` |
| Bow / generic get models | `src/toBeSorted/item_mdl_item.cpp` |
| Item → model index | `src/toBeSorted/item_mdl_name.cpp` |
| Put vs get dual models | `src/toBeSorted/item_mdl_misc.cpp` (`dItemMdlPut_c`) |
| OARC → brres load | `src/toBeSorted/arc_managers/oarc_manager.cpp` |
| Scene TEV color inject | `src/toBeSorted/d_scn_callback.cpp` |
| Room MA palette | `src/toBeSorted/d_d3d.cpp`, `src/d/d_light_env.cpp` |
| Draw helper (when matched) | `dAcObjBase_c::fn_8002ECD0`, `drawModelType1` |

### Follow-ups (not done in this scan)

- [ ] Inspect retail `GetBowA` `brres` material names on disc (MA** vs custom) — needs extracted SOUE01 Object tree, not in git.
- [ ] Match `fn_8002ECD0` / `drawModelType1` when decomp lands — may clarify meaning of draw types **6** / **7**.
- [ ] Compare SS get-item event path (`addGetEvent`, `GetDemo` state) to TP `daDitem_c` for presentation timing only (not rendering).
- [ ] Check whether any SS item uses a two-material ink pass (unlikely given art direction).

---

## Two-track experiment (bow pilot)

User-approved roadmap: try **both** presentation and held, starting with bow.

### Track A — Get-item (expect good results)

**Goal:** WW Hero’s Bow (`vbow`) during spinning “You got …!” for `dItemNo_BOW_e`.

| Item | Detail |
|------|--------|
| Hook | `daDitem_c` / `d_item_data::item_resource[dItemNo_BOW_e]` — today `"O_gD_bow"`, BMD index `0x3` |
| Swap | Arc `"itemmdl"`, BDL `dRes_INDEX_ITEMMDL_BDL_VBOW_e` (0xF), optional BTK `0x24` |
| Reference | TWW `daItemBase::CreateItemHeap`, `d_a_demo_item.cpp` |

**Test:** Toggle on → trigger bow get-item; toggle off → vanilla `O_gD_bow`.

### Track B — Held model (expect jank; gameplay OK)

**Goal:** See `vbow` parented to Link’s hand while aiming/shooting.

| Item | Detail |
|------|--------|
| Hook | `setBowModel()` in `d_a_alink_bow.inc` |
| Swap | Load `itemmdl` BDL instead of `AL_BOW` |
| Default | **Skip** TP bow BCK on swapped mesh (`BVJMPCL` targets `AL_BOW` bones) |
| Tune | Hand offsets in `d_a_alink.cpp` (~6078): `transM(-1.3, 0, -3)`, `XYZrotM(-74°, 43.6°, 1.9°)` |

**Test:** Equip bow, aim, shoot, Epona left-hand path.

---

## Implementation plan

### Phase 0 — Shared infra

1. Helper: `dComIfG_resLoad(&phase, "itemmdl")` + `getObjectRes("itemmdl", index)`.
2. Advanced Settings toggles (suggested keys):
   - `game.wwItemmdlGetItem` — Track A (**implemented** in Editor → ALBW)
   - `game.wwItemmdlHeldBow` — Track B
   - `game.wwItemmdlHeldBowNoBck` — default **on** for Track B
   - `game.wwItemmdlBowScSuppress` — SC ink pass A/B (Editor → ALBW)
3. Files: `include/dusk/settings.h`, `src/dusk/settings.cpp`, `src/dusk/ui/editor.cpp`, `src/d/d_ww_itemmdl_pc.cpp`, `src/d/d_ww_itemmdl_test.cpp`.

### Phase 1 — Track A (bow get-item)

1. Branch in `daItemBase::CreateItemHeap` when toggle on + bow (`d_a_itembase_static.cpp` → `"itemmdl"` arc). **Done.**
2. PC loader + material patch in `d_ww_itemmdl_pc.cpp`. **Done (fill pass; outline deferred).**
3. Optional BTK (`dRes_INDEX_ITEMMDL_BTK_VBOW_e`) — not wired yet (BTK disabled on WW path).
4. Playtest get-item cutscene / Editor replay. **Works with toggle; verify colors after material pass.**

### Phase 2 — Track B (held bow)

1. Branch `setBowModel()` when held toggle on.
2. Optional separate matrix scale/offset when WW held mode active.
3. Playtest combat; document screenshot-only tuning values.

### Phase 3 — Optional

- **ImGui itemmdl viewer** (Advanced → Tools): dropdown of 21 BDL indices, spawn at Link feet.
- **Field pickup** (`daItem` / `field_item_res`) for bow.
- **Extend** to hookshot, bottles, etc. using TWW `d_item_data` indices as cheat sheet.

---

## Key source files

| Area | Files |
|------|--------|
| `itemmdl` indices | `assets/GZ2E01/res/Object/itemmdl.h` |
| Get-item actor | `src/d/actor/d_a_demo_item.cpp`, `src/d/actor/d_a_itembase.cpp` |
| PC vbow load / materials | `src/d/d_ww_itemmdl_pc.cpp`, `include/d/d_ww_itemmdl_pc.h` |
| Editor replay helper | `src/d/d_ww_itemmdl_test.cpp` |
| BDLM arc loader (PC) | `src/d/d_resorce.cpp` (`BDLM` + `DoBdlMaterialCalc`) |
| Item resource table | `src/d/d_item_data.cpp`, `include/d/d_item_data.h` |
| Bow get row | `item_resource` entry `"O_gD_bow"` (~line 84) |
| Held bow | `src/d/actor/d_a_alink_bow.inc`, `src/d/actor/d_a_alink.cpp` |
| Settings | `include/dusk/settings.h`, `src/dusk/ui/settings.cpp` |
| TWW reference | [zeldaret/tww `d_item_data.cpp`](https://github.com/zeldaret/tww/blob/main/src/d/d_item_data.cpp), `daItemBase`, `d_a_player_bow.inc` (`LINK_BDL_BOW` — not `Vbow`) |

---

## Test checklist (bow)

- [ ] Get-item off → TP `O_gD_bow` appearance
- [x] Get-item on → WW `vbow` visible in get-item spin (mesh + anim)
- [ ] Get-item on → WW `vbow` **texture colors** readable (not dark / white shards)
- [ ] Get-item on → optional BTK spin
- [ ] Held off → vanilla `AL_BOW`
- [ ] Held on + No BCK → stiff WW mesh, aim/shoot works
- [ ] Held on + BCK → document deformation (likely useless)
- [ ] Epona / left-hand bow path
- [ ] Both toggles off → fully vanilla
- [ ] No crash on `itemmdl` load/unload

---

## Cut enemies & WW lineage (Moblin, Shadow Beast)

Same engine inheritance story as `itemmdl`: TP kept **disc assets** from the WW era but often **rewired or dropped** the code path. [zeldaret/tww](https://github.com/zeldaret/tww) is the Rosetta stone for what WW *meant* each enemy to be.

### WW: Bokoblin and Moblin are separate actors

| WW (TWW decomp) | TP retail |
|-----------------|-----------|
| **Bokoblin** — `d_a_bk.cpp`, arc `Bk` | Merged into **`E_OC`** (`daE_OC_c`), arc `E_oc` |
| **Moblin** — `d_a_mo2.cpp`, arc `Mo2` (comment: *Enemy - Moblin / モ２*) | **No `Mo2` / `Bk` / `Boko` headers on GZ2E01 disc** |
| **Weapons** — child `daBoko_c` props (stick, machete, Moblin spear, …) | Welded **`OC_WEPON` / `OC2_WEPON`** meshes on the orc body |

WW Moblin has its own AI (`ACTION_E3_DEMO`, defence, carry, jab/punch BCK set). TP collapsed both orc lines into **one** proc (`fpcNm_E_OC_e`) with an internal mesh variant:

| Variant | Select | Still a bokoblin? |
|---------|--------|-------------------|
| Club | `E_OC` / angle high byte `0` or `0xFF` | Yes — shipping field enemy |
| “Alt mesh” | `E_OC2` / angle high byte non-zero (e.g. `0x100`) | **Yes** — same skeleton + `E_ocb` anims; 220 HP + `NATA_*` SFX only |

**Trailer beta Moblin** (E3 footage: taller, big axe/hammer, own test room) is **not** `E_OC2` (confirmed in-game). Retail decomp has **no second orc actor** and **no WW `Mo2` arc**. Likely **fully cut before GZ2E01** or only present in pre-release builds — not recoverable via spawner angle tricks.

### Shadow Beast: actor vs prototype arc

| Layer | Arc | Retail `src/` |
|-------|-----|----------------|
| **Shipped twilight enemy** | `E_S2` (no jaw/tongue; ceiling-hang anims) | `daE_S1_Create` always `resLoad("E_S2")` |
| **Early prototype model** | `E_s1` (jaw + tongue joints; `S1_HOOK`, `S1_PULL`, `S1_DEFEND`) | **Never** `resLoad` / `getObjectRes("E_s1")` — audio/stage still say `"E_s1"` |

Spawn **`E_S1`** today → **shipped** beast (`E_S2` mesh). To see the **prototype**, need a dev fork in `d_a_e_s1.cpp` (arc swap + anim index / joint remap), not spawner-only work.

**Actor spawner notes:** [ImGuiActorSpawner](../src/dusk/imgui/ImGuiActorSpawner.cpp) presets label these honestly (`Bokoblin alt mesh`, `Shadow Beast shipped`). See also cut-enemy presets tooltip for what is **not** spawnable as-is.

---

## Finding orphan arcs (methodology)

**Orphan** = on-disc object archive (or header in `assets/GZ2E01/res/Object/`) with **no live gameplay load path**, or a **superseded** arc the code stopped using.

### Pass 1 — Decomp static scan (fast, imperfect)

Compare `assets/GZ2E01/res/Object/*.h` basenames to quoted strings in `src/`:

```text
resLoad / getObjectRes / resDelete / dComIfG_resLoad / OBJNAME("...")
```

**Caveats (false positives / negatives):**

| Pitfall | Example |
|---------|---------|
| Dynamic arc name | `E_OC2` loaded via `mName = "E_OC2"` in `d_a_e_oc.cpp` — literal grep for `"E_oc2"` misses it |
| Audio/stage only | `"E_s1"` in `d_stage.cpp` / Z2 tables — **not** the same as loading the `E_s1` **model** arc |
| Parent loads child | `E_ib` loaded through `E_ba` actor’s `arc_name[]`, not its own string |
| Header missing | True cut BMD on disc with **no** decomp `.h` yet — static scan cannot see it |

**Repeatable helper (local, gitignored):** `local_dev_backup/scan_orphan_arcs.py` — run from repo root:

```powershell
python local_dev_backup/scan_orphan_arcs.py
```

**GZ2E01 scan snapshot (2026-06):**

| Arc | Header on disc? | Strict literal `resLoad`? | Notes |
|-----|-----------------|---------------------------|--------|
| `itemmdl` | Yes | **No** | 21 WW item BDLs — this doc’s main track |
| `E_s1` | Yes | **No** (actor loads `E_S2`) | Prototype Shadow Beast mesh; `"E_s1"` still in stage/audio |
| `E_oc` / `E_oc2` | Yes | Misses naive grep | Loaded via runtime `mName` (`"E_OC"` / `"E_OC2"`) in `d_a_e_oc.cpp` |
| `Bk` / `Mo2` / `Boko` | **No** | — | WW boko/moblin/weapon arcs **not** on retail TP map |
| ~1154 / 1397 headers | — | Strict literal-string orphans | Includes many dynamic loads — **filter by hand** |

**High-confidence cut-content orphans (manual verify):** `itemmdl`, `E_s1` (prototype beast).  
**Not orphans:** `E_oc2` (bokoblin alt mesh via `"E_OC2"`).  
**Not on disc:** trailer beta Moblin (`Mo2` lineage).

### Pass 2 — Disc extraction (needed for beta Moblin)

Decomp headers only cover **mapped** archives. To hunt the trailer Moblin:

1. Extract full `Object/` from **GZ2E01** (WIT, QuickBMS, or your existing TP asset pipeline).
2. List `.arc` names — diff against `assets/GZ2E01/res/Object/*.h` basenames (**unmapped** = highest priority).
3. Inside suspect humanoid arcs: look for **BMD** + **BCK** with orc-scale joint counts; compare joint names to `E_oc` (29 joints) vs WW `Mo2` (different rig — pull from TWW disc if needed).
4. Cross-check TCRF / Unseen64 screenshots; match weapon mesh (axe/hammer vs spear vs club).

### Pass 3 — In-engine viewer (dusklight dev)

Once an arc name is known:

- **Minimal:** extend Phase 3 **ImGui object viewer** (same infra as `itemmdl`) — type arc + BMD index, spawn at Link’s feet.
- **Cut enemy:** fork actor create path (Shadow Beast → `E_s1`; Moblin → new `daE_*` or import rig).

### What we can do **right now** in this repo

| Action | Status |
|--------|--------|
| Document WW→TP enemy consolidation | Done (this section) |
| Confirm `itemmdl` + `E_s1` prototype are strict orphans | Done (`grep` + scan script) |
| Confirm **no** WW `Mo2`/`Bk` on GZ2E01 headers | Done |
| Spawn trailer Moblin | **Blocked** — no arc/actor identified on retail |
| Spawn prototype Shadow Beast | Needs **`d_a_e_s1.cpp` dev toggle** (medium code task) |
| Full disc orphan diff | **Needs your extracted GZ2E01 Object tree** (not in git) |

If you drop an extracted `Object/` listing (or point at an existing asset dump path), the next step is a **mapped vs unmapped arc diff** filtered for humanoid / enemy-sized BMDs.

---

## Build & scope notes

- Build with **`build_run.bat`** (RelWithDebInfo); no `DUSK_DRIVE*` for manual playtests.
- Dev toggles under **Advanced Settings** only — not shipping ALBW gameplay defaults.
- Do **not** conflate with Boss Refinement or combat meter work.
- Trailer use: once viewer or get-item path works, `v*` props are viable museum/B-roll assets ([trailer-handoff.md](trailer-handoff.md)).
- Boot crash with log `unhandled tcg src 21` → wipe **both** `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` and `pipeline_cache.db*` after **every** gfx rebuild ([build-fps-guidelines.md § GPU cache crash](build-fps-guidelines.md#addendum-gpu-cache-crash-2026-06-26)).
- Debug log: `Documents/dusklight/albw_ww_itemmdl_debug.txt` — grep `4E ambient-only`, `2B apply post-dl`, `4E: skip get-item beam particle`.

### Agent pitfalls (2026-06-26)

| Do not | Why |
|--------|-----|
| `git show HEAD:file.cpp > file.cpp` in PowerShell | Writes **UTF-16 LE BOM** — breaks compile/runtime. Use `git restore` (named file only) or Python `write_bytes` from `git show`. |
| `loadFromResourcePointer()` in `d_ww_itemmdl_pc.cpp` | Unsafe — caused crashes; stay on heap `resLoad` + `createHeap` path. |
| Re-enable MAJI or struct 14 for brightness | Kills tips/gradient/arrow; outdoor angle swing |
| Percentage room ambient alone (4C) | Neutral dim → neon yellow, not WW orange |
| Hide **both** `Vbow_v` and `SC_Vbow_v` at create on shared model data | Post–get-item crash; suppress at **draw time** only if A/B |
| Patch `d_a_alink_demo.inc` for message skip | Fix message timing in `d_ww_itemmdl_test.cpp` instead. |
| Revert unrelated features when bisecting boot `tcg src 21` | Clear GPU caches first; WW track disconnected from `files.cmake` did not cause logo death. |

---

## Agent prompt (paste)

> Read `docs/wind-waker-item-work.md` and `docs/build-fps-guidelines.md`. Implement Phase 0 + Phase 1 (bow get-item `itemmdl` swap with Advanced Settings toggles). Do not commit unless asked. Do not change user FPS/video AppData config.
