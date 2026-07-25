# Boss fights — live state

| Field | Value |
|-------|--------|
| **status** | Boss Refinement Layer A + Armogohma; **Diababa Refinement v1 finished**; Fyrus research ready — **§8 50%→15% B_GO window locked** (not coded); Layer B future |
| **owner_impl** | Cursor |
| **owner_review** | — |
| **next** | Implement Fyrus §8 (stun + PUTOUT + B_GO shared pool 50→15%) |
| **do_not** | Look up bosses by C++ filename — use `fopAcM_GetName()` / `fpcNm_*`; put drive code in `src/`; revert features to “fix” FPS |
| **playtest_combo** | Boss Refinement **On**, Boss Health Bars **On**, Boss HP × as needed |
| **updated** | 2026-07-24 — Fyrus §8 B_GO window + whip/blast/Keese research |
| **detail** | [boss-fights-handoff.md](../boss-fights-handoff.md) — multi-boss archive |
| **diababa** | [Boss-Fights-RefinedDiababa.md](../Boss-Fights-RefinedDiababa.md) |
| **armogohma** | [Boss-Fights-RefinedGohma.md](../Boss-Fights-RefinedGohma.md) |
| **fyrus** | [Boss-Fights-Fyrus-research.md](../Boss-Fights-Fyrus-research.md) — §3 attacks, §7 fire-off/PUTOUT, §8 50→15% B_GO |

---

## Three layers (short)

1. **Boss Refinement** (`game.bossRefinement`) — any-sword gates, Armogohma pacing/warp, Diababa 70% phase  
2. **Boss HP HUD** (`game.bossHealthBars`) — LoP bar from lock-on HP display  
3. **HP mult** (`game.hpMultBoss` / `hpMultMidBoss`) — true max HP on init  

## Build / handoff hygiene

- Build: `build_run.bat` only  
- Before coding: [build-fps-guidelines.md](../build-fps-guidelines.md), [commit-and-push.md](../commit-and-push.md)  
- After build: drive/FPS check via [state/drive-fps.md](drive-fps.md) when validating stability  
- Never commit `local_dev_backup/`, drive sources, `albw_*_debug.txt`
