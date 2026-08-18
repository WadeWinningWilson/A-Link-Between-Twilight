# SPEC — the WW generic prop handler ("catch-all")

> # ⛔ SUPERSEDED — DO NOT BUILD FROM THIS DOCUMENT
>
> **Status: DROPPED by user ruling, 2026-08-17.** *"we're not doing the catch
> all, after all."* `lwood` was reverted to `src/d/actor/d_a_lwood.cpp` as
> canonical and the plugin port was removed.
>
> **AND ONE RECOMMENDATION IN HERE WAS RETRACTED BEFORE THE DROP — it is the
> dangerous one, because it reads as an instruction:** the ruling below to
> *"PORT `d_grass` + `d_flower` + `d_tree` FIRST — the best single trade in
> the room"* was **RETRACTED BY FOUNDRY**. `src/d/actor/d_a_ext_vegetation.cpp`
> already serves **112 of the 187** grass placements (60%), and a wholesale
> port would plant receiver 31×31 grass on a donor island — **a DN-11 §31-B
> art breach**. *"Shared architecture is not shared assets."* The corrected
> step, if the work is ever resumed, is a **bounded extension of
> `d_a_ext_vegetation` to 8 remaining names (75 placements)** — not a
> three-subsystem port.
>
> **WHAT STILL STANDS — the MEASUREMENTS, never the directives:** the 87-names
> → **61 procs** collapse; the three-class split (**A** arc-backed 187 · **B**
> logic-tag 101, correct by doing nothing · **C** engine-batched 187); that
> class C cannot be served by a five-step prop body at any coverage; and the
> `name → arc` derivation. **Read this as a survey, not a plan.**
>
> *(Banner added on the Librarian's announce-vs-record doctrine, clause 3 —
> "when you find one, sweep the class." This document was found still issuing
> a retracted instruction during that sweep.)*

**Owner:** Integrator · **Status:** ⛔ SUPERSEDED (see banner) · **Origin:** the
user's proposal, 2026-08-17 — *"a 'parser' that catches all the wind waker
actors"* rather than one receiver actor per WW actor.

## The intention, restated mechanically

Do not add N donor actors to the receiver tree. Add **one** generic actor that
any unrecognised WW placement routes to, which does the things that are the
same for every static prop, and discovers everything else from the data.

**This is a good instinct and it is stronger than a labour saving.** See
"Why this removes a bug class" below — it deletes the exact failure that
produced three crashes on 2026-08-17.

---

## What is actually common to WW props

Reading the two ports already done (`akabe`, `lwood`) plus the donor sources,
a static prop's create/draw is the same five steps every time:

1. `dComIfG_resLoad(&phs, <arc name>)`
2. get the model resource → parse if raw → `mDoExt_J3DModel__create`
3. get the `.dzb` if the arc has one → `dBgW_NewSet(dzb, MOVE_BG_e, &mtx)`
4. build the matrix from the placement: `transS(pos)` · `ZXYrotM(angle)` ·
   `scaleM(scale)`
5. draw: `settingTevStruct(TEV_TYPE_BG0, &pos, &tevStr)` →
   `setLightTevColorType(model, &tevStr)` → `setListBG()` →
   `modelUpdateDL(model)` → `setList()`

`daLwood_c::_draw` **is** step 5 verbatim. `akabe` is steps 1–4 with no model.
Neither actor does anything in `_execute` but increment a timer.

**That five-step body is the catch-all.**

---

## Why this removes a bug class (the real argument)

The generic handler must not be told which resource index is the model and
which is the collision — **it walks the arc and finds them by type**: first
entry whose data begins `J3D2` is the model; first entry whose name ends
`.dzb` is the collision. Absent either one, that step is skipped.

That single decision deletes **R4** (`SPEC-patcher-requirements.md`) — the
authored-constant failure — for every actor at once:

| Authored constant | What went wrong | Under the catch-all |
|---|---|---|
| `alwd.dzb` index = 5 | truth was 7; crash | **discovered**, never authored |
| `alwd.bdl` index = 4 | right by luck | **discovered** |
| model arrives parsed | arrives raw; fault at `0x0` | **type-tested at the fetch site** |

Per-actor ports re-author these constants for every actor and get a fresh
chance to be wrong each time. The catch-all authors them **zero** times.

R2 and R3 still apply and are already solved: convert at the fetch site, and
memoise with `(arc, index, raw)` keying plus eviction at both release seams.

