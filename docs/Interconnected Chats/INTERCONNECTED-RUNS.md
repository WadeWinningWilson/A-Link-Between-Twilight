# Interconnected Runs — protocol

**Purpose:** How two (or more) AI instances collaborate under user control, using a per-run doc in `docs/Interconnected Chats/` plus live state and optional drive.

**Index:** [AGENT_INDEX.md](../AGENT_INDEX.md) · **Control:** [state/run-control.md](../state/run-control.md) · **Template:** [_RUN_TEMPLATE.md](_RUN_TEMPLATE.md)

---

## What an Interconnected Run is

A **named, user-started collaboration** between AI instances (typically Cursor + Claude, or two Cursor chats) on one feature or problem.

| Piece | Role |
|-------|------|
| **User brief** | Goals, guidelines, expectations — stated **before** the run starts |
| **Run doc** | One file under `docs/Interconnected Chats/` for *this* run (plans, critiques, sign-offs, decisions) |
| **Live state** | `docs/state/<topic>.md` — short current truth (status / next / do-not) |
| **Run control** | `docs/state/run-control.md` — idle / running / paused / stopped |
| **Drive** | **Optional** — only if the user asks; see [state/drive-fps.md](../state/drive-fps.md) |

Legacy interconnected diaries (e.g. Wind Curs↔Clau) are **archives**. New work uses this protocol + a fresh run doc (or a clearly labeled new section that follows the template).

---

## User control (authoritative)

| Command | Effect |
|---------|--------|
| **Start** | User fills brief → sets `run-control` to `running` → opens/pastes into both chats |
| **Pause** | `paused` + optional question → **all workers stop** (impl chats + drive if any). User may ask clarifying questions. |
| **Resume** | `running` again; agents continue only from signed-off plan + live state |
| **Stop** | `stopped` then `idle`; kill drive if active; do not continue the run |

Agents **never** start, pause, or stop a run on their own. They only read `run-control` and obey it.

Before every non-trivial step: if `mode` ≠ `running` → do not implement, build, or drive (you may answer a direct user question while paused).

---

## Deliberation protocol (required)

Instances communicate **asynchronously**, not simultaneously. Turn order is strict.

### Phase A — Independent plans

1. Each instance writes its own **Proposed plan** into the run doc (labeled by role, e.g. `Cursor:` / `Claude:`).
2. No instance implements code in this phase.
3. User (or runner) ferries: “Instance B, read Instance A’s plan and respond.”

### Phase B — Critical responses

1. Each instance reads the **other’s** plan.
2. Writes a **Critical response**: what holds, what fails, risks, missing constraints, better alternatives.
3. Judge with a **critical eye** — agreement is not the default; rubber-stamping is a protocol violation.

### Phase C — Unified plan

1. One instance (or each, then merge) drafts a **Unified plan** that absorbs valid critique.
2. **Both instances must explicitly sign off** on the unified plan in the run doc:

```markdown
| Role | Sign-off | When |
|------|----------|------|
| Cursor | APPROVED / REQUEST CHANGES | … |
| Claude | APPROVED / REQUEST CHANGES | … |
```

3. If either says **REQUEST CHANGES** → back to Phase B/C until both **APPROVED**.
4. **No implementation** until both sign-offs are **APPROVED** (unless the user explicitly overrides in the brief).

### Phase D — Execute (only after dual sign-off)

1. Roles follow the unified plan (e.g. Cursor implements, Claude reviews diffs/screenshots).
2. Update `docs/state/<topic>.md` when status/next changes — **overwrite the tip**, don’t append novels to the run doc for routine progress.
3. Significant decisions / rejected paths → short note in the run doc (keep it scannable).

### Phase E — Optional drive

Only if the user asked for a drive session in the brief (or mid-run):

1. `run-control` lists `drive` in `active_workers`.
2. Follow [state/drive-fps.md](../state/drive-fps.md) / performance-handoff drive protocol.
3. Drive does **not** participate in plan deliberation; it only validates after execute (or when user orders a check).
4. Pause/Stop kills the game process and clears `DUSK_DRIVE*` per existing hygiene.

---

## Communication rules

| Do | Don’t |
|----|--------|
| Write plans/critiques/sign-offs in the **run doc** | Assume the other chat “heard” you without a written turn |
| Wait for the other’s critical response before unifying | Plan and implement in one breath |
| Critique seriously; cite risks and alternatives | Rubber-stamp to go faster |
| Require **both** APPROVED before code | Ship on one-sided approval |
| Obey pause/stop immediately | Keep building while `paused` |
| Prefer live state for “what’s next” | Append megadoc session diaries as the tip |

User ferries turns (paste openers / “your turn”) unless a Phase-1 runner script exists later.

---

## Starting a run (user checklist)

1. Copy [_RUN_TEMPLATE.md](_RUN_TEMPLATE.md) → `docs/Interconnected Chats/<Run-Name>.md`
2. Fill **Goals / Guidelines / Expectations** (and whether drive is in scope)
3. Name roles (who implements, who reviews)
4. Point at live state topic (create `docs/state/<topic>.md` if missing)
5. Set [run-control.md](../state/run-control.md) → `running`, `run_doc`, `feature`, `active_workers`
6. Paste openers into each chat (template includes them)

---

## Relationship to older docs

| Old pattern | New pattern |
|-------------|-------------|
| Append-only interconnected megadoc as memory | Per-run doc for deliberation + `docs/state/` for tip |
| Informal “Cursor implements, Claude reviews” | Same roles **plus** dual sign-off before implement |
| Drive always / never | Drive **only if user asks** for that run |
| Chat OOM → handoff novel | Live state + short run-doc decisions |

Archives stay readable; new runs don’t grow them as the primary bus.
