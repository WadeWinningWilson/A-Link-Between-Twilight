# Boss Fights — Fyrus research (preliminary)

Preliminary orphan-BCK / cut-attack pass for Fyrus (Goron Mines), same style as
the Diababa look-pass. **No Refinement fight built yet** — research only, keep
handy for a future Boss Refinement pass.

**Actor:** `E_FM` (`fpcNm_E_FM_e`, `d_a_e_fm.cpp` / `d_a_e_fm.h`). Arc **`E_fm`**.
Not `B_FR`. Chains/handles are in-archive (`BMDR_KUSARI`, `BMDR_HANDLE`,
`chain_s mChain[4]`), not separate boss procs.

**Related:** `Obj_fm` / `OBJ_FMOBJ` (arena chain platform, deleted mid-intro);
`Obj_HHASHI` pillars; occasional `E_BA` Keese spawns on blast.

**ALBW today:** HP mult + fight-victory rupees + **Boss HP HUD** (`Twilit Igniter FYRUS`).
**TEMP orphan look-pass wired** (Boss Refinement On). **§8/§9 B_GO window wired** (Refinement On):
50% stun + PUTOUT + stuck Golem (kids on hidden `IS` skeleton) → proxy HP → 15% kid peel, stay hollow.
Refinement Create HP **200** (vanilla 50), then Boss HP ×. 0 HP → Fyrus floor-down, last hit `END`.

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
**Core Tg is not look-pass** — weak-spot shots fall through to `cc_at_check` (bar + `CHANCEDAMAGE`). Body-only projectiles still map as above.

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

**Frequency (vanilla — no dedicated cooldown):** Whip fires whenever `FIGHT_RUN` sees the near cone **or** wall+pillar. There is **no** `mTimers[]` gate. It “doesn’t fire every time” because:

1. Must already be in `FIGHT_RUN` (not mid-blast / breath / down / stagger).
2. Front cone `0x3000` (~67.5°) — beside or behind him, no whip.
3. After a whip, **50%** of finishes divert to fire breath (`ACTION_FIRE`) if still in range — that’s the main extra wait.
4. Near-range whip **preempts** blast (`else if` on the far check).
5. Return path `NORMAL` → `FIGHT_RUN` reseeds blast CD, but whip can fire again immediately if still in the cone (including during the 20–30f wait-in-place).

Refinement **+50% anim speed** shortens the whip clip itself (~2/3 wall-clock), which raises whip cadence whenever he isn’t diverted to breath. **Frequency knobs not changed this pass** (candidate: drop the 50% breath divert, or widen the cone).

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

**Frequency (vanilla — this one *does* have a timer):**

| Gate | Value @ 30 logic Hz |
|------|---------------------|
| `mTimers[2]` seed on every `FIGHT_RUN` mode 0 | `HIO field_0x40` (120) + rnd(0..60) → **120–180f ≈ 4.0–6.0s** |
| Far range | `pl_check(≈1500)` and **not** in whip cone (whip `else if` wins up close) |
| Out-of-arena (`field_0x809`) + facing ±0x400 | **Ignores** `mTimers[2]` — forced blast |
| Start sync | `F_FIGHT` waits until `mpFmBtk[0]` (idle FM tex) hits frame 0 before charging |

Reseed happens whenever he re-enters `FIGHT_RUN` (after whip, breath, blast→NORMAL, etc.). Staying in `FIGHT_RUN` mode 2 lets the timer run down once, then blast can repeat as soon as far-range is true.

Refinement **+50% execution** (anim 1.5× + `field_0x790` extra tick every other frame + FX 1.5×) makes the charge/explosion shorter; **it does not cut `mTimers[2]`**. So blast still waits ~4–6s between `FIGHT_RUN` entries. Candidate frequency bump (not this pass): seed **80 + rnd(0..40)** (2/3 of vanilla = +50% cadence).

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

### 3d. Attack trigger reference (master table)

HIO defaults @ 30 logic Hz: near **600**, far **1500**, blast CD seed **120** (+0..60 rnd),
arena radius **~1100** (`field_0xa0`). Refinement **+50%** anim/FX speed on whip, blast,
breath (does **not** shorten blast CD timer).

