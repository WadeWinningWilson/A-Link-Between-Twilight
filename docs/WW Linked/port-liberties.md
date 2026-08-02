# WW Port Liberties — faithfulness-debt ledger

Deviations from WW vanilla that are **port mechanisms achieving a faithful outcome, NOT literal
WW values/behavior.** Each entry is here so it can't be forgotten — it must eventually be
reconciled to the donor, or explicitly ratified by the user as a permanent port choice.

**This is NOT the hard-stop registry** ([DO-NOT.md](../DO-NOT.md) is that). This is the honest
ledger of where we used port values instead of donor bytes.

**Covenant scope (Housing §67 / DN-3):** internal *receiver architecture* — parse timing, model
resolvers, cache/pristine machinery — is the port's to design and is NOT tracked here. This ledger
tracks only deviations that (a) touch what the **player perceives** (geometry, lighting, timing,
content), or (b) were justified by a **claim that must be verified** (e.g. "the donor mechanism is
stubbed"). Mark a liberty the moment you take it; clear it when it's reconciled or ratified.

Status: `OPEN` (unreconciled) · `VERIFYING` (research in flight) · `RATIFIED` (user-approved as a
permanent port choice) · `RECONCILED` (restored to donor).

---

## L-1 — Loft tale-trigger geometry · **RECONCILED (donor byte-exact)** ✓ (2026-07-27)

**Was:** I claimed donor `= 1000/400` and "tuned" to `300/80` at a moved centre `(-225,375,-55)`.
**BOTH wrong.**

**Resolution — parsed the DZR directly:** `LinkRM/Room0 room.dzr` SCOB → the tale `TagEv`
(param `0x02FF110A`) is pos `(-413.6,375,314.7)`, scaleBytes `(10,4,10)`. WW SCOB scale byte =
`scale/10` (byte 10 → float 1.0); `d_a_tag_event.cpp:254` = `SQUARE(scale.x)*SQUARE(100)` →
**xz_radius = 100, y_halfband = 40** (byte/10 × 100). My `1000/400` was a byte×100 mis-scale. Applied
the donor values verbatim to `[tale_loft]`. The donor y-band 40 (`y∈[335,415]`) **already excludes the
ground floor** — the original entry-fire was purely my wrong number, NOT the arming. **Geometry is now
donor byte-exact; no port value remains here.**

---

## L-3 — Tale-window arm is auto-armed, not the donor beat chain · **OPEN (blocked on A_mori)**

**Deviation:** `dExtNpcMount_pollRegionTriggers` auto-arms `ba.tale_window` on `R_DL01` entry, no
prerequisite. **Donor:** the tale (type-0x0A, gated on `UNK_0E20`) is armed by a 3-stage TagEv chain,
NOT by ba1 — Beat B (`A_mori` forest, type 0x2 → `UNK_0101`) → Beat C (`sea` room 44 / Outset, type 0x3,
gated on `UNK_0101` → `UNK_0E20`) → gates Beat D (the tale). Placements captured
(`d_a_tag_event.cpp:90-99,278-311` + DZR parse).

**Why open:** Beat D (tale) is faithful. Beat C (Outset) is implementable — donor coords
`(-200242.8,3700,321928.5)`, r200/y200, `arm_if UNK_0101 & !UNK_0E20`, `set UNK_0E20`.
**RUNTIME-CONFIRMED (DuskTap roster census 2026-07-28):** the Beat C `TagEv` (type `0x3`) spawns at
exactly `(-200242.8, 3700, 321928.5)` — placement verified, no longer just decomp-inferred. **Beat B lives in
`A_mori` (Forest of Fairies), NOT yet restored** — so `UNK_0101` has no faithful setter, and the full
chain can't complete without a forest stand-in.

**Reconciliation:** when A_mori is restored → author Beats B+C as region triggers (donor geometry + the
flag chain) and DELETE the `R_DL01` auto-arm. Until then the auto-arm stays (or move the arm to
first-Outset-entry as a closer stand-in). Marked so it isn't forgotten.

**Player-perceived:** minimal & faithful-in-effect — the tale fires at the donor spot; only WHEN the
window opens (house-entry vs the Outset beat) differs, and in normal play they coincide (vanilla sets
`UNK_0E20` on Outset *before* the player enters the house).

---

## L-4 — Clothes-get renders as the talk box, not the donor item box · **OPEN**

**Deviation:** the "You got the Hero's Clothes!" get (tale.stb JMSG 3095, and the already-have
variant 4410) renders through the DN-4 catalog/talk flow, **not** the donor's item box.

**Donor:** `d_mesg.cpp:1958-1964` picks the box per-message from BMG `mTextboxType` — `==9` ⇒
`dMesg_screenDataItem_c` (the get-item box **with the clothes icon**), else the talk box. The get
obeys the identical suspend/wait/resume contract as talk (History §193 decomp pass).

**Why taken:** our WW get-text lives in the **catalog** (`ww_dialogue_full.txt` `[3095]`/`[4410]`),
NOT the TP BMG — and the native item box (`dMsgScrnItem_c`) reads text by BMG code, so it cannot
show catalog text (an earlier `setDemoMessage(151)` both mis-indexed — TP row 151 is the fairy-bottle
line — and could not render our text; zero `fukiKind=9` boxes ever appeared). Routing the get through
the same DN-4 flow gives the **correct text + step-in-step wait** immediately; the item **icon** is the
only remaining gap.

**Faithful end-state:** feed catalog text into the native item box — either inject the WW get
messages into a BMG the item box can read (`setDemoMessage(code)` → `dMsgScrnItem_c` with icon), or
add a raw-string path to `dMsgScrnItem_c`. Then the get shows the icon box, still step-in-step.

**Player-perceived:** text is faithful (donor string, donor timing); the missing clothes ICON on the
get box is the debt.

---

## L-2 — Demo-prop lighting uses the port's WW-item recipe, not WW's demo lighting · **OPEN**

**Deviation:** `src/d/actor/d_a_demo00.cpp` draw — for WW cel-shade demo models (a `ZA*` toon
texture), we **skip MAJI** and apply the port's clothes-bundle recipe: `settingTevStruct(0)` + neutral
ambient `(90,90,90,255)` + `dWwItemmdl_applyBowMaterialAmbientOnly` (mirrors `d_a_demo_item.cpp:518`).

**Why taken:** the port **added** `setLightTevColorType_MAJI` to demo00's draw (the **donor demo00 has
none**); MAJI blacks-out WW cel-shade models like `fuku.bdl`. So this deviation has TWO parts:
- **Skip-MAJI = FAITHFUL** (the donor demo00 doesn't MAJI at all — we're removing a port addition).
- **The neutral-ambient recipe = a PORT VALUE.** The `90/90/90` ambient is the port's own WW-item
  lighting (the same `d_a_demo_item` uses for bow/clothes), not WW's exact demo-model lighting —
  because WW's cel-shade demo lighting isn't reproducible 1:1 in TP's engine.

