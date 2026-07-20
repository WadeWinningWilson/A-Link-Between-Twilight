# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **Gate 11c Pick Identify BUILT** — click-only `Pick identify #N` log (TP core + buffer join + optional ExtNpc mount/head/mod); HUD `ID #N …` under SEL. Gate 11b model-tight highlight still awaiting playtest. Build via `build_run.bat`; factory recovered after accidental CMake root deletes. |
| **owner_impl** | Cursor (Auto — conservative) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | Playtest: Select Mode click TP + Outset actors → grep `Pick identify`; use journal for identity pass. No drag until 11a/11b signed. |
| **perf_bar** | ~250 / 270–300 fps; editor gated by `g_levelEditorSession` |
| **do_not** | 1c-drag / terrain / 11b mesh-tint before 11a signs; Phase 2 RARC |
| **drive_in_scope** | no |
| **updated** | 2026-07-19 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x / 1x.1 | Launch + deny Save | **Done** |
| 1a | Stage Inspector | **Done** (death-clear deferred) |
| 1b | Click-to-select in world | **Gate 11c identify + 11b highlight — playtest** |

## Gate 11c (Pick Identify) — general TP + mod

- **On click only:** `Pick identify #N` in log — procId, stage name, proc/arg/params, setID, layer, pos/home/spawn, buffer join, optional mount proc/head/mod
- **HUD:** `ID #N …` under `SEL …` on screen
- **Outset:** use click journal for identity pass (pairs with `population/identity.ini`); not Outset-specific code

| 1c / 1d | Gizmo + project.json; place/delete | **Later** — session drag (`home.pos`) after 11a/11b sign |

## Gate 10 (pick accuracy) — impl landed

- **Fix:** min-`distSq` winner (not min-depth); 15px depth tie-break; hysteresis; radius 50/100; same hover+click
- **Diag:** `Pick crumb` + screen `+` pick-dot; ImGui source only on evidence
- **Bar (user):** off-center actor highlights; no flip-flop; pick-dot under cursor

## Terrain approach — evaluated vs north-star (2026-07-18)

**In-game editor = GMod model:** terrain read-only; place/move actors. Terrain edit = offline bake (Phase 2). See run doc §Terrain.

## Notes

- Deferred: death-clear highlight, Stage pane sync on world pick, flying Kargaroks, silhouette
