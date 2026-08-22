# Boss fights — live state

| Field | Value |
|-------|--------|
| **status** | Boss Refinement Layer A + Armogohma; Diababa v1 done; Fyrus §9+§10 wired; **Morpheel P1 bubbled eye-mass construction live** |
| **owner_impl** | Cursor |
| **owner_review** | — |
| **next** | **Morpheel:** Lakebed playtest under Boss Refinement On ([§8](../Boss-Fights-Morpheel-research.md)). Then HP HUD. **Fyrus:** Mines playtest [§9e](../Boss-Fights-Fyrus-research.md#9e-playtest-checklist--remaining-2026-08-19) after. |
| **do_not** | Look up bosses by C++ filename — use `fopAcM_GetName()` / `fpcNm_*`; put drive code in `src/`; revert features to “fix” FPS |
| **playtest_combo** | Boss Refinement **On**, Boss Health Bars **On**, Boss HP × as needed |
| **updated** | 2026-08-19 — Morpheel §8 after vanilla intro → COMPOSITE |
| **detail** | [boss-fights-handoff.md](../boss-fights-handoff.md) — multi-boss archive |
| **diababa** | [Boss-Fights-RefinedDiababa.md](../Boss-Fights-RefinedDiababa.md) |
| **armogohma** | [Boss-Fights-RefinedGohma.md](../Boss-Fights-RefinedGohma.md) |
| **fyrus** | [Boss-Fights-Fyrus-research.md](../Boss-Fights-Fyrus-research.md) — §9e playtest, §8e whip paused |
| **morpheel** | [Boss-Fights-Morpheel-research.md](../Boss-Fights-Morpheel-research.md) — **§8 construction** (Refinement only) |
| **stallord** | [Boss-Fights-Stallord-research.md](../Boss-Fights-Stallord-research.md) — 4 orphan BCKs |

---

## Three layers (short)

1. **Boss Refinement** (`game.bossRefinement`) — any-sword gates, Armogohma pacing/warp, Diababa 70% phase, **Morpheel bubbled P1**
2. **Boss HP HUD** (`game.bossHealthBars`) — LoP bar from lock-on HP display  
3. **HP mult** (`game.hpMultBoss` / `hpMultMidBoss`) — true max HP on init  

## Build / handoff hygiene

- Build: `build_run.bat` only  
- Before coding: [build-fps-guidelines.md](../build-fps-guidelines.md), [commit-and-push.md](../commit-and-push.md)  
- After build: drive/FPS check via [state/drive-fps.md](drive-fps.md) when validating stability  
- Never commit `local_dev_backup/`, drive sources, `albw_*_debug.txt`
