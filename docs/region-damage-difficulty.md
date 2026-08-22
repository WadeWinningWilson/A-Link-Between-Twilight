# Region / damage difficulty — planning detail

Live tip: [state/region-damage-difficulty.md](state/region-damage-difficulty.md).  
Related: [Outfit Stats.md](Outfit%20Stats.md) · [shield-combat.md](shield-combat.md) · `src/d/d_albw_region_mult.cpp` · `damageMagnification()` in `d_a_alink_damage.inc`.

---

## 1. Region table (shipped)

Province step **+0.25** after Faron; dungeon = province **+0.15** (Forest Temple exception **1.05**). Castle Town / castle exteriors = Lanayru **1.50**. Late game starts at Gerudo Desert **1.75**.

Lookup order: **`(stage, room)` → stage-name → SaveTbl**. Twilight = same room day/night (no layer axis). Enemy **HP** stamps at spawn; crossing rooms does not resize already-loaded enemies.

**Settings split (2026-08-21):**

| Setting | Key | Effect |
|---------|-----|--------|
| **Region Damage** | `game.regionDamage` | Incoming damage to Link (standalone On/Off, default Off) |
| **Region Multipliers** | `game.regionMult` + Health / Rupees | Enemy HP + enemy-death rupees (master required) |

Same table for both. Old key `game.regionMultDamage` is unused.

### Room / stage overrides (code as of 2026-08)

**`F_SP121` Hyrule Field**

| Rooms | Mult |
|-------|------|
| 1, 6, 15 | 1.05 (Faron pockets) |
| 0, 2–5, 7 | 1.25 (Eldin) |
| 9–14 | 1.50 (Lanayru) |
| 8, 16+, other | SaveTbl FIELD fallback **1.05** |

**`F_SP117` Lost Woods / Grove**

| Room | Mult |
|------|------|
| r3 Lost Woods | **2.25** (was 1.00; Phase A1) |
| r1 Sacred Grove / r2 ToT Past | **2.25** |

**`F_SP122`:** always **1.50**.

Stage-name examples: `D_SB00` 2.00, `D_SB01` Ordeals 2.65, Eldin caverns 1.25, Lake cavern 1.50, Faron cave 1.00, `F_SP102` 1.05, desert/Bulblin hosts 1.75.

Open tweak candidates (not decided): Field **r2, r7, r14**; Hidden Village `F_SP128`; Prison; grottos `D_SB05–09`; WW hosts inheriting host SaveTbl.

---

## 2. Incoming damage stack (current)

```
finalDmg ≈ Atp × damageMultiplier × outfitReceived × regionDamage
```

Then `setDamagePoint` truncates and **+1 if any leftover tenth**. Wolf adds 2× on the magnification path (outfit received does **not** apply in wolf). Life units: **4 pieces = 1 heart**.

**Bug (fixed — source gate):** `regionDamage` used to apply inside `damageMagnification()` with no source tag. Now `getDamageMult()` is **1.0 unless a COVER site holds `dAlbwRegionMult_DamageScaleScope`** (fail-closed to vanilla).

### Region Damage source coverage (locked 2026-08-21 — **gate shipped**)

| Verdict | Sources |
|---------|---------|
| **COVER** | Enemy/boss CC hits; enemy/flower bombs; trap actors; **hazard polys** (lava/ice/fire/electric floors & walls); boss scripted chips (Morpheel hold); Stallord `setPlayerDamage`; PM fail chip (post-scale) |
| **EXCLUDE** | Fall/land height (`setLandDamagePoint`); player bombs; **sand wall**; **freeze DoT**; **throw/spit HP**; scene-carry `getLastSceneDamage`; goat/goron shove; Kago mishap |

**Throw HP:** Applied once at reaction start in vanilla. Locked **EXCLUDE** from RD — retail piece count after non-region magnifiers only.

Reference Atps for planning tables:

| Enemy | Atp | Notes |
|-------|-----|--------|
| Bokoblin (`E_OC` named) | **1** | Other OC skins **2** |
| Darknut (`B_TN` armored) | **4** | Armor-off swings **2** |

---

## 3. Planned — damage scaler

**Product:** A difficulty control in the **same organization** as Region Multipliers (settings tree, independent axes / modes), able to:

- **Stack** with the current Link incoming multiplier,
- **Build on** it, or
- **Wholly replace** it,

depending on the mode chosen at implement time.

**Hook:** Same family as region damage — `damageMagnification()` (and UI under Difficulty / ALBW as appropriate). Do not invent a second parallel damage pipeline.

**Not now:** Raising **particular enemies’** Atp / output. That may be a later evolution; it is **explicitly out of scope** for this plan.

---

## 3b. Region Damage also boosts rupees (shipped — Phase A2)

When **`game.regionDamage` is On**, enemy-death / fight-victory rupee grants gain a flat **×3.00** (triple), independent of the Region Multipliers master.

**Stack with Region Multipliers → Rupees:**

```
finalGrant ≈ baseGrant
            × (regionDamageOn ? 3.00 : 1.00)
            × (regionMult && regionMultRupees ? tableMult : 1.00)
```

(Same half-up rounding as today’s grant path. Shops unchanged.)

### Example (understanding check)

Assume a field kill pays **20** rupees base (Enemy Death Rupees on).

| Settings | Mults | Payout |
|----------|-------|--------|
| Region Damage **Off**, Region Mult **Off** | — | **20** |
| Region Damage **On**, Region Mult **Off** | ×3 | **60** |
| Region Damage **On**, Region Mult **On** (Rupees), Ordon **1.00** | ×3 × 1.00 | **60** |
| Region Damage **On**, Region Mult **On** (Rupees), Lanayru **1.50** | ×3 × 1.50 | **90** |
| Region Damage **On**, Region Mult **On** (Rupees), Hyrule Castle **3.15** | ×3 × 3.15 | **189** |

**Implemented:** `dAlbwRegionMult_scaleRupees` multiplies `getRupeeMult()` × `getRegionDamageRupeeMult()` (3 or 1). Grant site: `d_albw_enemy_rupee.cpp` `grantRupees`. Shops unchanged.

---

## 3c. Planned — other player-help systems (research + product lock)

### Shop health upgrades — uncap past soft ceiling

| Fact | Detail |
|------|--------|
| Where | Postman Upgrades → `VISIBLE_MQ_HEART` / `dAlbwMQ_*` |
| Cap today | **17** half-heart tiers; gauge stop at **20♥**; prices end at **9999** |
| **Product (locked)** | After soft-cap price **9999**, continue buys forever; grant **¼♥** each; price = **9999 + 333×n** (n = buys past soft-cap) |
| **Hearts HUD (locked)** | Classic: hearts past 20 **overlay / stack on the 20th** slot. LoP Health Bar: **leave fixed trough + % fill** (may retune width later — not required for C) |
| **Wallet (locked, C1)** | Colossal **9999 → 50000**. `mRupee` stays `u16` (no save layout change). **Must** ship 5-digit rupee HUD + widen `mRupeeNum` / `moveRupee` / `drawRupee` off `s16` with the cap |
| **Wallet (future)** | **99999** deferred — needs storage &gt; `u16` and a migration plan first (see below). Do not naive-widen `mRupee` in place |

#### Phase C1 — Rupee HUD: **insert path locked** (implement plan)

**Product look**

```
[💎 jewel]  0 9 9 9 9     @ 9999   (leading place on when ≥10000, or always-pad — see step C1-HUD)
[💎 jewel]  1 0 0 0 0     @ 10000
[💎 jewel]  5 0 0 0 0     @ 50000
```

Insert = **one new digit pane beside** the vanilla four (same art/size). **Not** overlay. **Not** replace. Economy uses real `mRupee` (u16); HUD digits are display only.

**Rejected**