| Attack | Action / anim | Trigger (must already be in fight — not down / Golem hold / kid phase*) | Damage / effect | Notes |
|--------|-------------|------------------------------------------------------------------------|-----------------|-------|
| **Engage** | `NORMAL` → `FIGHT_RUN` | Same BG as Link (`!fopAcM_otherBgCheck`) while wandering | — | Reseeds blast CD `mTimers[2]` |
| **Chain whip** | `N_FIGHT` / `ATTACK02` | From `FIGHT_RUN` walk: **(A)** Link within **~600** and **~67°** front cone, **or (B)** wall hit + pillar (`HHASHI`) within **500** ahead | `mChainAtSph` + hand `mAtSph`; frames **30–40** (L arm) / **50–60** (R arm) | No dedicated cooldown; near check **beats** blast (`else if`) |
| **Fire breath** | `FIRE` / `ANIMAL` | **50%** after whip ends if still in near **600** + front cone | Joint-3 `mAtSph` (breath window **~98–9a**); mouth fire particles | Distinct from roar `ANIMAL02` |
| **Blast AOE** | `F_FIGHT` / `ATTACK` + `TEXANM_ATTACK` | From `FIGHT_RUN` walk: **(A)** Link within **~1500**, **`mTimers[2]==0`**, **or (B)** pushed outside arena (`field_0x809`) + facing Link **±~45°** (ignores CD) | Expanding `mEffAtSph` + FX models; commit **`field_0x790`** @ anim **~88** | Waits `TEXANM_FM` or **`PUTOUT_WAIT`** BTK frame 0 (hollow fix) before charge |
| **Roar** | `ANIMAL` / `ANIMAL02` | **30%** after blast ends; **or** `DAMAGE_RUN` mode 1 timer path | Particles only (`animal_eff_set` type 2) | No At |
| **Latent body fire** | passive (walk / idle) | **`field_0x792 != 0`** (ablaze) + Link touches body `mCcSph[]` | Fire contact At on 8 body spheres | **See §3e — broken in current Refinement pass** |
| **Chain yank → down** | `STOP` → `DOWN` | Rear chain (2/3) stretched + grab while walking; or pillar slam timing | Opens core; impact **extinguishes** (`792=0`) | Refinement: no HP snap to 50 on down |
| **Core chip** | `DAMAGE_RUN` / `CHANCEDAMAGE` | Core `mCoreSph` Tg hit while upright | `cc_at_check` → real HP pool | Does **not** clear `792` |
| **Fire Keese** | child `E_BA` | Blast commit when arrows **≤5** or hearts **≤4** | Fire Keese AI | Cap **4** live in room |
| **Look-pass flinch** | `ANM_PREVIEW` orphan BCK | Refinement: body projectile Tg (not core) | No HP / no phase change | Bow/hook/boom/sling → orphan react |

\*Refinement **kid phase** (15% peel → last B_GOS dead): Fyrus forced passive — no
`FIGHT_RUN`, attn/look suppressed; resume only after `fyrusOnGolemKidsCleared`.

**Priority in `FIGHT_RUN` (same frame):** chain whip check **first**, then blast CD /
far range, then edge blast.

### 3e. Latent body fire — status (playtest 2026-08)

Vanilla **“walk into ablaze Fyrus → chip damage”** is **`field_0x792`** gating body
`mCcSph[]` **`OnAtVsPlayerBit`** every Execute (`792==0` → **`OffAtVsPlayerBit`**).

| Phase | Expected body burn | Current Refinement behavior |
|-------|-------------------|----------------------------|
| **Pre-50%** (lit, `792=1`) | Contact fire on approach | **Reported broken** — open bug; suspect Cc / look-pass interaction, not yet root-caused |
| **50%→15%** Golem window | Off (`792=0` + stun hold) | Correct (by design) |
| **15% kid peel + after** | Off (`fyrusStayHollow`, no re-light on get-up) | Correct (by design) |

Do **not** “fix” post-50% hollow by re-lighting `792` without a design pass — §8/§9
require stay-hollow. Pre-50% lit-phase burn needs a donor-aligned Cc audit (body At vs
look-pass Tg grp).

**Investigation (2026-08-19) — likely root cause pre-50%:**

Refinement look-pass (`e_fm_albwLookPassApplyBodyTg`) puts **projectile Tg** on the
same **`mCcSph[0..7]`** used for latent body **At**. While open it also sets
**`OnAtNoTgHitInfSet()`** so arrow/claw At doesn’t write bogus Tg hit info (comment in
`d_a_e_fm.cpp`: “Body fire At also hits arrow Tg”).

