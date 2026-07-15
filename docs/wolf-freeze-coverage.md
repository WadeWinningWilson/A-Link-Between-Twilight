# Wolf Freeze — Universal Enemy Coverage Campaign

**Goal:** the wolf Midna-charge freeze works on every eligible enemy actor.
**Rules (user):** ALL bosses + final bosses + TRAPS excluded; mid-bosses per-case.
Twilight enemies are excluded from *freezing* by design (they take the ×0.70
damage boost instead — `dAlbwWolfStun_isTwilightEnemy`), but belong to this
campaign as classifier entries.

**Mechanism recap:** freeze = `fpcM_PauseEnable` (works on any actor). Dispatch
gates ([d_cc_uty.cpp:699-711](../src/d/d_cc_uty.cpp)): wolf combat on ·
`fopAc_ENEMY_e` group · name ≠ B_ZANT · `AT_TYPE_MIDNA_LOCK` collider ·
damaging hit (`mAttackPower > 0`) · target survives · not twilight-classified.
Known systemic gaps: blocked/shielded hits never freeze; only E_OC/E_SH have
per-actor frozen hurt-sphere refresh (others may whiff follow-up bites while
frozen); source: albw-alpha-cleanup.md round-3 research.

Source mapping: TP actor spreadsheet (scratchpad tp_enemies.csv, 2026-07-14).

---

## A. REGULAR ENEMIES — target: freeze works (46)

| Actor | Enemy | Notes / expected wrinkles |
|---|---|---|
| e_oc | Bokoblin | ✅ already fully wired (reference actor, has hurt-sphere refresh) |
| e_sh | Stalhound | Has bridge support; i-frame hurt-sphere displacement whiffs bites (known, separate) |
| e_rd | Bulblin | Learn-guard AI; shielded variants = blocked-hit gap |
| e_ai | Armos | Statue wake states — freeze while dormant? |
| e_gs | Ghost Soldier | Sword+shield; blocked-hit gap |
| e_dd | Dodongo | — |
| e_dn | Lizalfos | Hang-bite grab (charge already wired); shield |
| e_mf | Dynalfos | Hang-bite grab (charge wired); AtSpl-10 breaker |
| e_sg | Skullfish | Aquatic — can wolf lock-attack underwater? verify |
| e_bs | Stalkin | Swarm, 1-hit kills — freeze may be moot but should not crash |
| e_sf | Stalfos | Learn-guard |
| e_df | Deku Like | Rooted; swallows — freeze mid-swallow state? |
| e_gm | Baby Gohma | Swarm |
| e_md | Suit of Armor | Just un-twilighted in batch 3 — verify freeze pose |
| e_sm | Chu Worm | Bubble shell — two-part collider |
| e_sm2 | Chu | Merging/splitting slimes — freeze mid-merge? |
| e_st | Skulltula | Ceiling drop states |
| e_sb | Shell Blade | Shell = TgShield-like; blocked-hit gap |
| e_cr | Bomskit | Flees; explodes |
| e_db | Baba Serpent | Rooted/severed states |
| e_gb | Big Baba | Two-part (head + Deku Like base) |
| e_hb | Deku Baba | Rooted |
| e_hm | Torch Slug | Fire contact — frozen fire? |
| e_tk | Water Toadpoli | Aquatic spitter |
| e_tk2 | Fire Toadpoli | Lava spitter |
| e_rb | Leever | Burrow cycle — freeze while submerged? |
| e_fs | Puppet | Skull Kid adds; infinite respawn — freeze OK, kill-grant moot |
| e_po | Poe | Hang-bite grab (charge wired); incorporeal phases + lantern state |
| e_mm | Helmasaur | Masked front — blocked-hit gap |
| e_mm2 | Helmasaurus | Larger variant, same actor file |
| e_fz | Mini Freezard | Ice enemy — freeze-on-ice redundancy, verify no weirdness |
| e_fb | Freezard | Breath turret — freeze mid-breath? |
| e_zs | Staltroop | Swarm |
| e_kk | Chilfos | Ice lance |
| e_hp | Imp Poe | Wolf-sense-only visibility — freeze while invisible? |
| e_fk | Phantom Jouster | Mounted — rider/mount split like E_RD? |
| e_ms | Rat | Trivial HP |
| e_nz | Ghoul Rat | Invisible outside senses; latches onto Link |
| e_ba | Keese | Flier — lock-attack reachable, dies in 1 hit usually |
| e_bu | Bubble | Flier |
| e_bug | Poison Mite | Swarm heap |
| e_kg | Young Gohma | Swarm |
| e_kr | Kargarok | Flier — dive windows only |
| e_sw | Moldorm | Burrower |
| e_ge | Guay | Flier |
| e_hz | Tile Worm | Mostly hidden — trap-adjacent; CONFIRM include? |
| e_ws | Skullwalltula | Wall-bound — lock reachable? |
| e_bg | Bomb Fish | Aquatic, explodes |
| e_tt | Tektite | — |
| e_dk | Bari | Electric — frozen electric aura? |
| e_ww | White Wolfos | — |
| e_gi | Redead Knight | Grab/scream paralysis; hang-bite charge wired |
| e_bi | Bombling | Explodes — freeze then boom? (has lockout perk interplay) |
| e_zh | Zant's Hand | Sol-carrier chase — freezing it is a GAMEPLAY question (it flees w/ sol); CONFIRM include? |

