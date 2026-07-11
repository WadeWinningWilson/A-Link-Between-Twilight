# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **Gate 7 implemented — awaiting playtest.** Select Mode (`V` + Stage button): freeze fly-cam mouse-look, `mouse.cpp` capture gate, HUD `SELECT MODE - V to exit`. `pick.cpp` unchanged. Test mouse-cam on + off. |
| **owner_impl** | Cursor (Auto — conservative) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | Playtest click-pick → pass = 1b DONE → gizmo/mutation gate. |
| **perf_bar** | ~250 / 270–300 fps; editor gated by `g_levelEditorSession` |
| **do_not** | Phase 2 RARC; 1c before dual APPROVED |
| **drive_in_scope** | no |
| **updated** | 2026-07-10 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x / 1x.1 | Launch + deny Save | **Done** |
| 1a | Stage Inspector | **Done** (death-clear deferred) |
| 1b | Click-to-select in world | **Gate 7 implemented** — playtest Select Mode + click-pick |
| 1c / 1d | Gizmo + project.json; place/delete | Later |

## Notes

- 1b lean: SDL left-click + mapped coords + project hit-test on **live** actors; Rml UI gate (G2)
- menu_pointer alone insufficient for PC mouse — documented in run doc
- Deferred: death-clear highlight, collision-sized box, Stage pane sync on world pick
