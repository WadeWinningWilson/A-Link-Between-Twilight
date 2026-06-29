# Outfit Stats

**Status:** Product spec — **not implemented**. Research and design lock for the Outfit Stats chat.

**Related:** [Quick-Resistance Work.md](Interconnected%20Chats/Quick-Resistance%20Work.md) · [Quick-Sumo Work.md](Interconnected%20Chats/Quick-Sumo%20Work.md) · [sumo-combat.md](sumo-combat.md) · [albw-port.md](albw-port.md)

---

## Scope gate

All features in this document apply **only when Outfit Stats is enabled** (dedicated settings toggle — exact key TBD at implement).

- **Human Link only** — wolf form uses existing wolf damage rules; outfit cloth multipliers do not apply.
- **Independent of** the global **Damage Multiplier** cheat/difficulty slider (`game.damageMultiplier`) — outfit mults stack **multiplicatively** with that setting when both are active.
- Ordon’s **2.5× received-damage** applies from **game start** when the toggle is on (new save, Ordon equipped).

---

## 1. Received damage multipliers (defensive)

Applied in the incoming-damage path (`damageMagnification()` → `setDamagePoint()`), keyed off **`dAlbwOutfit_getActive()`** (target outfit, not lagging model).

| Outfit | Received damage mult | Role |
|--------|---------------------|------|
| **Sumo** | **3.5×** | Glass cannon — highest fragility |
| **Ordon** | **2.5×** | Civilian / starter cloth |
| **Zora (dry land)** | **2.5×** | Same as Ordon — no free land defense |
| **Zora (water buff active)** | **1.5×** | Same tier as Hero’s — see §1.1 |
| **Hero’s** | **1.5×** | General combat tunic |
| **Magic / Deity** | **1.0×** | Raw DR from rupee armor mechanics, not cloth |

Higher multiplier = Link takes **more** HP damage (same convention as Settings → **Damage Multiplier**).

### 1.1 Zora water defense + 3-minute persistence

When the **Zora water buff** is active:

- Received damage mult = **1.5×** (Hero’s tier — **not** better than Hero’s on land).
- Buff is **on** while Link is **in water** (in-water predicate TBD at implement — must match “actually swimming / submerged,” not shallow puddle flicker).
- Buff **remains for 3 minutes (180 s) after exiting water**, then reverts to dry-land **2.5×**.
- Re-entering water **refreshes** the timer (standard “maintained while in water + 3 min grace” behavior).

**Magnetic / electrified hazards:** Existing Zora magnetic damage path (`damageMagnification` + `i_checkZoraMag` → **10×**) remains a separate special case; outfit mult stacking order TBD at implement (recommend: outfit mult first, then magnetic exception, or document explicitly in playtest).

### 1.2 Ordon — hearts per hit (2.5×)

TP life is stored in **pieces**; **5 pieces = 1 full heart** (see Editor max-life display: `hearts + pieces`).

Damage flow: `final_pieces = base_pieces × outfit_mult`, with **round-up** when the result has a fractional piece (existing `setDamagePoint` rule).

**Ordon (2.5×) — common examples**

| Base hit (vanilla pieces) | Vanilla hearts | Ordon 2.5× pieces | Ordon hearts |
|---------------------------|----------------|-------------------|--------------|
| 1 | 0 + 1 piece | 3 | 0 + 3 pieces |
| 4 | 0 + 4 pieces (~¾ heart) | 10 | **2 hearts** |
| 8 | 1 + 3 pieces | 20 | **4 hearts** |
| 10 | 2 hearts | 25 | **5 hearts** |

**Takeaway for playtest:** a typical **4-piece** bokoblin-tier chip (vanilla ~under 1 heart) becomes **2 full hearts** in Ordon. An **8-piece** swing becomes **4 hearts**. Early-game Ordon-only play will feel noticeably softer when the toggle is on.

Fall damage uses the same pipeline (`setLandDamagePoint` → `setDamagePoint`): small fall (base **4**) → **10 pieces (2 hearts)**; big fall (base **8**) → **20 pieces (4 hearts)**.

---

## 2. Sumo — offensive kit (Sumo + Wooden Sword, no shield)

When **all** of the following are true:

1. **Sumo outfit** is the active target (`D_ALBW_OUTFIT_SUMO`),
2. **Wooden Sword** is equipped (`dItemNo_WOOD_STICK_e`),
3. **No shield** is equipped (`dComIfGs_getSelectEquipShield() == dItemNo_NONE_e` — stored shields in Postman wardrobe do not count),

Link gains:

| Effect | Value |
|--------|--------|
| **Damage output** | **4×** multiplier on outgoing sword damage (apply in the Link→enemy attack-power path, after base sword scaling, before enemy HP — exact hook TBD; mirror `d_cc_uty.cpp` / FA resolver conventions) |
| **Hidden Skills** | **Unlocked** for Wooden Sword — Mortal Draw, Jump Strike, Helm Splitter, Great Spin, and related HS procs that are **blocked for wood today** (see Quick-Resistance §5 draft: “Wood … HS Blocked”) |

**Design intent:** Sumo + wood + empty off-hand = **high-risk striker** — 3.5× damage taken (§1) traded for 4× damage out and full HS access on the weakest sword.

**Explicit non-goals / edge cases (TBD at implement):**

- Shield **broken** but still “equipped” — does that break the combo? (Recommend: any equipped shield slot disqualifies.)
- **Sumo Fists** mode (`sumoOutfitFists`) — confirm whether hidden weapons disqualify the offensive kit.
- HS unlock requires **`hiddenSkillRework`** / FA systems already on — document dependency.
- 4× stacks with FA spend columns and future sword damage tables — playtest for burst damage.

---

## 3. Swimming speed + diving

When Outfit Stats is enabled:

### 3.1 Base water (all outfits)

- **+5% forward swim speed** (multiply `getSwimFrontMaxSpeed()` / relevant HIO forward caps — hook in `d_a_alink_swim.inc` swim locomotion).
- **Diving allowed** — enable underwater dive / submerged locomotion for non-Zora outfits where vanilla restricts it today (implement against `getZoraSwim()`, `procSwimDiveInit`, oxygen / `checkZoraWearAbility` gates; exact behavior locked at implement).

### 3.2 Zora armor

- **Additional +10% swim speed** while Zora clothes are worn (`checkZoraWearAbility()`).
- **Recommended stacking:** multiplicative on top of base buff → **1.05 × 1.10 ≈ 15.5%** total vs vanilla Zora swim; confirm in playtest.

Zora already ignores oxygen timer (`checkOxygenTimer`); unchanged unless product says otherwise.

---

## 4. Relationship to other ALBW systems

| System | Interaction |
|--------|-------------|
| **Quick-Resistance recovery tax** | Separate axis — outfit stack penalties (Sumo 5%, Ordon 10%, …) still apply when Quick Swap wardrobe tax is active. Damage mult and recovery tax reinforce the same fantasy (light cloth = fast reg, fragile). |
| **Magic Armor rupee drain** | Magic/Deity at **1.0×** received mult; rupee absorption unchanged. |
| **`DOUBLE_DEFENSE` magic mode** | Halves damage **after** magnification — order must stay documented at implement. |
| **Deity shop / session** | **Not in D-pad cycle.** Enter/exit via Postman shop — full spec: [albw-deity-armor-shop.md](albw-deity-armor-shop.md) |
| **Deity visual upgrade** | Fierce Deity on Magic + flag; overlay bring-up notes in Quick-Sumo CAUTION if model work expands beyond flag |

---

## 5. Implementation map (future)

| Feature | Likely hook |
|---------|-------------|
| Outfit received-damage mult | `daAlink_c::damageMagnification()` + `dAlbwOutfit_getActive()` |
| Zora water buff + 3 min timer | New save-backed or runtime timer; water-exit event sets 180 s grace |
| Sumo + wood 4× damage | `d_cc_uty.cpp` Link attack branch or FA melee resolver |
| Sumo + wood HS unlock | `d_a_alink_cut.inc` HS gates + `dMeter2_canALBWHiddenSkill` / wood checks |
| Swim +5% / Zora +10% | `d_a_alink_swim.inc` — `getSwimFrontMaxSpeed()`, dive proc gates |
| Master toggle | `dusk/settings.h` + settings UI (Gameplay or ALBW section) |

---

## 6. Implementation path (Outfit Stats chat — 2026-06-28)

