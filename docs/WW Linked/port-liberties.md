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
permanent port choice) · `RECONCILED` (restored to donor) · `BY-DESIGN` (accepted by design — **no
reconciliation owed**).

## Entries at a glance · Librarian-maintained index

> Numeric order for scanning (the entry **bodies** below run in priority order, and are
> authoritative — this table just mirrors each header's status). Updated 2026-08-08: L-10 added.

| Entry | Deviation | Status | Grade\* |
|---|---|---|---|
| L-1 | Loft tale-trigger geometry | `RECONCILED` ✓ | `DG-0` |
| L-2 | Demo-prop lighting = port's WW-item recipe | `OPEN` (likely reconciled by Housing lighting port) | `DG-3` |
| L-3 | Tale-window arm auto-armed, not the donor beat chain | `OPEN` (blocked on A_mori) | `DG-3` |
| L-4 | Clothes-get renders as the talk box, not the donor item box | `OPEN` | `DG-3` |
| L-5 | Outset `ikada_h` static — bob/flag/rope deferred | `OPEN` | `DG-4` |
| L-6 | Flowers as `NPC_YAFLW` J3D — **measured native** (§780) | `VERIFYING` | `DG-1` cand. |
| L-7 | Donor `Tag*` actors → port RegionTrig | `RATIFY` (recommended) | `DG-4` |
| L-8 | Ocean `Salvage` points omitted pending the mechanic | `OPEN` (deferred-mechanic) | `DG-4` |
| L-9 | WW mount ground shadow = flat up-normal projection | `RATIFY` (recommended) | `DG-4` |
| **L-10** | WW cutscene staffs performed by **TP's Link** | **`BY-DESIGN`** (2026-08-08) | *ungraded* |

**Tally:** 5 `OPEN` · 1 `VERIFYING` · 2 `RATIFY`-recommended · 1 `RECONCILED` · **1 `BY-DESIGN`**. Grades: 1 `DG-0` · **1 `DG-1` cand. (L-6)** · 3 `DG-3` · 4 `DG-4` · 1 ungraded — the **first `DG-1` candidate** (L-6, §780: `dFlower` measured native; pending user visual confirm).

**\*Grades CONFIRMED (§710)** by History (entry-owner for all ten); L-8 corrected `DG-3`→`DG-4` in the confirm pass. Doctrine below.

### Deviation grades (DG) — how far a deviation strays, on what evidence

> §708 semantics (History classified); Librarian-encoded 2026-08-11 (format is the Librarian's call per the handoff). Statuses answer WHERE an entry stands in reconciliation; **grades answer HOW FAR it strays and ON WHAT EVIDENCE.** Grades attach to entries; they never reclassify one — **no fourth class** (§700).

| grade | meaning | earned by |
|---|---|---|
| `DG-0` | byte-exact / none — donor bytes or algorithm verified equal | byte identity (sha256, differ=0) |
| `DG-1` | equivalent — different mechanism, same observable output | differ / golden-trace showing zero on the stated metric |
| `DG-2` | bounded — deviation MEASURED and quantified | a cited measurement artifact (axis-C = canonical numeric instrument, §700) |
| `DG-3` | unmeasured — deviation known, no instrument has quantified it | **default** for every graded entry until an instrument runs |
| `DG-4` | structural — donor mechanism absent or replaced by a different system | inspection; numeric comparison inapplicable — behavioral checklist |

**Rules (encode of §708):** (1) a grade is **EARNED downward, defaulted upward** — start `DG-3` (or `DG-4` on inspection); no instrument, no lower number. (2) **BY-DESIGN entries are UNGRADED** — reconciliation distance is undefined by definition; grading one puts an unanswerable measurement on the owed list (L-10 is the receipt, not a gap). (3) grades are **orthogonal to statuses** — `RATIFIED`+`DG-4` is coherent; a `RECONCILED` entry that can't earn `DG-1` has the wrong *status* (the grade audits the status). (4) **movement cites its instrument** (`DG-3`→`DG-2` names the measurement; `DG-2`→`DG-1` names the differ run). One vocabulary: DG also carries into V8's banner field if lineage-grading wants it (§708) — do **not** invent a second scale.

**The `DG-3` / `DG-4` criterion (§710 — the confirm pass earned it):** *is there a present mechanism to measure against?* Grade `DG-4` when the donor mechanism is **absent or replaced** in the port (dFlower packet, waveRot, Tag* actors, salvage, Acch shadow); grade `DG-3` when the mechanism is **present** and the deviation is a value / wiring / routing difference against it (ambient values, arm wiring, box routing). Test: could an instrument measure the deviation against a present mechanism TODAY? If nothing exists to measure against, `DG-4`. Consequence: **`DG-4` → `DG-2` requires porting the mechanism first** — which is why §708's "nothing earns `DG-1`/`DG-2` yet" and Foundry's instrument roadmap meet at the same gap.

---

### `BY-DESIGN` — the third status (L3, user-ruled; History classifies · Librarian records)

**What it is:** a deviation that is a *design decision of this port*, not a shortfall against the
donor. Nothing is owed. It is recorded here — rather than left out of the ledger entirely — so the
reasoning survives, and so nobody "reconciles" it later by mistaking a choice for a debt.

**Why it exists.** The other four statuses all imply motion: OPEN and VERIFYING are owed, RECONCILED
is finished, RATIFIED is a debt the user forgave. None of them fit a deviation that was never a debt.
Without this status such entries either sit on the owed list forever generating phantom work, or get
deleted and lose their reasoning — and a deleted decision is indistinguishable from an oversight the
next time someone reads the donor and finds a difference.

**The distinction from RATIFIED, stated once because it is the whole point:**

| | the donor's way was | we chose | what is owed |
|---|---|---|---|
| `RATIFIED` | achievable | not to, for cost/risk | nothing *now* — a forgiven debt |
| `BY-DESIGN` | **not applicable** | the only coherent option for this port | **nothing ever** |

RATIFIED says *we could have, and you said we needn't*. BY-DESIGN says *the question does not
arise* — usually because WW's answer presupposes a WW that isn't here.

**The canonical class: TP-Link-interaction deviations.** WW content in this port is played by
**TP's Link** — his proportions, his animation set, his interaction ranges, his acting vocabulary.
Every deviation that follows from *which hero is standing there* is BY-DESIGN, because the donor's
value describes a character the port does not contain. Filing those as OPEN would put "make TP Link
into WW Link" on the owed list, which is not a port liberty — it is a different project.

**Classifying test** (History applies it; if the answer is not clearly yes, the entry stays OPEN —
BY-DESIGN is the status most likely to be abused as a way to close hard entries):
> Would restoring the donor's value require changing something the port is not trying to change —
> the receiver's hero, engine, or platform? If yes, BY-DESIGN. If it would only cost effort, it is
> OPEN or RATIFIED.

**What it is NOT:** not a home for hard problems, not a synonym for "works well enough", and never
self-assigned by the lane that took the liberty — same separation the DO-NOT registry uses.

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

## L-6 — Flowers render as `NPC_YAFLW` J3D models, not the donor raw-GX `dFlower` packet · **VERIFYING** (§780 — measured native)

> **⚠ CORRECTION (History §780, MEASURED — Librarian-recorded 2026-08-11).** `dFlower` is **NATIVE** now: `d_a_ext_vegetation.cpp` (1,968 lines) carries both tiers with donor call sizes, a manifest-gated pack loader, 64-slot sway, cut particles on donor IDs 0x3DE/0x3DD; its banner declares the `d_flower.cpp` donor and the kind dispatch cites "now NATIVE (241/L-6)"; all 18 `d_flower__*` blobs live in the mod veg pack. **The "not the donor mechanism" claim is retired — it IS the donor mechanism.** → status **OPEN → VERIFYING** (pending user visual confirm: flowers on screen, white tier especially — the btk-era "likely pink not white" note needs eyes); grade **DG-4 → DG-1 candidate** once confirmed. **Named residual (not a blocker):** the `bessou` set (pos3/color3/texCoord3) is absent from the veg pack — sea room 0x21's villa variant only; flowers there draw the standard high set or refuse per the pack's own gate. History nearly wrote a duplicate packet TU — the ledger said OPEN, the tree said otherwise.

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

  > **⚠ RETIRED (History §780, MEASURED — Librarian-recorded 2026-08-11):** `dTree` = `d_ext_tree.cpp` (§366, 849 lines, donor-verbatim); **EXT_VEG `kind==1` rides it.** The swood "unplaced holes" were **STALE** — they route to the native packet. **No swood/`dTree` port owed** (this half of roadmap item #3 is done, not deferred).

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

---

## L-10 — WW cutscene staffs are performed by **TP's Link**, in TP's acting vocabulary · **BY-DESIGN** (2026-08-08)

**Deviation.** WW's event data addresses a `Link` staff with WW's own cut vocabulary —
`001n_wait`, `012unequip`, `005wait_turn`, `035door`. Those name **WW Link's** animations. The port
serves them with **TP Link's** native acting: the evmng `Alink`↔`Link` alias binds the staff, and
§379b's interpreter performs each cut through TP's own procs (`035door` → `DEMO_DOOR_OPEN`, whose
`param0&1` right/left select is the *same* semantic WW reads from the *same* property — the lineage
survives even though the animation data does not).

**Why BY-DESIGN and not OPEN.** Apply the test: *would restoring the donor's value require changing
something the port is not trying to change?* Yes — it would require **WW Link**: his rig, his
animation set, his proportions and interaction ranges. This port's premise is TP's hero in WW's
world. "Reconcile to donor" here decodes to "replace the protagonist", which is not a debt this
ledger can hold; it is a different project. Nothing is owed, now or later.

**What this status does NOT excuse.** Everything that is *not* about which hero stands there stays
owed. Cut **ordering**, **timing**, event **structure**, which staff exists and when it ends — all
donor law, all still reconcilable, and several were in fact reconciled (§423's parallel WW event
stack exists precisely so WW's staff semantics run WW's way). BY-DESIGN covers the *performer*,
never the *performance's script*.

**Scope.** Any deviation that follows from the identity of the acting character. First filed
instance; expect the door and tale demos to be the visible ones.
