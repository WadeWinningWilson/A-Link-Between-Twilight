# Outset Island layers — the intro (awake / walk / telescope)

> **Companion (History ruling 2026-07-29 — cross-link, not merge).** This doc = the **verified layer census / findings** (live reference). The open **layer-activation TODO plan** + telescope-quest checklist is [outset-followup-plan.md](../../WW%20Linked/islands/Outset/outset-followup-plan.md). Neither supersedes the other.

Which props + NPCs exist on Outset (`sea` Room 44) during the three opening beats, and how WW's DZR
layer system actually gates them. A **noclip + decomp** finding.

## ① The correction that reframes everything

The three intro beats do **NOT** each live in a different DZR layer. They **all** play on `sea` Room 44,
**layer 0 (`ACT0`)**, differentiated by **STB demo event** (`awake`, `get_telescope`, `BirdSta`…), not
by layer. (`sea_T` Room 44 is the separate **title/attract** screen — `ACTR`-only scenery, no NPCs.)

**Layer selection** — `dComIfG_play_c::getLayerNo(roomNo)` (`d/d_com_inf_game.cpp:184-269`):
base parity `0` day / `1` night, then for `sea` room 44 (`:192-200`) a flag cascade:
```
isEventBit(UNK_0520) → layer|4  (ACT4/5)   // endgame, monster-overrun Outset
isEventBit(UNK_0E20) → layer|2  (ACT2/3)   // KoRL era, Aryll gone
isEventBit(UNK_0101) → 9        (ACT9)     // early KoRL return
else                 → layer    (ACT0/1)   // ← the INTRO plays here
```
The number → chunk mapping is `layerLoader` (`d/d_stage.cpp:2070-2151`): the loader reads the
**unnumbered** `ACTR`/`SCOB`/`TRES` as always-present, then overlays **exactly one** numbered layer
`ACTn/SCOn/TREn`. The higher layers (`ACT9/ACT2/ACT4…`) are **later re-visit states**, gated by
`UNK_0101`/`UNK_0E20`/`UNK_0520` — flags set by `daTag_Event_c` / STB off the **KoRL-return** chain
(`d_a_tag_event.cpp:87,95,98`; `UNK_0004` set by the KoRL boat `d_a_bk.cpp:2943`). **The intro sets
none of them** → it runs in `ACT0` throughout.

**Implication for the port:** restore **`ACTR` + `ACT0`** for the whole intro; drive per-beat actor
state with the **demo/STB events** (which we already restore via the message/demo system — see
[../cutscene-work/demo-message-step-in-step.md](../cutscene-work/demo-message-step-in-step.md)). Do
**not** try to model the three beats as three layers.

## ② Roster (Room 44, parsed from `sea/Room44.arc` room.dzr, 31 chunks)

**Always present — `ACTR` (187) + `SCOB` (59):** `NpcSo` (resident NPC_SO), `Kamome`×4 (gulls),
`Pig`×6, `kani`×2 (crabs), `Oyashi`×8 (palms), `Kanban`×3 (signs), `Tpost` (postbox), `Ekao`, `Pitfall`,
`AND_SW2`; decor `kusa*`/`flwr*`/`pflwr*`/`swood*`/`lwood`/`woodb*`/`koisi1`/`kotubo`/`ootubo1`/`item`×11.
Tags/objects: `TagEv`,`TagIsl`,`TagKb`×10,`TagHt`,`Salvag*`,`HyoiKam` (hoist-gull),`agb*`,`SW_C00`,`ky_tag1`.

**Intro cast — `ACT0` (24):** `Ls1`×2 (**Aryll**), `Zl1` (**Tetra**), `Bm1` (pirate — telescope target),
`Pirates` (ship), `Ko1`×2/`Ko2` (kids), `Aj1`/`Ob1`/`Ym1`/`Ym2`/`Yw1`×2/`P1a` (villagers), `Dk` (bird),
`Throck` (throw-stone), `bridge`×2, `Kamome`×4.

Profiles via `OBJNAME` (`d/d_stage.cpp:695-856`): `Ls1`→NPC_LS1, `Zl1`→NPC_ZL1, `Ba1`→NPC_BA1,
`Bm1`→NPC_BM1, `Aj1`→NPC_AJ1, `Ob1`→NPC_OB1, `Ym1/Ym2`→NPC_YM1/YM2, `Yw1`→NPC_YW1, `Ko1/Ko2`→NPC_KO1/KO2,
`P1a`→NPC_P1, `P2b`→NPC_P2, `NpcSo`→NPC_SO, `Dk`→DK, `Throck`→THROWSTONE, `Vdora`→Obj_Gong.