| Path | Why not |
|------|---------|
| Overlay on thousands | Messy double-glyph |
| Free-draw primary counter | Doesn’t inherit LoP / cluster transforms |
| BLO `r_n_5` edit | Heavier; not needed for C1 |
| Cap-only to 50k | Breaks HUD + s16 tick/clamp |

---

##### C1 work order (single ship)

| Step | Work | Files / hooks | Done when |
|------|------|---------------|-----------|
| **C1-1** | Widen tick/apply off `s16`: `mRupeeNum` → **`s32`**; `moveRupee` max/sum/clamp use **`s32`**; `drawRupee(s32)` | `d_meter2.h/.cpp`, `d_meter2_draw.h/.cpp` | 50k can tick and clamp without wrap |
| **C1-2** | **Insert** 10000s digit: clone (+ shadow) from `r_n_4` / `_s`; `appendChild` on **same parent as `r_n_*`**; order **jewel → clone → vanilla row** | `initRupeeKey`, `drawRupee` | Pane follows LoP / `r_k_n` moves |
| **C1-3** | 5-place math (balloon `setScoreNum` style); textures 0–9 only; clone = `/10000`, vanilla = remainder as today | `drawRupee` | `10000`/`50000` read correctly |
| **C1-4** | **Clearance:** if jewel→clone gap tight, shift **entire** vanilla digit row **+1 advance** while 5th shown; restore under 10000 (default: **shift-on-show**, not permanent empty slot) | `drawRupee` | No overlap on gem or thousands |
| **C1-5** | Same count **scale / alpha** as vanilla digits; `getRupeeDigitMetrics` leftmost includes clone when shown (`+n` popup) | `drawRupee`, metrics, popup | Popup clear of leading digit |
| **C1-6** | `COLOSSAL_WALLET_MAX = 50000`; editor label if needed | `d_save.h`, editor strings | Cap holds 50k; **old saves OK** (`mRupee` stays u16) |

**Leading zeros (product default for implement):**  
Under 10000 keep **today’s** show/hide (thousands hidden &lt;1000; no permanent `00000` pad) unless playtest asks for always-five. At ≥10000 always five places (`1xxxx`…`5xxxx`). Optional later: always `00000`–style pad.

**Playtest gates**

1. Classic + LoP: `0` → `9999` looks unchanged; `10000` / `50000` readable, gem not covered.  
2. Earn/spend/shop at 10k+ uses real balance (not digit wrap).  
3. `+n` popup still left of leftmost digit.  
4. No FPS/factory work — normal `build_run.bat` only.

**Follow-ups (not C1):** 99999 / `mRupee` widen; permanent reserved gap; frame (`moyou_*`) widen if playtest wants it.

---

##### Background (why insert) — short

| Piece today | Fact |
|-------------|------|
| BLO digits | 4 only (`r_n_1`…`4` + shadows) |
| Cap | Colossal **9999**; `mRupee` **u16** (50k fits) |
| Tick | `mRupeeNum` / `moveRupee` **`s16`** — must widen with cap |
| LoP | Moves wallet via `r_k_n`; clone must share digit parent to follow |

Donor patterns: balloon 5-place math; item-pane `appendChild` clone; **not** popup free-draw as primary.

#### What a +2-byte `mRupee` widen would mean (future 99999)
#### What a +2-byte `mRupee` widen would mean (future 99999)

`mRupee` sits mid-struct in `dSv_player_status_a_c` (after life fields, before oil / items / wallet size). Growing it `u16`→`u32` **shifts every later field by 2 bytes** in the quest-log blob.

| If you widen with no migration | Likely playability hit |
|--------------------------------|------------------------|
| Load old save on new build | Wrong bytes read as oil, item slots, wallet size, magic, etc. — inventory / meter / form glitches, possible soft-lock |
| Load new save on old build | Same class of mis-parse |
| “Start a new save” only | New files OK; **all existing quest logs incompatible** unless converter runs |

**Does not force a new save for 50k** — constant-only cap change. **99999** needs either: (1) careful load migration that rewrites blobs, (2) overflow side-store (unused event / trailing padding) so offsets stay, or (3) accept 65535 ceiling. Research that path before coding 99999.


