# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **Gate 8e — pick feel polish landed. YOUR PLAYTEST.** Capsule hit (feet→head), larger radius, amber **HOVER** preview in Select Mode shows what a click will grab. |
| **owner_impl** | Cursor (Auto — conservative) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | Retest Select Mode: amber HOVER follows cursor over bodies; click should feel less finicky. |
| **perf_bar** | ~250 / 270–300 fps; editor gated by `g_levelEditorSession` |
| **do_not** | Blind tweaks without new logs; Phase 2 RARC; 1c before dual APPROVED |
| **drive_in_scope** | no |
| **updated** | 2026-07-17 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x / 1x.1 | Launch + deny Save | **Done** |
| 1a | Stage Inspector | **Done** (death-clear deferred) |
| 1b | Click-to-select in world | **Gate 8e** — hover + capsule; retest |
| 1c / 1d | Gizmo + project.json; place/delete | Later |

## Gate 8c changes

- Align `mDoLib_project` Y into gInf (match click map)
- Multi-point hit (feet / +80cm / eyes); radius 80px default, 140px Link/Horse/`Obj_Uma`
- Pick **Link** + **Horse** as LIVE snapshots (not PLYR spawn rows)

## Playtest

1. Fly Cam On → PC Hotkeys On → Select Mode On → hide Editor  
2. Click close Bokoblin, then Link, then Epona  
3. Expect cyan highlight + `Pick hit` in log  

## Notes

- Deferred: death-clear highlight, collision-sized box, Stage pane sync on world pick
