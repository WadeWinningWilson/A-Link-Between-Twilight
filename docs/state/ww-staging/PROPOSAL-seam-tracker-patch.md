# PROPOSAL — the seam tracker, and the patch it compiles

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
>
> ## ⏭️ THE EVENT THIS STAMP ANTICIPATED HAS NOW HAPPENED — amended 2026-08-17
> **THE MERGE LANDED.** `dusklight-main` fast-forwarded **39 commits to
> `c880d46fb5`** (2026-08-15), **ABI epoch 2**, and the services listed above
> are **ON DISK with real implementations** — 18 files in `src/dusk/mods/svc/`,
> verified at source, not relayed.
>
> **SO THE WORDS ABOVE HAVE CHANGED FORCE WITHOUT CHANGING MEANING.**
> *"Predates the services"* no longer reads as *"those services are
> hypothetical"*; it now reads as **"measured against a host that no longer
> exists."** Same sentence, opposite practical weight — a reader before the
> merge would treat these numbers as current-minus-a-roadmap, and a reader
> after it must treat them as historical.
>
> **A supersession stamp is itself dated evidence.** It was correct when
> written this afternoon and needed this amendment within hours, which is the
> argument for putting the COMMIT and the DATE in a stamp rather than only the
> list of what it predates: a list ages silently, a commit hash does not.
> (Integrator caught it; Librarian amended.)

>
> **AND ONE THING THIS DOCUMENT CARRIES THAT THE OTHER TWO DO NOT:** its
> premise is *the patch it compiles*. The user's 2026-08-17 delivery ruling
> (plugin + patcher, recorded in
> [SPEC-patcher-requirements.md](SPEC-patcher-requirements.md)) and the P1
> briefing's patch-free target both move the ground under that premise.
> **The seam MEASUREMENTS remain the record; the PATCH RATIONALE is superseded
> by a live ruling, not merely aged.** Stamped, not deleted — a proposal whose
> premise moved is still the evidence of why the premise was reasonable.

> **STATUS: PROPOSAL ONLY — DO NOT ACT.** User-ordered 2026-08-16, written for
> review by an outside source. No lane builds anything named here until the
> review lands and the user says go. Foundry authored; every number carries
> its command or its § receipt.

---

## 0. What this is

Two things, in dependency order:

1. **Close the blind spots** so "what makes Outset run on the fork" is a
   *measured* set, not a bounded estimate.
2. **A seam tracker** — the registry of every receiver-side change the plugin
   needs — from which a **minimal patch to the user's clean dusklight** is
   compiled and shipped alongside the plugin.

The second is worthless without the first: a patch compiled from an
incomplete set boots a black screen, and we already know exactly what that
looks like (the Outset warp: scene torn down, stage never registered, zero
errors).

---

## 1. Where the enumeration actually stands (honest state)

**What is measured:**

| quantity | value | command / receipt |
|---|---|---|
| Ratchet baseline (Phase 0) | TU 102 / sites 386 | `python tools/foundry/ww_ratchet.py` |
| tier2 split | A 72 · B 299 (140 must-hook across 73 genuine hosts) · C 15 | `tier2_census.py --all-hosts` |
| Migration candidate files | ~183 (not 1,088) | commit-history attribution, CALLS §"diffing steps" rows |
| Heuristic error, both ends | 2% false-neg · ~4% true false-pos | content sweeps, all-83 + sampled-977 |
| Confirmed both-directions worklist | 34 files, ordered | reconciliation + the 2 tier2 misses |
| Stage-hosting symbols ABSENT on vanilla | 6 | `upstream_conformance --symbol`, re-derived 2× by 2 lanes |

**What is NOT closed — the four blind spots this plan exists to kill:**

- **BS-1, the `\bww_` word boundary.** `RE_WW` cannot match names where `ww`
  follows an underscore (`d_ww_itemmdl_pc.h`). 138 known-invisible sites; the
  count of *unknown* invisible sites is itself unknown. Three instances of
  this one root shape in one session, two lanes.
- **BS-2, the `dExt*` call shape.** WW reaching native TP files through
  helper APIs (`d_cut_actor_spawn.cpp`, `d_demo.cpp`) — a *category* tier2
  does not model, found only because two attribution directions disagreed.
