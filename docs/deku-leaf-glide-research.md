# Deku Leaf Glide — research (TP cucco glide ⇄ WW Deku Leaf)

**Goal:** port a Wind Waker **Deku Leaf glide** to Dusklight (TP). Research pass only — no
code yet. Companion asset is the WW **`vleaf`** get-item model already on the TP disc
(`itemmdl.arc` index **0x15**, 2 joints `VLEAF`+`LEAF_MODEL`) — same pipeline as the shipped
WW bow/boots/arrow skins.

Sources: TP = this repo (`src/d/actor/d_a_alink*.cpp/.inc`); WW = `zeldaret/tww`
(`src/d/actor/d_a_player_main.cpp`, `assets/GZLE01/res/Object/Vleaf.h`, `src/d/d_operate_wind.cpp`).

---

## 1. TP already has the glide — it's the cucco carry

The glide state machine exists in full, gated on one predicate:
`checkGrabGlide()` → `checkGrabRooster()` ([d_a_alink_grab.inc:178](../src/d/actor/d_a_alink_grab.inc)) —
*"holding a grabbed actor named `NI` (cucco) or `NPC_TKJ2`, not a gold cucco."* Everything
downstream keys off that boolean, so **re-pointing it at a leaf item is the seam.**

| stage | site | behavior |
|---|---|---|
| entry | [d_a_alink.cpp:17682](../src/d/actor/d_a_alink.cpp) `procAutoJumpInit` | jump/fall while holding cucco → `mMaxSpeed=mCuccoJumpMaxSpeed`, launch `angle=mCuccoJumpAngle`, terminal `field_0x3478=mCuccoFallMaxSpeed` |
| fall clamp | [d_a_alink.cpp:17849](../src/d/actor/d_a_alink.cpp) | `setSpecialGravity(-1.0f, mCuccoFallMaxSpeed, FALSE)` — gentle terminal velocity = the float |
| steer | [d_a_alink.cpp:17753](../src/d/actor/d_a_alink.cpp) | `mNormalSpeed` chases `mMaxSpeed`; stick-steerable |
| updraft (air) | [d_a_alink.cpp:17868](../src/d/actor/d_a_alink.cpp) | `checkGrabGlide() && mWindSpeed.y>0 ⇒ speed.y=0` (hover/rise) |
| updraft (ground launch) | [d_a_alink.cpp:11512](../src/d/actor/d_a_alink.cpp) | `checkGrabGlide() && mWindSpeed.y>10 ⇒ procAutoJumpInit` (geyser launch) |
| anim | [d_a_alink.cpp:17858](../src/d/actor/d_a_alink.cpp) | upper `WALKHBS` (overhead hold); land rate halved ([18125](../src/d/actor/d_a_alink.cpp)) |
| wind source | [d_a_alink.cpp:5751](../src/d/actor/d_a_alink.cpp) `setWindSpeed` | 3 wind collision cylinders `mWindTgCyls`, hit by stage wind actors |
| tuning | [d_a_alink_HIO.inc:1591](../src/d/actor/d_a_alink_HIO.inc) | 4 live sliders: cucco jump max speed / angle / max fall speed / start speed |

## 2. Lineage confirmed — cucco glide IS WW's Deku Leaf glide, model swapped

| | WW (Deku Leaf) | TP (cucco) |
|---|---|---|
| glide gate | `checkFanGlideProc()` | `checkGrabGlide()`/`checkCokkoGlide()` |
| glide proc | `daPyProc_FAN_GLIDE_e` | `PROC_AUTO_JUMP` |
| tuning container | **`mAutoJump` HIO** | **`mAutoJump` HIO** (+ added `mCucco*` fields) |
| wind system | `d_operate_wind`/`d_point_wind`/`d_a_wind_tag` | `mWindTgCyls[3]` |

Both player actors carry the **same `mAutoJump` HIO struct**. TP inherited WW's auto-jump/glide
chassis wholesale and replaced the Deku-Leaf `FAN_GLIDE` proc + billowing model with the
cucco-carry. Same lineage ⇒ near-identical animations.

## 3. The usable model ≠ the get-item model — and it's an ANIMATION, not a mesh