### Sword-specific damage upgrades (locked shape)

**Effect:** +**1 Atp** per purchase to the **shop-selected** sword. **NG+ Tier 4:** Tier-1-like prices but **+2 Atp** per buy.

**UI:** picker on a **second Upgrades page**. Rows: **Wood, Ordon, Master, Light** — only after obtained (story or Dusklight editor).

| Tier | Gate | Steps |
|------|------|-------|
| **1** | Rupees only | 100, 200, 300, 400, 500 |
| **2** | Bug count check | 4+100₽ … 12+500₽ |
| **3** | Poe soul count check | 5+500₽ … 35+1000₽ |
| **4** (NG+) | Like Tier 1 prices | **+2 Atp** |

**Checks do not consume** bugs/poes.

**Vanilla records (Q3–4):**

| | What vanilla stores | “Currently held”? | Recommended shop check |
|--|---------------------|-------------------|------------------------|
| **Bugs** | Per-species **`isItemFirstBit`** (permanent once that insect was obtained — Agitha collection). Bottles can also hold insects temporarily | Bottle contents ≠ collection | Count species with first-bit (**0–24**). Does not remove Agitha progress or bottle bugs |
| **Poes** | **`getPohSpiritNum()`** — increments on collect; **no normal decrement**. Jovani uses milestones (20/60) on this total | No depletable “souls in pocket” | `getPohSpiritNum() >= N`. Shop payment does not reduce the number |

“Currently held” fits bottles for bugs (awkward vs needing 12) and **doesn’t exist for Poes**. **Locked:** Tier 2/3 = non-consuming progress checks (insect first-bit count + `getPohSpiritNum()`).

### Capacity → ALBW meter discount (shipped — Phase B2)

| Capacity | Matching ALBW drain |
|----------|---------------------|
| Big quiver (60) | **75%** of normal arrow / bomb-arrow arrow-share |
| Giant quiver (100) | **50%** |
| Giant bomb bag (max ×2, e.g. 60 normals) | **50%** of bomb / bomb-arrow bomb-share |
| Base quiver / bag | **100%** |

TP has no intermediate “Big” bomb bag — only base vs Giant. Slingshot unchanged.

### Soulbound → shield durability (shipped — Phase B1)

**Per drink:** **20%** of equipped shield max (`repairDurabilityFraction(1, 5)`). Durability setting On + shield equipped. Hook: `dAlbwPotion_consumeSoulboundDrink`.

| Shield | Max HP | Drain / block | Drain / small | **20%** repair |
|--------|-------:|--------------:|--------------:|---------------:|
| Ordon | **134** | 18 | 10 | **26** |
| Wooden | **120** | 15 | 8 | **24** |
| Hylian | **160** | 22 | 12 | **32** |

---

## 3d. Question status

| # | Answer |
|---|--------|
| 1–6 | Locked |
| 7 | **Locked** — Big **75%** / Giant **50%** of drain |
| 8 | **Locked** — soulbound **20%** of equipped max |

---

## 4. Parry Master (shipped core — Phase E; HUD deferred)

**Setting:** `game.parryMaster` (Gameplay; speedrun forces Off). Requires Shield Parry.

Inspiration: Lies of P guard / perfect-guard recoverability + ALBW meter as stamina.

### 4.1 Chip HP

| Block quality | HP |
|---------------|-----|
| Perfect parry | **0** |
| Normal / failed block | **15% of post-scale** incoming damage (vanilla Atp if scales off) |

**Order (locked):** multipliers first → then ×0.15 → tenths rule. Perfect = 0 chip.

### 4.2 ALBW meter stamina penalty

On **normal / failed shield block** only:

- Always **5% of `sOilBaseMax` (10900)** per effective ATP — even if live ceiling is 30900. Atp4 → **2180** unless ATP was multiplier-scaled.
- Fractional effective ATP: `effAtp × 545`, then **round**.
- **Stacks** with existing Shield Parry fail penalties when PM On.
- **No shield → no PM fail meter drain.**

