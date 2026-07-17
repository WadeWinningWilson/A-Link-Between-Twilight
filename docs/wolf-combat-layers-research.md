# Wolf Link Combat — New Layers Research (research pass, no code)

**Goal:** deepen Wolf Link combat (currently just the Midna field-attack *charge* economy) to match the human-Link depth added since (parry, Focused Arts, finishers, Flurry Rush). Three proposed layers:

1. **Wolf contributes Focused Arts (FA) charges** at **1.5×** the human rate.
2. **A third transform layer** — the giant Fused-Shadow Midna (the one that smashes the Ganon barrier at Hyrule Castle) as a playable "giant stomp / AOE blast" form.
3. **A Lies-of-P-style health-regain** — a hit "fades" a portion of Wolf Link's HP into a recoverable pool; biting back restores a % of it; a *second* hit before recovery burns the whole pending pool.

Verdict up front: **Layer 1 is easy and low-risk. Layer 3 is medium and mostly-clean. Layer 2 is blocked on a missing asset** (engine plumbing is fine; the giant model doesn't exist and must be authored/imported).

---

## ★ REVISION (2026-07-11) — final plan: wolf becomes a full FA earn+spend fighter, + 3 Midna arts

Design refinements from the second research round. The unifying theme: **all three new wolf attacks are Midna's shadow powers, channeled through wolf Link and paid for with Focused Arts charge.** Wolf now both **earns** FA (making it, and human, feel like one economy) and **spends** it on three "arts." The 1.5× multiplier moves off FA and onto **enemy-death rupees**.

### R0. Two economies, revised roles (user, 2026-07-11)
- **Wolf charge economy** (`mWolfChargeCount` 0–2, earned per 5 bites) → **spends on the three new arts** (howl / arm / giant) **AND** the existing Midna field/lock attack. **The arts consume WOLF charges, not the FA meter.** This means the existing wolf-charge spend guard ([d_a_alink_wolf.inc:7537-7559](../src/d/actor/d_a_alink_wolf.inc)) is the *exact* spend mechanism — no new FA-spend hook needed.
- **Focused Arts charge** (`s_bankCount`, tier 1–3) → wolf **EARNS** it (contributes to the shared FA economy that *human* Link spends); wolf does **not** spend FA. So FA is a one-way "wolf feeds the meter" link; the arts are paid for with wolf charges.
- **Charge economy REDESIGN (user, 2026-07-11 — tentative, may revert to FA later so keep the spend abstracted):**
  - **Cap is a SAVE-BACKED, shop-upgradeable value: starts at 1, upgrades to 2 then 3 (max).** (Currently the cap is a hard-coded 2 — `WOLF_CHARGE_MAX` + `< 2` checks — so this becomes a purchased upgrade.)
  - **DECOUPLED "charges needed to use" vs "charges spent" (user, 2026-07-11):**
    | Move | D-pad | Needed to use | Spent |
    |---|---|---|---|
    | Howl AOE | Up | 1 | 1 |
    | Midna punch (quick extend→punch→retract) | Right | **2** | **1** |
    | Giant AOE | Down | 3 (= max cap) | 3 |
  - **Earn: +1 per 5 normal bites, bite-only (unchanged rate; no kill/dodge sources; does NOT scale with cap).** Arts do NOT feed charge — only normal bites do (existing earn logic already isolates `HIT_TYPE_LINK_NORMAL_ATTACK`, excluding the Midna-lock type).
  - **UNLOCK PROGRESSION (user, 2026-07-11) — supersedes the "buy generic cap upgrades" idea. Each art is unlocked by a SHOP purchase gated behind a STORY MILESTONE; unlocking grants the move AND the charges to use it (howl→1, punch→2, giant→3):**
    - **Start:** only the EXISTING Midna lunge (the "current charge attack") — the early game teaches it.
    - **Howl** → shop unlock after **Eldin/Kakariko Twilight cleared**.
    - **Midna punch** → shop unlock after **Lanayru Twilight cleared**.
    - **Giant** → shop unlock after the **first Mirror Shard obtained**.
    - Shop-unlock shape mirrors the FA *tier* purchase (save event reg + shop row, [d_focused_arts.cpp:224-229, 402-412](../src/d/d_focused_arts.cpp)); the live charge COUNT stays session-scoped as today.
  - **IMPL RESEARCH ITEM (when wiring):** find the TP save/event flags for "Eldin Twilight cleared", "Lanayru Twilight cleared", "first Mirror Shard obtained" to gate the shop-unlock availability.
  - **EXISTING Midna lunge KEPT (user: do NOT fold).** The current charge-spender — the `AT_TYPE_MIDNA_LOCK` lunging pounce on the normal lock-attack ([procWolfLockAttack, d_a_alink_wolf.inc:8424/8489](../src/d/actor/d_a_alink_wolf.inc)) — stays as-is on the attack/lock input. The new D-pad-Right "punch" is a SEPARATE, snappier Midna-arm move. So two Midna-arm moves coexist on the shared pool (intended). **Open (minor, defer to impl):** leave the lunge's current "open-at-2 then chain-to-0" behavior, or normalize it to the need/spend model. **Still-open user ruling:** keep both vs. have the new punch replace the old lunge (user chose keep for now).
  - **HUD density note:** wolf form now stacks health bar + faded-HP segment (Layer 3) + earn-only FA meter + 1–3 charge pips — consider pairing the charge pips visually with the FA meter so it reads as 2 clusters, not 4 elements.
  - **Reversibility:** keep the spend behind one "can-afford / spend N" seam so a later switch of the arts' currency (wolf charge ↔ FA bank) is localized (user may revert to FA).

### R1. Wolf earns FA charge (rate un-multiplied now) — seam unchanged
As in Layer 1 below, but **drop the 1.5×** (it moves to rupees). Add `dFocusedArts_onConnectedWolfHit()` routing through `addFillSteps` at a flat wolf rate (tunable `kWolfFillStep`, e.g. the common 2/12), branched at the already-forked sword-hit site [d_a_alink_cut.inc:353](../src/d/actor/d_a_alink_cut.inc). Show the meter in wolf form (relax `lopFaMeterActive()`), **placed under the wolf health bar** in the LoP HUD (user's layout call — pairs with the Layer-3 faded-HP bar).

### R2. Wolf 1.5× on ENEMY-DEATH RUPEES — clean one-line seam
Every kill (sword, wolf bite, Ending Blow) funnels through `dAlbwEnemyRupees_onEnemyKill()` ([d_albw_enemy_rupee.cpp:346](../src/d/d_albw_enemy_rupee.cpp)); `daPy_py_c::checkNowWolf()` is in scope there. Branch: `if (checkNowWolf()) amount = amount * 3 / 2;` right after `lookupKillRupees()`. Wallet-cap is auto-clamped in `moveRupee()` ([d_meter2.cpp:2567](../src/d/d_meter2.cpp)); `biggerWallets` raises the cap transparently; bosses/beamos excluded (they credit via a different table). Gated by `game.enemyDeathRupees`. Trade-off: the global-form check also boosts a rare non-wolf-source kill while in wolf form (friendly-fire/hazard) — acceptable; per-hit `AT_TYPE_WOLF_*` precision is available but not worth the plumbing.

### R3. FA-spend seam for the arts (shared by all three)
- **Control scheme (user, 2026-07-11): the three arts live on the D-pad in wolf form —**
  - **D-pad Up = Howl AOE** (damage comparable to **Great Spin**).
  - **D-pad Right = Midna extended-arm strike** (damage comparable to **Helm Splitter**).
  - **D-pad Down = Giant AOE ultimate** (damage comparable to **Mortal Draw**).
  - ⚠️ **Conflict check pending** — the D-pad already hosts the quick-swap outfit cycle / quick-transform / item-slot system; whether these bind cleanly in wolf form (and how to deconflict) is a dedicated research item (see the "D-pad conflict analysis" section appended below).
- **Input dispatch:** `checkWolfAttackAction()` ([d_a_alink_wolf.inc:7361](../src/d/actor/d_a_alink_wolf.inc)) is the attack router — but the D-pad may be read by the quick-swap layer *before* the attack router; the conflict research determines the correct hook. Add branches → each art's `procWolf<Art>Init()`, gated on FA charge.
- **Spend (REVISED — wolf charges, not FA):** the arts consume `mWolfChargeCount` via the **existing wolf-charge guard** at [d_a_alink_wolf.inc:7537-7559](../src/d/actor/d_a_alink_wolf.inc) — gate on `mWolfChargeCount >= cost`, decrement, else deny SFX + `dAlbwWolfChargeHud_notifyDeny()` and fall through. No new FA-spend hook. (See R0's cap ripple — 4 consumers on a cap-2 pool needs the cap raised and/or the field attack folded into the arm art.)
- **Suggested cost profile:** Howl = **1**, Extended Arm = **2**, Giant = **3** — requires raising the wolf-charge cap to ≥3 (currently 2). Tunable consts.
- **Scope (user):** the arts are enabled **only when both quick-swap mode (`ExtraAndQuickSwap`) AND the FA/wolf-combat feature are on** — see R7 guard 2 (this deliberately scopes them to quick-swap users and removes the ring/map-suppression work).

### R4. Art 1 — Howl AOE (feasibility: HIGH) — **FIRST TO BUILD; full recipe below**
Fully mapped (anim + ~5s duration + howl music + Great Spin VFX loop + AOE collider). This is the trickiest art but every piece has a proven template.

- **Anim + ~5s duration:** `WANM_HOWL` (standing "howl in place") → `dRes_ID_ALANM_BCK_WL_HOWLA_e` ([d_a_alink.cpp:975](../src/d/actor/d_a_alink.cpp)); proc template `procWolfHowlInit`/`procWolfHowl` ([d_a_alink_wolf.inc:3481/3505](../src/d/actor/d_a_alink_wolf.inc)). Build a new `procWolfHowlBurst` off the standing (`param_0==0`) branch. The move lasts **~5 s** — hold it via the proc's existing sustain-loop (frames 38-56 rewind) or a fixed ~300-frame counter, ending on `checkAnmEnd(mUnderFrameCtrl)`.
- **HOWL MUSIC (plays during the anim; other audio cuts off — nearly free):** the howl *melodies* are real BGM IDs `Z2BGM_HOWL_*` ([Z2SeqMgr.h:331-376](../include/Z2AudioLib/Z2SeqMgr.h)), e.g. `Z2BGM_HOWL_ZELDASONG = 0x1000054` (NOT the `Z2SE_WL_V_HOWL` bark). Play one on proc init via `mDoAud_subBgmStart(id)` ([m_Do_audio.h:80](../include/m_Do/m_Do_audio.h)). These route through the **fanfare path** (`Z2SeqMgr::fanfareFramework`, [Z2SeqMgr.cpp:1349-1379](../src/Z2AudioLib/Z2SeqMgr.cpp)): starting one **auto-ducks the field BGM** (`mFanfareMute.fadeOut(30)`) and **auto-restores it** when it ends — so "any audio cuts off, then comes back" is built in. Stop early / on anim-end with `Z2GetSeqMgr()->stopWolfHowlSong()` ([Z2SeqMgr.cpp:1382](../src/Z2AudioLib/Z2SeqMgr.cpp)). The melodies are already resident (vanilla howling minigame uses them) → **no arc load/erase needed**. (Precedent for a *custom* tune: the ALBW Postman `ALBW_POST_SFX` loadSeWave/subBgmStart lifecycle in [d_a_npc_post.cpp](../src/d/actor/d_a_npc_post.cpp) — [[workflow-npc-sounds]].) Optionally still fire the `Z2SE_WL_V_HOWL` bark layer.
- **GREAT SPIN VFX, LOOPED (reuse the Hurricane pattern):** the Great Spin ring is the **`ID_ZI_J_KAITENGIRID_A..F`** ("Large Turn" kaitengiri) 6-emitter bank ([d_a_alink_effect.inc:1017-1024](../src/d/actor/d_a_alink_effect.inc)), stored in `field_0x3204[6]`, created via `setEmitter → dComIfGp_particle_set` attached to a Link anim-matrix joint. **Hurricane Spin is the looped template** ([d_a_alink_hurricane.inc:120-352](../src/d/actor/d_a_alink_hurricane.inc)): `setEffect()` pumps `setCutTurnEffect()` every frame, and the single control bool **`gsSustainedVfx`** ([d_a_alink_effect.inc:956](../src/d/actor/d_a_alink_effect.inc)) makes it **skip the one-shot frame gate, respawn any emitter whose particles ran out, and `becomeImmortalEmitter()` + `playCreateParticle()`** — so the ring never stops for the spin's duration; cleanup on proc exit via `clearCutTurnEffectID()` ([d_a_alink.cpp:15922](../src/d/actor/d_a_alink.cpp)). **To reuse for the howl: OR your new howl proc-id into the `gsSustainedVfx` condition** (exactly how `hurricaneProc` and the EB-AOE `ebGreatSpinAoe` are OR'd today), and drive it for the ~5s window. (Contrast — **Hero's Shade** fires the same VFX *un-looped*: raw `dComIfGp_particle_set(ID_ZI_J_KAITENGIRID_A/_B)` fire-and-forget, no stored id, no re-emit, [d_a_npc_kn.cpp:1924-1939](../src/d/actor/d_a_npc_kn.cpp) — the baseline you're *not* copying.)
- **VFX ⟂ DAMAGE — decoupled (confirmed):** the KAITENGIRI emitters are purely cosmetic; they register no collision. Hurricane/EB-AOE both drive damage from a separate collider + a Focused-Arts power value, and Hero's Shade's VFX has no collider at all. So the howl's **AOE collider is fully independent of the VFX**.
- **AOE collider:** the wolf **roll attack is already a Link-centered radial burst** — `mAtCyl.SetC(current.pos)` ([d_a_alink.cpp:6591](../src/d/actor/d_a_alink.cpp)) with growable radius via `cLib_chaseF(mAtCyl.GetRP(), target, rate)`. Set it with `setCylAtParam(AT_TYPE_WOLF_CUT_TURN, …, dCcD_SE_WOLF_BITE, power, bigRadius, tallHeight)` ([d_a_alink_cut.inc:223](../src/d/actor/d_a_alink_cut.inc)); arm with `onResetFlg0(RFLG0_UNK_2)` during the active window(s). **`AT_TYPE_WOLF_CUT_TURN` is already accepted by every enemy** — no per-enemy edits. *Caveat:* `setWolfAtCollision` ([d_a_alink.cpp:6590](../src/d/actor/d_a_alink.cpp)) special-cases `PROC_WOLF_ROLL_ATTACK` for the Link-centered `SetC`; add the new proc id to that `if`. **Damage target = Great Spin** (per the control-scheme mapping). Bonus: the howl's `dKy_Sound_set` ping already alerts/aggros. Over ~5s, pulse the collider (re-arm periodically) rather than one hit, so the sustained howl deals repeated AOE ticks.

### R5. Art 2 — Midna extended arm (feasibility: HIGH — model already present!)
- **The hair-hand is ALREADY on wolf Link:** `changeWolf()` loads it from arc `Wmdl` idx 11 (`BMD_MD_HAIR_HAND`) into `mpWlMidnaHairModel` ([d_a_alink_wolf.inc:179](../src/d/actor/d_a_alink_wolf.inc)); accessor `getMidnaHairHandModel()`. No new asset.
- **It's already aimable:** `daMidna_c::setBodyPartMatrix()` ([d_a_midna.cpp:972](../src/d/actor/d_a_midna.cpp)) poses the hair-hand off `JNT_HAIR_5` and has a **"big hand aimed at target" shape** (material node 2) it shows when `getWolfLockActorEnd() != NULL` ([d_a_midna.cpp:1013-1019](../src/d/actor/d_a_midna.cpp)). There's also a general "Midna hair reaches toward an actor" mechanic (`FLG1_MIDNA_HAIR_ATN_POS` / `getMidnaHairAtnPos()`) already used vs. Beast Ganon/chains.
- **Best build = a variant of the existing lock attack:** `procWolfLockAttackInit/Move` ([d_a_alink_wolf.inc:8424/8489](../src/d/actor/d_a_alink_wolf.inc)) already lunges at a Z-locked target with `AT_TYPE_MIDNA_LOCK` and drives the aimed hair-hand. The new art = reuse `getWolfLockActorEnd()`/`mWolfLockAcKeep` + `AT_TYPE_MIDNA_LOCK`, but instead of the ballistic pounce, **extend the hair-hand outward toward the target and run the collider along the hair-hand vector** (a reach-and-strike, not a body-lunge). The aim math + "big hand shown" state already exist.
- **Avoid depending on:** the `ANM_MGN*` catch/throw anims (boss-gated via `mMidnaAnm==10/11/12`) and the demo-arc high-detail hair-hand override — you can imitate the look with the persistent model + a new Midna reach pose. Zant's kill is pure cutscene (no reusable arm asset).

### R6. Art 3 — Giant Midna ultimate (FA-powered; up-scaled Midna is the TEST scaffold, the real giant model is the END GOAL)
- **Clarification (user, 2026-07-11): up-scaled Midna is only the TEST/prototype approach — the actual giant Fused-Shadow model IS the final product.** So plan for two phases: (a) **prototype** on an up-scaled existing Midna body to prove the ultimate's proc/stomp/collision, then (b) **final** = author/import the real giant rigged model (content pass; BMD/Blender pipeline) and swap it in behind the same proc.
- Same asset reality as Layer 2 below: **no giant arc exists yet.** Since Midna's rigged body already rides the wolf (idx 10, full 29-joint skeleton) and scaling is a one-line `setBaseScale`, the prototype **up-scales the existing Midna body** for a brief FA ultimate: spend a full FA bank → Midna grows huge → stomp/AOE-blast → revert. Validates the art before the giant model exists; the final swaps the placeholder for the real model.
- **Framed as an FA art, it does NOT need the binary-`checkWolf()` refactor** that a permanent third *transform* would (Layer 2's real cost). A temporary FA ultimate can be a **timed attack proc** (like the howl) that swaps to an up-scaled model for its duration and restores after — staying within the wolf state — rather than a new persistent transform flag threaded through hundreds of `!checkWolf()` sites. **This is a major simplification** vs. the original Layer-2 framing: prefer the timed-ultimate approach for v1; a true persistent giant transform stays a later, larger project.
- Stomp AOE reuses R4's Link-centered radial collider pattern (scaled up). Enlarged collision is hand-authored (not scale-derived). Nearest precedent: `procWolfGiantPuzzleInit()` ([d_a_alink_wolf.inc:1759](../src/d/actor/d_a_alink_wolf.inc)) (giant-shove puzzle state).

### Revised sequencing
1. **R2 (wolf 1.5× rupees)** — one-line seam, trivial, ship first.
2. **R1 (wolf earns FA) + meter-under-health-bar** — small, enables the arts' economy.
3. **R4 (Howl AOE)** — first art; proves the FA-spend + wolf-AOE recipe end to end.
4. **R5 (Extended arm)** — high payoff, model already present, reuses lock-attack.
5. **R6 (Giant ultimate)** — up-scaled-Midna timed ultimate (avoids the transform refactor). Prototype last.
6. **Layer 3 (health-regain)** — slot per appetite; independent of the arts, reuses the bite-heal/damage seams, FA bar sits under the health bar alongside it.

### R7. D-pad conflict analysis (Up=howl / Right=arm / Down=giant) — **VERDICT: SAFE, with guards**

**The mapping works** because the D-pad's quick-swap cycles are *human-only*: all three cycle handlers hard early-return on `checkWolf()`, so in wolf form **Up/Right/Down are physically reserved but no-op — effectively free.** And critically, **transform-BACK (wolf→human) is on R+Y and D-pad Left-tap — NOT Up/Right/Down** — so an art press can never also transform you. **Do NOT bind an art to Left** (it's the transform-back + Call-Midna input, fully live in wolf form).

Central dispatch is a linear sequence in `duskExecute()` ([f_ap_game.cpp:765-799](../src/f_ap/f_ap_game.cpp)); default binds (`applyDpadQuickSwapPresetBinds`, [dpad_quick_swap.cpp:26](../src/dusk/dpad_quick_swap.cpp)): Up=`CYCLE_SWORD`, Right=`CYCLE_SHIELD`, Down=`CYCLE_OUTFIT`, Left=layered tap.

| Dir | Human form | Wolf form (ExtraAndQuickSwap mode) | Proposed art |
|-----|-----------|-----------|--------------|
| **Up** | cycleNextSword (ring suppressed) | **free** — ring suppressed, cycle no-ops on wolf ([dpad_quick_swap.cpp:104](../src/dusk/dpad_quick_swap.cpp)) | Howl AOE ✅ |
| **Right** | cycleNextShield (map suppressed) | **free** — cycle no-ops on wolf (:155) | Extended arm ✅ |
| **Down** | cycleNextOutfit (ring suppressed) | **free** — cycle no-ops on wolf (:206) | Giant AOE ✅ (highest-risk slot, see guards) |
| **Left** | tap→wolf / dbl-tap Midna | **transform→human + Call Midna (LIVE)** | ❌ leave alone |

**Guards required (deconfliction):**
1. **Gate every new art reader on `checkWolf()`** so it's mutually exclusive with the human-only cycle handlers (they already bail on wolf; this closes the other side).
2. **Mode coverage — RESOLVED (user decision):** the arts are enabled **only in `ExtraAndQuickSwap` mode (+ FA/wolf-combat on)**. In that mode Up/Right/Down are *already* reserved-and-suppressed for the vanilla ring/map, so **no extra suppression work is needed** and the earlier `dMw_*_TRIGGER` wolf-suppression hack is unnecessary. Tradeoff accepted: players in Off/Extra modes don't get the arts (deliberate — these are advanced quick-swap-tier moves). This is the opposite scoping choice from the *outfit* system (which had to serve non-quick-swap players); here it's fine because the arts are a power-user layer.
3. **Down specifically:** also require `!quickTransformBoundToDpadDown(0)` (legacy configs could put `QUICK_TRANSFORM` on Down → a raw-Down art reader would fire the art *and* the transform on the same edge). Confirm `CYCLE_OUTFIT` (not `QUICK_TRANSFORM`) owns Down.
4. **Dispatch ordering + edge-consume:** insert the art readers into the `duskExecute` sequence (~777-799) with explicit order vs `handleQuickTransform`, and clear `mPressedButtonFlags` after consuming (as `handleWolfHowl`/`handleQuickTransform` already do at [d_a_alink_dusk.cpp:40,106](../src/d/actor/d_a_alink_dusk.cpp)) so the same edge doesn't leak to the menu-window layer. All readers are edge-latched `getTrig`, so without a flag-clear two readers can see the same press.
5. **Note the existing R+X wolf howl** (Suns Song / warp, [d_a_alink_dusk.cpp:11](../src/d/actor/d_a_alink_dusk.cpp)) — the new *plain-Up* combat howl is a distinct input (modifier vs no-modifier), no collision, but you'll have two "howl" bindings (warp-howl on R+X, combat-howl AOE on Up); intentional, just document it.

**Net:** no hard conflict with quick-swapping — the arts slot into the three wolf-form-free directions, and transform-back stays on Left/R+Y. The only real work is (guard 2) suppressing the vanilla ring/map on Up/Right/Down in wolf form so the feature isn't silently limited to quick-swap users, plus the standard wolf-gate + Down legacy-config guard + edge-consume ordering.

---

*Everything below is the original (first-round) research; R-sections above supersede Layer 1's multiplier (now rupees) and Layer 2's framing (now a timed FA ultimate).*

---

## Current state — what Wolf Link combat is today

Gated by `game.wolfLinkCombat` (`dAlbwWolfCombat_isEnabled()`, [d_albw_wolf_stun.cpp:469](../src/d/d_albw_wolf_stun.cpp)) **and** `checkNowWolf()`. Three subsystems:

- **Charge economy** — actor fields on `daAlink_c` ([d_a_alink.h:4627-4629](../include/d/actor/d_a_alink.h)): `mWolfChargeCount` (0–2, spendable), `mWolfBiteCount` (resets at 5), `mWolfSpendChainActive`. **Session-only** (plain fields, reset on construction/respawn; no save bit). Earn = 5 normal bites → +1 charge; spend = the Midna field/lock attack (2 to open, chains to 0). Earn/heal/stun all centralized in `cc_at_check` ([d_cc_uty.cpp:646-697](../src/d/d_cc_uty.cpp)); spend is **duplicated** across [d_a_alink_wolf.inc:7519-7560 + :7586-7597](../src/d/actor/d_a_alink_wolf.inc) (keep in sync).
- **Field-attack damage/stun split** (70/25) + twilight-enemy pause-stun ([d_albw_wolf_stun.cpp](../src/d/d_albw_wolf_stun.cpp)).
- **Bite heal** — inside the 5-bite event, if HP ≤ 50%, heal ¼ heart ([d_cc_uty.cpp:679-684](../src/d/d_cc_uty.cpp)).
- **Charge HUD** — 0–2 icons in the spur row ([d_albw_wolf_charge_hud.cpp](../src/d/d_albw_wolf_charge_hud.cpp)).

**Health model:** Wolf Link shares Link's global life gauge (`dComIfGs_getLife` / `getMaxLifeGauge` / `dComIfGp_setItemLifeCount`). There is **no wolf-local HP field**. (The LoP HUD *does* draw a health bar in wolf form — it renders the shared gauge; see Layer 3.)

**Transform:** binary human↔wolf, flag `FLG1_IS_WOLF` (`checkWolf()`, [d_a_player.h:385,1067](../include/d/actor/d_a_player.h)), proc `PROC_METAMORPHOSE` (`procCoMetamorphoseInit`, [d_a_alink.cpp:18254](../src/d/actor/d_a_alink.cpp)), model swap via `setArcName → loadModelDVD → changeWolf/changeLink`.

---

## Layer 1 — Wolf adds FA charges (1.5×) · **feasibility: HIGH**

### How FA charge works (owner: [d_focused_arts.cpp](../src/d/d_focused_arts.cpp))
- State is file-static: `s_bankCount` (whole banked charges, capped at `getMaxBank()` = effective tier 1–3) and `s_fillNumerator` (partial progress, out of `kFocusedArtsFillDenominator = 12`; rolls 12 → +1 bank). **Persists across transform** (only reset by stage load / toggle), so wolf-accrued charge survives reverting to human.
- **Every grant funnels through one file-static choke point:** `addFillSteps(int steps)` ([:288](../src/d/d_focused_arts.cpp)). Only **two** public accrual sites feed it today:
  - Melee sword hit — `dFocusedArts_onConnectedSwordHit()` ([:523](../src/d/d_focused_arts.cpp)), called from the **already-forked** wolf/human site [d_a_alink_cut.inc:353-355](../src/d/actor/d_a_alink_cut.inc) (currently gated `!checkWolf()`). Amount `getSwordFillStep()`: Ordon/Wood-Sumo = **+2/12**, Master/Light = **+1/12**, plain Wood = 0.
  - Projectile/item hit — `dFocusedArts_onConnectedItemHit()` ([:550](../src/d/d_focused_arts.cpp)), constant **+2/12**.
- Parries do **not** grant FA (route to bash/meter instead). Hidden-skill use and Flurry only **spend**.

### The seam
Add a public `dFocusedArts_onConnectedWolfHit()` beside the sword hook, routing through `addFillSteps` at **1.5×** a base step (e.g. base 2/12 → **3/12**, or a dedicated `kWolfFillStep`). Then at the sword-hit fork ([d_a_alink_cut.inc:353](../src/d/actor/d_a_alink_cut.inc)) branch: human → `onConnectedSwordHit()`, **wolf → the new wolf hook** (it's already the `checkWolf()` fork, so this is a natural split, not a new call site). Spend/Flurry/damage-resolver all read `s_bankCount` agnostically — **no changes needed there**.

### Things to reconcile (small)
- **Meter is hidden in wolf form.** `lopFaMeterActive()` ([d_meter2_draw.cpp:188](../src/d/d_meter2_draw.cpp)) early-returns on `checkWolf()`, so wolf-accrued charge is *invisible* until you revert. Decide: relax that gate to draw the lilac meter in wolf form, or leave it hidden (charge still banks, just unseen). Recommend showing it (the whole point is to make wolf feel connected to the FA economy).
- **`onConnectedSwordHit` bails on `dMeter2_isALBWLocked()`** — wolf has no ALBW meter; decide whether wolf accrual should respect that lock (probably not).
- **Which wolf hits count?** Normal bites only, or field/lock attacks too? Recommend normal bites (mirrors the sword-hit analogy; keeps the field attack about its own charge economy). Reuse the same 5-bite cadence's per-hit signal or grant per individual bite — a design choice (per-bite at 3/12 fills a tier-1 bank in 4 bites; tune to taste).

### Open design questions
- 1.5× of *what* baseline — the sword step, or a flat wolf rate? A flat `kWolfFillStep = 3` (= 1.5 × the 2/12 common step) is the simplest reading.
- Does wolf FA charge feed anything wolf can *spend*, or is it purely "bank now, spend as human later"? Today wolf has nothing FA-spendable (no parry/hidden-skill/Flurry in wolf form). If it's bank-for-later, that's fine and coherent; if you want wolf to spend FA, that's a Layer-1b design (e.g. an FA-powered wolf attack) — out of scope for the accrual change but worth noting.

---

## Layer 2 — Giant Midna transform · **feasibility: BLOCKED on asset (engine hook-up is clean)**

### The hard finding: there is no giant-Midna arc
Only **two** Midna object arcs exist: `Midna` (imp, rigged 29-joint `S_MD`) and `Dmidna` (dying/white Midna, rigged 25-joint `D_MD`). The barrier-smashing **giant is a demo-only performance** — the imp Midna actor streams per-cutscene override meshes `demo00_Midna_cut00_*` (hands/face/tongue/body + a `.bls` face blend-deform) from the room's demo arc ([d_a_midna.cpp:384-497](../src/d/actor/d_a_midna.cpp)). Those resolve **only when that cutscene's demo arc is mounted** — there is no room-independent giant arc to `resLoad`.
- `MGN`/`B_mgn` is a **red herring**: `d_a_b_mgn` is **Beast Ganon** (Hyrule Castle boss, [d_a_b_mgn.h:14](../include/d/actor/d_a_b_mgn.h)). The `MGN_*` SFX in imp Midna belong to her **magnified hair-hand** grab/throw, not a giant body.
- **Animation inventory:** imp Midna has a full idle/move/reaction + one attack (magnified hand) vocabulary, but at **imp scale**, bound to the shared anm archive. `Dmidna` ships **two collapsed-wait loops only** — no locomotion/attack. The barrier-smash is not a discrete reusable clip; it lives in the cutscene demo BCKs on the override meshes.

### The engine plumbing (this part is ready)
Transform is a two-arc model swap keyed on `checkWolf()`:
- Arc selection: `setArcName(BOOL isWolf)` ([d_a_alink_swindow.inc:14](../src/d/actor/d_a_alink_swindow.inc)) — add a new arc branch here.
- Swap driver: `loadModelDVD()` ([d_a_alink_swindow.inc:114](../src/d/actor/d_a_alink_swindow.inc)) — frees old heap, resLoads new arc, calls `changeWolf`/`changeLink` (PC build-then-swap alt heap already in place).
- Builders: `changeWolf()` ([d_a_alink_wolf.inc:147](../src/d/actor/d_a_alink_wolf.inc)), `changeLink()` ([:321](../src/d/actor/d_a_alink_wolf.inc)).
- Transition visual: `setMetamorphoseModel()` ([d_a_alink.cpp:14736](../src/d/actor/d_a_alink.cpp)) — the poof is a separate item-model anim, not a body scale.
- **Scaling is one line** — models scale via `setBaseScale` (imp Midna: [d_a_midna.cpp:849-856](../src/d/actor/d_a_midna.cpp)); a giant form could be a uniformly up-scaled rigged BMD.
- **The blocker in the machinery:** `checkWolf()` / `FLG1_IS_WOLF` is a **binary** human-vs-wolf bit threaded through *hundreds* of `!checkWolf()` (implicitly "human") call sites. A third form **cannot overload that bit** — it needs its own state flag + an audit of those assumptions. This is the real engineering cost, separate from the art.
- Note: `procWolfGiantPuzzleInit()` ([d_a_alink_wolf.inc:1759](../src/d/actor/d_a_alink_wolf.inc)) already exists as a *puzzle* giant-shove state — not a combat form, but worth mining for scale/collision precedent.

### Realistic paths
- **Prototype (cheap, ugly):** reuse an **up-scaled existing rigged model** (imp `S_MD`, or even the wolf) as a placeholder giant to validate the new state flag + transform branch + stomp gameplay before any art exists. Proves the machinery and the `checkWolf()`-audit cost without waiting on a model.
- **Real version (content task):** author/import a rigged giant model + idle/move/attack/**stomp** BCKs (Blender → BMD via the project's reskin/BMD tooling — cf. the Armogohma/BMD-reskin memories), wire a new transform arc into `setArcName → loadModelDVD`, add the giant state flag, and build the stomp as new attack code with **enlarged collision cylinders** (Link collision is hand-authored in `d_a_alink*`, not derived from scale) + an AOE query.
- The demo `_BD_*` override meshes are the closest to a "giant Midna" look but are demo-arc-bound and include a bespoke `.bls` deform — not a clean reusable rig.

**Bottom line:** this is not "point existing code at an existing asset." Budget it as: (a) a **content pass** (giant rigged model + anims), plus (b) an **engine pass** (third transform state flag + the `checkWolf()` binary-assumption audit + stomp AOE). Recommend the placeholder-model prototype first to de-risk (b) independently of (a).

---

## Layer 3 — Wolf health-regain (Lies of P) · **feasibility: MEDIUM (mostly clean)**

### The reference mechanic
**Lies of P "Guard Regain":** blocking still chips HP, but the chipped portion becomes a recoverable **white** segment on the health bar; **attacking the enemy refills** HP up to that white segment (per-hit amount scales with a Guard-Regain/Vitality attribute; can be partial or full); the white segment **slowly drains over time** (P-Organ upgrades slow the drain). It's tied to *guarding*, and vanilla loses the white to time-drain, not to the next hit. (Closest "on any hit" analog is **Bloodborne's Rally/Regain**.)

**Your Wolf spec is a deliberate, harsher hybrid:** trigger on **any** hit (Rally-like, not guard-gated) · recover via **bites** (each returns a % of the faded pool) · **a second hit before recovery burns the whole pending pool** (stricter than LoP's time-drain). Reads well for a melee-only, dodge-light form: rewards immediate aggression, punishes eating a second hit.

### Seams in this codebase
- **Fade on damage:** `procWolfDamageInit()` ([d_a_alink_wolf.inc:6025](../src/d/actor/d_a_alink_wolf.inc)) is the single wolf damage-reaction entry — the place to (a) compute the faded fraction of the hit's damage and (b) record it into a new session-only "faded pool" field, and to apply the **"burn on second hit"** rule (if a pool already exists when a new hit lands, zero it).
- **Recover on bite:** the existing **bite-heal block** ([d_cc_uty.cpp:679-684](../src/d/d_cc_uty.cpp)) is the natural recover hook — instead of (or alongside) the ¼-heart heal, return a % of the faded pool per bite and shrink the pool. Note today it fires once per 5 bites; a regain mechanic probably wants **per individual bite**, so the recover step likely moves to the per-bite branch ([d_cc_uty.cpp:~648](../src/d/d_cc_uty.cpp)) rather than the 5-bite event.
- **HP is the shared life gauge** — the faded pool must be tracked as a **separate session field** (e.g. `mWolfFadedHp`, actor-scoped like the charge fields) and reconciled against `dComIfGs_getLife()`. Care: human form shares the same gauge, so the pool must be wolf-only and cleared on transform-to-human / stage load / death (mirror the charge fields' session scope).
- **Display:** the LoP HUD already draws a wolf-form health bar (red bar, `project_lop_hud`, wolf-form Option-B gating in [d_meter2_draw.cpp](../src/d/d_meter2_draw.cpp)) sourcing the shared gauge — so the **bar exists**; the work is overlaying a "faded/white" segment on it (analogous to how the FA meter draws a partial segment). This is a smaller lift than the wolf-combat agent implied (no bar needs building from scratch).

### Tunables / design questions
- **Fade fraction** per hit (what % of damage taken becomes recoverable vs. permanently lost immediately).
- **Recover per bite** (% of the pool per bite; whether a single bite can fully recover a small pool).
- **Loss rule:** hard "burn on next hit" (your spec) vs. LoP-style slow time-drain vs. both (drain + hard-burn). Recommend exposing this as a small enum so it can be tuned/reverted.
- **Interaction with the existing bite-heal** (¼ heart ≤50%): keep both, or fold the ≤50% heal into the regain? Simplest is to keep them independent (regain = recover *recently faded* HP; the ≤50% heal = separate sustain), but they touch the same block so decide the ordering.
- **Does it also apply in human form?** Spec says wolf-only; keep it wolf-gated (the pool clears on transform).

---

## Cross-cutting notes

- **Persistence:** wolf combat is entirely **session-scoped** (actor fields, no save bits). The faded-HP pool (Layer 3) should be the same (session-only, cleared on transform/death). FA charge (Layer 1) is file-static and **persists across transform** by design — good (wolf banks, human spends).
- **The `checkWolf()` binary assumption is the single biggest structural risk** — it only bites Layer 2 (the third form). Layers 1 and 3 live *within* the existing wolf state and don't touch it.
- **Fragile spot to respect:** the charge **spend** logic is duplicated ([d_a_alink_wolf.inc:7519 + :7586](../src/d/actor/d_a_alink_wolf.inc)); any new wolf resource that spends must update both.
- **All new numeric tuning** (FA wolf rate, fade %, bite-recover %, loss rule) should be settings/consts so they're tunable and reversible, consistent with the project's pattern.

## Recommended sequencing
1. **Layer 1 (FA 1.5×)** — smallest, self-contained, immediate "wolf feels connected to the meta." Ship first; decide the meter-visibility question.
2. **Layer 3 (health-regain)** — medium; reuses the bite-heal + wolf-damage seams + the existing LoP wolf health bar. High gameplay payoff for a melee form.
3. **Layer 2 (giant form)** — largest and asset-blocked. Start with the **up-scaled placeholder prototype** to validate the third-state-flag + stomp machinery independently of the art pipeline; commit to the content pass only once the engine path is proven.

---

## IMPLEMENTED — Wolf Howl art (Art 1), as built 2026-07-13

The first art (**D-pad Up = combat Howl AOE**) is built and playtested. How the pieces work today:

### Input & gating ([`handleWolfHowlBurst`](../src/d/actor/d_a_alink_dusk.cpp))
- Dispatched from the D-pad handler when in **wolf form + quick-swap mode** (plain Up; the vanilla `cycleNextSword` no-ops on wolf). Consumes the D-pad edge (`mPressedButtonFlags = 0`) so it doesn't leak to the menu layer.
- Gates: `checkWolf()` + `dAlbwWolfCombat_isEnabled()` + `dusk::isDpadQuickSwapEnabled()`. Then the **unlock** (save event bit 713, `dAlbwWolfArts_isHowlUnlocked`) and **1 wolf charge** cost — **both bypassed by the editor toggle `game.wolfArtsDevTest`** ("Wolf Arts Dev Test", ALBW editor section). Must be grounded, not flying, not in a cutscene.
- Sets a one-shot `mWolfHowlWantCombat` that `procWolfHowlInit` consumes into `mWolfCombatHowlActive` (so a vanilla demo howl — [d_a_alink_demo.inc:335/338](../src/d/actor/d_a_alink_demo.inc) — is never mistaken for a combat howl), then picks + starts a song and calls `procWolfHowlInit(0)`.

### Animation — the DUET pose (not the sitting howl)
- The combat branch of `procWolfHowlInit` plays **`WANM_HOWL_SUCCESS` (WL_HOWLC) looped** (`mUnderFrameCtrl[0].setLoop(27)` + `setAttribute(2)` + `setFaceBasicTexture(FTANM_WL_HOWLC)`) — the "singing with the Golden Wolf" pose from the Hero's-Shade duet. Exit = `WANM_HOWL_END` (built-in 30-frame return).

### Duration — tied to the SONG length ([`procWolfHowlCombat`](../src/d/actor/d_a_alink_wolf.inc))
- No fixed timer. Each frame polls `Z2GetSeqMgr()->isItemGetDemo()` ("a fanfare/song is playing"): hold the loop while true, then a **60-frame (1 s) buffer**, then `WANM_HOWL_END` → gameplay. Auto-fits every tune's length. Startup grace (`mWolfHowlElapsed > 15`) + ~30 s hard cap guard against a 1-frame gap / a stuck/silent tune.

### Music — random from a FOUND-songs playlist
- Pool built each howl from **3 always-on solos** (`Z2BGM_HOWL_TOBIKUSA / UMAKUSA / ZELDASONG`) + each **learned DUO** (`Z2BGM_*_DUO`, gated on its event bit F472–F477 via Golden-Wolf idx). Picked with `cM_rndF`; `mDoAud_subBgmStart` (fanfare path, auto-ducks field BGM); `stopWolfHowlSong` on end. Editor has a **"Preview Howl Tune"** audition picker (kept for a future player-facing song chooser).

### AOE collider (cosmetic VFX is fully independent)
- `procWolfHowlInit` sets a Link-centered radial collider (`setCylAtParam(AT_TYPE_WOLF_CUT_TURN, …, radius 300, height 155)`); armed every singing frame via `onResetFlg0(RFLG0_UNK_2)`, placed at the wolf by `setWolfAtCollision`'s `PROC_WOLF_HOWL` branch. Tunable placeholders.

### Ring VFX — **base KAITENGIRIL, plain fire-and-forget** ([`setWolfHowlSpinEffect`](../src/d/actor/d_a_alink_wolf.inc))
This is the current, deliberately-minimal state after a long detour (see the lessons below):
- Emits the **4-emitter KAITENGIRIL "Light" set (A–D)** via the **NULL-tevStr** overload `dComIfGp_particle_set(effId, &pos, &rot, scaleArg)` ([d_com_inf_game.h:3368](../include/d/d_com_inf_game.h)). NULL tevStr is essential — it makes the effect keep its **own authored colors (white) + sparkle** instead of being tinted by the actor's `tevStr`.
- **Fire-and-forget only.** Re-emitted from `procWolfHowlCombat` on a cadence (`period`); each burst lives its natural lifetime. Centered at `current.pos + Y30`. A **sweep** yaw (`shape_angle.y + elapsed × sweepRate`) winds over the howl so the ring rotates around the static wolf (the wolf never turns); an optional **orbit** offsets the emission point along the sweep.
- **FINALIZED LOOK (user-tuned 2026-07-13, baked as both the `l_wolfHowlVfx*` constants and the ConfigVar defaults):**

  | Param | Value |
  |---|---|
  | Tilt (X) | 0 (natural angle) |
  | Roll (Z) | 0 |
  | Width % | 90 |
  | Height % | 90 |
  | Sweep Rate | 300 (~one rev / 3.6 s) |
  | Orbit | 0 (centered) |
  | Period | 1 (burst every frame) |

- The editor tuner remains for later re-tuning: master toggle **"Wolf Howl VFX: Apply Tuner"** (`game.wolfHowlVfxOverride`) + six sliders (Tilt 0–360 / Roll 0–360 / Width / Height / Sweep Rate / Orbit / Period). **Override OFF = the baked finalized look**; ON applies the sliders live mid-howl. All cosmetic.

### Audio — enemy SFX punch through the howl song
The howl song (fanfare path) normally **culls almost every sound effect** while it plays — `Z2SeMgr::isSoundCulling` ([Z2SeMgr.cpp](../src/Z2AudioLib/Z2SeMgr.cpp)) whitelists ~18 SE during `isItemGetDemo()` and drops the rest, which silenced enemies being hit by the AOE. (The music-side duck, `mFanfareMute`, only scales the 3 BGM handles — it never touched SE and is untouched.) Fix: during the **wolf-howl tunes ONLY** — detected by `Z2SeqMgr::isWolfHowlSong()` (mirrors `stopWolfHowlSong`'s case list) — the culling gate lets through the **enemy block `[0x30000, 0x40000)`** (`Z2SE_EN_*` voices/damage/death) and the **impact block `[0x40000, 0x50000)`** (`Z2SE_HIT_*` weapon thunks). Item-get and other jingles keep the vanilla culling.

### Charges — the arts never build them, and the howl needs 2
- **Charge economy (confirmed):** both D-pad arts NEED 2 charges to fire; the **howl spends 1**, the **Midna arm spends 2**. `handleWolfHowlBurst` gates `mWolfChargeCount < 2`, decrements 1.
- **Arts don't feed the counter:** the howl AOE rides Link's own collider (`AT_TYPE_WOLF_CUT_TURN`, owner = ALINK) so it passed the bite-counter guard — a latent bug. Excluded via `!link->mWolfCombatHowlActive` at the increment ([d_cc_uty.cpp](../src/d/d_cc_uty.cpp) `cc_at_check`); Link can't bite mid-howl so the flag exactly identifies howl-AOE hits, and the vanilla roll attack (same AT type) still counts. The Midna-arm art will be a separate non-ALINK actor, so its hits are excluded automatically (`mHitType` stays generic — the guard requires `HIT_TYPE_LINK_NORMAL_ATTACK`).

### Lessons learned (why it looks the way it does)
- **The FA great-spin finisher uses a DIFFERENT bank.** Captured at runtime (a since-removed `ALBW-GSDIAG` probe in `setCutTurnEffect`): it is **6× `KAITENGIRID` "Large" (A–F)**, immortal + continuously re-driven every frame, with per-emitter staggered local tilt (`leftRotLarge`: X 180° / Y 45°–60° / Z 13°) + lift (`leftTransLarge` Y 0/35/0/45/30/50), global rot = body yaw. The "whirlwind vs base" the user recalled is **spin direction** (`getCutTurnDirection` → sign of `field_0x3180`), *not* sword angle; the two-variant switch was a removed `hurricaneTestVfx` setting (tilt-on = "base" flat ring vs tilt-off = "whirlwind" upright spray).
- **KAITENGIRIL is (mostly) rotation-invariant** to the finisher's exact angles — the 180° X-flip is a visual no-op on its symmetric spray (it *does* respond to a strong ~90° flip, proven by diagnostic), so the finisher's tilt table doesn't read on it.
- **`becomeImmortalEmitter` + `playCreateParticle` + continuous re-drive ALTERS a fire-and-forget effect's particle appearance** — that (not the color) is what "broke" KAITENGIRIL's particles when replicating the finisher. **Do not use immortal/forced-continuous emission to "sustain" a fire-and-forget effect; periodic re-emit preserves the natural look.**

### Next on this art
- **Recreate the finisher *look* on base KAITENGIRIL via effect-rotation-over-time** (user goal): wind the emit yaw a little per burst so the ring **sweeps around the static wolf** (the wolf never spins) — the single biggest "great spin" cue — plus optional per-emitter position lift/spread for a layered ring. All particle-safe (orientation/position only, still fire-and-forget).

### FUTURE IDEAS (user, 2026-07-13 — banked, not yet researched/built)
- **Per-song special effects on the howl:** each howl tune gets its own flavor of VFX while it plays (the song is already known at pick time in `handleWolfHowlBurst`; a song→effect table would drive extra emitters/tints in `procWolfHowlCombat`).
- **Per-song KAITENGIRIL color:** tint the howl ring by the playing song — e.g. **Song of Healing = light red/pink/violet**. Note the current emit deliberately passes NULL tevStr to keep the effect's own white; a per-song tint would pass a prm/env color (the `dComIfGp_particle_set` overloads accept GXColor prm/env args) — needs a visual test to confirm KAITENGIRIL's particles take a prm-color tint well.
- **New wolf art candidate — Midna's rideable Twilit Kargarok:** spawn the Carrier Kargarok (E_YC — the actor the player already rides in vanilla Lake Hylia/City sequences) as an attack: strikes enemies **from above** and/or **dashes into them**. Precedent: the arm art proved the spawned-helper-actor pattern (own collider, non-ALINK owner, auto-excluded from the charge counter); E_YC already has flight + carry logic to borrow.
- **Giant "demo" Midna (D-pad Down ultimate)** — pursue next; findings below.

### GIANT MIDNA — research findings (2026-07-13)
**The demo giant EXISTS in the game files** (the old "no asset" conclusion was wrong): the Fused-Shadow
barrier-destruction giant is real-time geometry — a **4-part composite** `demo00_Midna_cut00_BD_*`
(body `_tmp` 0x6C + Fused-Shadow mask 0x57 + hands 0x56 + giant hair-hand 0x6B) living in the
cutscene archives ([Demo30_01.h](../assets/GZ2E01/res/Object/Demo30_01.h):79-98, also Demo24_02 /
Demo30_02). No pre-rendered video exists (zero `.thp` in the game). `daMidna_c::createHeap` loads
these only while a demo arc is mounted ([d_a_midna.cpp:387-500](../src/d/actor/d_a_midna.cpp)) —
they are not resident in free gameplay as-is.

Other key findings:
- **Whole-model giant scale already exists**: `mpHIO->m.scale` → `setBaseScale` on both Midna models
  ([d_a_midna.cpp:853-859](../src/d/actor/d_a_midna.cpp)); the devs left a 0–5× debug slider on it.
  An up-scaled imp Midna is essentially free (the mechanic-first scaffold).
- **The base `Midna.arc` already carries the fused-shadow parts** (mask / hands / halo / giant
  hair-hand, loaded at [d_a_midna.cpp:549-664](../src/d/actor/d_a_midna.cpp)) — a "fused-shadow
  hand slam" is assemblable without the demo composite.
- **The SLAM animation exists**: Midna's boss **catch-and-hurl** set `ANM_MGN*`
  (resIDs 0x1CD–0x1D2 + `MDN_V_MGN_CATCH/TAME/THROW` voice, table
  [d_a_midna.cpp:242-247](../src/d/actor/d_a_midna.cpp)) — reads as a grab/slam attack.
- **Mortal Draw damage = atp 4** ([d_a_alink_cut.inc:1383](../src/d/actor/d_a_alink_cut.inc)) — the
  arm already uses 4; a giant slam wants 4–5 with a ~250–400 radius.
- **D-pad Down is FREE** (Quick Transform is D-pad LEFT single-tap in the current binding setup) —
  no input conflict for the giant.
- **Charge-cap ripple**: need-3/spend-3 requires raising the cap 2 → 3 (earn cap, spend gates,
  HUD max pips).
- **Recommended path**: build the mechanic end-to-end on the up-scaled scaffold first (always-resident
  assets, arm-actor pattern), then swap the visual to the repackaged demo giant as a pure upgrade.

### GIANT MIDNA — the pipeline (research pass 2, 2026-07-13)
**Reframing discovery: the BD giant is a SKIN, not a creature.** The 4 `demo00_Midna_cut00_BD_*`
models share the ordinary imp-Midna skeleton: at runtime `BD_tmp` is driven by **copying every joint
matrix from the standard animated Midna model** ([d_a_midna.cpp:1107-1111](../src/d/actor/d_a_midna.cpp)),
mask/hands/hairhand are joint-parented (HEAD / HAND_L+R / HAIR_5, `setBodyPartMatrix` :975-1172), the
skin swap is one pointer switch (`initMidnaModel` :3018-3025, toggled by STB flag `FLG1_UNK_1`
:1561-1569), and the **giant size is just the runtime scale knob** (`setMatrix` :852-859 — default
1.0; the cutscene scales it up). There are **no BD-specific animations** — the giant plays normal
Midna body/face anims, so **the `ANM_MGN*` catch/slam works on the BD skin directly.**

**Loading is trivial:** arc resolution is a raw string → `/res/Object/<name>.arc` (no manifest;
[d_resorce.cpp:53-70](../src/d/d_resorce.cpp)), so `dComIfG_resLoad(&phase, "Demo24_02")` (the lean
stock arc carrying the BD set, indices 0x35/0x2A/0x29/0x34) works from any gameplay actor — same
call the Shade Boss Wolf uses for "GWolf". Even better: the **Aurora overlay** (custom_assets Layer A)
resolves paths the game never shipped, so a **tiny custom arc with ONLY the 4 BD BMDs** can be dropped
in `model_replacements/` and loaded by name — needs an external RARC packer (wszst/GCFT; no packer in
tools/). Animation comes from the resident `Midna` arc regardless, so the custom arc stays tiny.

**Build recipe (two options):**
- **Option A (least code):** drive the real daMidna_c — mount a BD-carrying arc, set `FLG1_UNK_1`,
  write the scale knob big. Her existing compositor/draw does everything.
- **Option B (standalone, recommended for a combat art):** a spawned actor (arm-art pattern) that
  loads the BD arc + the `Midna` arc, builds one McaMorf on the Midna skeleton (arc idx 14) as the
  anim driver + the 4 BD models as plain J3D models, per-frame joint-parents them exactly as
  `setBodyPartMatrix` does, `setBaseScale(6-8x)`, plays `ANM_MGN*` as the slam, owns a big sword
  collider (atp 4-5, non-ALINK owner). Gotchas: arc must stay mounted for the actor's life;
  BD_tmp cannot animate without the Midna skeleton; per-frame tevStr + `modelEntryDL` per part.

**Load strategy:** preload + pin the slim arc at shop-unlock into a retained heap
(`d_ww_itemmdl_pc.cpp:285` pattern) → D-pad press spawns instantly, no DVD stall. (Async cPhs on
spawn = Shade-Wolf pattern is the lazy fallback; micro-stall on first use.)

### ⛔ THE IDLE IS FINAL — DO NOT TOUCH, DO NOT "DIAGNOSE" (locked 2026-07-15)

**The READY idle arch is EXACTLY what the user wants, as of this version. It is a settled PREFERENCE,
not an open question, not a symptom, and not evidence of anything. Any future work that proposes
changing it, or cites it as a defect, is WRONG BEFORE IT STARTS.**

Reference look: one D-pad press, no enemies — Midna's hair arcs up and forward over the wolf's head,
hand hanging at the snout. If a change alters that silhouette, the change is wrong.

**The specific error that produced this lock, so it is never repeated:** the ALBW-ARMIK probe reports
`dot ≈ -1` on the idle (all four rendered segments "pointing away from the target"), and this was
written up as "the idle is a myth / the idle is broken too." **That reading is invalid.** The idle's
`atn_pos` is a *synthetic hover point* — a puppet handle invented by `d_a_albw_midna_arm.cpp` to
SHAPE a pose. The hair is not supposed to point at it. `dot ≈ -1` there means the arch is working.

**Rule: `dot[]` is only meaningful while `striking=1`.** On idle rows it measures nothing. More
generally: **a telemetry number cannot refute a stated preference.** The idle is defined by what the
user sees and has approved, full stop.

### ★ ARM — CURRENT = the idle+vanilla baseline; NEXT LEAD = the RETRACT SNAP (user epiphany)

**2026-07-16 late: elevated strike point tested — "didn't have any change" — REVERTED (clean).
That null result is itself evidence: moving the outbound aim point dramatically (ground vs
elevated) changed NOTHING visible ⇒ the visible artifact is not produced by the outbound aim.**

**USER EPIPHANY (the live lead): what reads as "shooting backwards" is the RETRACTION —**
`clearReachPos()` drops the aim flag and `modelCallBack` gates per-frame, so the whole rendered
chain SNAPS from aimed pose to the riding-anim pose (up/back) in ONE frame, while the outbound
reach is slew-limited (0x1800/frame, 11-17 f needed vs a 16 f window) and barely draws.  Eye
gets: invisible slow forward creep -> single-frame backward snap = "it shot backwards," snap size
scaling with how far the aim had swung (= enemy's relative position).  Every probe was throttled
STATIC snapshots — structurally blind to a temporal discontinuity.  Retro-fits: the frozen
screenshot's backward arm = the anim pose post-snap; "usually when the move ended" = the biggest
snap (actor death).  **PLAYTESTED: THE BACKWARDS SHOT IS DEAD** — the user's snap diagnosis was
the answer; the no-snap retract (eased glide strike->hover, striking=false, aim flag never drops
mid-cycle) is permanent.  **COMMITTED `ab6a1b500f`** (+ `4e3dbb9562` vendor-gitlink fix).

**What playtest revealed next: the fan is a WEATHERVANE** — angle-only, it orients toward the
published point and CANNOT extend (enemy dead ahead = hand does not move at all, its direction was
already right; off-axis = hand yaws to face the enemy, then stops).  Distance to target has zero
effect on the pose.  Corollary: when vanilla hair actually EXTENDS (Ganon/Wchain grabs) that is
the ANM_MGN catch ANIMATIONS via the FLG0_UNK_200000 skeleton-driven path — the fan only ever did
the pre-grab aiming.  We spent sessions trying to get a thrust out of the aiming system; vanilla
never got one out of it either.

**STRIKE UNCURL — PLAYTESTED WORKING** (2026-07-16): "the hand reaches forward to the enemy" —
correct shape + direction; the fan collapse was always right, the confounds were what sank it.
Speed then tuned in two steps (both playtested): vanilla slew = too slow (10-14 f) -> div3/0x4000
(~4 f) = "faster" -> **1-FRAME SNAP (current): while striking the pose is ASSIGNED to the strike
line** (`*angle_z = target_angle_z` at the slew site; 1 f = the 30 Hz floor).  Striking-gated:
idle + return glide keep the vanilla ease — crack out, soft back.  User verdict: max pose speed
reached, **still not fast enough overall** -> the remaining speed lives in the PUBLISHED POINT:
STRETCH eases hover->enemy over `kArmStretchFrames=12` (0.4 s), so the snapped pose rides a
12-frame-slow aim line.  NEXT SPEED LEVERS (in order): kArmStretchFrames 12->6ish (the jab
itself), kArmHitFrames/kArmPauseFrames (cycle cadence).  Reach question still open after speed
(physical ~140-150 from Midna's head vs 480 accept gate; options: cut gate / ANM_MGN catch-anim
extension research).

### (superseded — NULL RESULT, reverted) ARM — ELEVATED STRIKE POINT (2026-07-16 evening)

**The hand-aim fix below was BUILT, TESTED, REFUTED, and surgically REVERTED** (still fired
backwards; its ARMHAND probe data — 35 rows — showed the hand matrix X-axis at +0.96..0.995
toward the enemy DURING the wrong-looking strikes, matching the approved idle's +0.9999).  That
completes the elimination: chain joints ✓, tip segment ✓, hand matrix ✓ — every component aims
correctly and the move still reads backwards.  **The read is the fan's whole ARC, and the arc's
character is set by TARGET ELEVATION**: the fan is anchored UP at the root — high target (Ganon,
the idle hover) = small graceful forward arc; low target (ground enemy eyePos, 50-140 below the
root) = ~190-deg up-and-over sweep that reads as firing backwards.  Ganon looks right because
Ganon is TALL.

**CURRENT BUILD:** d_a_midna.cpp = 100% vanilla (hand-aim insertion deleted).  ONE change, in OUR
actor only (`d_a_albw_midna_arm.cpp`): STRETCH/HIT clamp the published aim point to never sit
below `mReachFrom.y + kArmStrikeElevate(40)` — the enemy's column at/above Midna's head height,
a touch above the READY hover.  The fan stays in its happy regime (Ganon-style forward reach
toward/over the enemy); the COLLIDER still sits on the real enemy (`tgt->current.pos`, untouched);
flying/tall enemies keep their real eyePos (max, not override).  Expected look: reach/grab toward
the enemy from above, not a straight punch.  Tuning knob: kArmStrikeElevate.

### (superseded — REFUTED IN PLAY + REVERTED) ARM — TRUE ROOT CAUSE hypothesis: HAND-AIM FIX (2026-07-16)

**THE "FIRES OPPOSITE" BUG WAS NEVER IN THE CHAIN.  It was the HAND MODEL, unoriented.**
Found via the user's Beast-Ganon probe capture (16 vanilla-lock rows): the working Ganon grab and
our wrong-looking strike produce NEAR-IDENTICAL chain geometry (TIPDOT 0.90-0.995 vs 0.86-0.9999).
The difference is `setBodyPartMatrix` (d_a_midna.cpp ~:1009-1053): the hand BMD
(`mpShadowHairhandBmd` — the big directional shape that visually IS the move) is oriented by
exactly two vanilla branches: "no aim" (natural correction) and "wolf lock"
(`getWolfLockActorEnd()` → explicit face-the-target `XrotM(vec.atan2sX_Z() - shape_angle.y)` +
`bvar8=true` → material node 2 = the big dome GRAB hand).  Our art publishes an aim point with NO
wolf lock → NEITHER branch ran → the hand rendered with a raw unaimed matrix, visibly firing the
wrong way while the thin chain aimed right.  The user's eyes were correct the entire time; every
chain-side probe was measuring joints and could not see this model's orientation; every chain-side
"fix" (scale/fan/slew, all sessions) was tuning the part that was already right.

**FIX BUILT (one block):** third arm on that if-chain — module- and STRIKING-gated — the vanilla
wolf-lock formula verbatim fed our published reach point, `bvar8=true` so strikes show the dome
grab hand (the Beast-Ganon look, user decision).  Idle byte-identical (striking=false never enters
the branch); vanilla untouched (runs only when the wolf-lock branch did not).

**Verdict = the user's eyes** (the probe cannot see hand orientation): during punches the big grab
hand should face the enemy; the idle arch unchanged.

### (superseded) ARM — VANILLA-GRAB BUILD (2026-07-16 — chain reverted to pure vanilla; carried forward into the hand-aim build above)

**Playtest verdict on the stab build: REJECTED — "starts rolling into itself and looking clumsy."
User decision: KEEP the idle exactly as-is; everything non-idle = the stock vanilla hair reach,
specifically the Beast-Ganon lock-on grab look.**

**What this build IS:** `d_a_midna.cpp`'s hair pipeline is now **100% vanilla** — every behavioral
modification is DELETED (not gated off): strike scale 3.57, straight-fan collapse, stab slew boost,
fast decay, fast-shed net, IK overshoot clamp, wind-down support.  Only the ALBW-ARMIK probe
(non-behavioral) remains, plus a JUT! tripwire (fires only if someone reintroduces a scale hack).
The arm actor keeps its state machine (idle hover / stretch / hit / release-return retract / pause)
publishing reach points; the hair renders them through the untouched vanilla fan — the exact
mechanism the approved idle arch uses, aimed at the enemy.  RETRACT is back to `clearReachPos()`
(release-return, the user's original A/B preference — jut-proof now because nothing is magnified).
WINDDOWN state removed (nothing to shed).

**Consequences to expect (by design, not bugs):**
- Strikes are NORMAL-SIZE hair doing the Ganon-style reach — the big 3.57 arm is GONE.  It was the
  root of every jut (magnified non-aimed pose) and never lengthened the skeleton anyway.
- At long range the hand reaches TOWARD the enemy without touching (like Ganon).  `kArmMaxRange`
  stays 480 (gameplay range, mid-boss fix); cut it toward ~140-200 if hand-contact matters.
- If a big arm is ever wanted again, it needs a NEW lever (mesh/model work), NOT mHairScale.

**Revert map:** the current d_a_midna.cpp IS vanilla — nothing to revert there.  The stab build and
the screenshot baseline are both re-buildable from the recipes/history in the superseded sections
below; the 5-step recipe below (stab-era) no longer matches the file and is kept as history only.

### (superseded — REJECTED in play) ARM — THE STAB BUILD (2026-07-16) + its revert recipe

**Intent correction (user): the attack was always meant to be a quick STAB/jut — the baseline's
energy, aimed AT the enemy.  The "slap" was never a goal; it was the aimed pose arriving with no
speed left (vanilla slew cap 0x1800/frame low-pass-filters the 12 f published thrust).  Do not
frame "hand touches enemy gently" as success — the target is a fast stab that points correctly.**

Playtest verdicts folded in: direction improved with the straight aim (user-confirmed); residual
opposite-jut at MOVE END confirmed by instrument — 14 `JUT!` lines, all `reachUp=0`, scaleZ decaying
2.43→1.34 at exactly the vanilla 0.1 rate = actor death cleared the reach while the scale was still
strike-sized, and both safeguards were gated behind reach-active so neither engaged.

**CURRENT BUILD (2026-07-16) = reverted baseline + one coherent layer, five pieces:**
1. Straight aim while the art aims (fVar1 collapse, gate `striking || scaleZ>1.15`) — d_a_midna.cpp.
2. **STAB SNAP** — slew boost while aiming (`kArmJabSlewMax=0x4000, Div=3` vs stock 0x1800/5): the
   pose whips to the strike line in ~4 f and holds through impact.  The stab energy, aimed.
3. Guided RETRACT (striking held true, ease-out glide back) — kills the post-impact jut window.
4. **STATE_WINDDOWN_e** — life expiry no longer dies on the spot; 18 f holding the hover while the
   scale sheds fast, THEN clean death.  Kills the measured end-of-move jut.
5. Fast-shed safety net in d_a_midna.cpp vanilla branch (`scale->z>1.15` → 0.3): covers abrupt
   deaths (transform-out, feature off) the actor can't wind down.  Idle-proof — vanilla hair scale
   never exceeds ~1.0.
Every piece is dead in the idle by construction (striking=false + scale 1.0 fail both gate terms).

**⛔ REVERT RECIPE — TO THE SCREENSHOT BASELINE (surgical, verified-complete as of 2026-07-16).**
The older recipe further below is STALE (predates the fan/retract work — executing it alone leaves
a hybrid).  To restore the baseline (big 3.57 arm, vanilla fan+slew, release-return, fires-opposite
bug intact, idle arch perfect):
1. `d_a_midna.cpp` — delete the "STAB" `#if TARGET_PC` gate block after the FLG0 set/clear (the
   `kArm*`/`armReachActive`/`armJabStraight` block).
2. `d_a_midna.cpp` aim branch — restore stock slew + ramp: the two
   `cLib_addCalcAngleS(angle_*, target_angle_*, 5, 0x1800, 0x100)` calls unconditional, plain
   `fVar1 += 0.3f;` (delete the armJabStraight ternary + boosted-slew branch).
3. `d_a_midna.cpp` scale site — keep the striking→3.57@0.3 branch; delete the reach-not-striking
   fast-decay `else if` and the fast-shed net in the vanilla `else`.
4. `d_a_albw_midna_arm.cpp` STATE_RETRACT — replace the guided-glide block with the single line
   `dAlbwMidnaArm_clearReachPos();`.
5. `d_a_albw_midna_arm.cpp` — remove STATE_WINDDOWN_e (enum in the header, enterState case, the
   bail split back to one combined `if`, the switch case) + `kArmWinddownFrames`.
KEEP regardless (non-behavioral): the ALBW-ARMIK probe + JUT! detector, `armLocalVec0` capture,
the corrected `kArmMaxRange` comment.  The IDLE is not touched by ANY step above.

### (superseded) ARM AIM — MECHANISM FOUND + GUIDED-RETRACT FIX BUILT (2026-07-15, awaiting playtest)

**The "arm fires opposite" was never an aiming bug.  It was the post-impact window.**  Frame-by-frame
(34 f cycle): STRETCH 12 f + HIT 4 f render the aimed pose (the user's "slap" — correct); then the
old RETRACT called `clearReachPos()`, dropping the aim flag, so for 12 f the hair rendered the RAW
riding BCK animation — at a strike scale that decays far too slowly to matter (fixed-step
`cLib_chasePos`, c_lib.cpp:368 — ~44 f home vs an 18 f gap) — followed by 6 f of PAUSE arch, still
huge.  18 of every 34 frames were a giant non-aimed pose.  **The probe was gated on the reach being
active, so those exact frames were never logged** — every "healthy" log row was a STRETCH/HIT frame.

**Load-bearing measurement (position-only probe): `reachLen` ~75–110 units at EVERY scaleZ 1.0→3.1.**
The hair scale does NOT lengthen the skeleton — joint translations never scale, only the per-segment
mesh magnifies.  Consequences:
- The visible arm = mesh magnified ~3.57× hanging off a ~100-unit skeleton.  Its direction is the
  TIP's orientation, amplified — which is why small tip errors read as huge wrong-way limbs.
- The old "3.57 × 5 × 28 = ~500 reach" justification for `kArmMaxRange=480` was FICTION (comment now
  corrected in the actor).  480 is a gameplay range, not a geometric one.
- Distance-adaptive scale is pointless (tried, refuted, removed) — scale is a pure look knob.

**THE FIX (built, uncommitted):**
1. **Guided RETRACT** (`d_a_albw_midna_arm.cpp`) — RETRACT now publishes an ease-out glide from the
   strike point back to the READY hover with `striking=true`, instead of `clearReachPos()`.  The
   hair stays aimed + at strike scale the whole cycle; the jut window ceases to exist.  NOTE: this
   changes the A/B'd release-return feel — accepted cost; re-tune the glide, never restore
   clearReachPos.
2. **Straight-fan gate** (`d_a_midna.cpp`) = `striking || scaleZ > 1.15` (backstop for tails).  Two
   failed gates are documented in the code comment — striking-only (arch snapped back early) and
   scale-only+adaptive-scale (close targets fell under the threshold: `dist=117 scaleZ=1 straight=0`).
3. **Fast decay on disengage** — reach-active-not-striking sheds scale at 0.3 (~15 f) vs vanilla 0.1.
4. **Probe additions**: `TIPDOT` (HAIR_4→HAIR_5 vs to-target, pure positions — the visible punch
   direction) and a `JUT!` detector that fires iff a non-aimed pose renders at strike scale (the old
   blind window).  A silent log after a strike session = the jut is dead.

**Playtest verdict keys:** slap still correct; hair visibly PULLS BACK from the enemy instead of
snapping to giant anim-hair; no `JUT!` lines; TIPDOT positive during strikes.

### (superseded) ARM AIM — OPEN BUG + FULL HANDOFF (2026-07-15)

**STATE: the art is functionally complete and playtest-approved EXCEPT the hair visually fires ~180°
opposite the target. Damage is correct (the collider is placed at the target independently of the
hair render). Everything below is UNCOMMITTED.**

#### What is GOOD and must be preserved
| Thing | Value / state | Notes |
|---|---|---|
| **Reach scale** | **UNIFORM `{3.57, 3.57, 3.57}`** (`d_a_midna.cpp` ~:2732) | **User: "I do LIKE the visual size of it."** Keep this size. It fattens vs. the old z-only scale — user accepted the look. |
| **Idle READY stance** | forward rest, `kArmIdleForward=150`, `kArmIdleHoverY=20` | **Works, user-approved** ("idle position still remains great"). Probe shows `dotFwd=+0.998` here. |
| Jab pacing | 12f out / 4f hit / 12f back + 6f pause ≈ 1.13 s/punch | 30 Hz sim tick. User-approved. |
| Motion style | ease-out jab out, release-return | User A/B'd and preferred this. |
| Range gate | `kArmMaxRange=480`, measured to `tgt->eyePos` | Fixes mid-boss targeting (Twilit Bloat E_YMB, Carrier Kargarok E_YC — both ENEMY group; the group gate was never the problem, the origin-distance was). |
| Free targeting | `LockonTarget(0)` returns the soft attention candidate — no Z-lock needed | User: keeper. |
| IK overshoot clamp | in `setHairAngle`, module-gated | Harmless + correct-by-construction; left in although the bug it targeted was refuted. |

#### DEAD ENDS — all four REFUTED BY EVIDENCE. Do not re-tread.
1. **Companion-vs-riding frame mismatch (`shape_angle` 0x8000).** REFUTED twice. `shape_angle`
   **cancels** — both the aim frame (`YrotS(-shape_angle.y)`) and the render base
   (`shape_angle.y + 0x4000`, `modelCallBack` ~:281) key on it, so only their constant 0x4000
   relationship survives the world→local→world round trip. Patching the aim alone broke the idle;
   patching **both** = behaviourally identical to stock (verified in play). **Also: the vanilla
   `atn_pos` users (Obj_Wchain, B_MGN) are WOLF-Link contexts, not companion — the whole premise
   was fiction.**
2. **Software-chain overshoot.** REFUTED by its own probe. During a punch the IK scratch chain
   **converges monotonically**: `r = 355.6 → 338.1 → 276.6 → 176.7 → 79.4`, stopping 79 units short.
   It never sails past the target; the clamp never even engages (`armRemain` 177 > `step` 100).
3. **Non-uniform scale shear / wrong bone axis.** REFUTED: **uniform `{3.57,3.57,3.57}` still flips.**
   (Also disproved analytically: positive-diagonal scale can't mirror — det > 0 and it preserves
   every component's sign; and vanilla ships worse anisotropy (7:1 in `l_hairScale`) through the same
   path with no flip.)
4. **Render path broken.** REFUTED: `mDoMtx_XYZrotM` = `M * Rz(z) * Ry(y)` (`m_Do_mtx.cpp:58-73`), so
   the non-root undo `YrotM(-y); ZrotM(-z)` is an **exact inverse** — the render is a clean
   absolute-angle chain.
5. **Scale magnitude drives the flip.** REFUTED IN PLAY (2026-07-15): forced to `{1,1,1}` — the idle's
   own scale — the strike **still fires opposite**, just smaller. The scale is innocent. It is a pure
   look knob; leave it at the approved `{3.57,3.57,3.57}`.
6. **`bVar4` unwrap-branch selection.** REFUTED BY THE PROBE (2026-07-15): predicted the idle takes
   `bVar4=1` and the strike `bVar4=0`. **Both take `bVar4=0`** — the idle's local `y` is NEGATIVE
   (`lvec=(1.0,-35.7,176.0)`), because `kArmIdleHoverY=+20` is measured off the wolf's ORIGIN and
   still lands below Midna's hair root. Same branch in both states; not the difference.
7. **The probe samples the wrong model (`mpShadowModel` vs `mpModel`).** REFUTED BY READING:
   `d_a_midna.cpp:1107-1112` copies `mpShadowModel`'s BaseTRMtx and every AnmMtx straight into
   `mpModel`. They are the same pose by construction.

#### Verified reference facts (don't re-derive)
- **The `mHairPos` software chain is an IK SCRATCH — it does NOT render.** Its only output is the
  angles. The visible hair is posed by `daMidna_c::modelCallBack` (~:260-294) from `mHairAngleY/Z`.
  Visible length comes from the BMD bone offsets, never from `hairOffset`.
- **`angle_z` convention** (cross-verified against BOTH consumers): `0` = BACK, `-0x4000` = UP,
  `+0x8000` = FORWARD, `+0x4000` = DOWN. So a settled tip angle of ~26000 (~143°) is **correct**
  (forward-and-down), not an anomaly.
- `scaleM` post-multiplies ⇒ the software step is exactly `28 * scale.z`; `scale.x`/`scale.y` are
  **exact no-ops** on the software chain (they multiply `hairOffset`'s x=0, y=0).
- The aim math is a **cosmetic look-at spline** with no termination condition or overshoot guard.
  Vanilla is safe only because its 140-unit chain can never reach Wchain / Beast Ganon.
- J3D composes joints **T * R * S**, S inherited by children unless the joint's `scaleCompensate`
  bit is set. **`scaleCompensate` for the hair joints is UNREAD** — one of two unverified links.
- **The BMD hair bone axis is UNVERIFIED** — the other unverified link. Code samples `BaseX` for the
  head forward (:2678) and the hair tip (:2746), implying +X, but the software chain uses +Z.

#### ★ THE CONFOUND — the most important thing for the next attempt
**`striking` and `scale 3.57` are the SAME CONDITION in our code.** The scale branch keys on
`dAlbwMidnaArm_isReachStriking()`, so every observation so far changed both at once:

| Condition | scale | `dotFwd` | Visual |
|---|---|---|---|
| **idle** (`striking=0`) | 1.0 | **+0.998** | **correct** (user-confirmed) |
| **strike** (`striking=1`) | 3.57 | +0.72 | **flipped** |
| decaying | 2.07 | −0.11 | (`r[]` rises 226→275→317) |

**We have NEVER observed a strike at scale 1.0.** So "the scale causes the flip" and "something about
the strike path causes the flip" are indistinguishable on the current evidence.

**NEXT TEST (cheap, decisive):** make the scale branch ignore `isReachStriking()` — i.e. use scale
**1.0 while striking** (or drop the striking gate so both states use the same scale). Then:
- **Strike at 1.0 points CORRECTLY** ⇒ the flip is **scale-magnitude**-driven; the render cannot take
  a 3.57× hair. Answer = accept a smaller reach, or find the real length lever.
- **Strike at 1.0 STILL flips** ⇒ **the scale is innocent entirely** and the flip lives in the STRIKE
  path — i.e. in what the arm publishes. Suspect then: the idle publishes a point derived from the
  **wolf's facing** (`kArmIdleForward` ahead of the perch) while the strike publishes the **enemy's
  world `eyePos`**. Look there, not in `d_a_midna.cpp`.

#### The probe — ALBW-ARMIK (LEAVE IT IN until the bug is dead)
In `setHairAngle` (end of the segment loop), gated on `dAlbwMidnaArm_getReachPos(NULL)`, throttled 15f.
Grep `dusklight-*.log` for `ALBW-ARMIK`. Logs the **outcome**, not the intermediate math:
- `r[0..4]` — remaining-distance ladder (does the scratch chain converge or sail past?)
- `dotFwd` — tip's world `BaseX` vs. direction to target: **+1 = points at it, −1 = points away**
- `step`, `scaleZ`, settled `angZ4`, `tipPos`, `atn`

**Reading it:** `r[]` falling + `dotFwd` > 0 = aim solved correctly. `r[]` rising = chain diverging.
`r[]` clean but `dotFwd` < 0 = aim right, render misreading. **Note the trap that bit us: the software
tip and the render tip are DIFFERENT PLACES** (measured: 79 vs 288 units from the target) — `r[]`
describes the scratch chain, `dotFwd`/`tipPos` describe what you actually see.

**A probe should have been added before the first fix, and must not be stripped before the bug is
confirmed dead.** (It was stripped early once; that was a mistake and cost a round.)

#### If the next attempt also fails: REVERT-TO-BASELINE (surgical edits, NEVER git checkout)
1. `d_a_midna.cpp` `setHairAngle` — delete the `l_armReachScale` branch so reach mode chases the stock
   `l_hairScale[4]` (`{1,1,1}`); delete the IK overshoot clamp (`armRemain`, `segOffset`) and the
   ALBW-ARMIK probe block + the `dusk/logging.h` include.
2. `d_a_albw_midna_arm.cpp` — drop `kArmMaxRange` back to ~140 to match the unscaled chain.
3. KEEP everything in the "GOOD" table above that doesn't depend on the scale (pacing, motion, idle,
   free targeting, eyePos range measurement).
Result: a shorter-reach arm that aims correctly — a working, shippable art.

### (superseded) ARM AIM — KNOWN COSMETIC BUG: hair fires ~180° opposite the target
**Status:** REVERTED TO BASELINE and parked. The art is functionally correct (damage lands on the
right enemy — the collider is placed at the target independently of the hair render); only the hair's
visual direction is wrong. Two fix attempts failed; **the ruled-out ground below is the valuable part
— do not re-tread it.**

**Symptom:** with an enemy in front of the wolf, Midna's hair/hand visually extends ~opposite. The
READY idle also wants to sit above/behind her rather than forward.

**Runtime evidence (ALBW-ARMAIM probe, since stripped).** For an enemy provably in front (target
world yaw ≈ 6383 vs `linkYaw` 6275, `headAng` 6392):
- The published reach point is CORRECT (`atn - base` = world yaw ≈ 35° ≈ the wolf's facing).
- While riding, `shape_angle.y == linkYaw` exactly, and `head_angle ≈ shape_angle` within ±24°
  (never 180° apart) — so head-vs-body divergence is NOT the cause.
- `bVar4=1` in every sample of the round-4 build → fires the clamp
  `if (vec.y < 1.0f && i < 4) vec.y = 1.0f` → forces the base segments upward. **That clamp is the
  upside-down-hand-over-her-face symptom**, and it is a signature of the aim vector having negative
  z (i.e. of the round-4 patch), not an independent bug.

**RULED OUT — do not retry:**
1. **The `mHairPos` software chain is NOT the render path.** It is built with
   `YrotM(inv_head_angle) · XrotM(angle_z)` ([d_a_midna.cpp](../src/d/actor/d_a_midna.cpp) ~:2830),
   but the VISIBLE joints are posed by `daMidna_c::modelCallBack` (~:279-287):
   `YrotM(shape_angle.y + 0x4000) · XrotM(0x8000)` then per-joint
   `XYZrotM(0, mHairAngleY[j], mHairAngleZ[j])` — note angle_z applied about **Z**, not X. Analyzing
   the software chain is a dead end for anything visual.
2. **`shape_angle` cannot be the cause — it CANCELS.** Both the aim frame (`YrotS(-shape_angle.y)`)
   and the render base (`shape_angle.y + 0x4000`) key on `shape_angle`, so only their *relationship*
   (a constant 0x4000) survives the world→local→world round trip. The companion-vs-riding 0x8000
   difference in `shape_angle` therefore has no effect on aim direction. Corollary: patching the aim
   alone (round 4) breaks the relationship to −0x4000 (→ the clamp/idle bug); patching **both**
   (round 5) restores 0x4000 = **behaviourally identical to stock** (verified in play: idle returned
   to baseline, flip unchanged).
3. The 90° in the render base (`+0x4000`) is almost certainly the deliberate **axis remap** that makes
   a Z-rotation there equivalent to the X-rotation semantics the aim math computes — i.e. stock is
   self-consistent by design.

**Where a future attempt should look:** not at the shape/head yaw frames. Candidates: the hair bones'
REST axis in the Midna bmd vs. what the angle formulas assume; `XYZrotM`'s rotation ORDER; the
sign/handedness of `mHairAngleY` in the render (`YrotM(-mHairAngleY[j-1])` un-rotates the parent —
verify that hierarchy unwind is correct when the chain is scaled 3.57×); or whether the riding
seat matrix (`getWolfMidnaMatrix` + `ZXYrotM(-0x4000,-0x4000,0)`) leaves `J3DSys::mCurrentMtx` in a
handedness the root's `transS`-only rebuild doesn't account for. A probe that logs the FINAL world
matrix of `JNT_HAIR_5` (not the intermediate angles) would settle it in one capture.

**Baseline state (what shipped):** both 0x8000 patches removed; `setHairAngle`'s `atn_pos` branch and
`modelCallBack`'s hair root are stock. KEPT (all independent of the bug and playtest-approved):
500-unit length-only reach (`l_armReachScale z=3.57`, module-gated), 480/eyePos range gate
(mid-boss targeting), jab ease-out stretch, release-return, 6f inter-punch pause, the READY forward
idle rest, and the striking-vs-idle bridge API.

### NEW IDEA — slam finisher on the hair (arm) attack (user, 2026-07-13)
While **Midna's Grasp is active** and the player is **Z-targeted on a viable actor**, pressing
**Triangle/Y (the Dig bind — normally inactive while Z-targeted on a lock target)** triggers the
**`ANM_MGN*` catch/throw slam** through the hair attack — a manual finisher layered on the auto-punch
loop. Gating: arm art active + Z-lock held + the Dig input's Z-target dead slot. (Input seam: the
Dig bind dispatch; the slam anim would play on the Midna actor while the arm actor drives a heavier
strike window.)

---

*Research pass only — no code written. Source maps: three Explore sub-agents (wolf combat/charges, FA system, giant-Midna/transforms) + Lies of P Guard Regain (Samurai Gamers, GodisaGeek, Lies of P Wiki). Related memories: [[project-lop-hud]] (FA meter + wolf health bar), [[reference-bmd-reskin]] / [[armogohma-phase3-blender]] (BMD authoring pipeline for a giant model).*
