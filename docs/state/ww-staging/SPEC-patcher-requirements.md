# SPEC — Dusklight patcher requirements, proven by the plugin pilot

> ## ⏱️ MEASUREMENT-DATE STAMP — added 2026-08-17 on the user's supersession order
> **The measurements in this document were taken against a MAY-2026 VANILLA
> DUSKLIGHT.** The tree was **2,861 commits behind origin** (diverged
> 2026-05-08) when they were made. They therefore **PREDATE** origin's
> `SaveService`, `StageService`, `ItemService`, `WindowService`,
> `mods::hook::uninstall`, the embedded symbol database, the **Game ABI/headers
> refactoring** (GameService major 2, 2026-08-12), and open PR **#2270 "Mod SDK:
> Actor service"**.
>
> **NOT DELETED AND NOT WRONG.** These findings were correct for the binary they
> were measured against, and they are the BEFORE side of the comparison the
> post-merge Phase-0 rule depends on (*anything that moves after the merge is a
> merge artifact until proven otherwise*). **Re-measure before citing any number
> here as current.**


**Owner:** Integrator · **Status:** banked, measured on vanilla Dusklight
**Source runs:** `dusklight-20260816-234053` → `dusklight-20260817-001512`

## What this document is

The plugin pilot (`mods-src/ww_donor_disc/`) was built to answer questions the
patcher would otherwise have to discover the expensive way. It answered three
of them on an **unmodified receiver** — no receiver source edits, no rebuilt
executable, no name rows. Those answers are recorded here so they survive the
pilot itself.

It is **not** a record of the actor ports. Those are a separate question and a
separate recommendation (see "What the pilot did NOT settle").

---

## ⚖️ DELIVERY MODEL — RULED BY THE USER, 2026-08-17

> **PLUGIN + PATCHER, COMBINED. The patcher provides all necessary content in
> the receiver's tree; the plugin does what only it can do.**
> *(User ruling, given directly. Foundry was told separately.)*

This settles the question that had been gating several decisions, and it
settles it as **both**, not either:

| | delivers | carries |
|---|---|---|
| **Patcher** | content into the **receiver's tree** | engine logic, ported subsystems, tree-side actors |
| **Plugin** | a **stock, unmodified** dusklight | donor identity content — placement names, arc names, donor bytes (Zone 1) |

### Consequence 1 — the 7-actor tree-side FREEZE IS LIFTED

The freeze existed for one reason: under a plugin-only model, every tree-side
actor was one a stock user could never receive. **The patcher removes that
premise.** Tree-side is now a sanctioned delivery path.

`d_a_ext_ep` · `d_a_ext_plank_span` · `d_a_ext_vegetation` · `d_a_ww_demo00` ·
`d_a_ww_item` · `d_a_ww_shutter` · `d_a_ww_tsubo` — **unfrozen. No migration
to the plugin, and the count may grow again.**

And the migration argument dies with it: rewriting typed receiver actors as
`void*` plugin actors was always trading a compiler for a method with a
**documented four-out-of-four authored-constant defect rate** (dzb index, the
vtable invariant, `TEV_TYPE_BG0`, `UseSingleDL`). Under the combined model
there is no longer any reason to pay that.

### Consequence 2 — the split rule is unchanged and now load-bearing

> **Code goes where DN-11 allows. REGISTRATION always goes mod-side.**

Registration is placement **names** and **arc names** — donor identity content
by DN-11, *and* the part that must resolve on a binary we do not control.
Both reasons still point mod-side, and the combined model does not weaken
either. **The grass split remains the worked example:** engine → tree
(`d_ww_grass.cpp`), its 11 placement names → plugin rows.

### Consequence 3 — R1 is now a saving, not a requirement

R1 proved the patcher needs **zero** `OBJNAME` rows. Under plugin-only that
was load-bearing. Under the combined model the patcher *could* add name rows —
**it simply does not have to**, and every row not added is a receiver edit not
made. The finding keeps its value and loses its fragility.

## R1 — The patcher needs ZERO `OBJNAME` rows

**Proven.** A donor actor spawned, registered collision, and drew, routed
entirely through the existing `dStage_searchName` hook, with **no row added to
the receiver's name table**.

Receipts: `lwood_ready bound=10 missing=0 tev=1` → `lwood_created n=1..4`, all
with `cbgw_set ret=0` and `dbgs_regist ret=0`. Same pattern previously on
`TagSo`/`TagMSo`/`Akabe`.

**Consequence:** the "split" option — adding donor names to the receiver's
`OBJNAME` table — **is dead**. It was the fallback for paths the hook cannot
reach (reverse `dStage_getName`, or code walking the table directly). Nothing
in the Outset load path took those paths.