### 4.3 Chip recoverability queue

**Queue:** pending recoverable chip inputs (HP already lost; queue = what can still return).

**FIFO reclaim:** oldest input first; **one reclaim = that one input’s HP only** (never the whole queue).

**Time model (locked — LoP hybrid; replaces old snap-6s wipe):**

| Phase | Duration | Behavior |
|-------|----------|----------|
| **Grace** | **3 s** | Recoverable pool **does not melt**. New chip **refreshes grace** (and extends lifetime). |
| **Melt** | **6 s** | After grace, remaining recoverable pieces **linearly drain** to 0. |
| **Total** | **9 s** | From last chip: 3s hold + 6s melt (unless refreshed / wiped / reclaimed). |

Death / 0 HP / open-hit wipe still clear all. No other cap.

**Regain:**
- Damaging hit (any arsenal, wolf/human, multi-hit per tick) → **1** input.
- Perfect parry → **up to 2** inputs (one perfect with one chip left still heals that one; two perfects can clear four).

**Other HP interactions:**
- Potion/heal: fills HP and **consumes** matching recoverability (FIFO).
- **Enemy open hit** (non-block): **wipe entire queue +** that hit’s HP loss.
- PM chip: **adds** to queue; **refreshes grace**.

**HUD:** see **§4.5 E-HUD** (**shipped**).

### 4.4 Gates / classification

- Perfect vs failed = **existing Shield Parry** system.
- Fail chip + meter = **shield block path only**.
- **Guard-break shatter:** **exempt from PM fail chip + meter + queue push for now** (vanilla break is still a shield-hit AtSpl 9/10/11 event; PM simply does not add chip/drain on shatter).
- Speedrun: PM **Off**.

**Future desire (not implemented; needs scrutiny):** user wants **all** guard-breaks to apply only when a **perfect parry was not achieved**. Guard-break is largely **enemy-/AtSpl-specific** (ALBW already defers some breaks) — separate research pass before changing break rules.

**Magic Armor (PM):** Hearts at risk → like **no armor**. Enemy hit with **0 HP** absorb → queue **stays**. ALBW unfunded → like no armor. **Outfit swap does not wipe** reclaim queue. Detail §9.

### 4.5 E-HUD — faded reclaim (**shipped**)

**Goal:** Show reclaimable HP like LoP Guard Regain — **dull red** segment / heart quarters — on **both** LoP Health Bar and classic hearts. Replicate the **visual difference** (same hue, ~half brightness, hard cut), not LoP’s absolute RGB.

**Inspiration screenshot:** LoP bar with live crimson vs darker dull-red regain (repo asset / chat 2026-08-21). Measured contrast: dull ≈ **0.45–0.50×** live red luminance; sharp vertical join; empty track near-black.

#### Timing (sim + HUD share one clock)

Same as §4.3: **3s grace** (no melt; chips refresh) → **6s linear melt** → gone. Total **9s** from last chip unless refreshed/wiped/reclaimed. Implemented in `d_albw_parry_master`; HUD reads `dParryMaster_getRecoverablePieces()`.

#### Color (relative to Dusklight live)

| Surface | Live | Dull reclaim |
|---------|------|--------------|
| **LoP Health Bar** | Kantera tint black **(90,18,18)** / white **(215,40,40)** | Hybrid: black **(77,15,15)** / white **(183,34,34)** (~0.85×) + reclaim pass **0.95×** α |
| **Classic hearts** | Texture fill; HIO α filled **0.7**, empty base **0.4** | Full ghost hearts at **0.5×** mark α via `applyParryMasterHeartReclaim()` |

No grey. Hard edge live↔dull. Dull clearly above empty trough.

#### Draw paths

1. **`drawLopHealthBar()`** — pass 1 dull to `life + reclaim`; pass 2 live crimson on top.
2. **`applyParryMasterHeartReclaim()`** (each frame before heart rasterize) — empty hearts in reclaim range as dull ghosts (melt shrinks them).

