# Run control — Interconnected Runs traffic light

**Protocol:** [Interconnected Chats/INTERCONNECTED-RUNS.md](../Interconnected%20Chats/INTERCONNECTED-RUNS.md)

Agents: read this **before every non-trivial step**. If `mode` ≠ `running`, do not implement, build, or drive (you may answer a direct user question while `paused`).

User (or later `run-control.ps1`) is the only authority that changes `mode`.

---

| Field | Value |
|-------|--------|
| **mode** | running |
| **run_name** | WW Bridge Tool — Bridge ↔ Housing Security |
| **run_doc** | docs/Interconnected Chats/WW-Bridge-Tool-Cursor-History.md |
| **feature_state** | docs/state/ww-bridge-tool.md |
| **active_workers** | cursor,housing-security |
| **drive_in_scope** | no |
| **pause_question** | — |
| **drive_pid** | — |
| **updated** | 2026-07-19 (two-party lane; History not in this run) |

---

## Mode meanings

| mode | Workers |
|------|---------|
| **idle** | No active Interconnected Run |
| **running** | Deliberate / execute / optional drive per run doc |
| **paused** | **All stop** — user clarifying; answer questions only; kill drive if any |
| **stopped** | Run ended; do not continue; clear to idle when cleaned up |

## Commands (user)

| Command | Set |
|---------|-----|
| **Start** | `mode=running`; fill `run_name`, `run_doc`, `feature_state`, `active_workers`, `drive_in_scope` |
| **Pause** | `mode=paused`; optional `pause_question`; kill `dusklight` + clear `DUSK_DRIVE*` if drive was active |
| **Resume** | `mode=running`; clear `pause_question` |
| **Stop** | `mode=stopped` then `idle`; kill drive; clear workers |

## Active workers (examples)

- `cursor,claude` — deliberation / execute only  
- `cursor,history` — implement + archaeology/critique  
- `cursor,claude,drive` — user requested drive for that run  
- `cursor` — single-sided work still under run-control (rare)