`cCcS::SetAtTgCommonHitInf` gates At↔Tg crosses on **`!pat_obj->ChkAtNoTgHitInfSet()`**.
With that bit set on Fyrus’s body At, **At→Link Tg contact may never register** even
when `792==1` and `OnAtVsPlayerBit` is on — vanilla never dual-purposed these spheres.

**Confirm A/B:** Boss Refinement **Off** → walk into lit Fyrus pre-50% (burn should
return). Refinement **On** → burn absent while look-pass Tg is active.

**Sanctioned fix direction (next pass):** separate look-pass Tg from body-burn At
(donor pattern: don’t share the same sphere for both), or only enable `AtNoTgHitInf` on
projectile-class hits — not a blanket body-sphere flag.

### 3f. Proposed tuning (not implemented — playtest targets)

| Knob | Current | Requested | Where |
|------|---------|-----------|--------|
| Breath after whip | **50%** (`cM_rndF(1.0f) < 0.5f` in `e_fm_n_fight`) | **70%** | One literal in `d_a_e_fm.cpp` |
| Whip / blast / breath anim speed | Refinement **1.5×** (`kAlbwFmAttackAnimSpeed`) | **+15% faster** — clarify intent: **1.15× vanilla** vs **1.5×→1.725×** vs **replace 50% boost with 15%** | `kAlbwFmAttackAnimSpeed` + blast `field_0x790` extra tick |
| Blast cooldown seed | **120 + rnd(0..60)** frames on each `FIGHT_RUN` entry | **80 + rnd(0..40)** (~2.7–4 s) | HIO `field_0x40` |
| Blast far range | **1500** | **1200** (narrower band) | HIO `field_0x20` |
| Breath execute speed | **1.0×** (`BCK_FM_ANIMAL`) | **1.15×** uniform; morf **3** on `ANIMAL` start (WAIT01 kept before `ACTION_FIRE`) | `e_fm_fire` / `kAlbwFmBreath*` |

### 3g. Golem-phase room camera (investigation — 2026-08-19)

**Symptom:** Room camera still **centers / pulls toward Fyrus** during B_GO clump
(phase 1) and B_GOS kid crowd (phase 2), instead of vanilla boss-room framing on
the active threat.

**Two separate systems (do not conflate):**

| System | Driver | Golem phase today |
|--------|--------|-------------------|
| Link look / neck | `daPy_py_c::setLookPos` → `mLookPosFromOut` | Phase 1: redirect to B_GO `eyePos` via `dAlbwBoss_fyrusTryGolemLookPos`. Phase 2: **`set_look_pos = false`** — no update; stale pos may remain Fyrus. |
| Chase / lock framing | `dAttention` **`fopAc_AttnFlag_BATTLE_e`** → `LockonTarget(0)` | Phase 1: B_GO gets BATTLE attn. Phase 2: **31× B_GOS** get BATTLE attn; B_GO attn cleared; Fyrus attn off. |

**Likely causes (ranked):**

1. **Phase 2 has no look redirect** — `fyrusTryGolemLookPos` gates on
   `fyrusGolemWindowIsLive()` (**phase 1 only**). Kid phase clears Fyrus look
   but never repoints; `mLookPosFromOut` can stay on Fyrus `eyePos` from earlier
   fight frames (`daAlink_c::getLookAtPos` still reads it when
   `ERFLG0_UNK_4` + angle check pass).

2. **`setLookPosFromOut` origin-distance gate** — once `ERFLG0_UNK_4` is set,
   a new look pos is **rejected** if the stored pos is closer to **world origin**
   than the candidate (`current.pos.abs2(mLookPosFromOut) <
   current.pos.abs2(*i_pos)`). In the Mines arena this can **block the B_GO
   redirect** even in phase 1, leaving Fyrus eyePos winning.

3. **Fyrus geometric anchor** — Fyrus stays at room center (huge, stun hold).
   Chase cam `relationalPos(mpPlayerActor, …)` is player-anchored, but lock-on
   roll bias (`mCamParam` flag **0x1000**) and attention midpoint math still
   blend toward **`LockonTarget(0)`**. With 31 kids, target may hop; Fyrus mass
   at center still reads as the “boss pole” in the room.

4. **`onBossRoomWait()` still runs** on E_FM every fight frame (vanilla boss
   rooms) — keeps boss-room player stance; **not** the center bug by itself, but
   confirms the room is still “Fyrus boss context.”

**Would “order camera to track Golem” help or murkier?**