**Caveat, not yet falsified:** this is proven for the actors exercised. A
consumer that walks the name table directly would still defeat it. No such
consumer has been observed; none has been ruled out either.

---

## R2 — A resource fetched BY INDEX arrives RAW

**Proven, twice, the hard way.**

The plugin's conversion arms are keyed on resource **NAME**. `dComIfG_getObjectRes(arc, index)` bypasses **all** of them. This is a property of
the routing design, not of any one resource class, so it holds for every
resource type — including ones nobody has written a conversion arm for yet.

- **dzb:** handled by calling `ConvDzb` explicitly at the fetch site.
- **model:** NOT handled initially. A raw `J3D2 bdl4` image reached
  `mDoExt_J3DModel__create`, was cast to `J3DModelData*`, and faulted at
  address `0x0`.

**Requirement:** the patcher must convert at the **fetch site**, using the
receiver's own loaders, for every by-index fetch. Never assume a name-keyed
arm ran.

**The failure mode to watch for:** applying this rule to one call and not to
the identical call three lines above. That is exactly how it shipped a crash
here — a half-applied rule looks handled.

---

## R3 — "I parsed it" does NOT mean "it is parsed", and the cache needs a lifecycle

**Proven.** `getObjRes` does not hand back the parsed object. It hands back
**the same raw image on every call**, because the by-index path cannot replace
the arc entry the way the name-keyed arm does.

So the second consumer re-parses the first consumer's buffer — whose vertex
arrays the first parse already endian-fixed **in place**. The format list then
reads as byte-swapped garbage:
`OSPanic J3DModelLoader.cpp:312 "Unable to find vertex attribute format!"`
(a `TARGET_PC`-only path; real GC hardware never runs it). This is **DN-3**.

*Falsified against the data, not assumed:* `alwd.bdl` was extracted from the
disc and its VTX1 block dumped directly. All three arrays (POS/CLR0/TEX0) DO
carry format entries. **The file is innocent and the receiver is innocent** —
the corruption happened between the two passes.

**Requirement — memoise, and give the memo a lifecycle:**

1. Key on **`(arc, index, raw)`**, all three. An address alone is not a
   durable key: free the arc, a later allocation reuses the address, and the
   table hands out a `J3DModelData*` into reused memory.
2. **Evict at the release seams.** Per-arc immediately before every
   `dComIfG_resDelete`, and the whole table on stage load. (2) covers what (1)
   structurally cannot: the *same* arc unmounted and remounted at the *same*
   address, where every key field still matches but the parsed object is dead.

**The exposure is ordinary play, not an edge case:** `daLwood_c::_delete`
(`d_a_lwood.cpp:162`) calls `dComIfG_resDelete` on **room exit**.

**Cost asymmetry, which is why both guards ship:** re-parsing a few models on a
stage change costs nothing measurable. A stale `J3DModelData*` costs a
corruption two stages later that looks like anything but this — which is
exactly how the room-lane mesh corruption presented.

**Prior art the donor already had:** `ConvDzb`'s `0x80000000` latch **is** this
guard, done natively. The collision path was safe the whole time for this
reason; the model path simply lacked the equivalent. DN-10 step 1 would have
handed us R3 before the crash rather than after.

---

## R4 — A named donor constant is DATA. Read it; never infer it.

Not a patcher mechanism — a **process requirement**, earned at a rate of three
in one session:

| Constant | I wrote | Truth | Authority |
|---|---|---|---|
| `alwd.dzb` resource index | 5 | **7** | `Lwood.arc`'s own RARC file table |
| "parsed head is a vtable, so no re-parse" | invariant | **false** | `getObjRes` returns the raw image |
| `TEV_TYPE_BG0` | 0 | **1** | donor `d_kankyo.h:135` (0 is `TEV_TYPE_ACTOR`) |

**THE ARC IS THE AUTHORITY** for resource indices — there is no generated
header for lwood (`d_a_lwood.h` is 34 lines with no index enum). The index is
the file's position in its own RARC table. Instrument: `tools/foundry/arc_index.py` (History/Bridge → Foundry).

**And the habit underneath all three:** validating the *condition* without
reading the *code that consumes it*. The winding question came out right
precisely because both the helper (`cM3d_CalcPla`) **and** the call site
(`SetupFrom3Vtx`) were read — a swapped call site would have flipped the sign
regardless of the helper. Read the consumer. Make it the default.

---

## R2 (amended) — a conversion that is a SEQUENCE is not satisfied by its first step

The original R2 said "convert at the fetch site with the receiver's own
loaders." That was right and too loose: I used **one** of them.

`d_resorce.cpp` converts a model in four steps, and **every arm is identical**:

```
load / loadBinaryDisplayList(res, flags)
  → newSharedDisplayList(J3DMdlFlag_UseSingleDL)   // 0x40000
  → simpleCalcMaterial(j3dDefaultMtx)
  → makeSharedDL()
```

The receiver `return -1`s if `newSharedDisplayList` fails. A port must refuse
identically rather than hand a half-built model onward.

## R5 — port the donor's TRANSLATION UNIT, not its METHOD TABLE

`daLwood_c::CreateInit()` is **not** in `daLwood_METHODS`; it is called from
`_create`. A method-table-shaped reading of the donor misses it entirely —
which is how a port that was "verbatim" for `_create`/`_execute`/`_delete`/
`_draw` silently dropped `setBaseScale`, `setBaseTRMtx`, `mModel->calc()`,
`fopAcM_SetMtx`, the cull box, and the cull-far.

## R6 — "bound" measures AVAILABILITY, not COVERAGE

`lwood_ready bound=10 missing=0` printed on every boot while
`setBaseTRMtx`, `fopAcM_setCullSizeFar`, `cM_rndF` and `dBgW::Move` were
**bound and never called**. The symbols resolved; the calls did not exist. A
receipt that counts resolved symbols says nothing about whether the port uses
them, and it was read as completeness for four boots.

**Any instrument reporting a bind/resolve count must be checked for this
reading.** Coverage requires a call-site count, not a symbol count.

## What the pilot did NOT settle

### OPEN DEFECT — the lwood draw crash, UNROOTED. Closed out 2026-08-17 by the user's call.

Six boots. Recorded here so the next lane starts from the evidence rather
than from the beginning.

**Symptom (deterministic, reproduced on every submitting boot):** four
`lwood_exec` and four `lwood_draw_phase … done` complete — one full frame —
then the process dies before frame 2 with **no CPU exception, no `Fault
addr`, no `rva=`, no dump**, from a handler that *did* report the earlier
access violations.

**Established POSITIVE:**
- Not submitting the model → 15+ frames, clean shutdown. **Collision is
  exonerated**: four MOVE_BG BgWs, with the per-frame rebuild
  (`d_bg_w.cpp:788`) running every frame, are fine.
- **The room's own WW models render.** `model.bdl`/`model1.bdl`/`model2.bdl`
  from `R44_00` are parsed by the same loader at the same flags (`0x1010`)
  and drawn by the receiver's own `daBg` **in the same run that our model
  kills**. A WW BDL is not inherently unrenderable on this backend.

**Established NEGATIVE — two real defects found and fixed, symptom unchanged:**
- the truncated conversion sequence (R2 amended) — `sharedDL:0`, binds
  `1111`, still died;
- the unported `CreateInit` (R5) — `scale:1 trmtx:1 calc:1`, still died.

**The untested split, and where the next lane should start:** our **draw
sequence** versus the receiver's. A four-mode bisect is staged in the plugin
(`s_lwoodDrawMode`: 0 full · 1 none · 2 `modelUpdateDL` only · 3 tev+lists
with no submit) — one boot per mode, no rebuild.

**Backend note:** the renderer is Dawn/WebGPU. A device-lost kills the process
with no CPU exception, at submission, after every draw call has returned —
which matches the signature exactly. **"No fault marker" is evidence, not
missing evidence.**

**Process lessons that cost the most here:** a bisect indicting a *path* does
not identify a *line* (I promoted a real defect to *the* defect twice without
a confirming run), and a log read while the process is still writing it reads
exactly like an early crash (2,338 lines vs 3,505 final).
- **The BgW target was mis-derived.** "3 → 16" came from fork-vs-vanilla
  *placement* deltas, and placements are not collision. Of the five remaining
  minimum-set actors, only `knob00` registers a `dBgW` at all
  (`tsubo` 3,647 lines → **zero**). The fork's 184/192 completion figure is
  also inflated by a deliberate fork divergence
  (`d_a_ext_vegetation.cpp:995`, "NEVER return `cPhs_ERROR_e`"), so it is not
  a donor-true target.
- **"Born-dead" actors are correct behaviour.** Donor `daGrass_Create` returns
  `cPhs_ERROR_e` on every path; the receiver's is a singleton-manager doing the
  same. 118 placements with 99 dying is the expected shape, not a defect.

## Recommendation on actor ports

**Stop the actor-by-actor plugin ports.** They rebuild donor actors out of
`void*`, hand-measured PDB offsets, and hand-authored constants, with no
compiler checking any of it — and the R4 table is what that produces. The fork
tree already carries several of these natively and in typed form
(`d_a_knob00.cpp`, 1,685 lines behind `DUSK_WW_KNOB00_NATIVE`;
`d_a_ext_vegetation.cpp`, 1,968 lines). Actor work belongs there.

R1–R4 are the pilot's deliverable, and they are complete.