## ③ Per-beat (all within `ACT0`, differentiated by STB/flag)

| Beat (STB events) | Active | Notes |
|---|---|---|
| **1. Awake** (`awake`/`awake_z`) | Aryll (`Ls1`) wakes Link; Tetra (`Zl1`) in carried/demo state (`init_ZL1_1`, `!UNK_0101`, `d_a_npc_zl1.cpp:300`) | pirates + villagers **dormant** |
| **2. Walk to Grandma's** | Aryll back on the lookout; village NPCs idle | **Grandma (`Ba1`) is in her INTERIOR room, not Room 44** (exterior only in `ACTa`) |
| **3. Telescope** (`get_telescope`/`BirdSta`/`BirdLoo`/`R29LOOK`) | Aryll telescope-look targets `Bm1`; `Pirates` ship; bird grabs Tetra | completion `dComIfGs_onEventBit(UNK_0310)` (`d_a_npc_ls1.cpp:1596`) — does **not** flip the layer |

Aryll event table `l_evn_tbl = {"zelda_fly","omedeto","get_telescope","eTalk"}` (`d_a_npc_ls1.cpp:54-58`).

## ④ noclip confirmation + the friendly layer names

noclip.website (WW → Outset Island) **renders `ACTR`+`ACT0`** (lookout tower, bridge, village houses,
beach, palms all match) and — critically — its **Layers panel names the DZR layers in plain English**,
which both **confirms** the decomp and gives better story-labels than my inference. noclip's layer list:

| noclip name | ≈ DZR chunk | Notes |
|---|---|---|
| **Intro (Day)** ← *selected* | **`ACT0`** | **the single layer all three intro beats play in** ✓ |
| Intro (Night) | `ACT1` | intro is daytime → near-empty |
| Pirates Arrived | `ACT2` (`UNK_0E20`) | village + `P2b`, no Aryll |
| Day / Night | later free-roam | KoRL-return era |
| Demo: Ending | (demo layer) | ending cutscene |
| Demo: Sister Kidnapped | (demo layer) | the kidnap beat |
| Demo: Departure | `ACTa` | **Grandma exterior** (`Ba1`), `Vdora` gong — Link leaving |

So the **intro = "Intro (Day)" = `ACT0`** is confirmed three ways: `getLayerNo` (no intro flag →
default), the DZR parse (`ACT0` = the intro cast), and noclip's own named layer.

**noclip DOES render the NPCs in full, IDable detail** (correcting an earlier wrong assumption of mine):
selecting **Intro (Day)** shows the live `ACT0` cast. Confirmed visually so far: the young Outset woman
with indigo hair + pink/blue-swirl dress = **Sue-Belle** = one of the `Yw1` (NPC_YW1). So the villager
human-name census IS doable in noclip by model recognition (navigate to each actor, ID the character) —
faster/surer than BMG for most. Remaining profiles to name this way: `Aj1`,`Bm1`,`Ym1`,`Ym2`,`Ob1`,`P1a`,
`Ko1`,`Ko2`,`Dk`. No `ACT3` exists, so `UNK_0E20` at night has no overlay.

## ⑤ `ACT0` placement (from room.dzr — what the port must place)

Positions (x,y,z) as parsed; `y` immediately reveals staging:

| Actor | pos (x, y, z) | Reading |
|---|---|---|
| `Ls1` (**Aryll**) ×2 | (-195205,**1650**,313823), (-195506,**1650**,313774) | **on the lookout** (elevated) — awake + telescope |
| `Kamome` ×4 (gulls) | ~(-195400, **2307**, 313800) | circling the lookout above Aryll |
| `Dk` (bird) | (-203543, **4282**, 316574) | high-flying bird |
| `Pirates` (ship) | (-187000, 0, **304000**) | offshore — the pirate ship |
| `Zl1` (**Tetra**) | (-204814, 0, 322091) | beach — where the bird drops her |
| `Bm1` (pirate) | (-203698, 497, 316431) | Aryll's telescope-look **target** |
| `Ob1`,`Ym1`,`Ym2`,`Aj1`,`Yw1`×2,`Ko1`,`Ko2`,`P1a` | ground level, scattered | **village NPCs the walk passes** |
| `bridge` ×2, `ikada_h` (raft), `Throck` | — | props |

This is the whole intro layer to restore: place `ACTR` (scenery) + these `ACT0` actors; the demo/STB
events (awake / get_telescope / bird) then animate Aryll, the gulls, the bird, Tetra, and the ship —
the villagers stay idle until their own logic activates. Full positions in
`scratchpad/dzr_act0_pos.py` output.