---

## 8. Locked + remaining

### Locked this pass

| Topic | Answer |
|-------|--------|
| Guard-break vs PM fail | **Exempt for now** |
| Future guard-break rule | Break only if **not** perfect — **research later** |
| MA rule of thumb | Hearts at risk → like **no armor**; 0-HP absorb → no fail chip/meter |
| Cosmetic / DD / ALBW unfunded / Normal heavy | Like **no armor** for PM shield fail |
| Real HP loss | Wipe queue + HP |
| 0-HP enemy absorb wipe | **No** — queue stays |
| Outfit swap wipe | **Dropped** — swap does not clear queue |
| ALBW unfunded | Like no armor (confirmed) |
| Meter % / round / perfect×2 / enemy open HP wipe / shield-only fail / FIFO | Prior locks |

### Still open before implement

1. Guard-break × perfect research (Phase G).
2. Horse + shield (assumed same as foot).
3. Open room mult candidates (not decided).

**Soulbound 20% / capacity 75·50 / progress checks / ×3 rupees / heart uncap / wallet 50k / sword shop shape:** locked — see §7 phases.

---

## 9. Magic Armor mode review (code fact → PM proposal)

Setting: `game.armorRupeeDrain` / UI **Magic Armor Behavior**. Default **ALBW Armor**. Requires `checkMagicArmorWearAbility()` (Magic/Deity clothes equipped) unless noted.

**`checkMagicArmorNoDamage()`** (`d_a_alink_damage.inc`) decides absorb vs hearts. When true, shield-hit code often skips the `!armor_no_dmg` fail/parry branch (break AtSpl still can run earlier).

| Mode | UI | When absorb (`NoDamage`)? | Absorb cost | When hearts take damage |
|------|-----|---------------------------|-------------|-------------------------|
| **NORMAL** | Normal | Wearing MA and **not** “heavy” (`!checkMagicArmorHeavy()` — typically still has rupees / not depleted-heavy) | `rupeeCount −= dmg×10` (vanilla-style) | Heavy / broke → full HP through `setDamagePoint` |
| **ON_DAMAGE** | On Damage | `rupees != 0` | `rupeeCount −= dmg×10` | Wallet empty → full HP |
| **DOUBLE_DEFENSE** | Double Defense | **Never** (`NoDamage` always false) | — | Always HP; **÷2 after** magnification if wearing MA |
| **INVINCIBLE** | Invincible | **Always** while wearing | None | Never HP from `setDamagePoint` absorb path |
| **COSMETIC** | Cosmetic | **Never** | — | Always full HP (looks like armor only) |
| **ALBW** | ALBW Armor | Funded block: `canALBWArmorBlock()` && rupees≥500, **or** Deity + rupees>5000 | −500 + meter depower, or Deity −2500; body path also forces depower attempt | Unfunded (no meter/rupees for block) → HP; body hit still calls `onALBWArmorHit` when wearing |

ALBW body-hit special (`damage.inc` ~801): even before `setDamagePoint`, funded ALBW/Deity can zero `dmg`; unfunded still depowers meter then HP applies.

### Parry Master × MA mapping (user 2026-08-21)

**Rule of thumb:** If that hit **can / does remove hearts**, PM treats it like **no armor** (fail chip+meter on shield fail; wipe queue on open HP loss). If armor **fully absorbs** (0 HP), no PM fail chip/meter and **no** queue wipe from that hit.

| Mode | Shield fail → PM chip+meter | Enemy hit **0 HP** (absorb) → wipe queue? | Enemy hit **HP lost** → wipe+HP |
|------|----------------------------|---------------------------------------------|--------------------------------|
| NORMAL absorbing | **No** | **No** | When heavy: **Yes** |
| NORMAL heavy | **Yes** (like no armor) | n/a | **Yes** |
| ON_DAMAGE (₽>0) | **No** | **No** | Empty wallet: **Yes** |
| DOUBLE_DEFENSE | **Yes** (like no armor) | n/a | **Yes** |
| INVINCIBLE | **No** | **No** | n/a |
| COSMETIC | **Yes** (like no armor) | n/a | **Yes** |
| ALBW funded | **No** | **No** | n/a |
| ALBW unfunded | **Yes** (like no armor — hearts at risk) | n/a | **Yes** |

