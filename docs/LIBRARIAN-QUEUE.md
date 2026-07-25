# Librarian Queue — routed flags

> **What this is.** The Librarian's flag registry. Each entry is a *structural* observation
> — a lint, a coverage gap, a stale pointer, or a content contradiction the Librarian is
> **not allowed to decide.** Every entry names the **lane it routes to via the user.** The
> Librarian never resolves a routed flag itself; it only records, and executes the
> *mechanical* fix once the owning lane/user rules.
>
> Charter: [Librarian.md](Librarian.md). This file is librarian-owned; lanes read their
> routed entries and reply through the user.

**Legend — type:** `LINT` (mechanical) · `COVERAGE` (something risks being lost) ·
`STALE` (pointer/number drifted from its authority) · `CONTENT` (truth question — Librarian
does **not** rule) · `ORPHAN` (missing back-link/discoverability).
**Status:** `OPEN` · `ROUTED` (awaiting owning-lane/user) · `CLEARED`.

---

## A. Cookbook fork (three files; canonical mechanism already ruled 2026-07-23, bus §106/§107)

Files: canonical [WW-Restoration-Cookbook-CANONICAL.md](WW-Restoration-Cookbook-CANONICAL.md) (writable) ·
frozen fork **A** [WW-Restoration-Cookbook.md](WW-Restoration-Cookbook.md) ·
frozen fork **B** [WW Linked/WW-Restoration-Cookbook.md](WW%20Linked/WW-Restoration-Cookbook.md).

| ID | Type | Finding | Evidence | Routes to | Status |
|----|------|---------|----------|-----------|--------|
| CB-1 | LINT | CANONICAL's pending-corroboration index lists the **DECOMP-FIRST** and **OffsetPos** banners as living in "linked copy" only. They are **byte-identical in BOTH forks** — i.e. **BILATERAL**, the cheapest admission route, not the History-confirm route the index assigns. | `diff` of lines 10–65 in A vs B = identical; banners present in both. | Index owner (Housing / canonical maintainer) | ROUTED |
| CB-2 | COVERAGE | Fork **A**-only technical chapters — lighting 4-table chain, RTBL trap, veg-asset extraction, arc-adaptation internals — are **not tracked as distinct pending rows**. The catch-all row calls process chapters "both copies, diverged," but lighting is A-only. If the index drains and the forks retire, A-only doctrine is **silently lost.** | `grep -c PAL0`: A=3, B=0. Companion/`PARENT-COMPOSE`: A=0, B=1 (mirror gap, B-side). | Index owner | ROUTED |
| CB-3 | STALE | Both fork banners assert **"542 diff lines"**; live `diff` now reports **534** (files changed after the banner was written; charter said ~536). Cosmetic. Candidate to replace with a generated count per the `inventory.md` "ground truth is the tool" model. | `diff A B \| grep -c '^[<>]'` = 534. | Index owner | ROUTED |
| CB-4 | CONTENT | Fork **A** §0 states №31 in its **older, narrower** form ("donor spaces receive only donor assets"); fork **B** §0 is the **broadened 2026-07-20** form (rupees, №31-B shaping, №31-C UNKNOWN). Canonical adopted **B**. **Is A's narrower §0 purely superseded, or does it phrase anything B/canonical dropped?** Librarian does **not** rule this. | Presentation-Parity: A=1, B=0. `COVERS SHAPING`/`CANNOT RUN`: A=0, B=1. | **History** (owning lane) | ROUTED |

**Librarian's own remaining mechanical tasks (do only on user go-ahead):** when the pending
index drains, collapse both frozen forks into one-line pointers to CANONICAL.

---

## B. Governance / process-model propagation (lane split + Librarian)

Scan question (user, this session): *do the docs abide by the lane/tool-split work process, or
are they products of it?* Answer: the **product docs** (state tips, recipes, handoffs, research)
are governed correctly via AGENT_INDEX → state; the **governance layer itself has not absorbed
the newest model** (four lanes + Librarian + ledger-as-data). Details:

| ID | Type | Finding | Evidence | Routes to | Status |
|----|------|---------|----------|-----------|--------|
| P-1 | ORPHAN | ~~The **Librarian** lane is referenced by exactly one file — its own charter.~~ **CLEARED 2026-07-25:** back-links added from [AGENT_INDEX.md](AGENT_INDEX.md), [AGENTS.md](../AGENTS.md), and [LANES.md](LANES.md). Bus back-link left to Bridge (not a Librarian edit). | `grep -rl Librarian docs/` → `docs/Librarian.md` only (before). | User (orientation-doc owner) | CLEARED |
| P-2 | ORPHAN | ~~The four-lane roster + tool-split is not defined in any governance doc.~~ **CLEARED 2026-07-25:** roster shelved to [LANES.md](LANES.md), provenance-stamped to the user's ruling + existing anchors (cookbook §0 division-of-labor; Librarian.md). Content is the user's to correct; Librarian maintains structure only. | `grep` for roster terms in bus → 0 hits (before). | User | CLEARED |
| P-3 | STALE | AGENT_INDEX line 32 marks the **Active run** as *Cut-Actors / Demo Restore*; `run-control.md` (the designated authority) says the active run is *WW Bridge Tool — Bridge ↔ Housing Security*. | run-control `run_name` vs AGENT_INDEX:32. | User / orientation owner | ROUTED |
| P-4 | STALE | AGENT_INDEX's Interconnected-Runs table **omits** the currently-active WW-Bridge-Tool run doc, its bus (`WW Linked/ww-bridge-tool-interconnected.md`), and its state tip (`state/ww-bridge-tool.md` — not linked anywhere in AGENT_INDEX). | `grep ww-bridge-tool docs/AGENT_INDEX.md` → no match. | User / orientation owner | ROUTED |

**Note (Librarian):** P-1…P-4 are all *flag-only*. Adding the lane roster or rewriting
AGENT_INDEX's orientation is **authoring / governance** — outside Librarian scope. Fixing the
P-3/P-4 stale pointers to match `run-control` is mechanical lint the Librarian *can* execute,
but only on explicit go-ahead (it involves a curation choice: point at one run, or list both).

---

*Maintained by the Librarian lane. Entries are structural, never truth rulings. Last swept: cookbook fork + governance scan.*
