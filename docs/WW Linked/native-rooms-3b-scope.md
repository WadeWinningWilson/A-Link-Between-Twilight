# Native Rooms — Option 3b (LOADER PORT) — SCOPE

> **Status:** scoped, not started. Foundry, 2026-08-08. Ruling that authorised it:
> user, "that ranking sounds great, scope 3b" (bus §605).
> Receipts: §602 (donor path read), §603 (lineage measured), §604 (V1's `SCLS` hit),
> §605 (3a-vs-3b cost split).

## 0. What 3b is, in one paragraph

Port Wind Waker's **readers** — `dStage_dt_c_roomLoader`, its chunk `FuncTable`,
and the `ROOM_SCENE` phase chain — so that **donor code reads donor bytes and
writes the RECEIVER's in-memory structures.** The ~880 receiver call sites that
consult `dComIfGp_getStage()` / `dComIfGp_roomControl_` keep working untouched,
because the structures they read are the ones they have always read.

**Why not option 2 (the current Space Kit):** option 2 performs the same
translation at BAKE time, in Python, from our understanding of the formats.
3b performs it at LOAD time, in ported donor C++, from the donor's own parsing
semantics. Under the always-native covenant that is the difference between a
bridge and an endpoint.

**Why not 3a (true parallel stack):** 3a is more native still, and its cost is
dual-pathing ~880 consumer sites. 3b's cost is bounded by the loader surface.
3a remains the recorded endpoint; 3b is a real step toward it, not a detour,
because every ported loader is a loader 3a would need anyway.

## 1. The unit of work, measured

`dStage_dt_c_roomLoader` on both sides:

| | donor (WW) | receiver (TP) |
|---|---|---|
| location | `D:/XXXXXXX/WW DP/src/d/d_stage.cpp:2180` | `src/d/d_stage.cpp:3035` |
| chunks | **22** | **14** |
| signature | `(void*, dStage_dt_c*)` | `(void*, dStage_dt_c*, int roomNo)` |
| after decode | *(nothing)* | `layerTableLoader(...)` |

### 1a. SHARED — 14 chunks, both sides have an init function of the same name

```
PLYR  RCAM  RARO  RTBL  AROB  Virt  SCLS
LGHT  RPPN  RPAT  FILI  FLOR  LBNK  SOND
```

Every chunk in TP's room table is also in WW's. **TP's room surface is a subset
of WW's.** That is the single most encouraging fact in this scope: nothing in
the receiver's room loading has no donor counterpart.

Per-chunk work for these = **verify the record layout, then port the reader**.
V1 (`tools/foundry/accessor_differ.py`) is the verification instrument.

### 1b. WW-ONLY — 8 chunks TP's room loader does not handle

```
2Dma   2DMA   Pale   Colo   EVNT   EnvR   LGTV   SHIP
```

These are **not** necessarily missing from the receiver — several (`Pale`,
`Colo`, `EnvR`, `LGTV`) exist in TP at **stage** scope. So the question per
chunk is *"does `dStage_dt_c` have room-scope storage for this, or only
stage-scope?"* — **OPEN, one read each, not yet done.**

`SHIP` is WW-specific gameplay (sail spawn points) with no TP analogue at all.

### 1c. The one ARCHITECTURAL difference, not a layout one

```
WW    EVNT  in roomLoader        (rooms carry events directly)
TP    REVT  in roomReLoader      (dStage_mapEventInfoInit, a different phase)
```

This is exactly the `EVNT → REVT` translation the Space Kit already performs at
bake time. Under 3b it becomes a loader-level decision, and it is the **one place
where 3b cannot be a pure port** — the donor reads an event table in a phase the
receiver does not read one in. Flagged as the scope's principal design question.

## 2. Known layout findings so far (V1)

| chunk | donor | receiver | verdict |
|---|---|---|---|
| `ACTR` | 0x20 | 0x20 | **AGREE**, validator confirms |
| `PLYR` | 0x20 | 0x20 | AGREE (validator template unparseable) |
| `SCOB` | 0x24 | 0x24 | **AGREE**, validator confirms |
| `SOND` | 0x1c | 0x1c | **AGREE**, validator confirms |
| `MECO` | 0x02 | 0x02 | **AGREE**, validator confirms |
| `SCLS` | **0xC** | **0xD** | **DISAGREE — `mWipe` 0xA → 0xC** |
| `RTBL` | 0x8 | 0x8 | validator CONTRADICTS (C=0x4) — open |
| `EVNT` `FILI` `SHIP` | — | — | uncomparable, receiver names unresolved |

**4 of ~34 stage/room chunks cleared. One hit. The map is small and says so.**

## 3. Work breakdown

### Phase 0 — finish the layout audit (V1) — PREREQUISITE
Extend V1's `BLOCK_MAP` to the 22 room chunks; resolve the 3 uncomparable
receiver struct names; fix the `LBNK`/`MEMA` container-vs-record rows; route
`RTBL`'s validator contradiction. **Every block cleared here is a difference the
loader will not discover at runtime.** No loader code should be written against
an unaudited chunk.

### Phase 1 — the dispatch seam
Add a WW-scoped branch at the room-load entry, using machinery that already
exists:
- `dStage_Create()` has **one caller**, `src/d/d_s_play.cpp:1678`
- `dExtWwSave_isWwHostStage()` is the established runtime predicate
- `src/d/d_stage.cpp` **already carries 5 WW-scoped hooks** (№93 RTBL clamp,
  residency clamp) — this is not a new pattern in this file
Deliverable: a `ww_` room loader entered only on a WW host stage, initially
delegating 100% to the receiver's existing loader. **Lands inert; changes
nothing; provable by inspection.** (Same shape as step 19 Phase 1's shim, which
is the precedent for landing a seam before behaviour.)

### Phase 2 — port the SHARED 14, one chunk at a time
Each chunk: port WW's init function under a `ww_` name, writing the receiver's
struct. Route through the Phase-1 seam one chunk per step. `SCLS` is the
**first** one worth doing, because it is the only chunk with a *known* layout
difference and therefore the only one where the port is currently provably
better than the bake.

### Phase 3 — rule on the WW-ONLY 8
Per chunk: room-scope storage exists / stage-scope only / absent. Then port,
relocate, or declare out of scope. `SHIP` is likely "out of scope until sailing".

### Phase 4 — the `EVNT`/`REVT` question
The one genuine design decision (§1c). Not a port; a ruling. Should not be
attempted before Phases 1–3 make the surrounding shape concrete.

## 4. What this scope does NOT cover

- **`d_s_play.cpp`'s stage lifecycle** — unread. Where `dStage_Create` sits in
  the play scene's phase chain is not established.
- **Heap / zone ownership** — `dStage_roomControl_c::createMemoryBlock`,
  `JKRExpHeap` per room, the zone counter. Unread on both sides.
- **`dMap_c`** — 2,211 donor lines. Unexamined; may be independent, may not.
- **The `ROOM_SCENE` phase chain itself** — §602 mapped it; porting it is a
  separate unit from `roomLoader` and is NOT in this scope.
- **Whether 3b ever completes into 3a.** Recorded as the endpoint; not planned.

## 5. Kill switch and reversibility

Every phase lands behind `dExtWwSave_isWwHostStage()`, so mainline TP is
untouched by construction. Per the WW shared-path scoping rule, the gate is
runtime and at the source, not a per-consumer null-guard. Phase 1's seam is
inert by design, and each Phase-2 chunk is independently revertible by removing
its row from the `ww_` FuncTable — the same granularity step 19 used to sever
129 symbols in five reviewable steps.
