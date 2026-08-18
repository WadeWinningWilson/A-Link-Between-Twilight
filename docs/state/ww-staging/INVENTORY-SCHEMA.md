# WW Port Inventory — SCHEMA (Librarian custody)

> **Author/owner: Librarian** (document structure). **Foundry** builds ingest / compute / query-CLI
> against this schema; **History + the tools** supply the measured values (content). User-approved
> plan, outside-eye reviewed (§921 era, 2026-08-14).
>
> **Truth is a queryable JSON store. The human inventory doc is a RENDERED VIEW of it — generated,
> never hand-kept.** Edit the render and the next `render` overwrites it (exports are read-only).

## 0. The one rule — no bare number

**Every record declares its UNIT TYPE, and every stored/rendered number carries its SCOPE.** A bare
count or percentage is **inadmissible**. Two failures this schema exists to prevent:
- *Silent scope-widening reads as a crash.* `59.5% (Outset)` and `7.5% (corpus-wide)` are the **same
  measurement at two scopes** — an 8× different read; without the tag, widening the scope looks like
  a regression.
- *Silent unit omission mis-counts.* "47 vs 60" happened because the unit set (actors only vs
  actors+assets) was never declared.

`UNKNOWN` is mandatory (№31-C): a missing axis value is **UNKNOWN**, never assumed clean.

## 1. Record shape — one JSON object per unit

```json
{
  "id":        "<donor-native id>",         // e.g. "d_a_tsubo", "Vfuku.arc" — never a fan label
  "unit_type": "actor | asset | <subsystem>",  // §3 — must be one the schema header declares
  "scope":     "<denominator context>",     // §2 — MANDATORY, e.g. "Outset" | "corpus" | "sea/Room44"
  "axes": {
    "portable":  { "state": "...", "src": "..." },   // §4.1
    "linked":    { "state": "...", "src": "..." },   // §4.2
    "exercised": { "state": "...", "src": "..." },   // §4.3
    "byte_true": { "state": "...", "src": "..." },   // §4.4
    "verified":  { "state": "MATCH|DRIFT|UNKNOWN", "src": "..." }   // §4.5
  },
  "asof": "<ISO date the axes were measured>"   // values are snapshots; date them
}
```
Every `axes.*` object is `{state, src}` — **`src` is required** (which tool/measurement produced the
state, so the value is falsifiable from its own output; a value with no `src` is `UNKNOWN`).

## 2. SCOPE — mandatory on every number (§2)

- Each **record** carries `scope` = the context its axes were measured in.
- Each **aggregate** the query-CLI emits (count, %, ratio) is printed **with its scope** — the
  renderer must never print a number without one.
- Values: a stage (`Outset`, `sea/Room44`), `corpus` (whole tree), or a named subset. A number at one
  scope is never silently compared to another.

## 3. UNIT TYPE — declared, never silently omitted (§3)

The **schema header of any inventory instance must state which unit types it covers**, one of:
- **`actors+assets only`** — stated explicitly; or
- **`actors + assets + subsystems`** — where subsystems are `ja1`, `evt1`, `kankyo`, `ww_jpa`,
  measured by census axes **C / D / P / W**.

Silent omission is forbidden (it caused the 47-vs-60 count). A record's `unit_type` must be one the
header declares; the CLI refuses records whose type is outside the declared set.

## 4. The five axes

Four axes measure *presence/fidelity of the port*; the fifth measures *whether it actually behaves* —
and the fifth is the one that catches what the other four miss.

