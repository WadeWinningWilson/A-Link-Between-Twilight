# Boss Fights — Refined Diababa (Forest Temple)

Canonical record of Diababa research + Boss Refinement fight work in ALBW
Dusklight. Split from [boss-fights-handoff.md](boss-fights-handoff.md) so that
file stays a multi-boss archive; handoff cross-references here.

**Actors:** `B_BQ` middle head (`d_a_b_bq.cpp`) + `B_BH` side tentacles
(`d_a_b_bh.cpp`) + `E_MB` Ook (`d_a_e_mb.cpp`). Arcs: `B_bq`, `B_bh`.
`E_GB` (Giant Baba) is **not** this fight.

**Gate:** all refined fight behavior is behind `game.bossRefinement`
(`dAlbwBossRefinement_isEnabled()`). Refinement **off** → vanilla flow.
Helpers live in `d_albw_boss.h` / `d_albw_boss.cpp`.

**Overall status (2026-07-24):** **Finished** for v1 Refinement fight — 70% late
phase, siphon (poison/lunge + side-head 3% boss-max), hang/lunge/barrage, body
chips. Door-volume pressure / rhythm-parry remain design-only follow-ups.

---

## 0. Refined fight — design contract (Boss Refinement ON)

| Rule | Value |
|------|--------|
| **Base HP pool** | PC Create seeds `health = field_0x560 = 100`, then Boss HP ×N |
| **Late phase** | Sticky when remaining HP **≤ 70%** of max (bomb −30 from full 100 enters late) |
| **Hang hard cap** | **15 s** (`450` frames) under Refinement **or** **4 core hits** / kill → RETURN |
| **Hang hit close** | Refinement: `field_0x1392` counts down from 4 per core hit (combo `cutCount>=4` also) |
| **Bomb reception** | Flat **−30** (does **not** use FA/lockout); skip vanilla `health = 50` |
| **Body chips** | Arrow / iron ball only: FA+lockout resolved AP × **0.85**; flinch BCK, no `mAction` change |
| **Poison aim** | **2800**; sway **±0x100**; head track **+5%** (HEAD-joint beam; no sphere retarget) |
| **Siphon** | Poison/lunge: `(dmg/LinkMax)×bossMax` (1×/attack); side heads: **3% of Diababa max** per damaging hit |
| **Phase 1** | Bomb → hang → chance/return → post-return poison + siphon |
| **Phase 2 (≤70%)** | Bomb → RUNAWAY thrash **with** poison/siphon → quick L/R/L/R/BOTH → pop out → hang → vanilla spray → wait |
| **Side-head roll** | Wait intervals × **0.6** (−40%) |
| **Side-head attack** | Execute × **1.15** (anim + timers + dash speed) |
| **Side-head travel** | Max dash home→reach **1700 → 2800** |
| **Middle lunge** | eyePos≤**1700**; HEAD At r=**337.824** (+2% from 331.2); head-only aim; siphon on hit |
| **Side-head At** | CcSph radius **×1.02** under Refinement |

Vanilla vulnerability (hang) window after bomb-down anim: HIO **`mChanceTime = 200`**
frames (~6.7 s @ 30 Hz). Early exit if `cutCount >= 4` or `health <= 0`.

---

## 1. Actor / HP / HUD map

| Role | `fpcNm_*` | File |
|------|-----------|------|
| Middle head | `B_BQ` | `d_a_b_bq.cpp` / `.h` |
| Side heads | `B_BH` | `d_a_b_bh.cpp` / `.h` |
| Ook / bomb delivery | `E_MB` | `d_a_e_mb.cpp` |

- HP mult + Boss Health Bars list `B_BQ` / `B_BH` in `d_albw_hp_mult.cpp`.
- Display: plain `health` / `field_0x560` via `dAlbwBoss_diababaQueryHealthBar`
  (no Armogohma-style composite).
- Vanilla Create never set HP; hang hard-assigned **50**. PC seed **100/100**
  so bars and Boss HP × have a max before the first bomb.

---

## 2. Live fight flow

