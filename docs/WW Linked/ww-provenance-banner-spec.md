# WW provenance banner — Foundry spec (roadmap step 10)

> **Lane: Foundry specs · Engine lands · HousingTemp controls.**
> **Status: SPEC. Reader + lint built and piloted; mass application is Engine's.**
> Closes census D-5 (unmapped TUs), D-6 (`[provisional]` groupings), and is the
> precondition for Axis B's rulability at roadmap step 9.

## 1. Why a banner, and why not another convention

Roadmap step 2 generates the WW-layer roster from `files.cmake` by **filename
convention**. Posture §1.1 records why that has a ceiling: the convention is
structurally blind to the most donor-faithful work in the tree, because a direct
port of WW's `d_a_lamp.cpp` is *correctly* named `d_a_lamp.cpp`. It gets blinder
as the porting gets better.

Three census results now depend on declared lineage rather than inferred:

* **D-5** — Axis D maps a receiver TU to its donor object by **basename or a
  cited path**. 7 of 10 subsystems sit below 100% coverage, and an unmapped TU
  has no decomp status at all.
* **D-6** — the `[provisional]` groupings (V6/V8) split whole-file ports from
  leg-carriers **by path**, because nothing checkable separates them.
* **Axis B** finds candidate arrays but cannot prove **donor origin**, so the
  byte tally is not yet a rulable trip-wire (b) number.

And one project-level result: the user's standing direction is that we
**re-base onto the finished decomp**. A per-file upstream reference turns that
re-base into a version bump against a known origin instead of archaeology across
~73 TUs.

**This spec EXTENDS the already-ratified §426 tag rather than inventing a second
convention.** `// KIT-LINEAGE:` is live on 15 TUs, is grep-exact, and both the
lint and the census already read it. A second banner format would be a second
roster that drifts — the exact failure census spec §1 exists to prevent.

## 2. The banner

Grep-exact, one field per line, immediately following the file's opening comment
block. Field order is fixed so a reader can be a line scan.

```
// KIT-LINEAGE: native-port | bridge-owed[:ledger-ref] | host-plumbing
// KIT-DONOR: <donor source path, donor-relative>        | none
// KIT-DONOR-REF: <upstream ref>                         | unpinned
// KIT-DONOR-STATUS: Matching | NonMatching | Equivalent | MatchingFor(<versions>) | UNKNOWN
```

Worked example, `src/d/actor/d_a_ext_ep.cpp`:

```
// KIT-LINEAGE: native-port
// KIT-DONOR: d/actor/d_a_ep.cpp
// KIT-DONOR-REF: unpinned
// KIT-DONOR-STATUS: Matching
```

### Field rules

| Field | Rule |
|---|---|
| `KIT-LINEAGE` | Unchanged from §426. Already ratified; do not redefine. |
| `KIT-DONOR` | Path **as the donor's own build names it** (`configure.py` object paths), so Axis D maps by identity, not by basename guessing. `none` is a positive claim — "this TU has no donor counterpart" — and is different from omitting the field. |
| `KIT-DONOR-REF` | **E5 (2026-08-06): now pinned** — `zeldaret/tww@<sha>`. SEMANTIC, stated precisely because the obvious reading is not the true one: this is the donor ref the TU's donor facts were **VERIFIED AGAINST**, not necessarily the ref the port was originally *taken from*. Ports landed over months against a donor checkout that has moved since, and no per-TU record of the then-current donor commit exists — so "taken from" is unknowable for most TUs and stamping it would be a false claim of the exact kind this banner exists to prevent. "Verified against" is true for every TU (E1–E4 checked each donor path and status against this checkout) and it is the property re-baselining actually needs: diff the donor file between this pin and any future commit. `unpinned` remains valid for a TU whose donor facts have not been re-verified. |
| `KIT-DONOR-STATUS` | The donor's declared status **at the time of porting**. A SNAPSHOT, and per the user's supersession rule the **upstream decomp's eventual ruling wins over it**. |

### F1 — multiple donors, divergent statuses

A TU may port from several donor objects whose statuses differ.
`d_a_ext_vegetation` is the live case: three donors, statuses `Matching`,
`Matching`, `MatchingFor` — **no single aggregate value is true.**

Either form is valid, and the lint enforces the choice:

```
// KIT-DONOR: d/d_grass.cpp Matching          <- per-line status
// KIT-DONOR: d/d_flower.cpp MatchingFor
// KIT-DONOR-STATUS: UNKNOWN                  <- aggregate MUST be UNKNOWN
```

**Rule:** with more than one `KIT-DONOR` and no per-line statuses, the aggregate
`KIT-DONOR-STATUS` **must be UNKNOWN**. Declaring any single value across
divergent donors is a DISAGREES, because it is the one move that makes a
checker go quiet without making the statement true. Engine hit this in E1 and
resolved it correctly before this rule existed.

### F3 — hunk scope, for shared receiver TUs

A banner labels a whole TU. That is exactly right for the 60 TUs that ARE ports,
and wrong for TP's own files — `d_stage.cpp`, `d_demo.cpp`, `d_particle.cpp` —
which carry donor-derived lines inside otherwise-receiver code. For those,
neither file-level value is true:

* naming a donor path is **false**: the file is TP's, not a port of WW's. It is
  a seductive falsehood because TP and WW share paths (`d/d_stage.cpp`,
  `d/d_door.cpp`, `d/actor/d_a_kamome.cpp` exist in BOTH), so Axis D resolves the
  claim by identity and the lint passes on a lie.
