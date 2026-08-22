# Boss Fights — Morpheel research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Morpheel (Lakebed Temple), same style
as [Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md). **No Boss
Refinement fight built** — research only.

**Date:** 2026-08-19.

---

## 0. Actor map

| Role | `fpcNm_*` | File | Arc |
|------|-----------|------|-----|
| Body / core / fish | `B_OB` | `d_a_b_ob.cpp` | **`B_oh`** |
| Tentacle head (collision) | `B_OH` | `d_a_b_oh.cpp` | **`B_oh`** |
| Tentacle visual (IK rope) | `B_OH2` | `d_a_b_oh2.cpp` | **`B_oh`** (model `0x29`) |
| Bomb fish (phase 1) | `E_OctBg` | `d_a_e_oct_bg.cpp` | **`E_bg`** |
| Link hang anims | — | `daPy_anmHeap_c` arc slot 2 | **`B_oh`** AL_* indices |

Runtime lookups: **`fopAcM_GetName()` → `fpcNm_*`**, not C++ filenames.

**ALBW today:** Boss HP × + fight-victory rupees only. Boss HP HUD lists Morpheel
in help text but **no Morpheel-specific query wired** (same as most non-Armogohma /
non-Fyrus bosses). No `dAlbwBossRefinement` hooks in `d_a_b_ob.cpp`.

**Arena:** `D_MN01A` (Lakebed Temple boss room).

---

## 1. Full BCK list (`B_oh`)

Enum `B_oh_RES_File_ID` in `d_a_b_ob.cpp` / `d_a_b_oh.cpp` (matches
`assets/GZ2E01/res/Object/B_oh.h`):

| ID | Symbol | Used in fight? |
|----|--------|----------------|
| 0x08 | `BCK_AL_FOIFINISH` | **Link** — demo/hang arc via `daPy_anmHeap` (`B_oh` slot) |
| 0x09 | `BCK_AL_OICUT` | **Link** — same |
| 0x0A | `BCK_AL_OICUTB` | **Link** — same |
| 0x0B | `BCK_AL_OIFINISH` | **Link** — same |
| 0x0C | `BCK_AL_OIHANG` | **Link** — same |
| 0x0D | `BCK_AL_OIHANGMISS` | **Link** — same |
| 0x0E | `BCK_AL_OIHANGWAIT` | **Link** — same |
| 0x0F | `BCK_AL_OIHANGWAITB` | **Link** — same |
| 0x10 | `BCK_AL_OISPIT` | **Link** — same |
| 0x11 | `BCK_OH_CORE_OPEN` | **Init-only** — bound to `mpCoreMorf` at create; not replayed via `anm_init` |
| 0x12 | `BCK_OI_APPEAR` | Yes — phase 1 core rise |
| 0x13 | `BCK_OI_APPEAR02` | Yes — phase 2 fish intro |
| 0x14 | `BCK_OI_DEAD` | Yes — death |
| 0x15 | `BCK_OI_EAT` | Yes — core grab / swallow chain |
| 0x16 | `BCK_OI_EAT02` | Yes — post-swallow chew |
| 0x17 | `BCK_OI_LASTDAMAGE` | Yes — finisher hits |
| 0x18 | `BCK_OI_OPENMOUTH` | Yes — vacuum start |
| 0x19 | `BCK_OI_OPENMOUTHWAIT` | Yes — vacuum hold |
| 0x1A | `BCK_OI_SWALLOW` | Yes — vacuum swallow |
| 0x1B | `BCK_OI_SWIMWAIT` | Yes — fish idle loop |
| 0x1C | `BCK_OI_TENTACLE_END` | Yes — phase 1 → 2 transition |
| 0x1D | `BCK_OI_THROWUP` | Yes — spit Link after chew |
| 0x1E | `BCK_OI_WAIT` | Yes — core idle / hand-move loop |

**Hard BCK orphans in `B_oh`:** **none** — every indexed BCK is referenced. The
nearest “soft orphan” is **`BCK_OH_CORE_OPEN`** (core eye morf init bind only; live
core vulnerability uses manual `mpCoreMorf->setFrame(field_0x4740)`, not a replay of
that clip).