**Status:** Planning lock — **not implemented**. Ship **Magic Armor fixes first** (separate track, toggle-independent); then Outfit Stats phases below.

### Recommended order

```text
Phase 0 — Product lock (both tracks)
    ↓
Phase 1 — Magic Armor ALBW fixes          ← ship first (~1 PR)
    ↓
Phase 2 — Outfit Stats foundation         (toggle + query helpers)
    ↓
Phase 3 — Received damage + Zora timer
    ↓
Phase 4 — Swimming (+5% / dive / Zora +10%)   (spike dive scope first)
    ↓
Phase 5 — Sumo offensive kit
    ↓
Phase 6 — Docs + playtest matrix
```

**Quick-Resistance** (recovery tax, provisional wood meter spend) is a **parallel track** — not blocking Outfit Stats except where Sumo wood HS meter rules need a single owner (see Phase 5).

### Phase 0 — Lock before coding

**Magic Armor**

| Decision | Options |
|----------|---------|
| Under-500 body hit | Always depower (gray + `dMeter2_onALBWArmorHit` + heavy)? Block HP only when ≥500? Rupee cost when broke (none / drain-to-zero / −500 clamped)? |
| Clean +300 reward | Keep “any armored body hit taints” (current), or taint only when HP actually damaged (`!armor_no_dmg`)? |

**Outfit Stats**

| # | Topic | Lean at implement |
|---|--------|-------------------|
| 1 | Master toggle | Gameplay or ALBW section; **does not require** Quick Swap mode |
| 2 | Zora in-water | `MODE_SWIMMING` + depth (`mWaterY` vs Link Y) |
| 3 | Sumo+wood edge cases | Any equipped shield disqualifies; decide Fists mode |
| 4 | Zora swim stack | Confirm multiplicative **15.5%** vs flat **15%** total |
| 5 | Non-Zora diving | Spike: surface dive only vs full submerged locomotion |

### Phase 1 — Magic Armor ALBW fixes (separate PR)

**Problem:** Below 500 rupees, `checkMagicArmorNoDamage()` is false → no `dMeter2_onALBWArmorHit()`, no gray, no −500, HP damage applies. Product expects **depower on any armored body hit**; wallet gates only block + rupee cost.

**Design split**

| Concern | Gate |
|---------|------|
| Block HP + −500 | Current: ≥500 rupees + `dMeter2_canALBWArmorBlock()` (+ Deity exception) |
| Depower on hit | New: ALBW mode + `checkMagicArmorWearAbility()` + enemy **body** hit (not shield branch) → always `dMeter2_onALBWArmorHit()` |

**Touchpoints:** `d_a_alink_damage.inc` (body-hit depower hook; optional encounter taint fix), `d_a_alink.cpp` (per-frame gray / golden restore when depleted but broke), `d_a_alink_wolf.inc` (model load respects `dMeter2_isALBWArmorDepleted()` in ALBW mode).

**Suggested PR split:** (1) depower + visual/load fix; (2) encounter taint policy if changed.

### Phase 2 — Outfit Stats foundation

New helper surface (suggest `d_albw_outfit_stats.h/.cpp` or section in `d_albw_outfit.cpp`):

- `dAlbwOutfitStats_isEnabled()` — master toggle
- `dAlbwOutfitStats_getReceivedDamageMult()` — 3.5 / 2.5 / 1.5 / 1.0
- `dAlbwOutfitStats_isSumoOffensiveKitActive()` — Sumo + wood + no shield
- `dAlbwOutfitStats_tick()` / `dAlbwOutfitStats_isZoraWaterBuffActive()` — 180 s grace

Settings: `include/dusk/settings.h`, `src/dusk/settings.cpp`, `src/dusk/ui/settings.cpp`. Default **Off**.

**Identity rule:** Always `dAlbwOutfit_getActive()` (target outfit). Skip when wolf.

### Phase 3 — Received damage (§1)

Hook `damageMagnification()` **after** global `damageMultiplier`, **before** wolf 2× and Zora magnetic 10×.

Stack: `base × damageMultiplier × outfitMult × (wolf / magnetic exceptions)`.

Zora grace timer: set/refresh on water enter/exit from `d_a_alink_swim.inc`; tick from Link execute.

