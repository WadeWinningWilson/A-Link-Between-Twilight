# Outset follow-up plan — layer activation + identities (telescope-quest-critical)

> **Companion (History ruling 2026-07-29 — cross-link, not merge).** This doc = the **open TODO plan** (flag→layer mapping, telescope-quest checklist). The **verified layer census / findings** is [outset-intro-layers.md](../../../gaming%20systems/layers/outset-intro-layers.md). Neither supersedes the other.
>
> **✓ STATUS REFRESHED (History, 2026-07-30 — native Aryll port landed, bus §247/§249).** The native
> `d_a_npc_ls1` actor now spawns, renders, and runs her full state machine (was an audition-mount
> stand-in before). Net effect on the telescope quest: the **spawn/model/actor-state** side is now
> MET; the remaining blocker is the **player telescope ITEM** — a true native subsystem not yet ported.
> Per-item status in the SPAWN CHECKLIST below. See the CUTSCENE-NPC recipes (cookbook 10–15) for the
> actor-side port. **Standing directive: the deferred native subsystems (player telescope item, the
> `JA_SE_CV_*` voice path, `getP_BtpData` face decode) WILL be ported — the current reconstructions are
> bridges, not endpoints.**

> Planned History, 2026-07-26. Feeds the **Aryll telescope** interaction, which depends on the
> right actors being spawned/drawn in the right story layer. Method = the elevated noclip
> identity-match rig for CONTENTS + decomp for ACTIVATION
> ([../../noclip-fast-track.md](../../noclip-fast-track.md)). Contents are verified; activation
> (which flag turns a layer on) is decomp-only — noclip cannot answer it.

## What we already know (verified)

- **Layer 0 IS the telescope-beat layer.** Aryll (`Ls1` / `NPC_LS1`) is placed at
  `(-195205, 1650, 313823)`, params 3 — `y=1650` = the lookout height (matches the §52 walk-route
  "-195400, y=1650, lookout"). Her **layer-9** placement `(-196262, 3178, 321891)` params 2 is a
  DIFFERENT, higher beat (likely kidnap/departure).
- **`Bm1` co-spawns with Aryll in layer 0** `(-203698, 497, 316431)`. Aryll's own actor searches
  `mBm1ProcID` and sets a go-flag off it ([presence/npc-presence-recipe.md](presence/npc-presence-recipe.md)) —
  so if `Bm1` is absent from the active layer, her state machine can stall.
- **The telescope lives in Aryll's actor** (`D:/XXXXXXX/WW DP/src/d/actor/d_a_npc_ls1.cpp`):
  `mpTelescopeModel`/`mTelescopeScale`, the `"get_telescope"` action, `dItemNo_TELESCOPE_e`, and a
  `mType` / `m84B` / `mEventIndex` / `mMesgAnimeTag` state machine (bodies present, unlike Ba1).
- **Telescope item = TP-analogue** (`vtele`→`Tele_00.arc`→TP Hawkeye) — get-item + held path works
  ([../../../wind-waker-item-work.md](../../../wind-waker-item-work.md)); a folder flag
  `aryll.telescope` already exists (`d_ext_mod_flags`).
- Per-layer CONTENTS census done: [presence/layers-noclip-census.md](presence/layers-noclip-census.md).

## Follow-up B — layer-activation flag mapping  **(PRIORITY: gates the whole quest)**

Goal: know which story flag(s) turn on Outset (sea room 44) layer 0, so the telescope beat
reliably spawns Aryll-at-lookout + `Bm1`.

1. **Decomp — find the layer selector.** How WW computes the active layer for sea/room 44 from
   story flags. Targets: `d_stage.cpp` room/layer control, `dStage_dt` layer bits,
   `dStage_roomControl`, and the room DZR's layer chunk (SCLS/LBNK). This is the activation logic
   noclip does not run.
