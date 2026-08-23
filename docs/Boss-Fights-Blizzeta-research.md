# Boss Fights — Blizzeta research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Blizzeta (Snowpeak Ruins), same style as
[Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md).

**Date:** 2026-08-23.

**No Refinement fight built yet** — research only.

---

## 0. Actor map

| Role | `fpcNm_*` | File | Arc |
|------|-----------|------|-----|
| Blizzeta (boss) | `B_YO` | `d_a_b_yo.cpp` / `daB_YO_c` | **`B_YO`**, **`L5_R50`**, `ykW` / `ykW1`, `E_FZ` |
| Icicle ring (×10) | `B_YOI` | `d_a_b_yo_ice.cpp` / `daB_YOI_c` | parent `B_YO` heap |
| Freezard spawns | `E_FZ` | `d_a_e_fz.cpp` | `E_FZ` (child of `B_YO`) |
| Yeta/Yukke warp stone | `OBJ_YSTONE` | `d_a_obj_ystone.cpp` | — |

**Arena:** Snowpeak boss room (`L5_R50` BG registered from `B_YO` create — normal + arena
`dBgW`).

**ALBW today:** HP mult + fight-victory rupees (`dAlbwEnemyRupees_tryGrantFightVictory` on
`ACT_DEATH`). Boss HP HUD help text names Blizzeta but **no query wired**. No
`dAlbwBossRefinement` hooks.

---

## 1. Full BCK list (`B_YO` — Yeta human form)

Enum `daB_YO_c::Animation` / `B_YO_RES_FILE_ID` in `d_a_b_yo.cpp` (matches `B_yo.h`).
All clips load through `setBck()` → `yo_demo_bck_idx[]`.

| ID | Symbol | Used? |
|----|--------|-------|
| 0x07 | `BCK_YKW_B_ANGRY` | Yes — opening / debug |
| 0x08 | `BCK_YKW_B_ANGRY_WAIT` | Yes — opening |
| 0x09 | `BCK_YKW_B_DOWN` | **ORPHAN** — in `yo_demo_bck_idx`, never `setBck` |
| 0x0A | `BCK_YKW_B_DOWNA` | Yes — death demo (wall impact) |
| 0x0B | `BCK_YKW_B_DOWNB` | **ORPHAN** — in table, never `setBck` |
| 0x0C | `BCK_YKW_B_DOWNC` | Yes — death demo (×3; skips B/DOWNB) |
| 0x0D | `BCK_YKW_B_FLOAT` | Yes — opening |
| 0x0E | `BCK_YKW_B_FLOATB` | Yes — serious demo |
| 0x0F | `BCK_YKW_B_SCREAM` | Yes — serious demo |
| 0x10 | `BCK_YKW_B_SCREAM_WAIT` | Yes — serious demo |
| 0x11 | `BCK_YKW_B_TALKA` | Yes — opening |
| 0x12 | `BCK_YKW_B_TALKB` | Yes — opening |
| 0x13 | `BCK_YKW_B_TO_TALKB` | Yes — opening |
| 0x14 | `BCK_YKW_B_WAITA` | Yes — opening / default heap |
| 0x15 | `BCK_YKW_B_WAITB` | Yes — opening |
| 0x16 | `BCK_YKW_B_WAITC` | Yes — serious demo |
| 0x17 | `BCK_YKW_B_WAITC2` | Yes — serious demo |
| 0x18 | `BCK_YKW_B_WAITD` | Yes — serious demo |
| 0x19 | `BCK_YKW_B_WALK` | Yes — opening |

**Hard BCK orphans (2):**

```
ORPHAN: DOWN (0x09), DOWNB (0x0B)
```

Live death chain: `DOWNA` → `DOWNC` (repeat) — **`DOWNB` and final `DOWN` are skipped**.

---

## 2. Fight has no attack BCKs

Unlike Fyrus/Diababa, **live combat never plays a Yeta BCK**. After `ACT_OPENING`, the visible
boss is the rolling **ice-ball stack** (`BMDE_YO01` / `YO02` / core models). Attacks are
procedural movement + child actors:

| Action | Symbol | What happens | Damage |
|--------|--------|----------------|--------|
| Chase / grow | `ACT_CHASE` | Roll, wall-bounce, scale up via `mModelNo` | Body `mAtCyl` while rolling |
| Freezard spray | `ACT_CHASE` (model ≥ 4) | Spawns `E_FZ` children | Freezard breath |
| Icicle orbit | `ACT_JUMP` → `ACT_ATTACK_ICE` | `B_YOI` ring tracks player, drops | Icicle At |
| Icicle gather | `ACT_ATTACK_YOSE` | Pulls ring toward Link | Sets up body slam |
| Body slam | `ACT_ATTACK_BODY` | Spin-drop + quake + pullout | Icicle-break throw + landing AoE |
| Mid-fight demo | `ACT_SERIOUS_DEMO` | Yeta form reappears (human BCKs) | — |
| Icicle revive demo | `ACT_DEMO_REVIVAL` | Re-spawns ring after partial break | — |
| Final phase chip | `ACT_DAMAGE` | 3× iron-ball hits → death | Iron ball only |

**Progression:** iron-ball hits during `ACT_CHASE` increment `mModelNo` (0→7). At
`mModelNo >= 6`, next iron hit triggers `ACT_SERIOUS_DEMO`. Final phase: **3** unshielded
iron hits (`mLastPhaseDamage`) → `ACT_DEATH`.

Create HP **80** (`health = 80`) but iron hits reset `health = 100` — **real pacing is
`mModelNo` + last-phase counter**, not the HP field.

---

## 3. Live fight flow

```text
ACT_OPENING (Yeta BCKs — walk/talk/angry/float, henshin to ice ball)
  → ACT_CHASE (roll + grow on iron-ball hits; freezards at model ≥ 4)
  → [optional] ACT_SERIOUS_DEMO at model ≥ 6 (human BCKs — scream/float/wait)
  → ACT_JUMP → ACT_ATTACK_ICE (icicle ring drop)
  → ACT_ATTACK_YOSE (gather icicles on Link)
  → ACT_ATTACK_BODY (slam / vibrate / jump away)
  → ACT_DEMO_REVIVAL (if icicles broken mid-fight)
  → ACT_DAMAGE ×3 iron hits (final form)
  → ACT_DEATH (DOWNA → DOWNC chain; Yeta human form)
```

---

## 4. Orphan notes (look-pass angles)

| BCK | Likely intent | Why unused |
|-----|---------------|------------|
| `DOWNB` | Middle stanza of collapse sequence | Death demo jumps `DOWNA` → `DOWNC` |
| `DOWN` | Final collapsed pose / hold | Same — `DOWNC` reused instead |

Both are wired in `yo_demo_bck_idx` / `yo_demo_btk_idx` (BTK `YKW_B_DOWN` shared with
`DOWNA/B/C`) so a look-pass could call `setBck(ANM_DOWN_B)` / `setBck(ANM_DOWN)` without
new assets.

**Not orphans:** every other YKW_B clip is referenced from opening, serious demo, or death.

There are **no cut attack animations** for the ice-ball phase — tuning is HIO (`mChaseSpeed`,
`mGatherTimer`, `mAttackTimer`, `mFreezardNum`, scale table `mScale[8]`).

---

## 5. Hard orphans (copy-paste)

```
ORPHAN: DOWN (0x09), DOWNB (0x0B)
LIVE substitute for death chain: DOWNA (0x0A) → DOWNC (0x0C)
No attack BCK orphans — combat is procedural / B_YOI / E_FZ
```

---

## 6. Refinement opportunities (design-only)

| Idea | Surface | Notes |
|------|---------|-------|
| Death-chain look-pass | `executeDeath` modes 2–110 | Wire `DOWNB` between `DOWNA` and `DOWNC` |
| Icicle pacing | `ACT_ATTACK_YOSE` / `ACT_ATTACK_BODY` | HIO timers already exposed |
| Freezard pressure | `setBreakFrizad` / `ACT_CHASE` | Scales with `mModelNo` |
| Boss HP HUD | `d_albw_boss_hp_hud.cpp` | Needs query on `mModelNo` or last-phase counter |
| Iron-ball window | `mTgIronSph[]` shield vs chill-key | Pause / shield timing is the skill gate |

---

## 7. Next steps

1. **User call:** Refinement scope — icicle loop tuning vs death-chain orphan look-pass.
2. **Playtest** vanilla Snowpeak if wiring `DOWNB` / `DOWN` in death demo.
3. **Optional:** trace whether `DOWN` was meant as the post-`DOWNC` hold before pit fall.

---

*Related:* [Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md),
[Boss-Fights-Stallord-research.md](Boss-Fights-Stallord-research.md),
[boss-fights-handoff.md](boss-fights-handoff.md).