**Faithful end-state:** the skip-MAJI is already faithful. The ambient VALUES are a port choice, shared
with every WW item in the port (internally consistent, but not a donor byte).

**Reconciliation:** ratify as a permanent port choice (consistent with `d_a_demo_item`'s WW-item
lighting) UNLESS a faithful WW demo-lighting path is found. Low priority — the outcome (textured, lit
like the port's other WW items) is correct.

**Player-perceived:** yes (the prop's shading), but it matches how the port lights ALL WW items, so
it's internally consistent.

---

## L-5 — Outset `ikada_h` = correct model (`vtsp.bdl`), static — bob/flag/rope deferred · **OPEN**

**§214 → §215:** first static placement used `model=vikah.bdl` and drew a **submarine** (mType-2 model).
Root cause found in the donor: `d_a_obj_ikada.cpp:1430` — `bdl[] = {VIKAE(0), VTSP(1), VIKAH(2),
VTSP2(3), VSVSP(4)}`, `mType = fopAcM_GetParamBit(param,0,4)`. Outset `ikada_h` params `0x21` →
**mType 1 → `vtsp.bdl`** (decomp-backed, not a guess). Manifest corrected to `vtsp.bdl`.

**Remaining deviation (static vs the full actor):** `daObj_Ikada` also (a) **bobs on the sea**
(`mSvWaveOffs*`/`mWaveMaxVelocity` movebg physics), (b) spawns a **separate `MAJUU_FLAG` actor**
(`fopAcM_create(fpcNm_MAJUU_FLAG_e…)`, line 1372), (c) attaches a **rope** (`mpRopeEnd`, Link/ropeend),
and (d) plays **`svship_kaiten.bck`** (idle sway). The static placement renders the correct hull model
at the census position/rotation but omits all four.

**Full-port scope (§216, decomp-mapped, deps verified present in the port):** `daObj_Ikada` mType 1 is a
**rideable moving-BG platform**. A faithful ext-actor port is:
- **model** `vtsp.bdl` (done, static stopgap) ·
- **rideable moveBG** — `cBgW` + collision from `dzb[1]` + `SetRideCallback(ride_CB)` +
  `SetCrrFunc(dBgS_MoveBGProc_TypicalRotY)` (the rock + standable) · *(port has `cBgW`,
  `dBgS_MoveBGProc_Typical`, `SetRideCallback`)* ·
- **wake/splash particles** — `createWave()` → `dComIfGp_particle_set(ID_AK_JN_SHIPWAVE00 / SHIPSPLASH00)`
  at the hull sides · *(port has `dComIfGp_particle_set`)* ·
- **bonbori point-light** — `isBonbori()` (mType 1 true) → `dKy_plight_set(&mPLight)` · *(port has
  `dKy_plight_set`)* ·
- **per-joint node rotations** — `_nodeControl` node callback ·
- **NOT needed for this instance:** path (`mPathId=0xFF` moored → `dLib_pathMove` unused), flag
  (`isFlag`=false for mType 1 → `MAJUU_FLAG` unused), water-snap (`dLib_getWaterY` missing → use census Y).

**§217 — the faithful full port is BLOCKED on missing WW subsystems (found by doing the port work):**
- **Wake/splash particles** use WW-specific particle assets — `dPa_name::ID_AK_JN_SHIPWAVE00`,
  `SHIPSPLASH00`, `SHIPTAIL00` — that are **NOT in the TP port's particle table** (the port has TP's
  `createSimpleEmitterID` set, not these). Reproducing the wake needs the WW ship-wave particles ported
  first (a Foundry-style particle-asset port, cf. the windline `0x31` work).
- **`dLib_waveRot`** (the water-surface tilt that drives `shape_angle.x/z` = the rock) is **MISSING**.
  Only the sin-sway component (`waveAnim1X/Z`, ~1° amplitude) is reachable — an approximation, not the
  water-coupled rock.
- **Reachable with current deps:** the model (done) + the bonbori point-light (`dKy_plight_set`, warm
  600/400/120 glow) + the sin-sway approximation. Modest and partial — the *visible* element (the wake)
  is the blocked part.

**Verdict:** don't pass a partial approximation off as "the port." The correct static `vtsp.bdl` stands
as the faithful-VISUAL Outset ship; the full behavioral port is **gated on porting the WW ship-wave
particle system + `dLib_waveRot`** first — the same WW-systems tier as the `dTree`/`dFlower` packets.

**Player-perceived:** the **correct ship model** is now at the Outset dock (was a wrong submarine),
moored and still. It doesn't rock, throw a wake, or glow — deferred to the full port. **Visual-confirm
pending:** `vtsp.bdl` is the donor's mType-1 model (the "Beedle" label was the user's; the donor picks by
mType) — confirm it reads as the intended vessel.

