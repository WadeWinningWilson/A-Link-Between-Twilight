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
| **Shop page (locked, C2)** | Heart upgrades stay on **Upgrades & Services** (same page as today). **No second page** for hearts. (Phase D sword Atp picker is separate and still planned later.) |

#### Phase C2 — Heart shop research (2026-08-21; **before** implement)

**Product (locked)**

| Phase | Price | Grant | Shop row |
|-------|-------|-------|----------|
| Soft tiers 1–17 (unchanged array) | … → **9999** | **½♥** each (+2 quarter-pips) | Existing `VISIBLE_MQ_HEART` |
| Past soft-cap | **9999 + 333×n** (n = buys after tier 17) | **¼♥** each (+1 quarter-pip) | **Same row**, same page |
| Cap | Unlimited for now | — | Never sold-out by tier array |

Classic HUD past 20♥: fill / stack on **20th** heart only. Health Bar: leave.

---

##### Shop system map (fragile — do not invent a second path)

Postman UI is **two layers**: rental list logic (`d_albw_rental.cpp`) → native letter-select renderer (`d_albw_shop.cpp`).

```
kPages[]  →  rebuildActivePages()  →  sActivePages[] (non-empty tabs only)
                ↓ L/R
         currentCategory()
                ↓
         rebuildVisibleList()  →  sVisibleList[] (THIS PAGE ONLY)
                ↓
         getVisibleList()      →  dALBWVisibleEntry[] (names/prices/descs)
                ↓
         dALBWShop_c::populateRows()  →  6-row viewport + scroll
```

| Piece | Fact | C2 rule |
|-------|------|---------|
| Pages | Items / Swords / Shields / Armor / **Upgrades & Services** | Heart stays on **Upgrades** |
| Page existence | `categoryHasVisibleRows(CAT_UPGRADES)` true if MQ on **or** potion/FA/shade/oocoo rows | Keep MQ clause; unlimited heart must **not** empty the page gate |
| Heart row build | Always pushed when `cat==UPGRADES && MQ` (even if `!purchasable`) | Keep; do **not** add `VISIBLE_*` or a new page |
| Sold-out name | `showNameWhenSoldOut = true` → still **"Heart Upgrade"** | Keep |
| Sold-out price today | `price = 0` when `!purchasable` | With unlimited, row stays purchasable → real formula price |
| Viewport | **6** rows; scroll follows selection | More Upgrades rows already OK; heart is still one row |
| Purchase | `tryPurchase` → price check → `dAlbwMQ_tryPurchaseHeartShop` → deduct → `rebuildActivePages` + `rebuildVisibleList` | Same path; only MQ helpers change |
| Price type | `int` in visible entry; shop `snprintf("%d Rupees")` | Fine for 5-digit; deduct still `(u16)price` — OK until ~65k |
| **Fragility lesson** | Armor tab once vanished when page-gate ≠ row-builder (clothes ownership). Keep-alives must **mirror** `rebuildVisibleList` | Touch **only** MQ heart helpers + HUD overlay; do not “simplify” page rebuild |

**Do not for C2:** new shop page, new `VisibleKind`, dual heart rows, ImGui shop, or skipping `rebuildVisibleList` after buy.

---

##### MQ heart state today

| Store | Width | Role |
|-------|-------|------|
| Event reg **100** (`kHeartShopTierReg`) | **u8** (0–255) | Purchase count / next tier index |
| Event reg **102** (`kBonusHalfHeartsReg`) | **u8** | Bonus **half**-hearts (×2 → quarter-pips in gauge) |
| Soft array | 17 prices ending **9999** | `canPurchase` false when `tier >= 17` **or** gauge would exceed **80** (20♥) |
| Grant | `grantHalfHeartMaxCapacity` | +2 quarters; refuses if `gauge+2 > 80` |
| `getMaxLifeGauge` | save max + `getBonusMaxLifeQuarters()` | True HP ceiling |

**Implement sketch (not coded)**

