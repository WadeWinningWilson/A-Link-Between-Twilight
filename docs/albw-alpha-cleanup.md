# ALBW Alpha Cleanup — status & research log

**Purpose:** single tracking doc for the pre-alpha cleanup pass (started 2026-07-14).
Each item carries a status: **DONE** (implemented + built), **ROOT-CAUSED** (fix designed,
not implemented), **RESEARCHED** (findings recorded, awaiting decision), **DEFERRED**.

Build discipline for this thread: `build_run.bat` (RelWithDebInfo) only, GPU cache wipe
after every build, no commits unless explicitly requested — see
[build-fps-guidelines.md](build-fps-guidelines.md) and [commit-and-push.md](commit-and-push.md).

---

## 1. FA meter — no fill on guard-blocked hits — DONE (awaiting playtest; possible revert)

**Fix:** the FA sword grant in `setSwordHitVibration` now requires a clean connect:
`!i_gobj->ChkAtShieldHit()` added at [d_a_alink_cut.inc:359](../src/d/actor/d_a_alink_cut.inc).
The engine sets `AtShieldHit` on Link's attack collider for both enemy shield TGs
(`dCcS::ChkShield`, [d_cc_s.cpp:195](../src/d/d_cc_s.cpp)) and invulnerable armor-clank
surfaces (`TgHitMark 8`, [d_cc_s.cpp:551](../src/d/d_cc_s.cpp)); both paths skip `PlusDmg`,
so a guarded hit deals zero damage and now grants zero meter.

- Item-hit path needed no change: `dFocusedArts_onConnectedItemHit()`
  ([d_cc_uty.cpp:609](../src/d/d_cc_uty.cpp)) only fires after damage actually lands.
- No interaction with guard-piercing moves — Link has none (see §6, no Link attack sets
  `AtSpl >= 0xC`).
- **User may revert** depending on enemy-type feel (shielded enemies charge slower now).
  Revert = remove the one condition; nothing else depends on it.

### FA fill-rate reference (12 steps = 1 bank bar; max bank = shop tier 1–3 @ 100/300/500)

| Source | Steps/hit | Hits per bar |
|---|---|---|
| Ordon Sword | 2 | 6 |
| Wooden Sword (wood-skills outfit) | 2 | 6 |
| Wooden Sword otherwise | 0 | — |
| Master / Light Sword | 1 | 12 |
| Damaging item hit (arrow/bomb/slingshot/iron ball/type 40) | 2 | 6 |
| Wolf form | — | no fill (planned 1.5× layer still research-only) |

Cumulative full-bank: T1 = 6 Ordon / 12 MS hits; T2 = 12 / 24; T3 = 18 / 36.
Loss rules: damage / hidden-skill use / HS charge start drain **partial fill only** (banks
persist); hits while bank is full are wasted (no pre-charge); each art spend = 1 bar.
Balance watch-items for alpha: Master Sword half-rate (36 clean hits for full T3), and
items filling at Ordon rate makes bow spam better meter economy than sword play late-game.
Constants: [d_focused_arts.cpp](../src/d/d_focused_arts.cpp) (`kItemFillStep`,
`getSwordFillStep`, `kFocusedArtsFillDenominator=12`).

---

## 2. Oocoo's Return — price DONE, crash ROOT-CAUSED

**Price:** 150 → **15** rupees ([d_albw_oocoo.cpp:20](../src/d/d_albw_oocoo.cpp), `kPrice`).
Shop display + charge share the constant. Built + caches wiped 2026-07-14. **CONFIRMED by user.**

**Crash fix — DONE (2026-07-14, awaiting playtest):** `resolveDungeonEntrance()` looks up
the death stage in the warp-menu `gameRegions` table and warps to its first room entry +
first point (the same baseline the warp menu lands on). `canShowInShop` now hides the
service for unresolvable stages; `executePendingWarp` refunds + bails as a backstop. The
mislabeled arg-order comment is fixed (signature is `(stage, point, room, layer)`).
Playtest priority: die in Goron Mines → buy the service → should land at room 1 / point 0.