### Phase 4 — Swimming (§3)

`getSwimFrontMaxSpeed()` — ×1.05 all outfits when toggle on; extra ×1.10 when `checkZoraWearAbility()`.

Diving: spike every `getZoraSwim()` / `checkZoraWearAbility()` gate in `d_a_alink_swim.inc` before committing to full non-Zora underwater scope.

### Phase 5 — Sumo offensive kit (§2)

`d_cc_uty.cpp` — ×4 outgoing sword damage when kit active (after FA resolve, before HP mult).

`d_a_alink_cut.inc` + meter — wood HS when kit active; coordinate with Quick-Resistance §5 wood HS draft (pick one owner).

Requires `hiddenSkillRework` / FA on.

### Phase 6 — Playtest matrix

| Area | Cases |
|------|--------|
| Magic Armor | &lt;500 hit → gray; ≥500 block; shield vs body; clean +300 |
| Outfit mult | Ordon 4-piece → 2 hearts; Zora land/water/grace; Sumo 3.5× |
| Quick Swap | Cycle during/after depower; sumo peel; stored-outfit pool |
| Swim | +5% / Zora +15%; non-Zora dive entry |

---

## 7. Quick Swap interactions (Outfit Stats chat — 2026-06-28)

**Related:** [Quick-Sumo Work.md](Interconnected%20Chats/Quick-Sumo%20Work.md) · [Quick-Resistance Work.md](Interconnected%20Chats/Quick-Resistance%20Work.md)

Outfit Stats and Magic Armor fixes are **mostly orthogonal** to the D-pad outfit cycle — they hook combat/swim/meter paths, not `dpad_quick_swap.cpp`. The exceptions below are where quick-swap state **does** matter.

### Independence (no coupling required)

| Feature | Why it’s safe |
|---------|----------------|
| **Outfit Stats master toggle** | Does **not** require `extraItemSlotMode == ExtraAndQuickSwap`. Works with Quick Swap on or off. |
| **Received damage mults** | Read `dAlbwOutfit_getActive()` each hit — target semantics match what the player intended to wear, including mid-async equip. |
| **Zora water 3 min grace** | Runtime timer on Link — should **persist across outfit swaps** (buff is on the player, not the cloth save bit). |
| **Swim +5% / dive** | Key off swim procs + `checkZoraWearAbility()` for the Zora bonus — not the D-pad cycle. |
| **Sumo 4× / wood HS** | Kit checks **target** Sumo + **equipped** sword/shield slots. D-pad Down cycles **outfits only** — sword/shield are separate slots; no new coupling unless product wants “stored wood disqualifies kit” (today: equipped slot only). |
| **Magic Armor −500 / +300** | Key off `checkMagicArmorWearAbility()` (equipped native clothes) and ALBW meter — not outfit cycle API. |
| **Quick-Resistance recovery tax** | Separate axis (wardrobe active pool when Quick Swap ON). Stacks narratively with Outfit Stats fragility; no shared code path with damage mult. |

### Direct interactions (must respect quick-swap invariants)

| Interaction | Behavior today | After Magic Armor + Outfit Stats work |
|-------------|----------------|--------------------------------------|
| **Swap blocked while heavy** | `dAlbwOutfit_isSwapBlockedState()` → deny SFX if `checkBootsOrArmorHeavy()` (depowered Magic) or ALBW lockout exhaustion or iron boots or ghoul rat | **Keep gate.** Under-500 depower fix **increases** how often depowered Magic blocks cycling away — **intended** (same as 0-rupee heavy today). |
| **Equip into Magic** | Allowed while powered | Unchanged. Model reload uses `Mmdl` / magic BRK path — see Quick-Sumo “CAUTION — Magic Armor”. |
| **Cycle away from depowered Magic** | Blocked via `checkMagicArmorHeavy()` → `dMeter2_isALBWArmorDepleted()` | Under-500 fix makes hits set depleted → **swap-away blocked** even when wallet &lt;500 (today broke players stay golden and can cycle away — fix closes that gap). |
| **Swap in progress** | `dAlbwOutfit_isSwapInProgress()` blocks D-pad; `mClothesChangeWaitTimer` delays magic BRK gray/golden updates | Stats use **target** `getActive()` — during peel (sumo → base) mult may flip one frame before model catches up; acceptable if target semantics stay consistent. |
| **Sumo overlay + stats** | `getActive()` returns **SUMO** while bit 700 set, even if native base is Hero's under the hood | Sumo **3.5×** + offensive kit apply while sumo is target; peeling to Hero's drops to Hero's **1.5×** on next target. First Down off sumo = peel to base, not ring advance (Quick-Sumo peel rule). |
| **Stored outfits (Postman)** | When Quick Swap ON, `getNextOwned` skips non-active wardrobe entries | Player cannot cycle into stored outfits; stats for “what you’re wearing” still follow equipped/target state — no special stored-outfit mult. |
| **Magic in rotation** | Native `Mmdl` in ring after Zora; sumo-over-Magic uses non-Zora face path (no Kmdl donor) | Magic Armor **fixes** touch damage/meter only — **do not** change `applyTargetKind` / arc residency (Quick-Sumo aliasing rule). |

