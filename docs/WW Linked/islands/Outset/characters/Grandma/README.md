# Grandma (`Ba1`) — hero's-clothes scene scoping (№228)

**Questline position:** the next beat after the awake cutscene — Link visits Grandma, she gives
the hero's clothes. Carries three work fronts: her **interior positioning**, her **dialogue**,
and the **clothes handover**.

**Roadmap context (user, 2026-07-22):** the questline wires ahead of the final audio pass;
visuals before audio; Outset "done" is declared only after the full audio sweep (every cutscene,
NPC voice, ambient).

---

## 1. Interior positioning — CORRECTED (user, 2026-07-22)

- **Grandma is ALREADY present in her house interior in the mod** — the doors/interior work
  spawns Ba1 there today. The earlier draft of this section wrongly promoted the LinkRM
  census-defect note into "her placement is blocked"; the census defect explains why the ROOM
  CENSUS shows no NPCs, not whether she is placed (she is, via the manifest/population path).
- What actually remains for the questline: whether the CLOTHES SCENE requires a specific
  spot/pose/state for her (read from the scene's event data when identified), and any
  story-layer variation of her placement (№222 ladder, per-room) — both are event-data reads,
  not spawn work.

## 2. Dialogue

- Same pipeline as Aryll's awake lines (№217): donor message rows → `dialogue/` data, R6
  throughout. Her scene's message ids come from the event data / STB once identified in
  `LinkRM`'s EVNT table.
- Donor actor caveat: `d_a_npc_ba1.cpp` is largely **Nonmatching** in the decomp (demo/wait/exec
  procs are stubs) — so her flow must be read from EVENT DATA (event_list/STB + EVNT), not actor
  source. Budget accordingly.

## 3. The clothes handover — covenant analysis

> ### ⛔ DECOMP CORRECTION (2026-07-26, IVAN — supersedes the get-item framing)
> **`Ba1_Get_Itm` is NOT the clothes give — it is Grandma's ELIXIR SOUP give.** Its one data
> node is `prm0 = 0x55 = dItemNo_SOUP_BOTTLE_e` (verified). The Hero's Clothes item is
> `dItemNo_FUKU_e = 0x32`, and **no event in `LinkRM` carries `0x32`** (all 79 scanned) — because
> in WW the clothes are a **wear-change (texture swap)**, not a get-item box. (Msg 151 "You got the
> Hero's Clothes!" is just `0x32 + 0x65` via `MSG_NO_FOR_ITEM`; there is no override, and
> `mItemMesgNum` is vestigial.) The earlier build **repurposed** the soup event as a clothes
> get-item box; per the covenant (user ruling, 2026-07-26) that is **unwound**:
> - `Ba1_Get_Itm` `prm0` **restored to 85 (soup)** in `R_DL01/STG_00.arc` — it is now the faithful
>   soup event, reserved for the later post-kidnap recovery beat.
> - The clothes get-box kits (`getitem/clothes_bundle*.ini`) are **deprecated/inert**.
> - **Clothes = wear-change only** (`dAlbwOutfit_equip → setClothesChange`), which already works.
> - **Engine action pending:** `grant_outfit:heros` must fire NO get-item demo (wear-change only).
>   Until then the clothes scene shows a spurious item-85 box (transient, not a new bug).
> This aligns with the texture-swap finding immediately below — the get-item box was never faithful.

- **DONOR FINDING: WW's hero's clothes are a TEXTURE SWAP, not a model.** WW Link's body model is
  one mesh; casual ⇄ hero is a `linktexS3TC` texture switch in the player (d_a_player_main.cpp
  ~12372-12382 comments; `clothes.bti` = item icon in d_item_data). There is no separate
  "clothes" model asset to hand over — the *item presentation* and the *wear state* are separate
  problems in the donor too.
- **Covenant (user):** the given clothes cannot be a TP substitute — the handover prop/item
  presentation must be WW's (№31 space purity; R6 for any text).
- **OPEN DESIGN DECISION (user input needed): what TP Link WEARS after the scene.** Readings:
  1. TP Link switches to his own native Hero's Clothes (his green tunic IS TP's "Hero's
     Clothes"; if he runs Outset in Ordon clothes until this beat, the scene grants the switch —
     TP asset on TP body, story beat from WW, no franken-model);
  2. an actual WW-style outfit on TP Link (asset work: new texture/model state).
- **Prior art either way:** the sumo outfit system (d_albw_outfit) — model/texture outfit STATE
  machine on TP Link with per-save worn bits, changeLink integration, D-pad quick-swap, cap
  recolor pipeline. The wear-state mechanics are already owned; only the asset/design question is
  open.

## 4. Cutscene player-drive observations (user, side-by-side testing)

Recorded verbatim intent: during cutscenes TP Link **rotated and froze animation at the exact
moments WW Link does** — then cycled ~"1/4 of an idle stand, paused, repeated" while standing for
the remainder.