1. **Price:** tier &lt; 17 → array; else `9999 + 333 * (tier - 17)`.  
2. **canPurchase:** MQ on; drop “tier &lt; 17” and “gauge ≤ 80” stops (unlimited). Still fail if wallet short (caller).  
3. **Grant:** tier &lt; 17 → keep **½♥**; tier ≥ 17 → **¼♥** (+1 quarter). Prefer redefining bonus reg as **quarter-pips** (or additive quarter counter) so ½ and ¼ compose; u8 quarters ≈ 63♥ bonus headroom before needing another reg.  
4. **Tier bump:** keep incrementing u8 (255 soft ceiling of buys — note in playtest; not product “sold out”).  
5. **Piece / container MQ grants:** still hard-capped at 20♥ today — **leave capped** unless product says otherwise (C2 = shop row).  
6. **HUD:** `drawLife` only has **20** `mpLifeParts`. If `max_heart_cnt > 20`, clamp visible slots to 20 and **overlay** extra fill on index 19 (20th). Health Bar already % of true max — leave.  
7. **Parry Master heart reclaim:** uses heart indices — verify overlay doesn’t break dull reclaim on the 20th (playtest).

---

##### Copy — Upgrades page (same row)

**Name:** keep **`Heart Upgrade`**.

| State | Description (proposed) |
|-------|-------------------------|
| Soft-cap phase (next buy still ½♥) | `Permanently increases your maximum health by half a heart.` *(current)* |
| Past soft-cap (¼♥ buys) | `Permanently increases your maximum health by a quarter heart. Extra hearts stack on your last heart container.` |
| Optional short sold-out | Only if a hard cap returns later: `Sold out.` — not needed while unlimited |

Stamina / other Upgrades rows: **unchanged**.

---

##### C2 implement checklist (when greenlit)

| # | Work | Touch |
|---|------|-------|
| C2-1 | Price formula + canPurchase/tryPurchase/grant ¼♥ past soft-cap | `d_albw_master_quest.*` only for economy |
| C2-2 | Desc strings (table above) | `dAlbwMQ_getHeartShopDesc` |
| C2-3 | Classic HUD overlay past 20 | `drawLife` (+ reclaim smoke-test) |
| C2-4 | Confirm rental row still single Upgrades entry; no page/kind changes | `d_albw_rental` **read-only** unless price pub path needs sold-out≠0 fix |

**Out of scope:** Phase D sword page; Health Bar width; 99999 wallet; piece/container past 20.

**Playtest (2026-08-21):** LoP HUD at ≥10000 — overlap + wrong 10k digit color + shift direction. Research: **C1-HUD follow-up** below.

#### Phase C1-HUD — follow-up research (2026-08-21)

**Reported (LoP, ≥10000):** vanilla four-digit row shifts one slot; inserted 10k digit overlaps; clone shows correct **font** but **white** vs cream digits; catch-up slow on large grants.

##### 1. Layout / shift (overlap)

**Shipped logic** (`d_meter2_draw.cpp`):

| Piece | Behavior |
|-------|----------|
| Clone | Runtime `J2DPicture` at **leftmost** vanilla slot (`leftIdx` from init `r_n_*` X) |
| Vanilla row | All four `mpRupeeTexture[*]` get `paneTrans(..., + fiveShift)` while ≥10000 |
| `fiveShift` | `mRupeeFiveDigitShiftSign × mRupeeDigitAdvance` |
| Sign | Init: jewel X vs clone X — `+1` shift row **away** from jewel |
| Advance | One inter-digit spacing from init BLO (nearest right neighbor of leftIdx) |

**Likely overlap causes (match screenshot):**

1. **`mRupeeDigitAdvance` too small** — fallback `sizeX × 0.6` if neighbor probe fails; one advance may be **less than full glyph width**, so shifted thousands still sits on clone.
2. **Clone shares leftIdx geometry** — until `fiveShift` fully clears one digit width, thousands (now showing `digit_3` of remainder) and 10k place occupy the same real estate.
3. **LoP vs classic** — shift sign/advance computed from **init** BLO positions (bottom-right). LoP only lifts `r_k_n` (`sLopRupeeYOffset`); local digit spacing unchanged, but **user-reported shift “left” in LoP** suggests verifying sign against **runtime** jewel→digit order (top-right anchor may need re-measure after LoP cache, not only init centers).
4. **Classic HUD untested** in report — same code runs; worth A/B before assuming LoP-only.