---

## What it cannot do, and the rule that makes that safe

A generic handler cannot supply **behaviour**: state machines, animation,
interaction, item drops, damage response, switch/door logic, cutscene binding.
So under the catch-all, a pot is a pot you cannot break and a door is a door
you cannot open.

**The rule that makes this acceptable — FAIL INERT, NEVER ABSENT.** The
handler must never return `cPhs_ERROR_e`. An actor that cannot do its job
still stands in the world, holds its collision, and occupies its placement
slot. The fork already learned this the hard way and wrote it down at
`d_a_ext_vegetation.cpp:995`:

> NEVER return `cPhs_ERROR_e` from create. […] Fail INERT instead.

(In the fork the reason was a spawner FIFO identity shift; here the reason is
simply that a missing prop is worse than a static one.)

**Behaviour-bearing actors are then an upgrade path, not a prerequisite:**
each real port replaces one inert prop with a live actor, and the stage is
walkable the whole time instead of only at the end.

---

## Receiver-side cost — the part the user cares about

Two things, both bounded:

1. **One profile.** A single `actor_process_profile_definition` with generic
   methods. Not one per actor.
2. **One fallback in name resolution.** Any name the receiver does not know,
   on a WW host stage, resolves to that profile instead of failing.

That is the whole receiver patch for the static-prop class — **constant size,
independent of how many WW actors exist.** Compare: the per-actor path costs
944 lines for `knob00` and 3,647 for `tsubo`, each needing its own review.

**WW-scope it at the source** (standing rule): the fallback must be gated on
`dExtWwSave_isWwHostStage`, so an unknown name on a mainline TP stage keeps
failing exactly as it does today. A catch-all that catches TP actors too would
convert honest failures into silent inert props — the opposite of useful.

---

## What this does NOT fix, stated plainly

**It is not a crash fix.** The generic handler performs the same five steps
lwood performs, so the currently-unrooted crash (deterministic, reproduced on
two consecutive boots, no fault marker, occurring after the first full frame
of four lwood draws) would most likely reproduce under it unchanged.

The catch-all solves **scaling** — how many actors we can support for how much
receiver code. It does not solve **this defect**. Those are independent, and
the crash still needs rooting on its own terms.

---

## 📍 WHERE THIS RUNS — the question this spec forgot to answer

**Foundry, 2026-08-17:** this document contained **zero occurrences of `mod`,
`plugin`, or `hook`.** It specified a five-step body in detail and never said
its home. Verified: the count was zero. That omission is mine, and it decides
whether the thing reaches a user at all.

### There are TWO delivery models, and the spec silently assumed one

| | reaches | carries |
|---|---|---|
| **Plugin** (`mods-src/`, above-enum + 2 hooks) | a **stock, unmodified** dusklight | donor identity content is allowed here (Zone 1) |
| **Patcher** (fork tree, `d_ww_*`) | a binary **we modify** | engine logic only (Zone 2), stripped before push |

Tree-side actors are **not** undeliverable — they are deliverable *by patcher*.
Foundry's axis ("does this have to work on a binary we do not control?") is
right, but it collapses the two models into one and reads tree-side as dead.

### THE RULE — split at code vs REGISTRATION, not per subsystem

> **Code goes where DN-11 allows. REGISTRATION always goes mod-side.**

Registration is placement **names** and **arc names** — donor identity content
by DN-11 — *and* it is the part that must resolve on a binary we do not
control. So it belongs in the plugin's name table regardless of where the code
lives.

Worked through:

- **Grass:** engine → **tree** (`d_ww_grass.cpp`, Zone 2, pure geometry/draw).
  Its 11 placement names (`kusax1`, `kusax7`, `flower`, `swood`, …) → **plugin
  name rows**. One subsystem, both homes. Same split the audio stack already
  runs: JA1 engine in-tree, donor BMS/IBNK bytes never in-tree.
- **Generic prop handler:** its whole *input* is names and arcs, so its
  registration is mod-side necessarily. Its **body** may live either side —
  that follows the delivery decision below, not this rule.

### Existing precedent, measured

The plugin's `kObjectNames[]` table (`registry.cpp:2662–2711`) is **5 rows**
(`WwPilot`, `TagSo`, `TagMSo`, `Akabe`, `lwood`), plus `kAkabeArcs[4]` at
`:1776`. **Extend that table; do not derive a second map.**
*(Foundry cited 45 rows; the measured count is 5. The pattern is real and
established — its scale is pilot, not production.)*

