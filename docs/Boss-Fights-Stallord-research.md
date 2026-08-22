# Boss Fights — Stallord research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Stallord (Arbiter’s Grounds), same
style as [Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md).

**Date:** 2026-08-19.

---

## 0. Actor map

| Role | `fpcNm_*` | File | Arc |
|------|-----------|------|-----|
| Stallord (both phases + bullets) | `B_DS` | `d_a_b_ds.cpp` / `daB_DS_c` | **`B_DS`** |

Single actor class; **create param** selects mode (`daB_DS_c::Arg0`):

| Param | Symbol | Role |
|-------|--------|------|
| 0 | `TYPE_BATTLE_1` | Phase 1 — skeleton on sand, hand/breath |
| 2 | `TYPE_BATTLE_2` | Phase 2 — floating head + spinner chase |
| 1 / 3 / 4 | `TYPE_BULLET_A/B/C` | Breath fire columns (spawned by parent) |

**Arena:** `D_MN10A` (Stallord boss room).

**ALBW today:** Boss HP × + fight-victory rupees. No `dAlbwBossRefinement` hooks.
Boss HP HUD help text names Stallord but **no query wired** (same gap as Morpheel).

---

## 1. Full BCK list (`B_DS`)

Enum `daB_DS_c::AnmID` in `include/d/actor/d_a_b_ds.h` (matches `B_ds.h`):

| ID | Symbol | Used? |
|----|--------|-------|
| 0x06–08 | `ATTACK01_A/B/C` | Yes — `ACT_BREATH_SEARCH` (`executeBreathSearch`) |
| 0x09–11 | `ATTACK02_*_*` (9 clips) | Yes — `ACT_BREATH_ATTACK` (3 backbone tiers × start/loop/end) |
| 0x12–17 | `ATTACK_L/R_A/B/C` | Yes — `ACT_HAND_ATTACK` |
| 0x18–1D | `CIRCLE_L/R_A/B/C` | Yes — `ACT_CIRCLE` |
| 0x1E–21 | `DAMAGE_A/B` + `_WAIT` | Yes — phase 1 bone-break damage |
| 0x22 | **`ENDDEMO`** | **ORPHAN** — never `setBck`; P1 exit uses fall + spawn P2 actor |
| 0x23–28 | `HEAD_*` damage/down | Yes — phase 2 hit reactions |
| 0x29–2B | `HEAD_FATTACK_A/B/C` | Yes — P2 breath / tired attack chain |
| 0x2C | `HEAD_FWAIT` | Yes — P2 idle loop |
| 0x2D | **`HEAD_RETURN`** | **ORPHAN** — compared in `mBattle2_spinner_damage_check` but **never played** |
| 0x2E | **`HEAD_TIRED`** | **ORPHAN** — `ACT_B2_TIRED` reuses `HEAD_FATTACK_*` instead |
| 0x2F–30 | `LASTDAMAGE` + `_WAIT` | Yes — P1 finisher |
| 0x31–39 | `NODAMAGE_01/02/03_*` | Yes — `ACT_ETC_DAMAGE` (wrong-item / immune hits) |
| 0x3A | `OPDEMO` | Yes — opening |
| 0x3B | **`REVIVAL`** | **ORPHAN** — no reference in `d_a_b_ds.cpp` |
| 0x3C–3E | `WAIT01_A/B/C` | Yes — `ACT_WAIT` |
| 0x3F | `ZK_DSOP` | Yes — Zant intro (raw `getObjectRes` 63) |
| 0x40–43 | `ZNTA_DSOP_*` / `WAIT_*` | Yes — Zant model anims (indices 64–67) |

**Hard BCK orphans (4):**

```
ORPHAN: ENDDEMO (0x22), HEAD_RETURN (0x2D), HEAD_TIRED (0x2E), REVIVAL (0x3B)
```

---

## 2. Live fight flow

### Phase 1 (`TYPE_BATTLE_1`)