---

## L-6 — Flowers render as `NPC_YAFLW` J3D models, not the donor raw-GX `dFlower` packet · **OPEN**

**Deviation:** WW Outset flowers (census `pflower`/`pflwrx7` = kind 3 pink, and now `flower`/`flwr7`/
`flwr17` = kind 2 white via `npc_yaflw_w.ini`) are drawn as discrete `NPC_YAFLW` J3D models
(`Yaflw00.arc`/`yaflw00.bdl`) placed per census row.

**Donor:** `d_a_grass.cpp` (`GRASS_e`, kind 2/3) registers each flower with the batched raw-GX
**`dFlower` packet** (`dComIfGp_getFlower()->newData(flowerType, …)`; `dFlower_packet_c::draw` =
`l_OhanaDL`/`l_Ohana_gutDL` display lists, room-tevstr colour). It is an instanced field-flower
system, not per-actor models — and it carries the eaten/cut (`gut`) state.

**Why taken:** the port never ported the `dFlower` raw-GX packet; the pre-existing `NPC_YAFLW` model
substitute already rendered pink `pflower` acceptably, so the white flowers reuse it rather than
block on a second raw-GX packet reimplementation (the grass packet was already a large effort).

**Two sub-debts:** (a) **the white colour is unresolved (§214).** First attempt added
`btk=yaflw00_01.btk` to select white — it **broke the draw** (flowers spawned per the `[Spawn]` log but
were invisible). The btk is **removed**; the white variant now renders as the `yaflw00.bdl` default
(same as pink `pflower`). So the white flowers should now *appear*, but likely **pink, not white** —
true white needs the real `dFlower` texture path (or a verified recolour), NOT another guessed btk.
(b) the donor's instancing + cut/`gut` state are not reproduced (models are static, non-cuttable).