### OPEN — the 7 tree-side actors: FREEZE, do not migrate

`d_a_ext_ep` · `d_a_ext_plank_span` · `d_a_ext_vegetation` · `d_a_ww_demo00` ·
`d_a_ww_item` · `d_a_ww_shutter` · `d_a_ww_tsubo` (verified: exactly 7).

**Ruling: do not migrate them, and do not add an eighth, until the delivery
model is chosen.**

- Migrating means rewriting typed receiver actors as `void*` plugin actors —
  **precisely the method that produced four authored-constant defects out of
  four attempts tonight** (dzb index, vtable invariant, `TEV_TYPE_BG0`,
  `UseSingleDL`), because no compiler checks any of it.
- The patcher makes tree-side deliverable, so migrating now optimises for a
  constraint that may not apply.
- But Foundry's warning stands: **every tree-side actor added is one a stock
  user cannot run.** Hence freeze rather than either migrate or grow.

## ⚖️ FOUNDRY'S RULING (2026-08-17) — measured, and it supersedes the estimates below

**Verdict: the catch-all is not void, but it is not the main event.** It can
reach **at most 39%** of Outset, and only if given an input this spec never
specified. The highest-ROI item in the room is not the catch-all at all.

### The collapse that reframes the cost model

`dStage_objectNameInf` is `{ char name[8]; s16 procname; s8 argument;
s8 gbaName }` — **there is no arc field.** A name maps to a **procname**, and
procnames are **shared**: `kusax1`/`kusax7`/`kusax21`/`flower`/`flwr7`/`swood`
are all one `fpcNm_GRASS_e`.

**87 names · 475 placements · but only 61 distinct procs.**
**Port cost scales with PROCS, not names.**

### The three classes (the arc / no-arc binary was the wrong cut)

| Class | Procs | Names | Plc | Catch-all? |
|---|---|---|---|---|
| **A · arc-backed** | 35 | 42 | 187 (39%) | ✅ **only with a name→arc map** |
| **B · logic tag** | 25 | 34 | 101 (21%) | ✅ **already correct, build nothing** |
| **C · engine-batched** (`GRASS_e`) | 1 | 11 | 187 (39%) | ❌ **cannot serve at any coverage** |

**Class C is the finding.** `d_a_grass.cpp` loads **no arc** and builds **no
J3D model** — it registers into `d_grass`/`d_flower`/`d_tree`, which draw
through `GXLoadPosMtxImm` with their own vertex data. The five-step body has
nothing to consume.

> **⚠️ CORRECTION (Foundry, later the same day): class C is NOT UNSTARTED — it
> is 60% ALREADY SERVED.** `src/d/actor/d_a_ext_vegetation.cpp` covers
> `kusax1` (46) · `kusax7` (30) · `kusax21` (36) = **112 of the 187**. The
> paragraph above is correct that a prop body cannot serve class C, and was
> written as though nothing served it. **Both halves matter: the catch-all
> still cannot reach it, AND the gap is 75 placements, not 187.**

### The ruling, in ROI order

1. **Port `d_grass` + `d_flower` + `d_tree` FIRST.** 479+585+687 lines, **one
   subsystem**, lighting up **187 placements / 39% of Outset**. The best
   single trade in the room — and it was not in the catch-all plan at all.
2. **Build the catch-all, with its missing step 0:** a `name → arc` map
   **derived mechanically from donor source** (`m_arcname` statics,
   `dComIfG_getObjectRes("<arc>", …)` literals). That is *discovery from donor
   data*, not authoring. **Without it, `resLoad(<arc>)` has no input and
   coverage is 15%, not 39%.**
3. **Ship nothing for class B.** It works by doing nothing — the spec's one
   unambiguous win.

### Q1 corrected — the discriminator is CLASS, not animation

