# Quick / Resistance Work

**Interconnected chat doc (ARCHIVE / session diary)** — D-Pad Quick Swap ↔ ALBW meter recovery tax ↔ Postman shop storage ↔ per-item spend / parry tuning.

**Live tip:** create [`docs/state/`](../state/) entries when this track resumes; keep this file intact for locked specs and revision history.

| | |
|---|---|
| **Quick Swap chat** | `dpad_quick_swap.cpp`, wardrobe cycle, gates, stored-count helper |
| **Meter / shop chat** | `d_meter2.cpp`, `d_albw_rental.cpp`, `d_albw_shield.cpp`, recovery mult, shop store/return |
| **Status** | **Spec locked for review** — not implemented. **Sword spend table provisional** (product update incoming). |
| **Index** | [`docs/AGENT_INDEX.md`](../AGENT_INDEX.md) |

**Related:** [d-pad-reworking.md](../d-pad-reworking.md) · [Quick-Sumo Work.md](Quick-Sumo%20Work.md) · [albw-port.md](../albw-port.md) · [shield-combat.md](../shield-combat.md)

---

## How to use this doc

Shared workspace for **parallel Cursor chats** on Quick Swap loadout pressure and ALBW recovery/spend.

**Rules:** Same as [Quick-Sumo Work.md](Quick-Sumo%20Work.md) — label edits (`Quick Swap Chat:` / `Meter Chat:` / `Shop Chat:`), link long design elsewhere, lock decisions in the table below.

**New chat connecting here:** read **Locked decisions** and **Open confirmations** first; sword section marked **PROVISIONAL**.

---

## Locked decisions

