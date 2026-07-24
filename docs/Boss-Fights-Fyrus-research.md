# Boss Fights — Fyrus research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Fyrus (Goron Mines), same style as
the Diababa look-pass. **No Refinement fight built yet** — research only, keep
handy for a future Boss Refinement pass.

**Actor:** `E_FM` (`fpcNm_E_FM_e`, `d_a_e_fm.cpp` / `d_a_e_fm.h`). Arc **`E_fm`**.
Not `B_FR`. Chains/handles are in-archive (`BMDR_KUSARI`, `BMDR_HANDLE`,
`chain_s mChain[4]`), not separate boss procs.

**Related:** `Obj_fm` / `OBJ_FMOBJ` (arena chain platform, deleted mid-intro);
`Obj_HHASHI` pillars; occasional `E_BA` Keese spawns on blast.

**ALBW today:** HP mult + fight-victory rupees only. No TEMP look-pass, no
`d_albw_boss` Fyrus helpers, no Boss HP HUD query wired for `E_FM` yet.

**Date:** 2026-07-23.

---

## 1. Full BCK list (`E_fm`)

Local enum `E_FM_RES_FILE_ID` in `d_a_e_fm.cpp` (matches regional `E_fm.h`):

| ID | Symbol | Used in fight? |
|----|--------|----------------|
| 0x07 | `BCK_EF_FMATTACK_A` | Yes — blast FX |
| 0x08 | `BCK_EF_FMATTACK_B` | Yes — blast FX |
| 0x09 | `BCK_FM_ANIMAL` | Yes — fire breath |
| 0x0A | `BCK_FM_ANIMAL02` | Yes — roar / phase return |
| 0x0B | `BCK_FM_ATTACK` | Yes — far fire blast |
| 0x0C | `BCK_FM_ATTACK02` | Yes — near chain whip |
| 0x0D | `BCK_FM_BREAKCHAINL` | **ORPHAN** |
| 0x0E | `BCK_FM_BREAKCHAINR` | **ORPHAN** |
| 0x0F | `BCK_FM_CHANCE` | **ORPHAN** (SE stub only) |
| 0x10 | `BCK_FM_CHANCEDAMAGE` | Yes — core chip → damage-run |
| 0x11 | `BCK_FM_DAMAGEWALK` | Yes — staggered walk |
| 0x12 | `BCK_FM_DAMAGE_L` | **ORPHAN** |
| 0x13 | `BCK_FM_DAMAGE_R` | **ORPHAN** |
| 0x14 | `BCK_FM_DEAD` | **ORPHAN** |
| 0x15–17 | `BCK_FM_DEMOEND01/02/03` | Yes — death demo (03 = morph model) |
| 0x18 | `BCK_FM_DOWN` | Yes — chain-pull knock-down |
| 0x19 | `BCK_FM_DOWN02` | Yes — arrow-overkill down |
| 0x1A | `BCK_FM_DOWNDAMAGE` | Yes — sword hits while down |
| 0x1B | `BCK_FM_DOWNFR` | **ORPHAN** |
| 0x1C | `BCK_FM_DOWNWAIT` | Yes |
| 0x1D | `BCK_FM_HANGWAIT` | Yes — intro hang |
| 0x1E | `BCK_FM_KYORO2` | Yes — look-around |
| 0x1F | `BCK_FM_OPDEMO` | Yes — intro |
| 0x20 | `BCK_FM_UP` | Yes — recover from down |
| 0x21 | `BCK_FM_UP02` | Yes — heavy down-hit recover |
| 0x22 | `BCK_FM_WAIT01` | Yes |
| 0x23 | `BCK_FM_WALK` | Yes |
| 0x24 | `BCK_FM_WALK02` | Yes |

`anm_init` loads `"E_fm"` by res ID and sets `mAnm`.

---

## 2. Hard orphans (copy-paste)

```
ORPHAN: BREAKCHAINL (0x0D), BREAKCHAINR (0x0E), CHANCE (0x0F),
        DAMAGE_L (0x12), DAMAGE_R (0x13), DEAD (0x14), DOWNFR (0x1B)
LIVE substitute for CHANCE: CHANCEDAMAGE (0x10)
NO TEMP/Refinement hooks on E_FM yet
```

| BCK | Notes / look-pass angle |
|-----|-------------------------|
| `BREAKCHAINL` / `BREAKCHAINR` | Cut L/R chain-break reacts. Live cutoff is `carry_off()` + SE only — no BCK |
| `CHANCE` | Never `anm_init`’d; dead SE stub when `mAnm == BCK_FM_CHANCE` (`Z2SE_EN_FM_V_CHANCE`). Live vulnerability uses **`CHANCEDAMAGE`** |
| `DAMAGE_L` / `DAMAGE_R` | Side damage reacts — unused |
| `DEAD` | Dedicated death clip unused; death uses `DEMOEND01/02/03` |
| `DOWNFR` | Alternate down (front?) unused; live `DOWN` / `DOWN02` |

**Best first look-pass candidates:** `CHANCE` (SE already wired), `BREAKCHAINL/R`
(visual for existing `carry_off`), then `DAMAGE_L/R`, `DOWNFR`, `DEAD`.

---

## 3. Vanilla attack vocabulary

| Attack | Action | Anim | Damage driver |
|--------|--------|------|----------------|
| Near chain whip | `ACTION_N_FIGHT` | `ATTACK02` | Frame flags → `mChainAtSph` + hand `mAtSph` |
| Far fire blast | `ACTION_F_FIGHT` | `ATTACK` + TEXANM | Expanding `mEffAtSph` (fire) + FX BCKs |
| Fire breath | `ACTION_FIRE` | `ANIMAL` | Head-joint At + mouth particles |
| Roar | `ACTION_ANIMAL` | `ANIMAL02` | Particles only |
| Walk / pressure | fight-run / normal | `WALK` / `WALK02` / `WAIT01` / `KYORO2` | Body `mCcSph` fire At while active |
| Chain yank → down | `ACTION_STOP` → down cycle | → `DOWN` / `DOWNWAIT` / `UP` | Vulnerability = core `mCoreSph` |
| Core chip / exhausted | `DAMAGE_RUN` | `CHANCEDAMAGE` → `DAMAGEWALK` | Player damages him |

HIO ranges (approx.): near ~600, far ~1500.

---

## 4. Collision note (orphans)

**Playing an orphan BCK alone does not deal damage.** At is gated by per-frame
flags (`field_0x1829`, etc.), not by anim ID — same class of finding as Diababa’s
pre-wired lunge. Safe for visual/SE look-pass unless you also author At windows.

`mFEffAtSph[8]` is constructed but parked off-world and never `Ccsp()->Set` in
the fight — dead prep, unrelated to orphans.

---

## 5. Next steps (when starting Fyrus Refinement)

1. TEMP item→orphan BCK preview (Diababa pattern), gated on Boss Refinement.
2. Decide which orphans become fight verbs (`CHANCE` window? chain-break react?).
3. Wire Boss HP HUD query for `E_FM` if bars should show him.
4. Design contract in a future `Boss-Fights-RefinedFyrus.md` (do not invent here).

**See also:** [Boss-Fights-RefinedDiababa.md](Boss-Fights-RefinedDiababa.md) (look-pass → production pattern),
[boss-fights-handoff.md](boss-fights-handoff.md), [state/boss-fights.md](state/boss-fights.md).