**Crash root cause (user report: crash warping back to Goron Mines):** the warp hardcodes
room 0 / point 0 ([d_albw_oocoo.cpp:109](../src/d/d_albw_oocoo.cpp)) and
`dComIfGp_setNextStage` performs **no room/point validation**
([d_com_inf_game.cpp:1472](../src/d/d_com_inf_game.cpp)). Per the level-editor room table
([map_loader_definitions.h](../include/dusk/map_loader_definitions.h)):

- **Goron Mines `D_MN04`: no room 0 (rooms start at 1)** → guaranteed crash.
- **Hyrule Castle `D_MN09`: no room 0** → same latent crash.
- Other 7 dungeons have a room 0, so they load — but point 0/room 0 is not verified to be
  the true entrance everywhere.

Secondary notes: the comment at the call site mislabels arg order (signature is
`(stage, point, room, layer)`); the standalone `procDungeonWarpSceneStartInit()` call runs
the dungeon warp-out demo from the shop stage — out-of-flow but not the crasher.

### Related: death-toast "Dungeon Entrance" choice is MISLABELED (RESEARCHED 2026-07-14)

The post-death warp toast's A choice ("Dungeon Entrance" when the death stage starts
`D_MN`, [d_gameover.cpp:547-548](../src/d/d_gameover.cpp)) does **not** warp to the
dungeon entrance. Choice 0 falls through `saveClose_proc` to the plain vanilla-continue
branch (`dComIfGp_setGameoverStatus(2)` at [d_gameover.cpp:485](../src/d/d_gameover.cpp))
— no `setNextStage` at all — which respawns via the restart-room data (`dSv_restart_c`,
written on door transitions), i.e. **the last door you walked through**, not the
entrance. So: **no crash risk** (restart data is always valid — the Oocoo room-0 bug does
not exist here), but the label over-promises. Only the B/Ordon choice
([:449](../src/d/d_gameover.cpp), `F_SP103` point 0 room 1) and the Shade Watcher
override ([:475](../src/d/d_gameover.cpp)) queue real stage changes.

**DECIDED (2026-07-14): option 2 — relabel the A choice to "Continue Here"** (queued for
the next code pass; no build while user is mid-playtest). The dungeon-death toast string
lives at [d_gameover.cpp:547-548](../src/d/d_gameover.cpp).

### Playtest note: Oocoo's Return + mid-boss arenas (expected behavior, not a bug)

Dying to Dangoro warps you back to **Dangoro's arena**, not the Goron Mines entrance:
the arena is its own stage (`D_MN04B`, 7 chars — survives the stage-name capture), and
`resolveDungeonEntrance` resolves it to the arena's own table entry (room 51). Behavior
is "entrance of the exact stage you died in"; boss/mid-boss sub-stages (`D_MN04A` Fyrus,
`D_MN04B` Dangoro, `D_MN01A` Morpheel, …) resolve to the arena. User verdict: fine,
arguably a feature. If parent-dungeon routing is ever wanted, it's a small
sub-stage → parent mapping on top of the lookup.

**Fix plan (agreed direction):** reuse the warp-menu baseline — on map select the warp UI
defaults to the **first room entry + first point** of the `gameRegions` table
([warp.cpp:107, 233-234, 325](../src/dusk/ui/warp.cpp)); for `D_MN04` that is room 1 /
point 0. Oocoo's Return should look up `sDeathDungeonStage` in `gameRegions` and warp to
that same baseline; unknown stage → abort + refund. User is verifying in-game whether the
first table entry is the true entrance for all 9 dungeons; any exceptions get a small
hand-authored override table on top.

---

## 3. Enemies breaking Link's guard — RESEARCHED

Decision is attacker-driven: Link classifies the incoming collider's `AtSpl` in
`checkDamageAction` ([d_a_alink_damage.inc:707](../src/d/actor/d_a_alink_damage.inc)).

- **AtSpl 9/10/11 = instant guard break** (`procGuardBreakInit`).
- AtSpl 1/6/13 (large) and 2/7/14 (huge) = guard **slip** (shove); vanilla breaks on the
  4th consecutive slip ([d_a_alink_guard.inc:285](../src/d/actor/d_a_alink_guard.inc)) —
  counter only active when shield durability is disabled.
