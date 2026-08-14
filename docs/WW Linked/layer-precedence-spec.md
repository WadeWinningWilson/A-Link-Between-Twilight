# L1 — The Precedence Chain (Band 0 layer-model spec)

> **Status:** SPEC, in force on publication. Foundry, 2026-08-09, per the Band 0
> assignment in [ttw-methods-review.md](ttw-methods-review.md) (L-series).
> This is the ordering that is expensive to get wrong; everything else in the
> L-series hangs off it.

## The chain

```
1  DONOR DISC      immutable        the WW disc image / Ex WW tree. Never edited,
                                    never re-encoded, never "fixed". Zero-bake's
                                    endpoint (L2 reads it directly).
2  TRANSLATION     donor→receiver   consumption-boundary code: the room-seam
                                    translators, arc-name aliasing, JPA lineage
                                    handling, attr repack. Reads layer 1, writes
                                    ONLY in-memory receiver structures.
3  INTERACTION     cross-game rules TP↔WW damage/weapon/item/health mappings —
                                    DATA rows in R5 (L4), never inline in actor
                                    code. Reads layers 1-2's output.
4  TUNABLES        numbers exposed  R5 value rows with a precedence read (L5).
                                    A tunable OVERRIDES an interaction default;
                                    it never rewrites the mapping itself.
5  MOD OVERRIDES   player intent    load-order manager (top-wins, playtested) +
                                    later DuskScript (L7). Overrides tunables;
                                    touches nothing below.
```

## The one rule: NO LAYER EDITS THE LAYER BENEATH IT

Each layer **adds** — a translation, a mapping row, a value, an override. It
never reaches down and modifies what it consumes. This is TTW's load-order
model and the project's own **never substitute — add and label** rule expressed
as architecture. Precedent already paid for: the §604/§610 class happened
because layer-2 work (bake) rewrote layer-1-shaped data into ambiguous form;
the load-time translator fixed it by moving the same work to the correct layer.

## What belongs where — the sorting test

Ask: **what does it read, and what would break if the layer below changed?**

- Reads donor bytes, writes receiver structures → **2 (translation)**.
- Encodes "WW bomb does N hearts to TP Link" → **3 (interaction)**, as an R5
  row. Inline in an actor is a misfiled layer-3 fact and becomes a rewrite when
  L5 arrives — the sequencing trap named in the plan.
- A number a designer might tune → **4**, reading its default from 3.
- Anything a player installs → **5**.
- "Accepted by design — no reconciliation owed" (L3's third status) is a
  layer-3 **classification**, not a layer-2 patch: the donor stays untouched,
  the deviation is labeled where it lives.

## Violations, named so they can be lint-checked later

- **Downward edit**: any tool or code that modifies a lower layer's artifact
  (re-encoding donor assets; a mod that patches translation code).
- **Layer skip**: layer-5 content reaching into layer-1/2 directly (a mod
  shipping edited donor bytes instead of overrides).
- **Inline interaction**: a cross-game rule hard-coded in actor code instead of
  an R5 row — the L4 discipline breach. Not detectable by lint until R5 exists,
  which is why R5 precedes R1 in the amended ordering.

## Interfaces between layers (current, named)

```
1→2  wwIsoPath / ww_donor_disc (L2, Housing Security) · Ex WW tree today
2→3  receiver in-memory structures (dStage_dt_c etc.) — the seam's output
3→4  R5 mapping rows (Bridge hosts, History authors)
4→5  load-order manager (built, playtested) · DuskScript event bus (L7, unbuilt)
```

A future layer change renegotiates its interface; it never bypasses it.
