# SPEC — the seam-tracker row store (roadmap A3)

> Foundry spec, 2026-08-16, per the amended roadmap's owner column
> ("Bridge hosts · Foundry specs"). **Bridge owns hosting decisions** —
> location may move; the properties below may not, because each one is a
> failure this estate already paid for. Gate: **two lanes edit different
> rows on one shared surface and merge clean.**

## 1. Shape: one file per row

`docs/state/ww-staging/tracker/rows/<ID>.md` — proposed location, Bridge's
call. **Authored content lives ONLY in per-row files.** The index
(`tracker/INDEX.md`) is GENERATED, never hand-edited, and regenerating it
must MERGE nothing — it is a pure function of the row files.

*Why:* the tale bus's one shared authored surface produced 26 § collisions
and forced an allocator. Disjoint files make git the allocator.

## 2. IDs: minted, random, never the symbol

- **ID = 8 lowercase hex chars, randomly minted** (`s3f9a2c1` style, `s`
  prefix). No sequential counter — a counter needs an allocator and the
  allocator needs a lane to remember it (§P2). Random IDs mint offline in
  parallel; a collision is a filename clash git surfaces loudly.
- **The seam symbol is a FIELD, not the filename.** Symbols get renamed; a
  rename must not orphan the row or its retirement history (the
  `B2b`-vs-`B2-b2` lesson: mutating an identifier made a closed item and an
  open one indistinguishable).

## 3. Row fields (all `key: value`, ASCII keys, one per line)

```
id:            s3f9a2c1
symbols:       dExtWwSave_registerWwStage        # comma list; renames edit HERE
doorway:       ABSENT-unhookable                  # EXISTS | GATED |
                                                  # ABSENT-hookable | ABSENT-unhookable
destination:   PATCH                              # axis A, from the ownership map's domain
portable:      UNKNOWN                            # PER-AXIS fields, INVENTORY-SCHEMA vocabulary
linked:        NOT                                # ONLY (A5 ruling). The spec's first example
exercised:     UNKNOWN                            # carried `state: DECLARED` — a value on NO
byte-true:     UNKNOWN                            # axis, violating the rule on its own line;
verified:      UNKNOWN                            # the validator REFUSED it (7/7 selftest) and
                                                  # History/Bridge's per-axis hosting decision
                                                  # is adopted here as the spec.
provenance:    see-file                           # NEVER copied here — parsed live from the
                                                  # TU's KIT-LINEAGE header (kit_laws.lineage)
negative-control: |
  Removed from the patch, WW stage warp black-screens (no registration);
  detected by the seam fire-receipt going absent on the next fork run.
citations:
  - src/dusk/boot_stage.cpp:111 (read 2026-08-16T04:52Z)   # WwPilot lesson:
  - upstream_conformance --symbol ... MISSING (run 2026-08-15)  # cite WHEN
created:       FOUNDRY 2026-08-16 build:<id>
```

Field rules, each with its receipt:

- **`doorway` and `destination` values validate against DECLARED domains**
  in `tracker/_schema.json`. Unknown value → the index generator refuses
  loudly, never buckets silently (§P3 — five specimens in one session).
  The schema enumerates **the bounded side** where a partition exists
  (§P4 — the floor rule).
- **`provenance` is never stored** — it is parsed from the TU's own
  `KIT-LINEAGE` header at index time. A copied value is a second roster
  that drifts (the fifth-roster failure, committed once, caught once).
- **Every source citation carries a read timestamp.** On a live tree "I
  read X" and "it is Y" are both true (`WwPilot 0→2`).
- **`negative-control` is mandatory at intake** for PATCH-destination rows:
  "removed, what breaks, how noticed." Assumed-necessary does not ship.

## 4. Hygiene, enforced at index generation

The generator runs `c0_audit` over every row file and **refuses the index**
on any invisible byte or BOM. Receipts: a BS byte disabled a regex for a
tool's whole life; `Set-Content -Encoding UTF8` BOMs broke row parsing
twice; **row content is ferried via Write-tool files only, never shell
arguments** (`file_row.py`'s own doctrine, violated twice tonight by its
maintainer, caught both times by its byte audit).

## 5. Lifecycle interplay (pointers, not restatements)

- **Load-bearing tier** (amendment §2): where the registration IS the row,
  the row file is *generated from* the registration and carries
  `generated: true` — hand edits to generated rows are refused.
- **RETIRED is a state transition, never a deletion** (C4). The retirement
  ledger (C1, plugin repo, embedded hunk) references the row **ID** —
  which is why IDs outlive symbols.
- **The A4 baseline snapshots the ROW SET** (IDs + content hashes). A row
  present in the last baseline and absent now, with no retirement entry,
  fails the build — the deletion bypass (amendment §6c) dies there.

## 6. What the gate test actually is

Two lanes each add one row and each edit a *different* existing row, on
diverging branches; merge. **Pass:** zero conflicts, index regenerates
identically from either side. **Fail:** any conflict, or an index that
depends on merge order. Run it before the store is declared live — a store
that has never merged two branches has an untested load-bearing property
(the vacuous-pass rule, applied to infrastructure).