- **BS-3, the ~20 commit-heuristic misses** implied by the 2% rate,
  unenumerated.
- **BS-4, the SKINS/PORT conflation.** The skins track reads TP's *own*
  byte-identical WW payload (`TPHistory.md` §2.1) and needs no donor disc —
  it is arguably not migration debt at all, yet it is inside the 386.

Consequence, already printed by the ratchet itself: **the 386 rider is an
upper bound and an undercount simultaneously.** No patch gets compiled from a
number in that condition.

---

## 2. The distinction the whole plan turns on

**"Everything that changed" ≠ "everything that makes Outset run."**

Every measurement above is *static* — it enumerates the fork-vs-vanilla
delta. But the delta includes scaffolding: probes, half-ports, retired
approaches, WW_PROBE instrumentation awaiting strip. A patch that ships the
delta ships our debris; a patch that ships our *guess* at the load-bearing
subset is unverified in exactly the way the Outset warp was — everything
green until the one unregistered thing is needed.

The only closure that can *fail* (and therefore the only one that counts) is
**runtime evidence**: run Outset on the fork, record which WW seams fire, and
reconcile:

- **fired ∧ enumerated** → confirmed load-bearing. Patch/plugin candidate.
- **fired ∧ NOT enumerated** → *a blind spot found by the control.* This is
  the row that proves the method works.
- **enumerated ∧ never fired** → deferral candidate — possibly dead, possibly
  non-Outset content. NOT deleted, *classified*.

This is the estate's own doctrine pointed at the completeness claim itself: a
static "we found everything" is vacuous until a live run can contradict it.

---

## 3. The plan, in phases

### Phase A — close the static blind spots *(instrument work, gated)*

1. Fix `RE_WW` (BS-1) and add `dExt*`-call detection (BS-2) to
   `tier2_census`. **Not Foundry unilaterally** — tier2 is a gate and the
   defects were found by the lane proposing the fix; the change lands with a
   negative control per direction (a fixture tier2 must FIND and one it must
   IGNORE) and a second lane's eyes, per `control.py` doctrine.
2. Enumerate BS-3: content sweep of the full 977 with a *code-only*
   fingerprint (comments excluded — the `d_msg_object.cpp` lesson, where the
   match was a sentence *denying* donor content).
3. Split SKINS/PORT (BS-4): tag the 138 skins sites; teach `ww_ratchet.py` to
   report the two tracks separately.
4. Re-run everything. Publish **v2 scope numbers** alongside — never over —
   the pinned Phase-0 baseline. The v1 `386` stays stamped; a baseline edited
   to match a later measurement destroys the only evidence the number moved.

### Phase B — the runtime control *(the phase that makes A honest)*

1. Instrument the fork's WW seams with one-shot fire receipts — many already
   log; the gap is a uniform, greppable receipt format and coverage of the
   silent seams (registration tables consulted, not just functions entered).
2. One user playthrough of the Outset loop on the fork (boot → warp → walk →
   interiors → back). Log is the artifact.
3. Reconcile fired-set vs enumerated-set. Every `fired ∧ not-enumerated` row
   is a Phase-A defect; iterate A until a full run produces zero of them.
   **Exit gate: two consecutive runs, zero unenumerated fires.**

### Phase C — the seam tracker *(the deliverable)*

One row per seam. **Intake discipline — a row earns its place only by
doorway class, proven by command on both images:**

| doorway | meaning | disposition |
|---|---|---|
| **EXISTS** | symbol reachable on vanilla (manifest, incl. anon-namespace) | **plugin binds it — NOT a patch entry** |
| **GATED** | present but compiled out (`'BMDL'` `#if DEBUG`) | patch candidate **and** origin proposal, built as though the proposal never lands |
| **ABSENT** | not in the vanilla binary | **patch entry** |

Seed rows: the 6 ABSENT stage-hosting symbols (`dExtWwSave_registerWwStage`,
`dBootStage_add`, `dExtWwSave_isWwHostStage`, `dExtNpcDoors_armArrivalGuard`,
`markDebugWarpStorySuppress`, `setRoomLayerOverride`) + the BMDL gate. Note
the 15 `l_objectName` rows are the worked example of intake *rejecting* an
entry: doorway EXISTS via `dStage_searchName` — my own withdrawn claim, kept
in the doc as the cautionary row.