```text
OPDEMO (+ Zant warp sub-demo on B_DS Zant/Sword BMDs)
  → ACT_WAIT / ACT_CIRCLE (track Link)
  → ACT_HAND_ATTACK (L/R hand sweep, frames ~100+ At on hand cyls)
  → ACT_BREATH_SEARCH (ATTACK01 — hand sweep while turning)
  → ACT_BREATH_ATTACK (ATTACK02 — fire breath + spawn B_DS bullets)
  → ACT_DAMAGE / ACT_ETC_DAMAGE (bone break / nodamage flinch)
  → LASTDAMAGE → fall demo → fopAcM_create(TYPE_BATTLE_2) → delete self
```

**Backbone tiers:** `mBackboneLevel` 0/1/2 picks A/B/C variant of each attack family
(wait, circle, hand, breath, nodamage, damage).

### Phase 2 (`TYPE_BATTLE_2`)

```text
B2_OPENING_DEMO (sand rise, head assemble)
  → ACT_B2_WAIT → ACT_B2_F_MOVE (orbit + HEAD_FWAIT)
  → HEAD_FATTACK_A/B/C + fire breath (spinner chase)
  → ACT_B2_TIRED (same FATTACK chain, not HEAD_TIRED clip)
  → ACT_B2_DAMAGE (spinner hit → HEAD_DAMAGE / down chain)
  → ACT_B2_DEAD → END demo (camera + Zant sword — no ENDDEMO BCK)
```

---

## 3. Attack catalog (live)

| Attack | Phase | Action | BCK family | At |
|--------|-------|--------|------------|-----|
| Hand sweep L/R | 1 | `HAND_ATTACK` | `ATTACK_L/R_*` | Hand cyls, frame-gated |
| Breath search swipe | 1 | `BREATH_SEARCH` | `ATTACK01_*` | Hands frames 177–195 |
| Fire breath | 1 | `BREATH_ATTACK` | `ATTACK02_*` | Bullets + breath FX |
| Head fire sweep | 2 | `B2_F_MOVE` / `B2_TIRED` | `HEAD_FATTACK_*` | Breath + wall traps |
| Immune flinch | 1 | `ETC_DAMAGE` | `NODAMAGE_*` | None (feedback only) |
| Bone break | 1 | `DAMAGE` | `DAMAGE_*` / `LASTDAMAGE` | None |

---

## 4. Orphan notes (look-pass angles)

| BCK | Likely intent | Why unused |
|-----|---------------|------------|
| `ENDDEMO` | Dedicated fight-end clip | P1 deletes after fall; P2 uses long camera demo without this BCK |
| `REVIVAL` | Skeleton reassemble / phase transition | P2 intro demo replaces it |
| `HEAD_RETURN` | Head retreat after spinner hit | P2 uses `HEAD_DAMAGE` / `HEAD_DOWN` chain; `HEAD_RETURN` check is dead code |
| `HEAD_TIRED` | Exhausted idle before breath | `ACT_B2_TIRED` plays `HEAD_FATTACK_A→B→C` instead |

**SE hooks exist** for end (`Z2SE_EN_DS_END_*`) — audio without matching body BCK on
P2 finish.

---

## 5. Refinement opportunities (design-only)

| Idea | Surface | Notes |
|------|---------|-------|
| %-HP bone pacing | `mBackboneLevel` / damage thresholds | Natural Refinement seam in P1 |
| Wrong-item feedback | `ACT_ETC_DAMAGE` already has 9 `NODAMAGE` clips | Zant-style tool gate could reuse |
| Orphan look-pass | 4 BCK above | Lower priority than wiring HUD / phase pacing |
| Spinner / tired tuning | `ACT_B2_TIRED`, HIO `mP2*` | Gameplay knobs, not orphans |
| Boss HP HUD | `d_albw_boss_hp_hud.cpp` | Query on `B_DS` P1/P2 health pool |

---

## 6. Next steps

1. **User call:** Refinement scope — P1 bone pacing vs P2 spinner chase vs orphans-only.
2. **Playtest** vanilla `D_MN10A` if orphan look-pass is desired (`HEAD_TIRED`, `REVIVAL` reads).
3. **Optional:** trace whether `ENDDEMO` was meant for cut P1 ending before fall-handoff was authored.

---

*Related:* [Boss-Fights-Morpheel-research.md](Boss-Fights-Morpheel-research.md) (closed — no orphans),
[boss-fights-handoff.md](boss-fights-handoff.md),
[state/boss-fights.md](state/boss-fights.md).