- True breakers: King Bulblin axe (11), Ganondorf (10/11), Darknut (10), Aeralfos (10),
  Dynalfos (10). Dangoro is **not** a breaker — AtSpl 2 (Iron Boots) / 7 (else), see §5.
- ALBW parry deferral: `dShield_shouldDeferGuardBreak`
  ([d_albw_shield.cpp:1328](../src/d/d_albw_shield.cpp)) makes break attacks parryable;
  fail = 1.5× durability drain then the break still fires
  (`dShield_onFailedGuardBreakBlock`, [d_albw_shield.cpp:1333](../src/d/d_albw_shield.cpp)).

---

## 4. King Bulblin axe parry — ROOT-CAUSED (exemption is inverted)

**Finding that changes the design:** `d_a_e_rdb` **is the on-foot axe King Bulblin in every
fight it participates in** (camp `T_DESER`, `F_SP118`, castle `D_MN09`). It has one attack
collider (`field_0xe64`, `AtSpl 11` set once at
[d_a_e_rdb.cpp:1886](../src/d/actor/d_a_e_rdb.cpp)) driving both axe swing and spin attack;
its `BCK_RB_R*` riding anims are dead resource IDs — it never mounts.

The horseback joust is attacked by **different actors**: boar `E_WB` (`AtSpl 7`,
[d_a_e_wb.cpp:5436](../src/d/actor/d_a_e_wb.cpp)) and rider `E_RD` (`AtSpl 0`, never calls
`SetAtSpl`). Neither is in the guard-break set {9,10,11}, so neither ever reaches the
deferral/exemption logic; the mounted knock-off-Epona path (`procHorseHangInit`) is a
separate branch entirely.

**Therefore the `fpcNm_E_RDB_e` entry in `isMountSpearGuardBreakExempt`
([d_albw_shield.cpp:561-568](../src/d/d_albw_shield.cpp)) only ever affected the on-foot
axe fights — the opposite of its intent.**

**Fix — DONE (2026-07-14, awaiting playtest):** `fpcNm_E_RDB_e` dropped from the
exemption; `fpcNm_B_GND_e` kept (mounted Ganondorf stays instant-break). Result:
successful parries work against the axe (swing + spin) in all three fights; failed
parries still guard-break with 1.5× durability cost; horseback fights unchanged.

**Possible future layer (user note):** let some health damage slip through on failed
blocks/parries against heavy hitters — not designed yet, post-cleanup candidate.

---

## 5. Dangoro — bash→knockdown DONE; block tuning REJECTED

**Playtest update (user):** Dangoro IS parryable today — just very hard (rising-edge
parry window vs a slow telegraph). Decision: keep the difficulty, **no block/slip tuning**
(the 40.0f lever menu below is retained for reference only), and instead reward the
parry→bash loop:

**Shield bash knocks Dangoro down — DONE (2026-07-14, awaiting playtest).** In
`damage_check` ([d_a_e_gob.cpp](../src/d/actor/d_a_e_gob.cpp)), a connecting
`AT_TYPE_SHIELD_ATTACK` hit forces the hip-down knockdown (`ACTION_DAMAGE`, mode 0,
`Z2SE_EN_GOB_V_HIP_DOWN`) — the same state as the AP≥30 / 4-cut path. Checked before his
`field_0xd20` guard clank so the bash pierces his guard; gated on
`dShield_isParryCombatEnabled()` and live-combat actions only (FIGHT/ATTACK/DEFENCE — a
bash can never yank him out of ball roll, grab, or cutscene states). Design loop: bash
charges come from parries, so parrying the hard slam earns the knockdown punish.

### Reference: original block-flow research (tuning REJECTED, kept for the record)

A blocked slam costs **zero HP** — the "flatten" is knockback + input lock, plus meter
economics:

- **The big lever:** in the Dangoro arena (`D_MN04B`, via `checkMiddleBossGoronRoom`) the
  guard-slip speed is hardcoded **`40.0f`** at
  [d_a_alink_guard.inc:351](../src/d/actor/d_a_alink_guard.inc) — but **only when NOT in
  Iron Boots** (`!checkBootsOrArmorHeavy()`). Iron Boots (AtSpl 2) fall through to the
  HIO `mGuardSpeedHuge` default and decelerate 2× faster (`speed_step` 2.5 vs 1.25,
  [d_a_alink_guard.inc:384](../src/d/actor/d_a_alink_guard.inc)) — Iron Boots is already
  the less punishing block.
- Slip lock: `PROC_GUARD_SLIP` consumes input until speed chases to 0 + anim ends —
  ~32 frames from 40.0f. Dangoro chains Attack A→B, so the follow-up lands in the lock.
- Per-block costs (parry-combat on): −10% ALBW meter (−20% Hylian) + 1–2 bash charges
  ([d_albw_shield.cpp:1297-1326](../src/d/d_albw_shield.cpp)); **meter empty →
  `procGuardBreakInit()`** — that cascade is the real "instantly flattened".
  Durability: flat `drainBlock` (Ordon 18/134, Wooden 15/120, Hylian 22/160 ×1.30) — no
  huge-attack multiplier on normal blocks.
- Dangoro slam raw power is only `mAtp = 2`; single swept hit per swing (no multi-hit).

**Tuning levers, ranked by fit for "normal shielding should be survivable":**
1. Lower the `40.0f` override (guard.inc:351) — e.g. 12–18 — and/or drop the boots gate.
2. Downgrade Dangoro's AtSpl 2/7 → 1/6 ([d_a_e_gob.cpp:2213/2215](../src/d/actor/d_a_e_gob.cpp)):
   moves him to the gentler `mGuardSpeedLarge` class and skips the 40.0f override entirely
   (cleanest Dangoro-only fix, no shared-Link-code change).
3. Soften the ALBW failed-block penalties ([d_albw_shield.cpp:59-61](../src/d/d_albw_shield.cpp))
   so repeated blocks don't cascade into a meter-empty guard break.
4. Raise `speed_step` 1.25f (shorter lock at same peak speed).

**DEFERRED (user call):** attacker-side parry reaction — Dangoro has a ready knockdown
state (`ACTION_DAMAGE` hip-down, [d_a_e_gob.cpp:124-137, 258](../src/d/actor/d_a_e_gob.cpp))
that a parry could force; Darknut's ALBW bash-stagger (`albwApplyPhase*BashGuardBreak`,
`ACT_YOROKE`) is the template. Candidate pattern for extending parry reactions to other
enemies later. On-demand tap-parry (`kTestingParryReworkEnabled`) explicitly **not**
pursued for this.

---

## 6. Player breaking enemy guards — RESEARCHED (reference)

- No Link attack pierces guard at the collision level: `ChkAtNoGuard` = `AtSpl >= 0xC`
  ([d_cc_d.cpp:204](../src/d/d_cc_d.cpp)); all Link melee is AtSpl 0/1; only enemy bosses
  use 12+.
- Openers are behavioral: (a) geometry — hits outside the shield front-range arc / on
  non-shield colliders; (b) the **learn-guard system** — enemies (e_rd, e_rdb, e_rdy, e_dn,
  e_sf) only guard cut types they've been hit by (`mHitBit = cc_pl_cut_bit_get()`,
  [d_cc_uty.cpp:103-137, 347](../src/d/d_cc_uty.cpp)); the hidden skills (Back Slice,
  Helm Splitter, Mortal Draw) map to bit 0 → permanently unlearnable; (c) Darknut-specific
  cut-type shield drops (HEAD_JUMP / MORTAL_DRAW, combo ≥4, stagger) in `setBodyShield`
  ([d_a_b_tn.cpp:986-1090](../src/d/actor/d_a_b_tn.cpp)); (d) ALBW shield bash → zero-damage
  `AT_TYPE_SHIELD_ATTACK` grants Helm-Splitter punish credit + head-lock
  ([d_albw_shield.cpp:1726](../src/d/d_albw_shield.cpp)), full-bar tier vs armored Darknut.
