# The Staging Protocol — parallel lanes, one integrator, batch delivery

> **Status:** SPEC, Foundry, 2026-08-09 (user directive: end blocking waits;
> queue finished work; one end-of-line reviewer; redo loop; deliver at once).
> Supersedes nothing — the bus stays the record; this changes how work REACHES it.

## The problem it solves, from this project's own receipts

- Lanes block on each other's bus turns; wait times compound.
- Parallel sessions self-assign §N → three numbering collisions in one day
  (§592, §596/§597, §601).
- Work ordered off a stale board: §590's "Phase 2 opened" after Engine closed
  step 19; the stale warp-row ask; "verbatim" claims after the tree moved.
  Every one cost a turn or a retraction.

## The shape

```
lane A ──draft──▶
lane B ──draft──▶   STAGING AREA ──▶ INTEGRATOR ──▶ one batched bus append
lane C ──draft──▶   (manifest)        (verify/flag)      (final §N assigned HERE)
        ◀──────────── REDO (flagged, reason named) ◀──
```

## Mechanics

**1. Staging area:** `docs/state/ww-staging/` — one file per deliverable,
named `<lane>-<local-id>.md` (`foundry-3.md`, `housing-7.md`). Drafts carry
**lane-local ids, never §N** — final numbers are assigned only at merge, which
kills the collision class outright.

**2. The manifest:** `docs/state/ww-staging/MANIFEST.md`, one row per item:

```
| id | lane | title | state | depends-on | claims |
state ∈ DRAFT → READY → (FLAGGED → READY)* → MERGED
```

`claims` = the checkable assertions the draft rests on ("file X contains Y",
"§N ruled Z", "tool T exits 0") — written by the author, verified by the
integrator. A claim nobody can check is a claim the integrator flags on sight.

**3. Lanes never wait.** Finish → write draft → set READY → take the next
task. Blocked-on-upstream work is visible as `depends-on` a non-MERGED row —
a queue position, not a stalled session.

**4. The integrator** (end-of-line instance — any lane can serve a shift, but
one at a time) runs the pass:
   - **claims vs tree** — does the file/function/number exist AS CLAIMED
     *now*? This is the stale-state detector (§594) finally built as a role.
   - **cross-draft consistency** — two drafts asserting different states of
     one item = both FLAGGED, both named.
   - **dependency order** — a FLAGGED upstream HOLDS its dependents, visibly.
   - **no content authoring** — the integrator flags and merges; it never
     fixes a lane's work (content-neutrality, the Librarian's wall applied).

**5. Redo loop:** FLAGGED rows carry the named inconsistency. The owning lane
fixes, re-sets READY. Nothing merges around a flag.

**6. Batch delivery:** the integrator appends all MERGED items to the bus in
ONE write, dependency order, sequential §N assigned at that moment, each
section headed `## §N — <lane> (staged as <id>):`. One append = no collisions,
and consumers of the bus see a consistent world-state, not interleaved
mid-states.

## What stays the same

The bus remains the permanent record; WHOSE-TURN blocks still close every
delivered batch; lane charters unchanged; user rulings still arrive any time
and outrank the queue. Small urgent findings (crash in a file someone is
editing NOW) may still go straight to the bus — the protocol is for
deliverables, not alarms.

## Tooling (Foundry, next increment)

`staging.py`: manifest lint (states legal, ids unique, depends-on resolvable),
claim extraction for the integrator's checklist, and the batch-append writer
(assigns §N atomically off the live bus max). Until it exists the protocol
runs by hand — it is a discipline first, a tool second, same as L4.

## Evidence-persistence rule (added 2026-08-11, from the first FAILED claim)
Fan-out agent output is persisted VERBATIM to the staging area BEFORE any
condensation. Task transcripts are NOT durable (first claim-3 failure: a
50-row table existed only in a transcript that read back 0 bytes). The
condensed extract is a convenience on top, never the artifact of record.