Every row carries:
- **the doorway verdict + the command that re-derives it** (derived, never
  hand-maintained — the gate-count lesson: 7→9→13 in one evening, every
  citation accurate at the keystroke and all wrong by midnight);
- **a runtime receipt** from Phase B (this seam FIRED on run N);
- **a negative control**: *"remove this from the patch — what breaks, and how
  would we notice?"* An entry that cannot answer is assumed-necessary, not
  proven-necessary, and does not ship;
- **a read timestamp** on every source citation (the `WwPilot 0→2` lesson:
  on a live tree, "I read X" and "it is Y" are both true).

The tracker is the **complement of the plugin**: exactly the doorways the
plugin provably cannot open. Every migration or accepted origin proposal
deletes a row. **Target state: zero rows** — the roadmap's "patcher possibly
never," held as an invariant rather than a hope.

### Phase D — the patch *(compiled from C, never from a build diff)*

1. **Source of truth is the tracker.** Patch churn is bounded by tracker
   churn — this is what evades the roadmap's moving-target objection, and why
   this proposal does not reverse that ruling but formalizes its escalation
   clause.
2. **Apply-time resolution via the embedded symbol manifest**, not raw
   offsets. The manifest ships in every dusklight image and carries
   anonymous-namespace symbols; a patcher that resolves targets by name at
   apply time (as the plugin does at runtime) survives origin releases a raw
   offset diff cannot. Version support becomes "manifest resolves" —
   testable, refusable, and honest when it fails.
3. **Refuse-don't-guess:** unresolvable target → the patcher declines with a
   named reason. No partial application ever.
4. Distribution form (patch the user's exe in place vs. produce a patched
   copy side-by-side) is a user/legal call, explicitly out of scope here.
   The tracker is identical under either.

---

## 4. Open questions for the reviewer

1. **Does the runtime-control loop (Phase B) justify its cost** — seam
   receipts + at least two full user playthroughs — against shipping a patch
   whose completeness is only statically argued? (Foundry's position: yes;
   the black-screen failure mode is silent by construction and was actually
   reached once already.)
2. **Manifest-guided patching**: is apply-time symbol resolution feasible in
   whatever patch engine gets chosen, or does it force writing our own
   applier? (Nobody has scoped an applier; this is genuinely unknown.)
3. **Where does the GATED class ship from** — is carrying an origin-proposal
   dependency in the *tracker* (not the patch) acceptable bookkeeping, or
   should GATED collapse into ABSENT until origin answers?
4. **Ratchet governance**: v2 numbers land as a new pinned scope beside v1.
   Is dual-baseline acceptable, or does the reviewer want a single re-pin
   with the v1 numbers archived to the tale only?
5. **The deferral set** (enumerated ∧ never-fired): who owns deciding
   dead-vs-dormant? Deleting a dormant seam breaks non-Outset content later;
   keeping a dead one bloats the tracker. Proposed: classified now, decided
   per-row at migration time, never in bulk.

---

## 4b. AMENDMENT RECEIVED (outside reviewer, 2026-08-16) — Foundry assessment

The reviewer's amendment (plugin-first intake · load-bearing declarations ·
build-integrated enforcement · retirement ledger) was assessed against the
tree on request. **Verdict: sound, adoptable, with six modifications.** Full
assessment routed to the reviewer; the tree-verified facts it rests on:

- `hook.hpp:160-175` citation **exact** — `hook_install` resolves lazily
  per-entry and returns `MOD_UNAVAILABLE` on a null target (read 2026-08-16).
- **19c's load-time gate is ABSENT** — `fold_control.py` exit 2: *"DETECTOR:
  ABSENT — nothing in src/dusk/mods or tools implements address-collision
  detection"* (control audit, this session). §5b depends on it existing.
- **No route library exists** (docs swept); **V10-a's emission point is
  [QUEUED]** in BUILD-QUEUE:266, not landed → **retire-by-judgment is not
  licensable at switch-on; retire-by-migration only.**
- `git config core.hooksPath` is **unset on this clone and no `.githooks/`
  exists** — the pre-commit half must be created, not merely verified.
- `build_run.bat` is **non-interactive** — the §5 declaration must be a
  flag/queue-derived input, never a prompt.