- Naming note: `d_a_b_tn` = the Darknut (ALBW debug strings confirm); `d_a_e_dn` =
  Dinolfos (has `ACTION_WOLFBITE`).

---

## 7. Wolf d-pad arts vs guards — RESEARCHED (two distinct systems!)

**Learn-guard:** all wolf cut types (`0x2C`–`0x39`) map to bit 0 in `cc_pl_cut_bit_get` —
wolf attacks are **already unlearnable**, enemies can never add them to dodge masks.
Hardening nicety: combat Howl never calls `setCutType` (unlearnable by luck, not
construction) — one-line `setCutType(CUT_TYPE_WOLF_*)` in `procWolfHowlInit`'s combat
branch fixes that.

**BUT (playtest-confirmed by user): physical/state guards are independent of learnability
and both showcase enemies hold them AGAINST wolf specifically:**

- **King Bulblin:** `player->checkNowWolf()` is an OR-condition to enter `ACTION_DEFENCE`
  ([d_a_e_rdb.cpp:993](../src/d/actor/d_a_e_rdb.cpp)) — wolf attacking within 500 units
  from the front raises his guard. His shield is one small front sphere (r≈50) active only
  during the guard anim; his 3 body spheres are always live → a bite that lands in a guard
  gap forces `ACTION_DAMAGE` and fully drops the guard. Hence "bite opens the window,
  works more on KB".
- **Darknut (B_TN):** `checkNowWolf()` **keeps the arm shields raised** in `setBodyShield`
  ([d_a_b_tn.cpp:1025, 1064-1068](../src/d/actor/d_a_b_tn.cpp)) — a near-360° multi-sphere
  cage with no wolf exception, no wolf-bite reaction, and the human drop-conditions
  (HEAD_JUMP etc.) are unperformable as wolf. Only the unshielded rear `mSphB` spheres
  land. The ALBW guard-open window `field_0xaa2` bypasses the arm shield **only in the
  unarmored phase** (:1704); the armored-phase block (:1662) doesn't honor it.
- Collider facts: Howl = `AT_TYPE_WOLF_CUT_TURN`, AtSpl 1, atp 8, r300/h155
  ([d_a_alink_wolf.inc:3513](../src/d/actor/d_a_alink_wolf.inc)); Midna arm =
  **separate actor** `d_a_albw_midna_arm` carrying `AT_TYPE_NORMAL_SWORD`, AtSpl 2, atp 4
  ([d_a_albw_midna_arm.cpp:66-80](../src/d/actor/d_a_albw_midna_arm.cpp)) — enemies treat
  it as a sword. Neither reaches AtSpl ≥ 12, so both are shield-blocked; neither triggers
  armor clank (needs AtSpl 0/5/8). Neither enemy sets `TgShieldFrontRange` — shields block
  from all angles; rear hits work only via unshielded rear colliders.

**Ranked mechanisms to open/bypass (no decision yet):**
1. **Per-enemy drop conditions** (in-idiom, precise): Darknut — treat wolf-art hits like
   HEAD_JUMP in `setBodyShield` and/or honor `field_0xaa2` in the armored branch; KB —
   remove/qualify the `checkNowWolf()` defence trigger or force the guard cooldown on a
   wolf-art hit. Precedent: Dinolfos `ChkAtType(AT_TYPE_WOLF_ATTACK)` handling
   ([d_a_e_dn.cpp:2470](../src/d/actor/d_a_e_dn.cpp)).
2. **Guard-open-window credit** (Darknut-ready): let the Howl call
   `albwBeginGuardOpenWindow` ([d_a_b_tn.cpp:1335](../src/d/actor/d_a_b_tn.cpp)) like the
   shield bash does; needs the armored-phase block to honor the window; KB would need an
   equivalent added.
3. Geometry (widen Howl radius to catch rear spheres) — zero-risk, unreliable.
4. `AtSpl ≥ 0xC` NoGuard on the arts — **last resort**: bypasses every shielded enemy in
   the game, kills clank feedback, and AtSpl doubles as a behavior selector elsewhere.

