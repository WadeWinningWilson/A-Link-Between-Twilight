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
