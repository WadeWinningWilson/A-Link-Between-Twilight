# Deity Armor — Postman shop & session spec

**Status:** Product spec — **not implemented**. Locked in Outfit Stats / shop design chat (2026-06-28).

**Related:** [albw-port.md](albw-port.md) · [Outfit Stats.md](Outfit%20Stats.md) · [Quick-Sumo Work.md](Interconnected%20Chats/Quick-Sumo%20Work.md) · [Quick-Resistance Work.md](Interconnected%20Chats/Quick-Resistance%20Work.md)

---

## Summary

Deity Armor is **Magic Armor + `dItemNo_DEITY_ARMOR_e` flag** (Fierce Deity presentation on the golden armor). It is **never D-pad quick-swappable**. Enter and exit only through the **Postman rental shop**.

- **Enter:** Pay **5000 rupees** each session → auto-store all other owned outfits → equip Magic + flag → ceremony → exit shop → see armor in the field.
- **Exit:** **Store Magic Armor** on the shop screen (same row as vanilla Magic rent) → clears Deity flag → **free restore** all Postman-stored outfits → default equip **Hero’s**.

Deity is **repurchased every session** (5000 each time). High rupee rewards from the mode justify repeat entry fees — not a one-time unlock.

---

## Scope gates

| Rule | Detail |
|------|--------|
| **Not in outfit cycle** | `getNextOwned` never returns `D_ALBW_OUTFIT_DEITY`. No D-pad equip/unequip. See Quick-Sumo CAUTION for overlay bring-up if FD model work lands later. |
| **Shop only** | Enter via Deity shop purchase; exit via Magic row store action. |
| **Human Link** | Block purchase in wolf / mid-cutscene forced outfit (same family as other shop equips). |
| **Sumo peel** | If sumo overlay active, clear/peel before Deity equip (same as Magic rental grant today). |

---

## Shop prerequisites (unchanged baseline)

Deity row visible / purchasable when existing gates pass:

- Magic Armor rental eligibility (`dMeter2_isALBWRentalEligible(ARMOR_e)` — stripped at least once).
- **Colossal Wallet** (Cave of Ordeons completion).

No separate “all outfits pre-stored before buy” gate — **purchase itself** performs auto-store (Option B).

---

## Enter Deity — purchase ceremony (5000)

**Charge timing:** Deduct **5000 rupees at ceremony start**. **All-or-nothing rollback** if storage, equip, or reload fails — no partial Deity state.

**Atomic pipeline (order):**

1. Validate preconditions (wallet, eligibility, not wolf, sumo peel if needed).
2. Deduct 5000 rupees.
3. **Auto-store** every **other** owned wardrobe type to Postman storage: Sumo, Ordon, Hero’s, Zora, Magic (stash bits / wardrobe stored state — all non-Deity owned slots).
4. **Equip Magic Armor + set Deity flag** (+ Fierce Deity visuals when wired). No Hero’s pivot step on purchase — go straight to Magic + flag (same family as `dMeter2_grantRentalClothes(ARMOR_e)` + flag bit).
5. **Freeze player input** until clothes reload completes (shop UI may remain visible; block movement + menu confirm spam).
6. **Time-of-day flip (persists):** invoke the same **kankyo time machinery** as the Dusklight **Sun’s Song** cheat (`game.sunsSong` / `Z2WOLFHOWL_TIMESONG` path in `d_kankyo.cpp` — `time_change_rate` burst until day/night threshold). **Toggle every purchase:** day → night, night → day. **Persists in save** (do not restore previous time after ceremony). Outdoor Postman area (F_SP103) — no NPC time-pass concerns for v1.
7. **Power-up SFX** starts with reload; continues through reload and **a few seconds after** if reload finishes quickly (`Z2SE_AL_M_ARMER_RECOVER` family or dedicated cue — implementer choice).
8. Unfreeze input; close shop (or auto-close after SFX — product lean: **auto-close** so field reveal is first gameplay frame).
9. Player sees **Fierce Magic armor** in the overworld.

**Shop UI feedback while Deity active:**

- All other outfit rows (Sumo, Ordon, Hero’s, Zora, **Magic**) appear **re-rentable / available** on the shop screen — visual proof everything else is stored.
- **Magic row** is the **OFF switch**, not “buy another Magic for 500.” Action = **Store Magic Armor** → ends Deity session (see Exit below). Description text explains auto-store on Deity buy; re-rentable rows are sufficient visual feedback for many players.

---

## Exit Deity — store Magic (flag off)