* `none` is **worse than false**: it is a positive clean claim, and it is the
  sentence the strip-set generator reads as "nothing here". Whole-file stripping
  is impossible for these TUs anyway — the build needs `d_stage.cpp`.

So provenance moves to the edit site, and the file top points at it:

```c
// KIT-LINEAGE: mixed
// KIT-DONOR: per-hunk
// KIT-DONOR-REF: unpinned
// KIT-DONOR-STATUS: per-hunk

    ...receiver code...
    // KIT-DONOR-HUNK: d/d_stage.cpp Matching
    OBJNAME("Ls1", fpcNm_NPC_LS1_e, 255),
    // KIT-DONOR-HUNK-END
    ...receiver code...
```

**What counts as a hunk.** Lines whose CONTENT came from the donor: copied or
transliterated code, and donor-authored constants/data (an emitter id, an
`OBJNAME` row, a TEV type the donor chose). Receiver code we WROTE to match donor
*behaviour*, with the donor cited as the authority for the decision, is **not** a
hunk — it is our code, and marking it would attribute our own engineering to the
donor. The `isWwHostStage` gate in `d_demo.cpp` is ours; the assignment it guards
is the donor's form. Only the second is marked.

This distinction is the whole point: it is the difference between a file that is
*assumed* clean and one that has been *looked at*. Four TUs on the roster
(`JASChannel.cpp`, `d_event_data.cpp`, `d_event_manager.cpp`,
`f_pc_profile_lst.cpp`) declare donor citations and were found on inspection to
carry no donor content at all — their `none` is a checked claim.

**The sentinel is not a dodge.** `banner_lint.py` rejects `per-hunk` with zero
hunk markers, hunk markers without a `per-hunk` file top, unbalanced
HUNK/HUNK-END pairs, a hunk donor absent from the donor's `configure.py`, and a
hunk status that disagrees with the donor build. All five are negative-controlled.

### Two rules that are not negotiable

1. **A missing field is UNKNOWN, never assumed.** This is №31-C applied to
   provenance. A TU with no banner is not "probably fine"; it is unmeasured, and
   the census must say so.
2. **The banner is DECLARED, never inferred.** No tool may write
   `KIT-DONOR` by pattern-matching a filename. That inference is exactly what
   the banner exists to replace, and re-introducing it inside the fix would
   repeat posture §1.1's own diagnosis one level down.

## 3. Donor data — the Axis B half

Axis B locates candidate arrays but cannot show donor origin. One marker, placed
on the declaration, closes it:

```
// KIT-DONOR-DATA: <bytes> <class> <donor symbol or source>
static u8 l_toonMat1DL[0xA5] ATTRIBUTE_ALIGN(32) = { ... };
```

`<class>` uses census spec §6's own four buckets: `gx-register-state`,
`lookup-table`, `display-list`, `asset-like`.

**Why this and not a heuristic:** trip-wire (b) is defined on *donor* bytes
("countable on one hand"). Axis B currently reports 123 candidate arrays / 8,468
sized bytes, of which the overwhelming majority are receiver-authored arrays that
merely live in roster TUs. Handing that number to a ruling would overstate the
donor payload by an order of magnitude. **Only a declared marker makes the tally
rulable**, which is why roadmap step 9 is blocked behind step 10 rather than
behind the census run.

## 4. What Engine lands, and in what order

Ordered so that each stage is verifiable before the next:

| # | Step | Why this order |
|---|---|---|
| E1 | Banner the **10 `native-port` TUs already tagged** | Smallest set, already lineage-declared; proves the format against real ports |
| E2 | Banner the **Axis D unmapped TUs** (D-5) | Highest measurable payoff — each one converts an UNKNOWN status into a declared one |
| E3 | Banner the **V6/V8 provisional groups** (D-6) | Re-partitions two verdicts; must re-enter the adoption record's P-protocol at P1 |
| E4 | `KIT-DONOR-DATA` on donor arrays (Axis B) | Unblocks roadmap step 9's user ruling |
| E5 | Backfill `KIT-DONOR-REF` when vendoring is pinned | Depends on roadmap step 12/13; `unpinned` until then |

**E3 changes adopted verdicts.** Per the adoption record's update protocol it is
not a silent improvement: re-run P0, record what moved *and by how much*, strike
rather than delete the superseded rows, and **re-audit at P1 before re-adoption**.

## 5. Verification — built, so the landing is checkable

* `tools/foundry/banner_lint.py` — reports per TU: banner present, fields
  complete, `KIT-DONOR` resolvable against the donor's `configure.py` object
  list, and `KIT-DONOR-STATUS` **agreeing with** that list. A banner that
  disagrees with the donor's own build description is a defect, not a fact.
* The census reads the banner where present and falls back to basename/citation
  mapping where absent — so coverage rises monotonically as Engine lands E1–E3,
  and **no run is blocked on the banner being complete**.

## 6. Honest limits of this spec

* A banner records what a human **declared**, so it can be wrong in a way a
  measurement cannot. The lint checks internal consistency and agreement with
  the donor build description; it cannot check that the port is faithful.
* `KIT-DONOR-STATUS` is a snapshot with a shelf life, exactly like Axis D. It is
  worth recording because it dates the port, not because it stays true.
* A one-to-one `KIT-DONOR` cannot express a TU assembled from several donor
  objects. Multiple `KIT-DONOR` lines are permitted; the reader takes all of
  them, and Axis D maps to each.
