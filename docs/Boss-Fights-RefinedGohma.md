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

## Code / file state summary

| File | State |
|---|---|
| `d_albw_boss.h/.cpp`, `d_albw_boss_hp_hud.cpp`, `d_a_e_gm.h` | HP bar composite meter — **implemented, uncommitted** |
| `d_a_b_gm.cpp` (egg branch) | Vanilla egg fix — **implemented, uncommitted** |
| `d_albw_boss_hp_hud.cpp` (`D_ALBW_BOSS_BAR_INTRO_STYLE`) | Name styling — implemented; **white-colour reset pending** |
| `d_a_alink_bow.inc` (`deleteArrow`) | Bomb-arrow diagnostic — **in place** (remove after crash pinned) |
| `d_a_b_gm.cpp` (`D_ALBW_ARMO_PURSUIT_TEST`) | Giant pursuit test — **implemented, DISABLED (define 0)** |
| Diagnostics | `albw_bombarrow_debug.txt`, `albw_armo_pursuit_debug.txt` |

## Open threads / next steps
- [ ] Reproduce the bomb-arrow crash (sumo + laser) → read `albw_bombarrow_debug.txt` tail → land the guarded-decrement fix.
- [ ] Reset boss-name fill colour to white (keep ruby font + name).
- [ ] Decide on the pursuit test (keep behind toggle / promote to a real move / drop).
- [ ] Phase-3 reveal: start with tier-1 (`eye_test.bck` trigger) prototype, then tier-2 model split if the look warrants.
- [ ] Commit the HP-bar / egg / name work (strip diagnostics + pursuit-test probes first, per build guidelines) when the user asks.
