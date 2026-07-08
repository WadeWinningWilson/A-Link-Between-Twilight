# Boss Fights — Refined Gohma (Armogohma)

Canonical, self-contained record of all Armogohma (Twilit Arachnid) work in the ALBW
Dusklight port — from the boss HP bar through the beta single-eye reveal research.
Split out of [boss-fights-handoff.md](boss-fights-handoff.md) (which was becoming
Armogohma-centric); that doc now cross-references here.

**Actors:** `B_GM` (`d_a_b_gm.cpp`, giant body) + `E_GM` param 3 = `TYPE_GOMA`
(`d_a_e_gm.cpp`, floor eye). Object arc: `B_gm.arc` (model `GOMA`).

> **Custom-model delivery** (loading a modified `B_gm` model **without repacking
> the arc**) is handled by the general Custom Models system — see
> [Custom-Model-API-Work.md](Custom-Model-API-Work.md). `B_gm` is that system's
> first **Layer-B** consumer (`dusk::custom_assets::try_load("B_gm", 0x25)` in
> `useHeapInit`). This doc keeps only the Gohma-specific reveal design; the API
> internals live there.

**Overall status (2026-07-05):** HP bar + egg fix + name styling + crash diagnostic
landed but **uncommitted**; pursuit test implemented then **disabled**; beta-eye
research complete (proven in Blender), **no in-game reveal built yet**.

---

## ▶ New-chat handoff (2026-07-06)

This picks up in a **fresh chat** after a long unrelated session (runtime-toggleable
custom **audio** — now committed, not part of this fight). Start here:

- **Goal for this thread:** build the **phase-3 "true Gohma reveal"** — see [§10](#10-phase-3-true-gohma-reveal--in-game-plan)
  for the tiered plan. Recommended path is **tier 2**: split the face-plate into its
  own shape+material in Blender (already prototyped), export `B_gm_37.bmd` (res `0x25`),
  drop it in a `model_replacements/` folder (Layer-B loose-BMD — no arc repack), then at
  phase 3 fade the face material alpha→0 **+** play `eye_test.bck` **+** `eye_zoom.btk`.
  Tier 1 (`eye_test.bck` trigger only, eye emerges *through* the face) is the cheap
  first prototype if you want a fast in-engine check before the Blender round-trip.
- **Repo state:** the custom-audio feature is **committed** (`6389807a6a`, local only —
  not pushed; user pushes manually). The Armogohma work below is still **uncommitted**
  in the working tree (HP bar, egg fix, name styling, bomb-arrow diagnostic, disabled
  pursuit test). Don't `git add -A` blindly — see the "Never commit" list in
  [commit-and-push.md](commit-and-push.md); commit Armogohma work only when the user asks.
- **Also open (not phase 3, pick up if asked):** bomb-arrow crash repro + guarded-
  decrement fix; boss-name fill colour → white reset. See "Open threads" below.
- **Watch item:** user reported a possible **FPS dip** after the audio session. Likely
  the post-build GPU-cache-wipe shader-recompile transient (recovers when
  `dawn_cache`/`pipeline_cache` warm). If it persists with custom audio **active**, it's
  an audio-hot-path optimization for the **build-analysis chat**, not this fight.
- **Build/commit discipline:** `build_run.bat` (RelWithDebInfo) + wipe GPU caches after;
  read [build-fps-guidelines.md](build-fps-guidelines.md) + [commit-and-push.md](commit-and-push.md);
  **never push** unless the user says so.

---

## 0. Refined fight — design contract (Boss Refinement ON)

Locked-in gameplay contract for the refined `B_GM` fight (values in `d_albw_boss.cpp`):

| Rule | Value |
|------|--------|
| **Win condition** | Still **3 Dominion Rod statue hits** (`mHitCount >= 3`) |
| **Max HP pool** | Scaled `field_0x560` after `dAlbwHP_tryApplyTrueMaxHp()` |
| **Bow core hit** | **−4% of max HP** per eye/core arrow (`kAlbwArmogohmaBowChipPct`) |
| **Egg waves** | **4 total**, queued on downward HP cross, consumed one-at-a-time on ceiling |
| **Egg gates** | **85%, 75%, 65%, 20%** ("last cry") — `kAlbwArmogohmaEggGatePct` |
| **Floor openings** | **75%, 45%, 15%** — crossing sets `s_armogohmaPendingCeilingDrop` |
| **Post-statue HP snap** | after hit 1 → **60%**, after hit 2 → **35%** (`kAlbwArmogohmaPostStatueSnapPct`) |
| **Eye hits** | Chip HP only — do **not** directly queue eggs |

Eggs fire on the ceiling path (`b_gm_move` case 3) via `dAlbwBoss_armogohmaTryBeginEggPhase()`. Boss Refinement OFF → vanilla egg logic (`field_0x1ad5 == 2` / arrow count — see §2).

**Tuning backlog (playtest):**
- Fight can end **too quickly** if bow chip % is high vs the HP mult.
- Sometimes only **~2 egg drops** — gates skipped when a statue snap jumps HP past several thresholds in one frame; may need staggered consumption or a lower bow chip.
- Opening gates (75/45/15) queue the ceiling drop — verify the drop anim actually fires.
- Arrow damage may still feel strong — check whether the vanilla `cc_at_check` path also reduces HP outside the Refinement chip.

---

## 1. Boss HP bar — composite two-phase meter  *(IMPLEMENTED, uncommitted)*

One bottom-centre "Armogohma" bar spanning both fight actors via a normalized
`fillRatio ∈ [0,1]` — the HUD does **no** HP math.

**Files:**
| File | Change |
|------|--------|
| `include/d/d_albw_boss.h` | `dAlbwBoss_ArmogohmaBarState` gains `f32 fillRatio` (HUD's sole fill input); `current/max` demoted to lock-on/F5 info |
| `src/d/d_albw_boss.cpp` | `dAlbwBoss_armogohmaQueryHealthBar()` rewritten to emit `fillRatio` |
| `src/d/d_albw_boss_hp_hud.cpp` | `computeLayout()`; draw from `fillRatio`; early-out fix; name styling |
| `include/d/actor/d_a_e_gm.h` | `albwGetBossHitRemaining()` accessor (private `field_0xa74`) |

**Fill mapping:**
- **Phase 1 (`B_GM`)** → top half `1.0 .. 0.5`.
  - Refinement ON: `health / s_armogohmaMaxHp` (pool drains via rod snaps 60/35% + bow chip −4%).
  - **Vanilla:** `B_GM.health` is a constant **500** (written once at create, never decremented) — so progress is driven off the Dominion Rod counter `mHitCount` (0→3): `1.0 − mHitCount/3`.
- **Phase 2 (`E_GM` TYPE_GOMA)** → bottom half `0.5 .. 0.0`, from the hit counter `field_0xa74` (3→0), **not** the eye's useless 1/1 health. `fillRatio == 0` hides the bar.
- Constants (boss module): `kAlbwArmogohmaPhase1FillMin=0.5`, `Phase2FillMax=0.5`, `Phase1RodMax=3`, `Phase2HitMax=3`.

**HUD (`d_albw_boss_hp_hud.cpp`):** geometry all comes from `computeLayout()` (fractions `kBarWidthFrac` etc.); fill = `layout.barW * fillRatio`; **early-out changed to `fillRatio <= 0`** (was double-gated on raw current/max, which hid a half-full composite bar).

**Transition behaviour:** bar is hidden across the 3rd-rod demo (eye not spawned); vanilla is continuous (drops to 0.5 then hidden then 0.5); refinement steps to ~0.675 then hidden then 0.5. Boss-HP ×4 drains phase 1 slower, phase 2 still exactly 3 hits.

**Name styling** — reversible `#define D_ALBW_BOSS_BAR_INTRO_STYLE 1`:
- Name string → **"Twilit Arachnid Armogohma"** (Armogohma only).
- Font → `mDoExt_getRubyFont()` (the boss-intro card font from `dMsgScrnBoss_c` / `zelda_boss_name.blo`).
- Colour → cream/gold `kNameColorTop=0xF6E8B0FF` / `kNameColorBot=0xE0B84AFF`.
- **PENDING:** user asked to reset the fill colour to **white** (keep font+name) — *not yet applied*. Change the `#if D_ALBW_BOSS_BAR_INTRO_STYLE` colour branch to `0xFFFFFFFF`.
- Note: font+colour apply to **both** boss bars (Armogohma + Hero's Shade) — they share one `J2DTextBox`.

---

## 2. Vanilla egg-spawn fix  *(IMPLEMENTED, uncommitted)*

Vanilla Armogohma lays eggs (baby Gohmas) when arrows run low (`dComIfGs_getArrowNum() <= 3`) — an arrow-refill mechanic. In ALBW arrows are never the resource (the meter is), so the count sits ≤3 forever → **endless eggs**.

**Fix** ([d_a_b_gm.cpp](../src/d/actor/d_a_b_gm.cpp) ~503): dropped the `arrowNum <= 3` trigger from the vanilla (`!Refinement`) branch; kept the post-rod batch (`field_0x1ad5 == 2`). Refinement fight uses its own HP-gated egg system (`dAlbwBoss_armogohmaTryBeginEggPhase`) and is untouched. Console `#else` path unchanged.

---

## 3. Bomb-arrow crash — diagnostic + hypothesis  *(DIAGNOSTIC in place, root cause unconfirmed)*

**Symptom:** crash when Link, in the **sumo outfit**, is hit by the eye laser while drawing a **bomb arrow**. Not in vanilla.

**Path:** eye laser damages Link via the standard AT/CC system (`mBeamSph`, unmodified). The fault is Link-side: `daAlink_c::deleteArrow()` ([d_a_alink_bow.inc](../src/d/actor/d_a_alink_bow.inc) ~157), which fires on the damage-interrupt branch (`mDamageTimer == mInvincibleTime || checkModeFlg(8)`). Two suspects:
1. **Inventory underflow (most likely):** the ALBW bomb arrow is meter-based (`dMeter2_canALBWBombArrow`), so `mSelectItemId` count is 0, but the interrupt path still runs `dComIfGp_addSelectItemNum(mSelectItemId, -1)` / `setItemArrowNumCount(-1)` **unguarded** → decrements 0.
2. **Use-after-free:** a dangling arrow actor kept in `mItemAcKeep`.

**Diagnostic (in place):** writes `Documents/dusklight/albw_bombarrow_debug.txt` (the mod's `fopen` log convention). Captured non-crash calls show `arrow_p` = **valid stable pointer** (no UAF evident) with `selNum=0` on the interrupt path (consistent with the underflow theory). **Actual crash not yet captured** — needs the sumo+laser repro.

**Fix direction (when confirmed):** guard the two decrements under `#if !TARGET_PC`, and/or validate `mItemAcKeep` before deref. Remove diagnostic after.

---

## 4. Cut moves

### 4a. Giant cut locomotion anims (beta tunnel-crawl)
`GOMA_MOVE`(22), `GOMA_SLOW_MOVE`(25), `GOMA_STEP_L/R`(26/27), `GOMA_UP/UP_02`(28/29) — **define-only, never `anm_init`'d**. GDC 2005 showed Armogohma crawling a tunnel; retail uses dash/beam/egg/landing only. `STEP_L/R` = leg-by-leg walk; `UP*` = climb. **`GOMA_MOVE` confirmed floor-upright** (from the pursuit test below). These are art with no AI — recreation is authoring, not un-gating.

### 4b. `E_GM` eye pursuit AI — cut, type-gated *(never documented before this session)*
The eye actor ships a complete Link-pursuit: `normal_wait` → `chaseCheck()` → `normal_chase` ([d_a_e_gm.cpp](../src/d/actor/d_a_e_gm.cpp) ~1154), tuning `player_walk_pursuit_speed=15`. `chaseCheck()` commits when **Link's back is turned** (angle-to-gohma vs `shape_angle.y` ≥ `0x4000`), within Y/XZ range — a stealth creep. **Gated behind `TYPE_NORMAL` (param 2), which Armogohma never spawns** — eggs are `TYPE_0` (wander), the boss eye is `TYPE_GOMA` (flee). Test it by spawning `E_GM` param 2, or flip the egg `createChild` param `0`→`2` at [d_a_b_gm.cpp](../src/d/actor/d_a_b_gm.cpp) ~666 so the babies chase.

---

## 5. Giant floor-pursuit TEST  *(IMPLEMENTED then DISABLED)*

Recreated the cut giant chase as a self-contained state, gated behind `#define D_ALBW_ARMO_PURSUIT_TEST` in [d_a_b_gm.cpp](../src/d/actor/d_a_b_gm.cpp). **Currently `0` (off) — the whole machinery compiles out; set `1` to restore.**

- **State:** `ACTION_PURSUIT_TEST` (12) + `b_gm_beginPursuitTest()` / `b_gm_pursuit_test()`. Snaps the giant upright (`angle.x=0`, floor height `field_0x73c.y=0`), steers `angle.y` at Link, and lets the shared `speedF` integrator at the tail of `action()` drive forward motion. `mPlayerDistance > 350` gate so it doesn't bulldoze.
- **Modes / triggers (all TEST):** hookshot → **DASH** (`GOMA_DASH`, full speed); double clawshot / boomerang → **CRAWL** (`GOMA_MOVE`, 0.35× speed); slingshot/arrow(temp) → **EYETEST** (`ANM_EYE_TEST` in place). Hookshot vs double clawshot split by equipped item (`getReadyItem() == dItemNo_W_HOOKSHOT_e`), since both fire `AT_TYPE_HOOKSHOT`. Needs `#include "d/actor/d_a_alink.h"`.
- **Playtest results:** DASH pursuit = "terrifyingly perfect," pinpoint Link tracking; `GOMA_MOVE` crawl **renders floor-upright**. Diagnostic `albw_armo_pursuit_debug.txt` showed **the slingshot never registered on the body sphere** (pachinko pellet not caught by the body `Tg`) — hence the arrow fallback for the eye-test.

**Collider inventory** (why the chase can hurt Link) — `dCcD_SrcSph` at [d_a_b_gm.cpp](../src/d/actor/d_a_b_gm.cpp) ~2295:
| Collider | Anchor joint | AT-type | Damages Link? |
|---|---|---|---|
| `mCoreSph` (eye) | `0x15` | `0x0` | No (receive-only weak point) |
| `mBodySph[0]` face / `[1]` behind | `6` / `2` | `0x0` | No |
| `mFootSph[8]` legs | per-leg | `AT_TYPE_CSTATUE_SWING` | **Yes (~¼ heart)** |
| `mHandSph[2]` claws | — | `AT_TYPE_CSTATUE_SWING` | Yes |
| `mBeamSph` | — | `AT_TYPE_CSTATUE_SWING` | Yes (the laser) |

To make a body-slam hurt, give `body_sph_src` an `AT_TYPE_CSTATUE_SWING` (enlarging alone does nothing — body spheres have no attack type).

---

## 6. Eye rig & guard mechanic (code)

All in `d_a_b_gm.cpp`. **Joint names** (from `B_gm.h`): `0x15 = EYE`, `0x16 = EYELIDT`, `0x17 = EYELIDU`.

- **Eye hitbox + lock-on `eyePos`** anchor to joint `0x15` + `JREG_F(8–15)` offsets (~line 1905).
- **Guard:** when `mInvincibilityTimer != 0` **or** `field_0x1ad6 == 0`, the core sphere is teleported `+20000` Y (unreachable) and the face body sphere covers it (~line 1910). `field_0x1ad6` = the "vulnerable" flag, set `1` only during the beam (`b_gm_beam`), reset `0` every frame in `action()`.
- **Eyelid actuator:** `field_0x1ad6 != 0` → `field_0x1ad8` opens (`6000`), else closes (`−3900`) (~line 2176); applied to joints `0x16 (+)` / `0x17 (−)` in `nodeCallBack`. **So the "eye cover" is a real eyelid that opens on vulnerability.**
- **Eye look-at:** joint `0x15` rotated `Y(field_0x1ada) + Z(field_0x1adc)`; aim math (~2041) measured against the **local Y axis** (`atan2s(z,y)` pitch, `atan2s(x,y)` yaw), clamped `±0x1400`. Unusual axes → the eye was rigged to look along a different axis than the retail spider-face points.
- **Twitch:** `field_0x1ade` (set `60` on the eye-reveal demo) adds decaying sinusoidal jitter to the aim — the "orange spec twitching."
- **Pupil dilation:** `field_0x1ae0` (`31`↔`0`) drives the `EYE_ZOOM` BTK (`mpZoomBtk`).
- **`eye_check` (field 0x49):** vestigial — set `false` at init, never read.

---

## 7. Beta single-eye creature — theory + code/asset evidence

**Hypothesis:** retail Armogohma is a beta single-eye ("cyclops") Gohma with a spider face/carapace grafted over the same skeleton; the back "eye cover" is the beta eyelid, and `eye_test.bck` is the beta reveal that slides the eye to the face.

**Evidence:**
- **Non-spider anatomy:** the skeleton is a spider (`BODY`, `BELLY1-3`, 8 `LEG*`, `HASAMIL/R` pincers on `UDE` arms, `OAGO/AGO` mandibles) **plus one giant `EYE` with two `EYELID`s**. Real spiders' eyes are the small dots on the face — a single eyeball with eyelids is not spider anatomy.
- **Self-contained eye subsystem** (look-at + twitch + dilation + eyelids) over-built for "a weak point on the back."
- **Odd orientation:** eye joint aim referenced to the local Y axis; rest matrix rotated ~78°.
- **Vestigial `eye_check`.**
- **Resources** (`B_gm.h`): `0x25 = BMD_GOMA`, `0x6 = BCK_EYE_TEST`, `0x28 = BTK_EYE_ZOOM`.

---

## 8. Model inspection — Blender / SuperBMD  *(PROVEN)*

Extraction at `D:\XXXXXXX\Ex TP\Blender workflow\B-gmarc` (SuperBMD → DAE). Driven live via the **Blender MCP addon on port 9876** (raw JSON socket: `execute_code` + `get_viewport_screenshot`, spoken to by a small Python client — no interactive Blender needed).

**`GOMA` model = 4 separable meshes / materials:**
| Mesh | Skin joints | Material / texture | Part |
|---|---|---|---|
| mesh-1 | 1 (`eye`) | `eye_mat01` / `goma_eye01` (iris) | **Eyeball** (round reptilian iris) |
| mesh-3 | 1 | `eye_mat02` / carapace tex | 4 small secondary eyes |
| mesh-2 | 64 | `body_mat` / `goma2_body01` | Whole shell: face + mandibles + 8 legs + body |
| mesh-0 | 35 | `hair_mat` / `goma2_hair` | Abdomen fur |

**Key facts proven live:**
- The eyeball is its **own mesh on one joint** — a standalone single eye.
- The **eyelid geometry is welded into `mesh-2`** (verts weighted to `eyelidT/U`, ~87 verts) but cleanly selectable by vertex group. Hiding it **reveals the eyeball sphere in the socket**.
- The **spider face is NOT its own material** — it's part of the 64-joint `mesh-2` (shared with legs/body). So in code you cannot hide *only* the face; that needs a Blender vertex split. The front face-plate that specifically caps the eye is a small central patch (~41 verts, mostly `body` group, world Y ≈ −165..−316, centred (15,−257,235)).
- Textures: eyeball `goma_eye01`, body/face/legs `goma2_body01`, abdomen `goma2_hair`, plus `goma_kankyo` (env) and `eye_enbos` (eye emboss).

---

## 9. `eye_test.bck` — the reveal animation (parsed)

Parsed directly (J3D `ANK1`, length 40 frames). The eye/eyelid joints are **relocated** (their tracks hold a moved local transform, not rest):

| Joint | rest local ≈ | `eye_test` local | effect |
|---|---|---|---|
| `eye` (0x15) | (214, 11) rot ~78° | **(318, −31, 0), rot ~0** | slides forward to the face, reorients to face front |
| `eyelidT` (0x16) | — | (317, −38, 0), **Z +18°** | upper lid opens |
| `eyelidU` (0x17) | — | (315, −37, 0), **Z −22°** | lower lid opens |

So `eye_test.bck` **is** the beta reveal: it drags the eye+lids to the mouth/face and opens them. Applied in Blender, the eye head moved from world (0, 31, 308) → (0, −73, 265) — forward and down toward the mandibles.

**Blender proof shots** (in the Gohma output folder): with the front face-plate removed, the eye relocated per the BCK, lids opened, and the eyeball oriented via its UV iris-axis to face front — a single reptilian eye stares straight out of the face where the spider mask was, in full colour.

---

## 10. Phase-3 "true Gohma reveal" — in-game plan

The reveal is *exposing shipped content*, not authoring it. The one wall: **the spider face is welded into the same shape/material as legs+body**, so a naive hide/fade affects the whole shell.

**Tiers (cheapest → cleanest):**
1. **Code-only:** trigger `eye_test.bck` on `B_GM` at a scripted beat — the eye slides to the face and the lids open using the shipped anim. *Caveat:* the carapace face is still present, so the eye **emerges through** the face rather than the face vanishing. Creepy, beta-ish, but not the clean "mask gone" look. Good as a first prototype.
2. **One modified model (recommended):** in Blender, split the face-plate into its **own shape + material** (prototyped this session), re-export BMD via SuperBMD. Now the face is separately addressable → hide/fade **only** it at runtime. **No repack needed** — drop the modified `B_gm_37.bmd` (0x25) into a `model_replacements/` folder and the Layer-B loader picks it up (see [Custom-Model-API-Work.md](Custom-Model-API-Work.md)).
3. **Full model swap** for phase 3 — heaviest, unnecessary if tier 2 is done.

**Runtime levers (J3D, via the boss's `mpModelMorf`):** play a BCK (`eye_test.bck`), suppress a shape/material (no-draw flag or **material alpha → 0**, selective only with tier 2), and BTK texture anim (`eye_zoom.btk` = pupil dilation).

**Texture replacement mid-fight:** can't delete geometry, but a **material alpha fade dissolves the face** — *if* the face is its own material (tier 2). Vanilla it shares one material with the whole shell, so it'd dissolve legs+body too.

**Ideal phase-3 sequence (tier 2):**
1. Blender: face-plate → own shape+material; re-export; repack arc.
2. In-game phase 3: fade the face material alpha → 0 (mask dissolves) **+** play `eye_test.bck` (eye slides forward, lids open) **+** `eye_zoom.btk` (pupil dilates).
3. The spider face melts away and the single beta eye emerges and locks onto Link — a transformation built almost entirely from shipped assets.

**Caveats / open design:**
- SuperBMD round-trip fidelity is the main testing burden; the modified model must load/skin correctly in-engine. (Arc repack is no longer needed — Layer-B loose-BMD delivery.)
- Timing the alpha fade + BCK + BTK is tuning.
- The beta eye is on **`B_GM`** (giant); the retail phase-2/3 weak point is the separate **`E_GM`** floor eye — decide whether the reveal *replaces* or *precedes* the existing eye phase.

---

## 11. Phase-3 damage model & defense tuning  *(verified 2026-07-07; base 50% divisor)*

**Design:** reveal fires on the **2nd statue hit** (not 3rd). HP is set to **35%** there (existing `kAlbwArmogohmaPostStatueSnapPct[1]`), then the **ground-chase phase 3** drains that 35% → ~5% via real weapon damage, at which point the existing disappear cutscene (`mMode=20 / mDemoMode=30`) is **relocated** to fire here and hand off to the `E_GM` floor eye (3-hit finisher). A **phase-3-only defense divisor** (base ÷2 = 50%) tames incoming damage so a finisher can't one-shot the sliver pool. Handoff at ≤5% (not exactly 0) so a large last hit can't overshoot and the cutscene always fires cleanly.

### Damage pipeline (all verified in code)
- Enemy converts the attack code `atp` → HP via `at_power_get()` keyed on `mPowerType`. **B_GM never sets `mPowerType` → defaults to 0.** PowerType-0 table ([d_cc_uty.cpp:168](../src/d/d_cc_uty.cpp)): atp1→1, atp2→**10**, atp3→**30**, atp4+→**200**, atp6→80.
- **Sword tier** (melee only, [d_cc_uty.cpp:437](../src/d/d_cc_uty.cpp)): Wood **÷2**, Ordon **×1**, Master & Light **×2** (`checkMasterSwordEquip` covers both — [d_a_player.cpp:460](../src/d/actor/d_a_player.cpp)). Sword tier does NOT change raw `atp`; the multiply is applied after the power table.
- **Sword-attack-up buff** (`getSwordAtUpTime`): temporary ×2 (not tabled).
- **Focused Arts** (only when FA enabled): hidden-skill finishers scaled `(4+2·tier)/10` = ×0.4 (t0) → ×1.0 (t3); items (arrow/bomb) `(10+5·tier)/10` = ×1.0 (t0) → ×2.5 (t3). Basic/standard slashes & normal spin are **not** hidden skills → FA-immune. Max tier = 3.
- **Outfits do NOT affect damage.** `dAlbwOutfit_isActive` appears only in HUD / sumo body-swap / the recovery-penalty; the outfit "stack rates" (Sumo 0.05 … Deity 0.50) tax **heart recovery**, not attack. **"With outfit stats" == "without"** for phase-3 pacing.
- **Global Link-damage-decrease** divisor (`linkDamageDecreaseMult`) applies before ours (not tabled).

### Base per-hit damage (B_GM, PowerType 0)
| Attack | atp | Base HP (Ordon) |
|---|---|---|
| Basic slash | 2 | 10 |
| Vertical / thrust / normal spin | 3 | 30 |
| Great Spin / hidden-skill finisher | 4 | 200 |
| Arrow (bow) | 2 | 10 |
| Bomb / Bomb Arrow | 4 | 200 |

### Scenario tables — 35% pool (175 @1×, 700 @4×), base 50% defense (×0.5)

**S1 — Vanilla, no refinement:** phase 3 **nonexistent** (reveal morf gated on `dAlbwBossRefinement_isEnabled()`; `rodMax` stays 3; `health` constant 500; no pool drain). No damage table — this is the gate working as intended.

**S2 / S3-without-FA — Refinement (± outfit; outfit is a no-op):**

| Attack | Tier | Eff. dmg | Hits @1× | Hits @4× |
|---|---|---|---|---|
| Basic slash | Wood / Ordon / Master·Light | 2 / 5 / 10 | 88 / 35 / 18 | 350 / 140 / 70 |
| Standard slash / spin | Wood / Ordon / Master·Light | 7 / 15 / 30 | 25 / 12 / 6 | 100 / 47 / 24 |
| Great Spin / finisher | Wood / Ordon / Master·Light | 50 / 100 / 200 | 4 / 2 / 1 | 14 / 7 / 4 |
| Arrow (bow) | — | 5 | 35 | 140 |
| Bomb / Bomb Arrow | — | 100 | 2 | 7 |

**S3 / S4 — Refinement + FA:** FA moves only the finisher and item rows (Ordon shown):

| Attack | FA tier 0 | FA tier 3 |
|---|---|---|
| Great Spin / finisher (base 200) | eff 40 → 5 @1× / 18 @4× | eff 100 → 2 @1× / 7 @4× |
| — Master/Light finisher | eff 80 → 3 / 9 | eff 200 → 1 / 4 |
| Arrow (bow) | eff 5 → 35 / 140 | eff 12 → 15 / 59 |
| Bomb / Bomb Arrow | eff 100 → 2 / 7 | eff 250 → 1 / 3 |

Read: FA never breaks phase 3. At low tier it *helps* pacing (finisher weakened to ×0.4); at max tier finishers return to baseline and items get spicy (tier-3 bomb one-shots @1×) — which supports treating **bomb-into-the-back-hole as intentional bonus damage** rather than something to suppress.

### Tuning intent (start simple, adjust later — CONFIRMED better to defer)
- Ship Stage 2 with ONE tunable: phase-3 defense = **÷2 (50%)**. Route all phase-3 damage through a single helper so per-source multipliers slot in later with no refactor.
- Deferred, playtest-driven options: **per-source defense** (e.g. divide bombs/finishers harder than slashes), or a **reactive eye-close** — boss auto-shuts its eye vs bomb-arrows in phase 3 (input-read) **unless stunned**, making bombs situational instead of a one-shot. Both are behavior tweaks best judged after the base cycle *feels* right, so they are NOT in the first Stage 2 pass.

---

## 12. Phase-3 gameplay — implemented state machine + reveal-model fixes  *(2026-07-08)*

**Stage 1 (DONE, builds, uncommitted):** reveal model built alongside vanilla in `useHeapInit` (gated `dAlbwBossRefinement_isEnabled()` + `dusk::custom_assets::try_load("B_gm",0x25)`); heap `0x8C00→0xC800`; load-once cache + leak diagnostic in `custom_assets.cpp`; `b_gm_activateReveal()` pointer-swap; statics reset in `daB_GM_Create`.

**Stage 2 (DONE, builds, uncommitted) — the `ACTION_PHASE3` (=13) sub-state machine in `d_a_b_gm.cpp`:**
- Trigger: 2nd statue hit → `dAlbwBoss_armogohmaOnRodHit` (35% snap) → `b_gm_beginPhase3()` (rodMax=2 only when the reveal loaded).
- **P3_INTRO** — plays `ANM_GOMA_RETURN` (get-up) keeping the smashed upside-down orientation, snaps upright only on the anim's final frame, then swaps in the reveal model (`b_gm_activateReveal`) and starts the dash. Fixes the "instant-flip blows up the foot IK" model break.
- **P3_DASH** — chases Link (GOMA_DASH, **anim ×0.95** per playtest), eye blink 5s / open 5s (`kAlbwArmoP3EyeCycleFrames=300`) driving `field_0x1ad6` (gates the core sphere = vulnerable window).
- **P3_VULN** — 8s stationary stagger (`kAlbwArmoP3VulnFrames=480`), eye open, **`ANM_GOMA_WAIT`** (was `LANDING_WAIT`, which is authored for the on-back pose and sank under the floor upright). Counter resets after.
- **P3_LASER** — 10s (`600`), eye INVULNERABLE (never raises `field_0x1ad6`), faces Link (turn `0x435`, +5%), beam ignites at 3s (`kAlbwArmoP3LaserFireAt=180`) by ramping `field_0x6c0` (0.105 rate) — reuses the vanilla beam-render (fires from eye joint 0x15 = the mouth on the reveal model, tracks Link with lag via `field_0x6c8`). NOTE: `eye_test.bck` is deliberately NOT played on the reveal model (it yanks the baked eye off the mouth); a neutral hold pose is used.
- **Single hit counter** `s_gmPhase3HitCount` (damage_check phase-3 block): high threshold first — `≥5` → VULN (reset after), else `>3` → LASER (retained). Only counts in P3_DASH; entering either shuts/gates the eye so a burst can't multi-trigger.
- HP drains through `dAlbwBoss_armogohmaPhase3Damage()` (÷2 defense, `kAlbwArmogohmaPhase3DefenseDiv`) whenever the eye is open; `≤5%` → `b_gm_beginPhase3Handoff()` relocates the vanilla `mMode=20/mDemoMode=30` disappear cutscene → E_GM eye.

**Frame constants assume 60 fps — confirm on playtest.** Timing: cycle 300, vuln 480, laser 600/fire 180.

### Reveal-model bugs — RESOLVED in sequence (2026-07-08)
Three separate defects, each fixed and playtest/offline-verified:

**(a) 180° inverted — FIXED (playtest-confirmed upright).** The reveal scene's objects (meshes +
`skeleton_root`) carried a **+90°X `matrix_world`**, and the SuperBMD recipe *also* passed `--rotate`
→ two 90° rotations → 180° flip. Fix: apply the +90°X object transform in Blender (armature + all
meshes together) so `matrix_world` is identity, then re-export with the same `--rotate` = exactly one
rotation. `D_ALBW_ARMO_P3_DIAG_NOSWAP` flipped back to `0`. (Details: `memory/armogohma_phase3_blender.md`.)

**(b) Eye + eyelids snapped to the dorsal socket (mouth left a gaping hole) — FIXED (code anchor).**
The BMD correctly bakes the eye/lids at the mouth (JNT1 eye 0x15 local = (455.6,−62,−7.1) rotZ −78.5°;
vanilla = dorsal (214.3,10.9,0)). But **Nintendo BCK/ANK1 anims carry TRS tracks for every joint**, so
each shared body anim drove the eye/lid joints to the vanilla dorsal each frame, and `nodeCallBack`
only layered rotations, never re-asserting the mouth translation (the old "no body anim translates
these joints" assumption was untested and wrong). Fix: `b_gm_revealAnchorMouthJoint()` in
`d_a_b_gm.cpp` rebuilds joints 0x15/0x16/0x17 (reveal model only) as `getAnmMtx(0x6) [body] × baked
mouth-local`, so they ride the animated body but ignore the BCK's dorsal translation.

**(c) Legs shattered under animation — FIXED (binary "reskin", offline-verified).** The Blender
round-trip re-derived every bone's local frame (positions survive to 0.3 u, frames don't; rigid verts
are joint-local, so they weld to the wrong frames → shared vanilla anims tear long joint chains). Body
/eye frames happened to match vanilla (0.0° diff) so only the legs broke. Neither Blender nor SuperBMD
(`--transform_mode` is irrelevant) can reproduce vanilla frames → fixed post-export with
**[`tools/bmd_reskin/bmd_reskin.py`](../tools/bmd_reskin/bmd_reskin.py)**: transplant vanilla's frames,
re-express each rigid vertex/normal by `targetWorld_J⁻¹·sourceWorld_J`, rebuild EVP1 inverse-binds.
Verified offline — rest-position drift **0.0032 u**, frame-match to vanilla **0.0001** ⇒ mesh intact +
animates like vanilla. Deployed as a loose-BMD swap (no rebuild). Full writeup:
**[BMD-Reskin-Tool.md](BMD-Reskin-Tool.md)**. This is a **reusable precedent for any Blender-edited
rigged model reusing original animations.**

**(d) Eye rendered as a pure white orb — FIXED (binary texture inject, playtest-confirmed).** The eye
material is environment-mapped: TEV stage 1 = `(TexMap1·iris + TexMap1.alpha)×2`. Vanilla's TexMap1 is
`goma_kankyo` (a low-alpha white reflection sphere) → soft highlight. SuperBMD only embeds the
mesh-diffuse textures, so the env map + emboss (`eye_enbos`) were dropped and redirected to the
**opaque** `goma_eye01` (alpha=1) → stage 1 = `(…+1)×2` → clamps to pure white. Neither Blender nor
SuperBMD (`--transform_mode` irrelevant) can embed the non-diffuse maps. Fixed post-export with
**[`tools/bmd_reskin/bmd_addtex.py`](../tools/bmd_reskin/bmd_addtex.py)**: copies `goma_kankyo`+`eye_enbos`
from vanilla `goma.bmd` into the reveal BMD's TEX1 (both are the last sections, so rebuildable) and
repoints the eye material's texNo slots to `[eye01, kankyo, enbos]`. Verified offline (5 textures, eye
resolves to those indices, injected bytes match vanilla, reskin still intact). Result: authentic shiny
env-mapped orange iris at the mouth. See [BMD-Reskin-Tool.md](BMD-Reskin-Tool.md) §Companion.

### Playtest findings still open
- HP bar drain / eye sword-vulnerability + `÷2` defense feel — re-validate now that the eye is at the
  mouth (front, hittable head-on).
- Optional polish: eye size (a ~5% shrink or eyelids-follow-eye was floated) — reassess now that it's a
  proper iris rather than a glaring white blob; deferred pending a look.
- Confirm the P3_VULN `WAIT` pose reads as "weak" (may want a distinct upright stun / OoT iris-spin).
- Deferred: eye_test literal-anim option, per-source defense, reactive bomb-arrow eye-close, 60 fps
  timing confirm, beam aim on the ground model.

---

## Code / file state summary

| File | State |
|---|---|
| `d_albw_boss.h/.cpp`, `d_albw_boss_hp_hud.cpp`, `d_a_e_gm.h` | HP bar composite meter — **implemented, uncommitted** |
| `d_a_b_gm.cpp` (egg branch) | Vanilla egg fix — **implemented, uncommitted** |
| `d_albw_boss_hp_hud.cpp` (`D_ALBW_BOSS_BAR_INTRO_STYLE`) | Name styling — implemented; **white-colour reset pending** |
| `d_a_alink_bow.inc` (`deleteArrow`) | Bomb-arrow diagnostic — **in place** (remove after crash pinned) |
| `d_a_b_gm.cpp` (`D_ALBW_ARMO_PURSUIT_TEST`) | Giant pursuit test — **implemented, DISABLED (define 0)** |
| `d_a_b_gm.cpp` (`nodeCallBack` / `b_gm_revealAnchorMouthJoint`) | Phase-3 eye/lid mouth anchor — **implemented, uncommitted, playtest-confirmed** |
| `B_gm_37.bmd` (reveal model) | Orientation-fixed + **reskinned** (vanilla frames) → legs animate; **deployed to `model_replacements/`** |
| `tools/bmd_reskin/bmd_reskin.py` (+ `B_gm_37_prereskin_SOURCE.bmd`) | Reusable BMD reskin tool — see [BMD-Reskin-Tool.md](BMD-Reskin-Tool.md) |
| `tools/bmd_reskin/bmd_addtex.py` | Companion: inject donor textures into a BMD + repoint a material (restored the eye env/emboss) |
| Diagnostics | `albw_bombarrow_debug.txt`, `albw_armo_pursuit_debug.txt` |

## Open threads / next steps
- [ ] Reproduce the bomb-arrow crash (sumo + laser) → read `albw_bombarrow_debug.txt` tail → land the guarded-decrement fix.
- [ ] Reset boss-name fill colour to white (keep ruby font + name).
- [ ] Decide on the pursuit test (keep behind toggle / promote to a real move / drop).
- [x] Phase-3 reveal model: upright (orientation), eye/lids anchored at the mouth, legs animate (reskin). See §12.
- [ ] Phase-3 reveal eye polish: restore env/emboss shading (kill the white orb), ~+3% eye shrink, then re-run the reskin from the updated Blender export.
- [ ] Commit the HP-bar / egg / name work (strip diagnostics + pursuit-test probes first, per build guidelines) when the user asks.