| Phase | Track B_GO? | Verdict |
|-------|-------------|---------|
| **1 — clump** | Already intended (`setLookPos` + B_GO BATTLE attn). Fix **abs2 gate** + verify lock target clears Fyrus, don’t add a second cam owner. | **Clarify existing wiring** before new camera code. |
| **2 — kids loose** | B_GO is frozen/passive; camera on parent **ignores the fight**. | **Murky / wrong** — track **nearest B_GOS** or kid-cluster centroid, not parent. |
| **Fyrus-only fight** | User wants **vanilla** cam. | **Do not** leave golem overrides active outside phases 1–2. |

**Sanctioned fix direction (next pass, not this one):**

- Extend look redirect through **phase 2** (kids), or **clear** `mLookPosFromOut` /
  `ERFLG0_UNK_4` when entering golem phases so Fyrus cannot stick.
- Fix or bypass the **origin `abs2`** compare for Fyrus→golem handoff (donor:
  boss cam uses attention lock, not this helper — audit before baking).
- Ensure Fyrus is **removed from lock-on list** when `attn_on == false` (not
  only flags cleared — verify `dAttention` drops stale E_FM entry).
- Phase 2: pick **one** camera policy — nearest kid lock, or no lock (pure
  vanilla room cam) — avoid dual B_GO + kid targets.

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
4. ~~Implement §8 window~~ **wired** (Refinement On) — playtest Mines, then `Boss-Fights-RefinedFyrus.md`.
5. ~~Wire Boss HP HUD query for `E_FM`~~ **done** (`dAlbwBoss_fyrusQueryHealthBar`, name **Twilit Igniter FYRUS**). Hidden during `ACTION_START` / `ACTION_END`. Gated on `game.bossHealthBars`. Vanilla knockdown still resets `health` to 50, so the bar can refill on a down until §8 changes the pool. **Refinement:** upright core hits now `cc_at_check` (no A_DOWN `health=50` snap).

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

**§8:** do **not** force the parent mesh on. `B_go` BMD `IS` is the same leftover skeleton as cut `E_IS` (Armos Titan). Kids stick to parent joints with the Titan mesh hidden — invisible armature + 31 gorons.

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
**Wired 2026-08-18** (Boss Refinement On). Playtest Mines next.

### 8a. Contract

| Gate | Behavior |
|------|----------|
| Fyrus HP crosses **≤ 50%** of `field_0x560` (true max, after Boss HP ×) | Enter **stun hold** once; `field_0x792 = 0`; play **`TEXANM_PUTOUT` → `TEXANM_PUTOUT_WAIT`**; random-rotate stun BCKs (`CHANCE` / `DAMAGE_L` / `DAMAGE_R`); **spawn `B_GO`** (kids stick, never detach — §6; parent `IS` mesh stays hidden). Fight **starts full** — this gate is from combat damage, not room spawn. Refinement Create HP **200** (vanilla 50), then scaler. |
| While golem live | Hits on **parent B_GO only** proxy-drain E_FM (shared bar). Fyrus **untargetable** (no battle attn / core Tg). No whip / blast / breath / walk-At. |
| Fyrus HP crosses **≤ 15%** | Cascade despawn B_GO + kids; end stun hold; **resume Fyrus still hollow** (`792` stays 0, keep `PUTOUT_WAIT` — do not re-light) |

```text
[ ablaze fight ]
    │ HP ≤ 50% (once)
    ▼
stun + PUTOUT → PUTOUT_WAIT + 792=0 + spawn B_GO (stuck)
    │ shared pool: proxy hits → E_FM  (50% … 15%)
    ▼
HP ≤ 15% → despawn B_GO → Fyrus continues **hollow** (no re-light)
```

### 8b. Why this fits the code

- **`792 = 0`** is exactly vanilla “safe body” (no contact burn, particles off) — §7a.
- **`TEXANM_PUTOUT`** is the missing extinguish transition vanilla skipped — §7b; use it on the 50% enter beat, then hold WAIT for the whole golem phase.
- **Stun BCK** alone does not clear fire — must pair with `792` + tex (§2b / §7e).
- **Shared pool** must ignore B_GO’s dead `health=1000` stubs — proxy only (§6b).
- **Fyrus attack mute** during window: do not run `fight_run` → `N_FIGHT` / `F_FIGHT` (§3a–b); clear `field_0x790` / `1829` so leftover blast/whip At cannot fire. Keese (`mDoCreateBa`) only arms on blast commit — muted if blast never starts (§3c).

