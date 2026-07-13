# Hero's Shade Secret Boss

**Status:** Research / feasibility — **NO code written yet**
**Scope:** A new post-game secret boss: after all Hidden Skills are learned, a final Hero's Wolf Shade appears, warps the player to the Hero's Shade arena, and this time you fight the Hero's Shade as a *real* boss (full health bar + victory condition) instead of a scripted lesson.
**Related:** [project-stargazer-mod (Shade Watcher)](../memory/project_stargazer_mod.md-equivalent), [boss-fights-handoff.md](boss-fights-handoff.md), [albw-boss-hp-hud-tuning-brief.md](albw-boss-hp-hud-tuning-brief.md), [suppress-enemy-spawns]
**Caveat:** All line numbers are point-in-time and WILL drift — re-verify against current source before editing.

---

## The idea

After the player has learned all 7 Hidden Skills (secret techniques), one last golden Hero's Wolf Shade appears in the world. Touching it warps the player to the Hero's Shade spirit arena one more time — but instead of another teaching duel, you fight the Hero's Shade properly: he has a real boss health bar, attacks aggressively, and can be defeated (victory condition + reward/flag).

This is the ONE feature that intentionally *extends* the vanilla Hidden-Skills questline, in contrast to the sibling Shade Watcher / Stargazer system which deliberately **clones** the gold wolf and avoids inheriting the questline.

---

## Feasibility verdict (high level)

**Feasible, and the integration seams are unusually clean.** The "framing" (detect skills → spawn final wolf → warp to arena → pick a duel behavior) is ~80% reuse of code that already exists or is already scoped. The cost concentrates almost entirely in **one place**: authoring a combat AI + defeat sequence for a new behavior mode of an actor that was *built to accept new modes*. There is no architectural fight.

| Beat | Concrete hook | Effort |
|---|---|---|
| Detect all skills learned | `F_0344` event bit ("Obtained 7 secret techniques") | trivial |
| Spawn final wolf | gold-wolf clone runtime spawn (Stargazer pattern, `d_s_room.cpp`), gated on `F_0344` | low |
| Warp to arena | wolf's own `dStage_changeScene` → SCLS exit → `F_SP200` | low |
| Select duel behavior | spawn `d_a_npc_kn` with param → `mType == 7` (existing free slot) | low |
| Gate the duel Shade | extend `isDelete()` with `F_0344` appear / new "duel cleared" del flag | low |
| **Deal damage to Shade** | add `mLife`; decrement in `tgHitCallBack`, gated on `mType == 7` | moderate |
| **Health bar UI** | `dComIfGs_BossLife_public_Set/Get` (same API Fyrus uses) | low–moderate |
| **Combat AI + defeat seq** | new `mType == 7` selector driving the existing move-methods | **the real work** |
| Don't leak into mod systems | classify duel-Shade in `dAlbwHP` `sBoss`/`sFinalBoss` | low |

---

## Code map (verified this session)

### 1. Detection — all skills learned
- `F_0344` = `0x2a20`, "Secret techniques — Obtained 7 secret techniques" — [include/d/d_save_bit_labels.inc:241]. This single event bit IS the gate. (`F_0338`–`F_0344` = skills 1–7 learned.)
- Do NOT confuse with: `F_0472`–`F_0477` = *distant howling* summons; `F_0492`–`F_0497` = gold-wolf *disappearance* flags (the per-session consume pattern, a model for a "duel cleared" flag).

### 2. The final wolf (appear + warp)
- Source to clone: `src/d/actor/d_a_npc_gwolf.cpp` (`daNpc_GWolf_c`).
- Vanilla wolf self-gates appearance on the howling flags via `l_warpAppearFlag[7]` (~:184); warps with `dStage_changeScene(l_warpIdList[mParamMode-1], ...)` (~:1629). Exit IDs 8/9/A/B/C resolve through the **current overworld stage's SCLS exit table** to the spirit arena.
- Runtime-spawn template (gated): the postman block in `src/d/d_s_room.cpp:252-263` (`#if TARGET_PC`, `fopAcM_create(...)` after a stage/room/event-bit gate). This is the established mod pattern for placing a new gated actor.

### 3b. Asset/distribution model + runtime collision (researched 2026-07-02)
- **Dusklight ships as CODE (the recompiled `dusklight.exe`).** ALL game data — stages, room layouts, models, collision — loads from the **player's own game ISO/DOL/RELs at runtime** (`src/dusk/dvd_asset.cpp`: `DVDGetDOLLocation`, `JKRDvdRipper::loadToMainRAM`, RELS archive; version-keyed by offset). There is **no asset-override / bundling mechanism** in the mod. ⇒ **Editing a stage file does NOT reach players.** To ship stage edits you'd need to either (a) bundle modified archives + add an override loader (doesn't exist), or (b) patch stage data in code at runtime. This is exactly why everything so far is runtime actor spawns.
- **`setNextStage(stage, point, roomNo, layer)`** ([d_com_inf_game.h:1212]). Our Milestone-B warp used `("F_SP200", 0, 0, -1)` → layer **-1 (default)** = the howling room. The **vanilla duel arena is F_SP200 layer 7, room 0** (confirmed in-game). Aiming at it = change the layer arg to `7` (one line).
- **Runtime walkable collision IS possible without stage edits — the `MoveBG` object system.** Floor/platform object actors (`d_a_obj_lv4floor`, `d_a_obj_lv4sand`, `d_a_obj_mirror_sand`, `d_a_obj_vground`, …) self-load their own arc (`dComIfG_resLoad`) and register a walkable collision surface via **`MoveBGCreate(arcName, …)`** ([d_a_obj_lv4floor.cpp:52]) — the arc carries a model + DZB collision. These are spawnable. ⇒ Spawning such a floor object in F_SP200 to ADD walkable arena floor is feasible at runtime (the "Morpheel sand as POC" path), no stage-data editing. Caveats: visual mismatch (sand vs misty floor), manual placement, and some carry stage-specific dynamics (rotation/rising/switches) to avoid.

### 3c. Invisible walls = baked DZB (researched 2026-07-02)
- Walls in TP are **DZB collision polys tagged with a "wall code"** (`dBgS::GetWallCode`, `dBgS_CheckBWallPoly`, `ChkWallOff` — [d_bg_s.h:204/263]). Same baked stage data as the floor, just a wall-attributed poly. The Hero's Shade arena's containment is NOT actor-driven (no containment code in `d_a_npc_kn`; no return/keep-in tag) — it's the stage collision.
- ⇒ **A bigger MoveBG floor is pointless if the boundary is a wall:** the original baked wall still blocks Link at the old radius, and it can't be removed at runtime / wouldn't ship (same asset constraint as §3b). The only runtime wall-off is a global DEBUG HIO flag (`FLAG_WALL_OFF_e`), not a shippable per-arena tool.
- **Open question (testable in-game):** is F_SP200's boundary a WALL (Link stops dead) or an EDGE/fall-off (Link would fall)? If it's a fall-off edge, a MoveBG floor DOES extend walkable space; if it's a wall, it does not. Walk to the edge to tell.
- **CORRECTION on the "wall-off" debug flag (2026-07-02):** `FLAG_WALL_OFF_e` / `ChkWallOff()` is **viewer-only** — it just hides wall polys in the debug collision overlay draw routine (`poly_draw`, [d_bg_s.cpp:651]); it has ZERO effect on actual collision. My earlier note calling it a "wall-off" was wrong.
- **The REAL wall-collision disable** is `ChkAcchWallOff()` → early-return from `dBgS::WallCorrect()` / `WallCorrectSort()` ([d_bg_s.cpp:1173/1217]) — the routines that push actors out of walls. BUT it is **`#if DEBUG`-only** (compiled out of shipped RelWithDebInfo builds) AND **global** (all actors/all walls, not per-room). So the built-in flags cannot give a shippable per-room wall-off.
- **Custom per-room wall-off IS buildable but invasive:** add a `#if TARGET_PC` gate in `WallCorrect` skipping STAGE-wall correction for Link when in the arena room + toggle. `WallCorrect` runs for every actor every frame (hot path). Side effects: Link no-clips ALL stage walls in that room (boundary + interior + wall-type actors), and ground correction still applies so he'd fall off ground edges → must pair with a bigger MoveBG **ground** floor. There's also an existing per-Acch `MoveBGOnly` flag ([d_bg_s.cpp:1197], collide only with MoveBG objects, ignore baked stage) — cleaner in spirit but still requires setting it on Link's collision (in `d_a_alink`) and providing MoveBG floor. Both are core-collision surgery with real jank risk; the clean path remains hosting on a stage with a naturally large native arena.
- **Strategic conclusion:** for a genuinely larger fightable space, the realistic path is to **host the fight in a different stage that already has a large open native collision area** (avoids floor + walls + clouds all at once), rather than hacking F_SP200's baked boundary. Tradeoff: lose the authentic misty-arena look.

### Combat AI progress (2026-07-03)
`heroShadeCombat` (mType-7 action, [d_a_npc_kn.cpp]) tuning, all motion-driven (no events):
- Approach: charge at `speedF 14.0f` (7x the 2.0f lesson `move_speed`) when Link is >250u; `calcSwordAttackMove(1)` handles facing + the swing trigger (motion `0x19`).
- Damage: sword `At` `SetAtAtp(12)` for mType 7 in `resetCol` (vanilla default is atp 0 = harmless); ≈3 hearts on the quarter-heart scale — confirmed good in playtest.
- Late tracking: keep re-aiming through the swing until frame 36 (hit window 30-40), lock the final ~10%. Confirmed reliable/landing.
- Intro delay: mMode 2 holds ~180 frames (~3s) before engaging so Link's spawn animation finishes.
- Re-attack loop FIX: `0x19` persists after a swing, so re-arming the timer every frame kept it from re-expiring (he stopped after 1-2 swings). Now: on swing completion (`getFrame() >= 40`) return to ready (motion 9) + re-arm timer ONCE.
- Point-blank: if Link crowds within 120u and not mid-swing, clamp the timer to swing soon.
- **Menu-crash fix (unrelated pre-existing bug):** opening the Collection/status menu crashed in `daAlink_c::changeModelDataDirect` ([d_a_alink_swindow.inc:352]) — hardcoded hat-model joints 6/7 are null for some modded-outfit models. Added a null-guard. NOT the Shade feature; cross-refs the sumo status-window crash in [[project-sumo-outfit]].
- Guard / shield-bash / STUN loop (2026-07-03): normal state = body Tg shielded (`setCollision` case 7 `OnTgShield`) so Link's attacks are blocked; a shield BASH (`ChkTgShieldHit` + `CUT_TYPE_GUARD_ATTACK`) breaks the guard → mMode 4 stun (motion 0xb head-down + `onHeadLockFlg`, ~180-frame ≈ 3s), during which the Tg shield drops (`OffTgShield`) so ALL attacks land (Helm Splitter/hidden skills/basic). Recovers to mMode 3 after the timer. Mirrors the vanilla Helm-Splitter lesson (teach04 shield-bash → headBreak).
- Blade `At` radius: +5% for mType 7 (`SetR 31.5f`).
- **Stun-loop refinement (2026-07-03, build 25):** (a) stun shortened 180→**120 frames (~2s)**; (b) **stun hit mark = `MARK_1`** (build 27, CORRECTED — briefly tried `MARK_2` in build 25, which was WRONG). **Vanilla reference `setCollisionTeach04`:** case 9 (guard) = `MARK_2` + `OnTgShield`; case 10/11 (stun/hittable) = **`MARK_1` + `OffTgShield`**. `MARK_2` is the SHIELD/metal material (the clank); `MARK_1` is the vulnerable-body/flesh material. Using `MARK_2` during stun sounded like a shield block even with the shield down. Vanilla stun-hit also plays `Z2SE_HIT_SWORD` at volume `0x1e` (matched). Vanilla only reacts to `CUT_TYPE_HEAD_JUMP` (Helm Splitter) in that state — the boss accepts ALL attacks via `ChkTgHit`; (c) mMode-4 reacts on **`ChkTgHit() || ChkTgShieldHit()`** with damage voice/SE. **NOTE (build 26):** initially tried motion **0x1b** as a flinch — WRONG: 0x1b is part of his guard/attack set (bundled with swing motions 0x12/0xe/0x1e in the teach combos) and visually snaps the shield UP, which re-reads as a block during stun. Removed it — he now **stays slumped in the 0xb stun pose** while taking hits (shield stays down in collision via `mMode==4` case 7, so hits genuinely land). **Hit-reaction (build 28):** identified — mMode 4 now plays vanilla's recoil animations **`0x12`** (hit from front, `rel < 0x4000`) / **`0xe`** (hit from behind) with knockback speed ZEROED, so he reels IN PLACE. NOT 0x1b (that's guard/shield-raise).
- **Vanilla hit-reaction catalog (all teach lessons follow ONE pattern):** the lesson's *correct* skill → `MARK_1` + `OffTgShield` → recoil `0x12` (front) / `0xe` or `14` (behind) **+ `speedF = ±attack_disappear_speed_h` + `speed.y = attack_disappear_speed_v` + `DAMAGE_L` voice + `HIT_SWORD`** = the FLY-BACK-AND-DISAPPEAR (spectral vanish). *Any other* attack (basic sword) → `MARK_2` + `OnTgShield` → motion `0x1b` (shield-block) + `HIT_SHIELD_ATTACK`/`HIT_SWORD` = he GUARDS it. Per-lesson correct skill: teach02/03=`TWIRL` (Back Slice), teach04=`HEAD_JUMP` (Helm Splitter → headBreak/splitter mActionMode 11), teach05=`MORTAL_DRAW_A/B`, teach06=`LARGE_JUMP*` (Great Spin/jump), teach07=`LARGE_TURN_L/R`. **The fly-back (`0x12`/`0xe` + speed + vanish) is the finishing-blow reaction → reserved for 1 HP.**
- **Grounded stagger = motion `0x1E`** (build 29). `0x12`/`0xe` with speed zeroed looked awkward (frozen mid-lunge) — WRONG approach. Correct: use a *different* animation. Motion `0x1E` is vanilla's `LARGE_JUMP_INIT` reaction (teach06/07) — a reel-in-place with NO `speedF`/`speed.y` (only a tiny `field_0x15bc` slide, which we omit). **Motion→anim decode:** `l_motionSequenceData[132]` groups 4 steps per motion (motion M step S = table index M*4+S); each step's animIdx → `l_motionAnmData[245]` (7 ints/anim: `{bckFileIdx, resArcIdx, loop,...}`). `resArcIdx` 0 = base/`KN_a`, 2 = `KN_tch01D` — BOTH loaded for type 7 (`l_loadResPtrn0={1,2,-1}`). `0x1E`→anim7(bck8,res0)+anim2(bck39,res2), both valid for type 7 (bck8 is also the stun-pose start; bck39 is the idle). Key motions: 9=idle(anim2), 0xb=stun/head-down(anim7,8), 0x19=swing(anim12/bck30), 0x1b=guard(anim5/bck11), 0x12=recoil-front(anim13/bck17,res2), 0xe=recoil-back(anim23/bck18,res2), 0x1E=grounded stagger. NOTE: `0x1E` returns to idle after the reel, so between combo hits he may stand rather than stay slumped — ambiguous with guard.
- **Stun hit-reaction FINAL = Option B (build 30, 2026-07-03, USER-APPROVED "perfect"):** on each hit in mMode 4, **re-trigger motion `0xb` from frame 0**. `0xb` is two anims — `bck8` (head snaps down) → `bck9` (sustained head-down hold) — so replaying it shows the head-snap flinch AND settles back into the same slump. One motion = both the reaction and the between-hit "downed/vulnerable" pose (mirrors vanilla `teach04_finishWait`, which holds `0xb` on a loop). He never drifts to idle/guard → unambiguous "stunned" read. No timer, no `0x1E`. Options compared: **A** = `0x1E` reel + `field_0x15be` reel-hold timer snapping back to `0xb` (keeps the more expressive reel; drafted, not shipped — available if a bigger reaction is wanted later); **C** = hold `0xb`, SE only (rejected earlier as "no response"). `0x12`/`0xe` stay reserved for the 1-HP finisher fly-back.
- **Multi-hit handling — vanilla does NOT sustain combos (finding 2026-07-03):** every vanilla vulnerable window is engineered for ONE decisive hit. On the qualifying hit the Shade sets `mTargetPos` (short slide vector) + **`field_0x15bc = 1`** + `ClrTgHit()` + **transitions out** (`mActionMode` change or `mEvtNo` event/teleport-vanish) and `break`s that frame. `field_0x15bc` then drives **`calcSlip()` (~:5305)**: slides him toward `mTargetPos` at rate 0.25, self-clears at `absXZ < 5` or wall-hit — this is vanilla's knockback (a controlled slide, NOT persistent `speedF`), and it gates `calcSwordAttackMove` off while reacting. Rapid successive hits never happen to a vulnerable vanilla Shade. **Our 2s multi-hit stun is an extension:** we `ClrTgHit()` each hit AND stay in mMode 4 (no transition), so each new swing re-registers as a fresh `ChkTgHit` → another reaction. `calcSlip` + `field_0x15bc` is the clean tool for the finisher knockback later (set `mTargetPos`, let it slide) instead of raw `speedF`.
- **Attack timing (current wiring, for tuning):** swing = motion `0x19` (bck30). Blade `At` active **frames 30–40** (`setCollisionSword`), we cut back to ready at `getFrame()>=40`. Inter-attack wait = `field_0x15d0` = **60 frames** normal, **forced 20** point-blank (`kn_dist<120`). So point-blank a hit lands ~30 frames after swing start (~0.5s @60fps logic tick); cadence ≈ 20 wait + 30 wind-up = ~50 frames between hits (~0.83s); normal (>120u) ≈ 60+30 = ~90 frames (~1.5s). Charge speedF=14 when `kn_dist>250`. No separate telegraph — the 30-frame wind-up IS the tell. Raw bck30 length past frame 40 not yet measured (we truncate to ready at 40).
- **Motion `0x19` (swing) — attack-timing tuning OPTIONS (analyzed 2026-07-03; cadence at ~60/s logic tick).** Playback rate is settable via `mpModelMorf[0]->setPlaySpeed(f32)` (McaMorfSO, `mFrameCtrl.setRate`; default 1.0). NOTE: `setNo`'s 2nd param is the morf/blend time, NOT speed. "Quickened 20%" = rate **1.2** (≈16.7% less time); literal 20%-less-duration = rate 1.25. **Baseline:** wind-up-to-hit 30f, strike→ready 10f, wait (`field_0x15d0`) 60f normal / 20f point-blank; hit-to-hit **100f normal / 60f PB**.
  - **Option 1 — whole swing 1.2×** (setPlaySpeed 1.2 frames 0–40, reset 1.0 on exit): wind-up 25f, strike+recovery 8.3f, active blade window 8.3f (**17% shorter**). Hit-to-hit **PB 53f / normal 93f** (~11%/7% faster). Strike visibly faster.
  - **Option 2 — wind-up ONLY 1.2× [SELECTED]** (setPlaySpeed 1.2 at swing start → 1.0 at `getFrame()>=30`): wind-up 25f, strike+recovery 10f (unchanged), active window 10f **preserved**. Hit-to-hit **PB 55f / normal 95f** (~8%/5% faster). Strike reads normal — only the commitment is quicker.
  - **Wait −15%** (`field_0x15d0` 60→51 / 20→17): trims −9f normal / −3f PB per cycle.
  - **Combined Option 2 + wait −15% [BUILT build 31, 2026-07-03]:** ready→hit 42f PB / 76f normal; hit-to-hit **52f PB / 86f normal**. Impl: top-of-`heroShadeCombat` resets `setPlaySpeed(1.0)` every frame; mMode 3 bumps `1.2` while `getNo()==0x19 && getFrame()<30`. Wait cuts: all `cLib_getRndValue(60,60)`→`(51,51)` (mMode 2 first-attack, mMode 3 swing re-arm, mMode 4 recovery), point-blank `20`→`17`. Intro delay (180f) left as-is. Stun (`field_0xdec`=120/2s) unaffected — separate timer, and rate reset covers `0xb`.
  - **CRITICAL:** `setPlaySpeed` sets a PERSISTENT rate on `mpModelMorf[0]`, so it must be reset to 1.0 whenever NOT in the wind-up (strike, return-to-ready, and defensively on stun/guard/idle entry) or those animations (incl. the `0xb` stun) would also play at 1.2×.
