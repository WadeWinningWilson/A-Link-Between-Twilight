# Boss fights — design handoff & session archive

**Purpose:** Preserve critical boss-combat work from the long Cursor chat so you **do not need to reopen that session** (it will OOM).

**Live state (read first):** [state/boss-fights.md](state/boss-fights.md) — status / next / playtest combo. **This file** remains the full design archive for revisions.

> Continue boss fight work — read `docs/AGENT_INDEX.md` + `docs/state/boss-fights.md` first; use this handoff for detail.

**Related docs:** [albw-port.md](albw-port.md) (docket), [combat-refinements-handoff.md](combat-refinements-handoff.md) (field combat: enemy targeting, hidden skills × meter), [albw-boss-hp-hud-tuning-brief.md](albw-boss-hp-hud-tuning-brief.md) (HUD art), [shield-combat.md](shield-combat.md) (parry/bash/durability).

**Per-boss canonical docs:** [Boss-Fights-RefinedDiababa.md](Boss-Fights-RefinedDiababa.md), [Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md), [Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md).

**FPS / golden build (separate doc):** [state/drive-fps.md](state/drive-fps.md) + [future-performance-leaning.md](future-performance-leaning.md) — not covered here.

---

## Settings (in-game)

| Setting | Config key | Default | What it gates |
|---------|------------|---------|---------------|
| **Boss Refinement** | `game.bossRefinement` | **Off** | Armogohma %-HP pacing, warp bootstrap, any-sword gates (Zant/Ganon/Argorok) |
| **Boss Health Bars** | `game.bossHealthBars` | **Off** | LoP-style reactive bar in `d_albw_boss_hp_hud.cpp` |
| **Boss HP ×** | `game.hpMultBoss` | **1×** | True max HP for boss-tier actors (`fpcNm_B_*`, etc.) |
| **Mid-Boss HP ×** | `game.hpMultMidBoss` | **1×** | Darknut (`B_TN`), Aeralfos (`B_GG`), etc. |
| **Shield Parry & Bash** | `game.shieldParryCombat` | On | Bash charges, perfect parry — used in Darknut fight |
| **Manual Shielding** | `game.manualShielding` | Off | ZR guard required for parry/bash |
| **Darknut Bash Debug Log** | `game.showDarknutBashDebug` | Off | Writes `albw_darknut_debug.txt` |

**Playtest combo for boss work:** Boss Refinement **On**, Boss Health Bars **On**, Boss HP × as desired (4× was used during Armogohma tuning).

---

## Architecture — three layers

```text
┌─────────────────────────────────────────────────────────────┐
│  Boss Refinement (game.bossRefinement)                      │
│  • Layer A: any-sword gates (Zant, GND, MGN, Argorok)       │
│  • Armogohma: %-HP bar, egg gates, warp bootstrap           │
│  • Diababa (live): see Boss-Fights-RefinedDiababa.md        │
│  • Fyrus: orphan BCK research — Boss-Fights-Fyrus-research  │
│  • Future Layer B: Zant 6-phase tools, Ganon CHANCE duel    │
├─────────────────────────────────────────────────────────────┤
│  Boss HP HUD (game.bossHealthBars)                          │
│  • Draws bar from dAlbwHP_getLockonDisplayHp()              │
│  • Armogohma phase 1/2 routing via dAlbwBoss_armogohma*   │
├─────────────────────────────────────────────────────────────┤
│  HP multiplier (game.hpMultBoss / hpMultMidBoss)            │
│  • dAlbwHP_tryApplyTrueMaxHp() on actor init                │
│  • Categories keyed on fpcNm_* process name, not filenames  │
└─────────────────────────────────────────────────────────────┘
```

**Important:** Runtime lookups use **`fopAcM_GetName()` → `fpcNm_*`**, not C++ class names or `d_a_*.cpp` filenames.

---

## Building Boss Refinement features (new chats)

**Always read first:**

| Doc | Why |
|-----|-----|
| [build-fps-guidelines.md](build-fps-guidelines.md) | Canonical build (`build_run.bat` / RelWithDebInfo), launch hygiene (no `DUSK_DRIVE*`), what never goes in the repo |
| [commit-and-push.md](commit-and-push.md) | Push to **upstream** (`WadeWinningWilson/A-Link-Between-Dusklight`), not `origin`; stage/don't-stage rules |

**If the work touches…**