### 8c. Cc / targeting lock (user 2026-08-18)

| Choice | Lock |
|--------|------|
| Lock-on | **Golem parent only.** Fyrus untargetable for the whole window |
| Hull | **Mimic Fyrus** — wire parent’s unused sph/cyl as joint body volumes (not 31 kid Co) |
| Kid Cc | **Off** while stuck (vanilla Co-off stays). No kid Tg |
| Damage | Parent hull Tg → `cc_at_check` on E_FM (real sword/arrow/FA/outfit/scaler). Ignore 1000 stubs. i-frames on B_GO `unk_0x690`, not Fyrus leftover invuln |
| Golem At | Wire unused joint-3 sph At on native `unk_0x660` frames 25–33 (Fyrus `at_sph` type/Atp/spl). B_GO **walks** (`h_wait`/`h_walk`/`h_attack`); Fyrus stays in stun hold |
| Hull init | One-shot `mStts.Init` when weight is not yet 0xFA (inverted `!=` check never armed Tg) |
| 50% gate | **Not** a room-start. Latch when remaining HP first drops to ≤50% from combat. Do **not** snap HP up to 50% (overshoot stays) |
| 15% exit | **Stay hollow** — `792` stays 0, no `TEXANM_FM` re-light |
| Stun BCK | **Random rotate** among `CHANCE` / `DAMAGE_L` / `DAMAGE_R` while held (re-roll when clip ends; prefer not the same twice) |

### 8d. Implement order

1. ~~`E_FM` HP% latch (50% enter / 15% exit)~~ **done** (`dAlbwBoss_fyrusUpdateGolemWindow`).
2. ~~Enter: stun + PUTOUT + mute + untarget~~ **done** (`ACTION_ALBW_GOLEM_HOLD`).
3. ~~Spawn `B_GO` stuck + hide parent `IS` mesh~~ **done** (`field_0x692=2` pinned; HIO display stays off).
4. ~~Parent Fyrus-style body Cc + proxy~~ **done** (unused sph/cyl, `cc_at_check` → E_FM). Hull Tg one-shot init + Golem `unk_0x690` i-frames.
5. ~~Exit at ≤15%: cascade despawn, keep hollow~~ **done**.
6. ~~B_GO walk + slam At stub~~ **wired** (kids stay stuck; Fyrus does not walk).
7. Playtest Mines (Golem walks/slams, hull swords/arrows move the real bar, Fyrus stays put, no fire return); then `Boss-Fights-RefinedFyrus.md`.

---

## 9. Current plan (2026-08-18, post-playtest)

Locked fight + 15% kid peel **wired**. Unused B_GO clips (`FALL`, `DEAD_01/02`, …) wait until this pass is playtested.

### 9a. Locked fight

| Beat | What |
|------|------|
| Pool | Refinement Create HP **200**, then Boss HP × / region. Vanilla stays 50. No mid-fight snap to 50. |
| Bar | Real `cc_at_check` (swords, arrows, FA, outfit, scaler). Core hits skip look-pass. Empty bar stays up during floor-down until last-hit `END`. |
| 50% | Fyrus stun hold + PUTOUT / hollow. Spawn `B_GO`. Kids **stuck** (`field_0x692 = 2` pinned). Parent `IS` mesh **hidden**. Parent walks / slams. Hull Tg → E_FM. Look-pos on Golem `eyePos`. |
| Camera | While window live: Fyrus `setLookPos` points at B_GO `eyePos`. After 15%: Fyrus owns look-pos again. |
| 15% | **Shed** — unmerge (`field_0x692 = 1`), hull off, parent idle/hidden. Kids fall and soldier. Fyrus stays hollow (`792=0`). Parent deletes when the last kid stones. |
| Kids done (phase 3) | **`field_0x770 = 1`** — core weak spot reopens (stay hollow). `FIGHT_RUN` resume when Link on arena BG. |
| 0 HP | Fyrus `ACTION_DOWN` with `mDownCnt = 3` (stays down until last hit). Floor core open. **Last hit** → vanilla `ACTION_END` / `DEMOEND` (not `FM_DEAD`). Not a B_GO defeat. |
| Deferred | Look-pass unused Fyrus clips (`DEAD`, `DOWNFR`) and unused parent B_GO clips (`FALL`, `DEAD_01/02`, …). |

### 9b. Kid-soldier layer (at 15% peel)

