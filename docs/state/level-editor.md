# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **1x landed + playtested** (2026-07-09). Next: **1x.1** save deny UX and/or **1a** Stage Inspector |
| **owner_impl** | Cursor (when opened) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | User Start → dual plans → critique → unified plan → both APPROVED → implement 1x.1 and/or 1a |
| **do_not** | Phase 2 RARC packer; DZB authoring; editor cost on normal Play; trust stale “no code yet” header over §10 |
| **drive_in_scope** | no (unless user asks) |
| **updated** | 2026-07-09 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x | Launch mode + session flag + save block | **Done** (playtest §10.4) |
| 1x.1 | Deny Save at selection + `Z2SE_SYS_ERROR` | Planned (§10.5) |
| 1a | Stage Inspector (enumerate + UI + highlight) | Next major (§4) |
| 1b+ | Click-select, gizmo, place/delete, DZB viz | Backlog |

## Security throughline (non-negotiable)

- `enableLevelEditor` requires `enableAdvancedSettings`; speedrun forces off.
- `g_levelEditorSession` — session only, not saved; all editor paths early-out when false.
- Level Projects on disk for edits; native saves blocked / denied in editor session.

## Key modules (expected)

- New: `src/dusk/leveledit/` (1a+)
- Touch: `settings.*`, `prelaunch.*`, `m_Do_main`, `autosave`, `d_menu_save`, `d_menu_window` / collect (1x.1)
- Read pillar: `d_stage.cpp` buffer re-walk — confirm resident resource key early (§8)