| Topic | Decision |
|-------|----------|
| Scope gate | **Human Link only**; active **only when D-Pad Quick Swap is ON** (`extraItemSlotMode == Extra + Quick Swap`). Wolf unaffected. Quick Swap OFF → no wardrobe recovery tax. |
| Core loop | **Active wardrobe** (owned − shop-stored) slows **passive + lockout** ALBW recovery; **Postman storage** sheds load. Replaces evict-order limiter (697–699). |
| Recovery reference | **1 sword + 1 shield + 1 outfit** after first Twilight (Hero's + Ordon sword + Ordon shield). |
| Extra swords / shields | **−10%** recovery per extra sword · **−15%** per extra shield (beyond 1 each). |
| Outfit recovery | **One outfit owned → 0%** outfit penalty. **Two or more outfits owned** → **stack** penalties (includes **currently equipped** outfit). |
| Outfit stack rates | **Sumo 5%** · **Ordon 10%** · **Hero's 25%** · **Zora TBD** (per owned type in active wardrobe). |
| Magic / Deity | **−30%** recovery while equipped (powered); **−50%** while powerless drained (**replaces** −30% until exit drained state). See §1 — stack vs worn interaction. |
| Shop store | **Free** to store. Prompt: *“Do you want to store this for later? A storage fee will apply upon its return.”* |
| Shop retrieve / buy | **100 rupees** per item (buy price change from current tier pricing — product locked 2026-06). Storing still free; fee applies on return. |
| Shop buy copy | Description text **unchanged** on purchase flow. |
| Shop eligibility | Item acquired once in normal play **and** shop row revealed (existing ALBW save bits). Store/return: swords, shields, outfits. |
| Parry refill | **% of current `sOilMaxVar`**, per **equipped** shield at parry time. |
| Wood FR | **Not** during ALBW meter lockout or empty. Wood-specific FR bonus code **out of scope** for this pass. |
| Sword spend / damage | **PROVISIONAL** — see §5; product will revise imminently. Do not implement sword table until updated. |
| Evict limiter | **Dropped** — shop storage replaces 697–699 evict-order design. |

---

## Open confirmations (awaiting product)

| # | Topic | Status |
|---|--------|--------|
| — | **Zora** outfit stack rate (when ≥2 outfits owned) | **TBD** |
| — | Magic/Deity **owned but not equipped** — use **30%** in stack when ≥2 outfits, or worn-only? | **Lean: 30% in stack when owned; 50% replaces 30% when equipped + drained** — confirm at implement |
| 4 | Minimum `recoveryMult` floor (stack + sword/shield sums can exceed 100%) | **Deferred** — stack redesign reduces harsh generic −20%/outfit; floor still sensible at implement |
| — | Sword spend table (§5) | **Await imminent revision** |

---

## 1. Wardrobe recovery modifier

Apply to **normal passive recovery** and **lockout passive recovery** (stacks on existing meter-tier scaling in `computeALBWRecoveryRate()` / `albwLockoutRecoveryRate()`).

**Stored** = owned in active wardrobe, **not** returned to Postman storage.

### Swords and shields

```text
swordPenalty  = 0.10 × (storedSwords  − 1)
shieldPenalty = 0.15 × (storedShields − 1)
```

Below reference on a slot → **faster** (negative penalty). Unchanged from prior spec.

### Outfits — single vs stack

**Exactly one outfit owned** (any type): **outfitPenalty = 0** — no recovery effect from outfits. A true **1:1:1** load at post–Twilight baseline therefore has **no outfit tax** (only sword/shield lines apply if above/below 1).

**Two or more outfits owned:** activate **per-outfit stacks**. Sum the penalty for **each owned outfit type** in the active wardrobe, **including the outfit Link is currently wearing**.

| Outfit type | Stack penalty (each owned) |
|-------------|----------------------------|
| **Sumo** | **5%** |
| **Ordon** | **10%** |
| **Hero's** | **25%** |
| **Zora** | **TBD** |
| **Magic / Deity** | **30%** when owned *(see drained rule below)* |

**Examples (outfits only, ignoring sword/shield lines):**

| Owned outfits | Equipped | Outfit penalty |
|---------------|----------|----------------|
| Hero's only | Hero's | **0%** |
| Sumo + Ordon | Ordon | **15%** (5 + 10) |
| Sumo + Hero's | Hero's | **30%** (5 + 25) |
| Sumo + Ordon + Hero's | any | **40%** (5 + 10 + 25) |

### Magic / Deity (equipped — drained state)

When **Magic Armor or Deity Armor is equipped**:

| State | Penalty |
|-------|---------|
| Powered | **30%** slower |
| Powerless drained | **50%** slower — **replaces** the 30% until armor is magic again or Link exits drained state |

**Implementation note:** When Magic/Deity is in a multi-outfit wardrobe, treat **30%** as its stack contribution while owned; bump that contribution to **50%** (not additive) while equipped **and** drained. Confirm at implement if worn-only should bypass stack math entirely.

### Combined formula

```text
outfitPenalty = 0                                    if ownedOutfitTypes == 1
              = sum(stackRate[type] for each owned)   if ownedOutfitTypes >= 2
              (apply Magic/Deity drained swap on Magic line when equipped)

recoveryMult = 1 − swordPenalty − shieldPenalty − outfitPenalty
             (clamp to sensible minimum at implement — see open §4)
```

---

## 2. Meter tier baseline (today's ALBW code)

Wardrobe mult stacks on top. **Rate** rises with upgrades; **normal time-to-full** at 1:1:1 creeps **~10.0 s → ~15.5 s** (bar grows faster than rate). **Lockout:** **~7.0 s** at base cap only; **~10.0 s** once expanded (7 s + 3 s), flat across expansion sizes before wardrobe mult.

| Tier | Capacity | Rate | Normal full @ 1:1:1 | Lockout @ 1:1:1 |
|------|----------|------|---------------------|-----------------|
| 0 — Base | 10,900 | 109 | 10.0 s | 7.0 s |
| +4 dungeons | 18,168 | 146 | 12.4 s | 10.0 s |
| All 8 dungeons | 25,436 | 184 | 13.8 s | 10.0 s |
| + 5 hearts | 33,916 | 228 | 14.9 s | 10.0 s |
| Max (+ armor cap) | 42,396 | 273 | 15.5 s | 10.0 s |

### Example wardrobe rows (divide baseline times by `recoveryMult`)

Illustrative loads — **outfit stack depends on which types are owned**, not just count.

| Row | Load | Outfit math | Approx mult | Normal @ tier 0 | Normal @ max |
|-----|------|-------------|-------------|-----------------|--------------|
| Early Ordon field | 1 sword, 0 shield, **1 outfit** (Ordon) | 0% outfit | **1.15×** (shield −1) | ~8.7 s | ~13.5 s |
| Post–Twilight baseline | 1:1:1, **Hero's only** | 0% outfit | **1.00×** | 10.0 s | 15.5 s |
| Sumo + Ordon stash | 2:2:2, outfits **Sumo+Ordon** | 15% + 10% sword + 15% shield | **0.60×** | ~16.7 s | ~25.8 s |
| Wide stash | 3:2:3, **Sumo+Ordon+Hero's** | 40% + 20% + 15% | **0.25×** | ~40 s | ~62 s |
| Magic drained | Sumo+Magic owned, wear Magic drained | 5% + **50%** + sword/shield lines | **varies** | — | — |

Lockout: **~7.0 s** (base cap) or **~10.0 s** (expanded) at mult **1.0**; divide by `recoveryMult` same as normal.

---

## 3. Shield parry recovery

**Today:** all shields **16.7%** (`1/6`) of `sOilMaxVar`.

| Shield | Item / arc | Parry grant | Full bar | Half bar |
|--------|------------|-------------|----------|----------|
| **Ordon** | `WOOD_SHIELD` / CWShd | **10%** | 10 parries | 5 |
| **Wooden** | `SHIELD` / SWShd | **25%** | 4 parries | 2 |
| **Hylian** | `HYLIA_SHIELD` / HyShd | **20%** | 5 parries | 2.5 |

Parry count independent of wardrobe mult; heavy stash slows **passive** only → **Wooden + parry** compensates intentional guard play. Hylian failed block **−20%** max (existing) symmetric with **+20%** success.

**One parry ≈ passive seconds (max tier, heavy 3:2:3):** Ordon ~6.2 s · Wooden ~15.6 s · Hylian ~12.5 s vs **62.3 s** passive full.

---

## 4. Postman shop — storage

| Action | Behavior |
|--------|----------|
| **Store** | Free. Removes item from active wardrobe → lowers recovery load. |
| **Buy / retrieve** | **100 rupees**. Normal description. Return fee included in buy price model above. |
| **Shop open** | Quick Swap suppressed (`dALBWRental_isOpen()`). |

Quick Swap cycles **owned, non-stored** items (exact skip rules TBD at implement).

---

## 5. Sword ALBW spending — ⚠ PROVISIONAL

**Product will revise this section.** Do not ship until updated.

Does **not** change FA/FR spend columns — ALBW meter drain on swings / hidden skills only.

### Damage (target)

| Sword | Damage |
|-------|--------|
| Wood | 5 |
| Ordon | 10 |
| Master | 20 |
| Light | 30 |

### Meter spend (draft)

| Sword | Normal swing | Swings / full @ base | Hidden skill | Notes |
|-------|--------------|----------------------|--------------|-------|
| **Wood** | ~991 (~1/11) | **11** (+5 vs today) | Blocked | No HS / no FA; FR allowed when implemented **except** lockout/empty |
| **Ordon** | 1817 (1/6) | 6 | 5450 (1/2) | Base |
| **Master** | 2726 (+50%) | ~4 | 6813 (+25% HS) | |
| **Light** | 3634 (+100%) | ~3 | 8175 (+50% HS) | |

Agility costs unchanged unless product says otherwise.

---

## 6. System map

```text
                    ┌─────────────────────┐
                    │  Active wardrobe    │
                    │  (owned − stored)   │
                    └──────────┬──────────┘
                               │
         Quick Swap ON         │         Shop “Store” (free)
         (human only)          │
                               ▼
              ┌────────────────────────────────┐
              │  recoveryMult → passive +        │
              │  lockout refill                  │
              └────────────────────────────────┘

  Equipped sword ──► ALBW swing / HS spend (§5 TBD)
  Equipped shield ─► parry % grant (§3)
  Equipped Magic/Deity ─► worn recovery tax (§1)
```

---

## 7. Implementation touchpoints

| Feature | File(s) |
|---------|---------|
| `recoveryMult` + Magic/Deity worn | `d_meter2.cpp` |
| Per-sword spend + wood HS block | `d_meter2.cpp`, `d_a_alink_cut.inc` |
| Per-shield parry % | `d_albw_shield.cpp` |
| Shop store/return + stored state | `d_albw_rental.cpp`, `d_albw_shop.cpp` |
| Stored count API | new helper; consumed by meter + Quick Swap |
| Quick Swap gate | `dpad_quick_swap.cpp`, `action_bindings.cpp` |

---

## 8. Explicitly out of scope (this pass)

- Evict-order limiter (697–699)
- “Already owned / out of stock” shop buy blocking
- Wood FR bonus during lockout/empty (separate future hook)
- FA/FR column changes
- Storage save-bit layout (TBD when shop chat implements)
- Minimum `recoveryMult` floor (numeric cap TBD)
- Zora outfit stack rate

---

## Quick Swap Chat

### Response to Meter / Shop chat

Wardrobe tax is gated on **`isDpadQuickSwapEnabled()`** + human Link. We will call a shared **`dAlbwWardrobe_getRecoveryMult()`** (name TBD) from meter tick — do not duplicate stash math in `dpad_quick_swap.cpp`. Cycle lists should exclude shop-stored items once shop exposes **`dAlbwShop_isStored(item)`** or equivalent.

### Ideas / paths ahead

- Layered Left d-pad, forced-outfit blocklist, Deity in rotation — still in [d-pad-reworking.md](../d-pad-reworking.md), unaffected by this spec until product prioritizes.

---

## Meter / Shop Chat

### Response to Quick Swap chat

Recovery mult hooks **`moveKantera()`** passive block (~2153+ in `d_meter2.cpp`). Shop storage needs save bits or rental-side flags for “stored not owned-for-load”; **100 rupee** retrieve replaces tier buy for storable swords/shields/outfits — confirm whether stripped-death rental rows stay separate.

### Ideas / paths ahead

- Persist stored items in save (new bits vs rental regs — TBD).
- Whether retrieve at 100 R applies to first-time buy vs re-buy only.

---

*Last updated: Quick Swap chat — outfit stack + Magic 30%/50% (2026-06). Sword §5 provisional.*