```text
Phase 1 (HP > 70% remaining):
  bomb → hang (−30) → carve core → chance/return → poison (2800) + siphon
  (first bomb from 100 → 70 enters late; sticky thereafter)

Phase 2 (sticky late, remaining HP ≤ 70%):
  bomb → RUNAWAY thrash with poison/siphon attached to the head
      → submerge + side-head L/R/L/R/BOTH (both mID slots via daB_BH_getPtr)
      → APPEAR pop-out → vanilla hang (4 hits or 15s) → vanilla spray → wait
```

**Hooks:** `dAlbwBoss_diababaUpdatePhase` / `IsLatePhase` / siphon /
`SetRetaliationPoison` / `SetPendingHangAfterAppear` /
`TakeChipLookMAlternate`. Link siphon: `ACTION_ATTACK` or retaliation poison flag.

**Ook / demo:** `field_0x6fb == 3` during submerged conductor; pauses/aborts
if `mDemoMode != 0`.

---

## 3. Vanilla research (kept for tuning)

### Side-head pacing (`B_BH`)

| Lever | Vanilla | Role |
|-------|---------|------|
| `attack_freq_a` | 350 | P1 per-head re-roll (`freq + rnd(0…freq)`) |
| `attack_freq_b` | 250 | Main-head-up window |
| `B_BQ.field_0x6fe` | 200–350 / 100–200 | Shared gate after a B-phase attack starts |
| Range gate | player &lt; **2800** of anchor | No commit if far |
| Post-attack `mTimers[0]` | 60–110 | Recovery before next wait cycle |
| `field_0x6fb != 0` | rewrites timers | Suppresses normal rolls during hang/poison |

Practical arena cadence (order-of-magnitude): ~3–7 s between *some* head
attack when both hot and Link in range; per-head rolls ~8–17 s.

### Middle-head poison

- Trigger: `field_0x11fc++` on bomb, then `mTimers[2] == 1` after return-to-wait.
- Spray: HIO `mWaterSprayTime` **150** frames; 4× `mCcMizuSph` (r=35),
  `AT_TYPE_CSTATUE_SWING`, AtSpl **0xd** (large).
- Vanilla: no siphon.

### Hang / return

- Submerge uses `BCK_BQ_RETURN01` / `RETURN02` inside `ACTION_DAMAGE` only
  (scripted timers, **not** player distance).
- Chance window: **`mChanceTime = 200`**.

---

## 4. Orphan BCK look-pass (research → production)

Archive `B_bq` includes clips that vanilla never `anm_init`s in combat.

| BCK | Research ID | Production use (Refinement) |
|-----|-------------|-------------------------------|
| `BCK_BQ_ATTACK` | Cut lunge | **Live** `ACTION_LUNGE` (bomb-wait) |
| `BCK_BQ_ATTACK_C` | Damage react | Body chip flinch |
| `BCK_BQ_LOOK_M` | Damage react | Body chip flinch (every other chip) |
| `BCK_BQ_TESTMOTION` | Damage react | Body chip flinch (iron ball path) |
| `BCK_BQ_RUNAWAY` | Never in vanilla gameplay | **Live** phase-2 post-poison submerge before side-head 5-hit |

**Side-head orphan:** `BCK_BH_WAIT01` — still not wired.

### Lunge collision / targeting (audit → fix)

- Vanilla `mCcSph` is Tg/Co only (At type 0). Poison uses separate mizu At.
- Playing `BCK_BQ_ATTACK` alone never damaged Link — At must be authored.
- **Live:** during lunge active frames, body `mCcSph` gets B_BH At
  (`SetAtSPrm(0xd)` = SET + VsPlayer), seated on `JNT_HEAD`.
- Targeting: combat never yaws `shape_angle.y` toward Link (only `mHeadRot`).
  Lunge snaps/tracks yaw for **~90%** of attack frames, then locks.

---

## 5. Lockout + Focused Arts (reception order)

Item hits that use `cc_at_check` resolve FA/lockout **before** Diababa
scalars. Diababa only applies boss-side reception after that.