Single exit path — **Store Magic Armor** on the shop Magic row while Deity flag is set:

1. Clear **`dItemNo_DEITY_ARMOR_e` flag** (Deity “stored” — turning god mode off).
2. **Free restore** all outfits from Postman storage to active wardrobe (**no 100-ru retrieve fee** — Deity session exception to Quick-Resistance retrieve pricing).
3. Default equipped native: **Hero’s** (`dItemNo_WEAR_KOKIRI_e` / Ordon-equivalent Hero’s clothes — implement against existing story default).
4. Magic returns to normal shop row behavior (500 rent when stripped, etc.).

Optional softer SFX on exit; **no required** time flip on exit (only on Deity **purchase**).

---

## Re-enter Deity

- Pay **5000 again** — full ceremony (auto-store, toggle time, SFX, freeze, Magic + flag).
- Not a free re-activate after first buy; Deity mode is a **paid session** each time.

---

## Relationship to other systems

| System | Interaction |
|--------|-------------|
| **Magic Armor ALBW fixes** | Rupee block / depower / +300 encounter — separate track; see [Outfit Stats.md §6](Outfit%20Stats.md#6-implementation-path-outfit-stats-chat--2026-06-28). Deity uses Deity block path when flag + >5000 rupees. |
| **Outfit Stats** | Magic/Deity **1.0×** received damage mult when toggle on. |
| **Quick Swap** | Deity **never** in cycle. Depowered Magic still blocks swap-away via `dAlbwOutfit_isSwapBlockedState`. |
| **Quick-Resistance** | Free bulk restore on Magic store (Deity exit exception). Recovery tax while Magic/Deity equipped when that system ships. |
| **Future FD overlay** | Quick-Sumo CAUTION still applies if Deity becomes Sumo-class model overlay; **v1 spec is flag-on-Magic + shop ceremony** documented here. |

---

## Implementation touchpoints (future)

| Feature | Likely file(s) |
|---------|----------------|
| Deity purchase ceremony | `d_albw_rental.cpp` (`tryPurchase` / Deity entry) |
| Auto-store all + rollback | `d_albw_wardrobe.cpp`, `d_albw_rental.cpp` |
| Magic + flag equip | `d_meter2.cpp`, `d_albw_rental.cpp` |
| Input freeze | `d_albw_rental.cpp` tick + Link clothes timer |
| Time toggle (Sun’s Song kankyo) | Extract/shared helper from `d_kankyo.cpp` + wolf howl curve-9 path; **do not** require wolf howl UI |
| Magic row dual mode (rent vs store/end Deity) | `d_albw_rental.cpp` shop list + availability predicates |
| Free restore on Magic store | `d_albw_wardrobe.cpp` |
| Sumo peel before equip | `d_albw_outfit.cpp` / existing rental grant path |

---

## Locked decisions log

| Date | Decision |
|------|----------|
| 2026-06-28 | Deity **not quick-swappable**; shop-only enter/exit |
| 2026-06-28 | **Option B:** Deity purchase **auto-stores** all other owned outfits (no pre-store gate) |
| 2026-06-28 | Purchase: equip **Magic + flag directly** (no Hero’s pivot on buy); Hero’s default **on exit** only |
| 2026-06-28 | **5000 rupees per Deity session** (repurchasable); charge at start, **rollback** on failure |
| 2026-06-28 | **Magic shop row** = store/end Deity when flagged; all rows look re-rentable while Deity active |
| 2026-06-28 | Exit: store Magic → clear flag → **free restore all** → Hero’s |
| 2026-06-28 | Ceremony: **freeze input**; power-up SFX over reload + short tail |
| 2026-06-28 | **Sun’s Song kankyo toggle** every Deity buy — **day↔night persists** (fires every time) |
| 2026-06-28 | Clear **sumo overlay** before Deity equip if worn |
| 2026-06-28 | No NPC time-pass concerns for Postman outdoor shop (v1) |

---

## Test matrix (when implementing)

- [ ] Deity buy at **day** → night persists after exit shop
- [ ] Deity buy at **night** → day persists after exit shop
- [ ] Second Deity buy toggles again
- [ ] 5000 deducted once; rollback on simulated storage failure
- [ ] Sumo worn → buy Deity → no crash; sumo stored + cleared
- [ ] While Deity active: Magic row stores → flag off, all outfits free-restored, Hero’s equipped
- [ ] D-pad Down never selects Deity
- [ ] SFX spans reload; input blocked until reload safe
- [ ] Field shows Fierce Magic after shop close