| Area | Also read |
|------|-----------|
| HUD / meter / shield / `game.lopHud` | [hud-performance-handoff.md](hud-performance-handoff.md) + [albw-hud-lop-layout-brief.md](albw-hud-lop-layout-brief.md) |
| LoP layout details | [albw-hud-lop-layout-v3-instructions.md](albw-hud-lop-layout-v3-instructions.md) |
| Boss fights / refinement | This file + per-boss docs above |
| Boss HP bar tuning | [albw-boss-hp-hud-tuning-brief.md](albw-boss-hp-hud-tuning-brief.md) |

**After they build — hand off for FPS review:**

| Chat | Doc entry |
|------|-----------|
| Build-analysis / FPS review | [build-fps-guidelines.md](build-fps-guidelines.md) → Doc map section, then [hud-performance-handoff.md](hud-performance-handoff.md) if HUD changed |

**Short prompt (paste into any new feature chat):**

> Before coding: read `docs/build-fps-guidelines.md` and `docs/commit-and-push.md`.  
> Build with `build_run.bat` only (RelWithDebInfo). No drive/conavigate code in `src/`.  
> If HUD/meter/shield: also read `docs/hud-performance-handoff.md`.  
> When done: git diff summary + build result → build-analysis chat for FPS check before commit.  
> Push to **upstream** (ALBW-Dusklight), not origin.

**Hard rules (one-liners):**

- **Build:** `build/windows-msvc-relwithdebinfo/dusklight.exe` via `build_run.bat`
- **Never commit:** `local_dev_backup/`, drive/conavigate sources, `albw_*_debug.txt`
- **Don't revert features to fix FPS** — optimize in place; use the build-analysis chat
- **Don't commit/push** unless explicitly asked

---

## Boss Refinement — implemented

### Module

- `include/d/d_albw_boss.h`
- `src/d/d_albw_boss.cpp`

### Layer A — any sword (when setting on)

Master Sword checks become “any sword obtained” for:

- **Zant** (`d_a_b_zant.cpp`) — damage type, big damage, pause timer
- **Ganondorf horseback** (`d_a_b_gnd.cpp`) — five MS gates
- **Beast Ganon** (`d_a_b_mgn.cpp`) — MS damage tier
- **Argorok** — MS collider checks

Helpers: `dAlbwBossRefinement_colliderCountsAsMasterSword()`, `dAlbwBossRefinement_playerHasBossSword()`.

### Boss warp bootstrap (Boss Refinement on only)

Hooks:

- `d_a_obj_bosswarp.cpp` — `dAlbwBoss_requestWarpBootstrap()` on warp travel
- `d_s_play.cpp` — `dAlbwBoss_onStageLoad()` on stage load
- `d_a_cstatue.cpp` — skip stale switch gates during Armogohma warp revisit
- `d_a_b_gm.cpp` — actor bootstrap, victory cleanup

**Design rule:** Warp bootstrap fixes **boss warp revisit only** — normal dungeon door entry unchanged.

**Armogohma warp fixes:**

- No egg-loop softlock when quiver empty (ALBW bow uses meter, not arrows)
- Zone/room switches reset so Dominion Rod statues work mid-fight
- Player can add/equip rod during fight — no hard inventory gate at warp-in

---

## Armogohma — refined fight (Boss Refinement ON)

> **📖 Canonical Armogohma doc:** the full, up-to-date record — HP bar, egg fix, bomb-arrow crash, cut moves, giant floor-pursuit test, the eye rig + guard mechanic, and the **beta single-eye reveal** research (model inspection + `eye_test.bck` + the phase-3 reveal plan) — now lives in **[Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md)**. The sections below are the earlier/original notes kept for history; prefer the RefinedGohma doc for anything current.

_Everything below the banner — refined-fight design contract, HP bar, egg fix, bomb-arrow crash, cut moves, giant floor-pursuit test, the eye rig + guard mechanic, and the **beta single-eye reveal** research + phase-3 plan — has moved to the canonical [Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md)._

---

## Diababa — refined fight (Boss Refinement ON)

> **📖 Canonical Diababa doc:** research archive + live contract (orphan BCKs, siphon, 70% phase, lunge, side-head pacing) — **[Boss-Fights-RefinedDiababa.md](Boss-Fights-RefinedDiababa.md)**. Prefer that doc for Diababa work.

**Status:** LIVE under `game.bossRefinement`. Vanilla when off. Actors `B_BQ` / `B_BH` / `E_MB`. Forest Temple crash on enter was fixed separately (shield attention lock).

---

## Fyrus — research only (not refined yet)

> **📖 Preliminary orphan-BCK + Goron Golem cameo research:** **[Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md)** (§6 = `B_GO`/`B_GOS`). Actor `E_FM` / arc `E_fm`. No Refinement fight hooks yet.

---

## Darknut (`B_TN`) — bash / parry integration

