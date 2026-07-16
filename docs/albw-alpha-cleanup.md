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
| **Wolf freeze coverage gaps** | Coverage report DONE (round-3 §1); remaining levers: blocked-hit no-freeze tradeoff, per-actor frozen hurt-sphere refresh, non-Zant boss audit. |
| **Wolf charge HUD toggle compliance** | **DONE 2026-07-14 (built, awaiting playtest):** new exported mode verdict `dShield_isParryHudPinned()` (ParryAlways/BothAlways of `ShieldHudVisibility`; mode-only, no shield-possession conditions) consulted once per frame in `dAlbwWolfChargeHud_draw` — pinned modes hold the wolf pips open; Off keeps the wolf-form summon/linger (shield button / enemy lock-on + 120f), the wolf analogue of guard+linger. DurabilityAlways correctly leaves wolf pips on linger (follows PARRY icons only). |
| **13 strippable items pricing** | Research DONE + user tweaks IMPLEMENTED 2026-07-14 (built): Slingshot 15 / Boomerang 30 / Bombs 50 / Bomblings 50 / **Water Bombs 150** / **Dominion Rod 150** / **Clawshot 100** / Bow 150 / Dbl Clawshot 200 / Spinner 200 / Ball&Chain 350 / Magic Armor 500 / Deity 5000-per-session (`kItems[]`, d_albw_rental.cpp). The 13 = `sALBWItemNos[12]` (d_meter2.cpp:721) + Deity flag; all perpetual rentals, no rent-vs-own tier. Doc gap: Deity purchase ceremony (auto-store+rollback per albw-deity-armor-shop.md) NOT implemented — flag+charge only, code-marked deferred. |
| **Magic Armor item-page row + auto-strip status** | RESEARCHED 2026-07-14 (no code yet — user decision pending). **Why strip stopped: two ownership models drifted apart.** The strip (d_meter2.cpp:842-855, unchanged since b270cb7765) works on the LEGACY model (item first-bit + worn clothes); the wardrobe/outfit system (built later) owns Magic via event bit 694 (`kStashMagic`, latched every frame by `dAlbwOutfit_syncWornOwnership` the moment Magic is ever worn, NEVER cleared by anything) + store bit 711. So: (cause 2) dying while NOT wearing Magic → worn-only ownership gate (d_meter2.cpp:832-835) makes the strip no-op entirely; (cause 1) dying while wearing it → strip clears first-bit + unequips but bit 694 keeps it in the D-pad cycle. Either way the wardrobe never loses it. **Bonus bug: 500r double-charge surface** — the row's visibility uses the worn-only test, so with Magic owned-but-stored the row shows as rentable and the player can pay 500r for an outfit they already have free. **Options (ranked):** (1-best/doc-aligned) remove the item-page row, move Magic fully to the wardrobe economy — strip parks/clears bits 694/711, re-acquire via Postman wardrobe (matches albw-deity-armor-shop.md's Magic-row-as-Store/OFF-switch design); (2-minimal) keep the row, make the strip clear bit 694+711 and the row visibility read `dAlbwOutfit_isOwned(MAGIC)` instead of worn-only; (3) hybrid context-sensitive row (doc end-state if Deity ceremony ships). |
| **Stalhound focused work** | Bite whiffs from i-frame hurt-sphere displacement + charge crediting; user flagged as its own pass. |
| **MQ shop uncap** | Future: allow continuous heart/stamina purchases past current tier caps. T20=3033 typo confirm pending. |
| **Magic Armor loss-on-death + row retirement** | **IMPLEMENTED 2026-07-14 (built, awaiting playtest)** after full cross-system research (both agent reports clean). Death now CONFISCATES Magic into Postman wardrobe storage: strip fires on wardrobe ownership (stash bit 694) not just worn (`dMeter2_stripRentalItemOnDeath` ARMOR_e branch precedes the worn-only gate), sets stored bit 711 via new gate-free `dAlbwWardrobe_storeOutfitOnDeath()`, KEEPS ownership (clearing-while-stored would orphan — Retrieve never re-grants), keeps `onALBWRentalEligible` (Deity gate reads that permanent bit), keeps the synchronous Kokiri clothes reset (defeats the per-frame syncWornOwnership re-latch). 500r kItems row REMOVED (eligibility bits are keyed by `sALBWItemNos` itemNo, NOT table position — verified no bit shift; `sALBWItemNos` untouched). Save-state only during the gameover window — no changeLink rebuild fires, so all documented sumo/cap swap-crash classes (FLG2 skip-path, initModel(NULL), demo-BCK, cap donor) are unreachable; model rebuilds fresh at respawn. **Buy-back = existing storage Retrieve row (100r).** CAVEATS flagged: (a) Retrieve requires Quick Swap ON — a Quick-Swap-OFF player who dies wearing Magic can't retrieve until enabling it (policy call pending); (b) True-ALBW mode lost its early Magic acquisition surface (story wear-once remains); (c) `game.albwMagicArmorRentableDebug` setting is now unused; (d) retrieve price is the standard 100r — separate death-buy-back price is a one-constant change if wanted. Playtest: die wearing Magic / owning-not-wearing / wearing sumo-over-Magic / capped — Magic leaves D-pad cycle, appears in Postman storage, retrieve restores it; Deity row still gates correctly. |
| **Fierce Deity → outfit tab migration** | **IMPLEMENTED 2026-07-15 (built, awaiting playtest).** Deity is no longer a kItems inventory row: new `VISIBLE_DEITY` outfit-economy row (d_albw_rental.cpp) appended in the CAT_ARMOR page next to the Sumo row while not owned. Eligibility semantics preserved exactly (Magic rental-eligible bit + Colossal Wallet, `deityRowEligible()`; still NEVER bypassed by True-ALBW; shows "?????" until eligible via showNameWhenSoldOut=false). Purchase (5000r, `kDeityArmorPrice`) sets the ability first-bit AND `dAlbwOutfit_recordOwnedByItemNo` (wardrobe stash bit 695) so the outfit systems own it — the deferred ceremony (albw-deity-armor-shop.md auto-store + rollback) bolts onto this path. Death-strip coherence: `stripAllALBWInventoryOnDeath` (d_meter2.cpp) now clears stash 695 + store 712 alongside the first-bit (same 694/711 rule as Magic). STILL OPEN: death mid-Deity restore-or-park product question; dual-mode Magic row as Deity OFF-switch (ceremony spec). Playtest: eligible save → Deity shows on Armor tab as named row, buys at 5000r, disappears once owned; ineligible → "?????"; die after buying → gone from shop-owned state, row returns. |
| **Wolf freeze on EVERY enemy actor** | STARTED 2026-07-14 — full mapping done from the TP actor spreadsheet: **[wolf-freeze-coverage.md](wolf-freeze-coverage.md)** (46 regular enemies in scope, 11 bosses + traps excluded, 11 mid-bosses awaiting per-case calls, and 3 TWILIGHT CLASSIFIER GAPS found: e_ym Shadow Insect / e_yc Carrier Kargarok / e_rdy Shadow Bulblin currently freeze when they should get ×0.70-no-freeze). Phase 1 = user decisions; phase 2 = batched per-actor code sweep; phase 3 = batched implementation. |
| **LANAYRU TWILIGHT BORDER — root-caused through 4 research passes (2026-07-15)** | FINAL STATE: user save is NON-truetest, Twilight Clear Level 2 (Lanayru bit CLEAR — no corruption; both original hypotheses REFUTED: early-wolf writes no save state; Fyrus boss actor sets no flags and no mines bit feeds F_SP121 layers). MECHANISM (empirically confirmed): F_SP121 locks ONE layer at stage entry from the START room's province; Eldin-side entry (room 7, Eldin cleared) → day layer 0 → the border wall didn't load, while base-layer twilight enemies (flag-gated, correctly uncleared) did. Warp to room 9 **point 10** layer auto → twilight layer 14 loads WITH the gate (verified in-game). Tmp-bit theory DEAD (0x0601/0x0602 = Telma-escort layers 3/2, runtime-only mTmp never saved — reload wipes them, but irrelevant to the wall). Wall pieces enumerated: dedicated LIGHT-SIDE piece `twGtK0709` (obj_twGate type 2, Lanayru-gated, self-deletes on province clear) exists in the resource table — vanilla shows it day-side, so its day-layer placement should load; why it didn't for this session is the ONE open question. **GROUND TRUTH (user's Stage Inspector, 2026-07-15): room 9 `twGate` authored ONLY on
ACTd/L13 + ACTe/L14 — no day-layer placement exists.** Content gap, not corruption; the
inspector also photographed the black-screen hang from inside (stage fully loaded, 47
actors live, only the PLYR spawn missing → hang = no player placement on resolved layer).
**FIX IMPLEMENTED (built, awaiting playtest): `d_albw_twilight_border` fallback module**
(new files src/d/d_albw_twilight_border.cpp + .h; driven per-frame from d_s_play next to
the wolf-stun bridge). STRICTLY FALLBACK per user requirement: per stay-room (60f cadence
+ room change), reads authored placements via `leveledit::enumerate_room_actors` (new
`systemUse` bypass of the editor-session gate); spawns `twGate` walls (type remapped to
the light-side twGt* variant where one exists: 6→0/7→1/8→2) and `TgTGate` entry triggers
from authored transform/params IFF: authored-in-room AND province uncleared
(`daObjTwGate_albwProvinceForType` / `daTagTWGate_albwProvinceForType`, new exported
helpers) AND no instance of that group is live (vanilla-spawned ⇒ no-op) AND not already
fallback-spawned (proc+room+500u position dedupe — L13/L14 double-author safe). Spawned
actors keep vanilla self-guards (despawn on legit clear). Border-generic + cause-agnostic;
logs `evt=fallback-spawn` to albw_tw_border_debug.txt. Playtest: original day-side route →
wall + entry trigger appear at the border; twilight-side warp → fallback stays silent
(vanilla instances live); cleared provinces → nothing. SHIPPED ALONG THE WAY: warp layer auto-reset, warp story-trigger suppression, truetest bubble self-heal (vessel-gated), vessel gating on all province-clear arrival triggers. ALSO: black-screen warp hangs = table points are LAYER-SPECIFIC (missing point on resolved layer = hang; Gerudo Desert same class) — separate warp-hazard item. |
| **WOLF ARTS FINISHING BATCH — resolved 2026-07-15** | (1) **HUD FIXED (built):** wolf charge icons never consulted the shared meter-gauge alpha (`getMeterGaugeAlphaRate(0)` — THE canonical hide signal covering cutscenes/dialogue/menus/map; the parry icons' early-out on it is their entire hide logic). Fix = fetch `dShield_getShieldHudDrawAlpha()` once per frame, early-out at <=0 (kills both the cutscene visibility AND the dark-square map artifact, which was the raw icon quad drawing over the map fade after every other HUD element stopped), + scale the three fixed icon alphas by it so they fade in lockstep. (2-4) **NO CODE BUGS — dev toggles were ON in the user's config (verified):** `wolfArtsDevTest=true` (bypasses BOTH art shop-unlock and charge cost — this IS the requested spam toggle, already in the editor ALBW tab, default off) and `albwJuniorMailTest=true` (bypasses the Junior Postman story/one-shot gates — the encounter is already save-gated on F_0601 + F_0813, the setting is a repeat-test bypass). Arts are save-gated on bits 713/714, purchases set them, dispatch checks them, rows hide once owned. USER ACTION: flip both toggles OFF in the editor ALBW tab for normal-play behavior; also `albwMagicArmorRentableDebug=true` is vestigial post-row-retirement (harmless, can be turned off / setting removed later). FOLLOW-UP FIXES (built 2026-07-15): **Postman story gate corrected** — was F_0601 "spoke to IMPRISONED Talo" (end-of-chase, dialogue-dependent — why a fresh game never armed the encounter); now F_0094 "Talo went after the monkey" (quest-start), so the encounter arms on the first North Faron visit DURING the save-Talo questline. **Art shop story gates pinned (user calls):** Wolf Howl row requires first twilight cleared (DarkClearLV 0); Midna's Grasp row requires Eldin twilight cleared (DarkClearLV 1, supersedes old Lanayru TODO). Playtest: fresh-game North Faron during Talo chase → mail encounter fires with toggles OFF; art rows absent until their twilight clears. |
| **Other-enemy parry knockdown reactions** | Dangoro bash→knockdown (§5) is the first instance of the pattern; extending reactions to other enemies deferred. |
| **Failed-block health slip-through layer** | User idea (§4): some HP damage through failed blocks vs heavy hitters. Post-cleanup. |
| **Howl `setCutType` hardening** | One-liner, fold into next wolf-combat edit. |
| **WOLF HUD ICONS INVISIBLE — regression, FIXED 2026-07-15 (built, awaiting playtest)** | Self-inflicted by the 2026-07-14 "HUD hide fix" (committed in f99a5bd6bb): the wolf charge HUD early-outs on `dShield_getShieldHudDrawAlpha()`, which returns **meter-gauge slot 0** — NOT a global HUD fade, but the **ALBW stamina meter's private alpha**, which `alphaAnimeKantera()` deliberately pins to 0 in wolf form (`dMeter2_isWolfForm()`, d_meter2.cpp:4520 — the only wolf alpha term in the file, in place since b270cb7765). **The exact condition that lets the HUD draw (`checkNowWolf()`, :136) is the exact condition that zeroes the alpha it requires** — a closed lock, 100% of wolf-form frames. The parry spurs read slot 0 safely only because `dShield_drawBashCharges()` bails out in wolf form (d_albw_shield.cpp:1882) before reaching it; that idiom does not transfer to a wolf-only HUD. Confirmed by 25-agent adversarial hunt (5 probes → 19 refutation attempts → synthesis); settings/save-bit hypotheses REFUTED structurally (empty pips draw at alpha 77 regardless of charges; the file reads no unlock bits and no dev toggle). **FIX:** follow the **rupee counter's** fade instead — `meterDraw->getRupeeHudAlphaRate()` — the row this HUD already anchors to and sizes from, and the idiom `dAlbwRupeePopup_draw()` already uses at the same call site. `alphaAnimeRupee()` (d_meter2.cpp:4577) carries the same cutscene/dialogue/menu hide set with **no wolf term**, PLUS `dMeter2Info_isSub2DStatus(1)` (the map/sub-screen) — which slot 0 lacks, so this also actually fixes the map "dark square" the first attempt only claimed to. Includes a `> 1.0f` clamp (the accessor returns an HIO product `mRupeeAlpha × mParentAlpha × mRupeeKeyAlpha` that can exceed 1.0 and wrap the u8 alpha casts — the rupee popup clamps for the same reason). `dShield_getShieldHudDrawAlpha()` left untouched (correct for the human-form spurs). LESSON: a "global HUD signal" borrowed from a human-form-only consumer must be re-validated in wolf form. Playtest: wolf icons visible in normal wolf play; hidden in cutscenes/dialogue; no dark square on map open/close. |
| **WARDROBE/ARMOR PAGE VANISHED — regression, FIXED 2026-07-15 (built, awaiting playtest)** | Self-inflicted: the same commit (f99a5bd6bb) that migrated Deity out of `kItems[]` and retired the Magic 500r row removed the last two *unconditionally-visible* CAT_ARMOR entries. Root: `categoryHasVisibleRows()` (d_albw_rental.cpp) — the gate deciding which tabs EXIST — scans only `kItems[]` + hardcoded CAT_UPGRADES/CAT_SWORDS keep-alive clauses; there was **no CAT_ARMOR clause**, so the dynamically-appended sumo/Deity/storage rows never counted toward page existence. The 3 clothes rows (Ordon/Hero's/Zora) *should* have kept it alive, but `itemRowVisible()` hides a row on `eligible && owned` (for Zora, eligibility and ownership read the *same* first-bit) — so on a save where the clothes are owned, all three self-hid, leaving zero visible CAT_ARMOR rows → tab dropped, counter 5→4. Deity/Magic (both `alwaysGated`, never "owned") had been masking this latent gap for months. Confirmed by 30-agent adversarial hunt. NO save data lost (eligibility bits are itemNo-keyed via `sALBWItemNos`, never kItems-index-keyed; wardrobe stash bits untouched) — page was unreachable, not deleted. **FIX (3 parts):** (A) added a CAT_ARMOR keep-alive clause to `categoryHasVisibleRows` mirroring the CAT_SWORDS precedent (sumo eligible / Deity-not-owned / any stored-or-owned outfit under Quick Swap); (B) — the real root — gave `itemRowVisible` the same clothes-wear ownership exemption the row builder already had, via a shared `isClothesWearItemNo()` helper used by BOTH sites so they can never drift again (clothes rows now always keep the page alive, as intended: "buying re-equips even while worn"). LESSON: page-existence was computed by a *second, stricter* reimplementation of row-visibility — two functions that must agree but didn't. Playtest: Armor tab present on late-game save (owned clothes), shows sumo/Deity/clothes; still present on fresh save (????? rows). |
| **MAGIC ARMOR UNRECOVERABLE — latent bug from the 500r retirement, FIXED 2026-07-15 (built, awaiting playtest)** | Found during the wardrobe-page hunt. The Magic 500r-row retirement (batch 5) claimed "buy-back = the existing storage Retrieve row," but that row's only build path was the `kItems` loop (`appendStorageRowsForOutfit` at the CAT_ARMOR entry) — deleting Magic's `kItems` entry killed it. Meanwhile d_meter2.cpp still confiscates Magic into storage (bit 711) on death ⇒ **die wearing Magic and it was gone permanently.** FIX: (1) inject `appendStorageRowsForItem((u8)dItemNo_ARMOR_e, nullptr)` directly in rebuildVisibleList's CAT_ARMOR section (no-ops unless Quick Swap on AND Magic owned/stored) so the Retrieve row builds; (2) the display name/desc resolver fell through to `swordNameForItemNo(ARMOR_e)`→"Sword"/"None" for a kItems-less row — added `outfitStorageNameForItemNo`/`outfitStorageDescForItemNo` ("Magic Armor"/"Deity Armor" + themed desc) chained before the sword fallback. Purchase path already generic (VISIBLE_ITEM storageRetrieve → `dAlbwWardrobe_tryRetrieveItemNo(ARMOR_e)`). Retrieve price = `kAlbwWardrobeStorageRetrievePrice` (100r). Playtest: die wearing Magic → Postman Armor page shows "Magic Armor" Retrieve row at 100r → buying restores it to the D-pad cycle. STILL: Quick-Swap-OFF players can't retrieve (pre-existing policy caveat). |
| **D-pad move damage tweaking** | **AUDITED + FIXED 2026-07-15 (built, awaiting playtest).** User hypothesis (wolf damage multiplier) REFUTED — no multiplier applies to art hits (master sword ×2 human-only; sumo ×3 disabled in wolf; the ×0.25/×0.70 wolf split gates on `AT_TYPE_MIDNA_LOCK` which neither art uses; `mWolfCombatHowlActive` carries zero damage effect). REAL CAUSE: vanilla `at_power_get` table cliff — powerType-1 enemies (~40 of ~55) map atp≥4 → flat **200** (ending-blow tier) vs atp×10 below it. Howl AOE atp 8 and Midna arm atp 4 (copied from Hurricane, whose 4 never reaches the table in human form thanks to the FA override) both landed on the cliff → 200/hit vs bite's 20-30. Side effect: both arts crossed the ≥60 wolf-hit Shadow Beast instant-kill threshold (e_s1) every hit. FIX (option 2, Hurricane-override pattern): fixed-power overrides in `cc_at_check` (d_cc_uty.cpp) — `kAlbwHowlAoeHitPower = 100`, `kAlbwMidnaArmHitPower = 100` (user call) — pinned after vanilla modifiers, before global sliders (invincibleEnemies / dAlbwHP_applyMult / bash +5% still stack); howl keyed on ALINK WOLF_CUT_TURN + `mWolfCombatHowlActive` (guard-opener disambiguation), arm keyed on actor name; covers frozen-bridge hits (same function). NOTE: 100 still > 60, so arts still one-shot Shadow Beasts (as does the Midna field attack) — flag if unwanted. Playtest: howl ticks and arm punches should feel ~half prior power; bites unchanged; sword-at-up buff no longer doubles howl ticks (override runs after it). |
| **FA shield-gate revert decision** | §1 — user testing enemy-type feel. |

### Playtest checklist for the 2026-07-14 batches
Batch 1 (verified in play: "fixes seem great"):
1. **Oocoo warp:** Goron Mines death → 15r service → lands room 1, no crash. ✔ (mid-boss
   deaths land in the arena sub-stage — expected, see §2 note)
2. **King Bulblin axe parry** ✔ · 3. **Dangoro bash→knockdown** ✔ · 4. **FA no-fill on clank** ✔

Batch 2 (built 2026-07-14, partial playtest):
1. **Bash stamina:** every connecting bash should now visibly refill ~5% meter — NOT YET
   TESTED.
2. **Guard openers:** Combat Howl + Midna arm CONFIRMED working better (user, 2026-07-14);
   **Hurricane NOT YET TESTED**; Great Spin / normal spin should still clank.
3. **Death toast "Continue Here":** NOT YET TESTED.
(Deferred confirmations — fold into the next playtest session.)

### Batch 3 — IMPLEMENTED 2026-07-14 (built, uncommitted, awaiting playtest)
1. **Freeze classifier fixed — CORRECTED SAME DAY:** the true shadow beast is **E_S1**
   ([d_a_e_s1.h](../include/d/actor/d_a_e_s1.h) "Shadow Beast" — resurrection howl,
   pack-finish, warp-appear, chest-mash), NOT E_SH (**Stalhound**, must stay freezable —
   its stun-collider bridge exists for exactly that). Final classifier entry:
   `fpcNm_E_S1_e` in `dAlbwWolfStun_isTwilightEnemy` (E_MD/Suit-of-Armor removed,
   per-case comments restored). An intermediate build briefly listed E_SH — superseded,
   never playtested. TWO actor-name mislabels are now documented here: E_MD≠Shadow
   Beast (it's Suit of Armor) and E_SH≠Shadow Beast (it's Stalhound).
   Playtest: shadow beasts (twilight arenas) take boosted Midna-charge damage with NO
   freeze; Stalhounds and Suit of Armor freeze normally.

1b. **Wolf charge on Shadow Beasts + chest-mash counter — IMPLEMENTED (fifteenths
   economy, user-tuned).** Root cause: a normal wolf bite on E_S1 triggers the hang-bite
   grab and returns before cc_at_check (the ONLY charge-accrual site), and the
   chest-mash applies `health -= 5` internally — zero charge for the whole sequence
   (only jump attacks counted).
   **Economy (user spec):** shared fractional accumulator (`mWolfBiteCount`) in
   FIFTEENTHS of a charge — normal bite = 3/15 (5 bites = 1 charge, unchanged feel),
   grab/chest-mash hit = 1/15 (15 mashes = 1 charge); fractions from both sources
   combine and carry across a completed charge, EXCEPT when the completion lands on the
   2-charge cap: leftover fraction is dropped. Helpers
   `dAlbwWolfCombat_onBiteConnect()` / `onChestMashHit()`
   ([d_albw_wolf_stun.cpp](../src/d/d_albw_wolf_stun.cpp), single source of truth —
   cc_at_check calls onBiteConnect too).
   **Wired enemies:** E_S1 + E_YC credit the initial grab chomp as a bite (3/15) and
   each A-mash at 1/15; Dynalfos (E_MF), Poe (E_PO), Lizalfos (E_DN), Death Sword
   (E_VT), Gibdo (E_GI), Twilight Insect Boss (E_YMB) credit each mash at 1/15 at their
   `checkWolfBiteDamage` sites. **Skipped:** B_MGN (boss, deliberate). **Deferred:**
   Stalhound (E_SH) — its bite whiffs come from i-frame hurt-sphere displacement
   (10-20f off-body), needs focused work per user.

1c. **MQ shop repricing — IMPLEMENTED (user curves,
   [d_albw_master_quest.cpp](../src/d/d_albw_master_quest.cpp)).** Hearts T1-10 →
   225/250/275/325/375/425/500/575/675/800 (T11-17 kept: 1000...9999). Stamina T1-20 →
   100..1000 (linear), 1200..2000 (+200), 2250..3000 (+250), then **3033** (T21-23 kept
   at 3333). NOTE: T20=3033 implemented as literally specified — flagged as a possible
   typo for 3333 (would make the tail 3333×4). Future idea on record: uncap both shops
   for continuous purchases.
2. **Twilight Kargarok rupees:** `E_YR` added to `lookupKillRupees` at 15 (identity
   verified by behavior: E_YR = STANDARD solo dive-attacking twilight kargarok; E_YC =
   the Lake Hylia one carrying the Shadow Bulblin rider — decomp briefs are crossed) +
   manual `onEnemyKill` grant in E_YR's cut-down finisher (health zeroed outside
   cc_at_check; grant idempotent).
3. **Map button (FIXED TWICE — final form 2026-07-14):** first attempt wrote TOUCHPAD to
   `openMapScreen[1]` — WRONG: bind slots are PER-PORT (player), not multiple bindings,
   so that was player 2's slot; worse, the stored int is interpreted per the port's
   active device (keyboard scancode vs SDL gamepad button), so the old keyboard-M preset
   read as gamepad button 16 (paddle) on controllers — map dead on pad regardless.
   **Final fix:** device-aware preset in `applyDpadQuickSwapPresetBinds` — port 0 gets
   TOUCHPAD when a controller drives it (with M→TOUCHPAD migration for existing
   configs), M when keyboard does; plus a one-shot re-apply in `updateActionBindings`
   on first controller connect (the boot-time preset call in m_Do_main runs before pads
   init). No-touchpad pads: rebind in Controller Config, L3 recommended. Also: D-pad
   quick-swap reservations now suppress menu-window triggers only while UNPAUSED —
   paused UIs (item/map screens) get full D-pad navigation back. Known quirk: a config
   migrated to TOUCHPAD (20) reads as scancode 20 (Q) if the port later switches to
   keyboard — acceptable, rebindable.
4. **Shade Watcher return re-priced 50 → 15** (placeholder TODO resolved; shares the
   Oocoo death-convenience tier).

**In-flight research:** shadow-beast wolf-charge registration + chest-attack (A-mash)
charge counter — agent running; implementation follows its report.

### New investigation round (2026-07-14) — ALL FOUR RESEARCHED, fixes designed

1. **Wolf freeze "twilight regression" — actually a LATENT DATA BUG, root-caused.**
   `dAlbwWolfStun_isTwilightEnemy` ([d_albw_wolf_stun.cpp:604-618](../src/d/d_albw_wolf_stun.cpp))
   lists `fpcNm_E_MD_e` commented "Shadow Beast" — but **E_MD is the Suit of Armor**;
   the real shadow beast **E_SH is missing**, so shadow beasts take the non-twilight
   branch (×0.25 + 300f freeze) instead of ×0.70-no-freeze. Git forensics CLEARED the
   suspected commits (fb0a4c0dcf/1ba40cfc5d — twilight branch byte-identical since
   a4e91484c7); 7ee0c23aa8's E_SH stun-bridge merely surfaced the mislabel. Authoritative
   spec confirmed ([d_albw_wolf_stun.h:29-32](../include/d/d_albw_wolf_stun.h)): twilight
   ×0.70 no stun / non-twilight ×0.25 + 300f stun. **Fix = swap E_MD→E_SH in the list +
   restore per-case comments** (one site fixes damage AND freeze; both consult sites call
   the same classifier). Side effects: E_MD (Suit of Armor) becomes freezable as intended
   (verify frozen pose); E_SH stun-bridge code becomes dead (harmless; prune deliberately
   or leave).
   **Freeze coverage matrix (non-twilight):** stun = `fpcM_PauseEnable`, works on any
   ENEMY-group actor that takes a DAMAGING Midna-lock hit and survives; only B_ZANT is
   name-excluded; E_RDB DOES respond (earlier report was the arts-vs-guard issue).
   Gaps ranked: (1) blocked hits never freeze (`mAttackPower>0` gate — by-design
   tradeoff); (2) only E_OC/E_SH have per-actor frozen hurt-sphere refresh — other
   enemies may whiff follow-up bites while frozen (add per-actor refresh if playtests
   show it); (3) non-Zant bosses technically freezable — low-priority audit.

2. **Twilight Kargarok rupees — root-caused, two-line fix.** The twilight kargarok is
   actor **E_YR** ("Twilight Kargorok Rider") and is **missing from `lookupKillRupees`**
   ([d_albw_enemy_rupee.cpp:171-292](../src/d/d_albw_enemy_rupee.cpp); default → 0).
   Fix: `case fpcNm_E_YR_e: return 15;` (match E_KR/E_YC) + mirror E_YC's manual
   `dAlbwEnemyRupees_onEnemyKill()` call in E_YR's cut-down finisher
   ([d_a_e_yr.cpp:1762](../src/d/actor/d_a_e_yr.cpp) — sets health=0 outside cc_at_check;
   grant is idempotent). All other twilight variants verified wired.

3. **Map button vs Extra+Quick Swap — gap, not bug.** Vanilla map = D-pad Left OR Right;
   Extra reserves Left (Call Midna), Quick Swap reserves Right (shield cycle) → both dead
   on controller; replacement binds `OPEN_MAP_SCREEN`/`TOGGLE_MINIMAP` are preset only to
   keyboard M/Tab ([dpad_quick_swap.cpp:51-52](../src/dusk/dpad_quick_swap.cpp)) — **no
   controller default was ever chosen** (docs/d-pad-reworking.md left it open). Fix plan:
   (1) preset a free controller button (R3 recommended; L3/touchpad free) for
   OPEN_MAP_SCREEN in `applyDpadQuickSwapPresetBinds` — action binds read SDL directly so
   this dodges the GC-profile analog-suppression trap; plus (3) narrow
   `dpadRightReservedForQuickSwap` to gameplay-only so D-pad Right works in pause/menus
   again ([action_bindings.cpp:236-239](../src/dusk/action_bindings.cpp)). Option 2
   (R-chord) available if preferred.

4. **Heart & stamina price table — researched.** Both are Master-Quest-only Postman shop
   rows ([d_albw_master_quest.cpp:23-30](../src/d/d_albw_master_quest.cpp)): hearts 17
   tiers 44→9999r (+½ heart each, cap 20 hearts, reg 100; MQ also halves found
   pieces/containers), stamina 23 tiers 100→3333r (+632 units, reg 101, replaces
   dungeon-clear expansion under MQ). The two +8480 tier bonuses are earned (10 hearts /
   Magic Armor), never sold. **Balance-pass flags:** Shade Watcher return 50r is a marked
   PLACEHOLDER (same service class as the 15r Oocoo); Wolf Howl/Midna's Grasp 100r each
   unreviewed post-rework; MQ curves unreviewed post-rework; FA docs stale
   (combat-refinements-handoff.md says 500/1500/3500, code is 100/300/500).