**C1-4 intent was “shift entire vanilla row +1 advance while 5th shown”** — implemented, but magnitude/sign may need tuning or **runtime** advance from global digit metrics (post-LoP), not init-only.

##### 2. Color (10k digit white)

Vanilla digits inherit **BLO material tint** (cream/gold) on `r_n_*` / `r_n_*_s`. C1 clone path:

```cpp
J2DPicture* pic = JKR_NEW J2DPicture(..., src->getTexture(0)->getTexInfo(), NULL);
parent->appendChild(pic);
```

**Does not copy** `src->getBlack()` / `src->getWhite()`. New pane defaults to **identity white** → bright digit vs neighbors. **C1-5 gap:** alpha wired in `setAlphaRupeeChange`; **black/white not**. `changeTexture()` each frame does not restore tint.

**Fix shape (when implementing):** at init (and after `changeTexture` if needed), `pic->setBlackWhite(src->getBlack(), src->getWhite())` on both shadow + face clones.

##### 3. Rupee counter catch-up speed

**Pipeline:**

```
pickup / shop / enemy grant
  → dComIfGp_setItemRupeeCount(±N)   // accumulates (+=)
moveRupee() each meter frame
  → flush: save = getRupee() + pending; clear pending (instant wallet)
  → display mRupeeNum ±= 1 per frame until == save
  → drawRupee(mRupeeNum)
```

**Key facts:**

| Fact | Detail |
|------|--------|
| Tick rate | **±1 per frame** — no larger steps in code |
| Large delta | Save jumps on flush; **HUD still counts 1/frame** |
| `>= 5` threshold | **Sound only** (`Z2SE_LUPY_INC_CNT_1` / `_2`) — not faster tick |
| Not a C1 regression | Same vanilla mechanism; Colossal 50k + ×3 region rupees make gaps huge |
| Example | +3000 grant ≈ **50 s** @ 60 fps to finish animating |

**Optional product directions (not chosen):** snap display when `\|delta\| > N`; multi-step per frame; time-based lerp; HD-style faster roll — all depart from vanilla +1/frame.

**Extra wrinkle:** slow tick crossing **9999→10000** toggles 5-digit layout mid-animation (shift + clone appear while `mRupeeNum` still climbing).

##### 5. Tiered tick speed — research (2026-08-22, product proposal)

**Tier table (locked 2026-08-22):**

| Remaining gap `\|savedRupee − mRupeeNum\|` | Step / frame |
|-------------------------------------------|--------------|
| 0–**100** (inclusive) | ±1 |
| **101–500** (inclusive) | ±20 |
| **501–1000** (inclusive) | ±100 |
| **≥1001** | ±1000 |

Each frame: `step = stepForRemaining(rem)` then `mRupeeNum += sign × min(step, rem)` — **always lands exactly** on save (no overshoot).

**Verdict: feasible** — `dMeter2_c::moveRupee()` only. Save flushes instantly; display animates.

---

###### Latch vs re-tier (item 2 — more detail)

Two ways to pick the step size each frame:

| Mode | Rule | +2033 example (0 → 2033) | Matches “1000s then ones”? |
|------|------|--------------------------|----------------------------|
| **A. Latch at flush** | Step fixed from **initial** \|Δ\| when grant hits | +1000, +1000, +33 (3 frames) | **No** — second jump is another 1000-chunk, then snap 33 |
| **B. Re-tier from remaining** (recommended) | Each frame, tier from **current** \|saved − display\| | +1000 → then +100×10 → then +1×33 | **Yes** — bulk 1000, hundreds roll, ones finish |

**Product intent (+2033):** Mode **B**. After the first +1000, remainder **1033** is in the 501–1000 band → step **100**, not another 1000. When remainder hits **33**, step drops to **±1** until display equals save.

**Exact value:** Guaranteed by `min(step, rem)` every frame — e.g. rem **33** at ±1 tier → 33 frames; rem **15** at ±20 tier → one frame +15 (not +20).

**New grant mid-animation:** Save updates on flush; **re-tier from new remaining** automatically absorbs it (no separate latch field required).