Native unmerge drops kids into `WAIT` / `WALK`. After peel settle (`mTimers[2]`), if Link is
within **500** XZ and **300** vertical of a kid, it enters `ACTION_FIGHT` — half the donor
OBJ_GRA search radius (1000). Chase uses `RUN_A`; punch uses **`grA` BCK 11** (OBJ_GRA
`setBaseAnm(3)` / `step` At windows on joints 11 and 17). Leaving **500** XZ (or vertical
≥ 300) **de-aggro**s back to `WAIT`. Punch still commits at **230** XZ / **15°** facing.
Not Dangoro.

Each kid **30** HP (own pool, then enemy scaler). `cc_at_check` + 6-frame i-frames — not
OBJ_GRA’s one-hit curl. At 0: `TO_STONE_NORMAL` then delete. Body Tg cyl mirrors OBJ_GRA
placement (foot + fwd 20, H 280, R 100) during kid phase — see §9c.

### 9c. OBJ_GRA soldier reactions — donor reference (kid tuning)

**Purpose:** Map native Regular Soldier (`daObj_GrA_c`, `mMode == 0`) reactions to
**Link actions**, and record what is ported onto `B_GOS`. Code paths: `hitChk()` (body Tg),
`step()` (punch At). Donor does **not** filter by weapon ID — only `ChkTgHit()` /
`ChkAtShieldHit()` / `ChkAtHit()`.

**Port lock (2026-08-19, wired same day):**

| Reaction | Port? | Notes |
|----------|-------|-------|
| Head-back (`grA` 12 / 13) | **Yes** | **`AT_TYPE_SHIELD_ATTACK`** on body Tg only (not block/parry); play speed **1/1.15** (~15% longer) |
| Totter (`grA` 14) | **Yes** | Punch `ChkAtHit` only (Link not guarding); **not** arrows |
| **`defence` (`grA` 10)** | **No** | Silent chip + i-frames only |
| `rollReturn` | **No** | Dungeon patrol soldier; not Fyrus kid context |

#### Target range (B_GOS vs OBJ_GRA)

| Gate | OBJ_GRA donor | B_GOS (wired) |
|------|---------------|---------------|
| Aggro / de-aggro | **1000** XZ (`getSrchCircleR`); walk leash **1100**; `\|Δy\| < 300` | **500** XZ aggro **and** de-aggro; `\|Δy\| < 300`; no home leash |
| Punch commit | **230** XZ; **15°** facing | Same (**230** XZ, **15°**) |
| Distance measure | XZ (`absXZ`) | XZ (`fopAcM_searchPlayerDistanceXZ`) |

#### Reaction table (OBJ_GRA vs B_GOS)

| OBJ_GRA reaction | Donor trigger | Link action / condition | B_GOS |
|------------------|---------------|-------------------------|-------|
| **`defence`** — guard recoil, BCK 10 | Body cyl `ChkTgHit()` in `hitChk()` | Any body-cyl weapon hit | **Not ported** — chip + i-frames |
| **Head-back** — `grA` 12 / 13 | Link shield bash (`AT_TYPE_SHIELD_ATTACK`) on body cyl | Guard-attack / bash into kid | **`ACTION_HEAD_BACK`** (block/parry on punch: no reaction) |
| **Totter** — `grA` 14 | Punch `ChkAtHit()` (not shield) | Link eats punch; not arrows | **`ACTION_TOTTER`** |
| **`rollReturn`** | Body hit during post-punch aggro timer | Patrol dungeon behavior | **Not ported** |
| **Stone / death** | HP 0 | Sustained body damage | **`ACTION_STONE_DIE`** at 0 HP |

```text
Kid aggro: Link within 500 XZ + |dy| < 300 → FIGHT
    │ Link leaves 500 XZ → de-aggro WAIT
    ▼
Kid punches (230 XZ, 15°)
  ├─ Link guarding     → no kid anim (Link-side feedback only)
  ├─ Link shield bash  → head-back (grA 12/13, 15% longer)
  └─ Link not guarding → punch connect → totter (grA 14)

Link hits kid body → cc_at_check chip (no defence anim)
```

**Golem phase 1 (cross-ref):** Player parry on B_GO slam — **`SetAtSpl(1)` + bit 12**
on all slam At volumes (`b_go_albwApplyParryableSlamAt`, wired 2026-08-19). Enemy-side
`b_go_albwShieldStaggerCheck()` still aborts slam on `ChkAtShieldHit` → `ANM_DAMAGE_01`.
Golem slams do **not** count toward Fyrus §10 counter.

