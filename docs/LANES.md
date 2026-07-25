# Lanes — who does what

> **Source of truth.** The lane split is a **user ruling** (data plumbing → Cursor;
> judgment / language work → Claude). This file *shelves* that roster so fresh instances can
> discover it; it does not invent roles. Role lines are anchored to existing ratified sources
> where they exist (cookbook §0 "Division of labor"; [Librarian.md](Librarian.md)). Corrections
> are the user's — the Librarian maintains this file's structure, never the doctrine in it.
>
> Recorded by the Librarian, 2026-07-25, from the user's tool-split ruling (this session).

**The split principle.** Cursor lanes handle **data plumbing / tooling**; Claude lanes handle
**judgment / language work**. Anchor: cookbook §0 — *"if it is a VALUE, History wrote it; if it
is a BEHAVIOR, Cursor built it; if it is a NAME ON A FACE, the user said it."*

| Lane | Agent | Owns | Home / charter |
|------|-------|------|----------------|
| **Engine** | Cursor | The port's **critical path** — actual game/port implementation. Deliberately *not* taxed with process tooling; at most occasional log-line hooks for drives. | live state in `docs/state/*.md` per feature |
| **Bridge** | Cursor | The **tooling backbone** — ledger-as-data, verdict/prediction engine, queue + boot-brief generators, the doc **linter**, the drive harness. Sequenced, not parallel. | [state/ww-bridge-tool.md](state/ww-bridge-tool.md) · bus: [WW Linked/ww-bridge-tool-interconnected.md](WW%20Linked/ww-bridge-tool-interconnected.md) |
| **History** | Claude | **Archaeology + value/classification judgment** — decomp sourcing, № classification (which supersedes which, live vs dead), identity research. Values, not behaviors. | [state/cut-actors-demo-restore.md](state/cut-actors-demo-restore.md) · [TPHistory.md](TPHistory.md) |
| **Housing Security** | Claude | **Containment audits only** — covenant gate, commit / mod-folder hygiene, and *negative-controls* on Bridge's tooling (can it report a broken ledger? does an empty query say UNKNOWN, not CLEAN?). Builds nothing. | [Librarian.md §"Relationship to Housing Security"](Librarian.md) |
| **Librarian** | Claude | **Document structure** — compaction, supersession stamping, lint, dedup, boot-brief curation, status-enum sweeps. *Operates* Bridge's linter; authors no content, rules no truth. | [Librarian.md](Librarian.md) · queue: [LIBRARIAN-QUEUE.md](LIBRARIAN-QUEUE.md) |

**The audit pairing (load-bearing).** The lane that **builds** an instrument is never the lane
that **audits** it — the anti-"marking your own homework" rule. Bridge builds the ledger tool;
**Housing Security** negative-controls it. Bridge builds the linter; the **Librarian** operates
it. This is why the Librarian is kept separate from Housing Security (HS audits *containment*,
the Librarian owns *structure*) — see [Librarian.md](Librarian.md).

**Multi-agent protocol.** Lanes coordinate through Interconnected Runs, not live messaging:
[Interconnected Chats/INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) +
the [run-control](state/run-control.md) traffic light. The Librarian holds **no turn** in a run
— it is asynchronous maintenance that runs *between* work.
