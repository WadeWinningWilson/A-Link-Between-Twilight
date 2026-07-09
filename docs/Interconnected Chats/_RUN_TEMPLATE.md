# Interconnected Run — TEMPLATE

Copy to `docs/Interconnected Chats/<Run-Name>.md` and fill before **Start**.

Protocol: [INTERCONNECTED-RUNS.md](INTERCONNECTED-RUNS.md) · Control: [../state/run-control.md](../state/run-control.md) · Index: [../AGENT_INDEX.md](../AGENT_INDEX.md)

---

## Run header

| Field | Value |
|-------|--------|
| **run_name** | |
| **created** | YYYY-MM-DD |
| **live_state** | `docs/state/<topic>.md` |
| **roles** | e.g. Cursor = implement · Claude = review |
| **drive_in_scope** | **no** / **yes** (only if user asks) |
| **status** | briefing → deliberating → signed-off → executing → done / stopped |

---

## User brief (required before Start)

### Goals

<!-- What success looks like. Be concrete. -->

### Guidelines

<!-- Constraints: build rules, do-not list, files to touch/avoid, settings gates, etc. -->

### Expectations

<!-- Quality bar, playtest bar, whether dual sign-off can be waived, review depth, commit policy. -->

### Drive (optional)

- [ ] Not requested  
- [ ] Requested — notes: (scene, PASS criteria, link [drive-fps](../state/drive-fps.md))

---

## Deliberation log

### Phase A — Proposed plans

#### Cursor — proposed plan

…

#### Claude — proposed plan

…

### Phase B — Critical responses

#### Claude — critique of Cursor’s plan

…

#### Cursor — critique of Claude’s plan

…

### Phase C — Unified plan

…

| Role | Sign-off | Notes | When |
|------|----------|-------|------|
| Cursor | APPROVED / REQUEST CHANGES | | |
| Claude | APPROVED / REQUEST CHANGES | | |

**Implementation gate:** both rows must be **APPROVED** before Phase D (unless user override in Expectations).

### Phase D — Execute notes (short)

<!-- Decisions, blockers, links to commits/PRs. Routine progress → live state tip only. -->

### Phase E — Drive results (if in scope)

| Dive / label | Result | Notes |
|--------------|--------|-------|
| | PASS / FAIL | |

---

## Paste openers

**Cursor (or implementer):**

> Interconnected Run: **\<run_name\>**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, this run doc, `docs/state/run-control.md`, and `docs/state/<topic>.md`. Obey run-control (no work unless `running`). Follow deliberation phases — do not implement until both sign-offs are APPROVED. Drive only if this run’s brief says so.

**Claude (or reviewer):**

> Interconnected Run: **\<run_name\>**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, this run doc, `docs/state/run-control.md`, and `docs/state/<topic>.md`. Your job is critical review and plan co-ownership. Critique seriously; sign off only when the unified plan is sound. No source edits unless the brief says otherwise. Obey pause/stop.

**After pause / resume:**

> Continue Interconnected Run **\<run_name\>**. Re-read `run-control` + run doc sign-off table + live state. Resume only if `mode=running` and dual APPROVED (or user override).