**Companion anims (not BCK):**

| ID | Type | Use |
|----|------|-----|
| 0x2F | `BRK_OH_LOOP` | Tentacle head + `B_OH2` pulse |
| 0x36 | `BTK_OH_LOOP` | Same |
| 0x32–34 | `BTK_AL_FOICUT*` / `FOIFINISH` | Link face during hang demos |
| 0x39 | `BTP_AL_FOIFINISH` | Link face |

---

## 2. Bomb fish (`E_bg` — `E_OctBg`)

Separate arc from `B_oh`. `d_a_e_oct_bg.cpp` loads model `0xA`, BCK `0x5`/`0x6`/`0x7`,
BRK `0xD`. Spawned in phase 1 by `OB_ACTION_BOMBFISH_SET` (up to 10 `E_OctBg` actors).
No orphan pass on `E_bg` in this doc yet — scope is boss-body clips.

---

## 3. Live fight flow (vanilla)

```text
Phase 1 — Core on tentacle (mCoreBattleMode)
  CORE_START → core rides B_OH tentacle joint → OI_APPEAR
  CORE_HAND_MOVE → grab / reposition on tentacle
  BOMBFISH_SET → spawn E_OctBg bomb fish
  CORE_HOOK → hookshot vulnerability window (spin + home return)
  CORE_CHANCE → player damage window after hook
  CORE_END → OI_TENTACLE_END → despawn B_OH tentacles → phase 2

Phase 2 — Fish body (mFishBattleMode)
  FISH_NORMAL → OI_SWIMWAIT patrol + tentacle slap via B_OH2 IK
  FISH_VACUME → OPENMOUTH → OPENMOUTHWAIT → SWALLOW → EAT02 → THROWUP
  FISH_END → LASTDAMAGE → OI_DEAD
```

**`B_OH` tentacle head:** attack/wait/caught are **procedural** (angle calcs +
`mColliders[]` At on joints 7–14) — **no BCK morf** on the head model beyond
`BTK_OH_LOOP` / `BRK_OH_LOOP`. Do not expect orphan tentacle *attack* BCKs in
`B_oh`; motion was authored in code + IK.

**`B_OH2`:** visual tentacle segments; `dmcalc()` IK chain; no action BCK.

---

## 4. Attack / damage catalog (live)

| Attack | Actor | Trigger | At / Tg |
|--------|-------|---------|---------|
| Tentacle sweep | `B_OH` | `OH_ACTION_ATTACK` when Link low + near | Colliders 7–14 Co → catch → `OH_ACTION_CAUGHT` |
| Core grab | `B_OB` | Core hand on tentacle + `OI_EAT` frames | Body At during eat window |
| Bomb fish | `E_OctBg` | Phase 1 spawn timer | Enemy bomb At |
| Hook window | `B_OB` | `CORE_HOOK` after damage | Link hookshot carry → `CORE_CHANCE` |
| Fish vacuum | `B_OB` | `FISH_VACUME` + Link in mouth range | Swallow → chew → throwup sequence |
| Fish body collision | `B_OB` | Phase 2 swim | `mFishHeadSph` / body sph |
| Hang minigame | Link + `B_OB` | Clawshot to eye / hang timer | Uses AL_* BCK from `B_oh` via player heap |

**Hang limits (vanilla):** `mHangFinishCount >= 3` extends timer; demo-driven
`AL_OIHANG*` / `AL_OICUT*` on Link.

---

## 5. Orphan summary (copy-paste)

```
B_oh BCK orphans: NONE (all 0x08–0x1E referenced)
Soft orphan: OH_CORE_OPEN (0x11) — create-time core morf only
AL_* BCKs: not orphans — Link demo/hang pipeline (not B_OB anm_init)
B_OH tentacle "attacks": procedural — no cut attack BCK in arc
E_OctBg: separate E_bg arc — not surveyed here
```

**Contrast with Fyrus:** Morpheel’s shared `B_oh` archive is **fully wired** for
fish-body phases. “Cut content” is more likely **discarded design** (procedural
tentacle instead of BCK swings) than unused clips sitting in the RARC.

### 5a. Phase 2 fish body — does it attack?

