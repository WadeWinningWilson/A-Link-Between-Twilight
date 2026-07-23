# Agent index — where live memory lives

> ⛔ **[DO-NOT.md](DO-NOT.md) — hard stops. Read it before touching BG/collision
> registration, room identity, or any surface it names. Entries are permanent
> rejections with the failure mechanism spelled out; no AI instance may
> self-approve an exception (escalate to the user).**

**Purpose:** One map for fresh Cursor/Claude chats. Prefer **live state** over reopening old sessions or reading interconnected megadocs end-to-end.

**How to use (paste opener pattern):**

> Read `docs/AGENT_INDEX.md`, then the matching `docs/state/<topic>.md`. Do the **next** item. When done, rewrite that state file’s top (status / next / do-not) — do not append a session novel.

**Interconnected Runs** (multi-agent collaboration): read [Interconnected Chats/INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) + [state/run-control.md](state/run-control.md) first. Dual sign-off before implement; drive only if the user asked.

**Live state rules:**

1. `docs/state/*.md` = **current truth** (overwrite the header fields; keep body short).
2. Long handoffs / interconnected logs = **archive / detail** — open only if the state file points there.
3. Git + code win over stale prose when they disagree.
4. Live state is **async shared memory**, not a real-time bus.
5. Active multi-agent work uses an **Interconnected Run** doc + `run-control` — not append-only diaries.

---

## Interconnected Runs

| Doc | Role |
|-----|------|
| [INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) | Protocol: brief → plans → critique → unified plan → **both sign off** → execute; optional drive |
| [_RUN_TEMPLATE.md](Interconnected%20Chats/_RUN_TEMPLATE.md) | Copy per run into `Interconnected Chats/<Run-Name>.md` |
| [Cut-Actors-Demo-Restore-Cursor-History.md](Interconnected%20Chats/Cut-Actors-Demo-Restore-Cursor-History.md) | **Active run** — cut actors / Demo restore; History archaeology + Cursor impl |
| [Level-Editor-Cursor-Claude.md](Interconnected%20Chats/Level-Editor-Cursor-Claude.md) | Prior run — level editor Phase 1; superseded for run-control while Cut Actors run is active |
| [state/run-control.md](state/run-control.md) | idle / running / paused / stopped — user Start/Pause/Resume/Stop |

**Deliberation (short):** each instance proposes a plan → each critically answers the other → unified plan → **both APPROVED** → then implement. Not simultaneous. Drive is **not** inherent — only if the user requests it for that run.

---

## Live state (start here)

| Topic | Live state | Detail / archive (only if needed) |
|-------|------------|-----------------------------------|
| Run control (traffic light) | [state/run-control.md](state/run-control.md) | [INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) |
| Enemy Death Rupees | [state/enemy-death-rupees.md](state/enemy-death-rupees.md) | `albw-port.md` (setting overview) |
| WW itemmdl / Wind Curs↔Clau | [state/ww-itemmdl.md](state/ww-itemmdl.md) | [wind-waker-item-work.md](wind-waker-item-work.md), [Interconnected Chats/Wind Curs-Wind Clau.md](Interconnected%20Chats/Wind%20Curs-Wind%20Clau.md) (archive) |
| Field combat refinements | [state/combat-refinements.md](state/combat-refinements.md) | [combat-refinements-handoff.md](combat-refinements-handoff.md) |
| Boss fights | [state/boss-fights.md](state/boss-fights.md) | [boss-fights-handoff.md](boss-fights-handoff.md), [Boss-Fights-RefinedGohma.md](Boss-Fights-RefinedGohma.md) |
| Drive / FPS oracle | [state/drive-fps.md](state/drive-fps.md) | [performance-handoff.md](performance-handoff.md), [build-fps-guidelines.md](build-fps-guidelines.md), [future-performance-leaning.md](future-performance-leaning.md) |
| Build / FPS review | [state/drive-fps.md](state/drive-fps.md) | [build-fps-guidelines.md](build-fps-guidelines.md) (has its own doc map) |
| Shield / parry | — | [shield-combat.md](shield-combat.md) |
| ALBW port overview | — | [albw-port.md](albw-port.md) |
| Cut Actors / Demo Restore | [state/cut-actors-demo-restore.md](state/cut-actors-demo-restore.md) | [Interconnected Chats/Cut-Actors-Demo-Restore-Cursor-History.md](Interconnected%20Chats/Cut-Actors-Demo-Restore-Cursor-History.md), [TPHistory.md](TPHistory.md) §7 |
| Level Editor (Phase 1) | [state/level-editor.md](state/level-editor.md) | [level-editor-phase1.md](level-editor-phase1.md), [Interconnected Chats/Level-Editor-Cursor-Claude.md](Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| Outfit / sumo / quick-swap | *(add `state/outfit-sumo.md` when next session starts)* | [Interconnected Chats/Quick-Sumo Work.md](Interconnected%20Chats/Quick-Sumo%20Work.md), [Outfit Stats.md](Outfit%20Stats.md), [sumo-combat.md](sumo-combat.md) |
| Quick-equip wheel + page 2 | [state/quick-equip-wheel.md](state/quick-equip-wheel.md) | `~/.cursor/plans/next_slice_shop_potion_stagger.plan.md` (original hold sketch); [deku-leaf-glide-research.md](deku-leaf-glide-research.md) P4; [d-pad-reworking.md](d-pad-reworking.md) |
| Ext Status (Collect sibling) | [state/ext-start-status.md](state/ext-start-status.md) | Tools/Quest/Atlas; `ext_inv/claims.ini`; №103 |
| Fado Ordon door unlock + warp sink | [state/fado-door-warp.md](state/fado-door-warp.md) | cut-actors №101; `d_a_door_knob00`; `d_ext_mod_flags` |
| Companion mods (separate release) | — | [research/albt-companion-mods-research.md](research/albt-companion-mods-research.md) |
| Mod API port (main `.dusk` extraction) | [state/mod-api-port.md](state/mod-api-port.md) | [state/mod-api-port-albw-meter.md](state/mod-api-port-albw-meter.md) (#1 plan); port canvases; `albw-port.md` |

---

## Always-on process (any feature)

| Doc | When |
|-----|------|
| [building.md](building.md) + [build-fps-guidelines.md](build-fps-guidelines.md) | Build / launch / FPS hygiene |
| [code-conventions.md](code-conventions.md) | `#if TARGET_PC`, Dusk markers |
| [commit-and-push.md](commit-and-push.md) | Commits / push (only when asked) |

**Default build:** `build_run.bat` → `build/windows-msvc-relwithdebinfo/dusklight.exe`.

---

## Note: async collaboration + user control

Two agents share **run docs** + **live state**; they do not message each other live. The **user** starts turns, pauses (freezes all workers + drive), resumes, and stops via `run-control`.

**Ideal Interconnected Run:** user brief → dual plans → critical responses → unified plan with **both sign-offs** → implement → optional drive if requested → user clarifies only when needed.