## B. TWILIGHT — classifier entries, ×0.70-no-freeze by design

In classifier today: e_s1 (Shadow Beast), e_yd, e_yh, e_yd_leaf, e_ymb, e_yk,
e_yr, e_yg. **GAPS FOUND vs the roster (currently these FREEZE — likely wrong):**

| Actor | Enemy | Verdict needed |
|---|---|---|
| **e_ym** | Shadow Insect | Twilight enemy — add to classifier? (bug-sense hunts; freezing may even break the Vessel flow) |
| **e_yc** | Twilit Carrier Kargarok | Twilight (Lake Hylia rider-carrier) — add? |
| **e_rdy** | Shadow Bulblin | Twilight-realm bulblin — add? |

## C. MID-BOSSES — per-case decisions (user)

| Actor | Enemy | Current freeze status / recommendation |
|---|---|---|
| e_gob | Dangoro | Has ALBW bash→knockdown; freeze during magnet-platform fight risky (falls?) — lean EXCLUDE or restrict |
| e_th | Darkhammer | Armored ball&chain — blocked-hit gap dominates; lean INCLUDE (reward flank) |
| e_dt | Deku Toad | Ceiling drop + spawn phases; lean INCLUDE |
| e_vt | Death Sword | Hang-bite wired; wolf-sense phases — freeze mid-invisible? per-case |
| e_mk | Ook | Boomerang fight, monkey — lean INCLUDE (comedy value) |
| e_pm | Skull Kid | Teleporting flute kid — freeze cheeses hide-and-seek? per-case |
| e_pz | Phantom Zant | Teleporter; PoT mid-boss — per-case |
| b_tn | Darknut | ALSO a repeatable late-game enemy (castle/CoO); has full ALBW layer — lean INCLUDE (guard rules apply) |
| b_gg | Aeralfos | Repeatable too; flier — lean INCLUDE |
| e_rdb | King Bulblin | Axe fights now parry/opener-wired; freeze responds already (round-3) — lean INCLUDE |
| e_ymb | Twilit Bloat | Twilight → no freeze by design (already classified) |

## D. EXCLUDED — bosses/final (11): e_fm (Fyrus), b_bq/b_bh (Diababa), b_ob
(Morpheel), b_ds (Stallord), b_yo (Blizzeta), b_gm (Armogohma), b_dr (Argorok),
b_zant/b_zant_mobile (Zant — already name-excluded in dispatch), b_gnd
(Ganondorf), b_mgn (Ganon), e_hzelda (Possessed Zelda).