| Hit | Reception (Refinement) |
|-----|------------------------|
| Arrow / iron ball body | ×0.85 chip, flinch, no flow interrupt |
| Bomb hang | flat **−30** (bomb branch never calls `cc_at_check`) |
| Core sword hang | full resolved melee AP |

---

## 6. Door / threshold safe zone (design only)

Camping the door stays safer than mid-arena because of the **2800** roll gate,
**1700** vanilla lunge cap (now 2800 under Refinement), poison LOS, and
boomerang-aim collider shrink. Options A–F compared in the handoff archive;
recommended stack still **door volume + middle-head response** — not coded yet.

---

## 7. Implementation checklist

- [x] Sticky late phase + siphon helpers (`d_albw_boss`)
- [x] Body chips 85% + flinch BCKs
- [x] Poison aim 2800 + Link damage siphon hook
- [x] Conductor: static beat latch (not `mTimers[3]`) + int beat (not `field_0x1393` s8)
- [x] Revert poison sphere retarget (HEAD-joint aim only)
- [x] Phase-2 bomb → poison → submerge+5-hit → appear → hang → vanilla spray
- [x] Shared L/R/L/R/BOTH conductor while middle head submerged
- [x] Ook `field_0x6fb==3` + demo softlock guard
- [x] Middle-head lunge + HEAD/body At + 90% track
- [x] Side-head travel 2800; roll −40%; attack +15%
- [ ] Door volume / door-zone reach
- [ ] Rhythm-parry poison immunity window
- [ ] Optional distance retreat
- [ ] Full playtest pass (Boss Refinement On, Health Bars On, HP × as desired)

---

## 8. File index

| Area | Files |
|------|--------|
| Fight state / siphon / phase | `include/d/d_albw_boss.h`, `src/d/d_albw_boss.cpp` |
| Middle head | `src/d/actor/d_a_b_bq.cpp` |
| Side heads | `src/d/actor/d_a_b_bh.cpp` |
| Ook | `src/d/actor/d_a_e_mb.cpp` |
| Siphon hook | `src/d/actor/d_a_alink_damage.inc` |
| HP HUD query | `src/d/d_albw_boss_hp_hud.cpp` |
| Multi-boss archive | [boss-fights-handoff.md](boss-fights-handoff.md) |
| Live state | [state/boss-fights.md](state/boss-fights.md) |

---

## 9. Open / stamp blockers (2026-07-23)

1. **Side-head barrage** — latch/`s8` beat spam fixed (2026-07-23); re-playtest mID1 freeze. Probe: `Documents/dusklight/albw_diababa_conductor_debug.txt` (expect `DONE` per beat, no runaway `beat=`).
2. **Lunge** — At r=**331.2**; head-only Link aim; siphon on hit. Playtest.
3. **Hang 4-hit close** — now counts real core hits (`field_0x1392`); playtest.
4. **Poison head track** — still vanilla math (see §9a); optional stronger `mHeadRot` later.

### 9a. Poison tracking (reference)

After hang return → wait: `field_0x11fc` armed, `mTimers[2] = 80`. When `timers[2]==1` → `ACTION_ATTACK`.

| Lever | Behavior (vanilla + Refinement) |
|-------|----------------------------------|
| Body yaw | not tracked |
| Head aim | vanilla `(angle)/3`; Refinement poison: **+5%** (`105/300`) toward Link, ±3500 clamp |
| Spray sway | vanilla `±0x500`; Refinement **`±0x100`** (less drift) |
| Beam / At | HEAD joint forward `poisonAim` + Y 500 + Z sin wobble (VFX + spheres share this) |
| Aim length | 2300 vanilla / **2800** Refinement |

**Reverted (2026-07-23):** player-chest `spD8` retarget — desynced At from purple VFX.

## 10. Open / tune later

- Conductor: one side head downed mid-sequence; interrupt policy
- Siphon: one-per-spray already; extra max-% cap?
- Hang feel vs vanilla 200f chance (15s hard cap under Refinement)
- Door camping stack
- Rhythm parry vs spray cadence