- **3rd blade sphere (2026-07-03, build 25):** `mSphCc[2]→[3]` in the header (STATIC_ASSERT 0x1758→**0x1890**, verified by compiler); `l_swordOffset[3] = {60,120,0}` with loop count `nSword = mType==7 ? 3 : 2`. The 3rd sphere sits at the **hilt/base (offset 0)** to catch point-blank attacks under the two blade spheres. Vanilla types unaffected (loop still runs 2, `mSphCc[2]` inert). All 3 share the loop `SetR`, so future radius tuning applies to all together. Base offset tunable via the debug collider view.
- **Debug: view attack spheres** — ImGui dev menu → Tools → Collision view → "Enable Attack Collider view" / "Target Collider view" / "Push Collider view" (`collisionView.enableAtView/TgView/CoView` in [ImGuiMenuTools.cpp:98]).
- **Great Spin wake-up punish (build 32, 2026-07-03):** the Shade has NO gameplay spin in his AI — the Great Spin is Link's move performed ON him (teach07 `CUT_TYPE_LARGE_TURN_L/R` → he reacts/fly-back). But he DEMONSTRATES it in the "ougi training" (`Z2BGM_OUGI_TRAINING`, `ECut_thirdSkillExplain`) via motions 0x11/0x18/**0x1c**, all in `KN_a` (res0 → loaded for type 7). Wired: **new mMode 5** — on stun recovery (mMode 4, `field_0xdec`==0), if `kn_dist < 100` (kSpinRange, "glued to him") → play **motion 0x18** (best-guess spin index) + `Z2SE_KN_V_ATTACK` + `field_0xdec`=60 safety cap; else normal recover to mMode 3. mMode 5 faces Link, exits when `getStepNo()>=1` (spin anim done → idle step) or the safety cap. **`setCollisionSword`**: `spinActive` = mType7 && getNo()==0x18 && stepNo==0 && getFrame()>=10 → the 3 blade spheres go live from frame ~10 (frames 0-10 = natural wind-up) and sweep 360 with joint 13. **UNVERIFIED ASSUMPTIONS (iterate after playtest):** (1) motion 0x18 IS the spin (candidates 0x11/0x1c if wrong); (2) wind-up cutoff frame 10; (3) kSpinRange 100u; (4) step-advance completion. No explicit wind-up pose — the anim's early frames + the attack shout serve as the tell.
- **TWO-LAYER MODEL / play-speed desync (VALIDATED 2026-07-03, no fix yet):** the Shade renders as TWO animated models — `mpModelMorf[0]` (main skeleton, `KN_a`) + `mInvisModel` (ghost/shader pass of morf[0]), AND `mpModelMorf[1]` (separate bmd `l_bmdData[0]={47,1}`, also `KN_a`). `setMcaMorfAnm` (base.inc:49-52) drives BOTH morfs in lockstep normally. **BUG:** the Option-2 wind-up `setPlaySpeed(1.2)` (d_a_npc_kn.cpp:1737 reset / 1818 bump) is applied to `mpModelMorf[0]` ONLY → morf[1] stays 1.0× → layers desync during the wind-up; collision spheres (from morf[0] joint 13) also lead → "ghostly arm + early spheres lead the armor." **FIX (pending): mirror EVERY `setPlaySpeed` onto `mpModelMorf[1]` too** (both the 1.0 reset and 1.2 bump) to re-sync. The speed tuning itself is fine.
- **His REAL attack repertoire (via `setSe()` anim→voice table ~:5413; all anims res0=`KN_a`, loadable for type 7):** motion **`0x11`**→anim20 `SOTOMOGIRI` = **Back Slice** (spinning slash — closest to a "spin attack"); **`0xd`**→anim28 `KABUTOWARI` = Helm Splitter; **`0xc`**→anim32 `IAIGIRI` = Mortal Draw; **`0xa`**→anim34 `JUMP_ATTACK_L` = Jump Strike; **`0x19`**→anim12 `ATTACK` = basic swing (wired). **Motion `0x18` (anim27) has NO attack voice → it is a GESTURE, marked "SWORD SALUTE"** (what the wake-up-punish currently mis-plays — NOT a real attack). **There is NO dedicated Great Spin (大回転斬り) performance animation** — no kaiten/spin cry exists; the game only ever has the Shade *demonstrate* it via the salute gesture + spoken explanation, never perform the 360 as a move. Best available "spin attack" = Back Slice (`0x11`). Wake-up punish should be re-pointed from `0x18` (salute) to a real attack (e.g. `0x11` Back Slice) + the morf[1] speed-sync fix.
- **PHASE DESIGN (proposed 2026-07-03, no code yet) — 3 phases by HP:** P1 100–70%, P2 70–40%, P3 40–0%; **final 1% deductible ONLY by a fatal/finishing blow** (the reserved fly-back `0x12`/`0xe` = the Ending Blow finisher). Per-phase escalation can unlock his real attacks (Back Slice `0x11`, Helm Splitter `0xd`, Mortal Draw `0xc`, Jump Strike `0xa`).
  - **Desync-as-telegraph (feasible precursor to Phase 2):** the two-morf play-speed desync we hit accidentally (morf[0] sped, morf[1] not — see two-layer note above) is now a KNOWN, controllable effect. Use it intentionally at ~70–80% HP as a "spectral form destabilizing" tell that P1 is ending.
  - **Armor shedding (P2+):** gate per-phase which model layers draw. Shed the solid outer layer (`mpModelMorf[1]`, bmd 47) → leave the ghost (`mpModelMorf[0]` + `mInvisModel` shader pass) → reveals his spectral form. **Sword is a design CHOICE, not forced:** the blade's collision rides morf[0] joint 13, so it is NOT bound to the shed outer layer — we can keep it (bared spectral warrior still armed — recommended) or drop it. VERIFY `l_bmdData` {47,48,49} + `mpPodModel` identities (armor vs ghost vs sword/pod) when building.
  - **Great Spin VFX reuse (feasible, USER-APPROVED 2026-07-03):** Link's Great Spin emits `ID_ZI_J_KAITENGIRI_A/B` (`_L_A` charged) particles (`d_a_alink_effect.inc` ~:985) + `Z2SE_AL_KAITEN_L_SLASH`/`Z2SE_AL_V_KAITENGIRI_L`. **FA "Great Spin AOE" is a DAMAGE-MODIFIER window, NOT a standalone hitbox:** `s_ebGreatSpinAoeFrames` (`kEbGreatSpinAoeDurationFrames=30`) makes the Ending Blow deal 50% GS damage over 30 frames via `dFocusedArts_resolveMeleeDamage` on LINK's OWN spin collision + sustained VFX (`gsSustainedVfx`). So for the Shade the HITBOX = his own sword spheres (`mSphCc`) swept 360 during the spin (as planned); the FA AOE code is Link-side and not a reusable enemy hitbox. **User OK with slight body/VFX disjoint** (he's supernatural). Recommended wake-up-punish redesign: salute (`0x18`) = WIND-UP tell → Back Slice (`0x11`, real spin motion + his own `mSphCc` collision) + `KAITENGIRI` VFX/SE = the hit. Replaces the current `0x18`-only mis-play; also fixes the morf[1] speed-sync.
  - **Other hidden skills = keep AS VANILLA AS POSSIBLE (design principle, user 2026-07-03):** Helm Splitter (`0xd`), Mortal Draw (`0xc`), Jump Strike (`0xa`), Back Slice (`0x11`) should read as authentic — he is Link's EQUAL wielding every hidden skill. Use his native attack anims + native cries (`KABUTOWARI`/`IAIGIRI`/`JUMP_ATTACK_L`/`SOTOMOGIRI`) unchanged.
  - **Sword joins the phantom = CONFIRMED feasible (user requirement 2026-07-03):** the sword collision + skeleton ride `mpModelMorf[0]` (joint 13). Shedding the outer layer (`mpModelMorf[1]`, bmd47) keeps morf[0]; the ghost look can use `mInvisModel` (shader pass of morf[0], which includes the sword). So the phantom retains the blade. (`mpPodModel`=bmd49 is the mType-4 Mortal-Draw energy FX, unrelated.) Verify exact per-layer draw + sword mesh assignment at build time by toggling each layer's `entryDL`.
- **PROJECTILE ATTACK — lightning/magic ball (RESEARCHED 2026-07-03, FEASIBLE for Phase 1):** the Shade's Shield-Attack lesson (teach02) has him throw an energy ball that Link deflects with a shield attack. Actor = **`daObjKnBullet_c` / `fpcNm_KN_BULLET_e`** (`d_a_obj_knBullet.cpp`). **Particle-based — NO model, NO arc dependency** (renders via `ID_ZI_S_KN_MAGICBALL_A/B/C` from the global `dPa_RM` bank; SE `Z2SE_KN_BALL_LEVEL`) → **spawnable by the type-7 boss anywhere via `fopAcM_createChild(fpcNm_KN_BULLET_e, GetID(this), 0, &spawnPos, roomNo, &angle, ...)`**. Spawned in `teach02_shieldReflectWait` (`d_a_npc_kn_teach02.inc:252`); throw anim = motion 32 → `0x19` with `mActionMode==7` (gives `Z2SE_KN_V_BALL_READY`/`BALL_THROW` voices via `setSe`). Bullet flies forward at `l_DATA.move_speed`, radius `l_DATA.radius`; **shield-attack REFLECT is built in** (`ChkTgShieldHit`+`CUT_TYPE_GUARD_ATTACK` reverses it toward the thrower — `d_a_obj_knBullet.cpp:110`), and a reflected ball hitting the Shade is detected at `d_a_npc_kn.cpp:5275` (`getActionMode()==0`). To implement: new boss attack state — face Link, play throw anim (motion 32/`0x19` + set `mActionMode`=7 for voices), `createChild` the bullet aimed along facing. Reflect interaction comes free (great parry/shield-attack mechanic for the boss).
  - **Ball data (`l_DATA`, `d_a_obj_knBullet.cpp:11`):** radius **30**, move_speed **20/frame**, lifetime **150f**. **At: `AT_TYPE_THROW_OBJ` atp `0xd`=13 (~3 hearts) damage.** Straight-line (no homing) along `shape_angle.y` set at launch; **flies THROUGH walls** (`fopAcM_posMove(NULL)` + `col_chk` only tests sphere hits, no wall/ground collision) → max reach ≈ 20×150 = **~3000u** (bigger than any arena) → **it always reaches Link in-arena; the arena size does NOT limit the ball's reach, only the TRIGGER window.**
  - **Throw lifecycle (teach02_shieldReflectWait):** motion 32 (ready) → motion `0x19`; at `0x19` frame **2–3** spawn the ball at joint **13** (`sp20(150,0,0)`×anmMtx(13)) inert (`mActionMode 0`); at frame **34–35** launch (`setActionMode(1)` + set ball `angle`/`shape_angle` = `mCurAngle.y`) → flies along facing. ~35f wind-up before release = the telegraph. Reflect states: `mActionMode 1`=flying at Link (GUARD_ATTACK→`mActionMode 2` reversed at parent; plain shield block or body-hit → delete + `setTalkFlag`); `mActionMode 2`=flying back at Shade (hits Shade → delete — **could be wired as self-damage for the boss's parry-punish**).
- **ARENA BOUNDS = STAGE COLLISION (DZB), NOT in source (research 2026-07-03):** F_SP200 room0 floor + invisible walls are stage data; invisible walls = DZB wall polys handled by `dBgS::WallCorrect` (cf. per-room wall-off note §3d) — there is NO simple queryable arena AABB. **To size the projectile trigger, MEASURE in-game:** walk Link to each wall/edge, read pos via F5 (Shade is at `{1258.5,1000,-1307.8}`) → min/max X/Z → max Link↔Shade distance; and/or use debug Collision view (Tools→Collision view) to see floor/wall extents. **Design use:** since the ball reaches anywhere, the arena extent only calibrates the THROW-TRIGGER window — throw when `kn_dist` is beyond melee (~250u swing range) up to the arena max; small arena ⇒ ball rare (Link usually in melee), large arena ⇒ ball is the anti-kite pressure tool. First pass can use a default range window (e.g. throw at 250–1200u on a cooldown), then tune to the measured arena.
  - **ARENA MEASUREMENTS (F_SP200 room0 layer0, via F5 Player Info; IN PROGRESS 2026-07-03):** floor is FLAT at **Y=1000**. Shade at `(1258.54, 1000, -1307.81)`. Points so far (user-believed N↔S line, but X drifts ~990 so the line is diagonal to world axes / arena is rotated): **P1 `(-164.95, 1935.61)`** (dist from Shade ≈ **3542u**), **P2 `(825.42, -1759.40)`** (dist ≈ **626u**). **P3 `(-1848.73, -662.05)`** (W, dist ≈ 3174u), **P4 `(1833.25, 695.96)`** (E, dist ≈ 2085u). 4-corner extremes: X ≈ −1848.7…1833.2 (~3682), Z ≈ −1759.4…1935.6 (~3695). ⇒ **~3700×3700 rotated quad (diamond in XZ); rough center ≈ (161, 52); Shade OFF-CENTER at (1258,−1308) toward the P2/P4 side.** Max Link↔Shade dist so far = **P1 ~3542u** (> ball's ~3000u reach).
  - **ARENA COMPLETE (6 boundary samples, F5, 2026-07-03; walk-drift acknowledged so treat as ~±100u):** P1 `(-164.95,1935.61)` 3542u · P2 `(825.42,-1759.40)` 626u · P3 `(-1848.73,-662.05)` 3174u · P4 `(1833.25,695.96)` 2085u · P5 `(-331.73,-1887.12)` 1692u · P6 `(359.70,1928.92)` 3359u. Boundary order (CCW around center): P3→P5→P2→P4→P6→P1. **Bounding box X[-1848.7,1833.2] (~3682) × Z[-1887.1,1935.6] (~3823); rough center (112,42); rotated hex/diamond.** Shade at `(1258.5,-1307.8)` is OFF-CENTER toward +X/−Z (SE-ish). **Max Link↔Shade = P1 3542u, min = P2 626u.** BALL REACH ~3000u ⇒ **OUT of reach: P1 (3542), P6 (3359), P3 (3174)** (safe zone — the far +Z/−X arc); **in reach: P2 (626), P4 (2085), P5 (1692)**.
  - **ARENA MODEL CORRECTION (user 2026-07-03):** the **Shade is ALWAYS inside the invisible walls** (never outside). Treat the arena as a **roughly REGULAR polygon** (near-circular, pointed edges) — the large spread in my per-point distances is amplified by walk-drift + the Shade's off-center placement, NOT a wildly irregular floor. More point-to-point samples would refine the true shape. **Practical upshot: the projectile trigger is a simple RADIAL check (`kn_dist >= n`) that is INDEPENDENT of arena shape;** the arena size only calibrates a sensible `n` and confirms Link can actually reach it. (The earlier "safe arc / out-of-reach corners" framing still holds physically for a SINGLE ball, but see barrage spec — the barrage's purpose is to deny that distance, not to reach it.) **THROW-TRIGGER RECOMMENDATION:** fire when `kn_dist` ∈ ~[400, 2800]u (beyond melee, within ball reach) on a cooldown; skip/So he charges instead when >2800u (ball would whiff) or <400u (just swing). Tune to feel.
- **PROJECTILE BARRAGE — DESIGN SPEC (user intent 2026-07-03, NOT yet gameplay-proven; Phase-1 anti-kite move):** a **4-shot fast projectile volley** (`KN_BULLET`s) that keeps the fight close-quarters and punishes Link for backing off.
  - **Trigger:** when `kn_dist >= n` units (n TBD — user may relay, else pick a "close-quarters" value beyond melee, e.g. ~600–900u, tune). Radial check, arena-shape-independent.
  - **Volley:** fire **4 balls**, each preceded by a wind-up, with a **short pause (a few frames) between launches**. **ONLY the first launch is gated on the `n`-units rule** — once the volley starts, launches 2–4 fire regardless of Link's current distance (he can't escape the volley by closing in).
  - **Tracking:** the wind-up → launch aim uses the **SAME tracking as the basic arm swing** (motion 0x19): rotate toward `searchPlayerAngleY` during wind-up, commit the aim at launch (each ball flies straight along facing at its launch frame — Link dodges by moving, like the sidestep vs. the swing).
  - **VULNERABILITY WINDOW (the counterplay):** the Shade is **SUSCEPTIBLE TO DAMAGE for the ENTIRE volley** — first launch through final launch, **including the pauses**. So a skilled Link who rushes in and dodges the balls turns the barrage into an **opening** (free hits). Implementation: during the barrage state, body Tg = `OffTgShield` + `MARK_1` (like the mMode-4 stun's vulnerable collision) so attacks land + react; contrast with normal combat where he guards (`OnTgShield`). This makes the volley a high-commitment, high-risk zoning tool (risk/reward), NOT a safe turret.
  - **Impl sketch:** new barrage state (e.g. mMode 6) entered from mMode 3 when `kn_dist>=n` (+ cooldown); loop 4× {wind-up w/ swing-tracking → `createChild(KN_BULLET)` at joint 13 → `setActionMode(1)` aimed at `mCurAngle.y` → pause}; hold `OffTgShield`/`MARK_1` throughout; on any `ChkTgHit` react (reuse the stun's flinch/`0xb` or a lighter reel) + (future) HP drop; exit to mMode 3 after the 4th. Ball spec/spawn per the PROJECTILE ATTACK note above (particle-based, no arc dep, atp 13, reflectable).
  - **FINAL DECISIONS (user 2026-07-03):**
    1. **Rhythm = 4 full wind-up→throw CYCLES** (each aimed, tracking like the swing), but **played FASTER via `setPlaySpeed` on BOTH `mpModelMorf[0]` AND `[1]`** (phase 1 = both layers present → sync them; this IS the desync fix applied to both morfs) so each cycle is *barely* readable.
    2. **Opening gate:** the Shade will **NOT use the barrage until AFTER his first basic sword swing** has been used toward Link — even if Link's spawn is already ≥ n from the Shade's placed spot. ⇒ add a `hasSwung` flag (set when the first motion-0x19 swing fires/completes); barrage trigger requires `hasSwung`. Keeps the fight opening on melee, never a projectile-spam start.
    3. **Damage = 3 hearts per ball** (pre-multipliers). Ball already carries atp `0xd`=13 (~3.25 hearts) — verify it actually damages Link in the boss context; target atp **12** (=exactly 3 hearts) if adjustable per-instance, else accept ~13.
    4. **Hits INTERRUPT the volley:** any `ChkTgHit` landed during the barrage (any launch or pause) **cancels the remaining shots and drops him into the STUN state (mMode 4) for the full shield-bash duration (`field_0xdec`=120 ≈2s)** — reuses the existing stun (head-down `0xb`, `OffTgShield`, hit reactions, Great-Spin wake-up on recovery). So rushing in + landing a hit = the same punish window as a shield bash. He is `OffTgShield`+`MARK_1` (vulnerable) for the WHOLE barrage until hit.
  - **IMPL PLAN:** new **mMode 6 (barrage)** entered from mMode 3 when `hasSwung && kn_dist >= n && barrageCooldown==0` (preempts the charge). Loop 4×: play `0x19` throw (both-morf `setPlaySpeed` bump), spawn `KN_BULLET` at joint 13 frame ~2-3, `setActionMode(1)` aimed at `mCurAngle.y` frame ~34-35, then short pause; track Link during each wind-up. `setCollision` case 7: treat `mMode==6` like `mMode==4` (`OffTgShield`+`MARK_1`). `setCollisionSword`: SUPPRESS the sword At during mMode 6 (0x19 plays but must NOT melee-hit — the balls are the attack). On `ChkTgHit` during mMode 6 → enter mMode 4 stun. After 4th ball → mMode 3 + set `barrageCooldown`. Fields: `hasSwung`=`field_0x15cf` (free), shot counter=`field_0x15be` (free), pause timer=`field_0xdec` (reuse), cooldown=`field_0x15bd` (free in combat path). **OPEN/PROPOSED:** `n` ≈ 800u (tunable — "close-quarters" threshold); throw-anim speed ≈1.4×; barrage cooldown ≈90f; reflected-ball self-damage (TBD).
  - **BUILT (build ~34, 2026-07-03):** new **mMode 6** in `heroShadeCombat`. Trigger (mMode 3): `field_0x15cf`(hasSwung, set on first 0x19 swing complete) && `field_0x15bd`(cooldown)==0 && `kn_dist>=800` && not swinging → enter mMode 6. Loop 4 throws: track Link (swing-tracking) during `0x19`, spawn `KN_BULLET` at joint 13 @frame≥2, `setActionMode(1)` aimed @frame≥34, finish @frame≥40 → next (pause `field_0xdec`=8) or end (@`field_0x15be`>=4 → mMode 3 + cooldown 90). `ChkTgHit||ChkTgShieldHit` during barrage → mMode 4 stun (120f) + cooldown. Both morfs `setPlaySpeed(1.4)` during throws (synced). `setCollision` case 7: `mMode==4||6` → OffTgShield+MARK_1 (vulnerable). `setCollisionSword`: swingActive suppressed when `mType==7&&mMode==6` (balls are the attack, sword must not melee-hit). **ALSO fixed the wind-up DESYNC: both morfs now reset to 1.0 each frame + swing wind-up (mMode 3) bumps BOTH morfs to 1.2.** Fields: hasSwung=`field_0x15cf` (added to init block =0), shots=`field_0x15be`, cooldown=`field_0x15bd`, pause=`field_0xdec`, held-ball=`parentActorID`. Damage = ball's native atp ~13 (~3 hearts), unchanged. **Held-ball edge:** if interrupted mid-wind-up (frame 2–34) a held ball (actionMode 0) lingers ~3s at spawn then auto-deletes (lifetime) — minor polish TODO. **SUSPEND-ZONE FIX (build ~35):** first playtest — throw anims played but NO ball appeared / no damage. Cause: the `KN_BULLET` spawns inside the F_SP200 suspend zone → gets status `0x20000000` → `Execute()` blocked (no move, no particles, no damage). SAME bug as the original Shade invisibility. Fix: exempt `fpcNm_KN_BULLET_e` in `daSus_c::check` (`d_com_static.cpp`), alongside the existing `NPC_KN` exemption. Balls then fired.
  - **BALL ORIGIN + SPEED FIX (build ~36):** 2nd playtest — balls fired but (a) spawned at the wind-up APEX (joint 13 = sword pointed skyward) so they flew over Link, and (b) too slow. Fixes: (a) origin is now a COMPUTED point in front of him at ~chest height from his FACING (`current.pos` + Yrot(mCurAngle.y)·(0,90,150)), NOT joint 13; the held orb is repositioned there each frame so it tracks in front while charging, and RELEASES at frame≥34 (the forward strike) aimed at Link. (b) added **per-instance `mSpeedMul`/`mRadiusMul`** to `daObjKnBullet_c` (header struct 0x71c→**0x724**; default 1.0 so the vanilla lesson is untouched); boss balls launch with **`setSpeedMul(5.0)`** (5× = 100 u/frame) + **`setRadiusMul(1.6)`** (radius 30→48, prevents tunneling at 5×). Reflect path also honors `mSpeedMul`. Tunable: speed, radius, origin height 90, release frame 34.
  - **BALL DAMAGE FIX (build ~37):** 3rd playtest — balls hit & Link reacts but NO damage. Cause: the lesson ball's collision source `l_sph_src` has **`mAtp = 0x0`** (the `0xd` I earlier misread as atp is actually part of the SE/`mBase` field of `cCcD_SrcObjAt {u32 mType; u8 mAtp; base}`). The tutorial ball is intentionally non-damaging — it only delivers a hit-REACTION. Fix: added `setAtp(u8)` to `daObjKnBullet_c` (calls `mCcSph.SetAtAtp`, same as the Kn sword's `SetAtAtp(12)`); boss balls launch with **`setAtp(12)` = 3 hearts**. Also bumped speed per "could be faster."
  - **FIRST-BALL DAMAGE + 10× (build ~38):** 4th playtest — 1st ball did ~1 heart, balls 2–4 did 3. Cause: `setAtp(12)` was called only at the launch frame, which raced the 1st ball's `Create()` (Create not done → atp still 0 → dealt the ~1-heart collision floor); balls 2–4 (Create long done) got 12. Fix: call `setAtp(12)` EVERY held frame (2–34), so all 4 are armed before launch. Also speed **6×→10×** (`setSpeedMul(10.0)`, 200 u/frame) + radius **1.8→2.5×** (radius 75) — at 200/frame a small sphere tunnels between frames (a target at the midpoint of two 200-apart spheres is missed unless ball_radius+link_radius ≥ 100), so the hitbox must scale up. **TRADEOFF:** big+fast ball; if undodgeable, the clean fix is SUB-STEPPING the ball's movement (small radius, move in N increments/frame checking collision each) instead of a giant hitbox.
  - **PASS 5 (build ~39):** still too easy to dodge (straight-line shots are sidesteppable). Changes: pause between launches `field_0xdec` **8→7** (frames are integer, so ~1-frame trim ≈ the requested 5%); ball speed **10×→12×** (`setSpeedMul(12.0)`, 240 u/frame) + radius **2.5→3.2×** (radius 96, anti-tunnel at 12×); volley count **4→7** (`field_0x15be >= 7`). **Held-orb cleanup:** on barrage interrupt, delete the un-launched charging orb (was lingering ~3s as a stray harmless visual during the stun). **GREAT SPIN VFX ADDED to mMode 5:** emit `ID_ZI_J_KAITENGIRI_A/B` (Link's Great Spin particles, `dPa_RM` + `dComIfGp_particle_set` one-shot, scale 2.0, at chest height) on stun-recovery spin — previously the salute had no VFX. NOTE: one-shot burst at spin entry; if it ends before the ~60f spin, make it sustained.
  - **VFX BANK FIX (build ~40):** the KAITENGIRI emit showed nothing because it was wrapped in `dPa_RM(id)` (=0x8000|id = the RESIDENT/RM bank, where `ID_ZI_S_KN_MAGICBALL` lives). But `ID_ZI_J_*` (Link/Great-Spin) effects are in the NORMAL bank — Link emits them via `setEmitter(rawId,…)` (d_a_alink_effect.inc:1111) with the BARE id. Fix: `dComIfGp_particle_set(ID_ZI_J_KAITENGIRI_A/B, &pos,&rot,&scale)` with NO dPa_RM. (Balls keep dPa_RM — MAGICBALL genuinely is resident-bank.)
  - **GREAT SPIN VFX v2 (build ~41):** raw-id emit showed a spin but it was the SMALL regular spin, too big, off-center. Fixes: (a) use the LARGE 'D' set **`ID_ZI_J_KAITENGIRID_A/B`** (Link's `effNameLarge` = the actual Great Spin) instead of `KAITENGIRI_A/B` (regular); (b) center on the **backbone joint** (`getAnmMtx(getBackboneJointNo())` → multVecZero) instead of `current.pos+80`; (c) **default scale** (dropped the 2.0× that made the radius too large). CONFIRMED: the oversized radius was the `fxScale(2.0)`. Awaiting playtest. Further large variants avail if needed: KAITENGIRID_C–F, KAITENGIRIL_*, KAITENGIRIH_*.
  - **SHIELD-BASH REFLECT = ALREADY WORKS:** `daObjKnBullet_c::col_chk` (actionMode 1) already reflects on `ChkAtShieldHit`+`CUT_TYPE_GUARD_ATTACK` toward the parent (the Shade) — active for boss balls (parent set via createChild). No code needed; if hard to time, that's the 12× speed, not a missing feature.
  - **Z-TARGET DROPS AFTER ~3 ATTACKS (open, likely HP-pool):** NOT the Shade's code — `setParam()` re-asserts `fopAc_AttnFlag_BATTLE_e` every frame (base.inc:102, top of execute), and `tgHitCallBack` only records cut type (no HP/death logic). So the flag isn't lapsing and he isn't marked dead. The lock-on debug overlay shows **HP 0/0** (no pool wired) — strongly suggests the mod's HP-aware lock-on/battle-target system drops a 0-HP target after it registers hits. **Likely fixed by wiring the HP pool (next roadmap item);** alt = dig into the mod's lock-on drop condition. Deferred pending user call.
  - **BARRAGE THROW TIMING (reference):** throw motion 0x19 spans frames **0→40** (cut to ready at 40); spawn@2, TRACK 0→**34**, launch@34, finish@40 ⇒ **tracking = 34/40 ≈ 85%** of the throw. At 1.4× playback: ~28.6 real frames total, ~24.3 real tracking (~0.4s). Change tracking % via the frame-34 threshold vs the 40-frame span.
- **GREAT SPIN SFX (build ~42):** added `Z2SE_AL_KAITEN_L_SLASH` (Link's large-spin slash whoosh, matches the KAITENGIRID VFX) via `Z2GetAudioMgr()->seStartLevel` at the Shade's pos on mMode-5 entry, alongside his `Z2SE_KN_V_ATTACK` voice. (No native KN spin cry exists, so borrow Link's — the voice one `Z2SE_AL_V_KAITENGIRI_L` is Link's grunt, NOT used.)
- **HEALTH POOL — STARTED (build ~42):** KEY: `daNpc_Kn_c : public fopEn_enemy_c` → the Shade is an ENEMY and inherits `health` (same field the Darknut uses via `actor->health`). Vanilla Darknut phase-2 body HP = **100** (`d_a_e_dn.cpp:3436`). Shade `health = 300` (**3× Darknut, BEFORE mult**) set in `reset()` gated on mType 7. Damage: each landed hit in a vulnerable window (mMode-4 stun + the mMode-6 barrage-interrupt hit) does **−10** (same as a Darknut → ~30 hits = 3× a Darknut's ~10). Floored at **1** (no defeat event yet; the final 1% is reserved for a finishing blow per the phase plan). **Likely also FIXES the Z-target drop** — the lock-on was probably releasing a `health==0` "dead" target; a valid 300 HP keeps it locked. NOTE: the lock-on overlay's "True HP mult 8x" is the mod's global enemy multiplier — it may scale the 300 base on top (user asked for 3× "before multipliers", so base=300 is intended). Per-hit damage (10) and total (300) are tunable.
- **BOSS HEALTH BAR — DONE (build ~43), reuses the existing HUD:** the boss bar already exists (`d_albw_boss_hp_hud.cpp`, gated behind the EXISTING `game.bossHealthBars` GLOBAL toggle — NOT per-boss; supports Diababa/Fyrus/Morpheel/Stallord/Blizzeta/Armogohma/Ganondorf per its help text, though only Armogohma was wired). It's a bottom-center red bar + boss name; HP from `dAlbwHP_getLockonDisplayHp` (same source as the lock-on debug); appearance is centralized `constexpr` (kBarWidthFrac/HeightFrac/TopFrac/Border/NameFontFrac/NameGap/Outline) + colors in `drawRect` — SHARED across bosses, so future scale/spacing/hue/opacity tuning hits every boss incl. the Shade. **Changes (additive, revertible):** (1) `bossDisplayName()` +`case fpcNm_NPC_KN_e → "Hero of Time"` (Armogohma's `B_GM` case untouched → its name unaffected); (2) `dAlbwBossHpHud_draw()` generalized: Armogohma query first (unchanged, precedence), else the Shade — only the {current,max,name} selection generalized, the bar render is shared; (3) new `dShadeBoss_queryHealthBar()` in the shade-boss module — `fopAcM_SearchByName(fpcNm_NPC_KN_e)` + `health>0` (uniquely the combat Shade — no mType coupling) → `dAlbwHP_getLockonDisplayHp`; (4) `reset()` now sets `field_0x560 = 300` too (the MAX; lock-on/bar reads max from it, else the ratio reads full as health drops). Single HP pool — no phase query needed (planned phases stay within one pool). **Revert = undo those 4 spots; Armogohma path is byte-identical.** Bar shows ONLY when `bossHealthBars` setting is ON.
- **DAMAGE + CATEGORY FIXES (build ~44):** (1) **Boss category** — added `fpcNm_NPC_KN_e` to `sBoss[]` in `d_albw_hp_mult.cpp` → the Shade is now Boss (gets `hpMultBoss`, not the Common 8×). Lesson Shades (health 0) are untouched by the scaler. (2) **Vanilla per-attack damage** — replaced the flat `health -= 10` (both mMode-4 stun + mMode-6 interrupt hits) with the shared pipeline `dCcU_AtInfo atInfo{}; atInfo.mpCollider = mCylCc.GetTgHitObj(); at_power_check(&atInfo); cc_at_check(this, &atInfo);` (added `#include "d/d_cc_uty.h"`), run BEFORE `ClrTgHit`. Same path the Darknut uses → sword/item/hidden-skill/flurry rates + the mod's Link-Damage/HP settings all apply. Floored at 1 (pending the 1-HP finisher). NOTE: a reflected-ball/overkill could momentarily hit 0 → the enemy-rupee grant may fire once; harmless, revisit with the defeat event.
- **TODO next — 1-HP ENDING-BLOW OPENING (issue 2, deferred this pass):** currently he floors at 1 HP with no knockback. Plan: at `health <= 1`, `setCollision` → OffTgShield + Tg hit-mark **3** (the Darknut's finishable mark, `d_a_e_dn.cpp:2428`) so he's always hittable/finishable; ANY hit (sword/item/skill/flurry) → new mMode 7 = fly-back using the RESERVED `0x12`/`0xe` recoils WITH knockback speed (+`speed.y` pop) → downed/open → ending blow. This is the finisher and dovetails with the defeat/farewell → warp-back event. Do as one coherent pass.
- **TODO next per roadmap:** defeat/farewell event → warp back to basement wolf; 3-phase structure (70/40/0%, 1%-finisher) + armor-shed + desync tell; more Phase-1 attacks. Then more attacks (jump strike, spin, mortal draw, clones) + parry-stagger. Point-blank sphere reposition.

### FULL 3-PHASE BOSS DESIGN (user vision, 2026-07-XX) — authoritative
Supersedes the earlier simple 70/40/0% HP sketch. Moves CARRY OVER across phases (all Hidden-Skill attacks + swing + barrage available: Back Slice `0x11`, Helm Splitter `0xd`, Mortal Draw `0xc`, Jump Strike `0xa`, Great Spin FX).

- **PHASE 1 — normal 1v1** with the armored Shade. Add MORE moves (the above hidden skills). Current state (done): swing, guard/bash→stun, wake-up Great Spin, 7-ball barrage, HP pool, boss bar. Still to add: the finisher (1-HP fly-back → ending blow) and the extra hidden-skill attacks.
- **PHASE 2 — GHOST CLONES.** The Shade sheds ARMOR (spectral/ghost form) and MULTIPLIES: the main ghostly Shade + **2 clones** (3 total), each named **"Clone n"**. Every clone has the SAME full moveset. They **take turns** attacking Link (turn manager). Leverage: the Kn's native clone/divide mechanic (`mDivideNo`, teach06/07 spectral-copy set piece) OR spawn 2 extra mType-7 Kn instances in ghost form (armor-shed = the morf-layer toggle we researched).
- **PHASE 3 — NPC GAUNTLET + vertical + final form.** The "clones" are now SEPARATE NPC actors that spawn and take turns: a **Goron**, a **Zora (armed with a weapon)**, and a **retextured Skull Kid** (Skull Kid = `E_PM`, `fpcNm_E_PM_e`). Each attacks Link with its own AI + its own HP pool. Meanwhile the **Shade retreats to a HIGHER FLOOR** — a ledge in F_SP200 that has its own (invisible) collision — and "hides" there when it's not his turn, **striking downward** for a few hits when it is. Once Link trims down ALL the unique NPCs' HP, the Shade descends again — this time swapped to a **custom Hero-of-Time-Link / Dark-Link model** (WORKSHOPPING the model) — for a final **1v1 using the full Hidden-Skill moveset**. On the finishing blow: **fly-back (ending blow) → swap back to the ARMORED Hero's Shade model → ending cutscene → warp back to Link's home**.
  - HP model note: P1/P2 can share the Shade's one pool, but **P3 inherently needs MULTIPLE pools** (Goron/Zora/Skull-Kid each + the Shade's final-form pool) — the "one pool" hope only holds through P2.
  - Assets needed (P3): armed-Zora model, retextured Skull Kid, Hero-of-Time/Dark-Link model, armored↔ghost↔final model swaps (model-swap STATE tech exists — cf. the sumo-outfit work).
  - To VERIFY in-game: the P3 ledge's collision/extent + a path for the Shade up/down (F5 + collision view).

### Combat roadmap (user direction 2026-07-03)
Order after the basic swing works:
1. **Combat AI** (IN PROGRESS): `heroShadeCombat` (mType-7 action, [d_a_npc_kn.cpp]) — approach + swing (motion 0x19 → `setCollisionSword` At-collision, motion-driven, no event). First pass = single basic attack; then add jump strike / great spin / mortal draw + clones, plus reaction-to-being-hit.
2. **Health pool** — give the Shade an HP value + damage intake (divert `tgHitCallBack`, gate on mType 7) + boss health bar (`dComIfGs_BossLife_public_Set/Get`).
3. **End event / defeat sequence** — on defeat, play a Hero's-Shade-style farewell (animation/cutscene like the vanilla skill-lesson conclusion, cf. `ECut_seventhSkillGet` BGM+dialogue) then **warp the player back to Link's House basement (R_SP01 room 7), in front of the summon wolf** at the spot they entered from (the wolf's spawn coords `{35.7236,-1082.5,-602.9236}`). i.e. the return warp target is the basement wolf, not Ordon proper.

### Phase 1 DONE — idle Shade renders in arena (2026-07-03)
✅ The idle Hero's Shade (`mType 7`) now spawns, stands, tracks Link, is Z-lockable, and has collision in **F_SP200 room 0 layer 0** (the Hyrule-castle "night" arena) at `{1258.5428, 1000, -1307.8118}` angle `-6770`. A "Speak" prompt shows but does nothing yet (no talk flow — intro dialogue is a later step).

**CRITICAL DISCOVERY — the suspend-zone system (`daSus_c`):** spawned actors were invisible because `daSus_c::check(actor)` ([d_com_static.cpp:201]) sets actor status **`0x20000000` (`fopAcStts_UNK_0x20000000_e`, 1<<29)** on actors inside stage-defined **suspend zones**, which blocks BOTH execute AND draw (checked at [f_op_actor.cpp:341/358] exec, and the draw gate). It exempts only groups 1 and 5 (Link). F_SP200's arena spots fall inside suspend zones, so any spawned NPC/enemy there (our Shade, debug bokoblin/beamos) gets suspended → never draws. The vanilla Shade renders because it sits outside those zones. **FIX (applied):** exempt `fpcNm_NPC_KN_e` from `daSus_c::check` (`fopAcM_OffStatus(...,0x20000000); return;`) — harmless to vanilla lesson Shades (already outside suspend zones). This is THE reason runtime-spawned actors don't render in F_SP200; note it for any future actor placed in cutscene maps.

Diagnostic journey (all temp logging since removed): confirmed spawn fires → create COMPLEATEs (model built, not hidden) → `Draw()` never called → cull status irrelevant (`Draw` fired once with cull off, then blocked) → `0x20000000` set every frame → traced to `daSus_c`. Files with the permanent fix: `d_com_static.cpp` (exemption). Type-7 infra: `d_a_npc_kn.cpp` (`heapSize[7]`, `isDelete` case 7, `l_loadResPtrn0`={1,2,-1}), `d_s_room.cpp` (spawn), `d_albw_shade_boss_wolf.cpp` (warp). Uncommitted.

### Phase 1 (earlier notes) — idle Shade placed (2026-07-02, BUILT CLEAN)
The idle Hero's Shade (`d_a_npc_kn` `mType 7`) now spawns in **F_SP200 room 0** (where the "Duel" warp lands). Edits: `heapSize[7]` 0x0000→0x4B10; `isDelete()` `case 7: return 0`; `d_s_room.cpp` spawn (param `0x0008`, gated toggle + F_SP200 + room 0). `mType 7` runs the `wait` action (stands, tracks Link) via `selectAction`'s default; `setParam`'s **default case** gives it lock/talk attention; `setCollision` = zero cylinder (no body collision yet — fine for placement). Confirmed placement (F5, 2026-07-02): **F_SP200 layer 0, room 0**, Shade pos `{1258.5428, 1000.0, -1307.8118}`, angleY `-6770`. (My first guess `{0,1000,-600}` was near the origin — actor spawned fine there, just off in the fog; Y=1000 was right, X/Z were the miss.) No conflict with vanilla trials (boss is post-all-skills; no lessons load F_SP200 then). **CONFIRMED: F_SP200 layer 0 room 0 already renders the full Hyrule-castle backdrop** — so the misty-castle look is shared (shared demo-vrbox), not layer-7-exclusive. That's the arena the "Duel" warp (layer -1→0) lands in. Uncommitted.

### Background question — ANSWER (2026-07-02)
Can layer 7's background be shown in a different F_SP200 layer/room? F_SP200's look is driven by a **special "demo vrbox"** (`dKyd_dmvrbox_getp`, [d_kankyo_data.cpp:754]) plus **heavy F_SP200-specific kankyo code** with **layer-keyed branches** (e.g. [d_kankyo_rain.cpp:2423] gates on `F_SP200 && getLayerNo(0)==0`). So: (a) the atmospheric background (misty sky/fog/vrbox) comes with the STAGE F_SP200 and shows in whichever room you warp to, but (b) it's **layer-dependent** in places, and (c) the distinct scenery (the castle geometry) is baked per room/layer. ⇒ You **cannot cleanly composite** layer 7's full background onto a different layer/room — the kankyo branches on layer and the scenery is baked. Practical answer: **to get layer 7's exact look, warp to layer 7** (`setNextStage(...,7)`); each layer renders its own baked background. (Not transplantable without stage editing.)

### 3d. Arena goals & open threads (user direction 2026-07-02)
- **Per-room wall-off is a LIVE candidate**, not dismissed. Approach: custom `#if TARGET_PC` gate in `dBgS::WallCorrect` (or `MoveBGOnly` on Link's Acch) to drop STAGE-wall collision for Link only while in the arena room + toggle, PAIRED WITH a larger MoveBG **ground** floor so Link roams the bigger area and stops at the new floor's edge. **Sequence LOCKED: walls first, then insert floors.**
- **The initial warp room (F_SP200, layer -1/default = the howling variant) may be the END-GOAL arena** — keep it as a candidate, don't assume layer 7 is required.
- **Open question — can layer 7's background/visuals be shown in the warp room?** Assessment: stage layers are **self-contained baked data** (geometry + kankyo per layer). Warping to a layer gives THAT layer's look + arena together; compositing one layer's background onto a different layer/room is **not cleanly feasible** without stage-data editing. Practical answer: to get layer 7's look, warp to layer 7 (`setNextStage(...,7)`); to keep the howling room, accept its own look. (Flagged to revisit.)
- **Edge-vs-wall test still pending** — decides whether any wall-off is even needed.

### 3. The arena
- `F_SP200` = "Wolf Howling Cutscene Map" — [include/dusk/map_loader_definitions.h:613-615], declared with **rooms 0–7** (`{0, {0,1,2,3,4,5,6,7}}`). It self-loads and has floor collision.
- **CORRECTION (playtest 2026-07-02):** `F_SP200` is the **wolf-howling** cutscene map (where wolf Link is teleported to howl), **NOT** the Hidden-Skill duel arena the player is transported to during the sword-skill trials. My earlier assumption (F_SP200 == duel arena) was WRONG. The real arena stage/room is **TBD** — user is locating it in-game. Milestone-B warp currently points at `F_SP200` as a working placeholder (it has collision, so Link stands fine); swap the target once the real arena room is found.

### 4. The Hero's Shade actor — `d_a_npc_kn`
Files: `src/d/actor/d_a_npc_kn.cpp` (5284 lines) + `d_a_npc_kn_base.inc` (1187) + `d_a_npc_kn_teach01.inc` (757) + `d_a_npc_kn_teach02.inc` (630). Header: `include/d/actor/d_a_npc_kn.h`.

**It is an NPC, not an enemy.** No `mLife`, no HP pool, no defeat/death state anywhere in 7,858 lines.

**Behavior is parameter-selected:**
- `getType()` (~:749) decodes spawn param low byte: param 1–7 → `mType` 0–6 (the seven lessons); **param 0 or ≥8 → `mType` 7 (unused/default slot)**. ← natural home for a new combat mode.
- `getDivideNo()` / `getPath()` (~:771/781): high byte → clone-split count / path index.
- `getFlowNodeNo()` (~:776): from `home.angle.x` (placement data).

**Spawn gating per type — `isDelete()` (~:785):** appear/delete keyed on event bits `0x152`–`0x158` (`l_appearFlag[7]`/`l_delFlag[7]`). A duel mode slots in identically: appear-gate = `F_0344`, del-gate = new "duel cleared" flag.

**The make-or-break hit hook — `tgHitCallBack`** ([d_a_npc_kn_base.inc:6]): currently just reads the player's `getCutType()` (which skill move) and stores it via `setCutType()`. ZERO damage/HP logic. This is THE single function to extend: add `mLife`, decrement here gated on `mType == 7`, route to `mDmgStagger`.

**Damage-reaction component already present:** `daNpcT_DmgStagger_c mDmgStagger` at struct offset `0x0D3C` (d_a_npc_kn.h). Framework-level hit-stagger machinery is already in the struct, unused by the lessons.

### 5. AI architecture — function-pointer state machine
- `action()` (~:1099) just calls `(this->*mpActionFunc)()`. The brain is **`selectAction()`** (~:1447): picks ONE member-function pointer (`mpTeachAction`) by `mType` + `mActionMode` (a single linear lesson counter).
- Per-skill sub-selectors `teachNN_selectAction()` (~:1515–1599) map `mActionMode` → a named move-method.
- Each move-method is its own `mMode` sub-state machine (0/1 init, 2 active, 3 done).

**Move repertoire (all already implemented as methods):**
| Method | Move (skill) | Mechanic |
|---|---|---|
| `teach03_attackWait` (~:1929) | standing attack (Back Slice) | basic swing + approach |
| `teach04_attackWait`/`_headBreakWait`/`_finishWait` | guard/overhead (Helm Splitter) | shield-aware, multi-phase |
| `teach05_surpriseAttackWait` (~:2242) | lunge (Mortal Draw) | fast surprise strike |
| `teach06_superJumpWait` + `_divideMove`/`_superJumpWaitDivide`/`_superJumpedDivide`/`_waitDivide`/`_warpDelete` | jump attack (Jump Strike) | **splits into spectral clones** |
| `teach07_superTurnAttackWait` + divide variants | spin attack (Great Spin) | **clones + warp** |
| `wait` | idle/guard | — |
| `test` (~:1685) | debug | just freezes him — NOT combat |
| (projectile) | energy ball — `Z2SE_KN_V_BALL_READY/THROW` in `setSe()` case 12, `mActionMode == 7` | sword-beam/ball attack |

### 6. One beat traced — `teach03_attackWait` (~:1929)
Active loop (`mMode == 2`) does three things per frame:
1. Configure hurtbox to player's move: read `getCutType()`, flip target cylinder shield/hit-mark (~:1948-1955).
2. **If player hits** (`mCylCc.ChkTgHit()`): react theatrically — fly back as if Back-Sliced, `Z2SE_KN_V_DAMAGE_L`, hit SE (~:1957-1997). ← *the branch a real fight replaces with `mLife--` + normal stagger.*
3. **Else**: count down a randomized attack timer (`attack_start_time` HIO tuning); on expiry/`checkCollisionSword()`, launch his attack (set `mEvtNo` + `mFlowNodeNo`, motion sequence); else `calcSwordAttackMove()` approaches (~:1998-2034).
`teach04_attackWait` (~:2045) adds an `mActionMode = 10` advance on shield-attack hit — the lesson-progress bookkeeping a fight would guard out.

### 7. Shared locomotion — `calcSwordAttackMove(int)` (~:5125)
Pure action-code primitive every attack rides on:
- In range (<250) rolls an attack-ready motion (seq 0x19); far (>250) walks at `move_speed` toward player.
- Always turns to face player (`cLib_addCalcAngleS2` → `searchPlayerAngleY`); `param_0 == 1` = aggressive turn (rate 2 / 0x800), else HIO `rotation_num`/`rotation_speed`.
- `mJntAnm.lookPlayer()` head tracking.
NOT event-bound. Fully reusable verbatim.

### 8. Event/flow system is DIALOGUE-only — the clean split
- `mEvtNo` indexes `l_evtList[26]` (~:209): every entry is a TALK/CUTSCENE event ("FIRST_ENCOUNT", "..._SKILL_EXPLAIN", "..._SKILL_GET", "LARGE_DAMAGE_TALK") — the spoken lessons, not swings.
- `mFlowNodeNo` + `initTalk()` drive `dMsgFlow` (the spoken explanation / "you learned X" payoff).
- **Therefore:** the actual attack (movement + swing animation via `mMotionSeqMngr.setNo()` + `mCylCc` At-collision) is PURE ACTION CODE and reuses verbatim. The event-script-bound parts are ONLY the teaching dialogue, which a boss fight drops entirely. The valuable half (combat) and the discardable half (lessons) live in different subsystems — a clean separation.

### 9. Boss health bar — the engine API
- No boss gauge in `d_meter2`. TP drives it via game-state value: `dComIfGs_BossLife_public_Set()` / `_Get()`.
- Precedent: Fyrus `src/d/actor/d_a_e_fm.cpp` (`_Set(100)` ~:2799, `_Set(0)` on death ~:3764); also `d_a_b_bq`, `d_a_e_rd`, `kytag06/08` read it.
- Hook: on duel start `_Set(max)`, decrement per confirmed hit, `_Set(0)` triggers defeat.
- NOTE: the Shade already calls `dMeter2Info_onLifeGaugeSE()` (~:4668) / `dMeter2Info_getNowLifeGauge()` (~:4761), but those are about **Link's** hearts (the heal-to-full lesson nicety, `dComIfGp_setItemLifeCount(getMaxLife())` ~:4667), NOT a boss bar.

### 10. The ONE real constraint — resource/animation loading
Move animations live in **7 separate arcs**: `l_resNameList[9]` (~:238) = `KN_a`, `KN_tch01D`…`KN_tch07D`. Each lesson loads only the 2–3 it needs (`l_loadResPtrnList[8]`, ~:295). The free slot `mType == 7` currently loads `l_loadResPtrn0 = {1,-1}` (base only) and `heapSize[7] = 0x0000` (~:559).
- To use EVERY move, the new pattern must load the **union** `{KN_a, tch01D…tch07D}` = 8 arcs. Loader caps at exactly 8 phase slots (`mPhase[8]`, `for i<8`) — **fits, zero headroom**.
- Must set `heapSize[7]` (currently 0) to cover the union — larger than any single lesson (~`0x4B10`–`0x5A00`).
- Graceful fallback if RAM tight: **phase-gated movepool** — load a subset, swap arcs at phase transitions (sword skills → jump/spin+clones). Also better-paced than dumping all moves at once.

---

### 11. Clone / divide mechanic (the spectral-copy set piece)
The master Shade spawns child copies of itself — full `d_a_npc_kn` actors with type+copy-index packed into the spawn param:
- Teach06 (Jump Strike) spawn, action case 70 (~:4446): `for i in 0..1: fopAcM_createChild(fpcNm_NPC_KN_e, fopAcM_GetID(this), ((i+1)<<8)+6, &pos, room, &angle, ...)`. Param low byte `6` → `getType()` → `mType 5` (teach06); high byte `i+1` → `mDivideNo` 1 or 2. ⇒ **2 clones spawned, divideNo 1 & 2.**
- Teach07 (Great Spin) spawn, ~:4779: same with low byte `7` → `mType 6`.
- Master vs clone distinction: `mDivideNo == 0` = master; `1`/`2` = clone copy #. The MASTER (only) also spawns a child gold wolf companion (~:869-872, `fopAcM_createChild(fpcNm_NPC_GWOLF_e, ..., 0xFFFFFF02, ...)` gated `mType <= 6 && (mDivideNo==0 || mDivideNo>2)`).
- Clones run the `*_divideMove` / `*WaitDivide` / `*edDivide` action variants (teach06 mActionMode 14-18; teach07 19-24), then leave via `*_warpDelete` (mode 17/23).
- Inter-actor coordination: `fpcM_Search(s_sub6, this)` / `s_subEnd6` (and teach07 equivalents) — the master pings its clones to synchronize the multi-angle attack. `mActorMngr1/2` (`daNpcT_ActorMngr_c`) track the player / partner refs.
- **For a boss:** this is a ready-made "Shade splits into copies and strikes from all sides" special. Reusable, but it depends on the teach06/07 mActionMode choreography — a combat mode would trigger the sequence as a discrete special attack, not free-roam. `mDivideNo` (param high byte) controls copy count.

### 12. HIO tuning block — `mpHIO->m` (the rebalance knobs)
Struct `daNpc_Kn_HIOParam` ([include/d/actor/d_a_npc_kn.h:7]): a `daNpcT_HIOParam common` base (0x00–0x8B: attention/talk distances & angles, body/head look-angle limits, weight — shared NPC params) + Kn-specific fields. Static defaults in `daNpc_Kn_Param_c::m` ([d_a_npc_kn.cpp:71]); in DEBUG a live HIO editor (`daNpc_Kn_HIO_c`, Japanese-labelled) lets you tune at runtime.

Kn-specific fields (offset / label / role):
| Field | Off | JP label | Role in a fight |
|---|---|---|---|
| `attack_wait_time` | 0x8E | 攻撃され待ちタイム | how long he stands open before acting — **lower = less passive** |
| `followup_wait_time` | 0x90 | 追い討ち待ちタイム | pursuit/follow-up delay |
| `small_slide_distance` | 0x94 | 小滑り距離 | minor reposition slide |
| `big_slide_distance` | 0x98 | 大滑り距離 | large reposition slide |
| `warp_distance` | 0x9C | ワープ距離 | teleport distance |
| `attack_disappear_speed_h` | 0xA0 | 攻撃吹っ飛び速度横 | knockback launch speed (horiz) when HE is hit |
| `attack_disappear_speed_v` | 0xA4 | 攻撃吹っ飛び速度縦 | knockback launch speed (vert) |
| `rotation_num` | 0xA8 | 回転分割数 | turn-toward-player steps |
| `rotation_speed` | 0xAA | 回転角速度 | turn rate — **higher = harder to flank** |
| `attack_start_min_time` | 0xAC | (attack-start floor) | **min frames before he initiates — core aggression cadence** |
| `attack_start_time_range` | 0xAE | (attack-start RNG) | random add on top of min (`cLib_getRndValue(min, range)`) |
| `move_speed` | 0xB0 | | approach speed — **higher = pursues faster** |

**Key rebalance levers for a real fight:** lower `attack_start_min_time`/`attack_start_time_range` (attacks far more often), lower `attack_wait_time` (aggressive not patient), raise `move_speed` (closes distance), raise `rotation_speed` (resists side-stepping). Note: in vanilla these are tuned for *survivable lessons* — a boss wants the opposite end of every knob. The DEBUG live editor makes this tuning empirical.

### 13. Player skill inputs (cut-types) + ALBW parry coupling
**Two independent directions of "reading":**

**A. Link → Shade (offensive): the cut-type enum.** `daPy_py_c::CutType` ([include/d/actor/d_a_player.h:616]) classifies which sword move Link used. The Shade reads it via `getCutType()` (stored by `tgHitCallBack`). Hidden-Skill-relevant values:
| Value | Enum | Hidden Skill |
|---|---|---|
| 0x05 | `CUT_TYPE_HEAD_JUMP` | Helm Splitter |
| 0x12-0x14 | `CUT_TYPE_LARGE_JUMP_INIT/_/_FINISH` | Jump Strike |
| 0x17/0x18 | `CUT_TYPE_LARGE_TURN_LEFT/RIGHT` | Great Spin |
| 0x1A/0x1F | `CUT_TYPE_MORTAL_DRAW_A/B` | Mortal Draw |
| 0x1E | `CUT_TYPE_TWIRL` | Back Slice |
| 0x29 | `CUT_TYPE_GUARD_ATTACK` | Shield Attack |
| 0x01-0x04 | `CUT_TYPE_NM_*` | normal swings |
The teach methods already branch on these (e.g. teach03 keys `TWIRL`; teach06 keys `LARGE_JUMP*`). A boss can react per-skill — only Mortal Draw breaks his guard, Back Slice hits his exposed back, etc. — making the fight a literal exam of every skill you learned. Thematically ideal.

**B. Shade → Link (defensive): the ALBW parry READS THE SHADE FOR FREE.**
- The Shade already has a real, active attack collision: `mSphCc[]` set up with `SetAtSpl(dCcG_At_Spl_UNK_1)` (= **0x1**, parryable — NOT a guard-break) + `SetAtType(AT_TYPE_800)` ([d_a_npc_kn.cpp:819-826]); enabled during swings (`OnAtSetBit` ~:1241) and he even self-checks `mSphCc[i].ChkAtShieldHit()` (~:1263).
- The parry hook is **generic and Link-side**: `d_a_alink_damage.inc:708` calls `dShield_onShieldHit(this, at_spl, tghit_ac)` for ANY attacker whose At-collision hits Link's guard, keyed only on `at_spl` + attacker. Guard-break attacks (`at_spl == 9/10/11`) divert to `procGuardBreakInit()` BEFORE the parry path; everything else is parryable.
- The Shade's `at_spl == 1` ⇒ **his sword swings ARE parryable today.** With parry combat enabled (`game.shieldParryCombat`), guarding his attack inside the 4(+1)-frame window (`PARRY_WINDOW_FRAMES`) fires `dShield_onShieldHit` → grants ALBW meter (+1/6), bash charge (+1), durability repair. **No new plumbing on the parry side.**

**What you ADD for a Lies-of-P-style parry feel:** the parry currently rewards the *player* (meter/charge/durability) but doesn't inherently stagger the attacker. Couple a parry-success to a Shade flinch using his existing `daNpcT_DmgStagger_c mDmgStagger` (struct 0x0D3C) — natural trigger point is his own `mSphCc[i].ChkAtShieldHit()` (~:1263), which already tells him "my attack was shield-blocked." That turns a parry into a stagger + punish window.

**Free design levers this unlocks:**
- **Parryable vs unblockable per move:** set a given Shade attack's `at_spl` to 9/10/11 → it becomes a guard-break (forces dodge, not parry). Mix per attack for rhythm.
- **Helm-bash punish tie-in:** the mod's helm-bash credit system (`dAlbwCombat_getHelmBashTier`, Darknut-style) can give the Shade a tier so the Helm Splitter skill opens a punish — another hidden-skill hook into the fight.
- **Settings inheritance:** parry/durability gate on `game.shieldParryCombat` / `dShield_isDurabilityEnabled()`; the fight inherits them automatically.

### 14. Defeat / reward sequence (what `BossLife == 0` triggers)
**Two reference templates exist in-tree; combine them.**

**(a) The Shade's own farewell = the cutscene/exit template.** `ECut_seventhSkillGet` ([d_a_npc_kn.cpp:4797]) is the vanilla "graduation" event cut — an event-manager sequence stepped by `prm` (0/4/6/10/12/15/20/30/40): repositions the player (`setPlayerPosAndAngle` ~:4844), starts farewell BGM (`Z2GetAudioMgr()->bgmStreamPrepare(0x2000038)` / `bgmStreamPlay()` ~:4852), runs farewell dialogue (`initTalk(0x305)`), broadcasts clones to end (`fpcM_Search(s_subEnd7, this)`). NOTE: the skill-grant + completion flags (F_0338–F_0344) are set by the EVENT/BMG SCRIPT that this cut runs, NOT inline C++ — so a custom victory should author its OWN event cut / flag set rather than reuse the skill-granting scripts.

**(b) Real bosses = the death + reward template.** Every TP boss on death does the same three things (e.g. Fyrus [d_a_e_fm.cpp:3363/3758], Diababa `d_a_b_bq`, Morpheel `d_a_b_dr`, Stallord `d_a_b_ds`, Armogohma `d_a_b_gm`, Argorok `d_a_b_ob`):
1. **Reward item:** `fopAcM_createItemForBoss(&pos, dItemNo_UTAWA_HEART_e, room, &angle, ...)` — `UTAWA_HEART` = Heart Container. (Swap the item id for a thematic reward; `fopAcM_createItem` for a plain heart.)
2. **Clear flag:** `dComIfGs_onEventBit(dSv_event_flag_c::saveBitLabels[N])` — sets the boss-defeated/dungeon-clear save bit.
3. **BGM:** `bgmStreamPrepare/Play` fanfare.

**Recommended victory path for the secret boss (on `BossLife` reaching 0):**
- Suppress further damage; broadcast clones to warp out (`fpcM_Search(s_subEnd7/6, this)`).
- Play a collapse/defeat animation + particles (reuse his knockback/`attack_disappear` motions, or a new BCK).
- Set a NEW "secret boss cleared" event flag — mirror the gold-wolf-disappearance pattern (`F_0492`–`F_0497` = "Gold wolf disappearance N") with a fresh unused bit; this also makes the final wolf one-shot (gate its `isDelete()` appear on `F_0344 && !clearedFlag`).
- Optional reward via `fopAcM_createItemForBoss` (Heart Container, or unique item), and/or fire a `src/dusk/achievements.cpp` achievement.
- Run a farewell event cut modeled on `ECut_seventhSkillGet` (BGM `0x2000038` + dialogue + player reposition), then warp the player out (the gold-wolf companion's exit warp, or `dStage_changeScene` back to the overworld).
- **Repeatable vs one-shot:** consume the clear flag (one-shot, canonical) OR leave `F_0344` as the only gate (rematchable). Design choice.

### 15. Clone HP model (shared vs split)
- Clones are **full independent `daNpc_Kn_c` actors** (`fopAcM_createChild` with param-encoded type+`mDivideNo`), each with its own `mSphCc` attack collision and state.
- The master **coordinates them by name-search broadcast**: `s_sub6`/`s_subEnd6` ([d_a_npc_kn.cpp:2363-2379]) (and `s_sub7`/`s_subEnd7` ~:2773) iterate ALL actors and call `setActionMode(18)` (engage) / `setActionMode(17)` (warp-delete) on every `NPC_KN` that isn't self. Clones find the master with `fpcM_SearchByID(parentActorID)`.
- Clones are **ephemeral** in vanilla: spawned for the Jump-Strike / Great-Spin set piece, they attack, then warp-delete on the master's broadcast — they are NOT persistent health pools.
- **Recommended model: MASTER owns the single `BossLife` bar; clones are timed phantom decoys (no independent bar).** Striking a clone can do nothing (pure decoy), dispel it early, or feed a "find the real Shade" beat — only the master's `mLife` moves. The existing broadcast (`s_subEnd*` → mode 17) cleanly dismisses all clones when the special ends or the real one is struck.
- A **split-HP model** (each clone a fraction, kill all to clear the phase) is *possible* since each clone is a real actor that could carry its own `mLife`, but it fights the vanilla ephemeral design (clones leave on a timer/broadcast, not on death) — only pursue if you re-author the divide lifecycle.

## "Can he use ALL his actions in one fight?" — Yes, with caveats
- **Logic is free:** nothing couples the move-methods to lesson order except `selectAction`. A new `mType == 7` selector can call any `teachNN_*` by combat criteria (range/timer/RNG/phase).
- **Teaching tails:** each `*_Wait` interleaves its attack with "wait for correct counter, then advance lesson." Keep the attack-launch + hit-react halves; guard the `mActionMode = next` advances with `if (mType != 7)` (or fork combat variants).
- **Clone-split moves** (teach06/07) are the showpiece boss mechanic and fully reusable; `mDivideNo` controls copy count.
- **Resource union load** is the only non-trivial cost (see §10).

---

## Implementation plan (phased — each phase builds, runs, and is in-game testable; commit at each ✅)

Guiding order: **de-risk infra first, make him damageable, make him winnable, THEN make him fight, then expand, then frame.** Dev-warp to `F_SP200` for all combat iteration; the wolf-summon framing comes last. All new/edited code uses `============` block comment headers + `// === NEW CODE — ALBW Port … ===` banners; build `cmd /c build_run.bat` (RelWithDebInfo); don't commit/push unless asked.

**Phase 0 — Settings toggle + module scaffold.**
- Add `game.heroShadeSecretBoss` setting (off by default) in `dusk/settings.h` + `src/dusk/settings.cpp` + `src/dusk/ui/settings.cpp`, mirroring an existing ALBW toggle.
- New module `src/d/d_albw_shade_boss.cpp` + `include/d/d_albw_shade_boss.h` (`#if TARGET_PC`, file-local `namespace{}`, `dShadeBoss_` externs). Register the TU in `cmake`/`files.cmake`.
- ✅ Test: builds; toggle shows in settings UI; no behavior yet.

**Phase 1 — `mType == 7` actor slot that idles in the arena (RISKIEST INFRA FIRST).**
- Extend `daNpc_Kn_c` for the new type: `getType()` already maps param 0/≥8 → `mType 7`; add `mType == 7` arms to `selectAction()` (→ `wait` for now), `setParam()`, `setCollision()`, `resetType()`, `isDelete()` (appear gate `F_0344`).
- Resource/heap: give `l_loadResPtrn0` (the type-7 pattern) at minimum `{1, -1}` (base `KN_a`); set `heapSize[7]` from `0x0000` to a real value (start ≈ a lesson's `0x4B10`).
- Spawn hook: in `src/d/d_s_room.cpp` (postman/Stargazer pattern), `if (getStartStageName()=="F_SP200" && roomNo==<pick> && isEventBit(F_0344) && setting on) fopAcM_create(fpcNm_NPC_KN_e, param→type7, …)`.
- ✅ Test: save-editor set `F_0344` (`ImGuiSaveEditor`), dev-warp to `F_SP200` (`map_loader` "Wolf Howling Cutscene Map"), confirm he loads, stands, breathes — **no crash** (validates union-load/heap path that everything else depends on).

**Phase 2 — Make him damageable + boss bar.**
- Add `mLife`/`mMaxLife` (new fields) initialized for `mType == 7`.
- Divert `tgHitCallBack` (`d_a_npc_kn_base.inc:6`): when `mType == 7`, decrement `mLife` by the player's hit (optionally weight by `getCutType()`), instead of/in addition to storing cut-type.
- Boss bar: on fight start `dComIfGs_BossLife_public_Set(max)`; each hit update; classify in `dAlbwHP` `sBoss`/`sFinalBoss` so HP-mult + enemy-persistence treat him correctly.
- ✅ Test: hit him → bar drains → at 0 he just freezes (defeat handled next). Validates the full damage→UI loop.

**Phase 3 — Victory / defeat sequence.**
- On `mLife <= 0`: suppress damage, broadcast clones out (`fpcM_Search(s_subEnd7/6)`), play a collapse motion (reuse `attack_disappear` knockback or new BCK), `dComIfGs_onEventBit(<new "shade boss cleared" bit>)`, optional `fopAcM_createItemForBoss(&pos, dItemNo_UTAWA_HEART_e, …)` reward + achievement, fanfare BGM, then warp player out (`dStage_changeScene` / gold-wolf exit).
- Make one-shot: `isDelete()` appear gate → `F_0344 && !clearedBit`.
- ✅ Test: drain bar → full victory → flag set → can't re-trigger (or can, if rematch chosen).

**Phase 4 — Combat AI (the real work): start with ONE move.**
- Replace the `mType == 7` `selectAction` `wait` with a combat selector: pick by range/timer/RNG. Implement the basic attack first (reuse `teach03_attackWait`'s attack-launch half — motion seq + `calcSwordAttackMove(1)` approach — but DROP the lesson tail: no `mActionMode` advance, no `mEvtNo`/`initTalk` dialogue).
- Activate his real `At` collision (`mSphCc[]`, `at_spl == 1`) so the attack deals damage AND is parryable. ALBW parry reads it for free (`dShield_onShieldHit`).
- ✅ Test: he approaches + swings + damages; guarding in the parry window grants a parry. Tune feel via the DEBUG HIO editor.

**Phase 5 — Expand to full moveset + clones.**
- Switch the type-7 resource pattern to the **union load** `{1,2,3,4,5,6,7,8,-1}` (all `KN_tch0ND` arcs; fits the 8-slot `mPhase[8]` cap exactly) and raise `heapSize[7]` to cover it. (If RAM-tight, phase-gate the movepool instead.)
- Add the remaining attacks (Helm-Splitter stance, Mortal-Draw lunge, Jump-Strike + clone split via `fopAcM_createChild`, Great-Spin + clones, energy ball) to the combat selector; clones = master-owned-bar phantom decoys.
- ✅ Test: full attack variety cycles; clone special spawns + warps out; bar still single-owner.

**Phase 6 — Bidirectional skill reactions + balance.**
- Offense: in `tgHitCallBack`, branch on `getCutType()` so specific Hidden Skills do more (Back Slice from behind, Mortal Draw guard-break, etc.).
- Defense: couple parry-success → Shade flinch via `mDmgStagger` at his `mSphCc[i].ChkAtShieldHit()` (~:1263); optionally set some attacks' `at_spl` to 9/10/11 for unblockable variety; helm-bash tier tie-in.
- HIO balance pass (lower `attack_start_*`/`attack_wait_time`, raise `move_speed`/`rotation_speed`).
- ✅ Test: fight feels like a real duel; each skill matters; parry staggers.

**Phase 7 — The final-wolf summon (earn-the-fight framing).**
- Gated final-wolf actor (gwolf clone, Stargazer/postman spawn) appearing on `F_0344 && !clearedBit` in an overworld stage; record live coords (F5 overlay) like the Ook-arena Watcher; warp to `F_SP200` via `dStage_changeScene`/SCLS exit.
- ✅ Test: full loop from overworld — wolf appears → touch → arena → fight → victory → wolf gone.

**Phase 8 — Integration hygiene.**
- Confirm `dAlbwHP` boss classification, enemy-persistence exclusion, settings inheritance (parry/durability), no regressions to vanilla lessons (`mType 0–6` untouched). Patch notes.
- ✅ Final commit / push to `upstream` (when asked).

**Open design picks (defaults chosen above; revisit anytime):** reward = Heart Container (vs unique/none); one-shot (vs rematch); clones = decoys (vs split-HP); all-moves union-load (vs phase-gated pool).

---

## Test Milestone A — gold wolf sits in Link's basement (toggle-gated placement proof)

**STATUS: IMPLEMENTED (uncommitted) 2026-07-02.** Toggle = **`game.heroShadeSecretBoss`**, exposed in the **Dusklight editor → ALBW tab** (right under "Shade's Refuge"), NOT the in-game settings menu — matches the sibling WIP toggles (FA test, WW Item, Shade's Refuge). Placement: `R_SP01` room 7, pos `{35.7236, -1082.5, -602.9236}`, angleY `-14913` (recorded live). Files touched: `include/dusk/settings.h`, `src/dusk/settings.cpp` (init+Register), `src/dusk/ui/editor.cpp` (ALBW tab row), new `src/d/d_albw_shade_boss.{cpp,h}` + `files.cmake`, `src/d/d_s_room.cpp` (include + spawn block beside the Postman/Watcher spawns). **BUILT CLEAN 2026-07-02** (`build_run.bat`, RelWithDebInfo, EXITCODE 0, `v1.4.1-114-dirty`; GPU caches wiped). Awaiting playtest: enable the ALBW-tab toggle, enter Link's House basement (R_SP01 room 7) → wolf should sit + pant. Uncommitted.

**Purpose:** smallest possible first build — validate the settings toggle + `d_s_room` spawn + live-coordinate placement pipeline with ZERO risk, before any actor extension / warp / boss work. Reuses the same runtime-spawn pattern as the Postman and Shade Watcher ([[project-stargazer-mod]]).

**Confirmed facts (this session):**
- **Stage / room:** Link's House = `R_SP01` (layer variant `R_SP01_4`); layer 4 holds rooms `0-4` ([map_loader_definitions.h:109-112]). The basement is one of those — **confirm the exact room# via the F5 Player Info overlay.**
- **Actor + safe param:** `fpcNm_NPC_GWOLF_e` with param **`0x02`** → `getTypeFromParam()` → **`mType 2`**. That type: `isDelete()` returns FALSE (never auto-deletes, [d_a_npc_gwolf.cpp:886]) and `getModeFromParam()` yields `mParamMode = -1` so the howl-warp branch (`mParamMode != -1 && != 0`, ~:1611) is skipped. It is exactly the sitting companion wolf the Hero's Shade spawns as a child (`fopAcM_createChild(fpcNm_NPC_GWOLF_e, …, 0xFFFFFF02, …)`). ⇒ appears, sits in WAITSIT, breathes, **no warp, no self-delete.**
- **Self-loads its arc:** `create()` calls `dComIfG_resLoad(…, "GWolf")` (~:313) — force-loads the `GWolf` archive itself, so it works in `R_SP01` even though the basement stage doesn't normally list it. (Archive-clearance trap risk = LOW for a static spawn.)

**Edits:**
1. **Toggle** — the Phase-0 `game.heroShadeSecretBoss` toggle (settings.h/.cpp, ui/settings.cpp) + module `d_albw_shade_boss.{cpp,h}` with `dShadeBoss_isEnabled()`, registered in `files.cmake`. (This milestone IS Phase 0 + a temporary test spawn.)
2. **Spawn block** in [src/d/d_s_room.cpp] (beside the Postman / Shade-Watcher `#if TARGET_PC` spawns ~:252-275):
```cpp
// === NEW CODE — ALBW Port: Hero's Shade boss — TEMP basement placement test ===
if (dShadeBoss_isEnabled() &&
    strcmp(dComIfGp_getStartStageName(), "R_SP01") == 0 &&
    roomNo == <BASEMENT_ROOM>) {                     // <-- from F5 overlay
    static const cXyz  kPos   = { <X>, <Y>, <Z> };   // <-- from F5 overlay
    static const csXyz kAngle = { 0, (s16)<ANG_Y>, 0 };
    static const cXyz  kScale = { 1.0f, 1.0f, 1.0f };
    fopAcM_create(fpcNm_NPC_GWOLF_e, 0x02 /* mType 2: sits, no warp, no delete */,
                  &kPos, roomNo, &kAngle, &kScale, -1);
}
```
   Include `d/d_albw_shade_boss.h` (+ the gwolf profile header for `fpcNm_NPC_GWOLF_e`).

**Coordinate capture (needed from playtest):** dev-warp to Link's House basement → stand where the wolf should sit → read from the F5 Player Info overlay: **room number**, **position (cXyz)**, **Y-angle (s16)**. Paste into `<BASEMENT_ROOM>` / `kPos` / `kAngle`.

**✅ Milestone A test:** enable the toggle → enter Link's basement → gold wolf sits and breathes at the spot. Toggle off → gone. (No interaction/warp yet — pure placement proof.)

**Note:** this basement spawn is a TEMP scaffold (marked in comments). Later milestones move the wolf to its real overworld gate (`F_0344`) and give it the warp-to-`F_SP200` behavior; the basement block is deleted then.

**Playtest 2026-07-02:** ✅ wolf sits + pants in the basement. Observed: no collision / can't lock on (the `mType 2` companion wolf has no target collision in this standalone context — expected; lock-on/interaction comes with the Milestone-B clone/interactive actor). **Added: BGM cut** — while the toggle is on and the player is in R_SP01 **room 7** (basement), the shared Ordon-interior BGM is silenced every frame via `dShadeBoss_suppressHouseBgm()` → `mDoAud_bgmStop(0)` (called from `dScnRoom_Execute`). Gated to room 7 (NOT all of R_SP01) because R_SP01 is shared by every Ordon house (Bo's/Sera's/Jaggle's/Rusl's) — a stage-wide gate would mute their music too. Only main BGM stops; the wolf's panting SE continues. Built clean, GPU caches wiped, uncommitted.

**Expected Milestone-A result (so it doesn't read as broken):** the `mType 2` wolf **sits in `ANM_WAITSIT` and pants** (`Z2SE_G_WLF_BREATH`, [d_a_npc_gwolf.cpp:994]) and is **lockable** (`attention_info` LOCK/TALK/SPEAK set, ~:566) — but it will **NOT** lunge, warp, or open a dialogue box yet. Those are Milestone B. Pant-and-sit is the correct first result.

---

## Test Milestone B — fight-trigger interaction (LOCKED: hybrid dialogue-confirm + authentic warp)

**Decision (locked with user):** the fight is triggered by a **hybrid** of the two vanilla behaviors — a Shade-Watcher-style native confirm box, then the authentic gold-wolf warp on accept. Best of both: player agency/flavor + the iconic lunge/white-flash into the arena.

**Flow:**
1. Wolf idles + pants (`mType 2` behavior from Milestone A).
2. Link interacts (A / speak range) → **native two-choice message box** (same infra as the Shade Watcher / [[workflow-npc-dialogue]]).
3. **Labels (locked):** `Test Your Will` / `Retreat`.
   **Flavor text (locked):** *"So… you have mastered all I have to teach. The Hero of Time deems you worthy of one final trial."* (shorter variant available: *"The Hero of Time deems you worthy of one final challenge…"*)
4. **"Test Your Will"** → play the authentic howl/lunge/white-flash and warp to the arena — reuse `EVT_ATTACK_WARP` choreography and/or `dStage_changeScene` to `F_SP200` (the same primitive the vanilla questline wolf uses, [d_a_npc_gwolf.cpp:1629]).
   **"Retreat"** → close the box, wolf returns to idle; re-interactable.

**Where the labels/text live:** RUNTIME text injection — **no BMG asset needed.** Confirmed pattern from the Shade Watcher's `talk()` ([d_a_albw_shade_watcher.cpp:1837]): `sFlow.initWord(this, "<descriptive text>", 0xFF, 0, NULL)` → `dMsgObject_setSelectWord(0,"Test Your Will")` + `dMsgObject_setSelectWord(1,"Retreat")` + `dMsgObject_setSelectWordFlag(2)` → poll `doFlow()` → `dMsgObject_getSelectCursorPos()` (0/1) → branch. MUST set select words AFTER initWord (it clears them) and reset `setSelectWordFlag(0)` after capturing the choice (stale select state breaks the next message box, incl. the game-over Retry menu).

**STATUS: Milestone B IMPLEMENTED (uncommitted) 2026-07-02 — BUILT CLEAN (EXITCODE 0, caches wiped).** New actor `daAlbwShadeBossWolf_c` (`d_a_albw_shade_boss_wolf.{cpp,h}`, cloned from the Shade Watcher) registered as `fpcNm_ALBW_SHADE_BOSS_WOLF_e` (0x319) + `fpcDwPi_ALBW_SHADE_BOSS_WOLF_e` + profile-list entry (index-aligned) + `files.cmake`. Refuge coupling stripped (Execute camera block, respawn calls, refuge include). `talk()` = flavor line + "Test Your Will"/"Retreat"; choice 0 → `dComIfGp_setNextStage("F_SP200",0,0,-1)` (plain warp; cinematic lunge/howl is a later polish), choice 1 → close. Basement spawn (`d_s_room.cpp`) swapped from vanilla gwolf → `fpcNm_ALBW_SHADE_BOSS_WOLF_e` param `0x0100`. Gives: sit+pant + **body collision** + **lock-on** + **our dialogue** → warp to F_SP200. Awaiting playtest. NOTE F_SP200 has no Shade yet (Phase 1) — "Test Your Will" currently just proves the transition (may spawn oddly / empty arena).

### Milestone B build plan — clone the Shade Watcher into a dedicated actor
The mod's **Shade Watcher** (`daAlbwShadeWatcher_c`, `d_a_albw_shade_watcher.{cpp,h}`) is a gold-wolf clone that already has EVERYTHING the summon needs: `dCcD_Cyl mCyl` body collision, attention/lock-on (`setAttnPos`/`lookat`, TALK/SPEAK ranges), the runtime-text `talk()` dialogue above, and `ECut_attackWarp` (the lunge/white-flash warp). It self-loads its `GWolf` arc. It differs from our need only in: dialogue text, on-confirm action (Refuge does rest/heal + `dShadeRefuge_setRespawn`; we want warp to `F_SP200`), and a Refuge-specific `Execute()` camera block.

**Clone steps (new actor `daAlbwShadeBossWolf_c`):**
1. Copy `d_a_albw_shade_watcher.{cpp,h}` → `d_a_albw_shade_boss_wolf.{cpp,h}`; rename symbols (`daAlbwShadeWatcher`→`daAlbwShadeBossWolf`, include guard, HIO macro, `g_profile_ALBW_SHADE_WATCHER`→`..._BOSS_WOLF`, method table).
2. **Registration (3 index-aligned tables):** append `fpcNm_ALBW_SHADE_BOSS_WOLF_e` to [f_pc_name.h] (next = 0x319, after SHADE_WATCHER 0x318); append `fpcDwPi_ALBW_SHADE_BOSS_WOLF_e` to [f_pc_draw_priority.h]; append `&g_profile_ALBW_SHADE_BOSS_WOLF.base.base,` to the END of the profile array in [f_pc_profile_lst.cpp] (indexed DIRECTLY by fpcNm — order MUST stay in lockstep). Add the TU to `files.cmake`.
3. **Strip Refuge coupling:** remove the `Execute()` `dShadeRefuge_consumeRespawnCamera()` block (→ plain `execute()`), the `#include "d/d_albw_shade_refuge.h"`, and the respawn calls in `talk()`.
4. **Our dialogue + warp:** in `talk()`, swap the text to the flavor line + "Test Your Will"/"Retreat"; on choice 0 → warp to `F_SP200` (reuse `ECut_attackWarp` choreography or `dStage_changeScene`); choice 1 → close, return to idle.
5. **Spawn swap:** in `d_s_room.cpp`, change the basement spawn from `fpcNm_NPC_GWOLF_e` (param 0x02) to `fpcNm_ALBW_SHADE_BOSS_WOLF_e` (param that selects the sit+talk `waitBrave`/type-2 idle). Keep the same coords/gate.
6. Build + playtest: wolf sits+pants+**collision**+**lock-on**, interact → **our dialogue box**; "Test Your Will" → warp to arena (lands in F_SP200 where the Phase-1 Shade will later wait).

**Behavior source map (for build):**
- The **two-choice box** is NOT native gwolf behavior — it's added (Shade Watcher clone pattern): `mMsgFlow.init(...)` → poll `doFlow(...)` → branch on `dMsgObject_getSelectCursorPos()` (0/1). Template: Kargarok `d_a_kago.cpp:244-271`.
- The **warp** IS native gwolf behavior: `EVT_ATTACK_WARP` (mOrderEvtNo) lunge/howl + `dStage_changeScene`. For our custom wolf we drive it on the "Test Your Will" branch and target `F_SP200`.
- **Dependency:** native message-box infra ([[workflow-npc-dialogue]]) — real but already-scoped work. Warp target `F_SP200` + arena spawn come from Phase 1 (the `mType == 7` Shade already waiting there).

**Sequencing:** Milestone B comes AFTER Milestone A (placement) and the Phase-1 arena Shade exists (so the warp lands somewhere real). Until the arena Shade is in, "Test Your Will" can temporarily just warp to `F_SP200` and confirm the transition.

---

## Phase 0 — file-level detail (settings + module scaffold)

**Goal:** a settings toggle and an empty module, both compiled in. Zero behavior. Risk: none — pure scaffolding to verify the build wiring before touching the actor.

**0.1 — Add the toggle to `UserSettings.game` ([include/dusk/settings.h] ~:181-194, beside `bossRefinement`):**
```cpp
// Hero's Shade secret boss: post-game real fight after all Hidden Skills (F_0344).
ConfigVar<bool> heroShadeSecretBoss;
```

**0.2 — Default + register ([src/dusk/settings.cpp]):**
- Init list (~:45-52, beside `.bossRefinement {"game.bossRefinement", false}`): `.heroShadeSecretBoss {"game.heroShadeSecretBoss", false},`  ← **default OFF** (new content, opt-in).
- `Register(...)` block (~:273-280): `Register(g_userSettings.game.heroShadeSecretBoss);`

**0.3 — UI row ([src/dusk/ui/settings.cpp] ~:1419, beside "Boss Refinement"):**
```cpp
addOption("Hero's Shade Secret Boss", getSettings().game.heroShadeSecretBoss,
          "After all Hidden Skills, a final Hero's Shade duel with a real health bar.");
```

**0.4 — New module + CMake:**
- `include/d/d_albw_shade_boss.h` + `src/d/d_albw_shade_boss.cpp` — `#if TARGET_PC`, file-local `namespace {}`, `dShadeBoss_` externs, `============` headers. Pattern-match `d_albw_combat.cpp`. Start with one accessor: `bool dShadeBoss_isEnabled() { return dusk::getSettings().game.heroShadeSecretBoss.getValue(); }` (mirror `dAlbw_isHiddenSkillReworkEnabled()`).
- Register the TU in [files.cmake] (~:185, beside `src/d/d_albw_boss.cpp`): `src/d/d_albw_shade_boss.cpp`.

**✅ Phase 0 test:** `cmd /c build_run.bat`; open Settings → the new toggle appears, persists across launches; no behavior change.

---

## Phase 1 — file-level detail (`mType == 7` idle Shade in `F_SP200`)

**Goal:** with `F_0344` set, dev-warp to `F_SP200` and the Shade loads, stands, breathes — **no crash.** Risk: HIGH — this validates the resource-pattern + `heapSize[7]` path that every later phase depends on. Do this before anything combat-related.

**1.1 — Resource pattern + heap for type 7 ([src/d/actor/d_a_npc_kn.cpp]):**
- `l_loadResPtrn0` (~:290) currently `{1, -1}` (base `KN_a` only) — KEEP for Phase 1 (idle needs no move arcs). Note for Phase 5: this becomes the union `{1,2,3,4,5,6,7,8,-1}`.
- `heapSize[8]` (~:559): change index `[7]` from `0x0000` to a real budget — start at `0x4B10` (a lesson's size). **Critical:** `0x0000` would fail `fopAcM_entrySolidHeap` → no actor. This is the single most likely Phase-1 crash cause.

**1.2 — Type-7 arms in the per-type switches (all in `d_a_npc_kn.cpp`):**
- `getType()` (~:749): already returns 7 for param low byte 0/≥8 — **no change**.
- `selectAction()` (~:1453 switch): add `case 7: mpTeachAction = &daNpc_Kn_c::wait; break;` (the `default` already does this — so optional, but make it explicit for clarity).
- `setParam()` (~:917), `setCollision()` (~:1170 switch), `resetType()` (~:879 switch): add a `case 7:` that mirrors the lightest existing lesson (`mActionMode = <idle>`, a basic `setCollisionTeach01()`/no-op). Goal is "stands with a sane collision," not combat.
- `isDelete()` (~:785): add
```cpp
case 7:
    if (dShadeBoss_isEnabled() && daNpcT_chkEvtBit(344) /* F_0344 */) return 0; // stay
    return 1; // delete
```
(`daNpcT_chkEvtBit(344)` = F_0344 — same flag-number convention as gwolf's `l_warpAppearFlag` `0x01D8`=F_0472. Phase 3 extends this with `&& !clearedBit`.)

**1.3 — Spawn hook ([src/d/d_s_room.cpp], in the existing `#if TARGET_PC` block beside the Postman/Shade-Watcher spawns ~:252-275):**
```cpp
// === NEW CODE — ALBW Port: Hero's Shade secret boss ===
if (dShadeBoss_isEnabled() &&
    strcmp(dComIfGp_getStartStageName(), "F_SP200") == 0 &&
    roomNo == <ROOM> &&                                   // pick an F_SP200 room (0-7); confirm via F5 overlay
    dComIfGs_isEventBit(dSv_event_flag_c::saveBitLabels[344])) {   // F_0344 (index==number, cf. postman [625]=F_0625)
    static const cXyz  kPos   = { /* record live via F5 overlay */ };
    static const csXyz kAngle = { 0, (s16)0, 0 };
    static const cXyz  kScale = { 1.0f, 1.0f, 1.0f };
    fopAcM_create(fpcNm_NPC_KN_e, (0x00u << 8) | 0x08u,   // low byte 8 -> mType 7; high byte 0 -> mDivideNo 0 (master)
                  &kPos, roomNo, &kAngle, &kScale, -1);
}
```
Include `d/d_albw_shade_boss.h`. Param `0x0008`: low byte 8 → `getType()` default → `mType 7`; high byte 0 → `getDivideNo()` 0 → master (not a clone).

**1.4 — Coordinate capture (same method as the Ook-arena Watcher):** dev-warp to `F_SP200` ("Wolf Howling Cutscene Map", [map_loader]), stand where the Shade should be, read Link's pos/angle off the **F5 Player Info overlay**, paste into `kPos`/`kAngle`. Pick the room with the cleanest open geometry for a duel.

**✅ Phase 1 test:**
1. `ImGuiSaveEditor` → set `F_0344` (Obtained 7 secret techniques).
2. Enable the Phase-0 toggle.
3. Dev-warp to `F_SP200` room `<ROOM>`.
4. Confirm: Shade loads, stands in `ANM_WAITSIT`-style idle, breathes, **no crash / no heap assert**. Toggle off or clear `F_0344` → he does not appear.

**Phase 1 gotchas:**
- `heapSize[7] == 0x0000` → silent no-spawn. Set it first.
- Archive-clearance crash trap (cf. [[workflow-npc-dialogue]]): `F_SP200` already loads `KN_*` arcs for the vanilla lessons, so the arcs should be resident — but confirm the room actually streams them; if not, the resLoad will stall. A standing test surfaces this immediately.
- Spawn fires on room LOAD only — re-enter the room (or warp) after setting the flag; it won't pop in mid-room.

## Open questions / next steps
- Exact source of `mType` request for the *final wolf's* warp (param wiring vs event) — partially answered (spawn param byte), confirm end to end at build time.
- Whether `F_SP200` rooms are interchangeable or per-skill specialized geometry — pick a clean room for the duel.
- One-shot vs repeatable: consume `F_0344`→a new flag (mirror `F_0492`-style disappearance) or allow rematch.
- Combat tuning: `mLife` value, HP-mult interaction (boss category), attack damage, phase thresholds for clone moves.
- Conventions: new/modified code uses `============` block comment headers (ALBW port rule); build via `build_run.bat` (RelWithDebInfo); push to `upstream` (ALBW-Dusklight); don't commit unless asked.
