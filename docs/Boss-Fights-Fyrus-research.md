# Boss Fights — Fyrus research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Fyrus (Goron Mines), same style as
the Diababa look-pass. **No Refinement fight built yet** — research only, keep
handy for a future Boss Refinement pass.

**Actor:** `E_FM` (`fpcNm_E_FM_e`, `d_a_e_fm.cpp` / `d_a_e_fm.h`). Arc **`E_fm`**.
Not `B_FR`. Chains/handles are in-archive (`BMDR_KUSARI`, `BMDR_HANDLE`,
`chain_s mChain[4]`), not separate boss procs.

**Related:** `Obj_fm` / `OBJ_FMOBJ` (arena chain platform, deleted mid-intro);
`Obj_HHASHI` pillars; occasional `E_BA` Keese spawns on blast.

**ALBW today:** HP mult + fight-victory rupees. **TEMP orphan look-pass wired**
(Boss Refinement On). No permanent fight verbs / HP HUD / Golem cameo code yet.

**Date:** 2026-07-24.

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
TEMP look-pass: ACTION_ANM_PREVIEW on E_FM (Boss Refinement) — see §2a
```

| BCK | Notes / look-pass angle |
|-----|-------------------------|
| `BREAKCHAINL` / `BREAKCHAINR` | Cut L/R chain-break reacts. Live cutoff is `carry_off()` + SE only — no BCK |
| `CHANCE` | Never `anm_init`’d; dead SE stub when `mAnm == BCK_FM_CHANCE` (`Z2SE_EN_FM_V_CHANCE`). Live vulnerability uses **`CHANCEDAMAGE`** |
| `DAMAGE_L` / `DAMAGE_R` | Side damage reacts — unused |
| `DEAD` | Dedicated death clip unused; death uses `DEMOEND01/02/03` |
| `DOWNFR` | Alternate down (front?) unused; live `DOWN` / `DOWN02` |

### 2a. TEMP look-pass map (2026-07-24)

Gated: `#if TARGET_PC` + `dAlbwBossRefinement_isEnabled()`. Body `mCcSph[]`
normally have **Tg type 0**; look-pass opens projectile Tg only.

**Fix 2026-07-24 (pass 2):** first wiring used `OnTgSetBit()` only → Tg
`GetGrp()==0` so projectile At never matched. Also body fire At was burning
arrow Tg. Now: `SetTgSPrm(0x3)` + `OnAtNoTgHitInfSet`, apply before
`Ccsp()->Set`, also widen core Tg, probe log
`Documents/dusklight/albw_fyrus_lookpass_debug.txt`.

| Trigger | Orphan BCK | Notes |
|---------|------------|--------|
| Bow / arrow | `BCK_FM_CHANCE` | Frame-20 `Z2SE_EN_FM_V_CHANCE` already stubbed |
| Clawshot | `BCK_FM_BREAKCHAINL` | Single claw (`HOOKSHOT`, not W) |
| Double clawshot | `BCK_FM_BREAKCHAINR` | `mEquipItem == W_HOOKSHOT` |
| Boomerang | `BCK_FM_DAMAGE_L` | |
| Slingshot | `BCK_FM_DAMAGE_R` | |

Still unused: `DEAD`, `DOWNFR`. Preview plays once → `WAIT01` → `ACTION_NORMAL`.
No HP chip / no fight state change. Skip during START / END / DOWN / A_DOWN.

### 2b. Look-pass playtest (user 2026-07-24) — research only

| Orphan | Read as | Vs Diababa look-pass |
|--------|---------|----------------------|
| `CHANCE` (bow) | Stun / react | Less “new verb” than Diababa chips |
| `DAMAGE_L` / `DAMAGE_R` | Stun / react (both) | Same class — flinch, not a fight phase |
| `BREAKCHAINL` / `BREAKCHAINR` | Chain-break reacts (as named) | Closest to a real missing wire (`carry_off` is SE-only today) |
| `DEAD` / `DOWNFR` | Not playtested this pass | Still unused |

Takeaway: most orphans are **react clips**, not phase tools. Useful placement depends on
knowing the **fire-off / vulnerability pipeline** (§7) — stun BCKs do **not** extinguish
him by themselves.

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
| Fire Keese | side-effect of blast | — | `mDoCreateBa` → child `E_BA` |