Darknut is **mid-boss tier** (`sMidBoss` in `d_albw_hp_mult.cpp`) for HP mult **and** shield durability mult (1.5×).

### Special HP display

Darknut does **not** use plain `health` in phase 2:

| Phase | Meter | Fields |
|-------|-------|--------|
| Armored | Armor pieces remaining | `mNextBreakPart`, `ALBW_ARMOR_PIECE_COUNT` |
| Transition | `ACT_CHANGEDEMO` | Shows 0 / armor total |
| Unarmored | Internal damage meter | `field_0x6fc` taken vs `field_0x700` max |

`dAlbwHP_getLockonDisplayHp()` exposes this via `customMeter` + `darknutPhase`.

### Bash guard break (ALBW)

Methods on `daB_TN_c` (see `d_a_b_tn.h`):

- `albwTryApplyBashGuardBreakFromHit()` — shield bash opens guard window
- Phase 1: `albwApplyPhase1BashGuardBreak()` → 90-frame open window
- Phase 2 unarmored: `albwApplyPhase2BashGuardBreak()` → 75-frame window, `ACT_YOROKE`
- Debug: `showDarknutBashDebug` → `albw_darknut_debug.txt`

Integrates with `d_albw_shield.cpp` bash charge economy and `d_albw_lockout.cpp` slingshot stun (`onSlingshotHitNative` for Darknut).

---

## Boss Health Bars HUD

- **File:** `src/d/d_albw_boss_hp_hud.cpp`
- **HP source:** `dAlbwHP_getLockonDisplayHp(boss)` — do not duplicate HP math in HUD
- **Art brief:** [albw-boss-hp-hud-tuning-brief.md](albw-boss-hp-hud-tuning-brief.md)
- **Armogohma composite meter:** [albw-armogohma-boss-bar-spec.md](albw-armogohma-boss-bar-spec.md)

Target look: Lies of P proportions — thin crimson fill `(176,8,8)`, translucent charcoal track `(18,18,22)` @ ~66% alpha, small white name with thin outline.

---

## HP multiplier tables

**File:** `src/d/d_albw_hp_mult.cpp`

| Tier | Examples |
|------|----------|
| **Mid-boss** | `B_TN` Darknut, `B_GG` Aeralfos, Death Sword, Phantom Zant, Ook, Bulblin, etc. |
| **Boss** | Diababa, Morpheel parts, Stallord, Blizzeta, **Armogohma**, Argorok, Zant phases, Fyrus, Possessed Zelda |
| **Final** | `B_GND` Ganondorf, `B_MGN` Beast Ganon |

**Not arena-specific** — same category everywhere the actor appears.

Shield durability scaling: `dAlbwHP_applyDurabilityMult()` — mid-boss 1.5×, boss/final 2× on non-parry hits.

---

## ALBW meter lockout (boss relevance)

**File:** `src/d/d_albw_lockout.cpp`

When ALBW meter hits 0:

- Ranged/magic/tool perks gated until meter recovers
- Slingshot hit on enemy → 4s “ranged opened” window + stun
- Darknut uses `dAlbwLockout_onSlingshotHitNative()` (native wobble, not fpcM_Pause)

Future: visual feedback when `dAlbwLockout_isRangedOpened()` (shimmer/HUD).

---

## Enemy Death Rupees (boss payouts)

Fight victories grant wallet credit once per profile per session. Hooked in boss actor files (`d_a_b_tn.cpp`, `d_a_b_gm.cpp`, etc.). Additive only — never replaces vanilla drops.

---

## Pending — Layer B (designed, not implemented)

Full briefings preserved below from original chat **`fbc16b50-bd3e-4cad-968e-e1c0bd114747`** (session deleted 2026-06-15 after archival).

### Boss Rush (future setting)

Sequential warp through all 8 dungeon bosses + finale. Shares warp bootstrap infra with Boss Refinement but **separate toggle**. Must not change vanilla dungeon door entry — bootstrap applies **only** on boss-warp revisit.

### Zant multi-phase redesign

**Status:** PENDING — confirm against current `d_a_b_zant.cpp` before coding.

Six phases, each in a different dungeon arena, **tool-locked damage** (wrong tool = no damage + SE). Layer 1 feedback: wrong-item hit → `Z2SE_EN_ZAN_V_NO_DMG` (preferred); backups `Z2SE_D33V_ZANT_BOUZEN`, `Z2SE_D33V_ZANT_IKIGIRE`. Layer 2 (later): segmented boss HP bar with phase icons — coexists with SFX layer.