| # | axis | question | states | `src` (measurement) |
|---|------|----------|--------|---------------------|
| 4.1 | **PORTABLE** | is the donor decompiled/portable? | `DECOMPILED` · `NONMATCHING` · `STUB` · `ABSENT` · `UNKNOWN` | donor decomp (`configure.py` / KIT-LINEAGE / census) |
| 4.2 | **LINKED** | registered in the plugin? | `REGISTERED` · `RELINQUISHED`* · `PENDING` · `NOT` · `UNKNOWN` | plugin register feed (`ww_profile_register`, V10a) |
| 4.3 | **EXERCISED** | resolved/hit at runtime? | `EXERCISED` · `REGISTERED-NOT-EXERCISED` · `NOT` · `UNKNOWN` | resolve feed (register-resolve / `DUSK_PLACE_TAP`) |
| 4.4 | **BYTE-TRUE** | byte/format equivalent? | `BYTE-TRUE` · `EQUIVALENT` · `DIVERGENT` · `UNKNOWN` | `l2c_equiv` / hash-vs-disc |
| 4.5 | **VERIFIED** | does it behave correctly? | **`MATCH` · `DRIFT` · `UNKNOWN`** | `state_gate` / `probe_differ` / golden traces |

\* `RELINQUISHED` = the receiver handed the row over and the WW layer legitimately owns it — a
**healthy** state, semantically *handed-over* (per the emitter's own definition); it is not abandonment.

**Why the fifth axis is not optional — the tsubo case.** `d_a_tsubo` scored clean on all four of
PORTABLE/LINKED/EXERCISED/BYTE-TRUE and **still fell through a shelf** in play. Presence and byte-fidelity
do not imply correct behavior. `VERIFIED` is the only axis that catches this, and it is **three-state**:
`MATCH` (behavior verified equal), `DRIFT` (verified different), `UNKNOWN` (no instrument has looked —
never read as MATCH). A record clean on 4.1–4.4 with `verified: UNKNOWN` is **unverified, not done.**

## 5. Rendering + retention policy (§5)

- **Rendering:** the human-readable inventory is a **rendered view** the query-CLI generates from the
  JSON. Never hand-edited; a hand-edit is overwritten by the next render. The render **always prints
  each number's SCOPE and the instance's declared UNIT-TYPE set** (§2/§3) so a copied figure keeps its
  qualifiers.
- **Retention:** the JSON store is the durable truth. Keep the live store + **dated milestone
  snapshots** (a snapshot is immutable; values carry `asof`). Superseded values are **struck, not
  deleted** (audit trail), consistent with the ledger's own supersession rule.

## 6. Excluded by ruling

- **FROGRESS export** — CLOSED by the user (no relationship with the decomp project). Do **not** design
  for it; no field, no render path.

## 7. First worked example — the receiver-layer count (three numbers, no tags)

The schema's first live case, surfaced by [`ww-clean-room-delivery.md`](../../WW%20Linked/ww-clean-room-delivery.md)
(Housing/Engine, 2026-08-14). "The receiver layer" — the very set that *defines* "everything we built" — has
been counted three ways, and **none of the three carried a scope or unit-type tag**:

| number | source | scope (as the source itself states it) | unit_type |
|---|---|---|---|
| **34 files** | [`NEVER-PUSH-STRIP-SET.md`](../../NEVER-PUSH-STRIP-SET.md), 2026-07-25 | strip-set membership, that day (a dated snapshot) | files |
| **79 `.cpp` + 51 `.h`** | row 85 (Librarian read) = the generated Tier-1 block | Tier-1 **UNION** (filename ∪ declared-lineage), **includes `d_a_*` actors + loose `d_*`**; headers by dir-scan | sources + headers |
| **49 `.cpp` + 44 `.h`** | `ww-clean-room-delivery.md`, measured 2026-08-14 | **four dirs only**: `ext_plugin/ + ext_seq/ + ext_evt/ + d_ext_*` | sources + headers |

**These do not contradict — they are three different SETS.** The widest (79/51) includes actors and counts
headers by directory scan; the narrowest (49/44) is four plumbing dirs; 34 is an older membership snapshot. They
*read* as a discrepancy only because each was published **bare**. Attach `scope` + `unit_type` (§2/§3) to each and
the conflict dissolves — that is the whole thesis of this schema, demonstrated on its own definitional set.

**What the schema does NOT do here (lane wall):** it does **not** rule which count is "the" receiver layer. That
is a re-measurement under one declared scope — **Foundry/Housing's to run through the ingest/query CLI, not the
Librarian's to assert.** This row is the **negative-control fixture** for the SCOPE/UNIT-TYPE requirement: three
honest numbers, one subject, reconcilable only by tag. A CLI that cannot keep these three apart by their tags has
not implemented §2/§3.

---

## 8. OWNERSHIP vs the five axes — RULED, not assumed (A5; Librarian, 2026-08-16)

**THE RULING: CROSS-REFERENCE, NOT MERGE. Ownership is a SIXTH axis and it is orthogonal to all five.**

`ww-ownership-map.json`'s `_categories` answers **"which track does this file belong to"** — a partition
of provenance. The five axes answer **"how far along is this unit, and is it right"** — per-unit progress
and verification. **A file's ownership implies nothing about its port state, and no port state implies an
owner.** `ww-port` + `PORTABLE:ABSENT` is a perfectly coherent record; so is `albw` + `VERIFIED:MATCH`.
Merging them would force a single field to answer two questions, which is how a vocabulary starts lying.

**But "different axes" does NOT discharge the gate — a shared axis is only one way to get a parallel
vocabulary. The other is COLLIDING TOKENS, and there are three:**

| ownership token | collides with | ruling |
|---|---|---|
| `UNKNOWN-VERDICT` | bare `UNKNOWN` on all five axes | **Never shorten it to `UNKNOWN`.** The suffix is load-bearing: it means *ownership undecided*, not *unmeasured*. A bare `UNKNOWN` in a record is an AXIS STATE and must never be read as an ownership value. |
| `dual-pending` | `PENDING` (LINKED §4.2) | **Two unrelated pendings.** LINKED `PENDING` = registration in flight. `dual-pending` = ownership contested between two tracks. Neither may be written as bare `pending`. |
| `not-ww` | `NOT` (LINKED §4.2, EXERCISED §4.3) | **`NOT` is an axis state meaning not-registered / not-hit. `not-ww` is a scope verdict meaning out of this inventory entirely.** A unit that is `not-ww` should not carry axis states at all. |

**THE PARALLEL VOCABULARY THAT ALREADY EXISTS IS INSIDE THE MAP, NOT BETWEEN THE MAP AND THIS SCHEMA.**
Every record carries BOTH `verdict` (`"WW-PORT"`, upper case, prose, sourced from `_verdict_sources`) and
`category` (`"ww-port"`, lower case, structured). **Same axis, two spellings, on all 162 records.** That is
the exact shape History/Bridge and the Integrator validated a law against tonight — two lanes string-matched
the free-text field while the structured one sat beside it. **RULING: `category` is authoritative and is the
only field a tool may branch on. `verdict` is provenance prose for a human reader and must never be parsed.**

**CASE HAZARD, flagged because tonight's law is about matchers:** `_categories` is lower-case-kebab in ten
of eleven members and `UNKNOWN-VERDICT` is the lone upper-case token. **A case-sensitive matcher over that
list is correct for ten values and silently wrong for one** — and the wrong one is the *we-do-not-know*
value, so its failure mode is to disappear rather than to error. Normalise on read; do not rely on the
declared spelling.

**DATA NOTE, measured this pass and NOT a criticism of the map:** five of the eleven declared categories
currently have **zero** members (`UNKNOWN-VERDICT`, `dual-pending`, `not-ww`, `stale-path`, `upstream-native`).
Live distribution is `ww-port 129 · shared-per-hunk 18 · albw-skins 8 · albw 5 · instrument 1 ·
fork-host-plumbing 1` = 162. **A declared-but-unused vocabulary entry is correct and should stay** — a
category list is a vocabulary, not a census, and deleting unused values is how a partition loses the ability
to express a state it will need again. **What IS stale is the PUBLISHED distribution** (announced as
`shared-per-hunk 17 · albw-skins 6 · dual-pending 3 · upstream-native 1 · ONE CONTESTED`); the map has
moved since, correctly, under later rulings. **Cite the file, not the announcement** — §0's rule, applied
to a census instead of a number.

---

*Librarian owns this schema's structure; changes to axes/fields are supersession-stamped here. The
values are the tools' and History's; the Librarian rules no truth, only the shape it is recorded in.*
