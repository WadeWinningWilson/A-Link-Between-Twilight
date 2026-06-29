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
| **Deity visual upgrade** | Out of scope for this doc — see Outfit Stats chat research (Fierce Deity texture swap on Magic + deity flag). |

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

## 6. Locked decisions log

| Date | Decision |
|------|----------|
| 2026-06-28 | Sumo **3.5×** received; Ordon **2.5×** from game start when toggle on |
| 2026-06-28 | Zora dry **2.5×**; water + 3 min post-water **1.5×** (Hero’s tier) |
| 2026-06-28 | Hero’s **1.5×**; Magic/Deity **1.0×** |
| 2026-06-28 | Sumo + Wooden Sword + no shield → **4×** damage out + wood **Hidden Skills** enabled |
| 2026-06-28 | Base swim **+5%** + diving; Zora armor **+10%** swim speed |

---

## 7. Open confirmations (awaiting product)

| # | Topic |
|---|--------|
| 1 | Master toggle name, settings tab placement, and whether Outfit Stats requires Quick Swap mode |
| 2 | Zora “in water” predicate (swim mode vs depth vs `FLG0_WATER_IN_MOVE`) |
| 3 | Sumo+wood: Fists mode, broken-shield edge case |
| 4 | Zora swim stack: multiplicative **15.5%** vs additive **15%** |
| 5 | Non-Zora “allow diving” — surface dive only vs full underwater locomotion like Zora |