---

###### Example trace: +2033 grant

Assume wallet was synced; grant flushes save to **2033**, display starts at **0**.

| Frame | Remaining | Tier | Step applied | Display after |
|-------|-----------|------|--------------|---------------|
| 1 | 2033 | ≥1001 | 1000 | 1000 |
| 2–11 | 1033→33 | 501–1000 | 100 ×10 | 2000 |
| 12–44 | 33→0 | 0–100 | 1 ×33 | **2033** |

~44 frames @ 60 fps ≈ **0.7 s** (vs ~34 s at ±1 only).

---

**Sound (item 3 — plain language)**

Vanilla plays a **counting sound** while the display catches up (“fast roll” when change ≥5). That is **separate** from how many rupees the number moves per frame.

**Recommendation:** arm fast-roll SFX when **initial** \|Δ\| ≥ **101** (first tier above ±1); keep **one tick sound per animation frame** while rolling; **`CNT_2`** when display hits save. Exact final digit does not require per-rupee sounds — the **number** still ends on 2033 exactly.

Small grants **1–100**: ±1, optional silent (vanilla only beeps when \|Δ\|≥5 today).

---

**Sound sync (vanilla model today)**

`mRupeeSound` bits in `moveRupee()`:

| Bit | Role |
|-----|------|
| **2** | “Fast **increase** roll” — set when flush Δ ≥ **5** |
| **3** | “Fast **decrease** roll” — set when flush Δ ≤ **−5** |
| **0** | Toggle: while bit 2, alternate frames → `Z2SE_LUPY_INC_CNT_1` |
| **1** | Toggle: while bit 3, alternate frames → `Z2SE_LUPY_DEC_CNT_1` |
| (end) | On reaching `savedRupee`: `LUPY_*_CNT_2`, clear bits |

Sounds are **per animation frame**, not per rupee — vanilla already decouples tick size from SE rate when bit 2 is on (+1/frame but SE every other frame).

**To keep audio aligned with tiered steps:**

| Change | Recommendation |
|--------|----------------|
| Fast-roll threshold | Raise bit **2/3** arm from `≥5` to **`≥101`** (first tier that uses step > 1) — avoids “fast roll” SFX on 5–99 that still tick ±1 |
| While animating | Keep **one** `INC_CNT_1` / `DEC_CNT_1` alternation **per display frame** (unchanged) — works with ±20/100/1000; roll ends quickly on big grants with a single `CNT_2` |
| Small grants 1–100 | Bit 2 off, ±1, **silent** (same as vanilla Δ<5 today for 1–4; 5–100 would stay ±1 — product call whether 5–100 should arm bit 2 for tick sound only) |
| `_delete` | Already plays `CNT_2` if bits 2/3 set — still valid |

**No sync work needed for:** enemy `+n` popup (fixed 120f, grant amount — independent); `drawRupee(mRupeeNum)` (each step redraws); wallet size / LoP layout flags (unchanged).

**Edge cases**

| Case | Handling |
|------|----------|
| Cross **10000** in one step | `drawRupee` toggles 5-digit layout that frame — OK; may pop layout (same as slow cross, faster) |
| Magic armor / shop spend | Same tiers on `\|Δ\|` decrease |
| Debug slider (`d_menu_window_HIO`) | Uses `setItemRupeeCount` — picks up tiers automatically |
| Δ > 1000 but remainder < step | Final clamp frame lands exactly on save |
| Frame hitches | Variable frame rate = variable wall-clock; step is per **meter** frame (vanilla) |

**Product locks (updated)**

1. ~~Boundaries~~ — **locked** (table above).
2. ~~Latch vs re-tier~~ — **re-tier from remaining** (Mode B) for 1000-then-ones feel + exact landing.
3. Sound — arm fast roll at initial \|Δ\| ≥ **101**; exact value is display math, not SFX.

**Setting (locked 2026-08-22):**

