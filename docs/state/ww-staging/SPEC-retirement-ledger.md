# SPEC — C1/C4: the retirement ledger that survives the fork's deletion

> Foundry specs · Bridge hosts (roadmap Phase C). **Scoping only.** Written
> 2026-08-16, before the first REAL retirement, because the current
> `tracker/RETIRED.md` is the minimal A4 form and writing the first real
> entry in the wrong shape would set the precedent this spec exists to
> prevent.

## 1. What exists vs what C1 requires

Today: `tracker/RETIRED.md`, fork-side, one-line entries (id, date, reason,
successors). Correct for its only current use (row SPLITS — state
transitions where the code moved nowhere).

C1's requirement is stronger because its subject is different: **when
retire-by-migration (C2) starts actually REMOVING fork code**, the entry
must remain useful **with the fork deleted**. A pointer into fork history
dangles the day the fork dies. Three properties follow:

1. **Lives in the PLUGIN repo** — the artifact that survives. (No new
   covenant surface: the plugin already carries donor-derived code.)
2. **Embeds the removed hunk** in the entry — not a commit link, not a
   graveyard file. Bounded, self-contained.
3. **Append-only**, one file per retirement (the row-store lesson: a single
   authored surface collides; git is the allocator).

## 2. Entry fields (C4, verbatim from the roadmap, plus the day's rulings)

- lane · timestamp · **build ID** (`build_ledger` identity — the 08-12
  crash was lost for want of it) · retirement class (migration / judgment)
- evidence + route coverage tested against (VERIFIED axis vocabulary;
  route-scoped like EXERCISED)
- **the negative-control answer**: "remove this from the patch — what
  breaks, and how would we notice?" (assumed-necessary does not retire)
- the embedded hunk (the code as removed, verbatim)
- commit link (convenience, allowed to dangle; the hunk is the record)
- replacement link old→new where behavior moved (never bare deletion)
- `src` on every claimed axis value (INVENTORY-SCHEMA §1)

## 3. Transitions, not removals

- Tracker rows go `RETIRED`, never deleted; "zero rows" = zero ACTIVE rows,
  queryable by surface. Only baseline-pinned rows need ceremony.
- **Un-retirement is cheap by design**: a state transition plus a revert.
  The ledger entry is not edited — a new entry records the reversal (the
  closed-rows-left-alone rule: rewriting the record is worse than the
  wrong call).

## 4. Gate (same-change rule)

- an entry missing its negative-control answer or build ID ⇒ REFUSED;
- a retirement whose row was baseline-pinned with no ledger entry ⇒ the
  seam gate's existing row-authority block already fires (verified live);
- fixture: a synthetic entry with a dangling pointer and no hunk ⇒ REFUSED
  ("useful with the fork deleted" is testable: the entry must contain the
  code, not reference it).

## 5. Out of scope

- C3 retire-by-judgment stays FROZEN behind D2 receipts + D3 routes.
- No migration is scheduled by this spec; it defines the shape the first
  one must land in.
