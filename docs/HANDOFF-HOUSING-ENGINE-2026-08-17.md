# HANDOFF — Housing/Engine (instance retiring 2026-08-17)

era: mounted
<!-- era rationale: instance handoff of the vanilla-collision + quest-wiring campaign; state, not method law | Housing/Engine, 2026-08-17 -->

> *(Cross-reference block, carried forward from the 08-16 handoff. Six files now
> answer to "the Housing handoff". Nothing merged, nothing rewritten. Any older
> "the current one is…" pointer below this block predates it.)*
>
> | doc | kind | dated |
> |---|---|---|
> | [`HANDOFF-HOUSING-ENGINE-2026-08-17.md`](HANDOFF-HOUSING-ENGINE-2026-08-17.md) | **CURRENT INSTANCE HANDOFF** — the vanilla-collision campaign, the EVNT quest chain, the warp menu, four rulings. **Start here for present state.** | 2026-08-17 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-16.md`](HANDOFF-HOUSING-ENGINE-2026-08-16.md) | **PRIOR INSTANCE HANDOFF** — §1002 order + §7 monitor exemplar. Superseded for state; still owns its DO-NOT-TRUST list and the monitor pattern. | 2026-08-16 |
> | [`HANDOFF-HOUSING-ENGINE-2026-08-14.md`](HANDOFF-HOUSING-ENGINE-2026-08-14.md) | **PRIOR INSTANCE HANDOFF** — pre-08-16 history only. | 2026-08-14 |
> | [`HANDOFF-HOUSING-ENGINE.md`](HANDOFF-HOUSING-ENGINE.md) | **PRE-ORDER HANDOFF** — pre-08-11 history only. | 2026-08-10 |
> | [`HOUSING-HANDOFF.md`](HOUSING-HANDOFF.md) | **RETIRED-INSTANCE RECORD** — trust-failure account; not current state. | 2026-08-04 |
> | [`HOUSINGTEMP-HANDOFF.md`](HOUSINGTEMP-HANDOFF.md) | **A DIFFERENT LANE'S CHARTER** — listed because its name says handoff. | 2026-08-05 |
>
> **Picking this lane up: read this file, then [`LANES.md`](LANES.md) and
> [`DO-NOT.md`](DO-NOT.md).** Older files only for the history each owns.

Written to the §1002 six-section order. Cross-reader: Integrator.
Retired on the user's all-lanes retirement order, 2026-08-17 (Decoder exempt).

---

## 1. WHAT THIS LANE OWNS

Receiver-side WW work: `src/d/` and `src/d/ext_plugin/`, plus the WW half of
`mods-src/ww_donor_disc/`. Housing Security duty rides with it — zone rulings,
strip-set / push-gate questions, and anything asking "may this ship".

**LANES.md was stale on 08-16 and may still be**: it listed Engine as a Cursor
lane and said Housing "builds nothing". Both are wrong. You hold both.

---

## 2. HALF-DONE — the one open item, and it is buildable cold

**The convergence-point collision fix.** NOT started.

- **The defect**: the plugin converts collision data (`dzb`) in a hook keyed on
  the resource NAME. A `dzb` fetched BY INDEX never meets it and reaches
  `cBgW::Set` unconverted — un-rebased offsets resolve against their own field
  addresses. Deterministic garbage, **no crash**, bad AABBs, `-INF` ground.
- **The scale, measured** (Decoder sweep, `decode-drafts/index-fetch-inventory.md`):
  **307 donor TUs fetch by index; 100 pair an index fetch with a collision
  consumer in the same TU.** Per-site guards would be ~100 hand-placed calls that
  each fail silently when forgotten.
- **The shape, settled**: hook **`cBgW::Set`** — the point the donor's own code
  converges on. Every `dzb` passes through it however it was fetched, so it needs
  no proof about the other paths and no future port can miss it. The receiver's
  `ConvDzb` carries a `0x80000000` latch, so conversion is idempotent — that
  property is what makes a `Set` hook safe to adopt without auditing anything else.
- **Already landed as belt-and-braces**: an explicit `ConvDzb` at the one live
  index-fetch site (`registry.cpp`, `WwAkabe_solidHeapCB`).
- **The user's delivery ruling (plugin + patcher, 2026-08-17) makes this
  deliverable**: tree-side code is patcher-delivered, not undeliverable.

---

## 3. DO NOT TRUST — my own wrong published claims

- **"The EVNT name is at 0x04"** — WRONG, it is at **0x01**. I transcribed the
  donor HEADER. Implementing it chops three chars off 57/57 names and matches
  NOTHING — no fault, no log. The bytes settle it; the header contradicts its own
  later fields and its own stated size.
- **"Effective name capacity is 12"** — that reading MAKES AN 11-RECORD DATA LOSS
  INVISIBLE, because nothing ever looks too long. The field is 15 wide.
- **"My offline AABB was wrong"** — WRONG RETRACTION. My numbers matched
  History/Bridge's independent derivation to within the engine's own ±1 padding.
  I withdrew a correct measurement on an assumption about what a tree node covers.
- **"The 31 banner-declared files are a push-gate blind spot"** — premise retracted
  by Foundry (the gate uses a UNION basis) and **my ruling on it is VOID**.
- **"grass-first"** — retracted by Foundry: the receiver ALREADY has a vegetation
  actor covering ~60% of that surface. Do NOT port `d_grass`/`d_flower`/`d_tree`.

---

## 4. LOAD-BEARING ASSUMPTIONS

- **`OFFSET_PTR` fields never take pointers.** I assigned a native array to one
  while the whole board was hunting an offset-vs-pointer bug. Cost: a compile
  error, because that field is TYPED. The same bug reached through a raw buffer
  cost a full day of probing. **Typed fields catch in seconds what raw buffers
  cost a day.**
- **WW-scope at the SOURCE** (`dExtWwSave_isWwHostStage`), never per-consumer
  guards. A catch-all that also catches TP names converts honest failures into
  silent inert props — strictly worse than the bug it replaces.
- **DISCOVER, NEVER AUTHOR.** Resource identity from TYPE, never a hardcoded
  index. Three authored constants were wrong out of three attempts in one night.
- **Parse once** (keyed `(arc,index,raw)`, evicted at BOTH release seams), and
  by-index resources arrive RAW — convert at the fetch site.

---

## 5. THE METHOD LESSON — the whole session in one line

**Nearly every defect — mine and other lanes' — came from reasoning about an
artifact instead of reading it.** A header that lied about a field offset. An
archive index taken from a comment. A metric that measured TU size when we needed
collision content. A tree node whose contents I inferred instead of checking.

The estate's answer, now doctrine: **read the artifact, publish the receipt, and
retract with a search step** — a retraction that reaches only the announcement
leaves the claim alive wherever it was quoted. That bit three lanes.

And its mirror, which cost a measurement we already had: **retraction needs a
receipt too.** Withdrawing on an assumption is the same error as asserting on one.

---

## 6. RULINGS I MADE (Housing Security)

1. **`d_ww_` filename prefix, binding on every ported subsystem file.** The strip
   set is GENERATED from filename convention; a donor-faithful name matches no
   pattern, collides with no receiver file, and **ships to public `main`
   invisibly**. Donor fidelity inside the TU (symbols keep donor names); receiver
   convention on the boundary (filenames). **Recorded durably at `DO-NOT.md`
   DN-11; enforcement verified BY EXECUTION** (the manifest catches
   receiver-named WW legs across 65 files).
2. **Registration always goes mod-side** — it is donor identity content AND it
   must resolve on an uncontrolled binary; both reasons point the same way.
3. **Negative receipts are unprunable** — a `not-ww` row is evidence a question
   was asked and answered; deleting one silently re-opens the ruling.
4. **VOID: the 31-file ruling.** See section 3.

---

## 7. WHAT SHIPPED THIS SESSION

All stamped `COMPILE STATUS` honestly; boot-verified where a boot reached them.

- **EVNT quest chain, end to end** — translator (`ww_stage_loader.cpp`), full-name
  hook, and the name-COMPARISON path routed through six audited consumers. WW
  stage events can reach the event manager for the first time.
- **Warp menu** — flat select restored per the user's ruling, islands in authored
  STORY order (Outset first), spawn defaulting to the measured stand-on-land point.
- **A_mori exit crash** — root-caused to a dangling msg-archive slot, fixed at the
  lifecycle owner. The trigger recurred 5× in one session and was survived each time.
- **Slot-1 owned mount**, **WW room reload table** (DOOR case-trap + TGDR
  inversion), **particle-id arming**, and the derived **name → arc map**
  (`tools/foundry/ww_name_to_arc.py`, 441 TUs, 192 arcs, 18 multi-arc TUs found).

---

## 8. OPEN ON OTHER SURFACES

- **The staged collision receipt is UNREAD.** One boot answers it. The fault is
  bounded: dzb parse, index/tree structure, group pass-check, AABB build, walk
  descent and leaf selection are ALL eliminated by measurement; the crash is
  logged **unrooted with full evidence** rather than given a plausible cause.
- **A tale-boot audit** (msg 3095 in the d09 item box) — runtime-only, never run.