| Phase | Arena | Damage constraint |
|-------|-------|-------------------|
| 1 | Forest Temple | Slingshot only during vulnerability window (damage scales to sword tier) |
| 2 | Goron Mines | Bomb arrows only during vulnerability window |
| 3 | Lakebed Temple | Water bombs only — Iron Boots to place on floor, Clawshot pull Zant to bomb before fuse; 1–3 bombs/cycle |
| 4 | Ook pillars | Pillar bash unchanged; wolf damage only after knockdown |
| 5 | Snowpeak | Spinner jump only during shrunken state (vanilla window) |
| 6 | Throne Room | Spur charges required; cap = shield tier (Ordon 2 / Wooden 4 / Hylian 6); charges persist across phases |

**Open:** Phase 3 bomb damage scaling; Phase 1 slingshot scaling persistence post-Zant fight.

### Ganon / CHANCE duel

**Status:** PENDING — numeric tuning needs source data + playtest.

Cumulative ALBW mastery test; vanilla one-on-one sword duel preserved.

**Phase 1 — negation active:** Heavy damage negation until **2 successful CHANCE wins** (no time limit).

**Phase 2 — negation removed:** Full sword damage; pressure systems remain.

**CHANCE triggers:**

| Path | Rule |
|------|------|
| Shield bash (preferred) | Full spur charges → CHANCE attempt; **all charges consumed** regardless of outcome |
| Health thresholds (assist, once each) | Auto-CHANCE at **80%**, **55%**, **20%** Ganondorf HP |
| Fishing rod (easter egg) | Full hearts + full spurs + ≥70% ALBW meter; meter drains toward 70%; stagger → near-CHANCE within 2 hits; counts toward 2-win counter if CHANCE won |

**Outcomes:** Win = meaningful damage (through negation in P1). Fail = **10 hearts** damage (placeholder).

**Pressure:** Increased Ganondorf damage mult; failed parry = shield durability + **4 hearts** seeping damage (placeholder). Spur charge level scales player damage in P2 (values TBD).

**Human-form heart recovery:** Mirror wolf bite recovery structure — rate/trigger TBD.

**Open:** Ganon HP pool, negation %, spur scaling, damage mult, meter drain stop at 70% vs overshoot.

---

## Key source files

| Area | Files |
|------|-------|
| Boss Refinement core | `d_albw_boss.h/cpp` |
| Armogohma actor | `d_a_b_gm.cpp` |
| Diababa middle head | `d_a_b_bq.cpp`, `d_a_b_bq.h` |
| Diababa side tentacles | `d_a_b_bh.cpp`, `d_a_b_bh.h` |
| Fyrus | `d_a_e_fm.cpp`, `d_a_e_fm.h` |
| Darknut actor | `d_a_b_tn.cpp`, `d_a_b_tn.h` |
| Zant / Ganon | `d_a_b_zant.cpp`, `d_a_b_gnd.cpp`, `d_a_b_mgn.cpp` |
| Boss warp | `d_a_obj_bosswarp.cpp` |
| Statues (Armogohma) | `d_a_cstatue.cpp` |
| HP mult + lock-on HP | `d_albw_hp_mult.cpp/h` |
| Boss HP HUD | `d_albw_boss_hp_hud.cpp/h` |
| Shield parry/bash | `d_albw_shield.cpp/h`, `d_a_alink_guard.inc`, `d_a_alink_damage.inc` |
| Lockout | `d_albw_lockout.cpp/h` |
| Stage hooks | `d_s_play.cpp`, `f_op_actor.cpp` (true HP apply) |
| Collision | `d_cc_uty.cpp` |
| Settings UI | `src/dusk/ui/settings.cpp` |

---

## Bugs fixed during boss chat (regression watch)

| Issue | Fix area |
|-------|----------|
| Forest temple (Diababa) crash on enter | Shield combat context / attention lock |
| Z-target works but shield won't raise in boss | `checkAttentionLock` / manual shield gating |
| Armogohma egg infinite loop on warp | Arrow-count gate incompatible with ALBW meter bow |
| Armogohma softlock without Dominion Rod at warp | Warp bootstrap + cstatue switch bypass |
| Boss bar only moved on rod snaps, not bow | `dAlbwBoss_armogohmaFillDisplayHp()` + opening gate wiring |

---

## Chat archive note

Original session **`fbc16b50-bd3e-4cad-968e-e1c0bd114747`** (~1.1 GB in Cursor DB) was **deleted 2026-06-15** after this doc was verified. Do not reopen in Composer.

**To continue work:** open a **new** chat and reference the matching per-boss doc (Diababa / Gohma / Fyrus research) + `docs/state/boss-fights.md`.
