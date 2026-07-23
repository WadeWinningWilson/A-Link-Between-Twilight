# Mod API port — live state

| Field | Value |
|-------|--------|
| **status** | First 10 logged; **#1 `albw-meter` extraction planning started** |
| **target** | TwilitRealm `origin/main` Mod SDK (`.dusk` + services) |
| **owner_impl** | — |
| **next** | #1 P0 in an **external** mod repo (not this tree); host = stock main — see [mod-api-port-albw-meter.md](mod-api-port-albw-meter.md) D3 |
| **do_not** | Ship WW donor bytes as `.dusk`; split meter-spenders into separate mods; mid-WIP rewrite of Diababa/Shade AI onto ABI; big-bang merge `origin/main` |
| **updated** | 2026-07-22 |
| **detail** | [mod-api-port-albw-meter.md](mod-api-port-albw-meter.md) (#1 plan), feasibility/comparison canvases, `docs/albw-port.md` |

---

## Bundle rule (locked)

**Any feature that currently spends ALBW meter ships inside the ALBW Meter mod (#1).**  
It may expose Config toggles, but it is not a standalone `.dusk`.

Meter spend surfaces today include (non-exhaustive): sword / HS / sidestep / roll / back-jump / bombs / arrows / sling / boom / spinner / iron ball / hookshot / Dom Rod / armor hit / Deku Leaf glide; shield fail-block penalties; FA/Flurry spend path (ALBW suppress); lockout perk deploy costs/refills that assume the meter pool.

Grants that only *refill* meter (parry reward, magic pickups) still live in #1 when they are part of those systems — do not fork a second meter economy.

Wolf combat is **outside** the ALBW meter (own charge bank) → allowed as its own mod.

---

## First 10 immediate port candidates

Ordered for extraction (easiest independent value first after the mandatory meter suite is named).

| # | Package id (proposed) | Contents | Tier | Why first | Notes |
|---|----------------------|----------|------|-----------|-------|
| **1** | `albw-meter` | **Energy meter + lockout** + **all meter spenders**: HS rework / `d_albw_combat`, Focused Arts, Flurry Rush, Shield parry/bash (+ durability/manual as same module), Deku Leaf meter costs, MQ **stamina** shop tiers, meter/FA skill scrolls, meter HUD draw (incl. LoP pieces required to show the bar) | C spine / B combat | Mandatory bundle; spine of ALBW | Largest package. Do **not** ship Flurry/FA/HS/Shield as separate mods. Outfit wardrobe recovery tax stays with outfits (later) unless it calls spend APIs directly. |
| **2** | `enemy-death-rupees` | Kill/fight-victory wallet credit + `+n` popup | B | Smallest isolated combat economy win | `game.enemyDeathRupees`; no meter spend |
| **3** | `hp-mult` | Enemy category HP × + Link damage decrease | B | Config sliders; shared HP query for #4 | Keep lock-on HP helper API stable for boss bar |
| **4** | `boss-health-bars` | LoP-style boss name + HP bar | B | Depends on #3’s HP query if possible | `game.bossHealthBars` |
| **5** | `boss-refinement` | Sword gates, Armogohma pacing/phases, Diababa when ready | B | High player value; Layer-B reveal models optional later | Reveal BMD needs Layer B host or whole-arc Overlay |
| **6** | `wolf-combat` | Bite charges, Midna arts/arm, stun split, charge HUD | B | Explicitly **outside** ALBW meter | Do not import meter spend APIs |
| **7** | `hold-a-crawl` | Hold-A ≥1s crawl without stealing door/talk/roll | B | Tiny QoL; one alink path | `game.enableHoldACrawl` |
| **8** | `death-recovery-orb` | Wallet half on death; Tear orb returns half | B | Self-contained game-over + room spawn | `game.deathRecoveryOrb`; F_0625 gate |
| **9** | `junior-postman-mail` | Phase-0 letter + North Faron deliver spawn | B | Story onboarding; save bits 812/813 | Keep test bypass editor-only |
| **10** | `postman-rental` | Death item strip + Postman shop + native dialogue/shop UI + Oocoo + MQ **heart** tiers (not stamina) | B | Full rental economy without meter spenders | Stamina MQ rows stay in **#1**. Clothes/outfit grants may stub until outfit mod exists |

---

## Explicitly not in the first 10

| Feature | Reason |
|---------|--------|
| Outfit / Sumo / Cap / Wardrobe / Quick Swap | Tier C (save bits, remount); recovery tax couples meter later |
| Quick Equip / Ext Status | Tier C menu ownership |
| True ALBW bootstrap | Tier C; unlocks many systems |
| Shade Refuge / Shade Boss | WIP / larger; after #4–5 patterns exist |
| Layer A/texture cosmetics | Tier A anytime — separate cosmetic packs, not ALBW feature #1–10 |
| ExtNpc / WW receivers | Pilot after boundary spec; content = bridge recipe only (**X**) |
| Ext Seq / audio shadow / Layer B / itemmdl / level editor | Tier C/D |

---

## Dependency sketch (first 10)

```
albw-meter (#1)  ←── no soft deps on #2–10
     │
     │  (optional later: wardrobe recovery tax → outfits)
     │
enemy-death-rupees (#2)
hp-mult (#3) ──► boss-health-bars (#4)
boss-refinement (#5) ── prefers #3/#4
wolf-combat (#6)
hold-a-crawl (#7)
death-recovery-orb (#8) ── may interact with postman-rental (#10) unlock gates
junior-postman-mail (#9) ── Postman actor shared with #10
postman-rental (#10)
```

---

## Extraction checklist (when a candidate starts)

1. New `.dusk` from mod-template; `FEATURES game` unless pure overlay.
2. ConfigService keys under `mod.<id>.*` (mirror today’s `game.*` where possible).
3. No WW literals / donor bytes in the DLL or redistributable pack.
4. `mod_shutdown` safe (scene change OK if documented).
5. GameService epoch: avoid new TARGET_PC fields on shared structs when possible.
6. Update this file’s **status** / **next** when work starts or ships.
