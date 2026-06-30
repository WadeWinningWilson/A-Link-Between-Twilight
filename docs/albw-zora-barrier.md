# Zora Electric Barrier (MM-inspired)

**Status:** Product spec — **not implemented**. Research + input lock for a future Zora armor kit. **No code from Majora’s Mask decompilation** — behavior reference only (manuals / wiki / playtest notes).

**Related:** [Outfit Stats.md](Outfit%20Stats.md) · [Outfit Stats Roadmap.md](Outfit-Stats-Roadmap.md) · [albw-port.md](albw-port.md)

---

## Summary

Optional **Zora armor** offensive/defensive ability while **submerged swimming**: a hold-activated **electric field** around Link that **damages enemies on contact** (Majora’s Mask Zora “Barrier” / “Zora Magic Force Field” fantasy).

Gated separately from base swim speed buffs in Outfit Stats §3 — implement **after** swim speed + dive baseline, or as its own PR once predicates are stable.

---

## Reference behavior (Majora’s Mask — design only)

| Aspect | MM behavior (public sources) |
|--------|------------------------------|
| Activation | **Hold** while swimming — not a projectile |
| Effect | Bio-electric shell; **ram enemies** to hurt/stun |
| Drain | Continuous **magic** while active; field weakens when empty |
| MM3D | Barrier can also **boost swim speed** while up |
| Land | MM uses R fin + R+B on floor — **out of scope** for v1 (swim only) |

**Dusklight v1 scope:** submerged swim only; no land barrier; no MM code port.

---

## Proposed input (locked for spec)

**Hold R1 (right bumper / shield lock) + B (attack)** while qualifying swim state.

| Reason | Detail |
|--------|--------|
| **R alone** | In TP, R while Zora swimming is **steer/turn** (`checkInputOnR` in `setSpeedAndAngleSwim`) — do not steal for barrier |
| **R1 + B** | Matches “guard channel + attack” muscle memory; B is unused for sword underwater (`swimDeleteItem`); R1 is less loaded than swim steer |
| **Hold** | Barrier maintained while **both** held; release either → field drops |

**Implementation note (future):** map to `getHoldLockR(PAD_1)` + attack button hold (`getHoldB` / equip attack path) — exact PC binding follows existing shield-R / attack conventions in `d_a_alink.cpp`.

---

## Activation gates (recommended)

All required unless product adds exceptions:

| Gate | Check |
|------|--------|
| Master toggle | **`Outfit Stats` enabled** (same as §3 swim buffs) |
| Outfit | `checkZoraWearAbility()` and/or `dAlbwOutfit_getActive() == ZORA` |
| Locomotion | `getZoraSwim()` true (submerged — predicate aligned with Zora water defense buff when locked) |
| Form | Human Link, not wolf |
| State | Not cutscene / heavy magic armor slow swim / iron boots sink (product call) |
| Meter | Enough ALBW meter to start; drain while active; deny new activation when lockout empty (align with `dMeter2_canALBW…` family) |

**Not granted:** non-Zora outfits even if Outfit Stats “allow diving” for Hero’s/Ordon.

---

## Runtime behavior (hold ability)

While **R1 + B** held and gates pass:

1. **Visual** — electric bubble / eff around Link (TP electric FX vocabulary; new asset OK).
2. **Offense** — Link **At** collider(s) with `dCcD_MTRL_ELECTRIC`; damage on enemy **contact** (tick or on-enter); enemy group filter `fopAc_ENEMY_e`.
3. **Defense** — product TBD: partial chip reduction, stun-only on small foes, or offense-only (MM had both).
4. **Drain** — continuous **`dMeter2_subALBWFraction`** (or per-frame cost); release or empty meter → bubble ends.
5. **Movement** — left stick steer unchanged; optional small speed bonus while active (MM3D) — **conflicts with** passive +10% Zora swim if stacked; decide at tune time.

**Magnetic 10× hazard** (`damageMagnification` + `i_checkZoraMag`) — barrier does **not** override; separate systems.

---

## Relationship to Outfit Stats

| Outfit Stats feature | Interaction |
|---------------------|-------------|
| §3.2 Zora +10% swim | Passive always when toggle on; barrier speed boost optional extra |
| §1.1 Zora water 1.5× defense | Independent; grace timer persists across outfit swap (lean: keep) |
| §3.1 non-Zora dive | No barrier for non-Zora |

---

## Relationship to ALBW meter / Quick-Resistance

| System | Interaction |
|--------|-------------|
| **ALBW meter drain** | Barrier uses same meter pool as sword/HS/sidestep — tune cost vs [Quick-Resistance §5](Interconnected%20Chats/Quick-Resistance%20Work.md) when that table is **final** |
| **Sword spend table** | **Separate doc** — Quick-Resistance §5 is **⚠ PROVISIONAL** (not locked for ship). Barrier drain is its own fraction row when implementing |
| **Recovery tax** | Quick Swap wardrobe tax unchanged |

---

## Implementation touchpoints (future)

| Layer | Likely hook |
|-------|-------------|
| Input + state | `d_a_alink_swim.inc` — `procSwimMove` / `getZoraSwim()` execute |
| Collision | Link `mAtSph` or dedicated cyl; `SetAtMtrl(ELECTRIC)` |
| Drain | `d_meter2.cpp` — `dMeter2_subALBWFraction` |
| Gating | `d_albw_outfit_stats.*` + `checkZoraWearAbility()` |
| FX / SE | `d_a_alink_effect.inc`, electric SE family |

**Roadmap slot:** [Outfit Stats Roadmap.md](Outfit-Stats-Roadmap.md) **Phase 7** (optional, after swim baseline).

---

## Open confirmations

| # | Topic |
|---|--------|
| 1 | Offense-only vs damage reduction while bubbled |
| 2 | Per-touch damage vs DPS tick; boss interaction |
| 3 | Drain rate vs one sword swing equivalent at base meter |
| 4 | Speed boost while active vs passive +10% only |
| 5 | R1+B vs alternative if shield rework binds R1 differently underwater |

---

## Locked decisions log

| Date | Decision |
|------|----------|
| 2026-06-28 | MM-inspired swim-only electric barrier; **no MM decomp code** |
| 2026-06-28 | Input: **hold R1 + B** while submerged Zora swim |
| 2026-06-28 | Gated on **Outfit Stats** master toggle + Zora armor |
| 2026-06-28 | Drain from **ALBW meter** (not vanilla magic) |
| 2026-06-28 | Separate doc; implement after swim speed/dive baseline |

---

## Test matrix (when implementing)

- [ ] R1+B underwater Zora → bubble + enemy contact damage
- [ ] R alone still steers; B alone does not bubble
- [ ] Release R1 or B → bubble ends
- [ ] Meter drains; empty → bubble ends; lockout behavior matches sword
- [ ] Non-Zora / toggle off → no bubble
- [ ] Magnetic hazard still 10× — barrier unrelated
- [ ] No regression on oxygen, swim speed §3 buffs
