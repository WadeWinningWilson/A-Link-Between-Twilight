# Boss Fights — Argorok research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Argorok (City in the Sky), same style as
[Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md).

**Date:** 2026-08-23.

**No Refinement fight built yet** — research only.

> **Actor correction:** Argorok is **`B_DR`** (`d_a_b_dr.cpp`). Not `B_OH` (Morpheel).

---

## 0. Actor map

| Role | `fpcNm_*` | File | Arc |
|------|-----------|------|-----|
| Argorok (fight + bullets/parts) | `B_DR` | `d_a_b_dr.cpp` / `daB_DR_c` | **`B_DR`** |
| Death / phase-2 demo dragon | `B_DRE` | `d_a_b_dre.cpp` / `daB_DRE_c` | **`B_DR`** (shared) |
| Opening flight demo | `L7op_demo_dr` | `d_a_L7op_demo_dr.cpp` | **`B_dr`** |
| Bridge destroy demo | `L7demo_dr` | `d_a_L7demo_dr.cpp` | **`B_dr`** |
| Peahat (camera handoff) | `E_PH` | `d_a_e_ph.cpp` | — |

**Create param (`arg0`):** `0xFF` = main fight, `0xFE` = armor-break / part-sync child,
`0x10` = breath bullet, `0x14`/`0x15` = body parts, `0xA` = draw-only part.

**Arena:** City in the Sky boss platform (pillar search uses `mPillarCc` + room switches).

**ALBW today:** HP mult + fight-victory rupees on tail-kill path. Boss HP HUD names
Argorok; **MS collider / any-sword gate** mentioned in handoff — verify before Refinement.
No `dAlbwBossRefinement` hooks in `d_a_b_dr.cpp`.

**Vanilla HP:** `health = 24` at create. Weak-point (back) cuts: **1** HP (normal sword) or
**2** (Master Sword), doubled on charged slash. Tail spike path uses separate damage demo.

**Armor phases:** `field_0x7d1` — 0 = armored, 1 = partial, 2 = core exposed (weak point
active).

---

## 1. Full BCK list (`B_DR` — dragon body)

`#define ANM_DR_*` in `d_a_b_dr.cpp` (matches `B_dr.h` 0x10–0x46). Fight actor loads via
`setBck()` → `dComIfG_getObjectRes("B_DR", i_anm)`.

### 1a. Fight actor (`B_DR` / `d_a_b_dr.cpp`)

| ID | Symbol | Used in fight? |
|----|--------|----------------|
| 0x10 | `BOOT_A` | Yes — boot / recovery |
| 0x11 | `BOOT_A_DEMO` | Yes — `executeDamageDemo` intro |
| 0x12 | `BOOT_B` | Yes — tail-hit phase |
| 0x13 | `BOOT_C` | Yes — ground impact / down |
| 0x14 | `BRAKE_TO_HOVER` | Yes — flight transitions |
| 0x15 | `BRIDG_DESTROYA` | **ORPHAN** — no `setBck` / demo consumer in repo |
| 0x16 | `BRIDG_DESTROYB` | **ORPHAN** — same |
| 0x17 | `BURU` | Yes — armor shatter sync (`arg0 0xFE`) |
| 0x18 | `CATCH` | Yes — weak-point window open |
| 0x19 | `CRAZY` | Fight: **no** — see `B_DRE` |
| 0x1A–1C | `DAMAGE_L/R`, `DAMAGE_LAST` | Yes — weak-point / finisher |
| 0x1D | `DEATH` | Fight: **no** — see `B_DRE` |
| 0x1E | `DEMO_DESTROY` | Fight: **no** — `L7demo_dr` |
| 0x1F | `DOWN_WAIT` | Yes — grounded idle |
| 0x20 | `FALL` | Yes — knockdown |
| 0x21 | `FALL_DEATH` | Fight: **no** — see `B_DRE` |
| 0x22 | `FALL_DOWN` | Yes — heavy fall |
| 0x23 | `FALL_REVIVE` | **ORPHAN** — no consumer anywhere in `src/` |
| 0x24 | `FALL_SCREAM` | Fight: **no** — see `B_DRE` |
| 0x25–27 | `FEINT_A/B/C` | Yes — breath feints |
| 0x28–2C | `FIREA`–`FIREE` | Yes — breath attack chain |
| 0x2D–2F | `FLAP`, `FLAP_TO_FLY`, `FLY` | Yes — hover / pursuit |
| 0x30–33 | `FLY_TO_GLIDE`, `GLIDE`, `GLIDE_TO_FLY` | Yes — glider attack |
| 0x32 | `GLIDE_BRAKE` | **SE stub only** — never `setBck` |
| 0x34 | `HOVER_TO_FLY` | Fight: **no** — `L7op_demo_dr` |
| 0x35–36 | `HOVERING`, `HOVERING_B` | Yes — idle / ride window |
| 0x37 | `IMPACT` | Yes — pillar / ground hit |
| 0x38 | `MID_WAKEUP` | Fight: **no** — see `B_DRE` |
| 0x39 | `OP_SCREAM` | Fight: **no** — `L7op_demo_dr` |
| 0x3A–3D | `POLE_STAYA`–`D` | Yes — pillar cling attack |
| 0x3E | `RIDE` | Yes — dragon ride weak-point |
| 0x3F–40 | `SHAKEA/B` | Yes — tail / weak-point setup |
| 0x41 | `TAKE_OFF` | Yes — leave ground |
| 0x42 | `TURN` | **SE stub only** — `L7op_demo_dr` uses raw 0x42 |
| 0x43 | `WIND_ATTACKA` | **SE stub only** — live wind uses `ATTOB` → `B` |
| 0x44–46 | `WIND_ATTACKATOB`, `B`, `BTOFLY` | Yes — wind attack (not A) |