---

## 8. Stamina recovery — RESEARCHED, tuning DEFERRED

System: [d_meter2.cpp](../src/d/d_meter2.cpp), integer units, base bar 10900, wall-clock
100 ms ticks. Base regen 109→273 units/100ms with expansion (`kALBWRecoveryBase`,
`kALBWRecoveryPerStep`); ~10.0 s → ~15.5 s empty-to-full; idle ×1.05; zero regen while
guarding; lockout track 7.0 s base + 3.0 s expanded (Z-target gated). Wardrobe tax
(`dAlbwWardrobe_getRecoveryMult`, [d_albw_wardrobe.cpp:442](../src/d/d_albw_wardrobe.cpp)):
−10%/extra sword, −15%/extra shield, outfit stack (Sumo .05 / Ordon .10 / Hero's .25 /
Magic-Deity .30/.50), floor 0.05, active only with D-pad quick-swap on + human form.

**Tuning verdict: safe — changes propagate automatically.** Wardrobe is a pure
post-multiplier on the live-computed rate applied to ALL recovery paths (normal/idle/
lockout/Z-target); zero hardcoded duplicates in code; debug overlay, HUD fill, and
parry/bash grants (fractions of max: parry +1/6, bash +5%, fail −10/−20%) all self-adjust.
Guardrails: never set base to 0 (untaxed path has no floor → soft-lock); don't raise
lockout ticks into the thousands (integer division → 0); keep `kALBWRecoveryPerStep`
< ~6,800 (int32 overflow). Fixed +36 expansion-celebration fill intentionally does not
follow tuning. Docs-only staleness: §2 table + §3 parry-seconds line in
`Interconnected Chats/Quick-Resistance Work.md`.

---

## 9. Spins + wolf arts vs physical guards — P0 IMPLEMENTED (2026-07-14, awaiting playtest)

**Scope per user decision: openers = Hurricane + Combat Howl + Midna arm ONLY.
Great Spin deliberately EXCLUDED (may be revisited)** — which is why the classifier keys
on the Hurricane *proc* (`PROC_CUT_GS_HURRICANE(_TIRED)`) rather than the shared
LARGE_TURN cut types, and on `mWolfCombatHowlActive` rather than the AT type the ordinary
wolf spin shares.

**Implemented:**
- Shared classifier `dAlbwCombat_isGuardOpenerHit()` + `kAlbwGuardOpenerWindowFrames = 40`
  ([d_albw_combat.cpp](../src/d/d_albw_combat.cpp) / [d_albw_combat.h](../include/d/d_albw_combat.h)).
- **Darknut** ([d_a_b_tn.cpp](../src/d/actor/d_a_b_tn.cpp)): the armored-phase arm-shield
  block AND the central-shield fast path now honor `field_0xaa2` (previously phase-2
  only); opener hits on any shield surface (or the facing guard-stagger branch) clank
  once and arm a 40-frame `albwBeginGuardOpenWindow` — follow-up Hurricane revolutions /
  Howl pulses / arm re-strikes land while it runs. Bash windows (90/75) unchanged.
- **King Bulblin** ([d_a_e_rdb.cpp](../src/d/actor/d_a_e_rdb.cpp)): opener hits on the
  body spheres bypass the `field_0x6c0` defence-recoil gate on a 40-frame cooldown
  (`sAlbwRdbOpenerCooldown`, reset in Create) — the spin sweep now scores on his exposed
  side spheres during ACTION_DEFENCE, throttled so a sustained Hurricane can't chain-stun.
- **Not yet extended** to P1 enemies (e_ai foot soldier, e_gob guard flag, b_gg Aeralfos)
  — pattern is ready (e_gob pre-clank branch), pending playtest of the P0 pair.

### Original research (blockers + full plan)

Great Spin / Hurricane and the wolf arts were all ineffective vs
physical-guard enemies, via two different blockers:

- **King Bulblin:** body-sphere damage is hard-gated on `field_0x6c0 == 0`
  ([d_a_e_rdb.cpp:735](../src/d/actor/d_a_e_rdb.cpp)); `ACTION_DEFENCE` sets it to 3, a
  clank to 8 — so during defence even a 360° spin overlapping his side spheres scores
  nothing. Wolf form triggers defence unconditionally (`checkNowWolf()` OR at :993);
  spins trigger it once learned (turn cuts ARE learn-bits 0x80/0x800 — unlike wolf arts).
- **Darknut:** armored-phase shield block at [d_a_b_tn.cpp:1662](../src/d/actor/d_a_b_tn.cpp)
  **ignores the ALBW guard-open window `field_0xaa2`** (only the unarmored block at :1704
  honors it); the unarmored "spin condition" (:1056-1061) actually KEEPS arms up for
  normal spins, and `(getCutType() & 4)` routes LARGE_TURN cuts to guard-stagger (:1742);
  `checkNowWolf()` force-raises arms in every phase (:1025, :1065).
- Hurricane (`PROC_CUT_GS_HURRICANE`, [d_a_alink_hurricane.inc:120-352](../src/d/actor/d_a_alink_hurricane.inc))
  is mechanically a ~300-frame looping Great Spin (same LARGE_TURN cut types, re-arms the
  AT sphere every frame) — it just clanks repeatedly. Spin AT setup never calls SetAtSpl
  ([initCutTurnAt, d_a_alink_cut.inc:87-97](../src/d/actor/d_a_alink_cut.inc)).

**Unified plan (ranked; primitive = the existing `field_0xaa2` guard-open window, i.e.
"opens guard for N frames", not "pierces"):**
1. **Darknut:** honor `field_0xaa2` in the armored block (:1662, mirror :1704); arm the
   window from qualifying hits (LARGE_TURN cuts + wolf-art AT types) parallel to
   `albwTryApplyBashGuardBreakFromHit` (:1406) with a SHORTER window than bash (30-45f,
   self-limiting vs Hurricane re-hits); let an active window win over the
   `checkNowWolf()` force-raise in `setBodyShield`.
2. **King Bulblin:** relax the `field_0x6c0` body gate (:735) for opener attacks with a
   short cooldown window (preferred), or exclude opener cut/AT types from the defence
   trigger like HEAD_JUMP already is (:986).
3. **Extend to other guard enemies** using the e_gob pre-clank opener branch as template
   ([d_a_e_gob.cpp:240-273](../src/d/actor/d_a_e_gob.cpp) — the bash→knockdown we just
   shipped): P1 = `e_ai` (Bulblin foot soldier front shield), `e_gob`, `b_gg` (Aeralfos);
   bosses P3 case-by-case.

**Openers:** Great Spin + Hurricane (`CUT_TYPE_LARGE_TURN_*`), Combat Howl
(`AT_TYPE_WOLF_CUT_TURN`), Midna arm (its actor collider). **Non-openers (keep clanking):**
normal swings, COMBO_STAB, and the plain charged spin (`CUT_TYPE_TURN_*`) — keeps
blocking readable and reserves opening for the skill tier. No save-format risk (all
runtime fields). Full coverage matrix + guardrails in the research transcript; key risks:
Hurricane re-hit needs window cooldown; scope KB changes away from boar/iron-ball states.

---

## 10. Stamina follow-up — bash grant bug FIXED; curve tuning DEFERRED

**Bash bug FIXED (2026-07-14, awaiting playtest):** `sBashAlbwGranted = false` is now
re-armed per spent swing in `dShield_tryBeginGuardAttack`
([d_albw_shield.cpp](../src/d/d_albw_shield.cpp)) — every connecting bash pays the +5%
again. **Grant kept at 5% per user decision** — feel it unbugged before touching
magnitude or regen rates.

**DEFERRED (user):** recovery-rate retune (`kALBWRecoveryPerStep` candidates below), and
a new idea: **+10% regen speed when NO shield is currently equipped**, applied at the
root (inside/just after `computeALBWRecoveryRate`) so the wardrobe multiplier stacks on
top of it.

### Original research