## ⑤ The WHEN — RUNTIME-MEASURED (DuskTap roster census, 2026-07-28)

The static layer gives WHICH+WHERE; a DuskTap **roster tap** (897 spawns, timestamped + positioned,
`docs/WW Linked/dolphin-captures-roster-20260728.txt`) gives the WHEN directly — resolving what no static
view could:

| Beat (~t) | Spawns | NPCs actually live |
|---|---|---|
| **Awake** (~2450) | 181 | **NONE** — scenery only; Aryll is a *demo actor*, not a placed spawn |
| **Walk / village** (~2710) | 331 | the **full `ACT0` roster** (`Ls1`×2, `Zl1`, `Pirates`, `Bm1`, all villagers, `NpcSo`, `Dk`, `Throck`) |
| **Tale-loft** (~2960/2972/3046) | 18 ea | **Grandma `Ba1`×4** spawns for the tale demo |
| Reload (~3060) | 331 | full roster again |

**So the restoration WHEN is:** awake = scenery + demo actors (village NOT spawned); the village NPCs
spawn at the walk; Grandma spawns at the loft for the tale. This refines the decomp's "villagers dormant" —
they're not even spawned during the awake.

**Trigger positions confirmed (runtime):**
- Tale `TagEv` `0x02FF110A` at **(-413.6, 375.0, 314.7)** = our `region_triggers.ini` `[tale_loft]` exactly
  → geometry triple-confirmed (DZR + roster). Companion `0xFF11120A` at (-396.5, 0, -34.1).
- Beat C `TagEv` type `0x3` (UNK_0E20 setter) at **(-200242.8, 3700, 321928.5)** = the L-3 arming-chain
  Outset beat, runtime-confirmed (see [../../WW Linked/port-liberties.md](../../WW%20Linked/port-liberties.md) L-3).

Ambient density (roster): `kusax21`×108, `kusax7`×87, `kusax1`×65 (grass), `Kamome`×26 (gulls),
`Oyashi`×24 (palms), `Pig`×15 (**= item 5's actor**), `TagKb`×20, `SalvagE`×20, `item`×22.

**Cross-checked against the P6 fact-sheet library** ([../../WW Linked/fact-sheets/sea.md](../../WW%20Linked/fact-sheets/sea.md)):
my Room44 `ACTR`/`SCOB` layer parse matches it **line-for-line** (independent sweep → high confidence).
The fact-sheet also carries **non-actor chunks my census skipped that the restoration needs:**
`TGDR` ×5 = **doors** (`KNOB00D`×4, `KNOB03D`×1) · `PLYR` ×24 = **Link spawn points** · `SHIP` ×2 (pirate
ship) · `RCAM` ×5 (room cameras) · `SOND` ×2 (sound triggers) · `RPPN` ×296 / `RPAT` ×40 (room paths).
Place these alongside `ACTR`+`ACT0` when restoring the room.

## ⑥ Open / ambiguous

- **Villager human names** (Sturgeon/Orca/Mesa/Rose/Abe/Joel/Zill…) are **not** in the decomp — only
  Aryll (`Ls1`), Tetra (`Zl1`), Grandma (`Ba1`) are source-certain. The **noclip** leg (visual model ID)
  + BMG dialogue are how we put names to `Aj1`/`Bm1`/`Ym1`/`Ym2`/`Yw1`/`Ob1`/`P1a`/`Ko1`/`Ko2`/`Dk`.
- **Per-beat visibility inside `ACT0`** is STB-script data (which demo hides/shows which actor), not
  resolvable from the C++ side — the STB parse (`jstudio_stb.py`) is the instrument if we need it exact.
- Flags are `UNK_`-named in the decomp; the semantic labels here ("Aryll gone", "endgame") are inference
  from usage, not source-stated.

### Citations
`d_com_inf_game.cpp:184-269` (getLayerNo) · `d_stage.cpp:2070-2151` (layerLoader), `:695-856` (OBJNAME) ·
`d_a_tag_event.cpp:87,95,98` (UNK_0004/0101/0E20) · `d_a_npc_ls1.cpp:54-58,1560-1620,1596` (Aryll/telescope)
· `d_a_npc_zl1.cpp:298-316` (Tetra gating) · `d_save_event_flag.inc:20,41,105` · parsed
`Ex WW/files/res/Stage/sea/Room44.arc` (+ `sea_T`).

## ⑦ Verification — census ↔ placement-file cross-check (2026-07-28)

History-lane audit of the restoration's **placement data** (`population/outset_placements.csv`,
`actor_map.ini`, `npc/*.ini`) against the **donor runtime census** (`dolphin-captures-roster-20260728.txt`,
897 spawns). **Verdict: the layer framework is data-complete and faithful; one exterior hole.**