| Field | Value |
|-------|--------|
| **Name** | Faster Rupee Tick |
| **Home** | Settings → **Quality of Life** — **directly after Bigger Wallets** (wallet / rupee HUD cluster; same section as Disable Rupee Cutscenes, No Rupee Returns) |
| **Key** | `game.fasterRupeeTick` (suggested) |
| **Default** | **Off** — vanilla ±1/frame when disabled |
| **Help** | "When the rupee counter is catching up after a large gain or loss, roll in bigger steps (up to 1000 per frame) so the display reaches the correct total faster. Off uses the original one-rupee-per-frame animation." |
| **Hook** | `moveRupee()`: if off, existing ±1 path; if on, Mode B tiers + `min(step, rem)` |

*Not* under ALBW Master Quest or Difficulty — affects all rupee motion (pickups, shops, armor drain, enemy grants), not MQ-only.

**Files:** `settings.h` / `settings.cpp`, `d_meter2.cpp` (`moveRupee` + `stepForRemaining(rem)` helper). No save/HUD layout change.

---

##### 6. Implement checklist (C1-HUD fix pass)

| # | Item |
|---|------|
| H1 | Copy black/white from `leftIdx` clone source to both 10k panes |
| H2 | Tune `fiveShift` — verify advance ≥ digit width; re-probe after LoP layout cached |
| H3 | Confirm shift sign classic **and** LoP (runtime jewel vs leftmost digit) |
| H4 | Playtest 9999→10000 transition during animated tick |
| H5 | **Tiered tick:** re-tier from **remaining**; 1/20/100/1000 + `min(step,rem)`; sound arm ≥101 |

---

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

**UI:** picker on a **dedicated Sword Upgrades page** (not QS `CAT_SWORDS`, not crowded Upgrades). **Four independent rows** — Wooden / Ordon / Master / Light — each its own save slot and purchase ladder. Row appears **only if that sword is in possession** (see possession check below).

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

#### Phase D — implement research (2026-08-21; **before** code)

**Page:** New category (e.g. **Sword Upgrades**), not `CAT_SWORDS` (QS wardrobe — tab **vanishes** if QS off) and not crowding **Upgrades & Services**. Mirror `categoryHasVisibleRows` ↔ `rebuildVisibleList` (Armor tab lesson).

**Per-sword rows (product lock):**

| Row | Item | Show when |
|-----|------|-----------|
| Wooden Sword | `dItemNo_WOOD_STICK_e` | Possessed |
| Ordon Sword | `dItemNo_SWORD_e` | Possessed |
| Master Sword | `dItemNo_MASTER_SWORD_e` | Possessed |
| Light Sword | `dItemNo_LIGHT_SWORD_e` | Possessed |

**Possession check (implement):** `dComIfGs_isItemFirstBit(itemNo)` for that sword (permanent obtain). Do **not** require currently equipped / B-button. Wardrobe-stored still counts (first-bit stays). Missing sword → **no row** (not greyed sold-out).

**Descriptions (product lock):** **Tier-shared** templates — same wording for all four swords at a given tier; only the **name** is per-sword. **N** = that sword’s **next-purchase gate** (bug count for Tier 2, soul count for Tier 3), not owned totals. Exact copy (no surrounding quotes; leading spaces intentional where present):

| Tier | Description |
|------|-------------|
| **1** | ` I hope you don't mind, but I've been using your house to sleep in every now and then, and I found this whetstone. I can sharpen your sword to repay your hospitality!` |
| **2** | ` I'm going to let you in on a new tip from a master swordsman in castle town, they say if you lather the pheremones of N bugs, your sword will get stronger` (substitute **N**) |
| **3** | `A gruff resistance member scolded me recently, saying slathering bugs on a sword never works. The true secret is refining with N souls, creepy magic but true.` (substitute **N**) |

**Examples (N = next gate):** Tier 2 first step → *“…pheremones of **4** bugs…”*; Tier 3 mid ladder → *“…refining with **20** souls…”*.

**Shop feasibility (research 2026-08-21 — feasible, no code yet):**

