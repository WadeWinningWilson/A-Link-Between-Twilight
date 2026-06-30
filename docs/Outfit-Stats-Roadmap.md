# Outfit Stats — Implementation Roadmap

**Status:** Planning — **not implemented**. Executable order for [Outfit Stats.md](Outfit%20Stats.md) and related tracks.

**Related:** [albw-deity-armor-shop.md](albw-deity-armor-shop.md) (separate) · [albw-zora-barrier.md](albw-zora-barrier.md) (optional Zora kit) · [Quick-Resistance Work.md](Interconnected%20Chats/Quick-Resistance%20Work.md) (parallel)

---

## What this roadmap covers

| In scope | Out of scope (parallel docs) |
|----------|------------------------------|
| Outfit Stats master toggle | **Deity shop ceremony** → [albw-deity-armor-shop.md](albw-deity-armor-shop.md) |
| Received damage mults + Zora grace | **Magic Armor ALBW fixes** → Outfit Stats §6 Phase 1 (ship **before** Outfit Stats code) |
| Swim +5% / Zora +10% / non-Zora dive | **Quick Swap** outfit cycle stability → Quick-Sumo Work |
| Sumo + wood 4× + HS | **Wardrobe recovery tax** → Quick-Resistance §1 |
| Optional **Zora barrier** | **Sword ALBW meter spend table** → Quick-Resistance §5 (**provisional**) |

---

## Spending values — did we already lock them?

**No — not for Outfit Stats, and not finally for ALBW sword spend.**