- **`outset_placements.csv` is the full donor DZR dump** — 474 rows keyed by chunk (`ACTR`/`SCOB` = always,
  `ACT0…ACTb` = layered). The **intro layer = `ACT0` (24 rows)** and `actor_map.ini [layers]` gates it
  `ACT0=` (blank ⇒ always-on) — matching the measured WHEN (intro roster present through the walk). Later
  beats gate on quest flags (`ACT8=qs.pirates_ashore`, `ACT9=qs.aryll_taken`, `ACTa/b=qs.depart`).
- **36 census actors are `actor_map`-covered.** The uncovered remainder is **non-content by design**:
  `LOD01–49` (LOD cull markers), `Tag*`/`agb*`/`SW_C00`/`AND_SW2`/`ky_tag1` (event triggers),
  `Salvag*`/`SwSlvg` (salvage points), `KNOB*` (doors — owned by `doors.ini`), `Link`/`sea`/`Ship`/`Pitfall`
  (engine). Scenery holes (`swood*`/`woodb*`/`flwr*`/`bonbori`) are the pre-declared N31/N42 skip-log set.
- **"Two Sue-Belles" was a noclip artifact, not a port bug.** `Yw1`/`Ko1` each carry a second `ACT0`
  event-alternate row (params `00ff00xx`); `actor_map` suppresses it via `disable_pos` (80-unit-radius
  match, `d_ext_npc_population.cpp:642-655`). noclip renders both because it doesn't evaluate the event
  switch; the port spawns exactly one.
- **The one genuine exterior gap — `NpcSo`.** Placed as an always-on `ACTR` row at
  `(-200900, 0, 303000)` (offshore, near the pirate ship) but **unmapped + no manifest**, so it never
  spawns in the port. Donor: `daNpc_So_c` / `g_profile_NPC_SO` (`d_a_npc_so.cpp`) — an offshore
  **minigame** NPC (`_searchEsa` bait / `_searchMinigameTagSo`). **Its arc is already extracted**
  (`arcs_lib/So.arc`). Closing it = a build-lane add (register `NPC_SO` proc + `npc_so.ini` manifest +
  `actor_map [NpcSo]` at a free arg ≥ 62 + port its minigame behavior) → **Housing queue.**
- **Interior-scope notes:** `Cb1` (Omori house) is already placed in `interior_placements.csv`; `Md1`
  (interior NPC) is unplaced — an *interior* hole, outside the intro-layer scope, logged for the interior pass.

## ⑧ Identity verification — heads / shirts / companions (2026-07-28 playtest log)

Verified every ACT0 NPC's **appearance parts** against the runtime resolution log
(`logs/dusklight-20260728-181626.log`; identity code unchanged by the audio build). **All resolve
clean — zero identity failures** (`slave_map miss`=0; the only warns are benign: offshore actors
keeping authored height instead of seafloor-snapping, dialogue pagination, door self-heal, `btp`
blink-texture skips).

| NPC | proc | head | shirt (bmt) | companion | log evidence |
|---|---|---|---|---|---|
| Zill | NPC_KO | `kohead01` ✓ | (shared `ko.bdl`) | — | `attachment 'kohead01.bdl' joint_slave → head` |
| Joel | NPC_KO2 | `kohead02` ✓ | **`ko02.bmt` applied tex=yes** ✓ | — | `body_bmt 'ko02.bmt' applied` |
| Mesa | NPC_YM | `ymhead01` ✓ | (shared `ym.bdl`) | — | `attachment 'ymhead01.bdl' joint_slave → head` |
| Abe | NPC_YM2 | `ymhead02` ✓ | **`ym2.bmt` applied tex=yes** ✓ | — | `body_bmt 'ym2.bmt' applied` |
| Sue-Belle | NPC_YW | `ywhead01` ✓ | — | — | `attachment 'ywhead01.bdl' joint_slave → head` |
| Rito postman | NPC_BM | `bmhead01` ✓ | — | **`bmarm.bdl` pairs=2** ✓ | `companion 'bmarm.bdl' mode=joint_slave pairs=2` |
| Aryll | NPC_LS | (baked) | — | **`lshand.bdl` pairs=2** ✓ | `companion 'lshand.bdl' mode=joint_slave pairs=2` |
| Tetra | NPC_ZL | (baked) | — | `cloth.bdl` ✓ | `head=companion:cloth.bdl` |
| Ob1 | NPC_OB | `oba_head` ✓ | — | — | `attachment 'oba_head.bdl' joint_slave → head` |
| P1a | NPC_P1 | `p1a_head` ✓ | — | — | `attachment 'p1a_head.bdl' joint_slave → head` |
| Pirates | NPC_P2 | `p2head01` ✓ | — | — | `attachment 'p2head01.bdl' joint_slave → head` |
| Dk (masked) | NPC_DK | `dk_kamen` (registry ✓) | — | — | quest-gated (`beat.helmaroc`), absent in normal intro |