2. **Map Aryll's two placements to states.** Confirm layer 0 (lookout, telescope) vs layer 9
   (higher, later beat) against the flags each requires. Do the same for `Zl1`'s spread
   (0/2/8/9/10 — layer 8 is `y=-5000`, an inactive/hidden placeholder).
3. **Cross-ref the telescope gate.** In `d_a_npc_ls1.cpp`, find which `mEventIndex`/`m84B`/flag
   state the `get_telescope` event requires, and confirm it lines up with layer 0 being active.
   Reconcile with the `aryll.telescope` folder flag.
4. **Deliverable:** an Outset flag→layer table + the confirmed "telescope beat = layer 0" spawn
   requirement, recorded here and cross-linked to the telescope quest doc.

## Follow-up A — villager identity resolution (IVAN)  *(supports B; parallel)*

Goal: name the layer-0/always cast so the quest census is correct. Per
[characters/README.md](characters/README.md): confirmed → name + ✓ decomp; otherwise keep the
census code marked `? (unverified)`.

- Resolve each code via decomp `d_a_npc_<code>.cpp` (class name, comments, message rows) + census:
  `Zl1`(NPC_ZL1), `Ob1`, `Ym1`, `Ym2`, `Aj1`, `Bm1`, `Ko1`, `Ko2`, `Yw1`, `Cb1`, `Md1`, `NpcSo`,
  `Dk`(fpcNm_DK_e).
- **Priority (telescope-adjacent first):** `Zl1` (likely Tetra) and `Bm1` (Aryll co-spawn), then
  the rest.
- Deliverable: updated `characters/README.md` roster.

## The payoff — telescope-quest SPAWN CHECKLIST

For the Aryll telescope interaction to fire, when its beat is active ALL must hold
(**status refreshed 2026-07-30** against the native port):
1. **Layer 0 active** (Follow-up B confirms the gating flag). — **✓ MET** (`ACT0` = always; unchanged).
2. **Aryll (`Ls1`) spawned at the lookout** `(-195205, 1650, 313823)`. — **✓ MET, now NATIVE** (§247;
   was a mount stand-in before — spawn-wired via socket `LS` → `fpcNm_NPC_LS1_e`, `l_objectName` row
   restored).
3. **Her `mpTelescopeModel` present** (telescope model resource loaded). — **✓ MET this session** —
   `itemCreateHeap` now sources `telescope.bdl` from her own `Ls` arc (the WW "Link"-arc source has no
   port equivalent; cookbook recipe 14). It loads + renders.