### 9d. Playtest watch

| Gap | Why it matters |
|-----|----------------|
| 31 punchers | Crowd lock-on and TTK (31 × 50 at 1×). |
| Punch BCK on `B_gos` GRA_A | `grA` index 11 on the kid model — if the clip is missing, punch falls back to `RUN_A` with the same At windows. |
| Floor-down vs kids | Soldiers still punching during Fyrus DOWNWAIT can steal attention. |
| Camera | After 15% look-pos returns to Fyrus, not a kid. |

---

## 10. Design lock — fire-off counter vulnerability (2026-08-19, user spec)

**Status:** Implemented (2026-08-19). Replaces naive “770 open whenever 792=0” for
ablaze phase; golem window unchanged.

### 10a. Terms (avoid confusion with golem `s_fyrusGolemPhase`)

| Name | When | `792` / `770` intent |
|------|------|----------------------|
| **Ablaze phase** | Pre-50% normal Fyrus fight | Fire **on** until counter fills; vuln **closed** until fire-off |
| **Golem window** | 50%→15% + kid crowd | Frozen — no counter, no vuln (existing §8) |
| **Hollow phase** | Post–last kid (golem phase 3) | Fire **off** by design; vuln rules differ (§10e) |

### 10b. Master rule (ablaze phase)

**Vulnerability (`770=1`) is tied to fire-off (`792=0`).** Core is not a sustained
pinata while ablaze.

**Attack counter** (target **14**):

- Each Fyrus **commit** of a damage-class attack (§10c) adds credits **once** —
  player outcome does **not** stack an extra credit on top for dodge / eat hit /
  normal parry.
- **+1** credit per commit: Link dodges, takes the hit, or **normal** shield parry.
- **+2** credits per commit on **perfect** shield parry: **+1** for the attack
  commit **+1** bonus for the perfect parry (user: confirm “1 bonus” at
  implementation touch-base — do not code until agreed).
- **L/R chain whips** = **two separate commits** when both arms fire (each arm
  resolves independently).
- At **14**: `792=0`, `770=1` (fire off, vuln open).

**Example (one whip commit):** player runs → **+1 total**. Perfect parry that
same whip → **+2 total** (not +1 attack + another +1 dodge).

**Shield / burn (while `792=1`):**

| Shield | Normal parry | Perfect parry |
|--------|--------------|---------------|
| Ordon / Wooden | Durability loss; **burn** | No burn |
| Hylian | No burn | No burn |

All listed Fyrus attacks parryable/shieldable (wooden needs perfect to avoid burn).

**Pre–fire-off chip damage (before 14 fill):**

Any player damage to Fyrus body / non-vuln (swords, arrows, FA, etc.) deals **1%**
of normal output and **resets the attack counter to 0**. **Does not** open vuln
(`770` stays 0), **does not** trigger stun-close cycle.

### 10c. Damage attacks that count toward 14

**Strictly Fyrus-owned commits only:**

| Attack | Count |
|--------|-------|
| Chain whip L | +1 or +2 (perfect) per arm commit |
| Chain whip R | +1 or +2 (perfect) per arm commit |
| Blast AOE | +1 or +2 per blast commit |
| Fire breath | +1 or +2 per breath commit |

**Does NOT count:** body burn contact, fire keese, B_GO slam, any non–E_FM actor.