| Fact | Implication |
|------|-------------|
| `ve.desc` is `const char*` on `dALBWVisibleEntry`; shop does `snprintf(mDescBuf, 256, "%s\n\nPrice:  %d Rupees", ve.desc, ve.price)` | Dynamic body is fine if getter returns a durable C string |
| Heart/meter/FA descs today are **string literals** (no `%d` in body) | Sword Tier 2/3 need a **formatted buffer**, not a literal |
| `getVisibleList` fills **all** rows’ `desc` pointers in one pass | **One shared static buffer would clobber** — Ordon’s “4 bugs” overwritten by Master’s “8 bugs”. Need **per-sword buffers** (×4) or equivalent |
| `mDescBuf[256]` holds desc + price footer | Draft Tier 1–3 lengths (~160–190) + `Price: …` fit; keep an eye if Tier 4 copy is longer |
| Word-wrap already runs on `mDescBuf` | Long Postman voice is OK; parchment already wraps MQ heart text |

**Do not:** bake N into one global string; format only on highlight (list still stores pointers per row).

**Atp apply:** Vanilla move Atp is **hardcoded** into `setSwordAtParam` / `initCutTurnAt` / sword `setCylAtParam` (`d_a_alink_cut.inc`). Hook: `SetAtAtp(vanilla_move_Atp + bonus)` where bonus is for **equipped** sword only. Do **not** buff wolf / iron ball / FA.

**Save:** Event regs **106–109** Atp bonus u8 ×4 swords; **110–113** purchase step u8 ×4. (100–105 taken by MQ/FA/potion/C2 quarters.)

**Gates:** Insect count = **first-bit only** (`dMenu_Insect`-style), **not** `checkGetInsectNum` (requires Agitha turn-in). Poes = `getPohSpiritNum()`. NG+ = `getClearCount() != 0`.

**Do-not:** QS-gate the Atp page; consume bugs/poes; bake Atp in shop UI; skip rebuild after buy; put rows only on Upgrades; change C2 heart regs; share one Atp counter across all swords; show a row for a sword never obtained.

**Playtest risk:** `at_power_get` cliffs (many enemies treat Atp≥4 as huge). Early A/B when bonuses push 3→4.

**Open product (non-blocking):** expand Tier 2/3 ellipses to five explicit bug/poe+₽ steps at implement; write the shared tier desc strings.

#### Phase D — Master Quest gate (research 2026-08-21)

**Product lock:** Sword Atp shop rides the **same** Settings → **ALBW Master Quest** toggle as heart + stamina (`game.masterQuest` / `dAlbwMQ_isEnabled()`).

**How heart/stamina use it today**

| Layer | Behavior |
|-------|----------|
| Setting | `dusk::getSettings().game.masterQuest` (default **off**). UI: Settings → ALBW Master Quest → “Master Quest” |
| API | `dAlbwMQ_isEnabled()` → that bool only |
| Upgrades page gate | `categoryHasVisibleRows(CAT_UPGRADES)` ORs `dAlbwMQ_isEnabled()` so the tab can exist for heart/meter |
| Row build | Heart + meter rows only if `cat == UPGRADES && dAlbwMQ_isEnabled()` |
| canPurchase / tryPurchase | First line: `if (!dAlbwMQ_isEnabled()) return false` |
| Runtime effect when MQ **off** | Bonus HP quarters → **0** (`getBonusMaxLifeQuarters`); meter shop steps ignored in meter2 — **regs keep values**, toggle back restores |

Help text today (implement will extend): *“…adds Postman heart and stamina upgrades…”*

**D must mirror (same switch, new page) — save persists; setting only chooses which Atp is live**

| Surface | Rule |
|---------|------|
| Sword Upgrades **page existence** | `categoryHasVisibleRows(CAT_SWORD_ATP)` true only if **MQ on** and ≥1 possessed sword row would build |
| Row build | MQ on + `isItemFirstBit` for that sword |
| canPurchase / tryPurchase | Fail closed if `!dAlbwMQ_isEnabled()` |
| Combat when MQ **ON** | `vanilla_move_Atp + saved_bonus` for equipped sword (e.g. move was 2, bonus 16 → **18**) |
| Combat when MQ **OFF** | **Vanilla move Atp only** (bonus dormant — e.g. same slash stays **2**, not wiped to a fake “10”) |
| Persist | Turning MQ off **must not clear** regs 106–113; turn back on → same purchased bonus resumes |
| Settings copy | Update Master Quest help to mention **sword** upgrades alongside heart/stamina |