HIO: near whip `field_0x24` ≈ **600**, far blast `field_0x20` ≈ **1500**, blast cooldown timer seed `field_0x40` ≈ **120**.

### 3a. Engage: chain whip (`ACTION_N_FIGHT`) — detail

**Entry** from `e_fm_fight_run` when either:

1. `pl_check(near≈600, sight 0x3000)` — player in front cone at whip range, or
2. Wall hit **and** pillar (`s_hasira_poscheck`) nearby

**Anim / At windows** (`BCK_FM_ATTACK02`, one-shot):

| Frames | `field_0x1828` (arm) | At | Notes |
|--------|----------------------|-----|-------|
| 30–40 | `1` (left / joint 0xF) | `field_0x1829=1` → hand `mAtSph` + `mChainAtSph` on that chain | Frame 35 `carry_off(1)`; frame 35ish `field_0x2399=2` chain FX |
| 50–60 | `0` (right / joint 8) | same | Frame 53 `carry_off(0)`; `field_0x2399=1` |

While `1829`: yaws toward player. End: 50% chance → `ACTION_FIRE` (breath) if still in near range, else `ACTION_NORMAL`.

**Reuse for B_GO:** whip is pure frame-flag At on existing chain spheres — not a separate actor. Golem cannot “borrow” this without either parenting to Fyrus chains or authoring its own At.

### 3b. Engage: fireball / arena blast (`ACTION_F_FIGHT`) — detail

**Entry** from fight-run when:

1. `pl_check(far≈1500, no sight gate)` **and** `mTimers[2]==0` (cooldown), or
2. Outside arena radius (`abs > HIO field_0xa0` ≈ 1100) sets `field_0x809=1`, and when facing player within ±0x400 → force blast

**Charge** (`e_fm_f_fight`): waits for `mpFmBtk[0]` frame 0 (sync), then `BCK_FM_ATTACK` + `TEXANM_ATTACK`. `tame_eff_set` charge particles; if `792==0` also spawns hollow-charge particles `0x81A3/0x81A4` (blast can still fire while extinguished!).

**Commit:** anim frame ≈ **88** → `field_0x790 = 1` (blast timeline in `effect_set`), quake, blast SE/voice. Optionally sets `mDoCreateBa` (see §3c).

**`field_0x790` timeline** (increments every `effect_set` tick while ≠0):

| Tick | Effect |
|------|--------|
| 2 | Start FX models `BCK_EF_FMATTACK_A/B`, particle `0x811B`, clear pillar actor cache |
| 10 | `carry_off` all 4 chains + wide `hasira_hahen_hit` |
| &lt; ~43 | Expanding `mEffAtSph` radius from FX frame; At vs player off if LoS blocked; `field_0x7b6=2` (bright blast light); pillar eff search |
| ≥ ~70 | `field_0x790 = 0` done |

Anim end → `TEXANM_FM`, often `ACTION_ANIMAL` (30%) or `NORMAL`.

**Reuse for B_GO:** blast damage is `mEffAtSph` on Fyrus + FX models already on `E_FM`. For a golem phase you’d either keep Fyrus frozen (no `790`) or spawn a separate AoE — do **not** assume B_GO has this pipeline.

### 3c. Fire Keese spawns (`E_BA`)

On blast commit only:

```text
if (arrowNum <= 5 || life <= 4) mDoCreateBa = TRUE;
```

Later in `action()`:

```text
cap at 4 live E_BA in room (s_ba_sub count)
create_num = 4 - ba_count
fopAcM_createChild(E_BA, parent=Fyrus, param=0xFFFF1F02, pos=eyePos, yaw spread)
```

Param `0xFFFF1F02` decode (`d_a_e_ba`):

| Field | Value | Meaning |
|-------|-------|---------|
| type bits 12–15 | `1` | `TYPE_FIRE` → arc **`E_fb`** |
| home low byte | `2` | `HOME_APPEAR` |
| distance nibble | `0xF` → 10 | fight-fly dist 1000 |
| path / switch | `0xFF` | none |

So: **fire Keese**, appear mode, children of Fyrus, soft mercy when Link is low on arrows/HP. Not a free mid-fight reinforce.

---

## 4. Collision note (orphans)