## E. EXCLUDED — traps/objects/unused: E_bm6 (Beamos), E_Make (spawner),
e_warpappear, e_arrow (projectile), e_ga (decorative moths), e_nest (beehive),
e_bee / e_is (Armos Titan) / b_go (Goron Golem) / e_zm2 / e_oct_bg (unused),
b_hashi / b_ling / b_znts (objects), leaf sub-actors (e_db_leaf, e_hb_leaf,
e_yd_leaf handled via classifier), e_ym_tag (tag).

---

---

# PHASE 2 RESULTS — full per-actor sweep (2026-07-14, 6 agents, 51 actors)

**The headline:** the generic snapshot bridge covers far more than expected.
**35 actors freeze correctly with ZERO new code.** Only 3 need the E_OC-style
hurt-sphere refresh (all for the same reason: same-frame i-frame position-hides),
~6 need small state guards, 2 need custom hooks, and the rest are structurally
excluded by the engine itself (wrong group, no common damage path, or
special-weapon-only design). Full agent transcripts hold per-line evidence.

## Consolidated verdict table

**WORKS-AS-IS (35):** e_sf, e_dn, e_mf, e_bs, e_rd (on foot) · e_ms, e_kg, e_gm,
e_cr, e_bi (fuse suspends!), e_fs, e_hz (attack pose self-power-zeroes), e_ww,
e_sw · e_ba, e_bu, e_ge, e_kr (fliers hang frozen mid-air — acceptable), e_sg,
e_tk, e_tk2 (aquatic: reach-limited to surface windows, land-initiated pounce +
height filter), e_ws, e_st · e_po (incorporeal = no collider = no bad state),
e_hp, e_fz, e_mm (front mask = known blocked-hit gap; flanks freeze), e_wb
(cavalry/Epona set-pieces skip the damage branch = self-excluded) · e_sm2 (its
OffTgSetBit i-frames are self-healed by the bridge's TgSet re-enable), e_dk
(electric aura is AT-only → auto-disarms while frozen), e_tt · mid-bosses:
e_th (spin invulnerable; freeze only in punish window), e_dt (tongue-only
damage), e_mk (grounded window only), b_tn (cleanest; repeatable enemy OK).

**NEEDS-REFRESH (3) — same root cause:** e_db (Baba Serpent), e_gb (Big Baba),
e_rb (Leever). Their freeze-frame sets an invuln timer and the SAME execute pass
then position-hides the hurt sphere (+10000/+30000 offsets); the paused timer
never ticks so the frozen sphere stays parked → follow-up melee whiffs. Fix =
add per-actor cases to `prepareStunnedEnemyForBridge` re-centering the sphere
(the exact E_OC pattern — which, it turns out, exists precisely for this actor
class). Refs: d_a_e_db.cpp:2027, d_a_e_gb.cpp:1408, d_a_e_rb.cpp:721.

**NEEDS-STATE-GUARD (6):** e_rd (skip while RIDE_MODE_RIDE/joust), e_gi (skip or
force-release while `m_cry_gi == this && mCryTimer != 0` — else camera stays
force-locked + all screams blocked 300f), e_nz (skip while latched — strands a
stick-slot + floating collider), e_kk (freeze only reachable post-armor-break by
design; guard shatter/thrown/ironball states), e_gob (skip ACTION_BALL/GLAB/JUMP
+ mDemoCamMode≠0 — mirrors the bash-knockdown state list), e_pz (skip warp mode
11 + demo modes; note: freezing the real Zant partially trivializes the
illusion puzzle — user call).

**NEEDS-CUSTOM (2):** e_fk Phantom Jouster — takes wolf damage via bespoke
At_Check (even reads MIDNA_LOCK) but never routes through cc_at_check → hurt
but never frozen; fix = direct `dAlbwWolfStun_apply` + charge credit in its
At_Check (d_a_e_fk.cpp:608-628). e_kk Chilfos — armored-phase MIDNA_LOCK
deliberately guard-breaks with no damage path (correct as-is; custom only if
armored freeze is ever wanted).

**TWILIGHT ADDS (3, verified by dark-render + dark-vanish marker sweep):**
e_ym (Shadow Insect), e_yc (Carrier Kargarok — STRONG: shared rider demo
desyncs + fall risk if frozen), e_rdy (Shadow Bulblin). Marker sweep confirms NO
other classifier stragglers (e_ms dark-render is a cosmetic quirk — leave
freezable).

**STRUCTURALLY EXCLUDED / FREEZE-IMMUNE (13):** e_gs (no hurt collider at all),
e_zs (MIDNA_LOCK inline-instant-kills — no freeze possible, acceptable), e_md
(ENV group — retro-corrects the batch-3 "verify freezable" note: it can never
freeze), e_bug (ACTOR group), e_zh (ENV group — the Sol sequence-break is
structurally impossible, no decision needed), e_ph (no cc_at_check + permanent
shield), e_bg (onWolfNoLock at create), e_fb + e_ai (iron-ball/bomb-only vanilla
design), e_hb (one-hit weak point, no cc_at_check), e_hm (no cc_at_check), e_df
(ACTOR group + bomb-kill only), e_sm (bubble shell rebounds all wolf melee;
core needs bomb-pop first — CUSTOM if ever wanted).

**DESIGN EXCLUDES (recommend, user confirms):** e_vt Death Sword (Zant-class
invisibility/phase scripting), e_rdb King Bulblin (**SOFTLOCK: defeat fires on
knockdown COUNT at health>0 — the exact freeze condition — freezing the final
knockdown stalls the defeat demo**; exclude, or guard mAction 6/7 everywhere).
Freeze-immune mid-bosses needing no decision: e_pm Skull Kid, b_gg Aeralfos
(both bespoke damage paths — freeze can't reach them).

## Phase 3 — implementation batches (proposed)
- **F1 (no design questions, high value):** 3 twilight classifier adds ·
  e_fk custom stun hook · 3 hurt-sphere refresh cases (db/gb/rb).
- **F2 (state guards):** rd-mount, gi-scream, nz-latch, kk-states, gob-states,
  pz-states (+ rdb-guard IF user includes him).
- **F3 (optional customs, user call):** armored-Chilfos freeze, Chu Worm core,
  Skull Kid / Aeralfos freeze-wiring, Staltroop freeze-instead-of-instakill.

## Decisions (user, 2026-07-14)
1. Death Sword: **EXCLUDED**. 2. King Bulblin: **INCLUDED with deferred-defeat
mechanic** (freeze skipped on knockdown/defeat states + final pre-defeat count;
frozen hits health-clamped at 1 with full hit feedback — defeat only registers
after thaw). 3. Phantom Zant: **EXCLUDED**. 4. F3 customs: deferred, contingent
on how the King Bulblin mechanic plays.

# PHASE 3 — F1+F2 IMPLEMENTED 2026-07-14 (built, uncommitted, awaiting playtest)

All eligibility now flows through ONE central gate inside `dAlbwWolfStun_apply`
([d_albw_wolf_stun.cpp](../src/d/d_albw_wolf_stun.cpp)) — `isFreezeExcludedName`
(E_VT, E_PZ) + `isFreezeUnsafeState` (per-actor case table) — so every caller
(shared cc_at_check path AND bespoke hooks) inherits the rules, and sweeping
changes stay one-place edits per the root-formula requirement.

Shipped:
- **Twilight adds:** E_YM, E_YC, E_RDY in `isTwilightEnemy` — all twilight
  actors now damage-variation-only, no freeze, game-wide.
- **Exclusions:** E_VT Death Sword, E_PZ Phantom Zant (central gate).
- **State guards (central case table):** E_RD ride_mode != 0 (mount/joust);
  E_GI scream owner (new `daE_GI_isScreamOwner` accessor over the file-static
  m_cry_gi + public `albwCryTimerActive()`); E_NZ latched (ACTION_STICK);
  E_KK via new public `albwIsFreezeUnsafeState()` (shatter/thrown/ironball);
  E_GOB ball/grab/lava-jump/demo-cam; E_RDB knockdown/defeat states + final
  pre-defeat knockdown count (D_MN09-aware threshold).
- **King Bulblin deferred defeat:** bridge hits on a frozen E_RDB clamp health
  at 1 (processBridgeHit) — hit sparks/feedback still play; his count-based
  defeat logic is paused anyway, so defeat resumes only after thaw.
- **Hurt-sphere refresh cases:** E_DB / E_GB / E_RB re-centered to eyePos each
  bridge frame in `prepareStunnedEnemyForBridge` (their freeze-frame i-frames
  park the sphere at +10000/+30000 offsets otherwise).
- **Phantom Jouster (E_FK):** direct `dAlbwWolfStun_apply` hook in its bespoke
  At_Check (it never routes through cc_at_check) — the one "hurt but never
  frozen" enemy now freezes; central gate applies to it automatically.

## HOTFIX (2026-07-14, post-playtest): generic collider un-park

User playtest (Bulblins/Puppets/Tektites): freezes worked but sword follow-ups
whiffed on Bulblins + Puppets (Tektite actually took all 4 hits — frozen
enemies just play no hit reaction, so it read as "no damage"). Diagnosis from
the always-on `albw_wolf_stun_debug.txt` + engine pair-loop analysis: the
STANDARD TP I-FRAME IDIOM parks hurt colliders 10k-200k units off-body while a
damage timer runs, the freeze always fires inside the damage frame that starts
that timer, and the pause keeps the timer alive → snapshot permanently parked
→ broadphase never pairs the sword with it. E_RD/E_FS/E_BA/E_BU/E_BS/E_BI all
use the idiom (phase-2 sweeps under-detected it; db/gb/rb were the ones caught).

**Root fix (generic, in `prepareStunnedEnemyForBridge`):** every bridge frame,
any captured collider farther than 2000 units from its frozen owner snaps back
to `eyePos` via the shape's `GetCoCP()` (real center for Sph/Cyl — the vast
majority of enemy TGs; harmless no-op for others). Idempotent, frozen-enemies-
only cost, covers every current and FUTURE actor; the per-actor refresh cases
remain as exact-position overrides. Logs `evt=unpark` for verification.

**Known follow-up (root-level reaction work):** frozen enemies play no hit
reaction — damage lands silently except hitmarks. Candidate for the central
while-frozen feedback hook (flash/shake/SFX in processBridgeHit).

## HOTFIX 2 (post-CoO stress test): double-death edge case — FIXED

User: Cave of Ordeals stress test = "really reliable" overall; edge case =
enemies killed while frozen granted rupees, then thawed and ran around at
0 HP, killable AGAIN for the same rupees. Two root causes, both fixed:
1. **Rupee guard table wipe** ([d_albw_enemy_rupee.cpp](../src/d/d_albw_enemy_rupee.cpp)
   `markKillGranted`): at 64 entries the guard RESET THE WHOLE TABLE,
   forgetting every granted kill — trivially reached in CoO sessions. Now a
   ring-evict (only the single oldest entry is forgotten per insert).
2. **Alive-at-0HP zombies**: most actors only process death in their own
   damage flow, which their stale (paused) i-frame timer gates shut on thaw.
   New post-thaw watchdog in d_albw_wolf_stun: a lethal bridge kill arms a
   30-frame grace entry; if the actor still exists at <=0 HP when it expires,
   it is despawned (`evt=zombie-despawn` logged). Actors with real 0-HP
   handling (e.g. E_OC's death dispatch) die naturally within the grace.

Playtest checklist:
1. Twilight areas: Shadow Insects / Carrier Kargarok / Shadow Bulblins take
   boosted Midna-charge damage, never freeze.
2. Death Sword + Phantom Zant: never freeze.
3. King Bulblin (camp + castle): freezable mid-fight; final knockdown never
   happens frozen; wailing on him frozen shows hits but can't finish him.
4. Redead mid-scream, mounted Bulblins, latched Ghoul Rat, Chilfos shards,
   Dangoro ball/grab: no freeze in those states; freeze works otherwise.
5. Baba Serpent / Big Baba / Leever: frozen heads still take follow-up bites.
6. Phantom Jouster: freezes on Midna-charge hits.
7. Regression: Bokoblins/Stalhounds freeze + follow-up + death behave as
   before (user note: Stalhound death behaviors were historically wonky —
   watch its bridge-kill reaction).
