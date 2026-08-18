# DECODER — the lane briefing

> Chartered by the user, 2026-08-17, on History/Bridge's precedent
> conversation ("Do we need a separate lane as the decoder — of course
> tracking their work so finished official decomp rules their work
> out/confirms it") and the user's authorization of the PowerPC toolchain
> download. Written by Foundry. **This is the lane's founding document;
> corrections to it follow the estate's rule — supersede with a receipt,
> never silently.**

## 1. The charter, in one paragraph

The Decoder lane turns **drafted donor reconstructions into typed, named,
verifiable code** — and holds every draft under the user's addendum:
*our own decoding may be used, PROVIDED it is traced and ultimately
replaced by the official decomp team's work.* The lane exists because 655
drafts with nobody's name on them will rot, and because `decomp_watch`
emits confirm-or-replace triggers that somebody must act on. **An unowned
trigger is the failure this estate keeps rediscovering; this lane is that
owner.**

## 2. What the lane is graded on — the two tests, by name

The phrase "vanilla byte true" is RETIRED (SPEC-byte-true-verified §3b).
Reviews name their axis:

- **(A) THE DECOMP TEST — the BYTE-TRUE axis, donor-side.** The C++
  compiles under the donor's own PowerPC toolchain and the REL is
  SHA1-identical to retail (`D:/XXXXXXX/WW DP/config/GZLE01/build.sha1`,
  one hash per REL). **Toolchain: `D:/XXXXXXX/ppc-compilers-20251118/`
  (user-authorized download, labeled by tag, outside both git trees).**
  Configure with `--compilers D:/XXXXXXX/ppc-compilers-20251118`. A
  SHA1-clean REL derives `BYTE-TRUE` with `src=ppc-rel-sha1:<rel>@<hash>`.
  **An (A)-clean function IS the decomp and belongs upstream in
  zeldaret/tww — drain-by-upstreaming is this lane's stated end-state,
  not a surprise at delivery.**
- **(B) THE PORT TEST — the VERIFIED axis.** The x86-64 port behaves
  identically, judged by the oracle stack (probe differ, state taps,
  golden traces) at the consumption boundary. **This is what the port
  needs**; byte-identity to a GameCube REL is not a property our binary
  can have. History/Bridge operates this review today.

A review that says "byte true" without naming (A) or (B) is refusable on
its face.

> **USER RULING 2026-08-17 — (A) IS THE LANE'S SOLE GRADING.** *"A is the
> way and only focus."* The Decoder lane aims every completed TU at a
> SHA1-clean REL under the donor's own toolchain; History's 20-batch
> reviews grade progress toward (A). (B) remains the PORT's acceptance
> test (Housing's consumption boundary) — it does not grade this lane.
> Consequence embraced, not feared: (A)-clean work is the decomp and
> flows upstream (drain-by-upstreaming), which is the lane's purpose.

### 2b. CHARTER CORRECTION — (A) IS NOT BINARY. THE ENUM HAS SIX VALUES.

**Amended 2026-08-17 on the outside review.** The charter as first written
made (A) SHA1-clean-or-nothing, which leaves a functionally-correct TU
blocked on register allocation with **nowhere to go** — and a lane with
nowhere to go grinds indefinitely. Upstream's own status vocabulary is the
authority and it has six values; all six enter this charter:

| status | meaning | when the lane files it |
|---|---|---|
| `Matching` | byte-identical, all versions | the goal |
| `Equivalent` | **functionally correct, not byte-exact** | accepted upstream *with an explanatory comment*; tww's `regalloc` label currently parks **7 TUs** here. **This is a real outcome, not a failure** |
| `MatchingFor(...)` | byte-identical on the listed versions only | version-scoped; ours is GZLE01 |
| `EquivalentFor(...)` | equivalent on the listed versions | as above |
| `NonMatching` | not byte-matching | the start state |
| `DEBUG_ONLY` | debug-build symbol | rare |

**Rule: escalate to `Equivalent` with a written reason rather than grind.**
A TU parked at `Equivalent` for a named regalloc reason is progress
recorded honestly; a TU ground on for days for a byte is progress lost.

> 🔴 **"EQUIVALENT" NAMES TWO DIFFERENT THINGS AND ONLY ONE IS OURS TO
> ASSERT** (History/Bridge, 2026-08-17 — correcting the table below,
> which as first written conflated them):
> - **UPSTREAM `Equivalent` is CONFERRED** — maintainers accepted the
>   source with an explanatory comment. **We never assert it. We record
>   it verbatim from `configure.py`.**
> - **OUR `byte-true: EQUIVALENT` is ASSERTED** — §2b entitles the lane to
>   it when the park conditions are met.
>
> **These come apart constantly.** `daLodbg_c::loadModelData` is
> **upstream `Nonmatching - regalloc`** and **ours `byte-true:
> EQUIVALENT`** — both true, neither implying the other. **A row files
> OUR assertion and records UPSTREAM'S STATUS VERBATIM beside it**, so
> the official decomp's own progress can later confirm or deny us. That
> separation IS the labelling the user asked for; collapsing it would
> report a maintainer's acceptance we never received.

**HOW EACH UPSTREAM STATUS FILES ON OUR TWO AXES** — this table reads
**upstream's CONFERRED status → how we FILE it.** It does not run
backwards: our asserted `EQUIVALENT` does NOT mean upstream conferred
`Equivalent` (see the box above). Added 2026-08-17 after History/Bridge
proved the `Equivalent` outcome is already fileable — no schema change
needed, and here is the second reason why: version-scoping collapses six
upstream values into our four, and `decomp_status.py` already implements
exactly this:

| upstream status | `portable:` | `byte-true:` |
|---|---|---|
| `Matching` | `DECOMPILED` | `BYTE-TRUE` (+ `src=ppc-rel-sha1:…`) |
| `MatchingFor(…)` **incl. GZLE01** | `DECOMPILED` | `BYTE-TRUE` |
| `MatchingFor(…)` **excl. GZLE01** | `DECOMPILED` | **`EQUIVALENT` — PROVEN-class** (see box) |
| `Equivalent` | `DECOMPILED` | `EQUIVALENT` |
| `EquivalentFor(…)` incl. GZLE01 | `DECOMPILED` | `EQUIVALENT` |
| `EquivalentFor(…)` excl. GZLE01 | `DECOMPILED` | `EQUIVALENT` — PROVEN-class |
| `NonMatching`, bodies present | `NONMATCHING` | `DIVERGENT` |
| `NonMatching`, bodies empty (`/* Nonmatching */`) | `STUB` | `UNKNOWN` |
| `DEBUG_ONLY` | `DECOMPILED` | `UNKNOWN` (no retail REL to hash) |

> 🔑 **TWO STRENGTHS OF `EQUIVALENT`, AND THE ROW MUST SAY WHICH**
> (History/Bridge, 2026-08-17 — correcting this table, which first mapped
> `MatchingFor(…)`-excluding-ours to `DIVERGENT` and thereby **threw away
> the strongest evidence available**):
> - **PROVEN-class.** Upstream is `MatchingFor(…)` / `EquivalentFor(…)` on
>   ANY version. **The IDENTICAL SOURCE compiles byte-identical on another
>   build — equivalence is DEMONSTRATED, not argued.** Only the target
>   version's allocator differs. `d_a_msw` = `MatchingFor("D44J01")`.
> - **ASSERTED-class.** Upstream is `NonMatching` on EVERY version, and
>   our claim rests on our own reasoning (liveness, functional reading).
>   `d_a_lod_bg` = NonMatching everywhere; the claim rested on a
>   dead-register argument the reviewer explicitly declined to launder.
>
> **Same axis value, two very different evidentiary strengths — so the ROW
> records upstream's status VERBATIM beside our assertion, and the two
> file separately so the decomp can confirm or deny each on its own
> terms.** Prefer PROVEN-class targets when triaging: their equivalence
> is already banked.
>
> 🛑 **AND PROVEN-CLASS CAN BE A PERMANENT CEILING — CHECK BEFORE
> SPENDING A PASS.** If the only version a TU matches is one we do not
> ship (ours is **GZLE01**), that TU **CAN NEVER GO BYTE-TRUE FOR US**.
> `daMsw_Execute` matches D44J01 only: **`EQUIVALENT` is not a waypoint
> there, it is the TERMINAL state, and stop-work is correct and
> permanent.** Record terminal-vs-waypoint on the row; a lane that
> re-attempts a terminal TU is burning passes on arithmetic.

**Read the instrument, not the raw string:** `decomp_status.py` already
performs this collapse for GZLE01 — including **compound expressions**
(`EquivalentFor(…) or MatchingFor(…)`, resolved as best-for-our-version;
the old regex silently VANISHED such rows, a №31-C failure that cost
NPC_PEOPLE and 19 DZR names). It reports **`NONMATCHING(version)`** as a
distinct value meaning *matched upstream, but not for our disc* — finer
than the row's `DIVERGENT` and worth reading when triaging: a
`NONMATCHING(version)` TU is often a small version-delta away, not a
decode away.

> ⚠ **THE TRANSITION TRAP (History/Bridge, 2026-08-17): THE TWO AXES MUST
> ADVANCE TOGETHER.** Reaching `Equivalent` and setting
> `byte-true: EQUIVALENT` while `portable:` still reads `NONMATCHING`
> trips the laundering rule — a refusal at the exact moment the lane
> finally achieved something. **Move both: `portable: NONMATCHING →
> DECOMPILED` AND `byte-true: DIVERGENT → EQUIVALENT` in the same edit.**
> The refusal message now spells out both readings and the remedy
> (`row_store.py:157`); the gate is unweakened (selftest 10/10, both
> laundering cases still refused).

> ⚠ **AND THE VOCABULARY SPLIT THAT COST A BATCH (History, 2026-08-17):**
> `NonMatching` as a **configure.py STATUS** means *compiled output does
> not byte-match* — the code can be complete and correct. `/* Nonmatching */`
> as a **BODY MARKER** means *declared, never decoded* — there is no code
> at all. A TU can be status-NonMatching with real bodies (`d_a_bridge`,
> 1/18 empty) or with none (`d_a_npc_ko1`, 134/139 empty). **The status
> alone cannot tell you; count the body markers.**

## 3. Responsibilities

1. **Own the drafts.** `docs/state/ww-staging/decode-drafts/` (655
   functions, 23,730 lines, 98 rodata tables). Every draft carries a name
   — this lane's — from the day it lands.
2. **Drive the MATCH metric — not the marker metric.** ⚠ **RE-POINTED
   2026-08-17 (outside review): the inference burn-down was aimed at the
   OPTIONAL axis.** Upstream's own guideline is that naming members is
   optional for a PR, and *"placeholder names are preferable to coming up
   with names that are incorrect if you aren't sure"* — their convention
   for unknowns is literally `field_0x290`. **Names do not affect
   codegen: you can burn every marker and be no closer to a match.**
   - **PRIMARY METRIC: functions at 100% per TU**, read from **objdiff**.
   - **SECONDARY: TUs flipped `NonMatching` → `Matching`** in
     `configure.py` (the flip is literally one word).
   - **Report BOTH headline numbers from the start**, as upstream projects
     do: *% decompiled* and *% fully linked* — a TU can have every
     function at 100% and still fail to link on weak-symbol ordering (tp
     reports 100% / 87.13%). And **progress is SIZE-WEIGHTED, not
     function-counted** — per-function counting inflates via two-line
     getters.
   - **The naming work is REHOMED, not discarded.** Upstream tp keeps
     `docs/re_notes.md`: a committed, greppable dump of recovered facts
     (class → byte size, member → type/offset, argument and temp names).
     **That is the correct home for the 1,224 markers** — reviewable and
     reusable instead of trapped in drafts, and a recognized upstream
     practice. Naming demotes to a documentation nicety on the schedule,
     never the burn-down.
3. **Act on `decomp_watch` triggers.** Baseline pinned at 1,099 TU
   entries (`tracker/_decomp_baseline.json`). Run `decomp_watch check`
   after any donor sync; every flip is a draft CONFIRMED or REPLACED —
   never quietly kept. The addendum's replace-half is this lane's duty.
4. **Label without exception.** Every draft-derived row:
   `portable: NONMATCHING` + `byte-true: DIVERGENT` (until an (A) hash
   says otherwise). Three gates refuse better — filing (`row_store`),
   build (Integrator), derivation (the byte-true feeder's cap) — and this
   lane never argues with them.
5. **Prove (A) when claiming it.** One REL build + SHA1 compare per
   claim; the gate carries a control (one perturbed byte must break the
   hash). No hash, no BYTE-TRUE — grade (B) and say so.
6. **Upstream what qualifies.** An (A)-clean TU is prepared for
   zeldaret/tww per their contribution norms; the retirement ledger
   records the exit (class: drain-by-upstreaming) with the hash and the
   upstream reference.
7. **Honor the chain-of-custody covenant** (`rel_decomp.py`'s governing
   rule): every draft traces to disc bytes through named tool steps;
   acceptance is the oracle stack, not resemblance.

## 3b. The operating loop (user-ordered, 2026-08-17)

1. **The 30-second heartbeat.** The lane's monitor is coupled with an
   explicit timer waking it every **30 seconds**. If the user has not
   responded/prompted in the interval, the lane **continues decoding** —
   same effort, or relocated. Two mechanisms, deliberately distinct: the
   TIMER drives work continuation; the exit-on-event WATCHER delivers
   inbound interrupts immediately (a user message never waits for a pass
   boundary). The pulse file carries `interval: 30` so Foundry's registry
   can tell alive-and-working from dead — with a 30s loop, quiet is never
   ambiguous for more than a minute.
2. **Barrier → flag → relocate, never wait.** A barrier needing user
   feedback is **flagged to FOUNDRY** as a CALLS row (what was tried,
   what input is needed, where the half-done work sits), and the lane
   relocates to another undecompiled system/NPC/texture/target in the
   same pass. **Park-with-a-row, never park silently** — a parked item
   without a row is the unowned-trigger failure this lane was chartered
   against. Foundry keeps the blocker ledger and routes answers back.
3. **The 20-batch review cadence.** After every **20 items decoded,
   typed, and named**, the lane **pings HISTORY** with a batch manifest —
   which 20, where the drafts live, the grading axis claimed (B today; A
   once the REL gate runs), and the inference count before/after (the
   burn-down, per batch) — then continues on the next 20 without waiting.
   **Rejected items re-enter the Decoder's queue AHEAD of new work**
   (rework-first), so corrections never pile behind fresh output.
   **THE UNIT, USER-RULED 2026-08-17: one item = one TU**, fully decoded,
   typed, and named. **Priority order: TUs first — function-level work
   (items needing only a function) follows after every TU-needing item;
   assets follow functions.** TUs > functions > assets, strictly.
4. **Outset first, then outward.** The initial scope is **everything
   missing on Outset Island** — decompile, type, and name it all — with a
   measurable DONE test: the Outset placement census's demand set × zero
   inference markers in any Outset-touched field. Then the sights expand
   (Windfall and the story order are the natural next ring). Honest
   scope note: "texture/otherwise" targets ride different pipelines than
   `rel_decomp` (BTI/asset formats — extraction verified by format, not
   by m2c); the loop and cadence apply unchanged, the toolchain differs.
5. **Board discipline at 30s cadence:** file on EVENTS (blocker, batch
   ping, scope completion) — never per-pass. A lane that files every 30
   seconds buries the board it reports to.

## 3c. THE PERSISTENCE ORDER (user, 2026-08-17) — ACCESS DENIAL IS NOT A BLOCKER

**If the lane is refused access to Ghidra, the shared Ghidra server, the
ZeldaRET Discord, decomp.me, or ANY other community tooling — IT DOES NOT
STOP.** Access is an accelerant, never a dependency. The lane files the
refusal as a barrier row to Foundry (per §3b.2), **relocates in the same
pass**, and continues on the routes it already owns: the debug maps, the
retail `.s`, assert strings, mangled names, matched sibling TUs, objdiff
against the local build. **Persistent and continuous is the standing
posture.** A refused request is a routing event, not a work stoppage —
and any pass that ends in idling because a door was shut is a defect in
this lane, not in the door.

## 3d. THE UPSTREAM WORKFLOW — adopt what already exists (outside review, 2026-08-17)

> **The finding that reframes the lane: we have been reconstructing from
> m2c drafts in OUR repo what upstream hands us as a structured scaffold
> with a build, a diff tool, and a PR path attached.** `d_a_npc_ko1.cpp`
> already exists upstream — 601 lines, 139 function definitions each
> carrying its mangled name and address range, the class hierarchy, the
> identity comment *"NPC - Joel & Zill (Outset Island)"*, and 134 bodies
> marked `/* Nonmatching */` and empty. The header exists too, with the
> member block as a placeholder (`/* 0x6C4 */ u8 m6C4[0x8AC - 0x6C4];`).
> **The lane is not duplicating — ko1 is genuinely NonMatching and the
> member work is real — but the WORK PRODUCT belongs in that scaffold.**

### Phase 0 — Adopt what is already in the tree (days, no new tooling)

- **`objdiff.json` IS ALREADY GENERATED** — 1.73 MB, **1,689 units**,
  `custom_make: ninja`, live watch patterns. `configure.py` writes it, and
  **the lane already ran configure.py for the (A) proof.** Install the
  objdiff GUI, point it at `D:/XXXXXXX/WW DP`, and it reads that config
  automatically. It diffs the compiled `.o` against the dtk-split target
  `.o` **per symbol, rebuilding on every source save.** That is the
  per-function match gradient the lane lacked — a GUI install, not a
  project.
- **THE BUILD ITSELF IS THE (A) GATE** — it runs `dtk shasum` against
  `config/GZLE01/build.sha1`: exactly what the lane hand-built as a
  bespoke SHA1 compare. **Switch the gate to the build; the bespoke
  script survives only as Foundry's control fixture.**
  > ⚠ **CORRECTED 2026-08-17, SUPERSEDED WITH RECEIPT (Decoder verified
  > at source; the error was mine).** The review named this target
  > **`ninja ok`** — **that target name is `sms`'s and DOES NOT EXIST IN
  > tww.** tww's equivalent is the **DEFAULT ninja target** (build +
  > `dtk shasum` verify), which the Decoder ran clean end-to-end (exit 0,
  > hash steps included). tww also ships `progress` / `baseline` /
  > `changes` / `diff` / `apply`. **Cite the default target, never
  > `ninja ok`.**
- **`ninja baseline` → edit → `ninja changes`** is the standing regression
  guard (did this edit un-match a previously-matching function).
  **Mandatory before touching any shared header** — editing an inline
  silently un-matches functions project-wide. Also present and unused:
  `ninja progress`, `ninja apply` (folds matched symbols into
  `symbols.txt`).
- **REQUIRED READING, in order:** `doldecomp/sms/docs/AGENT_MATCHING_TIPS.md`
  (37 KB, 36 numbered sections, MWCC-specific — **written for exactly this
  situation, an agent doing matching work; read it in full before touching
  another function**), then tww's `docs/decompiling.md`,
  `docs/regalloc.md`, `docs/coding_guidelines.md`.

### Phase 1 — Triage order, and where the work product lives

- **TRIAGE RULE (sms): resolve wrong instructions and instruction ORDERING
  first; address STACK FRAME SIZE LAST.** Frame size is downstream of
  everything else and chasing it early burns days.
- **Work the upstream scaffold in `WW DP`, not parallel drafts in our
  repo.** The m2c drafts become an INPUT to that work, not the artifact.
- **Adopt `field_0x290` for unknowns** rather than inferring a name.
- **Use m2c TARGETED:** upstream's use is for **switch statements**
  specifically (`-t ppc-mwcc-c++`), where Ghidra misreads comparison trees
  as if/else chains.

### Phase 2 — The debug maps' REAL job: inline discovery

> 🔄 **FIRST, THE ROUTE ORDER — AMENDED 2026-08-17 ON MEASURED EVIDENCE:
> A BYTE-MATCHED SIBLING BEATS A DEBUG MAP WHEN BOTH EXIST.** A map gives
> you NAMES; a byte-matched sibling gives you names AND class shape AND
> the idiom set AND a compiling reference. That is what turned
> `d_a_obj_msdan_sub2` into **18/18 functions, 15 of them at 100% on the
> FIRST compile** (History/Bridge supplied the lead, Decoder executed).
> **And the supply is large: 28 of 58 NPC actor TUs are byte-true for
> GZLE01, including 13 of the 14 named template siblings** — `km1 bm1 hi1
> jb1 ba1 ls1 p1 ah kamome zl1 mk tc tt`. **Only `ji1` (Orca) is
> NonMatching, making it a PEER of the Outset queue rather than a source
> for it.**
>
> **ORDER: (1) byte-matched sibling · (2) debug map · (3) headers.**
> Consult `configure.py` for a matched family member BEFORE decoding
> anything — it costs one grep. **Caveat: byte-matched ≠ class-cognate.
> The SUPPLY is measured; the FIT is a per-target read.**

The lane has been using the 372 maps for member naming. **Their
high-value use is finding inlines** — the single hardest part of mwcc
matching, because using the wrong inline causes regswaps in the caller.

- Debug builds compile with `-pragma dont_inline`, so **functions inlined
  away in retail exist as real symbols in the debug map.**
- In a per-actor map the **indented linker tree** gives call hierarchy;
  entries marked `(func,weak)` are inlines.
- **ORDERING RULE: process in linker-tree order starting from
  `g_profile_`, NEVER source order** — each inline appears only once per
  map, so source-order work misattributes them.
- In flat maps (`frameworkD.map`), multiple `.text` sections mean the
  first holds non-weak functions and the rest hold inlines, **listed in
  reverse**.
- **UNUSED map functions must STILL be reconstructed** — their inlined
  bodies determine caller codegen even though they never exist standalone.
  Treat the map's signature and size as hard constraints, recover the body
  from call-site patterns, validate by diffing the callers. **A too-large
  stack frame is the canonical symptom of an unreconstructed inline.**
- `dtk` has a first-class `map` subcommand and `configure.py` takes
  `--map` — this need not stay manual.
- ⚠ **DISCOVERED LIMIT (Decoder, 2026-08-17, first objdiff exercise): the
  debug build rides DIFFERENT VERSION GUARDS than retail**, so an inline
  check against a debug map can come back **inconclusive** for
  version-guarded code — the debug body may not be the retail body. **An
  inconclusive inline check is UNKNOWN, not "no inline"** (№31-C): record
  it as unresolved rather than concluding the function has none, and fall
  back to call-site reconstruction (frame-size symptom, caller diffs).

### Phase 3 — The struct workflow (currently being done the hard way)

1. **Instance size** is at `g_profile` in the generated `.s` (fifth line).
2. Build the struct in **Ghidra** with field 0 as parent, typed to the
   right base — `fopNpc_npc_c` for NPCs. Retype fields as you learn them.
3. Run tww's shipped **`docs/tww_class_to_cpp.py`** in Ghidra's Script
   Manager: it exports the whole struct to clipboard. Paste it into the
   header **over the placeholder** (for ko1: `/* 0x6C4 */ u8 m6C4[...]`).
4. **CTORS: decode the raw `.s` directly.** Walk the prologue and map each
   `stb/sth/stw/stfs` to one source assignment using the offset comments.
   If the asm writes `0xFC, 0x100, 0x104` and your source only covers
   `0x108..0x110`, you have **missing initialization** — one-for-one, no
   guessing.
5. **TWO FREE ORACLES, mine them harder:** assert strings bake in the line
   number AND the stringified condition, so **original variable names and
   exact source lines are recoverable** (the lane already hit this with
   `m_hed_tex_pttrn` — there is far more of it); and **mwcc mangled names
   encode parameter types** (`fopAcM_seStart__FP10fopAc_ac_cUlUl` decodes
   for free).
5a. **RETURN TYPES ARE INVISIBLE IN MANGLING AND VISIBLE IN THE CALLER'S
   CODEGEN — the third free oracle (Decoder, 2026-08-17, `d_a_obj_msdan`).**
   mwcc's scheme encodes **parameter** types only
   (`fopAcM_seStart__FP10fopAc_ac_cUlUl`) — **the return type is not in the
   name**, so for a wrapper you must otherwise guess. **The call site tells
   you:** a **signed compare (`cmpwi`)** on the result ⇒ the function
   returns `int`/signed · a **`clrlwi` parked STRAIGHT into the argument
   register** ⇒ it returns a narrow unsigned (`u8`) — because **an `int`
   return would insert an `mr` temp** at that park. Measured live:
   `prm_get_size` must return `int`, `prm_get_evId` must return `u8`,
   **and each wrapper's type is recoverable from its compare/park shape**
   even though the `daObj` template returns `int` natively. **Return-type
   choice is CODEGEN-VISIBLE: it is a matching lever, not a style
   preference.**
5b. **THE STATIC-LOCAL COUNTER DELTA — an INCLUDE-COMPLETENESS METRIC,
   readable from mangling alone (Decoder's find, 2026-08-17, `d_a_msw`).**
   mwcc numbers compiler-generated static-local guards (`xd$N` / `zd$N`)
   **monotonically per TU**, so the counter on a given function measures
   **how many compiler entities the TU declared BEFORE it**. Compare
   target vs ours: `d_a_msw` target `xd$4203/zd$4204` against our
   `xd$1763/zd$1764` means **the original TU numbered ~2,440 MORE
   entities before that function — a materially heavier include/inline
   set, quantified without building anything.**
   - **Why this matters: it is the FIRST quantitative signal for include
     archaeology**, which is otherwise guesswork. The frame-size symptom
     tells you *an* inline is missing; the counter delta tells you
     *roughly how much* is missing, and **converges as you add the right
     includes** — a feedback signal you can steer by.
   - **Caveat (position-dependent):** the counter only counts entities
     numbered BEFORE that function, so a LATE function in the TU gives a
     fuller reading than an early one. Read several across the TU.
   - **📏 THE SCALE — calibrated by History/Bridge, 2026-08-17, and this
     is what makes it an INSTRUMENT rather than an observation.** Two
     checks the discovery pass had not run: **(1) the counters are
     IDENTICAL across GZLE01 / GZLJ01 / GZLP01** — version-invariant
     across retail, so the signal is not a version artifact; **(2) the
     DEMO build differs from retail by FIVE** (`xd$4198` vs `xd$4203`).
     **So ~5 is what a genuinely different BUILD OF THE SAME GAME looks
     like. A delta of 2,440 is therefore not "a heavier include set" — it
     is a CATEGORICALLY DIFFERENT one.** Read deltas against that ruler:
     single digits = build variance, hundreds-to-thousands = the
     reconstruction is missing whole include trees.
   - **Sanity check before trusting a reading:** confirm the target's
     counters exist in the donor symbol tables and yours do not — that
     rules out reading your own artifact back as the target's.
6. **SEARCH MATCHED CODE FIRST.** Ctrl+Shift+F a similar decompiled actor
   beats reasoning from scratch — matched code encodes the project's
   discovered idioms. **For ko1 that is `d_a_npc_km1.cpp`** — 741 lines,
   `MatchingFor(GZLJ01, GZLE01, GZLP01)`, i.e. byte-verified. The lane
   already used km1 for stride checks; **it should be the primary idiom
   source.**

### Phase 4 — Flags: mostly not the lane's problem, and the one diagnostic

Flags are **per-library**, and actor RELs inherit wholesale —
`ActorRel(NonMatching, "d_a_foo")` picks up `cflags_rel` (framework flags
plus `-sdata 0 -sdata2 0`). **So the lane mostly does not hunt flags.**
The diagnostic to internalize: **pervasive noise across a whole library
means the FLAG is wrong; localized noise in one function means YOUR CODE
is wrong.** `-schedule off` mismatched produces "right instructions,
wrong order"; `-fp_contract` decides whether `a*b+c` becomes `fmadds`;
`-use_lmw_stmw` reshapes every prologue. A genuinely different TU scopes
via `extra_cflags`.

### Phase 5 — Community assets, then the PR loop on something cheap

**(Subject to §3c: refusal routes, it does not stop the lane.)**

- Request read access to the **shared Ghidra server** (`ghidra.decomp.dev`,
  port **13100**) — it hosts **`shield_chn_debug`, the TP debug build with
  inlines NOT inlined**, which is an asset for our **TP-side** work
  independent of WW.
- Join **ZeldaRET Discord**: `#tww-decomp` (kiosk-demo debug maps are
  pinned there) and `#tww-decomp-help`.
- Use objdiff's built-in **decomp.me** button for stuck functions —
  GC/mwcc is supported and tww ships `scratch_preset_id=73`.
- **Learn the PR loop on a cheap target first:** ~20 open *easy object*
  issues, claimed by commenting. **One TU per PR**, title `d_a_obj_foo OK`.
  **CI builds all four versions** (GZLE01 / GZLP01 / GZLJ01 / D44J01) — a
  PR must break none.

> 🔄 **GATE AMENDED 2026-08-17 — "ONE MERGED PR BEFORE ko1" IS REPLACED BY
> "FOUR-VERSION LOCAL BUILD GREEN BEFORE ko1" (Foundry, on History/Bridge's
> finding; this is the briefing's own text and mine to correct).**
>
> **THE PROBLEM THEY NAMED: `ko1` IS THE CHARTER'S OWN TARGET, so gating it
> on a MERGE put the Outset NPC line — the reason this port exists — behind
> MERGE LATENCY, which is entirely outside this estate's control.**
>
> **THE GATE HAS TWO SEPARABLE HALVES AND ONLY ONE IS SLOW:**
> - **MECHANICAL** — does it build and match on all four versions? Objective,
>   fast, and **fully in our hands: `configure.py --version` accepts
>   D44J01 / GZLJ01 / GZLE01 / GZLP01, so THIS NEEDS NO SUBMISSION AT ALL.**
>   Build all four locally; that IS the CI property, measured directly.
> - **SOCIAL** — do maintainers accept the naming and style (the invented
>   `Prm_e` / `mEventId` / `mState` class)? Real value CI cannot give, and
>   **unbounded in time.**
>
> **WHY PROCEEDING ON THE MECHANICAL HALF IS LOW-RISK, and it is the
> briefing's own established fact: NAMES DO NOT AFFECT CODEGEN.** The social
> feedback is almost entirely about naming and style, so a maintainer
> rejection costs a **rename pass**, not a re-decode. The matched bytes
> survive any naming verdict.
>
> ⚠ **AND THE FOUR-VERSION LOCAL BUILD IS ITSELF BLOCKED — corrected within
> the hour (Decoder's barrier row, verified by Foundry): `orig/GZLJ01`,
> `orig/GZLP01` and `orig/D44J01` contain ONLY `.gitkeep`. WE HAVE ONE DISC
> (GZLE01). No target objects can be split for the other three, so no
> per-version build can run — regardless of `--version` support.**
>
> **THE MEASURABLE PROXY THAT REPLACES IT, and it needs nothing from
> anyone:** upstream's `config/<ver>/build.sha1` **does** list expected
> hashes for all four versions of each TU (verified: all three flipped TUs
> appear in all four manifests), so the TARGETS exist even though the discs
> do not. **The risk a four-version build would catch is
> VERSION-CONDITIONAL CODE — and that is greppable.**
> - **GATE: (1) GZLE01 match verified locally · (2) ZERO `#if VERSION` /
>   `PLATFORM_` guards in the TU.** Measured on all four msdan/hami TUs:
>   **the only `#if` in any of them is the header include guard.** A
>   version-agnostic TU that matches GZLE01 byte-for-byte is *expected* to
>   match the others, because there is no conditional path to diverge on.
> - **IF A TU DOES CARRY VERSION GUARDS** (like `d_a_lod_bg`'s
>   `#if VERSION > VERSION_DEMO`), a plain `Matching` claim covers three
>   versions we cannot test — **narrow it to `MatchingFor("GZLE01")` or
>   flag the multi-version claim as UNVERIFIED. Do not let a one-disc
>   measurement wear a four-disc label.**
> - **RESIDUAL RISK, NAMED: version divergence can still arrive from
>   per-version headers, inlines or sdata layout even with no guards in the
>   TU.** The proxy is strong, not total; upstream's CI remains the only
>   complete answer, and that is a PR-time discovery, not a local one.
>
> **SO: ko1 unblocks on a green GZLE01 build plus a clean version-guard
> scan. The PR remains a LIVE PARALLEL TRACK — when it lands, its review feedback is applied
> RETROACTIVELY to everything decoded since, as a naming/style correction
> sweep.** Nothing waits on merge latency; the social learning still happens
> and still corrects.
>
> **UNCHANGED AND NOT MINE TO MOVE: the PR SUBMISSION still requires the
> USER'S EXPLICIT AUTHORIZATION.** This amendment governs what the LANE
> waits for internally; it does not authorise anything outward-facing.
  > ⚠ **QUEUE TRAP, MEASURED TWICE (Decoder, 2026-08-17): A "ONE FUNCTION
  > FROM FLIPPING" TU IS NOT A CHEAP WIN — IT IS THE RESIDUE UPSTREAM
  > ALREADY GAVE UP ON.** Both near-flip TUs the lane tried
  > (`d_a_lod_bg` 99.94%, `d_a_msw` 97.89%) turned out to be allocator
  > tie-breaks that upstream **already labels `regalloc`** — the last
  > function is last precisely because it is the hard one. **The
  > cheap-flip shortlist mines ARCHAEOLOGY, not decoding.** Cheap targets
  > are the *easy-object issues* (small TUs with real empty bodies) —
  > **select on EMPTY BODIES TO WRITE, not on proximity to 100%.**

### Phase 6 — Then ko1, in the loop

Ghidra struct → `tww_class_to_cpp.py` → **smallest function first** →
write → objdiff auto-rebuilds on save → read the per-symbol diff → apply
the regalloc ladder → 100% → strip `/* Nonmatching */` → repeat → flip to
`Matching` → `ninja ok`.

**THE REGALLOC LADDER, in order:**
1. **Verify functional equivalence first** — most "regswaps" are a swapped
   same-typed variable.
2. **Hoist and permute local declaration order** (highest yield).
3. Extract the base pointer to a local.
4. Try casts — **C-style vs `static_cast` differ in codegen**.
5. Split compound expressions into named temps.
6. **Confirm you are using the exact inlines the map specifies.**
7. Brute-force `const` — it changes both allocation and ordering, and is
   **invisible in mangled names for primitives**.
8. **RETURN TYPE of any wrapper in the expression** — codegen-visible per
   §3d Phase 3 item 5a, and invisible in the mangled name, so it is easy
   to have wrong and never notice.

> ⚠ **A LADDER RUNG THAT DOES NOT EXIST: SOURCE GROUPING OF A SUM.**
> **MWCC CANONICALIZES REASSOCIATION** — measured over NINE receipted
> source shapes (groupings, statement splits, out-of-loop local,
> int/u32 permutations, a cast-node block) that all produced *identical*
> output while the target associated differently
> (`(swSave + size<<16) + i*0x100` vs ours `(swSave + i*0x100) +
> size<<16`). **If an association order differs, PARENTHESES ARE NOT THE
> LEVER — stop permuting them.** Look instead for a **type/width
> difference** in one term, an **intermediate variable** the original
> held, or a **`const`** that changes the fold. Functional equivalence is
> trivial here (associativity of `u32 +`), so **park it §2b rather than
> spend a tenth experiment.**

**MWCC INVARIANTS (guesswork → rules):** it **never reorders non-weak
function definitions** — source order IS binary order · it **preserves
source order of memory operations** — ordering mismatches are cheap fixes ·
it **eliminates redundant reads but NOT writes** — transcribe every store,
since missing stores are the top cause of "frame too small" · it **never
swaps if/else blocks** · **switch cases must be written in binary order,
including the "useless" ones** · a **user-declared destructor, even empty,
makes a class non-trivial and forces stack allocation** — a matching
hazard AND a deliberate technique when the target reloads fields your
version caches.

### The agentic evidence, and the one tool NOT to build around

- **The Mizuchi benchmark** ran 60 functions through a programmatic phase
  (m2c → compile → objdiff → permuter), then an **LLM agent with objdiff
  feedback in the loop: 74% overall match rate, and ~50% of the AI matches
  landed on the FIRST attempt on functions the programmatic tools had
  already failed.** Caveat: measured on GBA/ARM and N64/IDO, **not mwcc**.
  But **the compile→objdiff→revise agentic loop is the demonstrated
  frontier — and this lane IS the agent. It simply is not in the loop
  yet.** Phase 0 puts it there.
- **DO NOT build around `decomp-permuter`.** Confirmed both streams: it
  parses C with `pycparser`, which **does not handle C++**, and GC/Wii
  projects are C++. `PERM_IGNORE` limits mutation to unwrapped islands,
  tww's docs never mention it, and its measured effectiveness was **zero**
  on GBA/ARM.

### What survives contact with upstream, unchanged

**The (A)/(B) axis split is better than anything upstream has — they have
no port-side acceptance concept because they have no port. That is OURS
and it survives.** Same for `decomp_watch`'s confirm-or-replace duty, the
chain-of-custody covenant, and the **junction workaround for the space in
`WW DP`** (it breaks mwcc's unquoted `-o` path).

## 4. The toolchain — every instrument the lane touches

| tool | role for this lane |
|---|---|
| **objdiff (GUI)** | **THE PRIMARY INSTRUMENT** — per-symbol match %, auto-rebuild on save; reads the ALREADY-GENERATED `objdiff.json` (1,689 units) at `D:/XXXXXXX/WW DP`. Carries the decomp.me button |
| **ninja DEFAULT target** | **THE (A) GATE** (build + dtk shasum vs `build.sha1`) — replaces the bespoke SHA1 script, which demotes to a control fixture. **NOT `ninja ok` — that name is sms's and does not exist in tww (Decoder-verified 2026-08-17)** |
| **`objdiff-cli`** (`D:/XXXXXXX/tools/objdiff-cli.exe` v3.8.0) | **the headless stand-in for the GUI — same `objdiff.json`, same per-symbol %.** This lane is an agent: the GUI's rebuild-on-save loop becomes **build + report per edit** |
| **`ninja baseline` / `ninja changes`** | the standing regression guard; **mandatory before editing any shared header** (inline edits un-match project-wide) |
| **`ninja progress` / `ninja apply`** | size-weighted progress; fold matched symbols into `symbols.txt` |
| **`sms/docs/AGENT_MATCHING_TIPS.md`** | **REQUIRED READING, in full** — 36 sections, MWCC-specific, written for agent matching work |
| **tww `docs/`** | `decompiling.md` · `regalloc.md` · `coding_guidelines.md` · **`tww_class_to_cpp.py`** (Ghidra struct → header, run in Script Manager) |
| **Ghidra** (+ `ghidra.decomp.dev:13100` if granted) | struct recovery; the server hosts `shield_chn_debug` (TP debug, inlines not inlined) — **useful to the TP side independently.** Refusal routes per §3c, never stops the lane |
| `tools/foundry/rel_decomp.py` | the draft pipeline (dtk split asm → m2c → fopAc offset receipts); its covenant governs. **m2c is now TARGETED — switch statements (`-t ppc-mwcc-c++`)**; drafts are INPUT, not the artifact |
| `D:/XXXXXXX/ppc-compilers-20251118/` | the (A) toolchain — MWCC set, tag-labeled, user-authorized |
| `D:/XXXXXXX/WW DP/configure.py --compilers <above>` + ninja | the (A) build; SHA1 verify against `config/GZLE01/build.sha1` |
| `tools/foundry/decomp_status.py` | the official ledger, five-class, version-scoped (GZLE01) |
| `tools/foundry/decomp_watch.py` | the time join — pinned; run after every donor sync; triggers are this lane's work items |
| `tools/foundry/coverage_join.py` | is the bottleneck us or them |
| `tools/foundry/port_preflight.py` | the card every queue row attaches (five-class aware; prints the required stamps) |
| `tools/foundry/row_store.py` | filing gate — refuses laundered claims, one TU per row |
| `tools/foundry/compile_gate.py` | receiver-side shape check (TIER2 = shape, NEVER cited as compile — DN-12) |
| `queue_verdict_lint.py` | no syntax pass dressed as a verdict on queue rows |
| debug maps (372 files) | **INLINE DISCOVERY FIRST** (linker-tree order from `g_profile_`; `(func,weak)` = inline; UNUSED entries still get reconstructed), naming second |
| `d_a_npc_km1.cpp` and other MATCHED TUs | **the primary idiom source** — search matched code before reasoning from scratch |
| oracle stack (probe differ / state taps / golden traces) | (B) acceptance |
| `file_row.py` | ALL board writes; one row per call; answer-notes don't wake lanes — results others wait on get their own addressed row |
| `lane_watch.py --exit-on-event` + `monitor_pulse.py` | the lane's monitor; **liveness, coverage, and DELIVERY are three properties — arm the exit-on-event form, verify the pulse, re-arm after every delivery** |

## 5. Standing laws the lane inherits (each with its receipt on the board)

- **DN-10** order of resort; the donor's own system first — always.
- **§801**: NonMatching-but-complete is admissible as a behavioural spec
  under labelling; the completeness test (dtk ranges vs empty bodies) has
  a control (trips 2/869).
- **Drafts are DRAFTS**: never MATCH, never BYTE-TRUE without an (A) hash.
- **A mechanism is standing when its OUTPUT exists, not when its code
  does** — ask for the artifact (the pin, the log, the hash), not the file.
- **Absence in the place you looked is not absence everywhere**; verify at
  the property, not the table (the inline-exports lesson).
- **A measured fact plus a plausible reading is not a finding** — the
  discriminating second measurement is usually already in your own output.
- 🔴 **SEMANTICS FROM CONSUMERS, LAYOUT FROM BYTES — A DECOMP HEADER IS A
  RECONSTRUCTION, NOT THE DISC** (Foundry's own error, 2026-08-17, caught
  by History/Bridge). A donor header is the same class of artifact as our
  drafts: written by someone reading bytes, and wrong in the ways drafts
  are wrong. **`d_stage.h` declares the EVNT name at `0x04`; 57 real
  records off `sea/Stage.arc` put it at `0x01`** — implementing the header
  chops three characters off every name and **silently matches nothing**.
  The same row reasoned an "overlap" hazard that does not exist, purely
  from the bad offset. **For OFFSET / SIZE / ARITY / CAPACITY questions
  the bytes are the authority and the header is a hypothesis; read the
  consumers for what a field MEANS, never for where it LIVES.**
- **4-byte `void` bodies are empty in retail** — decode by size+signature
  with certainty; 17 known (incl. `chngAnmTag` in Ym1/Yw1/Aj1 — the anime
  channel is donor-inert for those NPCs).
- **One-bat rule, c0 hygiene, CRLF for .bat, Write-tool for content** —
  the byte-class laws bind here as everywhere.

## 6. First work items, in order

1. **THE LANE'S FIRST ACT (user-ruled): fetch the remaining build tools**
   — `download_tool.py` for dtk / ninja / sjiswrap (small, GitHub
   releases) — then configure with the compilers
   (`D:/XXXXXXX/ppc-compilers-20251118/`, verified present, 165 MB,
   `mwcceppc.exe` smoke-tested EXECUTING natively on this machine) and
   run ONE (A) proof: build the smallest REL (`d_a_alldie.rel`) and
   SHA1-compare against `build.sha1`. That run turns the (A) gate from
   specced to real; Foundry registers its control the same day.
2. Stand the lane's monitor up (exit-on-event form) and file the arming
   row.

**SUPERSEDED 2026-08-17 by the outside review — items 2-3 of the original
list (name ko1 from its map; propagate template names) were aimed at the
optional axis. The replacement order is §3d's phases, and the first four
moves are:**

3. **Install objdiff and point it at `D:/XXXXXXX/WW DP`** — it reads the
   `objdiff.json` your own configure.py run already generated. **Gate:
   a per-function match % visible for ko1.** This is the single highest
   -value action available to the lane and it is an install, not a build.
4. **Switch the (A) gate to `ninja ok`**; adopt `ninja baseline` →
   edit → `ninja changes` as the standing guard.
5. **Read `AGENT_MATCHING_TIPS.md` in full**, then tww's
   `decompiling.md` / `regalloc.md` / `coding_guidelines.md`.
6. **Re-anchor the work product** into the upstream scaffold
   (`d_a_npc_ko1.cpp` exists there, 601 lines / 139 declarations); rehome
   recovered names to a `re_notes.md`-style committed dump; report match %
   and link % as the metrics.
7. **Then the cheap PR loop** (one *easy object* issue, one TU, four-version
   CI green) **before ko1 is attempted** — learn the loop where failure is
   cheap.