**Playing an orphan BCK alone does not deal damage.** At is gated by per-frame
flags (`field_0x1829`, etc.), not by anim ID — same class of finding as Diababa’s
pre-wired lunge. Safe for visual/SE look-pass unless you also author At windows.

`mFEffAtSph[8]` is constructed but parked off-world and never `Ccsp()->Set` in
the fight — dead prep, unrelated to orphans.

---

## 5. Next steps (when continuing Fyrus Refinement)

1. ~~TEMP item→orphan BCK preview~~ **done** (§2a–2b).
2. ~~Fire-off / vulnerability research~~ **done** (§7).
3. ~~Whip / blast / Keese + 50%→15% B_GO design~~ **done** (§3a–c, §8).
4. Implement §8 window (not started) — then `Boss-Fights-RefinedFyrus.md`.
5. Wire Boss HP HUD query for `E_FM` if bars should show him.

**See also:** [Boss-Fights-RefinedDiababa.md](Boss-Fights-RefinedDiababa.md) (look-pass → production pattern),
[boss-fights-handoff.md](boss-fights-handoff.md), [state/boss-fights.md](state/boss-fights.md).

---

## 6. Cut Goron Golem cameo (`B_GO` + `B_GOS`) — research (2026-07-24)

**Idea (superseded by §8 HP% window):** originally “on first down.” Live design is
**50% stun → B_GO → 15% despawn** — see §8.

**Design lock (user 2026-07-24, refined same day — see §8):** At **50%** Fyrus HP,
stun + `field_0x792=0` + spawn B_GO (kids stick, never detach). Shared pool
**50%→15%** via proxy hits on golem → E_FM. At **15%**, cascade despawn B_GO,
Fyrus fight continues. No temporary merge cycle.

**Actors / UI already present:**

| Piece | Name | Notes |
|-------|------|--------|
| Parent | `B_GO` (`fpcNm_B_GO_e`, `d_a_b_go.cpp`) | Arc `B_go` — amalgam skeleton |
| Children | `B_GOS` × **31** (`GORON_CHILD_MAX`) | Arc `B_gos` — small Gorons |
| Spawner | Cut Actors + ImGui “Goron Golem boss (B_GO)” | Despawn already cascades `createChild` lineage (~31 kids) |

Do **not** confuse with `B_GG` (Aeralfos).

### 6a. Composite / “merge”

On `B_GO` Create: spawn **31** `B_GOS` children in a ±500 XZ scatter, each with param = child index `0…30` → `mJointIndex` into `j_info[]` (31 joint attach slots on the parent skeleton).

Parent timer `field_0x692` toggles:

| Value | Child behavior |
|-------|----------------|
| **2** | Children → `ACTION_BALL` → `ACTION_STICK` (lerp onto parent joint matrices) |
| **1** | Detach → `ACTION_WAIT` / walk again |

So “merge” is a **temporary stick-to-skeleton cycle**, not a permanent single mesh. Visually they clump into a giant; mechanically they stay 31 actors + 1 parent.

Parent Draw is gated by HIO `mDisplayModelImage` (**default false**) — the big model is **invisible** unless that HIO is flipped. What you see in play is mostly the stuck children.

### 6b. Health pools — your suspicion confirmed

| Actor | Create HP | `damage_check` | Practical result |
|-------|-----------|----------------|------------------|
| `B_GO` | `health = field_0x560 = **1000**` | **empty stub** | Never takes damage |
| each `B_GOS` | **1000 / 1000** | **empty stub** | Never takes damage |

→ **31 + 1 independent pools that nothing ever drains.** There is no working fight HP today. For a Fyrus cameo, do **not** use these pools as-is; proxy sword Tg hits → `E_FM` (or a Refinement drain helper) and ignore/freeze child HP.

### 6c. Collision — why Link clips through

**Parent `B_GO`:** declares `dCcD_Sph` ×2 + `dCcD_Cyl` in the class, but Create/Execute **never** `Set` / `Ccsp()->Set` them. Attack frames only set `unk_0x660` (1/2) — **no At enabled**. Parent has bg Acch wall only.

**Children `B_GOS`:**

| Phase | Collider |
|-------|----------|
| Walk / wait / ball | `mCyl` **Co only** (r≈60, h≈60 at feet) — push, not a solid body volume |
| Stick (merged) | **Co bit OFF** — no Cc at all while stuck to joints |
| At | Src At type **0** — children never damage Link |
| Tg | Present on cyl Src (`0xd8fbfdff`) but unused without `damage_check` |