**BUG: the bash-connect stamina grant fires once per SESSION.** `grantBashAlbwOnce`
([d_albw_shield.cpp:583-595](../src/d/d_albw_shield.cpp)) is gated by `sBashAlbwGranted`,
which is only reset in `dShield_resetSession()` — called solely from
`dComIfG_resetToOpening()` (reset-to-title). The flag exists to de-dupe the two connect
call paths for one hit (`pollGuardAttackHit` per-frame + `onGuardAttackConnect` per-proc),
but nothing clears it per swing — so the +5% grant pays out once, then never again.
**Fix:** reset the flag at the start of each bash (`dShield_tryBeginGuardAttack` :1468 or
`procGuardAttackInit`); **magnitude knob:** `BASH_ALBW_HIT_GAIN_NUM/DEN` (:62-63,
currently 5/100) — user wants bashes to reward more, so likely raise alongside the fix.

**Recovery-at-max:** single-constant tune, `kALBWRecoveryPerStep`
([d_meter2.cpp:197](../src/d/d_meter2.cpp)); rate = 109 + expansion·perStep/31496.
Time-to-full today drifts 10.0s → 15.5s. Candidates: **315** = flat ~10.0s at EVERY
expansion level (elegant); **421** = upgrades speed recovery 10→8s; **244** = modest
15.5→12s. All ~13× under the int32 guardrail; base feel unchanged; wardrobe/idle/lockout
track automatically.

**End-to-end bash economy (current):** parry +16.67% meter +1 charge → bash swing −1
charge, no meter → connect +5% (post-fix, per connect) → failed block −10% (−20% Hylian).

---

## Deferred queue

| Item | Notes |
|---|---|
| **Guard-openers P1 extension** | P0 (KB + Darknut) IMPLEMENTED §9; extend the opener pattern to e_ai / e_gob guard flag / b_gg after playtest. |
| **Great Spin as opener** | Excluded by user decision; classifier is proc-keyed so adding it back = one condition. May be revisited. |
| **Stamina recovery retune** | DEFERRED until bash fix is felt: `kALBWRecoveryPerStep` picks (315 flat / 421 reward / 244 modest) + NEW idea: +10% regen when no shield equipped, applied at root so wardrobe stacks (§10). |
| **`BASH_ALBW_HIT_GAIN` magnitude** | Kept 5% for now; raise only if unbugged feel is still weak. |
| **Wolf freeze-move coverage report** | User saw it fail vs King Bulblin (camp axe fight); wants practical coverage across all fights. Not yet researched. |
| **Other-enemy parry knockdown reactions** | Dangoro bash→knockdown (§5) is the first instance of the pattern; extending reactions to other enemies deferred. |
| **Failed-block health slip-through layer** | User idea (§4): some HP damage through failed blocks vs heavy hitters. Post-cleanup. |
| **Howl `setCutType` hardening** | One-liner, fold into next wolf-combat edit. |
| **FA shield-gate revert decision** | §1 — user testing enemy-type feel. |

### Playtest checklist for the 2026-07-14 batches
Batch 1 (verified in play: "fixes seem great"):
1. **Oocoo warp:** Goron Mines death → 15r service → lands room 1, no crash. ✔ (mid-boss
   deaths land in the arena sub-stage — expected, see §2 note)
2. **King Bulblin axe parry** ✔ · 3. **Dangoro bash→knockdown** ✔ · 4. **FA no-fill on clank** ✔

Batch 2 (built 2026-07-14, NOT yet playtested):
1. **Bash stamina:** every connecting bash should now visibly refill ~5% meter (not just
   the first of the session).
2. **Guard openers vs Darknut:** Hurricane / Combat Howl / Midna arm clank once then land
   damage for ~40 frames (armored AND unarmored phases); Great Spin and normal spin still
   clank throughout; bash→helm flow unchanged.
3. **Guard openers vs King Bulblin (axe fights):** same three attacks connect through his
   raised-guard recoil roughly once per 40 frames; normal swings still clank.
4. **Death toast:** A choice now reads "Continue Here" in dungeons too and behaves as
   before (last-door respawn).