**Faithful end-state:** reimplement the raw-GX `dFlower` packet (mirrors the `dGrass` port) with the
white/pink display lists + cut state, then retire the `NPC_YAFLW` model substitute.

**Player-perceived:** flowers are present at the donor spots; pink is faithful, **white is
test-pending**, and flowers aren't cuttable. Reconcile with the `dFlower` packet port.

---

## L-9 — WW mount ground shadow is a flat up-normal projection · **RATIFY (recommended)**

**Deviation:** WW mount actors draw a **simple round shadow projected straight down** —
`dComIfGd_setSimpleShadow(&pos, pos.y, r, &gndNrm=(0,1,0), …)` (`d_ext_npc_mount.cpp` "I3: simple
ground shadow (no Acch — use up-normal)"). It uses a **fixed up-normal**, not the real ground normal.

**Donor:** WW NPCs/objects shadow-conform to the actual surface via an `Acch`/ground probe (the shadow
tilts and hugs slopes). Ours is flat.

**Why taken:** the up-normal simple shadow is one call, needs no per-actor `Acch` ground chain, and is
visually correct on the flat/near-flat ground the Outset cast stands on. A full ground-conforming shadow
is per-actor cost the roster doesn't warrant yet.

**Player-perceived:** on a slope the shadow won't tilt/hug the surface — low-salience for the current
(largely flat-ground) cast. **Ledger call: RATIFY** unless a sloped-terrain cast makes it visible;
revisit with an `Acch` shadow then.

## Deferred (not liberties yet — unbuilt donor systems, tracked so they aren't forgotten)

- **Trees (`swood`/`swood3`/`swood5` = `GRASS_e` kind 1)** — donor registers them with the raw-GX
  **`dTree` packet** (`dComIfGp_getTree()->newData`, `d_tree.cpp`, 1013 lines). No port equivalent
  exists; they stay **unplaced holes** (№31: a missing prop beats a wrong one) until the `dTree`
  packet is ported. Do NOT substitute a static tree model — swood is a specific `dTree` tree, and a
  wrong-model tree would violate №31. This is the remaining half of roadmap item #3.

---

## L-7 — Donor trigger `Tag*` actors replaced by the port's RegionTrig system · **RATIFY (recommended)**

**Assessed for the P9×P10 join (History, §213).** The donor's invisible trigger tags — `TagEv`,
`TagKb`×10, `TagSo`×5, `TagHt`, `TagIsl`, `TagMSo`, `TagMsg2` — are **not** placed as port actors; the
port implements the ones it needs through its **RegionTrig socket system** (`population/region_triggers.ini`
+ `dExtWw_pollRegionTriggers`). E.g. the tale chain's `TagEv` is the `[tale_loft]` RegionTrig (donor
byte-exact, L-1); the rest map to mechanics not yet restored (`TagSo` = `NpcSo`'s
`_searchTagSo`; `TagKb` = a Hyoi/bird trigger). **Ledger call: RATIFY** — RegionTrig is the port's
faithful mechanism for donor trigger tags (invisible, functional, region-gated); a 1:1 `Tag*` actor port
would duplicate it. Reconcile per-tag only as each dependent mechanic is restored. **Player-perceived:
none** (all invisible).