WW switches leaf presentation by proc state ([d_a_player_main.cpp:1155](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)):
- **held/folded** (not gliding): `mpEquipItemModel` + `setShapeFanLeaf()`.
- **gliding** (`FAN_GLIDE`/`FAN_SWING`): `mpParachuteFanMorf` billowing parasol + a dedicated
  `_FAN_GLIDE_` BCK (11 refs).

**Decisive:** WW's `Vleaf.h` arc = **1 BDL, 2 joints (`VLEAF`+`LEAF_MODEL`)** — identical to TP's
`itemmdl` 0x15. So **we already have the mesh.** What TP lacks is the **billow BCK** that opens
the `LEAF_MODEL` joint from its folded base size into the parasol. The "stretch/warp from a base
size" is that joint animating from rest.

## 4. Translation path (de-risked)

1. **Model — solved.** `vleaf` on the disc, same pipeline as the shipped WW skins.
2. **Glide physics — solved.** TP cucco chassis does float/steer/updraft-hover/ground-launch.
   Re-gate on a leaf item (new `checkDekuLeafGlide()` OR'd into the glide sites).
3. **Billow — the one real task, and it's small.** Reproduce procedurally: WW uses the exact
   idiom two lines away for the firefly bottle — `scale = 1.0 + 0.1*sin(frame)`
   ([d_a_player_main.cpp:1170](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)).
   A scale-pulse on `LEAF_MODEL` (open on entry from folded base, gentle breathing aloft), or a
   tiny hand-authored BCK on the 2-joint rig (like the bow flex we shipped).
4. **Activation cost.** Cucco glide is free; a Deku Leaf should drain a resource (magic / FA /
   stamina) while aloft, WW-style.
5. **Updraft content.** Reuse TP wind actors, or drop updraft volumes via the level editor
   (`mWindTgCyls` hit path already consumes them).

## 5. Billow curve (WW FAN_GLIDE)
Glide entry ([d_a_player_main.cpp:5750](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)):
`deleteEquipItem` (drop folded leaf) → `maxFallSpeed = mAutoJump.m.field_0x10` (the float clamp,
**same `mAutoJump` HIO as TP's cucco fall speed**) → `setSmallFanModel()` → `mEquipItem = DEKU_LEAF`.
The open/billow is a BCK/morf on `mpParachuteFanMorf` (`->play()`/`->calc()`), i.e. a joint
animation from the folded base pose. TP has no equivalent BCK → **synthesize** it: WW's own
firefly-bottle idiom `scale = 1.0 + 0.1*sin(37.699*frame/frameMax)`
([:1170](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)) is the pattern — a
one-shot open on entry then a low-amplitude sinusoidal breathe on the `LEAF_MODEL` joint scale.

## 6. Deku Leaf wind "attack" — the pushback (the hard part, but mostly solved)

**How it works, both games:** the gust is an **AT (attack) collision volume** registered into the
collision system with mass; enemies' Tg collisions receive the hit and each reacts in its own
`tgHitCallback` via `ChkAtType(...)`. There is **no universal "pushed by wind"** — reaction is
per-enemy, keyed on the AT bit. That is exactly why this is the hard part.

- **WW leaf:** `mFanWindCps` (R=70, grows to 120 on a swing) + `mFanWindSph`, fed
  `SetStartEnd`/`SetAtVec(gustDir)`/`SetR`, `Ccsp()->Set(...)` + `SetMass(1)`; during glide the
  sphere is pinned to Link's root joint ([d_a_player_main.cpp:10180](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)).

- **TP's real analogue is the GALE BOOMERANG, not the spinner.** `d_a_boomerang` builds
  `m_windAtCyl` — a vertical wind attack **cylinder** whose height grows along the arc
  ([d_a_boomerang.cpp:861](../src/d/actor/d_a_boomerang.cpp), src `l_windAtCylSrc` [:1406](../src/d/actor/d_a_boomerang.cpp)) —
  typed `AT_TYPE_BOOMERANG` (`1<<16`), `Ccsp()->Set` + `SetMass(1)`. **65 actors already react to
  it** (grep `AT_TYPE_BOOMERANG` in `src/d/actor/`): enemies blow away / scatter / bees disperse /
  bomb-carriers drop bombs / bosses stagger, plus flames blow out and light objects move. The
  spinner (`AT_TYPE_SPINNER`, `1<<19`) has only ~10 reactors — narrower; the gale is the base.

**Implementation options for the leaf gust (ranked):**
1. **Reuse `AT_TYPE_BOOMERANG`** — emit a wind cylinder/capsule from Link on flap, typed as the
   gale. Instant broad reaction, zero enemy-side code. Cost: a few enemies do boomerang-*specific*
   things (e.g. `d_a_e_bi` spawns a bomb-boomerang, [:156](../src/d/actor/d_a_e_bi.cpp)) that may read
   oddly under a leaf.
2. **New `AT_TYPE_WIND`** on a free bit (unused: `1<<6, 1<<8, 1<<11, 1<<12, 1<<15`) + copy the
   gale reaction branch into the target enemies. Faithful, but N-enemy edits = the real cost.
3. **Procedural sweep** (no AT/Tg): in the flap code, search actors in the gust cone and apply a
   direct velocity/knockback. Universal push without per-enemy code, but bypasses hitspark/reaction
   anims unless added.

**Recommendation:** prototype with **option 1** (gale type) — it's the same collision idiom as WW,
already broadly supported, and proves the feel; escalate to option 2 for enemies that misbehave.

**DECISION (locked):** **broad coverage, ZERO damage** (crowd-control tool, WW-faithful). Base =
`AT_TYPE_BOOMERANG` gust cylinder (57 reactors — widest), `atp=0`, wide `spl`, tuned R/H, `SetMass(1)`.
Reaction roster comparison (`ChkAtType` counts in `src/d/actor/`): BOOMERANG **57** · BOMB 37 ·
IRON_BALL 36 · WOLF_ATTACK 14 · SPINNER 9. `setCylAtParam(AtType, spl, hitMark, AtSe, atp, R, H)`
([d_a_alink_cut.inc:223](../src/d/actor/d_a_alink_cut.inc)) is the emit API — separates recognition
(AtType) / reaction-style (spl) / damage (atp), so all three are one-call tunable. Reactions are
authored **per-enemy** (each reads `ChkAtType`/`GetAtSpl`/`GetAtAtp`), so a dedicated
`AT_TYPE_WIND` (free bit `1<<6/8/11/12/15`) + a copied blow-back branch is reserved ONLY for the
few enemies whose native gale reaction reads wrong under a leaf.

## 8. Build plan / next steps (sequenced, each phase testable)

Prove the two NOVEL mechanics first (pure code), then layer the KNOWN-EASY asset work.

- **P0 — commit baseline.** The shipped WW bow/arrow/boot + warp-guard work is uncommitted; land it
  first so the glide feature starts clean (per commit-and-push.md; strip the ALBW-BOOT-DIAG probe).
- **P1 — glide re-gate (code, debug-toggled).** Add `checkDekuLeafGlide()` (true when the leaf is
  "out"); OR it into the `checkGrabGlide()` physics sites AND the `procAutoJumpInit` entry / fall→
  auto-jump transitions (entry currently keys on `grab_actor_name==NI`). Reuse `mAutoJump.mCucco*`
  (or add `mDekuLeaf*`) HIO. Milestone: activate → jump off a ledge → float/steer/updraft, no cucco.
- **P2 — gust attack (code, debug-toggled).** Emit an `AT_TYPE_BOOMERANG` cylinder from Link on the
  flap (via `setCylAtParam`, `atp=0`), `Ccsp()->Set`+`SetMass(1)`, forward/down gust vector, live
  R/H/spl knobs. Milestone: glide past enemies → broad stagger/scatter + flames out, zero damage.
  (P1+P2 together = the vertical slice that proves the feature, still using a placeholder held model.)
- **P3 — leaf model + billow (asset, proven pipeline).** Convert `vleaf` (itemmdl 0x15) BDL→BMD
  (bow/arrow recipe), attach overhead (WALKHBS pose / held-item mount), boots-style ambient (no
  MAJI), procedural sin-scale billow on `LEAF_MODEL` (§5). Milestone: the leaf reads correctly aloft.
- **P4 — activation cost + item UX.** Drain a meter (FA/stamina) while aloft; end on empty.
  **Exact WW economy is decompiled** — see §9. Wire how the leaf is pulled out (item slot/quick-swap).
- **P5 — updraft content.** Reuse TP wind actors, or drop updraft volumes via the level editor
  (`mWindTgCyls` hit path already consumes them).
- **P6 — polish + commit.** Leaf-flutter particles, `JA_SE_LK_FAN_WIND`-style sound, billow tuning;
  per-enemy `AT_TYPE_WIND` only where P2 misbehaves; strip toggles; commit.

## 7. Skull Hammer / 2-handed (parked, separate track)
Genuinely new animation/rig work (two-hand grip + swing arcs), unlike the glide which is a re-gate.
The WW player shows `dItemNo_SKULL_HAMMER_e` shares the `lHandA_jnt` mount with the leaf/sword
([d_a_player_main.cpp:1155](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp)) — starting
reference when that track opens.

*Related:* [Blender-WW-Items.md](Blender-WW-Items.md) (asset pipeline), the shipped bow flex
(2-joint BCK precedent), level-editor updraft-volume idea.

## 9. Deku Leaf magic economy — EXACT (decompiled)

Source: `zeldaret/tww` `src/d/actor/d_a_player_fan.inc` (the `FAN_GLIDE` proc body — an `.inc`,
which is why it isn't in `d_a_player_main.cpp`). Meter interval field = `mItem.mFan.m.field_0x8` = **40**.

- **Entry gate** (`checkFanGlideProc`): press Deku Leaf trigger → `if (getMagic() >= 1)
  procFanGlide_init(); else JA_SE_ITEM_TARGET_OUT` (fail buzz). Can't start on empty.
- **Open cost** (`procFanGlide_init`): `m3574 = field_0x8` (=40), then `if (!checkHeavyStateOn())
  setItemMagicCount(-1)` — **1 magic consumed on open** (skipped in heavy/iron-boots state),
  `setPlayerStatus1(DEKU_LEAF_FLY)`, SE `FAN_CHUTE_OPEN`.
- **Continuous drain** (`procFanGlide`): each frame `m3574--`; when `m3574==0 && getMagic()>=1`
  → `setItemMagicCount(-1)`, `m3574 = 40`. ⇒ **1 magic per 40 updates.** WW runs game logic at
  **30 updates/s** ⇒ **1 magic / 1⅓ s = 0.75 magic/s.**
- **Terminate**: cancel input, OR `getMagic() < 1 && m3574 == 0` → SE `FAN_CHUTE_CLOSE`, fall.
- **Visual tell**: leaf material shows while `getMagic()>=1`, **hides at 0** (leaf wilts/vanishes
  when out of juice) — nice cue to replicate.
- WW meter = 16 base / 32 upgraded ⇒ **~20 s glide** on base (16/0.75 − open cost), **~42 s** upgraded.

**Translate to Dusklight (FA/stamina meter):** keep WW's model — a frame counter draining 1 unit
per interval — but **scale the interval to our tick rate.** WW = 40 ticks @ 30 Hz; if Dusklight's
`execute` runs at 60 Hz, use **80 ticks** to keep 0.75 units/s. Target feel = **~20 s per full
meter** (base-magic parity); derive `interval = meterMax_units ⁻¹ · targetSeconds · tickHz`.
Mirror the three beats: 1-unit open cost (skip if heavy boots), continuous per-interval drain,
drop + close-SE + leaf-hide when empty. Gate entry on `meter >= 1` with a fail buzz, like the
`getMagic()` template at [d_a_player_main.cpp:3766](https://github.com/zeldaret/tww/blob/main/src/d/actor/d_a_player_main.cpp).

## 10. Gust attack — EXACT (decompiled, `d_a_player_fan.inc`)

WW's leaf gust is a distinct action from the glide: `procFanSwing` (ground/air flap), separate
from `procFanGlide` (float). The emit, verbatim:
```
setAtParam(AT_TYPE_FAN_SWING, 1, dCcG_At_Spl_UNK0, dCcG_SE_WOOD, dCcG_AtHitMark_Nrm_e,
           CUT_TYPE_NONE, 50.0f);
```
- **`CUT_TYPE_NONE`** ⇒ **zero damage** — WW's own leaf gust is non-lethal crowd-control. Validates
  our "no damage" decision at the source.
- **`dCcG_At_Spl_UNK0`** ⇒ the **mildest** special-reaction flag (a gentle shove, not a hard knock).
  For TP, use the mild `spl`, not a hard one.
- dedicated **`AT_TYPE_FAN_SWING`** type + `dCcG_SE_WOOD` hit sound + normal hit-mark.
- **Geometry** (`procFanSwing`, ~258-280): capsule `mFanWindCps` from `current.pos` up ~70;
  forward reach ~**100** units along `shape_angle.y` (ground-checked); collision **R≈50**; **aims at
  the lock-on target** if any (`m3538 = atan2(target)`). Particles `ID_AK_JN_UCHIWAWIND00` +
  `ID_AK_JN_LEAFFAN00`, SE `JA_SE_LK_FAN_SWING`.

**TP translation:** TP has no `AT_TYPE_FAN_SWING`, so reuse **`AT_TYPE_BOOMERANG`** (57 reactors)
via `setCylAtParam(AT_TYPE_BOOMERANG, <mild spl>, hitMark, AtSe, atp=0, R≈50, H≈70)`, emitted from
Link forward (~100) / at lock-on target, `Ccsp()->Set` + `SetMass(1)`. CUT_TYPE_NONE ≡ our `atp=0`.
Two player actions to add: **leaf glide** (float, §1 chassis) + **leaf swing** (the gust).

## 11. P1 test notes (2026-07, in-game)
- **Arm pose fix (done):** leaf glide now sets the `CARRYD` overhead-carry base pose (a grabbed
  cucco sets it; the leaf holds no actor) before `WALKHBS` layers, so the upper body matches
  cucco+glide. [d_a_alink.cpp ~17882].
- **Later:** auto-engage the glide on *any* drop (height delta with ground below), not only the
  ledge-run-off entry — needs a fall→glide entry hook when the leaf is out.
- **Later (conflict):** collides with the dive animation when **water is below** — the auto-jump/
  glide vs `procDiveJump` selection needs a water-below guard so diving still wins over gliding.

## 12. Build order revised (2026-07) — P3 before P2
Rationale: can't meaningfully test the gust without the leaf visible in-hand. New order:
**P1 (done) → P1.5 (entry polish) → P3 (model+billow) → P2 (gust) → P4 (cost, last)** → P5/P6.

### P3 — leaf model + billow (next). vleaf = bdl4+MDL3, 2 joints (Vleaf/leaf_model), mats
### Vleaf+leaf, tex V_leaf+ZAtoon. NO existing TP item slot to override → load standalone.
- **P3a Convert** `vleaf` (itemmdl 0x15) BDL→DAE→**BMD** with `-m`/`-x` (bow/arrow recipe; strips
  MDL3 so Aurora renders it natively). Verify 2 joints, mats, textures embedded. Deploy loose as
  `itemmdl_21.bmd` in "Wind Waker Skins".
- **P3b Load + hold** on glide-engage: `custom_assets::try_load("itemmdl", 0x15)` → build a
  `J3DModel` (file-static `s_dekuLeafModel`, like `s_albwWwBowNative` — no class-member add).
  Attach overhead each frame: `setBaseTRMtx` from a head/hand joint + the cucco overhead offset
  (mirror `setGrabItemPos`). Destroy/hide on let-go/land.
- **P3c Draw boots-style ambient** (no MAJI): `settingTevStruct(0)` + `setWwBowActorAmbient` +
  `applyBowMaterialAmbientOnly`, `modelEntryDL`. NOTE: `applyBowMaterialAmbientOnly` exact-matches
  `"SC_Vbow_v"` for the ink branch; leaf mats are `Vleaf`/`leaf` (no SC) → all get body-ambient
  (fine, matte green). Generalize the SC check to a `SC_` *prefix* if any WW item later needs it.
- **P3d Billow (procedural)**: sin-scale the `leaf_model` joint (idx 1) — one-shot open on engage
  (folded base → full), then `scale = 1.0 + amp*sin(rate*t)` breathe aloft (WW firefly idiom).
  Tunable amp/rate.
- **P3e Lifecycle**: leaf appears on A-engage, tracks Link through the glide, vanishes on let-go/
  land. Reserve the leaf-hide-on-empty cue for P4.
- **P3f Build + test**: renders overhead, matte (no bloom), billows, tracks Link.
- **Open decision:** exact mount point/offset (overhead like the cucco, or one-hand parasol like WW)
  — a tuning pass once it's on-screen.

## 13. DEFERRED — mod-manager API request (container with multi-select promotion)
Desired (distinct from current behavior): a **container folder holding N sub-mods/variants** where
the user **multi-selects which ones are promoted to the left/load-order pane** — each independently
enable/disable-able and orderable. NOT the current collection model (single mutually-exclusive
variant picker), and NOT N separate top-level mods. Use case: bundle the WW skins (boots/bow/arrow/
leaf) under one "Wind Waker Skins" container, pick which are active. Owner: load-order/mods chat
(custom_assets.cpp disk_entries/collection logic + mods.cpp UI). For now the Deku Leaf ships as its
own top-level mod ("Wind Waker Deku Leaf") — works, just not grouped.

---

## §14. WW Deku Leaf HOLD mechanics (from zeldaret/tww decomp, researched 2026-07-18)

Source: `src/d/actor/d_a_player_fan.inc` (included into `d_a_player_main.cpp`), `d_a_player_main_data.inc`, `include/d/actor/d_a_player_main.h`. Glide (parachute) is a SEPARATE state from the swing (attack).

1. **Body animation:** `ANM_USEFANB` (0xA2). Set in `procFanGlide_init()` via `setSingleMoveAnime(ANM_USEFANB, ...)`. Table entry (`d_a_player_main_data.inc`): under+upper bck both = `dRes_INDEX_LKANM_BCK_USEFANB_e` (`usefanb.bck`), hands = `HANDS_JNT_CL_LHANDD_e`/`CL_RHANDD_e` (closed grip "D" hand models, BOTH hands). Fully authored two-handed overhead grip — no procedural arm posing.

2. **Leaf attachment:** glide canopy = separate `J3DModel` (`mpParachuteFanMorf`/`mpEquipItemModel`), model `dRes_INDEX_LINK_BDL_FANB_e`, bck `FANBA`. Attached in `setItemModel()` for `daPyItem_UNK102_e`: `mpEquipItemModel->setBaseTRMtx(mpCLModel->getAnmMtx(CL_JNT_CHEST_JNT_e))` — anchored to the CHEST joint, NO offset, NO scale. (Contrast: the SWING fan attaches to left hand `CL_LHANDA`.)

3. **Hands<->leaf:** NO IK, NO coupling. Hands placed by bck; leaf placed by chest matrix. They align ONLY because `usefanb.bck` was authored to seat the hands on the canopy's grip arms. Leaf follows the body (chest); hands follow the body anim; neither follows the other.

4. **Billow:** `parachuteJointCB()` — `FANB_JNT_LARMB/RARMB` (ribs): `transS(m3600,0,0)`; `FANB_JNT_LROOT/RROOT` (grip roots where hands sit): `YrotS(m355E)`. Drive (end of `procFanGlide`): `m3600 = (1.5 + 0.75*x + rnd(0.3))*ssin(m355C)`; `m355E = (600 + rnd(100))*ssin(m355C)`; `m355C += (6000+..)+(12000+..)*rnd()`. Grip roots barely move (small yaw); ribs flutter more. Hands stay put under the authored pose.

5. **Scale:** none (native).

### Implication for TP recreation
WW hands don't clip because `usefanb.bck` was custom-authored to match the chest-anchored canopy grips. We use the CUCCO-HOLD pose instead, so the cucco hands won't naturally land on our leaf's grips regardless of anchor. Two routes:
- **Route A (WW-structural):** anchor leaf to TP chest joint, no offset, keep working orientation rotations. Lowers it naturally; but cucco hands not authored to grips -> may still clip.
- **Route B (adapt-to-pose, RECOMMENDED):** measure TP L/R hand joint world positions in the cucco-glide pose; position (and maybe modestly scale) the leaf so its two grip-root arms meet the hands. Possibly anchor between the two hands rather than the head.

### CONFIRMED-WORKING orientation (do not lose)
Leaf overhead orientation locked (playtested): head-anchor, `s_dekuLeafOffset(0,55,0)`, `s_dekuLeafBaseRotY=+0x4000`, `s_dekuLeafBaseRotX=-0x4000`, applied order Y then X after yaw. Z axis tried and removed. Offset still 55 (a touch high); lowering gated on the hand-hold fix above.

---

## §15. Product lock + aerial bomb-drop feasibility (2026-07-23) — research only

### Product lock (confirmed)

| Piece | Behavior |
|-------|----------|
| **Parachute float** | Stays — current cucco-chassis leaf glide (`checkDekuLeafGlide` → `checkGrabGlide`) |
| **Gust jump** | **Takeoff only** — R+A launches human Link (then float); not a continuous rise |
| **Activation** | **R+A**, human Link only |
| **Wolf** | Keeps **simple moon jump** (`f_ap_game.cpp` ~811: R+A → `speed.y = 56`) — no leaf |
| **Mid-air A** | Current engage/disengage toggles stay |
| **NEW ask** | While airborne **and leaf-gliding**, drop bombs on enemies below |

### Verdict — aerial bomb drop while leaf-gliding

**Feasible — with a dedicated glide-drop path.** Stock “pull bomb → grab → throw proc” is **not** viable if Link must **keep floating**.

WW stock does **not** support leaf-then-bomb mid-glide (speedrun tech is bomb-held → jump → leaf). This is a Dusklight extension, not a WW parity port.

### Why stock bomb use fails aloft

| Blocker | Evidence | Effect |
|---------|----------|--------|
| **A is already leaf toggle + LET_GO** | `procAutoJump`: A toggles `s_dekuLeafGlideActive` (~18132); later `field_0x300c` sets `BUTTON_STATUS_LET_GO` and A → `freeGrabItem()` (~18245) | Same press cannot mean “drop bomb, stay gliding” and “let go of leaf” |
| **Throw leaves AUTO_JUMP** | `procGrabThrowInit` → `commonProcInit(PROC_GRAB_THROW)` (~874) | Ends glide chassis for a ground throw anim |
| **WALKHBS re-assert** | While `checkDekuLeafGlide()`, every frame forces `WALKHBS` if missing (~18169) | Clobbers bomb grab upper anime |
| **Hands / overhead slot** | Leaf welded overhead; bomb uses `mGrabItemAcKeep` + `setGrabItemPos` | Visual + hold-slot fight if you try to carry a lit bomb under the canopy |
| **Grab-anime free** | `!checkGrabAnime() && grab actor ≠ NULL` → `freeGrabItem()` (~16475) | Bomb can vanish next frame if upper anime is leaf pose, not grab |
| **Mode bit 4** | Cucco at jump init clears bit 4 (`offModeFlg(4)` ~18088); leaf mid-air engage does **not** | Item-button equip may work while leaf-gliding today, but still hits the grab/pose/A conflicts above |

`checkUpperItemActionFly()` only handles bow / boomerang / copy-rod — **not bombs**. `procAutoJump` already calls `checkItemChangeFromButton()` (~18315), so face-button *equip* is closer than throw — still not a clean drop.

### Recommended path (low risk)

**Instant spawn-and-drop on bomb item button while `checkDekuLeafGlide()`**, stay in `PROC_AUTO_JUMP`:

1. Gate: leaf gliding + bomb on assigned face button + trigger + `dMeter2_canALBWBomb()` (existing).
2. `dBomb_c::createNormalBombPlayer` / water variant at Link pos (or slightly below feet).
3. Give downward (optional slight forward) velocity — do **not** `setGrabItemActor`, do **not** enter `PROC_GRAB_THROW`.
4. Charge meter via existing `dMeter2_onALBWBomb()`; leave leaf meter drain alone.
5. Keep A = leaf toggle only (do not route drop through LET_GO).

Optional polish: short arm flick / SE; cooldown so spam doesn’t feel broken; refuse when meter can’t pay bomb **and** leaf open cost in the same moment (product call).

### Rejected / high-cost alternatives

- **Hold bomb then leaf (WW speedrun order):** possible-ish if leaf engage doesn’t steal grab, but opposite of the ask and still fights canopy vs overhead bomb.
- **Full grab-throw while gliding:** needs exemptions for WALKHBS, A conflict, and a throw that doesn’t leave `PROC_AUTO_JUMP` — multi-site, easy to break cucco glide.
- **New `AT_TYPE` / fake bomb:** unnecessary; real `NBOMB` actors already explode and hit enemies.

### Scope note vs takeoff work

Aerial bomb drop is **orthogonal** to R+A gust-takeoff → float. Implement takeoff/float first; bomb-drop is a small `procAutoJump` (or item-button) hook once glide is stable. **Not implemented yet.**