**Outfit swap → wipe?** **Dropped** — changing outfits does **not** clear the reclaim queue.

### Clarifications

**0 HP absorb:** **Locked — queue stays.**

**Open hits:** **Enemy → Link** for wipe. Link’s attacks reclaim.

**ALBW unfunded:** **Confirmed** — like no armor for PM fail chip when hearts at risk.

**Queue wipe sources (final):** enemy hit that **loses hearts**; shared **6 s** timer expiry; **death / 0 HP**. Not: 0-HP absorb, potions (potions shrink queue by heal amount), outfit swap.

---

## 5. Planned — global `damageMultiplier` → New Game Plus

**Intent:** Repurpose Settings → **Damage Multiplier** as NG+ run scaler: 1st run **1×**, each NG+ **+1×**. Persistence TBD at implement. Speedrun forces safe defaults.

---

## 6. Planning numbers (region only)

Assumptions: outfit off, `damageMultiplier = 1`, Atp1 boko / Atp4 armored Darknut. Hit = full contact. PM chip = 15% of (Atp × R), then tenths rule.

| Area | R | Boko hit | Boko PM | Darknut hit | Darknut PM |
|------|--:|---------:|--------:|------------:|-----------:|
| Ordon / Faron | 1.00 | 1 | 1 | 4 | 1 |
| Field Faron / FT / grottos | 1.05 | 2 | 1 | 5 | 1 |
| Eldin | 1.25 | 2 | 1 | 5 | 1 |
| Goron Mines | 1.40 | 2 | 1 | 6 | 1 |
| Lanayru / Town / Prison | 1.50 | 2 | 1 | 6 | 1 |
| Lakebed | 1.65 | 2 | 1 | 7 | 1 |
| Desert / Camp | 1.75 | 2 | 1 | 7 | 2 |
| Arbiter's | 1.90 | 2 | 1 | 8 | 2 |
| Snowpeak | 2.00 | 2 | 1 | 8 | 2 |
| Snowpeak Ruins | 2.15 | 3 | 1 | 9 | 2 |
| Grove / ToT Past | 2.25 | 3 | 1 | 9 | 2 |
| Temple of Time | 2.40 | 3 | 1 | 10 | 2 |
| City / Ordeals | 2.65 | 3 | 1 | 11 | 2 |
| Palace | 2.90 | 3 | 1 | 12 | 2 |
| Hyrule Castle | 3.15 | 4 | 1 | 13 | 2 |
| Lost Woods (planned 2.25) | 2.25 | same as Grove | | | |

**Wild stack example** (Castle 3.15 × Ordon 2.5 × global 5×): Boko full **40** / PM **6**; Darknut full **158** / PM **24**.

---

## 7. Phased integration plan (non-integrated only)

All product locks above are **planned / not coded** unless marked shipped. Phases are ordered by dependency and blast radius. Do **not** start a later phase until the prior phase’s playtest gate passes (or user skips). Build via `build_run.bat`; no CMake reconfigure for these.

**Already shipped:** region table + Region Damage setting + Region Mult HP/Rupees axes + damage stack + Shield Parry + MA modes + MQ heart shop **capped** + **Phase A** + **Phase B** (soulbound 20%; capacity drains) + **Phase E core** (Parry Master; no LoP HUD yet).

**Parallel (user 2026-08-21):** Phase **E ran with B**.

---

### Phase A — Table + economy one-liners — **DONE**

| # | Work | Status |
|---|------|--------|
| A1 | Lost Woods `F_SP117` r3 **2.25** | **Shipped** |
| A2 | Region Damage On → rupees **×3** | **Shipped** |