- **`Aj1` (NPC_AJ) `head=MISS:no_head_attach` is CORRECT, not a defect.** `d_a_npc_aj1.cpp` has only a
  `_nodeCB_Head` look-at node callback on the body model and **no separate head-member load** — Aj is a
  unique villager with its head **baked into `aj.bdl`** (contrast Ko/Ym/Yw, which share a body so their
  head must be a swappable member). Nothing to attach; renders complete.
- **Props all spawn** (same log): `NPC_KAMOME`×590 gulls · `NPC_OYASHI`×553 palms · `NPC_KB`×430 pigs ·
  `NPC_KN`×147 crabs · `NPC_PIWA`×79 rocks · `NPC_KANBAN`×207 signs · `NPC_TPOST`×77 postboxes ·
  `NPC_YAFLW`×285 flowers · `NPC_LWOOD`×289 · `NPC_KOISI`×209 pebbles · `NPC_BOXA`×72 boxes ·
  `NPC_LAMP`×56 · `NPC_EKAO`×72 · `NPC_VLUPY`×740 rupees.
- **Only outstanding gap stays `NpcSo`** (§⑦ — offshore minigame NPC, arc extracted, Housing build).

## ⑨ Prop-placement completeness — every unmapped placement, adjudicated (2026-07-28)

Diffed `outset_placements.csv` (474 rows) against `actor_map.ini` → **44 distinct placement names have a
position but no spawn wiring.** Each was run through the donor `d_stage.cpp` OBJNAME table + the port's
render code. **None is a faithful-placeable oversight; every hold is principled (N31: a missing prop
beats a wrong one).** Breakdown:

| class | names | why unplaced (verified) |
|---|---|---|
| **Invisible triggers** | `Tag*` (Ev/Ht/Isl/Kb/MSo/Msg2/So), `agb*`, `Salvag*`/`SwSlvg`, `SW_C00`/`AND_SW2`, `HyoiKam`, `Pitfall`, `ky_tag1`, `AttTagB`, `Com_A`, `ITat00` | no visual — event/salvage/switch tags. Correctly never spawned. |
| **Later-beat enemies** | `keeth`×26 (Keese, ACT6/7/b), `Puti`/`c_green`/`c_red` (ChuChu, ACT5), `DmKmm`/`DmKmm2`/`Ktarur`/`kuro_s` (ACTb depart) | gated to their story beat, absent in the intro **by design**. |
| **Grass-family flowers/trees** | `flower`, `flwr7`, `flwr17`, `swood`, `swood3`, `swood5` (all `fpcNm_GRASS_e`) | port `d_a_ext_vegetation` renders **only grass (`kind==0`)**; trees/flowers hit `if (kind != 0) return … // inert` (`:509`). Mapping → `NPC_EXTVEG` renders them wrong (as grass) or invisible. Faithful render needs their **own packet** → Housing. |
| **Un-ported object actors** | `ikada_h` (`daObj_Ikada`, arc `IkadaH` present) · `woodb`/`woodbx` (`daObj_Wood`) | discrete actors with real behavior — `ikada_h` is a **`movebg` boat with wave physics** (`mWaveVelSpeed`/`mWaveMaxVelocity`/`mWaveCollapsePos`), NOT a static model. `static=1` placement would be a wrong prop. Needs the actor ported → Housing. |
| **NPC** | `NpcSo` | §⑦ — offshore minigame NPC, arc extracted, needs `NPC_SO` proc → Housing. |

**Verdict: intro prop/NPC placement is faithful-complete.** What the player perceives in the intro
(villagers with correct heads/shirts, grass, palms, gulls, pigs, crabs, rocks, signs, boxes, flowers via
`pflower`, rupees) is placed. The remaining names are either non-visual, later-beat, or need a specific
**actor port** (grass flower/tree packet · `daObj_Ikada` wave-bg · `daObj_Wood` · `NPC_SO`) — none is a
mapping I can add without rendering something wrong. **Housing build list**, not a History mapping gap.
