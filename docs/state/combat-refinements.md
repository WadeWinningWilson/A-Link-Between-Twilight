# Combat refinements — live state

| Field | Value |
|-------|--------|
| **status** | Windup targeting: Bokoblin landed (needs playtest). FA tier API + test settings shipped. Flurry Rush Phases 1–5 + snap landed |
| **owner_impl** | Cursor |
| **owner_review** | — |
| **next** | Playtest Bokoblin windup matrix; then roll pattern to Stalfos / Dynalfos / etc. FA: combat hooks + shop rows + HUD. Flurry: Phase 6+ per flurry brief |
| **do_not** | Full-animation homing; treat Boss Refinement as cause of field whiffs; strip Hidden Skills via FA |
| **updated** | 2026-06 (from handoff; refresh on next session) |
| **detail** | [combat-refinements-handoff.md](../combat-refinements-handoff.md) — full backlog, FA design, file maps |

---

## Priority queue

| # | Topic | Status |
|---|--------|--------|
| 1 | Enemy windup targeting | Bokoblin (`E_OC`) in; playtest then roll out |
| 2 | Focused Arts | Tier API + test settings shipped; hooks/shop/HUD next |
| 3 | Flurry Rush | Phases 1–5 + snap; Phase 6+ → [albw-flurry-rush-brief.md](../albw-flurry-rush-brief.md) |

## Windup design rule (short)

Extend soft tracking toward ~75% of windup (or last frame before hitbox) — **not** through the whole attack. Soft `cLib_addCalcAngleS`, stop when hitbox active.

## Playtest (Bokoblin — gate before roll-out)

1. Stand still, block — vanilla feel  
2. Z-target strafe — slight improvement OK  
3. Hold ZR, no Z-target, circle — fewer whiffs, still dodgeable  
4. Late dodge in commit — intentional whiff  

Build: `build_run.bat`.

## Related

- Boss layer: [state/boss-fights.md](boss-fights.md)  
- Shield: [shield-combat.md](../shield-combat.md)
