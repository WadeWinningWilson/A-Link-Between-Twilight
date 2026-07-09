# Boss fights — live state

| Field | Value |
|-------|--------|
| **status** | Boss Refinement Layer A + Armogohma work in handoff; Diababa in progress; Layer B (Zant/Ganon) future |
| **owner_impl** | Cursor |
| **owner_review** | — |
| **next** | Continue from handoff “implemented / in progress” sections; Armogohma detail → RefinedGohma doc; Diababa RUNAWAY arrow test |
| **do_not** | Look up bosses by C++ filename — use `fopAcM_GetName()` / `fpcNm_*`; put drive code in `src/`; revert features to “fix” FPS |
| **playtest_combo** | Boss Refinement **On**, Boss Health Bars **On**, Boss HP × as needed |
| **updated** | from boss-fights-handoff (refresh on next session) |
| **detail** | [boss-fights-handoff.md](../boss-fights-handoff.md) — full archive |
| **armogohma** | [Boss-Fights-RefinedGohma.md](../Boss-Fights-RefinedGohma.md) |

---

## Three layers (short)

1. **Boss Refinement** (`game.bossRefinement`) — any-sword gates, Armogohma pacing/warp, Diababa plans  
2. **Boss HP HUD** (`game.bossHealthBars`) — LoP bar from lock-on HP display  
3. **HP mult** (`game.hpMultBoss` / `hpMultMidBoss`) — true max HP on init  

## Build / handoff hygiene

- Build: `build_run.bat` only  
- Before coding: [build-fps-guidelines.md](../build-fps-guidelines.md), [commit-and-push.md](../commit-and-push.md)  
- After build: drive/FPS check via [state/drive-fps.md](drive-fps.md) when validating stability  
- Never commit `local_dev_backup/`, drive sources, `albw_*_debug.txt`