## L-8 — Ocean `Salvage` points (×19) omitted pending the salvage mechanic · **OPEN (deferred-mechanic)**

`Salvage`/`SalvagE`/`Salvag2`/`SwSlvg` (19 placements) are **ocean-floor treasure-salvage points**
(fished up with the grappling-hook/crane via treasure charts). The salvage/treasure-chart mechanic is
**not restored**, so these invisible markers have no visible content and are correctly unplaced. **Ledger
call (History): DEFER with the salvage mechanic** — not a bug, not a liberty-to-ratify; a hole that opens
only when the mechanic exists. **Player-perceived: none** until salvage is restored.

## §213 — Four "cast gaps" TRIAGED (History, with Engine) — only ONE is a real visible gap

Foundry's P9×P10 join flagged **Dk / HyoiKam / NpcSo / Com_A** as cast gaps (missing at donor-law layer 0).
Decomp triage collapses them:

| Actor | Donor | Verdict |
|---|---|---|
| **`Dk`** | `d_a_bdk` (masked man / `dk_kamen`) | **NOT a gap — correctly gated.** Already mapped `[Dk]→NPC_DK`, `spawn_if_flag=beat.helmaroc`; a later-beat actor, absent at layer 0 by design. The layer-0 "miss" is the gating working. |
| **`HyoiKam`** | `d_a_npc_kamome` variant | **Mechanic-tied — defer with the Hyoi Pear.** It's a normal `Kamome` seagull (which the port DOES spawn ×26) until a Hyoi Pear makes it controllable; the missing piece is the item mechanic, not a visible actor. |
| **`NpcSo`** | `d_a_npc_so` (offshore minigame NPC) | **The ONE real visible gap.** `So.arc` extracted (§⑦); needs an `NPC_SO` proc + its `_searchEsa`/minigame behavior → Engine/Housing build. |
| **`Com_A`** | `daObj_Coming` (`fpcNm_Obj_Coming_e`; cf. `Com_B`/`Com_C`) | **Invisible event object — liberty-class (like `Tag*`).** A scripted "coming/arrival" trigger, not visible cast. Ratify as omitted / RegionTrig-adjacent; not a content gap. |

**Net: of "4 cast gaps," 1 is real** (`NpcSo`), 1 is correctly gated (`Dk`), 1 is mechanic-tied
(`HyoiKam` → Hyoi Pear), 1 is invisible (`Com_A`). The binding layer-divergence verdict still needs the
two `[INFERENCE-NEEDED]` flag rows (ACT8/a/b forced-layer bits) in
[reference-ww-flags-triggers.md](reference-ww-flags-triggers.md) §N confirmed.