| Doc | What | Status |
|-----|------|--------|
| **Outfit Stats** | Cloth damage mults, swim %, Sumo 4× out | **Locked** in [Outfit Stats.md §8](Outfit%20Stats.md#8-locked-decisions-log) |
| **Quick-Resistance §5** | Per-sword **meter drain** (Wood ~991, Ordon 1817, HS 5450, …) + target **damage** (5/10/20/30) | **⚠ PROVISIONAL** — “product will revise; **do not ship until updated**” |
| **Live code today** | Existing ALBW drain in `d_meter2.cpp` / cut paths | **Implemented baseline** (e.g. HS ~5450); not necessarily matching §5 draft |

**Outfit Stats Phase 5 (Sumo wood HS)** must **pick one owner** with Quick-Resistance: either keep “wood HS blocked” until §5 is final, or Outfit Stats overrides wood HS when Sumo kit active without waiting for §5 numbers.

**Zora barrier** (if built) adds its **own** meter drain row — tune after swim baseline; not part of §5 sword table.

---

## Dependency graph

```text
[Magic Armor fixes]     ── independent, ship first
        │
        ▼
[Phase A: Foundation]   toggle + dAlbwOutfitStats_* helpers
        │
        ├──► [Phase B: Defense]     damageMagnification + Zora 180s grace
        │
        ├──► [Phase C: Swim]      +5% / Zora +10% / dive spike → dive impl
        │         │
        │         └──► [Phase D: Zora barrier]  optional — R1+B electric field
        │
        └──► [Phase E: Sumo kit]   4× out + wood HS (needs hiddenSkillRework)

[Quick-Resistance]      parallel — recovery tax, shop storage, sword spend §5 when final
[Deity shop]            parallel — albw-deity-armor-shop.md
```

---

## Phase 0 — Product lock (½ day)

Resolve open items in [Outfit Stats.md §9](Outfit%20Stats.md#9-open-confirmations- awaiting-product):

| # | Decision needed |
|---|-----------------|
| Magic Armor | Under-500 depower + clean +300 taint (blocks nothing in Outfit Stats but ship first) |
| 2 | Zora in-water predicate |
| 4 | Zora swim stack: 15.5% mult vs flat 15% |
| 5 | Non-Zora dive: surface-only vs full submerged |
| 6 | Zora grace persists on outfit swap (**lean: yes**) |
| Sumo | Fists mode, broken-shield edge case |
| Settings | Toggle name + tab (Gameplay vs ALBW); default **Off** |

**Exit criteria:** No TBD that changes hook choice for Phase A–C.

---

## Phase 1 — Magic Armor ALBW fixes (separate PR, prerequisite)

**Not Outfit Stats** — see [Outfit Stats.md §6 Phase 1](Outfit%20Stats.md#phase-1--magic-armor-albw-fixes-separate-pr).

| Deliverable | Verify |
|-------------|--------|
| Depower on any armored body hit (ALBW mode) | &lt;500 → gray + depleted; ≥500 → block + −500 |
| Optional encounter taint fix | Product call |
| Model load respects depleted in ALBW | No false golden when broke |

**Do not** touch quick-swap arc / `applyTargetKind`.

---

## Phase A — Foundation (PR 1)

**Goal:** One gate for all Outfit Stats features.

| Work | Files |
|------|--------|
| `game.outfitStats` (name TBD) bool, default Off | `settings.h`, `settings.cpp`, `settings UI` |
| `dAlbwOutfitStats_isEnabled()` | new `d_albw_outfit_stats.h/.cpp` (or `d_albw_outfit.cpp` section) |
| `dAlbwOutfitStats_getActiveOutfitKind()` wrapper → `dAlbwOutfit_getActive()` | same |
| Wolf early-out | callers skip when `checkWolf()` |

**Exit criteria:** Toggle persists; no gameplay change when Off.

---

## Phase B — Received damage + Zora grace (PR 2)

**Goal:** [Outfit Stats §1](Outfit%20Stats.md#1-received-damage-multipliers-defensive).

| Work | Files |
|------|--------|
| `dAlbwOutfitStats_getReceivedDamageMult()` | outfit_stats |
| Hook after `game.damageMultiplier`, before wolf 2× / magnetic 10× | `d_a_alink_damage.inc` `damageMagnification()` |
| Zora grace timer: enter/exit water, 180s tick | `d_a_alink_swim.inc` + Link `execute` |
| `dAlbwOutfitStats_isZoraWaterBuffActive()` | outfit_stats |

**Tests:** Ordon 4-piece → 2 hearts; Zora land 2.5× / water+grace 1.5×; Sumo 3.5×; Magic 1.0×; magnetic still 10×.

**Exit criteria:** Toggle Off = vanilla damage; On = table mults.

---

## Phase C — Swimming (PR 3 — may split C1 speed / C2 dive)

**Goal:** [Outfit Stats §3](Outfit%20Stats.md#3-swimming-speed--diving).

### C1 — Speed (low risk)

| Work | Files |
|------|--------|
| ×1.05 forward swim when toggle on | `getSwimFrontMaxSpeed()` |
| Extra ×1.10 when `checkZoraWearAbility()` | same |

### C2 — Non-Zora dive (spike first)

| Spike | Map `getZoraSwim()` / `checkZoraWearAbility()` / `procSwimDiveInit` / `swimBgCheck` / oxygen gates |
| Implement | Minimal path: surface dive → submerged proc for non-Zora when toggle on |

**Tests:** Timed swim vs vanilla; Zora ~15% total; Ordon can enter dive without Zora clothes.

**Exit criteria:** No Zora regression; iron boots / heavy armor behavior unchanged unless spec says otherwise.

---

## Phase D — Zora electric barrier (PR 4, optional)

**Goal:** [albw-zora-barrier.md](albw-zora-barrier.md).

| Work | Notes |
|------|--------|
| Hold **R1 + B** while `getZoraSwim()` + Zora armor + toggle on | Input |
| At collider + electric material + meter drain | Combat |
| FX + SE | Polish |

**Depends on:** Phase A + C1 (swim predicates stable). **Independent of** Phase E.

**Exit criteria:** Barrier doc test matrix green.

---

## Phase E — Sumo offensive kit (PR 5)

**Goal:** [Outfit Stats §2](Outfit%20Stats.md#2-sumo--offensive-kit-sumo--wooden-sword-no-shield).

| Work | Files |
|------|--------|
| `dAlbwOutfitStats_isSumoOffensiveKitActive()` | Sumo target + wood + no shield |
| ×4 outgoing sword damage | `d_cc_uty.cpp` after FA resolve |
| Wood HS unlock when kit active | `d_a_alink_cut.inc`, meter gates |
| Coordinate wood HS block with Quick-Resistance §5 | doc + code owner |

**Depends on:** Phase A; **`hiddenSkillRework` / FA enabled** for HS path.

**Tests:** Sumo+wood+no shield 4×; shield equipped disables; peel sumo disables; FA tier stacks burst check.

---

## Phase F — Integration + docs (PR 6 or continuous)

| Work | |
|------|--|
| Update Outfit Stats.md status → implemented sections | |
| Playtest matrix §6 + Quick-Sumo subset §7 | |
| ImGui debug readouts (optional) | outfit mult, grace timer, kit active |

---

## Parallel tracks (do not block Outfit Stats A–E)

| Track | Doc | When |
|-------|-----|------|
| **Quick-Resistance recovery tax** | Quick-Resistance §1 | After Quick Swap + shop storage stable |
| **Sword meter spend §5** | Quick-Resistance §5 | **After product finalizes numbers** — not provisional draft |
| **Deity shop** | albw-deity-armor-shop.md | Independent shop PR |
| **Magic Armor fixes** | Outfit Stats §6 Phase 1 | **Before** Phase A |

---

## Suggested PR sequence (summary)

| Order | PR | Est. risk |
|-------|-----|-----------|
| 0 | Magic Armor depower + visual | Low–med |
| 1 | Outfit Stats toggle + helpers | Low |
| 2 | Damage mults + Zora grace | Med |
| 3a | Swim speed | Low |
| 3b | Non-Zora dive | Med–high |
| 4 | Zora barrier (optional) | Med |
| 5 | Sumo wood kit | High |
| — | Deity shop (parallel) | High |
| — | Quick-Resistance tax + sword spend when final | Med |

---

## Milestone checklist (player-visible)

- [ ] **M1:** Magic Armor respects sub-500 depower (toggle-independent)
- [ ] **M2:** “Outfit Stats” setting appears; Off = vanilla
- [ ] **M3:** Ordon feels fragile; Hero’s/Zora water feel tougher; Sumo glass cannon
- [ ] **M4:** Everyone swims slightly faster; Zora faster still; non-Zora can dive
- [ ] **M5:** Zora R1+B electric bubble (if product ships Phase D)
- [ ] **M6:** Sumo + wooden sword striker build online

---

## Locked cross-links

| Date | Note |
|------|------|
| 2026-06-28 | Roadmap created; sword spend **not** locked (Quick-Resistance §5 provisional) |
| 2026-06-28 | Zora barrier → [albw-zora-barrier.md](albw-zora-barrier.md), Phase D optional |