Reading: the demo system's TRANS/ROTATE/pause channels **reach the TP player correctly** (his
storyboard participation works at the transform level). What doesn't land is the ANIMATION
channel: WW Link's demo anims (wake/stretch/walk) name WW archives that don't resolve on TP Link,
so his current TP idle gets driven by the storyboard's frame counter — a WW anim's frame range
looping over a fraction of the TP idle = the observed quarter-idle stutter-loop.

Work item (research, ties to the parked "WW Link anims on TP Link" idea): a demo-anim TRANSLATION
table — WW demo anim cue → nearest TP Link anim — applied where the player's demo data resolves
anims. Even a small table (wait/walk/surprised) would make him a convincing cutscene citizen.

## FOUND (№230): the scene's identity and mechanism  ⚠ IDENTITY SUPERSEDED — see VERIFIED below

- ~~**The clothes scene = `Ba1_Get_Itm`**~~ **WRONG (decomp, 2026-07-26): `Ba1_Get_Itm` is the
  ELIXIR-SOUP give (`prm0=0x55`). The clothes scene = the STB `TALE_DEMO` ("Grandma's Tale").**
  §230 was RIGHT about the *mechanism* (below) — `UNK_2A80` / `d_a_demo00` prm-op / SHAPE channel —
  but wrong that it's an event pack. It IS an STB (`tale.stb`), the very "no clothes storyboard"
  §230 ruled out. (Beat-list caveat: the "[04] = 0x0E20 kidnap" label is contradicted — decomp
  shows the TALE/clothes TagEv arms on `UNK_0E20`; treat `0x0E20`'s human label as UNVERIFIED.)
- **The wear-state flag is `UNK_2A80`** (player_main: casual clothes unless 2A80 || NG+). In STB
  scenes the donor sets such flags from the STORYBOARD's prm channel via `d_a_demo00` — the
  cutscene Link BODY-DOUBLE — whose prm-op table maps sub-id 1 -> 2A80 (and whose SHAPE channel
  swaps the double's outfit mid-scene). For the event-pack path the setter is likely in Ba1's
  (Nonmatching) code or the pack's own action — the pack parse decides.
- `LOOK_SHIELD`/`get_shield` living in the same table means the SHIELD handover scene is
  co-located — the same read covers the next questline beat after the kidnap.

## ✅ VERIFIED faithful spec (2026-07-26, decomp-cited) — W1 trigger + clothes presentation

The clothes-give is the **`TALE_DEMO` STB cutscene** (`tale.stb`), started by a **loft region
trigger**, and the clothes are a **texture swap driven by the STB** — no get-item box anywhere.

**W1 — the loft trigger (a `TagEv` / `d_a_tag_event` region SCOB in `LinkRM/Room0/room.dzr`):**
- pos `(-413.6, 375.0, 314.7)` (**y=375 = the loft**; ground spawns are y=0), scale `(10, 4, 10)`,
  param `0x02FF110A`.
- Decodes: `getType()=0x0A`, `getEventNo()=2` → `dComIfGp_evmng_getEventIdx(NULL, 2)` = **`TALE_DEMO`**
  (EVNT entry 0, event-number `0x0002`, PLAY cut → `tale.stb`).
- **B2a Y-BAND (confirmed):** `actionHunt` (`d_a_tag_event.cpp:242-263`) tests
  `abs2XZ() < SQUARE(scale.x)*SQUARE(100)` AND `|Δy| <= scale.y*100`. With scale (10,4,10):
  **XZ radius = 1000**, **Y half-band = 400** → arms for Link y ∈ ~[−25, 775] around the loft.
- **Gate:** type-0xA arms only while **`UNK_0E20`** is set (`d_a_tag_event.cpp:304-311`); on finish it
  sets `UNK_3202`; `swbit=17` is a same-session re-arm guard. (True once-gate is enforced at the
  actor-LAYER level via `getLayerNo`'s `UNK_0101→UNK_0E20→UNK_0520` ladder — treat as
  layer-confirmed, not a per-tag re-check.)

**Clothes presentation (texture swap, `linktexS3TC`, flag `daPyFlg1_CASUAL_CLOTHES`=0x08):**
- **Live, in-cutscene:** the STB's Link body-double sets `ENABLE_SHAPE` / `getShapeId()` (0=casual,
  1=hero) → `daPy_lk_c::setDemoData` swaps the texture (`d_a_player_main.cpp:10352-10367`).
- **Persistent:** the STB fires a `d_a_demo00` event-bit command (`d_a_demo00.cpp:651-712`, sub-id 1)
  → `onEventBit(UNK_2A80)`. On every load `playerInit` (`d_a_player_main.cpp:12356-12387`) reads
  **`UNK_2A80`**: set ⇒ hero clothes, unset (or NG+) ⇒ casual.
- So "Hero's Clothes on" = **`UNK_2A80`**; the tale STB both flips it live (SHAPE) and writes it
  persistent (demo00). F-2's `ba_wait_l` is Grandma's cradle inside this same STB.

**Rebuild — BUILD LOG (2026-07-26, History "go for it"):**
- ✅ **STB-package player EXISTS natively** — `dEvDtStaff_c::specialProcPackage()` →
  `dEvt_control_c::getStbDemoData(name)` (`d_event_data.cpp:1240/1289`, `d_event.cpp:1286`). The
  "can we play an STB package" blocker is CLEARED — it's TP's own JStudio path.
- ✅ **`tale.stb` staged** — present in our `arcs_lib/Demo01.arc` (with `tale_2.stb`, `ba_wait_l`,
  the vfuku present clips).
- ✅ **`TALE_DEMO` merged into `R_DL01/STG_00.arc`** (idx 1; `Ba1_Get_Itm` soup stays idx 0). Verified
  DN-2 PAL0 unchanged, only `event_list.dat` touched. Carries `FileName=tale.stb`, `Stage=LinkRM`.
- ✅ **Get-item box already dropped** for `grant_outfit:heros` (wear-change only, `d_ext_npc_mount.cpp`
  ~1234-46).

**PLAYTEST 1 (2026-07-27) — didn't fire; ROOT-CAUSED + FIXED:** trigger loaded, `ba.tale_window`
auto-armed, geometry fine (§52 shows `NPC_BA` + Link at the loft `(-225,375,-55)`, inside the box),
flags fine — the ONLY blocker was `ensureDemoArcResident("Demo01")` failing (`[RegionTrig] demo
archive 'Demo01' would not load`). Cause: `Demo01.arc` lived only in `arcs_lib/`, and the
custom-asset gate (`custom_assets.cpp:1657`) mounts arcs_lib stems ONLY when named `arc=<stem>` in
an `npc/*.ini`; `Demo01` is named only via `idle_attached_arc=` (loads via `resLoad`, which is why
F-2's pose worked, but NOT via `setObjectRes`). `Demo02` works because it's in `arcs/`. **FIX
(data): copied `Demo01.arc` → `arcs/` (parallels Demo02).** No rebuild.
Secondary (Engine): my 1-section `region_triggers.ini` parsed into **69 duplicate triggers** (guard/
parse bug); and `NPC_BA demoActorID=0` — Grandma needs demo-actor binding so the STB drives her
(else static during the tale).

**PLAYTEST 2 (2026-07-27) — data spine COMPLETE, blocker is now ENGINE binding.** Demo01 fix worked:
`[PACKAGE] PLAY FileName='tale.stb' … match=yes demoArc='Demo01' stb=OK`; `dDemo_c::start` parses
(same STB magic `53544200 feff0003` as awake.stb) → `mode=1`. Trigger→event→STB all verified. BUT
the tale STB drives NOTHING: `§52 read-back 'NPC_BA' demoActorID=0 actor=NONE enables=0x00` (Grandma
unbound) and no `ENABLE_SHAPE`/clothes swap for Link — vs awake's `NPC_LS demoActorID=2 actor=bound
enables=0xFF`. So the empty-cast demo plays and ends invisibly. **This is the §52 demo-actor binding
gap = THE blocker now.** Engine: bind the tale cast (NPC_BA as demo actor + the player SHAPE drive),
mirroring how the awake path binds NPC_LS. (Also still open: the 69-duplicate region-trigger parse.)

**ENTRY TRANSITION (user obs, 2026-07-27): camera-recenter → FADE-TO-BLACK → event.** DECOMP-verified:
`tale.stb` parse shows a `camera` object (recenter IS in the STB, rides the binding fix) but **NO
fade track** (blocks are only `JACT`/`JMSG`/`JSND`). The fade-to-black is **NOT a separate framework
call** (CORRECTED — two dead ends checked: `d_a_tag_event::cancelShutter()` is a flag gate for
`getType()==4/7/0xC`, not our `0x0A`, and does NO fade; `dEvent_manager_c::specialCast_Shutter` is
the physical DOOR shutter `daShutter_c`, not a screen fade). **The fade is almost certainly INSIDE
`tale.stb`**: its FIRST block is `JFVB` (JStudio Function-Value Block), and the demo exposes
`dDemo_ambient_c`/`dDemo_light_c`/`dDemo_fog_c::JSGSetColor` — so the STB drives scene colors to
black. Block order: `JFVB, JACT×5, JCMR, JMSG, JSND`. **Implication:** the fade RIDES the
demo-actor binding / playback fix — it is NOT extra work. Once the STB drives correctly (binding),
the `JFVB` fade + `JCMR` camera recenter play automatically. (Exact fade frame-range = the JFVB
keyframes; confirm via noclip playback if precise numbers are needed.)

**EXACT `tale.stb` CAST (parsed from the STB block table, 2026-07-27) — the precise binding spec:**
| STB block | object name | binds to | status |
|---|---|---|---|
| `JACT` | **`Link`** | the player | player setDemoData path |
| `JACT` | **`Ba1`** | **`NPC_BA` (mount Grandma)** | ❌ `demoActorID=0` — THE gap |
| `JACT` | **`d_act0`** | `d_a_demo00` body-double | ❌ not spawned/bound |
| `JACT` | **`d_act2`** | `d_a_demo00` body-double | ❌ not spawned/bound |
| `JACT` | **`d_act3`** | `d_a_demo00` body-double | ❌ not spawned/bound |
| `JCMR` | **`camera`** | event camera | rides binding |
| `JMSG` | **`message`** | message control | rides binding |
| `JSND` | **`SE`** | sound | rides binding |
| `JFVB` | 8 function values | **CAMERA recenter** (eye/target/FOV, 0–3.5s) — NOT a fade | rides binding |

So the tale cast = **`Ba1`(→NPC_BA) + `Link` + three `d_act0/2/3` demo00 doubles + camera**. The
`d_act` doubles are the `d_a_demo00` cutscene body-doubles — one is Link's clothes double whose
`ENABLE_SHAPE` does the texture swap + sets the `UNK_2A80`-equiv (§230). Engine must spawn+bind these
exactly as the **awake path (Demo02) already does for its cast** (that path binds `NPC_LS`
demoActorID=2 + its demo00 doubles; replicate for `NPC_BA` + `d_act0/2/3`). **CORRECTION (noclip pull, 2026-07-27):** the `JFVB` is the **camera recenter**, NOT a fade — 8
function-value tracks (`idNo 1`=FOV≈40.1°; `idNo 2-7`=eye/target coords in LinkRM interior space;
`idNo 0`=short ramp), range 0–3.5s. So `tale.stb` = camera-recenter + cast, and the **fade-to-black
is a SEPARATE mechanism NOT in this STB** (the user was right; History wrongly walked it back once).
Likely "the rest of the event" after the fade = **`TALE_DEMO2` / `tale_2.stb`**, which is NOT yet
merged. Open: find the fade source (game `mDoGph` fade, or a demo-to-demo transition) and whether
`TALE_DEMO2` is the main clothes content. (noclip is NOT blocked — earlier "blocked" was a transient
nav failure.)

**RESOLVED (user WW obs + noclip, 2026-07-27) — the fade is the CUTSCENE-COMMENCEMENT (entry) fade:**
The fade-to-black is the **gameplay→cutscene ENTRY transition**, fired as the demo COMMENCES — it
hides the camera cut + actor repositioning at demo start. It is NOT an outfit-swap cover and NOT
between demos. **noclip structurally cannot show it**: noclip renders only the cutscene proper
(starts already inside it, no gameplay to transition from) — every reload drops straight into the lit
scene, no fade. (History over-thought this into a "swap cover" once — WRONG; corrected here.)

Faithful entry order:
1. Link climbs to loft → region trigger.
2. brief camera to Grandma → **FADE-OUT to black (commencement)** — game fade, `mDoGph_gInf_c::startFadeOut`; hidden by black: camera cut to demo + cast reposition.
3. **FADE-IN → `TALE_DEMO`/`tale.stb` plays** (recenter beat + Grandma presents the folded clothes, screenshot-confirmed).
4. the give continues (`TALE_DEMO2`/`tale_2.stb` = Link now wearing them; `tale_1` = talk). The clothes SHAPE-swap is INSIDE the demo playback (ENABLE_SHAPE), a separate concern from the entry fade.

**History DONE this pass:** `TALE_DEMO2` merged into R_DL01 (events `Ba1_Get_Itm`/`TALE_DEMO`/
`TALE_DEMO2`; PAL0 OK); `tale_2.stb` staged in `arcs/Demo01.arc`. **Engine spec:** wrap the demo
COMMENCEMENT — region trigger fires `fade-out → order TALE_DEMO → fade-in` (`mDoGph_gInf_c::startFadeOut`).

**Commencement fade — CROSS-REFERENCED TO CODE + STB DATA (authoritative, not a video guess):**
The fade is a `d_a_demo00` command, prm **id 9 = BLACK fade** (`d_a_demo00.cpp:759-783`, readable):
`mDoGph_gInf_c::startFadeOut(count)` / `startFadeIn(count)`, `count` read from the STB. A byte-scan
of `tale.stb` AND `tale_2.stb` finds the command at analogous offsets — identical bytes
`00000009 29020014 00000000` — count `0x14` = **20 frames**. So the fade count is **20f**, from
game data (the ~15f YouTube read was close but the STB is exact). Direction pairing (out vs in) not
fully decoded — needs the JStudio object-data-paragraph parser — but the count byte (20) is
unambiguous. Structure from GC footage (@677-681s, ±1-2f): recenter (~21f) → fade-out → **black hold
~37f** (setup: camera cut + cast reposition + Demo01 load) → fade-in → tale.
**Engine:** fade-out/in = **20f** (STB `0x14`, `mDoGph_gInf_c::startFadeOut/In`); black hold = NOT a
constant — hold until `TALE_DEMO` resolves, then fade in. The whole fade is issued by the demo00
double (`d_act*`), so it comes with the cast-binding — Engine may not need to hand-fire it at all if
the demo00 commands are executed (unlike noclip, which skips them). (`d_a_npc_ba1::event_actionInit`
is stubbed; this came from `d_a_demo00` + the STB bytes, not ba1.) Cast-binding (`Ba1`/`Link`/`d_act*`/`camera`) is still the separate playback blocker.

## NEXT STEPS (tale cutscene, 2026-07-27)
1. **Engine — bind + RUN the tale demo00 cast** (`Ba1`→NPC_BA, `Link`, `d_act0/2/3` demo00 doubles,
   `camera`), mirroring the awake/Demo02 path. If the `d_a_demo00` commands execute, the fade (20f),
   camera recenter, and clothes SHAPE-swap all come from the STB automatically — noclip skipped them,
   our engine shouldn't. This is the single blocker.
2. **History — exact STB extraction** via a faithful JStudio parser (see ferry). Unblocks: the fade
   out/in direction split, the SHAPE/`UNK_2A80` timing, the camera curves — anything we currently
   byte-scan.
3. **Fade fallback:** if Engine fires it explicitly instead of via demo00 → `mDoGph_gInf_c::startFadeOut(20)`
   /`startFadeIn(20)` (STB `0x14`), black hold until `TALE_DEMO` resolves.

### ⇄ FERRY → Housing: JStudio/STB parser (JSystem-referenced)
**Ask:** build a faithful STB reader in `tools/ww_crew_restoration_skeleton`, mirroring the decomp
`D:/XXXXXXX/WW DP/src/JSystem/JStudio/` (the authoritative parser) rather than hand-rolling/byte-scanning.
Needed layers: (a) STB header → typed block walk (`stb.cpp`); (b) `TParse_TParagraph` iterator
(`u32Type`/`u32Size`/`content`, `jstudio-data.h`); (c) object-actor **data-paragraph decode**
(`jstudio-object.cpp` `do_data` → id + content) to read the `d_a_demo00` commands — the fade
`id 9 [dir,count]`, the SHAPE/`UNK_2A80` writes; (d) FVB function-value curves (`fvb.cpp`/
`functionvalue.cpp`) for camera eye/target/FOV keyframes. **Why Housing:** parsing/tooling help;
JSystem is the reference. **Payoff:** exact reads (fade out/in split, SHAPE timing, camera) for the
tale AND every future WW cutscene, replacing my byte-scan estimates.

**Remaining (Engine), with History trigger-data ready:**
1. **Loft region trigger** — ✅ DATA DRAFTED & PINNED: `population/region_triggers.ini` `[tale_loft]`
   (center `-413.6,375,314.7`; XZ radius 1000; Y half-band 400; `arm_if_flag=ba.tale_window`,
   `done_flag=ba.clothes_given`). Coords PINNED — our interior reuses LinkRM's native frame verbatim
   (interior_placements Ba1 rows == donor), and Link's loft spawns fall inside the box, so it arms as
   he wakes. Engine still needs the **region-check mechanism** (no native `d_a_tag_event`) to consume
   this and fire `TALE_DEMO`; currently the scene triggers on TALK — swap to the region.
2. **STB residency + resolve** — verify `getStbDemoData("tale.stb")` resolves with `Demo01.arc`
   resident in R_DL01, and whether the `Stage=LinkRM` package field matters cross-stage.
3. **SHAPE → texture swap + persistent flag** — drive Link's clothes from the STB's `ENABLE_SHAPE`
   (`setDemoData`) + a `UNK_2A80`-equivalent, replacing the talk-time `setClothesChange`.
The get-item box stays removed.

**Layer reconciliation (Follow-up B):** telescope beat = layer 0 (no bit); tale/clothes beat =
**layer 2** (`UNK_0E20`). Our port OMITTED `ACT2` — revisit so the tale beat's layer loads. The
prior `UNK_0E20`="kidnap" label is contradicted here (it gates the tale) — IVAN: leave 0x0E20's
human name unverified.

## Face estimate (№230): one WW opening expression on TP Link's rig

Chosen beat: **the wake "notice"** (the STB's own SE track names it: SLEEP → AWAKE → NOBI
(stretch) → NOTICE, №221) — WW Link's eyes closed through the first beats, then the wide-eye
notice when Aryll's voice lands.

- WW mechanism: pure texture panes — closed-eye texture swapped to wide-open; mouth texture to
  the small open "!" shape. No geometry moves.
- TP Link's rig (№229 evidence): eye/mouth texture-pattern channel (`mFaceBtpHeap`) PLUS a
  skeletal face BCK channel (`mFaceBckHeap`, per-expression loops, demo-resolvable by resource
  index at alink:8813).
- **Estimate:** the mapping is an UPGRADE, not a compromise. TP Link's own opening proves he has
  canonical sleep and surprise faces natively — so WW's closed-eyes pane maps to TP's sleep face
  (BTP eye-closed + relaxed face BCK), and the NOTICE beat maps to TP's surprise set (eye-wide
  BTP + brow/jaw BCK actually MOVING — something WW's rigid head cannot do). The translation
  table entry costs one row: WW demo face cue N -> TP face BCK/BTP pair. The face channels are
  demo-resolvable on TP's side already, so the wiring point exists; the work is the table and
  choosing TP's nearest expression per WW cue — an authoring task, not an engine one.

## THE SCENE, RECONSTRUCTED END-TO-END (№231)

**Dialogue arc = donor message rows 539-545 (consecutive block), give moment between 544 and 545.**
Row indices with a characterizing note (full text flows data->data at authoring time, R6):

| row | beat |
|---|---|
| 539 | her greeting as Link arrives (player-name insert) |
| 540 | the time-flies birthday lead-in |
| 541 | reassurance ("don't look so disappointed...") |
| 542 | the olden-days tradition — boys dressed as the hero at coming of age |
| 543 | the family shield on the wall — SEEDS THE LATER SHIELD BEAT in the same room |
| 544 | comprehension check — the last line BEFORE the give |
| — | **`Ba1_Get_Itm` pack fires: Ba1 `WAIT→GETITEM→WAIT`; Link `001n_wait→011get_item→001n_wait` (the hold-item-up pose); CAMERA staff alongside** |
| 334 | the item-get box ("received this treasured item...") |
| 329 | "HAPPY BIRTHDAY!" headline row |
| 545 | post-give: "they suit you perfectly" — proof the wear switch happens INSIDE the scene |

Neighboring rows for later states: 546 (go call Aryll — the NEXT questline hook), 548 (outfit
remark — likely lead-in variant), 552+ (post-kidnap states). Row order in the BMG matches play
order for the scene block; exact branch conditions live in Ba1's (Nonmatching) message tree and
will be settled by in-scene observation at wiring time, not guessed (IVAN).

**Implementation shape now fully known:**
1. Dialogue rows 539-544 through the demo-message pipeline (№217 pattern, data->data).
2. The pack: our merged event system already hosts event packs — `Ba1_Get_Itm` staffs are
   Ba1 + CAMERA + Link with six cuts total; the receiver equivalents are her mount's talk anims,
   the camera system, and TP Link's own get-item demo (he has a native item-get pose+jingle).
3. The give: item box (row 334 text, WW prop visual per covenant) + set our 2A80-equivalent
   mod flag; TP Link's wear switch per the user's pending design decision.
4. Row 545 as the post-give line; row 546 chains to the telescope beat (Aryll) — the questline
   continues in-sequence.

## №232 — wardrobe/store-own + vanilla-clothes integration research (COMPLETE)

**Verdict: the outfit system already owns every integration point the scene needs.**

| need | existing surface |
|---|---|
| Hero's Clothes as a wardrobe entity | `D_ALBW_OUTFIT_HEROS` — already in the outfit enum; ownership = native stash save bits (691-694) |
| The give = grant ownership | `dAlbwOutfit_recordOwnedByItemNo(itemNo)` — built for grant sites ("call alongside dMeter2_grantRentalClothes") |
| Store/own + D-pad cycle pickup | automatic once the stash bit is set — cycle walks owned kinds; Postman storage covers natives (`isStorableOutfit` = has storage bit) |
| The wear switch | `dAlbwOutfit_equip(D_ALBW_OUTFIT_HEROS)` — async-initiate, owned-only, mutual-exclusion, transition-safe |
| **Vanilla clothes changes** | `dAlbwOutfit_syncWornOwnership()` — per-frame "own what you wear": seeds the stash bit from `dComIfGs_getSelectEquipClothes()`, EXPLICITLY designed so vanilla-acquired clothes (Ordon at start, Hero's post-Faron, Zora/Magic via story) register "without per-grant-site hooks" |
| Crash class from the sumo era | already fenced: `canTouchLinkModel()` (blocks during transitions/pause/OPEN MESSAGE), `processPendingEquip()` drain, the menu-leave fix |

**Two-direction vanilla compatibility falls out for free:**
- Our scene grants first → vanilla's later Faron beat re-grants idempotently (bit already set; the
  native change itself is the same select-equip path the menu uses — no conflict).
- Vanilla grants/changes first (any order, any path — story, menu, workshop) → the own-what-you-
  wear sync records it; our scene then only needs its narrative beat (the box + flag), ownership
  already true.

**One timing wrinkle to verify at wiring:** `canTouchLinkModel()` returns false while a MESSAGE is
open — and the give happens inside a dialogue scene. So the scene's equip call QUEUES (by design)
and drains right after the item box closes — which is exactly the donor's own visible order (box →
wearing → row 545 "suit you perfectly"). Playtest confirms the drain lands before 545 opens.

**If the design decision goes to option 2 (WW-style outfit):** the enum pattern supports additions
(Deity precedent: owned-readable, out-of-cycle) — new kind + save bit + assets; everything above
still applies, only the asset work is added.

## №233 — where vanilla WW stores the clothes (answer: nowhere)

`dItemNo_FUKU_e` (0x32) is presentation-only: `VFUKU` arc (the held-up clothes-bundle prop),
`clothes.bti` icon, get-message 0x19F, `d_a_demo_item` display class; `NEW_FUKU_e` (0x37) =
assetless variant. NO status-screen, menu, or save-inventory presence anywhere in the donor —
the wear bit (2A80) IS the storage.

- 2nd wheel page: clothes stay OFF it (WW's wheel never carried them).
- WW start menu: additive design — surface the WARDROBE entry (stash bit) with `clothes.bti`.
- Handover prop = VFUKU bundle via the standard extract→adapt pipeline (covenant-clean).
- Msg id note: item table says 0x19F; BMG row 334 — different id spaces, resolve at wiring.

## №236 — pass opened: steps 1-2 DONE, step 6 resolved

- **Icon verify PASSED**: `clothes.bti` = C8 48x48 + RGB5A3 palette (Yaz0'd member); decoded
  preview = the green tunic, perfect. Conversion to the pipeline's direct RGB5A3 is mechanical.
  Icon archive survey: 108/114 C8 — the same converter covers the whole future menu-icon set.
- **`Vfuku.arc` adapted and installed** (both arc folders; vfuku.bdl TEV-normalized).
- **Get-item presentation = the bow's PROVEN PATH** (wind-waker-item-work.md): TP itemmdl.arc has
  no clothes model, so vfuku sources from OUR arc through the same CreateItemHeap-branch +
  patchModelForPc + locked TEV pipeline. LANE: itemmdl impl = Cursor; coordinate, don't solo.
- **Dialogue box**: current = ALBW-postman box (user called it); formatting gap cause verified
  (our wrap/pagination + pane binding vs TP's centered native formatter). Swap to the Shade
  Watcher's box = queued work item, cosmetic layer, must not break suspend/resume.

## №240 — step 6 SHIPPED (Cursor, 2026-07-22)

Engine presents the clothes bundle on host item `0x2F` via `getitem/clothes_bundle.ini` + overlay
arc (no WW name literals in exe). `grant_outfit:heros` fires present-demo / `DEFAULT_GETITEM`.
User verify: Grandma give → get-item pose with green bundle; wardrobe grant still works.
History: may trim `ww_ref=334` double-message / wire full Ba1 choreography next.

## №242b — vfuku crash FIX (Cursor, 2026-07-22)

Stale bow-era create (`getObjectRes` cast on BDLM/raw bmd3) → AV. Now uses mount
`acquireModelData` + boots/Ivan light/draw (no bow TEV scope). Re-verify give beat.

## №242c — BRK-init crash FIX (Cursor, 2026-07-22)

Crash #2: host `0x2F` BRK (`F_gD_rupy` idx 7) bound to vfuku mats → AV in
`searchUpdateMaterialID`. Clothes-bundle path now skips all TP anm init (donor FUKU
has none). Re-verify give → get-item pose.

## №253 — get-box kit SHIPPED (Cursor, 2026-07-22)

Whole kit rides `getitem/clothes_bundle.ini` (+ optional `clothes_bundle_text.ini`
overlay): `get_icon` (direct RGB5A3 BTI, oversized get-box alloc), `get_text` (msg
`host+0x65` body swap), `hand_offset_*` / `max_scale` for present-demo raised-hands
anchor. No WW name literals in exe. User verify: icon + text + hand seat.

## №255 — ladder climb gate SHIPPED (Cursor, 2026-07-22)

§63 r2: line check had wallCode=4 on the interior mount but `ChkWallHit==0`
blocked entry. Admit codes 4/5 without wall-hit (GLOBAL_e stays). Ladder-top
arrival cutscene still queued behind a working climb. Anchor nudge: `y=120 z=48`.

## №242 — FIRST PLAYTEST: crash + fidelity corrections (user, 2026-07-22)

**Vanilla scene corrections (user, direct game knowledge — these override №231's assumptions):**
- Grandma is on the **SECOND floor**; WW Link **climbs the ladder** to reach her.
- The scene triggers on **arriving at the ladder top** — a small cutscene — NOT talk-on-sight.
- During the cutscene she **carries the Hero's-clothes bundle in hand** (not before it) — the
  №218 attach system + a show/hide state gate (the telescope pattern, REQUIRED not optional).
- Then the get-item handover.

**Defects observed:** (1) CRASH at get-item start — Cursor's vfuku present-demo, dies at model
load/first-draw (patch-step suspect; forensics in bus doc); (2) ladder unclimbable — REGRESSION;
(3) Grandma on first floor; (4) dialogue text cropped (ALBW postman box — donor-wide fix =
native dMsgFlow_c + text injection, the Shade Watcher pattern); (5) Link freely moves during
dialogue (fixed by the same native-flow swap — speak events lock the player); (6) double-message
risk between our ww_ref=334 box and the native get-item message (trim ours when present-demo
lands).

## №243 — fidelity research COMPLETE (user items 5+6, data-proven)

**Her position (item 5) — DATA-PROVEN from `LinkRM/Room0.arc` ACTR:** four Ba1 placements,
params = her story-state variants:

| params | pos | reading |
|---|---|---|
| **0** | **(-225.0, 375.0, -55.0)** | **y=375 = SECOND FLOOR (the loft) — the birthday-scene state, as the user described** |
| 1 | (-308.1, 0, 379.1) | ground floor state |
| 3 | (575.1, 0, -27.3), ry=-10194 | ground floor state |
| 4 | (347.1, 0, 129.5), ry=-16384 | ground floor state |

Our mod currently spawns a ground-floor variant → the scene wants **params=0 upstairs**. Which
params spawns when is her actor's state logic (Nonmatching) — map by story flags at wiring, not
guessed. Same room also holds `SPitem` = the wall-shield field item (the get_shield beat's prop).

**The held bundle (item 6) — mechanism found in HER OWN ARC:**
- `ba_cloth.bdl` — the cloth-bundle model; textures decode **green-dominant** (dark folded-tunic
  green) — verified-by-convergence as the hero's-clothes bundle (green cloth + carry anim + the
  user's vanilla observation; flagged, not blindly asserted, per IVAN).
- `hold.bck` — her CARRY animation (10 KB — a substantial pose set).
- `05_ba_cut8_wait.bck` — a cutscene wait ("cut8" = the scene's demo cut).
- **Our manifest ALREADY loads `ba_cloth.bdl` as her companion model** — the remaining work is the
  STATE GATE: hidden by default, shown + `hold.bck` during the scene (the telescope's show/hide
  gate pattern — REQUIRED per [[feedback_port_full_state_machines]], now with its second customer).

**The scene trigger (user): the LADDER-TOP arrival starts a small cutscene** — not talk-on-sight.
Trigger = arrival volume at the loft edge; plays: her hold-state + dialogue arc + handover.

## №244 — two refinements (user, while Cursor wires the crash fix)

**(5 refined) DATA-DRIVEN PLACEMENT AS THE STANDING SYSTEM, not a one-off tweak.** The end goal:
follow WW's OWN mechanism — the room data carries ALL of an NPC's placements (Ba1 × 4, keyed by
params = story state) and the GAME selects which spawns as progress advances. Our spawner should do
the same: carry every param variant from the census and select by story state (our mod-flag
equivalents of the №222 ladder), so future story changes move her — and EVERY NPC — automatically.
No hand-moved positions, ever; manual placement is the anti-pattern. This generalizes: the same
params-variant selection applies island-wide as the questline advances NPCs through their states.

**(6 confirmed + a standing mystery SOLVED): the "green model in the middle of her head" —
present since her model landed — IS the mis-mounted clothes bundle.** `npc_ba.ini` mounts
`companion_model=ba_cloth.bdl` with NO `companion_mode` → default mode 0 = synced base TRMtx +
own BCK (none loaded) → the bundle renders unbound at her base matrix: the green blob. It was
never a glitch texture; it is the scene's key prop with no state gate. The scene-wiring fix
(hidden by default; shown + bound + `hold.bck` during the give) therefore ALSO fixes the visual
bug — one mechanism, two defects closed.

## №245 — placement system: the VISIBILITY axis (user refinement)

The data-driven placement contract has THREE axes, all from WW's own systems:

| axis | mechanism | evidence |
|---|---|---|
| **1. EXISTENCE (layers)** | room ACTR comes in LAYER chunks (ACT0..ACTb); the №222 story-flag ladder selects the ACTIVE layer; actors on inactive layers DO NOT EXIST | `d_com_inf_game.cpp:192-200` (the ladder) + the per-layer ACTR chunks |
| **2. POSITION/STATE (params)** | within existence, params selects the placement/state variant | Ba1 ×4 in LinkRM Room0 (№243) |
| **3. PRESENTATION (actor-state)** | an actor can EXIST as a process but be neither drawn nor collidable until its scene readies | Ls1 `telescope_proc` finds Bm1 via `searchByID` and sets `m881=1` — he IS live at the telescope beat; his hidden-until-look state is actor-internal (Nonmatching — exact draw/collision gate settled by observation, not guessed; both of the user's hypotheses remain possible for the FINAL presentation flip) |

**Symptoms this explains:** actors floating in mid-sea = layer-blind spawning (rows from inactive
layers, or merged-layer census); the postman standing visible at the mailbox pre-telescope =
missing axis 3 (he's placed right but should not be PRESENTED yet).

**Our system mirrors all three:** census rows carry layer + params; the spawner filters by the
ACTIVE layer derived from mod story flags (№222 mapping); presentation gating = per-actor
visibility state tied to scene flags (the manifest's spawn_if/unless_flag family extends to a
present_if family — hidden ⇒ no draw AND no collision AND no attention, exactly WW's contract).
No hand-tuning at any future story change — the data moves everyone.

## Order of attack (updated)

1. ~~Identify the clothes event~~ DONE (`Ba1_Get_Itm`). Next: parse the pack's staffs/actions
   (merge_event tooling) — dialogue ids, positioning, camera, the item beat, the 2A80 setter.
2. Her scene positioning/state per the pack's staff data (placement itself already exists).
3. Dialogue authoring from donor rows (pipeline exists).
4. Handover presentation (WW item prop) + the wear-state design decision → user.
5. Player anim translation table (research first, small table after).