---

### Phase B — Potion / capacity combat economy — **DONE**

| # | Work | Status |
|---|------|--------|
| B1 | Soulbound → durability **+20%** | **Shipped** (`dAlbwPotion_consumeSoulboundDrink`) |
| B2 | Quiver/bomb ALBW drain **75%/50%** | **Shipped** (`d_meter2.cpp`; bomb Giant only → 50%) |

---

### Phase C — Wallet + heart uncap

**Locks:** Colossal **50k**; HUD = **insert** 10000s digit (jewel → clone → vanilla row); hearts past 20 = **overlay on 20th**; Health Bar leave; shop **+333** / **¼♥**. No 99999 yet.

| # | Work | Hooks / files | Gate |
|---|------|---------------|------|
| **C1** | Insert-path 5-digit HUD + Colossal **50000** + s32 tick | `d_meter2*` / `d_save.h` | **Shipped** — playtest 10k/50k on classic+LoP |
| **C2** | Heart shop past soft-cap: **+333**, **¼♥**; 20th overlay | `d_albw_master_quest.*`; `drawLife` | Unlimited buys; HUD readable past 20♥ |

**Why before sword shop:** Late hearts and Atp need wallet headroom and a HUD that can show five digits.

---

### Phase D — Sword Atp shop

| # | Work | Hooks / files | Gate |
|---|------|---------------|------|
| D1 | 2nd Upgrades page picker: Wood / Ordon / Master / Light (obtained only) | Postman / MQ shop UI | Rows appear only when owned |
| D2 | Persist per-sword Atp bonus; apply on hit | Save field + attack Atp path | +1 Atp per buy; survives reload |
| D3 | Tiers 1–3 prices + **non-consuming** bug/poe gates | Insect first-bit count; `getPohSpiritNum()` | Failed gate = no buy; counts unchanged |
| D4 | NG+ Tier 4: Tier-1 prices, **+2 Atp** | NG+ flag | Only when NG+; +2 per buy |

**Why after C:** Expensive tiers need Colossal 50k. Largest UI + save surface in the player-help package.

---

### Phase E — Parry Master

| # | Work | Status |
|---|------|--------|
| E1–E5 | Setting, chip, meter, queue, GB exempt | **Core shipped** |
| **E-HUD** | Dull-red reclaim on Health Bar + hearts; sim **3s grace + 6s melt (9s total)**, chips refresh grace | **Shipped** — §4.5 |

Guard-break×perfect = Phase G research.

---

### Phase F — Difficulty scalers (settings)

| # | Work | Hooks / files | Gate |
|---|------|---------------|------|
| F1 | Damage scaler setting (stack / build-on / replace `damageMultiplier`) | Same family as `damageMagnification()` | Modes behave as labeled; no second pipeline |
| F2 | NG+ drives global mult: 1st run **1×**, each NG+ **+1×** | Persistence TBD at implement; speedrun safe | NG+0=1×; NG+1=2×; …

**Why after E:** Independent of player-help; changes global feel — ship when PM/economy are stable enough to A/B.

---

### Phase G — Research / deferred (no code until brief)

| Item | Note |
|------|------|
| Guard-break only if perfect missed | Enemy/AtSpl specific — research pass |
| Horse + shield PM | Assumed foot-equivalent until proven otherwise |
| Open room mult tweaks | Field r2/r7/r14, Hidden Village, Prison, grottos — not decided |
| Per-enemy Atp tables | **Out of scope** unless newly briefed |
| PM HUD polish | After E core | **Superseded** — full plan §4.5 E-HUD |

---

### Dependency sketch

```
A DONE → B DONE ∥ E core DONE → E-HUD DONE
           └── C (wallet 50k + heart uncap)
                └── D (sword Atp shop)
F (scaler + NG+) ── after playtest of E
G (research)     ── anytime
```

**Next (user pick):** Phase **C** (wallet 50k + heart uncap), or polish.