**Illustrative numbers:** User example “18 purchased / 10 vanilla” means *effective power with upgrades on vs off* — engine path is still **per-move vanilla Atp + u8 bonus**, not a single absolute “sword Atp = 10” field. Do **not** invent a second absolute Atp store that overwrites vanilla move tables.

**Do not:** separate “Sword MQ” setting; show Sword Upgrades when MQ off; apply saved Atp bonuses while MQ off; **wipe** purchased Atp when MQ toggles off; gate only the page but not combat (or vice versa).

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
| **C1** | Insert-path 5-digit HUD + Colossal **50000** + s32 tick | `d_meter2*` / `d_save.h` | **Shipped** |
| **C2** | Heart shop past soft-cap: **+333**, **¼♥**; same Upgrades row; 20th overlay | `d_albw_master_quest.*`; `drawLife` | **Shipped** — playtest past 9999 / past 20♥ |

**Why before sword shop:** Late hearts and Atp need wallet headroom and a HUD that can show five digits.

---

### Phase D — Sword Atp shop

**Shipped (2026-08-21):** `d_albw_sword_atp.*` + Postman **Sword Upgrades** page + combat hook in `d_a_alink_cut.inc`. See §3c.

| # | Work | Status |
|---|------|--------|
| D0 | MQ gate everywhere | **Shipped** |
| D1 | Four rows (Wood / Ordon / Master / Light) | **Shipped** |
| D2 | Per-sword save + Atp apply | **Shipped** |
| D2b | Tier descs + N format | **Shipped** |
| D3 | Bug/poe gates (non-consuming) | **Shipped** |
| D4 | NG+ Tier 4 (+2 Atp) | **Shipped** |

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
| Guard-break only if perfect missed | Enemy **AtSpl 9/10/11** path (`isGuardBreakAttack`) — research / implement when briefed |
| Iron ball swing “breaks all guards” | **Ratified 2026-08-21 — not a universal AtSpl/Atp opener** (see below) |
| Horse + shield PM | Assumed foot-equivalent until proven otherwise |
| Open room mult tweaks | Field r2/r7/r14, Hidden Village, Prison, grottos — not decided |
| Per-enemy Atp tables | **Out of scope** unless newly briefed |

#### Iron ball swing vs enemy guards (G ratification)

**Player report:** swinging Ball and Chain (not throwing) seems to break every enemy’s guard — useful if true for generalizing perfect-parry-before-break.

**Code verdict:** **Partially useful observation, wrong shared mechanism for Phase G.**

| Fact | Detail |
|------|--------|
| Swing Atp / AtSpl | **Atp 2**, AtSpl **0** (`d_a_alink_ironball.inc`) |
| Throw Atp / AtSpl | **Atp 3**, AtSpl **0** |
| Type | Always `AT_TYPE_IRON_BALL` |
| Why it *feels* universal | Many enemies **special-case that type** (and some `at_power_get` PowerTypes treat iron ball like Atp≥4 → power 200). Per-enemy branches, not one global “unblockable” |
| Still resist / soft | e.g. **Lizalfos shield** often just clanks; **Darknut front** heavy-stagger, not pierce; no IRON_BALL branch → normal Atp chip |
| Player guard-break (PM/G) | Still enemy **AtSpl 9 / 10 / 11** only (`d_albw_shield.cpp` `isGuardBreakAttack`) |

**Phase G path:** keep extending the **shared AtSpl 9/10/11** deferral (perfect → no shatter). Do **not** copy iron-ball’s scattered `ChkAtType(AT_TYPE_IRON_BALL)` tree as the generalizer. Iron ball remains a separate heavy-type surface (lockout / durability skip already exist).

---

### Dependency sketch

```
A DONE → B DONE ∥ E core DONE → E-HUD DONE
           └── C DONE (wallet 50k + heart uncap)
                └── D (sword Atp shop) ← next research-complete
F (scaler + NG+) ── after playtest of E
G (research)     ── AtSpl path; iron ball ratified as non-template
```

**Next (user pick):** Phase **D** implement (research locked), or Phase **G** brief on AtSpl×perfect, or polish.