4. **`get_telescope` event + flag wired** (her state machine + `aryll.telescope`). — **◐ PARTIAL** —
   her actor-side `get_telescope` state path is ported and present, but the demo branch is **dormant**
   because it gates on `daPyStts0_TELESCOPE_LOOK_e`, which only the player telescope item sets (item #5).
5. **Telescope item grant** (`dItemNo_TELESCOPE_e` → TP Hawkeye mapping). — **✗ BLOCKED — the true
   native subsystem to port next.** The scope-*message* plumbing is done (§245), but the **player
   telescope ITEM** (WW player-main state that sets `TELESCOPE_LOOK`, `dItemNo_TELESCOPE_e` currently a
   sentinel `#define`) is not ported. This is the single remaining gate — and per the standing
   directive it gets a real native port, not a shim.
6. **`Bm1` co-spawned** in the same layer (Aryll searches `mBm1ProcID`; absence may stall her). —
   **✓ MET (type), placement TBD** — `bm1` (Generic Ritos) is now natively ported/registered (§246), so
   `mBm1ProcID` can resolve; confirm the `ACT0` co-placement actually spawns a `Bm1` at `(-203698, 497,
   316431)` in play.

**Net:** items 1–3 fully met (2 & 3 by this session's native port), 6 met on the type side; the quest's
one true remaining blocker is **#5, the player telescope item** — the next native subsystem on the list.

## ═══ FOLLOW-UP B — RESULTS (2026-07-26, decomp-verified) ═══

### The WW selector (VERIFIED, not a stub)
`dComIfG_play_c::getLayerNo(int roomNo)` — `d_com_inf_game.cpp:185` (matched ROM range). Returns
**ONE** layer index 0..b (not a mask); the chosen `ACTn` is loaded **on top of** the always-`ACTR`
default (separate loader passes, `d_stage.cpp:2150/2203`). A forced start layer
(`getStartStageLayer() >= 0`, i.e. the demo/warp entry path) overrides everything.

For `stage=="sea" && roomNo==44` (Outset, `dIsleRoom_OutsetIsland_e`), the ladder is:

| Story input (save EVENT register `isEventBit`) | Layer | Our `[layers]` counterpart |
|---|---|---|
| **none set** (baseline) | **0 day / 1 night** | **`ACT0` = `""` (always)** |
| `UNK_0520` | 4 day / 5 night | `ACT4/5` = `qs.ah_state` |
| `UNK_0E20` | 2 day / 3 night | `ACT2` — **OMITTED in our port** |
| `UNK_0101` | 9 (flat) | `ACT9` = `qs.aryll_taken` |

Priority `0520 → 0E20 → 0101 → baseline`. Actor "layer" = which `ACTn` chunk it lives in; WW simply
never decodes the non-selected chunk, so those actors never spawn (no per-actor mask test).
Decomp names the bits `UNK_*` only — their human meaning is NOT source-labeled (IVAN: I map ours to
theirs by **layer correspondence**, not by asserting a name).

### The telescope conclusion (the answer to the quest dependency)
**Telescope beat = layer 0 = the day baseline with NO event bit set** — Aryll at the lookout
`(-195205, 1650, 313823)`. In our port `ACT0 = ""` (always), so **Aryll's lookout placement +
`Bm1` (also `ACT0`) already spawn**. The spawn dependency the quest needs is ALREADY MET by the
existing flag-gated system — nothing new to wire on the spawn side. The telescope *give* is gated
separately by Aryll's own actor (`d_a_npc_ls1`: `m850`/`mEventIndex` → `get_telescope` event →
`mTelescopeScale` reveal + `dItemNo_TELESCOPE_e`, behind event bit `UNK_0001`); the "You got the
Telescope!" line is already present (`ww_dialogue_full.txt` [114]). **That give-behavior port is a
separate follow-up (quest wiring), not a spawn/layer problem.**

### Fidelity gaps (NON-blocking for the telescope; log for faithful Outset)
1. **`ACT0` = "always" ≠ donor "layer 0 only when no bit set".** WW *drops* the day-1 cast once a
   progress bit sets; ours keeps it forever. Faithful fix later: gate `ACT0` `spawn_unless` the
   progress flags. (actor_map.ini already flags this: "later need flags".)
2. **`ACT2` (donor `UNK_0E20`, layer 2/3) omitted** — deliberately, to stop day-1 bleed/FPS; re-add
   as a flag-gated same-day variant if tww confirms.
3. **Day/night:** donor baseline splits layer 0 (day) / 1 (night); our `ACT0` isn't time-gated.
   (Our census showed layer 1 ≈ empty — night baseline genuinely sparse.)
4. **`ACT6/7/8/a/b` reconciliation:** the verified ladder only *quoted* the `0520/0E20/0101`
   branches (agent truncated `} ...`); confirm the full room-44 ladder before trusting our
   `qs.pirates_ashore`/`qs.depart` gates for layers 8/a/b.

## Sequencing + the one open risk

- **B before A's tail:** activation gates everything; do B first, run A in parallel (identities
  feed B's cast naming).
- **⚠ OPEN RISK — how does OUR port pick the Outset layer?** Donor selects layer by story flags
  (B, step 1). We must confirm whether Dusklight honors that flag→layer mapping or pins a layer for
  the WW-restored Outset. If pinned, the telescope beat (layer 0) must be made selectable by our
  quest logic — a History↔Engine coordination point, and the likely real blocker for the telescope
  quest. **Resolve this alongside B, step 1.**