**Mostly no.** After `OI_TENTACLE_END` the eight `B_OH` tentacle actors are
**deleted**. The giant fish (`B_OB` phase 2) is not a conventional attacker:

| System | Role in phase 2 |
|--------|-------------------|
| `FISH_NORMAL` | Swim / chase (`OI_SWIMWAIT`); body-part sph are **Tg only** (player hits fish) |
| `FISH_VACUME` | Offensive beat — mouth sequence (`OPENMOUTH` → `SWALLOW` → `EAT02` → `THROWUP`) |
| `mFishHeadSph` / fin sph | **Tg only** — no `OnAtSetBit` on fish body during swim |
| `mBodySph` | **Co only** — arena collision at home, not a slam At |
| Hang minigame | Link on eye; `mHangTimer` — uses `AL_*` Link clips, not a fish attack BCK |

So the “giant Morpheel” **vacuums / swallows** and **takes damage** on exposed
body Tg; it does **not** run a separate body-slam or fin-sweep attack clip. Phase 1
offense lives on **tentacles** (`B_OH` procedural sweep + catch) and **bomb fish**
(`E_OctBg`), not the fish mesh.

**Refinement implication:** Morpheel is already a multi-layer set piece; low
payoff for orphan-BCK mining compared to Fyrus/Stallord.

---

## 6. Refinement opportunities

| Idea | Donor surface | Notes |
|------|---------------|-------|
| **Bubbled eye-mass P1** | §8 below | **Construction pass live** under Boss Refinement |
| Hang cap / chance window | `mHangTimer`, `CORE_CHANCE` | Still open |
| Boss HP HUD | `d_albw_boss_hp_hud.cpp` | **Next** after playtest — eye + body pools |
| Claw-only vuln polish | `CORE_HOOK` | Shell rebound already in §8 |

---

## 8. Boss Refinement — bubbled eye-mass (construction, 2026-08-19)

**Timing:** Vanilla intro demo runs fully. At title-card end we only *arm* a
pending handoff; `CORE_COMPOSITE` starts on the first calm frame after
`mDemoAction` clears and the event bus is idle (same-frame switch on Evt
teardown crashed). Then fight goes live (bubble + tentacles + chase) and the
bomb ring spawns staggered.

**Loop:**

```text
[vanilla intro] → COMPOSITE settle →
RING   — eye+tentacles swim; EF_OISUI scaled as bubble; 8× E_OctBg orbit;
         sword rebounds; clawshot blocked
CLAW   — bombs cleared; clawshot eye → pop particles → EXPOSED
EXPOSED — sword damages eye; HP≤0 → CORE_END → vanilla phase 2
```

**Files:**

| Surface | Role |
|---------|------|
| `d_albw_boss.{h,cpp}` | Phase state, orbit snap, tentacle root, tick |
| `d_a_b_ob.cpp` | `OB_ACTION_CORE_COMPOSITE`, spawn ring, chase, damage gates, bubble draw |
| `d_a_b_oh.cpp` | Eye-root tentacles while refinement active |
| `d_a_e_oct_bg.cpp` | Ring param snap after move |

**Donor ports:** Chu Worm dual-sphere / hook-pop idea (`E_SM`); bomb orbit from `E_OctBg` chase offsets; tentacle grab AI kept; `core_end` handoff unchanged.

**Playtest:** Boss Refinement **On**, Lakebed boss. Expect mass chase + bomb ring before any eye hook.

**Deferred:** true `E_SM` bubble BMD, HP HUD, eye-vs-body health split, grab tuning under bubble.

---

## 7. Next research steps

1. Playtest §8 construction under Boss Refinement.
2. Wire Morpheel Boss HP HUD (eye + body pools).
3. Stallord orphan survey remains available in parallel.

---

*Status:* **Refinement construction live** — orphan survey closed; P1 rewrite gated on Boss Refinement.

*Related:* [Boss-Fights-Stallord-research.md](Boss-Fights-Stallord-research.md),
[state/boss-fights.md](state/boss-fights.md),
[Boss-Fights-Fyrus-research.md](Boss-Fights-Fyrus-research.md).