### Risk register (quick-swap bring-up)

1. **Depowered Magic + under-500 fix** — More players will hit `checkMagicArmorHeavy()` without 0 rupees; confirm swap block + heavy movement feel correct before extending cycle to Deity.
2. **Sumo peel mid-combat** — Offensive kit toggles off when target leaves Sumo; intentional but sharp — playtest D-pad mash during sumo+wood fights.
3. **Zora grace + swap to dry outfit** — Exiting water in Zora then cycling to Ordon: grace timer should still run (Hero's-tier 1.5× on land until timer expires) unless product wants grace cleared on outfit change (**TBD** — recommend **keep grace**).
4. **Concurrent chats** — Implement Magic Armor in damage/meter files only; Outfit Stats in magnification/swim/attack helpers; **avoid** edits to `getNextOwned` / `applyTargetKind` / `nativeClothesResourcesReady` unless Quick-Sumo chat signs off.

### Suggested cross-chat test checklist

After Magic Armor + Outfit Stats land, re-run Quick-Sumo re-test matrix subset:

- Cycle into/out of Magic with **&lt;500 rupees after a body hit** (depowered, swap-away blocked).
- Sumo + wood kit active → peel to Hero's via Down → kit drops, mult changes.
- Zora water grace active → cycle to Hero's → grace still applies for remaining timer.
- Full owned ring with Quick Swap ON + Outfit Stats ON — no crash regression (`outfit_swap_debug.txt`).

---

## 8. Locked decisions log

| Date | Decision |
|------|----------|
| 2026-06-28 | Sumo **3.5×** received; Ordon **2.5×** from game start when toggle on |
| 2026-06-28 | Zora dry **2.5×**; water + 3 min post-water **1.5×** (Hero’s tier) |
| 2026-06-28 | Hero’s **1.5×**; Magic/Deity **1.0×** |
| 2026-06-28 | Sumo + Wooden Sword + no shield → **4×** damage out + wood **Hidden Skills** enabled |
| 2026-06-28 | Base swim **+5%** + diving; Zora armor **+10%** swim speed |
| 2026-06-28 | Implementation path locked (§6); Magic Armor fixes ship before Outfit Stats phases |
| 2026-06-28 | Outfit Stats **independent** of Quick Swap mode; interactions documented (§7) |
| 2026-06-28 | Deity shop/session spec → [albw-deity-armor-shop.md](albw-deity-armor-shop.md) (not quick-swappable; 5000/session; Magic row = OFF) |

---

## 9. Open confirmations (awaiting product)

| # | Topic | Notes |
|---|--------|--------|
| 1 | Master toggle name + settings tab | **Locked for implement:** does **not** require Quick Swap (see §6 Phase 0, §7) |
| 2 | Zora “in water” predicate | Swim mode vs depth vs `FLG0_WATER_IN_MOVE` |
| 3 | Sumo+wood: Fists mode, broken-shield edge case | |
| 4 | Zora swim stack: multiplicative **15.5%** vs flat **15%** total | |
| 5 | Non-Zora “allow diving” | Surface dive only vs full underwater locomotion like Zora |
| 6 | Zora grace timer on outfit swap | Keep grace across cloth change vs clear on swap (**lean: keep**) |
| 7 | Magic Armor under-500 rupee cost + clean +300 taint rule | See §6 Phase 0 |
