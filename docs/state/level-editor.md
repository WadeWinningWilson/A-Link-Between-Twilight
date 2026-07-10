# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **1a DONE (playtested).** → **RECOMMEND COMMIT CHECKPOINT** (all uncommitted since 8924ceb7d3: 1x.1 + gates 1–6 + 1a + highlight + grouping) — user call, don't push. **1b (click-pick, zero mutation): Claude Phase A + critique + unified posted, APPROVED** — SDL/screen coords in `mDoLib_project` space; unified UI gate incl **ImGui WantCaptureMouse**; front-most depth tie-break; same `sSelection` as list. Awaiting Cursor concurrence → dual APPROVED → no 1b code till then. |
| **owner_impl** | Cursor (Auto — conservative) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | Claude Phase A/B on 1b → dual APPROVED → implement pick |
| **perf_bar** | ~250 / 270–300 fps; editor gated by `g_levelEditorSession` |
| **do_not** | Implement 1b before dual APPROVED; Phase 2 RARC; 1c before dual APPROVED |
| **drive_in_scope** | no |
| **updated** | 2026-07-09 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x / 1x.1 | Launch + deny Save | **Done** |
| 1a | Stage Inspector | **Done** (death-clear deferred) |
| 1b | Click-to-select in world | **Phase A posted** — deliberation |
| 1c / 1d | Gizmo + project.json; place/delete | Later |

## Notes

- 1b lean: SDL left-click + mapped coords + project hit-test on **live** actors; Rml UI gate (G2)
- menu_pointer alone insufficient for PC mouse — documented in run doc
- Deferred: death-clear highlight, collision-sized box, Stage pane sync on world pick