Foundry withdrew a rule they had drafted for Housing (*"an arc with a `.bck`
is expected to move, so static reads as broken"*): **it scores GRASS as a
correct no-op.** Grass has no `.bck`, no arc and no model, and its absence is
the *most visible* failure in the room at 187 placements.

- **Class B** is unobservable by construction.
- **Class C** is observable regardless of animation.
- **Only inside class A** does the `.bck` test do real work.

Doors keep their soft-lock carve-out independently.

### Q2 — `.bck` counts, and they are an UPPER BOUND

27 class-A arcs carry `.bck`/`.bca`, 18 carry none — **flagged as an upper
bound by Foundry themselves**: attribution is per-TU, so a TU merely
mentioning another proc inherits its arcs (`Link`, `Always` are that
contamination, not Outset props). Honest read: **animated arcs are roughly
half of class A, ~90 of 475 placements.**
**Design the animated path as the special case, not the main path.**

### Q3 — max-size single profile, confirmed

Content classes are model-only / model+dzb / model+bck; the deltas are a
pointer and a frame counter. Sibling profiles buy bytes and cost a second
create-time decision made from discovered content — a second place to be wrong
about discovery, in a design whose whole value is discovering right once.

---

## A room full of items, props and NPCs — what actually happens

The user's question, answered by class rather than in aggregate, because the
aggregate answer ("mostly yes") hides the one case that matters.

| Class | Arc contains | Catch-all result | Verdict |
|---|---|---|---|
| Static prop (trees, rocks, signs, houses) | model + `.dzb` | model drawn, collision registered, placed | ✅ **fully handled** |
| Decoration | model, no `.dzb` | drawn, walk-through | ✅ handled |
| Logic tag / trigger | neither | clean no-op | ✅ safe, inert |
| Field item (rupee, heart) | model | drawn, **not collectable** | ⚠️ cosmetic only |
| Door / knob | model + `.dzb` | **solid, never opens** | 🔴 **soft-lock risk** |
| NPC / enemy | model + `.bck` + more | **bind-pose statue** | 🔴 **worse than absent** |

**Props are the good news and they are the bulk of a room.** Everything that
makes a stage *stand up and be walkable* is the first row, and the catch-all
gets it completely — that is exactly what `lwood` proved.

**NPCs are where the limit is sharp, and the failure is ugly rather than
quiet.** An NPC without its animation step is not "an NPC that does nothing" —
it is a T-posed mannequin. Absent reads as unfinished; a statue reads as
broken. Same for a door that is solid and never opens: that is not a missing
feature, it is a **potential soft-lock** if the door gates progress.

### Consequence: the catch-all needs TIERS, not one behaviour

And the tier must be **discovered from the arc's contents**, never authored —
the same principle that deletes R4:

- **Tier 0 — skip.** Nothing usable in the arc, or a class where inert is
  actively harmful. Do not spawn. *Absent beats broken.*
- **Tier 1 — static prop.** Model + optional `.dzb` + transform + draw.
  The five steps above. Covers rows 1–3 of the table.
- **Tier 2 — animated prop.** Tier 1, plus: if the arc carries a `.bck`,
  bind it and loop it. **This is the cheap, high-value addition** — it costs
  one more discovery-by-extension and one anim update per frame, and it turns
  every NPC statue into an idling figure. It supplies no behaviour, but it
  removes the "broken" reading entirely.
- **Tier 3 — real port.** Behaviour. One actor at a time, replacing a Tier 1/2
  stand-in without the stage ever being un-walkable.

**Tier 2 is the answer to "a room full of NPCs."** Not because it makes them
work — it does not — but because it is the difference between a room that
looks unfinished and a room that looks wrong.

### Does it scale to a full room?

Yes, and the two mechanisms are already built:

- **Arc sharing.** Four `lwood` instances loaded `Lwood.arc` **once**
  (`dComIfG_resLoad` refcounts). N props of the same kind cost one arc.
- **Model sharing.** `wwParseModelOnce` returned one parsed model for all four
  instances — verified `model_parse_once n=1` against `lwood_created n=4`.

So per-*kind* cost is constant and per-*instance* cost is an actor struct plus
a `dBgW`. A hundred props of ten kinds is ten arcs, not a hundred.

**Unmeasured:** heap headroom for many distinct arcs at once, and draw-call
cost at high instance counts. Neither has been tested past four.

## Open questions before building

1. **Name → arc.** WW placements carry an 8-byte name; the object arc is
   usually named for it (`lwood` → `Lwood.arc`), but the casing rule and the
   exceptions are unmeasured. **Needs a census before it is a rule.**
2. **Cull box.** A generic `fopAc_CULLBOX_*` value will be wrong for very
   large or very small props. Start permissive; tighten from measurement.
3. **Params.** Some actors encode meaning in placement params (grass kind,
   pot contents). The generic handler must **ignore** params rather than guess
   at them — a wrong guess is worse than inert.
4. **Actors with no model and no dzb** (pure logic tags) must no-op cleanly,
   not fault.