- The ownership map would be the **FIFTH** roster, not the second
  (`layer_census`: 83/38/17/102 `.cpp`, measured disagreeing) — it must
  REPLACE the four, and its states must reuse `INVENTORY-SCHEMA.md`'s
  levels, which already define declared/linked/exercised.

## 4c. ROUND 2 (reviewer, 2026-08-16) — outcome

- **Modifications 1, 3, 4, 5 + both Foundry gaps: adopted.** Five reviewer
  refinements accepted (ownership map = adjudication work needing an owner ·
  stable row IDs, symbol as a field · retirement entries EMBED the removed
  hunk · `dStage_getName` synthesis gets its own hazard row · states reuse
  INVENTORY-SCHEMA.md).
- **Modification 2 REJECTED and replaced, correctly:** the declaration is
  DELETED — the diff already carries all its information; enforcement lives
  in the build toolchain, role-independent. Premise disconfirmed by the
  user: the queue has been forgotten and lanes have built directly.
  **Standing principle adopted: anchor to the artifact, not the actor.**
- **V10-a conflict PINNED BY MEASUREMENT (Foundry, 2026-08-16):** ONE
  emission point, not two. The `[QUEUED]` header at BUILD-QUEUE:266 is
  STALE — the row's own body records the code landed (§918,
  `ww_profile_register.cpp:361-400`, cl /Zs clean). **But all ten retained
  runtime logs are VANILLA-exe runs (rev 95608438) with ZERO `[V10a]` lines
  and ZERO resolve events** — the emission lives in a fork TU and no
  retained artifact shows it ever ran. The "33 register / 13 resolve /
  59.5% EXERCISED" figures have no artifact behind them on this machine.
  **Freeze verdict: retire-by-judgment FULLY frozen until a fork build
  emits and its log is pinned under a build ID** — which is §7's ledger
  point demonstrated on ourselves.

## 4d. ROADMAP STATUS (re-assessed 2026-08-16, post-adjudication)

- **A1 DONE** — map cut at 162/162 (`ww-ownership-map.json` authoritative;
  partitions demoted to views), owner split as specced + Housing on the
  leg-carriers. History-confirmed; one mis-verdict found and flipped.
  **Corrections to the roadmap text: disputed set was 152, not ~85; agreed
  core 10, not a majority.**
- **A2 half-done, superseded** by the enumerating-matcher law (landed in the
  three lineage matchers with fired controls). **tier2's `\bww_` remains the
  open half** — diagnosed, fix named, held for gate review.
- **A3 not built** (design held). **A4 prefigured** — ratchet baseline live;
  **completion redefined: TU == derived floor (6 hard, ≤22), pessimistic
  enumeration** — "monotonically decreasing" now reads "toward the floor".
- **A5 held**, one obligation open: reconcile the map's `_categories` with
  INVENTORY-SCHEMA in one pass.
- **OQ1 RESOLVED:** one emission point; [QUEUED] header stale, code landed,
  **zero runtime receipts** (ten retained logs, all vanilla, zero [V10a]) →
  **C3 FULLY frozen**; thaw = first fork run's log pinned under a build ID.
- **OQ2 RESOLVED** by execution.
- **B unbuilt as designed**; B1's classifier input now exists (map + domain
  validation, exercised); B4 gains the read-timestamp convention.
- **D2 resolved per OQ1; D1/D3 must-build; D4 confirmed.**
- **E2 substantially done:** fabrication + silent wrong-name branch read at
  source, constraint proven live (TagSo/TagMSo), `(proc,argument)` sufficient
  today, severity bounded. **Remaining: donor-side comparison.**
- **Principles earned: §P3** match-broadly/validate-domain/surface-loudly ·
  **§P4** enumerate the side that cannot grow, fail pessimistic; corollary:
  cite the artifact, never a lane's citation of it.

## 5. What this plan refuses to do

- **No entry ships on "probably needed."** Doorway-proven or absent from the
  patch.
- **No baseline edits.** Superseded numbers stay stamped; new scopes pin
  beside them.
- **No hand-maintained lists.** Every count in the tracker re-derives by
  command or it is prose, not a count.
- **No action before review.** This document is the deliverable; the first
  build waits for the outside source and the user's go.
