# Level Editor — Cursor ↔ Claude

**Interconnected Run** — in-game Twilight Princess level editor (Phase 1).

| | |
|---|---|
| **Protocol** | [INTERCONNECTED-RUNS.md](INTERCONNECTED-RUNS.md) |
| **Canonical design** | [level-editor-phase1.md](../level-editor-phase1.md) — keep for revisions; do not treat as live tip |
| **Live state** | [state/level-editor.md](../state/level-editor.md) |
| **Run control** | [state/run-control.md](../state/run-control.md) |
| **Roles** | **Claude** = design continuity / critical review · **Cursor** = implement + build (when opened) |
| **Drive** | **Not in scope** unless user asks mid-run |
| **Status** | briefing ready — awaiting user **Start** + dual deliberation before next code |

---

## User brief

### Goals

1. Ship Phase 1 of the in-game level editor on dusklight foundations (native renderer, actor DB, Aurora overlay path for later bake).
2. **Immediate next work** (from design doc §10.5 / §4):
   - **1x.1** — deny Save at selection in editor sessions (buzzer, no misleading “Saved” UX); keep §10.3 commit guards as backstop.
   - **1a** — Stage Inspector: enumerate placed actors, Stage tab UI, in-world highlight — **zero mutation**.
3. Preserve the security throughline: editor is a distinct launch mode; **zero cost** on normal `Play` (`!g_levelEditorSession` early-outs).

### Guidelines

- Build: `build_run.bat` → RelWithDebInfo only; wipe GPU caches after builds; follow [build-fps-guidelines.md](../build-fps-guidelines.md) + [commit-and-push.md](../commit-and-push.md).
- Re-verify line numbers in [level-editor-phase1.md](../level-editor-phase1.md) §7 / §10 before editing (they drift).
- `#if TARGET_PC` + project comment markers on new/modified code.
- Do **not** start the RARC/YAZ0 packer (Phase 2) or DZB authoring in this run unless the brief is amended.
- Do **not** put editor work on the normal-play hot path.
- Storage model stays **Level Projects** on disk (`level_projects/`), not per-save — see design §3.
- Push only when user asks; push to **upstream** (ALBW-Dusklight), not origin.

### Expectations

- **Dual sign-off required** before implementation of 1x.1 and of 1a (separate unified plans OK if sequenced).
- Critique seriously — rubber-stamping is a protocol violation.
- Claude may already have context from prior solo work; Cursor must still read live state + this run doc + design §4/§10, not rely on chat memory.
- Playtest bar for 1x (already passed per design §10.5): leave regression checks in place when touching save/prelaunch.
- Drive session: **not requested** for this run.

### Drive

- [x] Not requested  
- [ ] Requested — (fill if user asks later)

---

## Current picture (from design doc — refresh via live state)

| Milestone | State |
|-----------|--------|
| Research / pillars | Done |
| **1x** — `enableLevelEditor`, launch button, `g_levelEditorSession`, save block | **Implemented + playtested 2026-07-09** (§10.4 all six checks pass) |
| **1x.1** — deny Save at selection + buzzer | **PLANNED** (§10.5) — next UX polish |
| **1a** — Stage Inspector | **Next major build** (§4) — no mutation |
| 1b–1e, Phase 2 packer | Backlog — out of this run unless brief expands |

**Stale header note:** design doc top still says “NO code written yet” — that is outdated relative to §10; trust §10 + live state + git for 1x status.

---

## Deliberation log

### Phase A — Proposed plans

#### Claude — proposed plan

*(Claude: write first when run Starts — recommend order 1x.1 vs 1a, file touch list, risks.)*

#### Cursor — proposed plan

*(Cursor: write when opened — independent plan from design §4 / §10.5; do not copy Claude blindly.)*

### Phase B — Critical responses

#### Cursor — critique of Claude’s plan

…

#### Claude — critique of Cursor’s plan

…

### Phase C — Unified plan

…

| Role | Sign-off | Notes | When |
|------|----------|-------|------|
| Cursor | APPROVED / REQUEST CHANGES | | |
| Claude | APPROVED / REQUEST CHANGES | | |

**Implementation gate:** both **APPROVED** before Phase D.

### Phase D — Execute notes (short)

<!-- Decisions / blockers only. Routine progress → live state tip. -->

### Phase E — Drive results

N/A unless user enables drive.

---

## Paste openers

**Claude (review / design continuity):**

> Interconnected Run: **Level Editor — Cursor ↔ Claude**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, this run doc, `docs/state/run-control.md`, `docs/state/level-editor.md`, and `docs/level-editor-phase1.md` (§4, §10, §10.5). Obey run-control. Propose or critique plans critically; dual APPROVED before Cursor implements. Drive not in scope unless the brief changes. No rubber-stamping.

**Cursor (implementer — when you open the instance):**

> Interconnected Run: **Level Editor — Cursor ↔ Claude**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, this run doc, `docs/state/run-control.md`, `docs/state/level-editor.md`, and `docs/level-editor-phase1.md` (§4 Stage Inspector, §10.5 save UX, §7 code map — re-verify lines). Obey run-control (`running` only). Write your own Phase A plan; wait for critical exchange and dual APPROVED before coding. Build with `build_run.bat`. Drive not in scope unless asked.

**After pause / resume:**

> Continue Interconnected Run **Level Editor — Cursor ↔ Claude**. Re-read `run-control`, this doc’s sign-off table, and `docs/state/level-editor.md`. Resume only if `mode=running` and dual APPROVED (or user override).
