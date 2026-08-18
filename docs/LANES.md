# Lanes — who does what

era: era-independent
<!-- era rationale: the lane roster; the user's ruling, not a method | Librarian, 2026-08-16, user ruling "assign it by era" -->

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
| **Housing** | Claude | **Receiver-side implementation (files rows as Housing/Engine)** — ported subsystems and actors in the fork tree, the `d_ww_` naming covenant, and the vegetation/grass surface. Distinct from the retired *Housing Security* name: **this lane BUILDS, that one AUDITED.** | [HANDOFF-HOUSING-ENGINE-2026-08-17.md](HANDOFF-HOUSING-ENGINE-2026-08-17.md) **(current — start here)** · [HANDOFF-HOUSING-ENGINE.md](HANDOFF-HOUSING-ENGINE.md) *(pre-08-11 history only)* |
| **Integrator** | Claude | **The build, the gates, staging, log reading, and routing.** The only lane that compiles — nothing ships unless it builds. Also holds **containment audits** (covenant gate, commit / mod-folder hygiene, zone classification, negative-controls on other lanes' instruments) — *builds nothing it audits; self-clearing destroys the check.* | [state/ww-staging/HANDOFF-INTEGRATOR.md](state/ww-staging/HANDOFF-INTEGRATOR.md) |
| ~~Housing Security~~ | — | **CONSOLIDATED INTO INTEGRATOR** (user ruling, 2026-08-17). Same instance; the containment-audit role moved to the Integrator row above. Historical rows, rulings and attributions keep this name — **the record is not rewritten.** | [HOUSING-HANDOFF.md](HOUSING-HANDOFF.md) *(historical)* |
| ~~HousingTemp~~ | — | **CONSOLIDATED INTO INTEGRATOR** (user ruling, 2026-08-17; the identity had already been merged in practice on 2026-08-14 — *"HousingTemp/Integrator"* — and the roster never recorded it). | [HOUSINGTEMP-HANDOFF.md](HOUSINGTEMP-HANDOFF.md) *(historical)* |
| **Librarian** | Claude | **Document structure** — compaction, supersession stamping, lint, dedup, boot-brief curation, status-enum sweeps. *Operates* Bridge's linter; authors no content, rules no truth. | [Librarian.md](Librarian.md) · queue: [LIBRARIAN-QUEUE.md](LIBRARIAN-QUEUE.md) |
| **Foundry** | Claude | **Methods + instruments** — DuskTap & successors, capture reels/replay harness, probe schema + differ, golden traces, oracle-stack doctrine. Builds what the lanes work with; touches no content. *(Added on the user's spin-up ruling, 2026-07-28; bus §195–§196, §198.)* | [Foundry.md](Foundry.md) · [state/foundry-methods.md](state/foundry-methods.md) |
| **Decoder** | Claude | **Donor decompilation** — owns the 655 port-grade drafts in `docs/state/ww-staging/decode-drafts/`, burns down the `[INFERENCE-NEEDED]` markers (debug maps → sibling analogy → headers), and proves byte-true claims through the (A) gate (donor PPC toolchain → REL SHA1 vs `config/GZLE01/build.sha1`, perturbed-byte control per claim). Acts on `decomp_watch` flips so drafts are CONFIRMED or REPLACED by the official decomp rather than becoming permanent. | [DECODER-BRIEFING.md](DECODER-BRIEFING.md) |

**CONSOLIDATION, 2026-08-17 (user ruling).** *Housing Security* and *HousingTemp*
are folded into **Integrator** — one instance held all three names, and the
roster had never listed Integrator at all despite it being the only builder.
The consequence was concrete: rows addressed to "Housing Security" alone were
unroutable, because every tool derives its lane set from THIS TABLE.

**Only the live routing surface was changed.** The ~815 historical mentions
across the tale, `CALLS.md`, the archive and source comments are **left
standing** — they record who ruled what and who found which defect (e.g.
`m_Do_ext.cpp` credits HousingTemp for two real instrument-audit defects).
Renaming them would rewrite the record, which this estate forbids elsewhere
(the archive is append-only; negative receipts are unprunable). One lane going
forward, three names in the history.

**The audit pairing (load-bearing).** The lane that **builds** an instrument is never the lane
that **audits** it — the anti-"marking your own homework" rule. Bridge builds the ledger tool;
**Housing Security** negative-controls it. Bridge builds the linter; the **Librarian** operates
it. This is why the Librarian is kept separate from Housing Security (HS audits *containment*,
the Librarian owns *structure*) — see [Librarian.md](Librarian.md). **Foundry** builds the
measurement instruments (DuskTap etc.); **Housing Security** negative-controls them — this
transfer (2026-07-28) closed the anomaly of Housing having built DuskTap itself.

**Multi-agent protocol.** Lanes coordinate through Interconnected Runs, not live messaging:
[Interconnected Chats/INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md) +
the [run-control](state/run-control.md) traffic light. The Librarian holds **no turn** in a run
— it is asynchronous maintenance that runs *between* work.