So: amalgamated giant = **no attack volume and no usable body volume**; free children = tiny foot cylinders. Matches “clip through at the right angle” and “wonky attack/normal collision.”

Cameo work would need Refinement Cc (larger body Tg on parent and/or stuck children, optional At) — not a HIO tweak.

### 6d. Fyrus hook points

§8 gates on **HP%**, not vanilla DOWN. Still useful: DOWNWAIT-style freeze (no
whip/blast), `792=0` hollow, cascade despawn. Arena: Mines boss room needs
clearance for 31 stuck kids; despawn must cascade (Cut Actors already does).

### 6e. Cameo feasibility (verdict)

| Question | Answer |
|----------|--------|
| Can he appear in the fight? | **Yes** — spawn `B_GO` (loads `B_gos` kids) under Boss Refinement |
| Shared HP bar with Fyrus? | **Yes, by proxy** — wire Tg hits → drain `E_FM` (and Boss HP HUD). Do not rely on B_GO/B_GOS health |
| Is he fight-ready now? | **No** — empty damage, invisible parent mesh by default, no At, Co useless when stuck |
| Biggest cameo blockers | (1) damage + proxy HP (2) solid Tg while stuck (3) optional show parent model (4) clear/fail condition + cascade despawn (5) Mines arena space / softlock if spawn fails |

**Suggested implement order:** see **§8** (HP% gate, not vanilla first-down).

## 7. Fire-off / vulnerability pipeline (research 2026-07-24)

Two **independent** systems. Confusing them is the usual trap.

### 7a. Master “on fire” flag — `field_0x792` (`s8`)

| Value | Meaning |
|-------|---------|
| **1** | Ablaze (default after intro / skip-intro Create) |
| **0** | Extinguished (“hollow” / sword-safe body) |

**What `field_0x792` gates every frame (`effect_set` + Execute Cc):**

| System | When `792 != 0` (on) | When `792 == 0` (off) |
|--------|----------------------|------------------------|
| 14 joint fire particles + `Z2SE_EN_FM_BURNING` | Spawned / leveled | Skipped |
| Chain TEV red (`mChainColorR`) | Hot target ~40 | Cool toward 0 |
| Body `mCcSph[]` At vs player | `OnAtVsPlayerBit` | `OffAtVsPlayerBit` — **no contact burn** |
| Boss light / `field_0x7b6` ambient | Driven toward “burning” | Dim path |
| `dComIfGs_BossLife_public_Set` | 100 while burning | 0 |
| Real-time shadow | Suppressed while burning | Drawn when off |

**Turning fire OFF (only combat sites):**

Both knock-down impact frames do the same block:

```text
if (field_0x792 != 0) {
    field_0x792 = 0;
    mPlayTexAnmNo = TEXANM_PUTOUT_WAIT;   // BRK/BTK_FM_PUTOUTWAIT
    reset PUTOUT_WAIT brk/btk frame 0
}
```

| Path | Action | Impact frame | BCK |
|------|--------|--------------|-----|
| Chain yank → stop → down | `e_fm_down` mode 1 | **frame 90** | `BCK_FM_DOWN` |
| Arrow-overkill / 10 core chips upright | `e_fm_a_down` mode 1 | **frame 112** | `BCK_FM_DOWN02` → then `ACTION_DOWN` mode 2 |

Nothing else in the fight clears `792`. Core chips while upright (`CHANCEDAMAGE` /
`DAMAGE_RUN`) leave him **still on fire**.

**Turning fire ON again:**

| When | Where |
|------|--------|
| Intro OPDEMO ~frame 339 | `e_fm_start` mode 2: `792 = 1`, kankyo `field_0x5c8 = 1` |
| After down get-up | `e_fm_down` mode 4 (`BCK_FM_ANIMAL02`): **frame 60** → `792 = 1`; anim end → `TEXANM_FM` |

### 7b. `TEXANM_PUTOUT` explained (orphan extinguish transition)

Fyrus body materials use parallel **BRK** (TEV reg) + **BTK** (texture SRT) slots
selected by `mPlayTexAnmNo`. Heap init table index:

| Index | Enum | Res pair | Init play mode |
|------:|------|----------|----------------|
| 0 | `TEXANM_FM` | `BRK/BTK_FM` | loop (2) — default ablaze look |
| 1 | `TEXANM_ATTACK` | `…_ATTACK` | loop — blast charge look |
| 2 | **`TEXANM_PUTOUT`** | **`BRK_FM_PUTOUT` / `BTK_FM_PUTOUT`** | loop in init, but… |
| 3 | `TEXANM_PUTOUT_WAIT` | `…_PUTOUTWAIT` | loop — hollow hold |

**What PUTOUT is for:** the one-shot *visual extinguish* (materials cooling /
flame tex dying). Vanilla **never sets** `mPlayTexAnmNo = TEXANM_PUTOUT`. At down
impact it jumps straight to **`TEXANM_PUTOUT_WAIT`** (hollow idle materials) while
also clearing `field_0x792`.

**Why PUTOUT still behaves like a one-shot if you play it:** Execute clamps every
slot *except* `TEXANM_FM` and `TEXANM_PUTOUT_WAIT` to end−2 once it reaches the
tail — so `PUTOUT` plays through then freezes on the last frames (transition),
while `PUTOUT_WAIT` keeps looping as the sustained hollow look.

**Recommended hollow enter (Refinement / B_GO window):**

```text
field_0x792 = 0;                    // kill particles + body burn At
mPlayTexAnmNo = TEXANM_PUTOUT;      // play extinguish once (orphan — use it)
// on PUTOUT near-end / next mode:
mPlayTexAnmNo = TEXANM_PUTOUT_WAIT; // hold hollow materials
// + stun BCK (CHANCE / DAMAGE_* / DOWNWAIT — pick at implement)
```

Leaving `792=0` with `TEXANM_FM` still active would look wrong (ablaze mats, no
particles). Vanilla’s skip of PUTOUT is why down can feel abrupt — Refinement
can restore the missing beat.

### 7c. Core glow / targetability — `field_0x770` (separate from fire)

| Value | Effect |
|-------|--------|
| **0** | Core collider parked off-world (`eyePos.x + 20000`); not a Tg target |
| **1** | Core live at eye joint; BRK glow toward frame 29; particles if scale > ε |
| **>1** | Countdown toward 1 (intro uses large values) |

**Upright vulnerability (fire still ON):** core Tg hit → `field_0x804++` →
`ACTION_DAMAGE_RUN` + `BCK_FM_CHANCEDAMAGE` (live “chance” verb). At `804 >= 10` →
`ACTION_A_DOWN` (`DOWN02`) which **then** puts fire out at impact.

**Down vulnerability (fire OFF after impact):** `ACTION_DOWN` mode 2 (`DOWNWAIT`) —
core Tg → real HP via `cc_at_check`, `DOWNDAMAGE` flinch. `field_0x770` stays **1**
through DOWNWAIT (cleared only when UP starts: mode 3 sets `770 = 0`, restored to 1
when fully recovered).

Event bit **F_0257** (“hollow state”) is forced ON every frame of DOWNWAIT mode 2.

### 7d. Chain release — `carry_off(chainNo)` (BREAKCHAIN hook)

```text
carry_off:
  if grabbed (617e): clear + Z2SE_EN_FM_CHAIN_CUTOFF
  if player chain-grab (617c): clear, cancelFmChainGrab, shock
```

**No BCK.** Sites that already call it (natural `BREAKCHAINL/R` attach points):

- Near whip (`ATTACK02`) release frames
- Far blast (`field_0x790` timeline) — all 4 chains
- UP roar recovery (~frame 65) — all 4
- Intro arm swings — chains 0/1 then 2/3
- Death / todome paths

L/R orphans map cleanly to chain index side (0/2 vs 1/3) if/when wired.

### 7e. Implications for orphan placement (facts only — no design lock)

| If you want… | Must touch… | Orphan that only plays BCK is not enough |
|--------------|-------------|------------------------------------------|
| Safe approach / no body burn | `field_0x792 = 0` (+ usually `TEXANM_PUTOUT_WAIT`) | Stun clips leave fire/At on |
| Visible extinguish transition | Optionally `TEXANM_PUTOUT` then WAIT | Vanilla skips PUTOUT |
| Sword HP phase | `ACTION_DOWN` + core Tg (after fire-off impact) | Same |
| Upright “opened” stagger | Core Tg → live `CHANCEDAMAGE` path (`804`) | `CHANCE` is unused twin of that |
| Chain-break juice | Call site of `carry_off` + `BREAKCHAINL/R` | SE exists; BCK does not |
| Stun react without phase change | `DAMAGE_L` / `DAMAGE_R` / `CHANCE` as flinch | Does not clear `792` |