### 1b. Link cutscene BCK (same arc, 0x07–0x0F)

`AL_DRCUT`, `AL_DRFINISH`, `AL_DRHANG`, etc. — present in `B_dr.h`, **no `getObjectRes("B_DR"`
consumer found in `src/`**. Likely event/demo driven on Link's side.

---

## 2. Hard orphans — fight scope (`B_DR` only)

```
ORPHAN (never played): BRIDG_DESTROYA (0x15), BRIDG_DESTROYB (0x16), FALL_REVIVE (0x23)
SE-ONLY (setSe case, never setBck): GLIDE_BRAKE (0x32), TURN (0x42), WIND_ATTACKA (0x43)
LIVE wind substitute: WIND_ATTACKATOB (0x44) → WIND_ATTACKB (0x45)
```

**Encounter-wide (not fight orphans):** `DEATH`, `CRAZY`, `FALL_SCREAM`, `FALL_DEATH`,
`MID_WAKEUP` → `B_DRE`; `OP_SCREAM`, `HOVER_TO_FLY`, `TURN` → `L7op_demo_dr`;
`DEMO_DESTROY` → `L7demo_dr`.

---

## 3. Live fight flow

```text
Opening (L7op_demo_dr — OP_SCREAM, HOVER_TO_FLY, TURN, flight BCKs)
  → B_DR create (health 24, armored)
  → ACTION_WAIT / FLY_WAIT (hover / track Link)
  → ACTION_WIND_ATTACK (WIND_ATTACKATOB → B — not A)
  → ACTION_BREATH_ATTACK (+ FEINT_A/B/C, FIREA→E chain; spawns breath bullets)
  → ACTION_GLIDER_ATTACK (GLIDE / FLY_TO_GLIDE; floor wind At)
  → ACTION_PILLAR_SEARCH / PILLAR_WAIT (POLE_STAY* on pillars)
  → ACTION_TAIL_HIT (spike chain — separate kill path)
  → ACTION_WEEK_HIT (weak point — CATCH / RIDE / SHAKE / DAMAGE)
  → ACTION_DAMAGE_DEMO / MIDDLE_DEMO (camera + BOOT_A_DEMO)
  → Armor break (BURU via 0xFE child) → field_0x7d1 = 2
  → Kill → B_DRE death demo (DEATH / CRAZY / FALL_* / MID_WAKEUP)
```

---

## 4. Attack catalog (live — `B_DR`)