Commit = At window / anim commit fires even if Link is out of range (§10g #1).

### 10d. Ablaze phase — vuln window + stun-close (anti-abuse)

**Loop:**

1. Counter → **14** → fire off, vuln open (`792=0`, `770=1`).
2. **Any player-inflicted damage** to the open vuln (arrow, sword, FA, …) →
   **cut stun** (reimplemented stuns only — **no** vanilla run-away / chain-trip).
3. Stun resolves → **`770=0`**, **`792=1`**, counter **reset to 0**.
4. Back to ablaze; earn 14 again. **No** vanilla `STOP`→trip in ablaze phase.

**Abuse fix:** vuln cannot stay open for multi-arrow stun spam — first qualifying
vuln hit ends the window and relights fire.

### 10e. Hollow phase (post-kids)

**No 14-counter** for now (revisit only if high-scaling playtest demands it).
Golem window unchanged (§8).

| Beat | Behavior |
|------|----------|
| Default | Fire off (`792=0`), vuln open (`770=1`) after last kid |
| One arrow to vuln | **Vanilla** run-away → Link chain trip → `ACTION_DOWN` |
| Arrow does **not** temporarily close vuln — goes straight into vanilla down setup |
| Down core hits | Last vuln hit while down → vanilla `ACTION_END` / fight close |

### 10g. Clarifications — resolved (2026-08-19)

| # | Question | Answer |
|---|----------|--------|
| 1 | Commit out of range? | **Yes** — commit counts |
| 2 | Perfect parry math | **+1 attack + +1 bonus = +2** — confirmed at implementation |
| 3 | Counter after stun-close | **Reset to 0** |
| 4 | What triggers stun-close? | **Any** player damage on open vuln |
| 5 | What counts toward 14? | Whip L/R, blast, breath only |
| 6 | Hollow arrow | No counter; no vuln close; vanilla down path |
| 7 | Pre-14 body damage | **1%** chip + **counter reset**; no vuln open; no stun |
| 8 | Phase names | **Ablaze** = pre-50%; **Hollow** = post-kids |

**Hold:** ~~No implementation until perfect-parry bonus (+1 vs double-count) is
explicitly signed off.~~ Signed off 2026-08-19; coded in `d_albw_boss.cpp` /
`d_a_e_fm.cpp` §10 hooks.

---

## 8e. Golem-phase chain whip — **OPTIONAL / PAUSED** (2026-08-19)

Golem phase 1 offense today: **slam only** (`ANM_ATTACK`, live). No whip read on
the Titan skeleton from vanilla `B_go` orphans (`START_L/R` = wall chain-pull, not
outward whip).

| Path | Status | Notes |
|------|--------|-------|
| **Path B — new Titan BCK** | **Paused** | Blender + SuperBMD new whip on unchanged `B_go` skeleton → repack `B_go.arc` → `h_sweep` + frame-gated At in `d_a_b_go.cpp`. Outside tools required for art. |
| **Option D — E_FM overlay** | **Not chosen** | B_GO triggers; E_FM runs native `ATTACK02` at clump transform with draw swap + hold-pin exception. No new assets; code-heavy. |
| **Path 2 — fake whip on B_GO orphans** | **Rejected** | Wrong verb; no Fyrus whip read. |

**Resume when:** user explicitly unpause Path B (asset pass first) or picks Option D
(code pass first). Until then, golem window playtest scope is slam + hull damage only.

---

## 9e. Playtest checklist — remaining (2026-08-19)

**Combo:** Boss Refinement **On**, Boss Health Bars **On**, Mines (`D_MN04B` arena).
Wipe Dawn cache after build.

| # | Beat | What to verify |
|---|------|----------------|
| 1 | **Pre-50% ablaze** | §10 counter fills on whip L/R, blast, breath (+2 on perfect parry); 1% chip + counter reset on pre-14 body hits; fire-off at 14 → vuln open |
| 2 | **Vuln window** | First vuln hit → cut stun → fire relights, counter reset; no multi-arrow vuln spam |
| 3 | **50% gate** | Stun + PUTOUT + B_GO spawn; Fyrus frozen; look-pos on golem `eyePos`; hull swords/arrows move **real** bar |
| 4 | **Golem slam** | Walk→attack at 300; parry works (`SetAtSpl(1)` + bit 12); shield hit → `ANM_DAMAGE_01`; trimmed `h_wait` cadence feels right |
| 5 | **15% peel** | Kids unmerge; parent idle/hidden; Fyrus stays hollow; camera returns to Fyrus |
| 6 | **Kid soldiers** | 500 XZ aggro/de-aggro; punch at 230/15°; **totter only on punch connect** (not shield block); **head-back on shield bash only**; 50 HP → stone |
| 7 | **Last kid cleared** | Fyrus resumes `FIGHT_RUN` on arena BG (hollow); core vuln open |
| 8 | **0 HP** | Floor-down (`mDownCnt=3`); bar stays until last-hit `END` |
| 9 | **Hollow arrow** | One vuln arrow → vanilla run-away / chain trip / down (no §10 counter) |
| 10 | **Breath timing** | B morf + uniform speed; no 3.33× lead-in (C reverted) |
| 11 | **Crowd / camera** | 31× lock-on feel; floor-down vs punching kids; post-15% camera not stuck on kid |

**Deferred (not in this pass):** golem whip (§8e); unused E_FM / B_GO orphan look-pass;
`DEAD` / `DOWNFR` / `FALL` / `DEAD_01/02` clips.

---

*End Fyrus research (… §10 vulnerability counter).*