**Vanilla phase sketch (for placement context):**

```text
Ablaze (792=1, 770=1)
  → shoot glowing core → CHANCEDAMAGE / DAMAGEWALK (still ablaze)
  → yank chain while exhausted → STOP → DOWN
  → impact frame → 792=0 + PUTOUT_WAIT  ← fire dies here
  → DOWNWAIT sword core (real HP)
  → UP → ANIMAL02 frame 60 → 792=1  ← fire returns
```

---

## 8. Design lock — 50%→15% B_GO window (2026-07-24)

**Yes — feasible.** Matches the fire-off / stun / proxy-HP pieces already mapped.
Research lock only; not coded.

### 8a. Contract

| Gate | Behavior |
|------|----------|
| Fyrus HP crosses **≤ 50%** of `field_0x560` (true max, after Boss HP ×) | Enter **stun hold** once; `field_0x792 = 0`; play **`TEXANM_PUTOUT` → `TEXANM_PUTOUT_WAIT`**; stun BCK (pick at implement: `CHANCE` / `DAMAGE_*` / `DOWNWAIT`); **spawn `B_GO`** (kids stick, never detach — §6) |
| While golem live | Hits on B_GO/B_GOS **proxy-drain E_FM** (shared bar). Fyrus does **not** whip / blast / breath / walk-At. Core Tg on Fyrus optional off so damage only goes through golem |
| Fyrus HP crosses **≤ 15%** | Cascade despawn B_GO + kids; end stun; **resume Fyrus fight** (decide at implement: re-light `792=1` + `TEXANM_FM`, or stay hollow until a later UP) |

```text
[ ablaze fight ]
    │ HP ≤ 50% (once)
    ▼
stun + PUTOUT → PUTOUT_WAIT + 792=0 + spawn B_GO (stuck)
    │ shared pool: proxy hits → E_FM  (50% … 15%)
    ▼
HP ≤ 15% → despawn B_GO → Fyrus continues
```

### 8b. Why this fits the code

- **`792 = 0`** is exactly vanilla “safe body” (no contact burn, particles off) — §7a.
- **`TEXANM_PUTOUT`** is the missing extinguish transition vanilla skipped — §7b; use it on the 50% enter beat, then hold WAIT for the whole golem phase.
- **Stun BCK** alone does not clear fire — must pair with `792` + tex (§2b / §7e).
- **Shared pool** must ignore B_GO’s dead `health=1000` stubs — proxy only (§6b).
- **Fyrus attack mute** during window: do not run `fight_run` → `N_FIGHT` / `F_FIGHT` (§3a–b); clear `field_0x790` / `1829` so leftover blast/whip At cannot fire. Keese (`mDoCreateBa`) only arms on blast commit — muted if blast never starts (§3c).

### 8c. Open implement choices (not locked)

1. Which stun BCK loops for the hold (`CHANCE` vs `DAMAGE_L/R` vs `DOWNWAIT`).
2. After 15%: re-ignite fire immediately, or stay hollow until next vanilla down/UP.
3. Snap HP to exactly 50% on enter (prevent overshoot from a big hit), or only latch the phase flag.
4. Can Fyrus still take core damage during golem phase, or golem-only?
5. B_GO offense: none (statue) vs new At — vanilla B_GO has **no** whip/blast/Keese; those stay on `E_FM` (§3).

### 8d. Suggested implement order

1. Refinement HP% latch on `E_FM` (50% enter / 15% exit).
2. Stun hold + `792=0` + `PUTOUT`→`PUTOUT_WAIT`; mute fight actions / At.
3. Spawn `B_GO`, force stick, show mesh HIO, Tg + proxy → E_FM.
4. Exit: cascade delete, resume Fyrus, clear latch.
5. Playtest Mines arena space / softlock; then write `Boss-Fights-RefinedFyrus.md`.

---

*End Fyrus research (orphans §1–5, attacks §3, Golem §6, fire-off §7, B_GO window §8).*