| Attack | Action | Anim chain | At / effect |
|--------|--------|------------|-------------|
| Hover / chase | `WAIT`, `FLY_WAIT` | `HOVERING`, `FLAP`, `FLY` | Body Tg; sets up next attack |
| Wind blast | `WIND_ATTACK` | `WIND_ATTACKATOB` → `B` | `mWindAtCc*` wind spheres |
| Fire breath | `BREATH_ATTACK`, `BREATH_ATTACK2` | `FIREA`→`FIREB` loop, `FIREC/D/E` | `mBreathSet` + child bullets |
| Breath feint | (inside breath) | `FEINT_A/B/C` | Cancels into hover / reposition |
| Glider strafe | `GLIDER_ATTACK` | `FLY_TO_GLIDE` / `GLIDE` / `GLIDE_TO_FLY` | Low glide wind + foot At |
| Pillar perch | `PILLAR_SEARCH`, `PILLAR_WAIT` | `POLE_STAYA`→`D` | Cling + drop breath |
| Tail spike | `TAIL_HIT` | `BOOT_*`, `IMPACT`, `SHAKEA` | Spike kill route |
| Weak point | `WEEK_HIT` | `CATCH`→`RIDE`→`SHAKEB`→`DAMAGE_*` | MS/normal sword chip on core |
| Ground down | (shared) | `FALL`, `FALL_DOWN`, `DOWN_WAIT` | Down colliders |

HIO highlights: `wind_large_attack_lockon_range`, `breath_continue_time` /
`breath_continue_no_armor_time`, `no_attack_time`, feint timers, glider wind power.

---

## 5. Orphan notes (look-pass angles)

| BCK | Likely intent | Why unused |
|-----|---------------|------------|
| `BRIDG_DESTROYA/B` | Bridge collapse before arena | Authored; no demo actor references clip in this repo |
| `FALL_REVIVE` | Recover from fall death? | No `setAnm` anywhere |
| `WIND_ATTACKA` | Wind wind-up | Skipped — code goes straight to `ATTOB` |
| `GLIDE_BRAKE` | End-of-glide slowdown | Glide exits via `GLIDE_TO_FLY` / hover instead |
| `TURN` (fight) | In-flight heading change | Opening demo uses it; fight uses `FLAP` + angle calc |

**SE hooks exist** for `WIND_ATTACKA`, `GLIDE_BRAKE`, `TURN` in `setSe()` — frame keys ready
if a look-pass ever plays those clips on `B_DR`.

---

## 6. Hard orphans (copy-paste)

```
FIGHT ORPHAN: BRIDG_DESTROYA (0x15), BRIDG_DESTROYB (0x16), FALL_REVIVE (0x23)
SE-ONLY: GLIDE_BRAKE (0x32), TURN (0x42), WIND_ATTACKA (0x43)
LIVE wind: WIND_ATTACKATOB (0x44) → WIND_ATTACKB (0x45)
ENCOUNTER (not fight orphans): DEATH/CRAZY/FALL_*/MID_WAKEUP → B_DRE; OP_SCREAM/HOVER_TO_FLY → L7op; DEMO_DESTROY → L7demo
LINK BCK 0x07–0x0F: arc only — no src consumer
```

---

## 7. Refinement opportunities (design-only)

| Idea | Surface | Notes |
|------|---------|-------|
| Wind wind-up look-pass | `executeWindAttack` / `mGliderAtAniSet` | Play `WIND_ATTACKA` before `ATTOB` |
| Glide brake | `mGlider_AniSet` | `GLIDE_BRAKE` has glide SE already stubbed |
| Bridge intro | new demo hook or `L7demo_dr` | `BRIDG_DESTROY*` assets exist |
| %-HP armor pacing | `field_0x7d1`, `health <= 8` breath branch | Natural Refinement seam |
| MS / sword gate | `executeWeekHit` equip checks | Handoff lists Argorok — verify ALBW layer |
| Boss HP HUD | `d_albw_boss_hp_hud.cpp` | Wire `B_DR` health (24 pool) |

---

## 8. Next steps

1. **User call:** Refinement scope — breath/feint tuning vs orphan look-pass (`WIND_ATTACKA`,
   `GLIDE_BRAKE`).
2. **Playtest** opening + kill demos to confirm `BRIDG_DESTROY*` truly absent (cutscene-only?).
3. **Optional:** grep event data / Link demo tables for `AL_DRCUT` BCK indices.

---

*Related:* [Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md),
[Boss-Fights-Morpheel-research.md](Boss-Fights-Morpheel-research.md),
[boss-fights-handoff.md](boss-fights-handoff.md).
