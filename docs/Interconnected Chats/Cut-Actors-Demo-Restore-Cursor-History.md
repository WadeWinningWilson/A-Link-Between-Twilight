# Cut Actors / Demo Restore — Cursor ↔ History

**Interconnected Run** — cut-actor spawn diagnosis, Demo leftover stitch/taxonomy, careful naming, Moblin remnant hunt.

| | |
|---|---|
| **Protocol** | [INTERCONNECTED-RUNS.md](INTERCONNECTED-RUNS.md) |
| **Live state** | [state/cut-actors-demo-restore.md](../state/cut-actors-demo-restore.md) |
| **Run control** | [state/run-control.md](../state/run-control.md) |
| **Roles** | **Cursor** = implement + build (after dual sign-off) · **History** = disc archaeology / taxonomy / rename policy / critical review |
| **Drive** | **Not in scope** unless user asks mid-run |
| **Status** | **deliberating** — Phase B done; Phase C draft below |

---

## Run header

| Field | Value |
|-------|--------|
| **run_name** | Cut Actors / Demo Restore — Cursor ↔ History |
| **created** | 2026-07-17 |
| **live_state** | `docs/state/cut-actors-demo-restore.md` |
| **roles** | Cursor = implement · History = archaeology + critique |
| **drive_in_scope** | **no** |
| **status** | deliberating — Phase C draft |

---

## User brief (required before Start)

### Goals

1. **Diagnose and fix cut-actor spawn** for `E_ms` and `E_GS` (create-path logging → safe params → heap/resLoad), so they can be evaluated as live actors, not conflated with Demo meshes.
2. **Park / safely restore TestCube** — understand RelWithDebInfo crash (`Actor SubMtd = NULL` outside `#if DEBUG`); propose PC restore path using disc `K_cube00` / `K_cube01` / `K_cyli00` without crashing playtest builds until gated properly.
3. **Demo leftover lane** — treat Demo BMD/BDL pieces (hands/bodies/faces) as stitch candidates; keep viewer as the inspection tool; plan eventual reunification (inspired by community modder work) without rushing renames.
4. **Moblin remnant hunt** — History-led evidence pass (Demo cast, Object arcs, maps, WW `Mo2` contrast, trailer/external notes). Do not treat `E_OC2` “Moblin” UI label as proof.
5. **Naming policy** — disc / proc / arc names stay primary; display aliases only after dual APPROVED evidence (joints, BMD hash, create path). No mass catalog renames.

### Guidelines

- Build: `build_run.bat` → RelWithDebInfo; wipe GPU caches after builds; follow [build-fps-guidelines.md](../build-fps-guidelines.md). Agents build — do not ask the user to build.
- Dual sign-off **required** before implementation (Phase D). No code until both **APPROVED**.
- History: no source edits unless user amends brief; own evidence tables, alias proposals, Moblin search plan.
- Cursor: may instrument/spawn-tool after sign-off; keep Demo viewer and Cut Actors as separate lanes.
- Do **not** rename catalog keys or disc filenames without dual APPROVED + user OK.
- Do **not** conflate Demo mesh spawn success with actor proc spawn success.
- Level Editor Interconnected Run is **superseded for run-control** while this run is active; do not continue Level Editor execute under this traffic light.
- Push / commit only when user asks.
- Related prior work: `tools/demo_cut_content/`, `d_demo_leftover_viewer`, `d_cut_actor_spawn`, `docs/TPHistory.md` §7, Custom Models Beta Link pack.

### Expectations

- Critique seriously — rubber-stamping is a protocol violation.
- Playtest bar: spawn must be reproducible; failed creates logged; TestCube must not crash session until restore is intentional and gated.
- Alias table may land in docs/state or run-doc decisions; viewer UI soft-labels only after APPROVED.
- Drive session: **not requested**.
- **User process (2026-07-17):** All deliberation and progress goes in the **run doc** (and live-state tip). Do not rely on chat-paste ferrying as the record. When a **unified plan** is ready, **tell the user once** so they can give it a pass before continue/execute — then wait.

### Drive

- [x] Not requested  
- [ ] Requested — notes:

---

## Context snapshot (pre-deliberation — not a plan)

| Item | Known fact |
|------|------------|
| Demo arcs | ~365 unique BMD/BDL; leftover viewer + full catalog exist |
| Cut Actors UI | Presets include E_ms, E_GS, TestCube, etc.; despawn tracks up to 32 IDs |
| `E_MS.arc` / `E_gs.arc` | Present on extract; RELs `d_a_e_ms` / `d_a_e_gs` in dylink |
| E_ms params | Bits 16–23 = switch; `params=0` checks switch 0 → can ERROR; try `0xFFFF0000` |
| E_gs heap | `entrySolidHeap` size `0x4B000` — large; heap fail plausible |
| TestCube | Methods `#if DEBUG` only; non-DEBUG profile uses `NULL` methods → crash; assets on disc |
| E_OC2 label | UI said Moblin; playtest looked Bokoblin — treat as mislabel until evidence |
| User stance | Moblin remnants likely somewhere; Demo partials will reunite later; careful renaming; TestCube wanted eventually |

---

## Deliberation log

### Phase A — Proposed plans

#### Cursor — proposed plan

*(Posted 2026-07-17. Independent Cursor proposal — History writes its own plan; do not copy this.)*

**Sequencing:** diagnose spawn → park TestCube → naming/alias policy → Moblin hunt (parallel History) → stitch roadmap (later milestone).

1. **Instrument cut-actor create (small, first code after sign-off)**  
   - In `d_cut_actor_spawn` (or create wrappers): log proc name, params, `fopAcM_create` return, and if possible create-phase / ERROR reason for E_ms / E_GS.  
   - Goal: one playtest tells switch-kill vs heap vs resLoad vs REL.

2. **E_ms spawn fix**  
   - Change preset (or add param override) to `0xFFFF0000` so switch bytes are `0xFF`.  
   - Confirm `E_MS` res indices 20/16 still match disc.  
   - If heap `0x1860` fails after switch fix → separate heap bump proposal (needs evidence).

3. **E_GS spawn fix**  
   - Same logging.  
   - If heap fail: try quieter room / document requirement; only then propose heap size change with measured failure.  
   - Cross-check Demo “ghost soldier” meshes vs `E_gs` BMD (History can own joint/hash table; Cursor can dump names).

4. **TestCube**  
   - Immediate: mark preset **DEBUG-only / crashes RelWithDebInfo** in UI note; refuse spawn or no-op with message on non-DEBUG.  
   - Later (own mini unified plan): `#if TARGET_PC` (or DEBUG||TARGET_PC) enable methods + load `K_cube00` via placeable arg — **do not** enable in this first execute slice unless History agrees it’s safe.

5. **Renaming**  
   - Ship an **alias table** (display only): `arc/file → soft label → confidence → evidence link`.  
   - No catalog key renames in first execute. Wrong labels (e.g. Demo09 ym as Link, E_OC2 as Moblin) get demoted to “unverified” or corrected alias only after History evidence.

6. **Moblin**  
   - Cursor does **not** lead the hunt. After History posts search plan + first evidence table, Cursor may add viewer filters / spawn presets if dual-approved.  
   - Keep WW `Mo2` / trailer notes in History lane.

7. **Stitch (backlog this run unless amended)**  
   - Document candidate groups (body+face+hand) in live state; no auto-join pipeline yet.

**Out of this first execute slice:** mass rename, full stitch tool, enabling TestCube draw path without gate.

#### History — proposed plan

*(Posted 2026-07-17. Independent — written without reference to Cursor's plan, per protocol.)*

**Framing:** every lane below is evidence-first: establish what the DISC and the decomp
say must be true, then let implementation confirm against predictions — never the
reverse. The E_ms "Moblin" folk-label (overturned by skeleton + audio evidence,
TPHistory §3.3) is the cautionary tale this run exists to not repeat.

**H-1. Spawn diagnosis as falsifiable predictions (E_ms, E_GS)**

Before any instrumented build, History posts a **create-path precondition table** per
actor, derived from the decomp + disc, each row a testable prediction:

- *E_ms*: (a) params semantics from `daE_MS_Create` — which bit ranges are read
  (switch bytes 16–23 per snapshot; History verifies against source and documents ALL
  read fields, not just the suspected one); (b) resource preconditions — `E_MS` arc
  present, res indices 20 (BMD) / 16 (BCK) match the retail index (already verified in
  `E_ms.h`); (c) heap ask vs typical field free space; (d) `l_objectName`
  routing (`E_ms` → `fpcNm_E_MS_e`, verified). Prediction to beat: **create ERRORs at
  the switch check, not at resources** — resources are provably intact. If logging
  shows otherwise, the table says exactly which precondition to look at next.
- *E_GS*: History first answers a question the snapshot skips: **is E_GS placed in any
  retail stage?** (Ghost/soldier ambients may be shipped-but-rare rather than cut —
  the two have different diagnosis priors. A used actor failing to spawn in a field
  points at environment preconditions — room type, time-of-day, event bits — before
  heap.) Method: DZR placement sweep for the 8-char name across all 79 stages
  (read-only scripting over the extraction; no source edits). Deliverable: placement
  count + example stages, plus the same precondition table (incl. the `0x4B000` heap
  ask in context: what do comparable enemies request?).
- Shared rule: **one unexplained failure = stop and re-table**, don't tweak params
  by trial. Trial-and-error spawning is how mislabels get born.

**H-2. TestCube — restore against the reference build, not from scratch**

The archaeology fact that should anchor this lane: **the Shield port ships debug RELs
for every actor** (TPHistory §6.3), i.e. a reference `d_a_obj_testcube` with its
methods COMPILED IN exists on disc for a build we own. Plan: (a) short-term, the
preset must be inert on non-DEBUG (any UI note must say *why*: profile methods are
`#if DEBUG`, so the shipped profile carries NULL method pointers — the crash is
structural, not a bug to fix by poking); (b) restore path = mirror the debug build's
gating (compile methods under `DEBUG || TARGET_PC`), with the Shield debug REL as the
behavioral reference if questions arise; (c) History verifies the asset side first:
which arc actually carries `K_cube00/K_cube01/K_cyli00`, confirmed against the
resource indexes, before anyone wires a load. No restore lands this run unless the
gate story is airtight in the unified plan.

**H-3. Demo taxonomy — structure without renaming**

Treat the 365-entry catalog as an excavation ledger, not a naming problem. Proposed
taxonomy, all machine-derivable, all stored as ADDITIVE columns (disc names stay the
only keys):

1. **Filename grammar decode** (already latent in the names):
   `demo<NN>_<actor>_cut<NN>_<part>_<variant>` — parts vocabulary (`bd/fc/hd/hl/hr/
   gp/wl…`), variant vocabulary (`_o`, `_l`, `high`, `original`, `tmp`, `henkei`,
   `blend`, `tongue`). History delivers the decoded grammar table with meanings
   evidenced (e.g. `original` vs `high` body pairs in Demo01_00), flagging unknowns
   as unknowns.
2. **Skeleton families**: joint-name lists per BMD (cheap header parse) → same-rig
   grouping. This is the stitch-candidate generator: a body+face+hands trio is only a
   stitch candidate if the rigs agree. Also the mislabel killer (a "Link" label on a
   non-Link rig dies in this table).
3. **Cutscene co-occurrence**: same arc + same `cut` number = one composite character
   as the cutscene team built it — the natural reunification unit. The sumo composite
   (Custom-Model-API §5) is the in-engine precedent for runtime multi-piece
   composition; stitching should be RUNTIME composition against these groups, never
   file surgery. This run: candidate groups documented in live state only.

**H-4. Moblin hunt — fingerprint search, not vibes**

Design (History-led, read-only):

1. **Build the reference fingerprint first** from WW `Mo2.arc` (we own the
   extraction): joint-name list, animation-name vocabulary, texture names, weapon
   props (`Vboko` etc.). Without the fingerprint, "looks Moblin-ish" is noise.
2. **Sweep TP for partial fingerprint hits**, in evidence-strength order:
   (a) joint/anim/texture NAME occurrences across all TP arcs (the §2.4
   anim-vocabulary method that exposed the WW shot-list inheritance — it works);
   (b) symbol maps (`map/Final/Release`) for any mo/moblin-adjacent symbols;
   (c) audio namespaces — allocated-but-orphaned `Z2SE_EN_*` families (the E_ms
   9-SFX precedent shows late cuts leave audio scars); (d) the Demo cast (any
   quadruped/orc-rig outlier in H-3's skeleton families); (e) kiosk/Wii-rev0
   leftovers (the `E_base.arc` precedent shows dev arcs leaked once — check its
   members' provenance too).
3. **E_OC2 evidence protocol**: the UI label is the hypothesis under test, not a
   fact. Compare E_OC2's mesh/joints against (i) E_oc base and (ii) the Mo2
   fingerprint; document weapon-mesh provenance (`OC_WEPON/OC2_WEPON` weld — prior
   research says bokoblin skeleton + nata SFX). Expected outcome per existing
   evidence: **demote the label**; the hunt continues elsewhere.
4. **Deliverable**: one evidence table (find / fingerprint element matched /
   strength / interpretation), including the honest null result if that's what the
   disc says — "fully cut before GZ2E01" is a finding, not a failure. External
   record (2004 trailer, TCRF) cited as context, never as evidence of presence.

**H-5. Alias policy (History owns; proposal for unified plan)**

Three tiers, one table, display-only:

| Tier | Meaning | Rules |
|---|---|---|
| **Primary name** | disc file / arc / proc name | immutable, the only KEY anywhere (catalog, code, docs) |
| **Evidence alias** | display label backed by an evidence bundle | requires: skeleton/joint evidence OR byte-lineage (hash) OR create-path citation; **dual sign-off to promote**; every row links its evidence |
| **Folk label** | community/UI lore without evidence | rendered as `"…?" (unverified)`; demotable unilaterally (removing an unproven claim needs no ceremony); never promotable without moving to tier 2 |

Confidence enum on every alias: `confirmed / probable / speculative`. The table lives
in ONE place (proposal: `docs/state/cut-actors-demo-restore.md` appendix or a small
generated include the viewer reads for soft labels — decide in Phase C). Existing
demotions to apply on day one: `E_OC2 "Moblin"` → folk label; any Demo entry whose
label asserts an actor identity the rig table hasn't confirmed. No catalog keys, disc
names, or code identifiers renamed — ever — under this policy; a future rename would
be its own user-approved run.

**H-6. Sequencing (History's view)**

Evidence tables (H-1 preconditions, H-4 fingerprint) are PRE-implementation work —
History starts them immediately after dual sign-off and they gate nothing Cursor
needs for instrumentation; the two lanes converge when spawn logs meet predictions.
H-3 taxonomy is background-parallel. H-2 restore is LAST and optional this run.
History does not touch source throughout (brief-compliant); scripted read-only disc
analysis lands under `tools/demo_cut_content/` where the existing toolkit lives.

*Not signing off yet — awaiting ferry to Phase B per protocol.*

### Phase B — Critical responses

#### History — critique of Cursor’s plan

*(Posted 2026-07-17.)*

**What holds (genuine agreement, not courtesy):** the overall sequencing
(instrument → park TestCube → policy → parallel hunt → stitch backlog) is correct;
instrument-before-fix is the right instinct; display-only alias table matches my H-5;
the TestCube two-step (inert now, gated restore as its own mini-plan) matches my H-2;
Moblin deference to History is correct role hygiene; stitch-as-backlog matches H-3.

**What fails or carries risk:**

1. **Step 2 commits the fix inside the diagnosis (protocol inversion).** Changing the
   E_ms preset to `0xFFFF0000` in the same slice as the instrumentation means a
   successful spawn confirms nothing — you changed the variable and the measurement
   together. Worse, `0xFFFF0000` sets bits 16–31, but the snapshot only attributes
   bits 16–23 to the switch. **Bits 24–31 are an unread field** — in TP actors the
   top param byte very often selects type/variant. Setting it to 0xFF is an
   uncontrolled second change (E_OC's angle-byte mesh variant is the cautionary
   precedent in this very run). Counter-proposal: slice 1 logs with the CURRENT
   params and captures the failure; History's H-1 table (create-func read) then
   specifies the minimal param delta — likely `0x00FF0000` (switch byte only), not
   `0xFFFF0000`; slice 2 applies exactly that and the log confirms the predicted
   failure disappeared. One variable at a time.

2. **Step 1's logging target is underspecified and will miss phase-stage failures.**
   `fopAcM_create` returning an ID is not spawn success — TP actor creates run a
   multi-frame phase machine (`dComIfG_resLoad` → cPhs codes), and E_ms's create
   loads resources exactly that way. An actor can return a valid ID and then die in
   phase with cPhs_ERROR — indistinguishable from success in Cursor's proposed log.
   The brief's own "do not conflate" warning applies *within* actor spawning too.
   Requirement: hook or observe the CREATE-PHASE outcome (per-phase return codes),
   not just the factory return. Otherwise "one playtest tells switch vs heap vs
   resLoad" is not actually achievable — those distinctions live in the phase codes.

3. **Step 3's "try quieter room" starts from an unverified prior.** Whether E_GS is
   heap-starved is secondary to a question the plan never asks: **is E_GS placed in
   retail stages at all?** Ghost soldiers plausibly ship (sewers/graveyard family);
   a shipped actor failing in an arbitrary field room points first at environmental
   preconditions (event bits, room/time gating), not heap. My H-1 DZR placement
   sweep answers this deterministically before any build. If placements exist, the
   control experiment is "spawn in a room where retail places it" — strictly more
   informative than "a quieter room." If none exist, heap hypotheses get promoted.
   Sequencing ask: placement sweep BEFORE the E_GS instrumentation slice.

4. **Step 2's "confirm E_MS res indices 20/16 still match disc" is already done** —
   verified against the generated retail index (`E_ms.h`: BMD 0x14, BCK 0x10 block).
   Don't respend the effort; cite the evidence and move on.

5. **TestCube later-plan detail: "load K_cube00 via placeable arg" conflates two
   mechanisms.** The placeable table routes `unitC00/unitS00/unitCy0` to ONE proc —
   mesh selection almost certainly comes from the stage NAME/subtype path, not a
   free param. The restore design must first document how the actor selects its
   mesh (and which arc actually carries `K_cube00/01/cyli00` — unverified). Fine to
   defer, but the mini-plan should inherit these as open questions, not assumptions.

6. **Step 5's demotion timing is backwards for folk labels.** Cursor has wrong labels
   "demoted to unverified **after History evidence**" — but removing an UNPROVEN
   claim requires no evidence; only promotion does. `E_OC2 "Moblin"` and the Demo09
   ym label should be demoted on day one of execute (display-only change), while
   promotion to any new alias waits for the evidence bundle + dual sign-off
   (asymmetry is the point of H-5). Also unspecified: WHERE the alias table lives
   and how the viewer consumes it — one canonical location must be picked in Phase C
   or UI labels and docs will drift.

7. **Missing constraint — log and lifecycle hygiene.** (a) Create-path logging must
   be event-scoped (one line per spawn attempt), never per-frame, and gated/stripped
   before any commit per repo discipline. (b) Acceptance must include the
   **spawn → despawn → respawn cycle** without crash/leak: both actors phase-load
   resources, and deleting mid-phase is this codebase's best-documented crash class
   (dangling-phase/heap lessons). The spawner's 32-ID despawn tracking makes this
   cheap to test and the plan never mentions it.

8. **Minor: step 3's ad-hoc "Cursor can dump names" for ghost-mesh cross-check
   duplicates H-3.** The skeleton-family table generates this comparison
   systematically for ALL Demo entries; feed one table rather than two dump paths.

**Missing entirely from Cursor's plan:** per-lane acceptance criteria ("diagnosed"
= the log's failure code matches a precondition-table prediction; "fixed" = minimal
delta applied + predicted failure gone + cycle test passes). Propose these land in
Phase C verbatim.

*Awaiting Cursor's critique of the History plan; not signing off yet.*

#### Cursor — critique of History’s plan

*(Posted 2026-07-17. Critique of History Phase A + notes on History’s Phase B of Cursor.)*

**What holds (genuine agreement):**

- Evidence-first framing and the E_ms folk-label cautionary tale are the right north star for this run.
- **H-1 falsifiable precondition tables** + “one unexplained failure = stop and re-table” beats trial-and-error spawning. Accept for Phase C.
- **H-1 E_GS DZR placement sweep before promoting heap** is a real gap in my Phase A; accept. Control experiment = retail placement room when count > 0.
- **H-2 TestCube inert + structural NULL-methods explanation + asset verify before wire** — accept. Shield debug REL as behavioral reference is useful *after* gate story is locked.
- **H-3 additive taxonomy** (grammar / skeleton families / cut co-occurrence) + runtime composition not file surgery — accept; stitch stays backlog.
- **H-4 fingerprint-first Moblin hunt** and E_OC2 as hypothesis under test — accept; null result is a valid finding.
- **H-5 three-tier alias policy** (primary / evidence / folk) + asymmetric demotion — accept; pick one table home in Phase C.
- History Phase B points on **phase-outcome logging**, **spawn→despawn→respawn cycle**, **event-scoped logs**, and **acceptance criteria** — accept; my Phase A was underspecified there.
- History Phase B point on **not bundling param fix with first instrumented playtest** — accept one-variable sequencing.

**What fails or needs correction:**

1. **Fact error in History Phase B §1: bits 24–31 are not “unread.”**  
   `daE_MS_Create` reads them: `field_0x5b8 = GetParam() >> 0x18`. If `!= 0xff`, it can set `mAction = 10` / `field_0xba6` based on another switch. So `0xFFFF0000` *is* a two-field change (History’s sequencing objection still stands), but the top byte is a **documented behavioral field**, not noise. Minimal create-success delta for the *switch ERROR path* is `0x00FF0000`. Separately, if the actor then “exists but acts dead/hidden,” the next single-variable trial is top byte `0xFF` (`0xFF00FF00` only after the switch slice is proven). H-1’s “document ALL read fields” must list: bits0–7 → `5b6`, 8–15 → `5b7`, 16–23 → create-time switch kill, 24–31 → `5b8` action/switch path.

2. **H-6 sequencing contradicts H-1 / History’s own Phase B.**  
   H-6 says evidence tables “gate nothing Cursor needs for instrumentation,” but Phase B correctly requires the **E_GS placement sweep before the E_GS instrumentation slice**. Resolve in Phase C as:  
   - Slice 0 (History, read-only, can start as soon as dual APPROVED — or even during Phase C draft): E_ms full param-read table + E_GS/E_ms DZR placement counts.  
   - Slice 1 (Cursor): instrumented spawn with **current** presets; log create-phase outcomes.  
   - Slice 2: minimal param delta per table (E_ms: `0x00FF0000` first).  
   - E_GS heap hypotheses only after placement prior is known.

3. **“History starts tables after dual sign-off” is too late for the E_GS gate.**  
   Placement sweep and E_ms param field table are pure archaeology — they should be History’s first deliverable *immediately on APPROVED*, and ideally drafted into the run doc during Phase C so Cursor does not idle. Do not wait for Cursor’s first build to begin H-1.

4. **Shield-debug-REL restore path (H-2b) understates port risk.**  
   Mirroring `DEBUG || TARGET_PC` is the right *gate shape*, but debug REL paths often carry HIO / `OSReport` / MoveBG assumptions that are fine in DEBUG and noisy or crashy on PC. Phase C mini-plan for TestCube must require: methods non-NULL + asset index confirmed + **draw/execute smoke on TARGET_PC** as its own acceptance — not “compile like Shield and ship.”

5. **Folk-label day-one demotion needs an execute owner.**  
   History correctly says demotion needs no ceremony, but soft labels today live in viewer/catalog UI code Cursor owns. Phase C must assign: History marks rows in the alias table; Cursor applies display demotions in one small UI/data change in slice 1 (no renames). History cannot demote in-engine unilaterally under the brief.

6. **H-4 fingerprint sweep — false-positive budget missing.**  
   Shared Zelda enemy naming (`boko`, weapon props, generic joint roots) will hit Bokoblin/Bulblin families. Acceptance for a “hit” must require **multi-element** fingerprint (e.g. joint cluster + anim vocab + weapon prop), not a single name occurrence. Otherwise the hunt floods with Bokoblin noise and re-creates the E_OC2 mislabel.

7. **H-3 “machine-derivable” grammar is partly aspirational.**  
   Part tokens (`bd/fc/hd…`) are real; full `demo<NN>_…` regularity is not uniform across all 365 uniques. Phase C should say: decode what parses cleanly; **unknown grammar stays unknown** — no forced columns that invent structure.

8. **Missing from History plan: Cursor acceptance for instrumentation itself.**  
   History demands phase codes (good) but does not specify the observe point. Cursor proposal for Phase C: log at cut-actor spawn site + a create-complete/fail callback or polled phase until `COMPLEATE`/`ERROR`/`one-shot timeout`, one line per attempt, including proc, params, phase code, and whether ID was later despawned. Without that contract, “match precondition prediction” cannot be checked.

9. **Minor: E_ms res-index “already verified in E_ms.h”** — accept skip of rediscovery, but slice-1 log should still print resLoad phase so a *future* bad extract fails loudly; citing the header is enough research, not enough runtime proof on the user’s disc path.

**On History’s critique of Cursor — absorbed vs disputed:**

| History B point | Cursor stance |
|---|---|
| Don’t fix params in same slice as first log | **Absorbed** |
| Prefer `0x00FF0000` first; top byte separate | **Absorbed** (and stronger: top byte *is* read) |
| Phase-outcome logging required | **Absorbed** |
| E_GS placement before heap story | **Absorbed** |
| Skip rediscovering 20/16 as research | **Absorbed**; keep runtime resLoad in log |
| TestCube placeable-arg assumption weak | **Absorbed** — open questions for mini-plan |
| Folk demote day one; promotion gated | **Absorbed** with Cursor as apply-owner |
| Log hygiene + despawn cycle | **Absorbed** |
| Don’t duplicate ghost-mesh dumps vs H-3 | **Absorbed** |

**Not signing off yet.** Ready for Phase C unified plan that merges: one-variable E_ms slices, phase logging contract, History H-1 tables before E_GS, H-5 alias home, TestCube inert-only this execute, Moblin multi-element hits, stitch backlog.

### Phase C — Unified plan

*(Cursor draft 2026-07-17 — absorbs both Phase B critiques. History: REQUEST CHANGES or APPROVED.)*

#### C-0. Roles & homes

| Lane | Owner | Artifact home |
|------|-------|---------------|
| Create-path precondition tables (E_ms, E_GS) | History | Run doc appendix + tip in live state |
| DZR placement sweep (E_ms, E_GS) | History | Same; counts + example stages |
| Instrument + spawn/despawn tooling | Cursor | `d_cut_actor_spawn` (+ UI notes) |
| Alias table (3-tier) | History owns rows; Cursor applies soft labels | `docs/state/cut-actors-demo-restore.md` appendix (canonical); viewer reads for display only |
| Moblin fingerprint hunt | History | Evidence table in run doc / live state |
| Demo taxonomy (grammar / rig / cut groups) | History | Additive columns / tables; disc names remain keys |
| TestCube restore | Deferred mini-plan | Not in first execute |

#### C-1. Execute slices (ordered)

**Slice 0 — History archaeology (starts on dual APPROVED; no source edits)**  
1. E_ms param-read table: bits 0–7 `5b6`, 8–15 `5b7`, 16–23 create switch-kill, 24–31 `5b8` action/switch path — cite `daE_MS_Create`.  
2. E_GS (and E_ms) DZR placement counts + example stages.  
3. Alias appendix: demote `E_OC2 "Moblin"` and unconfirmed Demo actor-identity labels to **folk** (`…? (unverified)`).  
4. Kick Mo2 fingerprint build (H-4.1); full TP sweep can continue in parallel after Slice 1.

**Slice 1 — Cursor instrument + park + folk demote apply (no param “fixes” yet)**  
1. Event-scoped create logging: proc, params, factory ID, **create-phase outcome** (`COMPLEATE` / `ERROR` / timeout), never per-frame.  
2. Keep current E_ms / E_GS presets for first measured failure.  
3. TestCube: refuse/no-op on non-DEBUG with UI note explaining NULL methods.  
4. Apply folk demotions from alias appendix in viewer/Cut Actors display only.  
5. Acceptance: one spawn attempt produces one log line whose failure mode is classifiable; spawn→despawn→respawn cycle no crash.

**Slice 2 — E_ms minimal delta (only after Slice 1 log matches prediction)**  
1. If Slice 1 shows switch-kill at bits 16–23: set params to **`0x00FF0000` only**.  
2. Re-log; predicted ERROR gone.  
3. If actor present but wrong action/hidden: **separate** trial for top byte `0xFF` (document as second delta).  
4. Acceptance: prediction match + cycle test.

**Slice 3 — E_GS (after Slice 0 placement prior)**  
1. If retail placements exist: spawn in a known placement room as control; compare to arbitrary field.  
2. Log heap vs resLoad vs env; promote heap-size change only with measured `entrySolidHeap` fail.  
3. Acceptance: same as Slice 1/2.

**Out of first execute:** TestCube method enable, stitch pipeline, catalog key renames, evidence-alias promotions (need dual APPROVED per row).

#### C-2. Acceptance criteria (verbatim)

| Claim | Means |
|-------|--------|
| Diagnosed | Log phase/failure matches a precondition-table prediction |
| Fixed | Minimal documented delta applied; predicted failure gone; spawn→despawn→respawn OK |
| Demoted | Folk label display shows unverified; primary disc/proc name unchanged |
| Moblin hit | ≥2 independent fingerprint elements (joint cluster / anim vocab / weapon prop / audio); single name ≠ hit |

#### C-3. Sign-off table

| Role | Sign-off | Notes | When |
|------|----------|-------|------|
| Cursor | APPROVED | Draft author; ready to execute on History APPROVED | 2026-07-17 |
| History | **APPROVED** | Signed after source verification (not rubber-stamped): read `daE_MS_Create` in full. Both Phase-B accounts needed refinement — bits 16–23 = KILL-switch (set → `cPhs_ERROR`, line 1016-1019; `0xff` = sentinel), bits 24–31 = ENABLE-switch (unset → spawns DORMANT `mAction=10`, line 1054-1058; also polled at runtime, line 588). C-1's slices survive exactly as written; Slice 0 table now PREDICTS the intermediate state (with `0x00FF0000` in a switch-0-unset room: spawn succeeds but dormant → second delta `0xFFFF0000` full sentinel). My Phase-B "bits 24–31 unread" claim was wrong — Cursor's catch accepted; Cursor's field interpretation was swapped but its slice design was correct anyway. One non-blocking note for Phase D: the alias appendix→viewer consumption mechanism (runtime read vs mirrored constants) to be settled in Slice 1 — display-only either way. | 2026-07-17 |

**Implementation gate:** both rows must be **APPROVED** before Phase D (unless user override in Expectations).

### Phase D — Execute notes (short)

**History — playtest report №1 vs A1 predictions (2026-07-17):** user reports (1) E_ms
no spawn, (2) E_gs no spawn, (3) **CRASH on Makar preset**.

- *E_ms:* consistent with BOTH A1 branches — params=0 either ERRORs (kill-switch 0 set
  in room) or spawns DORMANT `mAction=10` (enable-switch 0 unset), which is visually
  identical to "didn't spawn." **The Slice-1 log line is the discriminator — need the
  phase code from the session log before Slice 2.** If log says COMPLEATE: it spawned
  dormant and `0xFFFF0000` (both sentinels) is the Slice-2 delta; if ERROR at the kill
  check: `0x00FF0000` first per C-1.
- *E_gs:* need its log line (it OS_REPORTs its params natively at create, plus Slice-1
  logging). Priors unchanged: environment first (D_MN09-only enemy), heap 0x4B000
  second. Control-room test (D_MN09 R03/R09/R12) remains the decisive experiment.
- *Makar CRASH — new finding, and it falsifies a spawner design assumption.* The
  d_cut_actor_spawn header says "stub RELs may spawn empty shells"; the crash says
  stub spawning is NOT safe. Decomp shows npc_mk as a pure stub (methods return true,
  actor size 0x1) — a 0x1-size actor struct means fopAc base-struct writes during
  create machinery land OUT OF BOUNDS. History proposal: **park ALL stub presets
  (npc_mk, npc_p2, npc_kdk) exactly like TestCube** (refuse/no-op + note) — same
  crash class, same remedy, arguably within Slice 1's park pattern; Cursor to apply
  as a Slice-1 addendum after confirming the crash site from the user's crashdump/log.

**History — playtest №1 LOG ANALYSIS (2026-07-17, log dusklight-20260717-223137): the
phase discriminators are already answered — no re-test needed before Slice 2.**

- **E_ms (pids 248, 249): create COMPLETED.** `fpcBs_Create OK` followed by the FULL
  resource load (all 14 `ms_*.bck` + `ms.bmd`) and no error → the kill-switch check
  passed (switch 0 unset in that room) and the actor reached the DORMANT branch —
  **A1 prediction branch B confirmed: it spawned, invisible, `mAction=10`.** Slice-2
  delta: recommend **`0xFFFF0000`** (both sentinel bytes) — the log has already
  discriminated both switch branches in one session, and both-sentinels is the
  semantically correct "no switches wired" spawner params; `0xFF000000` alone would
  also work in rooms where switch 0 is unset but is room-dependent. Rationale
  satisfies C-2 "minimal documented delta" via the A1 table.
- **E_gs (pid 272): create COMPLETED — heap hypothesis DEAD.** `entrySolidHeap
  0x4B000` succeeded (no fail line); it loaded `gho_gt.bmd` + `gho_gt_point/
  pointing/wait_a` BCKs. Two implications: (1) "didn't spawn" = spawned but not
  visible/idle — environment/visibility lane confirmed as the next question
  (D_MN09 control room stands); (2) **alias evidence: E_gs's resources are
  `gho_gt` = "ghost gatekeeper"** — a stationary POINTING ghost (wait + point
  anims), matching Temple of Time's way-pointing ghost soldiers (D_MN09-only
  placements corroborate). Alias tier: probable.
- **Makar (pid 274): crash mechanism confirmed from the log** — `procSize=1` create
  "OK" then immediate crash: the 0x1-size stub profile allocates a 1-byte proc and
  the framework's subsequent base-struct writes land out of bounds. Cursor's parking
  of all stub presets is the correct and sufficient remedy (already in new build per
  live state).
- Bonus from same session: E_dt (pid 250) and E_OC (pid 273) both created with full
  resource loads — consistent with shipped-actor reclassification.

Standing user rule recorded: during playtests, all agent lanes hold until the user
reports. (History lanes Mo2 fingerprint / E_IS check remain queued, resumed only
after playtest windows close.)

**Cursor — playtest №1 log pull + Slice-1 addendum (2026-07-17):**
- Session `dusklight-20260717-223137.log`: E_MS pid 248/249 created + full `ms_*` res
  load (still ambiguous kill vs dormant — resLoad runs *before* kill check). E_GS
  pid 272 created + `gho_gt*` res load. **NPC_MK pid 274 `procSize=1` → immediate
  `EXCEPTION_ACCESS_VIOLATION`** — History crash thesis confirmed.
- Slice-1 addendum built: refuse NPC_MK / P2 / KDK / HENNA0; log via `DuskLog` so
  `[CutActorSpawn] phase=` lands in session log.
- **Slice 2 landed:** E_ms preset params → `0xFFFF0000` (covers kill+enable sentinels;
  correct whether №1 was dormant or kill). E_GS label → ghost gatekeeper (probable).
  Build OK. Playtest: spawn E_ms should now be visible/active.
- **Slice 2 log confirm (2026-07-17, `dusklight-20260717-225422.log`):**
  `[CutActorSpawn] create … params=ffff0000` → `phase=COMPLEATE` for E_ms (pid 541).
  Also COMPLEATE: E_dt, E_S1, E_GS, E_OC2, beamos trio, E_IS, B_GO. Awaiting user
  visual confirm that E_ms is the visible rat (log ≠ eyeball).

- **2026-07-17 user pass:** Phase C approved to continue.
- **Slice 0 (History):** posted Appendix A1–A6 — E_ms/E_gs/E_dt reclassification (shipped); A5 demotions listed.
- **Slice 1 (Cursor) landed 2026-07-17:**
  - Create-phase observe: factory log + `tick()` → `COMPLEATE` / `ERROR` / `TIMEOUT` (`[CutActorSpawn]` OSReport, one line per attempt).
  - TestCube: refuse spawn when `!DEBUG` (NULL methods crash).
  - A5 display labels applied in `d_cut_actor_spawn` presets + Demo09 `ym` folk demote in catalog.inc.
  - **No param fixes** (still `params=0` for E_ms/E_GS) — Slice 2 waits on playtest logs matching A1 predictions.
  - Build: `build_run.bat` RelWithDebInfo OK; GPU caches wiped.
- **Playtest ask:** spawn E_ms / E_GS / TestCube; note status string + console `[CutActorSpawn]` lines; despawn→respawn cycle.

**History — playtest №2 (Slice 2) + evidence findings (2026-07-17):**

1. **E_ms: VISUALLY CONFIRMED — the rat appeared.** Alias promoted:
   `E_ms — rat family (shipped) — CONFIRMED` (placements + skeleton + audio + user
   sighting). The flagship diagnosis lane is CLOSED: dormancy semantics were the
   whole story, exactly per A1.
2. **E_IS: user visually identifies the Armos Titan.** Alias promoted (visual tier):
   `E_IS — Armos Titan (unused)` — matches the TCRF community record (TPHistory §6.3).
3. **B_GO despawn gap (Cursor item):** the Goron-Golem boss is a COMPOSITE — multiple
   goron child actors merge into one giant. The spawner's 32-ID despawn tracks only
   the parent → children survive despawn. Fix direction: track/cascade child procs on
   despawn (createChild lineage), or use a teardown that deletes the family.
4. **E_GS ↔ demo01_ctz1 hypothesis: FALSIFIED at the rig level.** Joint comparison:
   ctz1 = soldier rig with `armer01` ARMOR joint, naming grammar `armL1/legL1`
   (side-then-number); gho_gt = `arm1L/leg1L` grammar (number-then-side),
   backbone1/2, no armor joint; only generic humanoid names shared. Different
   models, different naming conventions (likely different authors/eras). The
   RESEMBLANCE is thematic — both knight-styled, and ToT's ghosts ARE ghost
   knights — but they are not the same asset. E_gs's invisibility discriminator
   remains: create COMPLETES, so the block is in execute/draw gating — next probe is
   either the D_MN09 control room or one draw-path log line (Cursor's call).
5. **"How do the parked ones show?" — two different answers.** (a) The stub actors
   (Makar/Medli/KDK/HENNA0) can NEVER show as-is: their code is gutted (25–53-line
   shells, actor size 0x1) and their models are NOT on the TP disc (full-disc SHA
   sweep found no WW NPC models). Showing them = future MOD work: import WW models
   through the custom-assets pipeline + new behavior (script-actor lane) — outside
   this run. (b) TestCube CAN show: assets on disc + the deferred H-2/C-1 mini-plan
   (`DEBUG || TARGET_PC` method gate) — still parked pending its own sign-off.
6. **BETA TWILIGHT BEAST LOCATED (user report: "I've seen it, don't know where it
   lies").** It lies in TWO generations on the retail disc:
   - **`Sample.arc / file2.bmd` (59,648 B)** — joint census this session: quadruped,
     `F_L/F_R/B_L/B_R_leg1-4` (four legs × four segments), `backbone1/2`,
     `neck1/2`, head — **the GDC 2005 trailer beast** (TCRF corroborates: Sample.arc
     holds the early Shadow Beast Link fights in a forest). Hidden behind the
     anonymous member name `file2.bmd` — which is why nobody finds it by name.
   - **`E_s1.arc`** — the second-generation prototype (jaw + tongue joints; full
     24-anim set incl. `S1_HOOK/PULL/DEFEND/SHRINK/STICK`), never loaded by retail
     code (`daE_S1_Create` always resLoads `E_S2`).
   **Viewer proposal (Cursor, small):** add both to the leftover viewer as
   out-of-Demo catalog entries (`Sample/file2.bmd`, `E_s1` model) so they can be
   seen in-game — display labels per H-5 evidence tiers.

**Cursor — playtest №2 follow-up (2026-07-17):**
- E_ms / E_IS labels → CONFIRMED in Cut Actors UI.
- B_GO despawn: cascade `createChild` (delete actors whose `parentActorID` is tracked) before parents.
- Demo Leftover Viewer extras: `Sample/file2.bmd` (GDC Twilight Beast), `E_s1/s1.bmd` (gen-2 proto) under Gold / All.
- E_GS: one-shot `[CutActorSpawn] E_GS draw-probe opacity=` after COMPLEATE (Draw early-outs if opacity < 1).
- Build OK; caches wiped.

**Cursor — playtest №3 log (`dusklight-20260717-231520`, 2026-07-17):**
- E_GS: `phase=COMPLEATE` + **`draw-probe opacity=1`** at create — then Execute fades
  opacity to 0 unless `daPy_py_c::checkNowWolfPowerUp()` (Wolf Sense). **Root cause found.**
- Sample + E_s1 viewer loads succeeded (`file2.bmd` 59648 B, `s1.bmd` 51840 B + anims).
- TestCube refuse confirmed.

**Cursor — playtest №4 follow-up (2026-07-17):**
- Relabel `Sample/file2` → **early Epona** (History joint match vs Horse/hs; not GDC beast).
- E_s1 remains beta Shadow Beast (user-confirmed).
- E_GS Cut Actors note: use Wolf Sense to see. Boko-flame bridge = proposal only (no impl).
- Build OK.

**Cursor — playtest №5 log (`dusklight-20260717-234548`):**
- B_GO despawn cascade **works**: `despawn cascaded 31 children` (×2).
- Sample `file2.bmd` loaded again; E_GS COMPLEATE + opacity probe=1 (Wolf Sense still required for sustained visibility).

**Cursor — playtest №5 follow-up impl (2026-07-18):**
- E_GS alias → CONFIRMED (Wolf Sense).
- E_dt: on COMPLEATE, force `ACT_WAIT`, restore feet pos, delete opening otama swarm.
- Demo viewer: E_s1 cycles all 24 BCKs via `mDoExt_McaMorf`.
- Build OK.

**Cursor — playtest №6 log (`dusklight-20260718-001158`):**
- E_dt: first spawn `deleted 0 otama`; second spawn **`deleted 20 otama`** + ACT_WAIT (override solid).
- E_S1 actor COMPLEATE (retail E_S2). E_s1 viewer anim cycle not in this log yet.

**User playtest №7 (2026-07-18, via Cursor chat):** E_dt spawns at feet but falls through
floor. Demo viewer E_s1 **cycles all BCKs** (confirmed). Question: fit gen-2 `E_s1` into Cut
Actors? History mid-Moblin — Cursor owns answer/plan.

**Cursor answer — E_s1 → Cut Actors (plan, no impl yet):**
- Cut Actors already has **`E_S1` proc** — but `daE_S1_Create` always `resLoad("E_S2")`
  (shipping mesh). Gen-2 `E_s1.arc` is a **different arc** (0 shared members vs E_S2).
- Actor AI **is** wired to anim indices on **E_S2** (`ANM_WAIT/WALK/ATTACK/HANG/SHOUT/…`
  state machine + frame checks). Unique E_s1 BCKs (`hook/pull/defend/attack03/change/
  dead01`) have **no callers** in `d_a_e_s1.cpp` — orphan prototype clips.
- Paths to Cut Actors:
  1. **Display-only preset** (small): same as viewer — spawn mesh+BCK cycle, no AI
     (fast; honest label “E_s1 proto — anim cycle”).
  2. **Live AI on E_s1 mesh** (large): TARGET_PC fork — load `"E_s1"`, remap every
     `getObjectRes` index, stub hang/shout paths missing from E_s1, retune collision
     for gen-2 rig. Dual APPROVED recommended.
  3. Keep status quo: Cut Actors = shipping E_S2 beast; Demo viewer = gen-2 E_s1.
- **E_dt next:** ACT_WAIT works; falls through floor → need `home`/`Acch` ground snap
  (or keep Y from gndCheck) after override — small follow-up.

### Phase C′ — Unified plan: Live AI on gen-2 `E_s1` (path 2)

*(Cursor draft 2026-07-18. User chose path 2. History: critique when free from Moblin H2.
**No impl until user pass** (+ History APPROVED preferred; user may override).)*

#### Goal

Cut Actors can spawn a **living** Shadow Beast that uses gen-2 **`E_s1.arc`** (`s1.bmd` +
proto BCKs), not shipping `E_S2`. Retail `E_S1`→`E_S2` path stays intact for normal play.

#### Facts (source + disc headers)

| | Shipping (`E_S2`) | Gen-2 (`E_s1`) |
|---|---|---|
| Arc load today | `daE_S1` always `"E_S2"` | never loaded by actor |
| BMD index | `0x22` `s2.bmd` | `0x1F` `s1.bmd` |
| Wait01 index | `0x1D` | `0x1A` |
| Shared members | — | **0** (different filenames) |
| Joint tree | `CENTER/NECK/FOOTL1…` (S2_JNT) | `WORLD_ROOT/JAW/TONGUE/TAIL…` (S1_JNT) — **not isomorphic** |
| Hang/shout/deadwake BCKs | present; AI calls them | **absent** |
| hook/pull/defend/attack03/change/dead01 | absent | present; **no AI callers** |

Hard ANM `#define`s in `d_a_e_s1.cpp` are **E_S2 indices**. Renaming the arc alone
**will crash or play wrong clips** (e.g. S2 `DAMAGED=7` vs S1 index 7 = `ATTACK03`).

#### Design

**Gate:** `#if TARGET_PC` only. Opt-in via Cut Actors preset (new entry), **not** a global
replace of all Twilight Beasts.

**Preferred shape:** keep one `daE_S1` binary; add a **res profile** selected at create:

1. Cut Actors passes a sentinel param bit / angle / dedicated flag (document exact
   encoding in Slice A) meaning `USE_E_S1_ARC`.
2. Create/`anm_init`/`useHeapInit`/`resDelete` consult profile:
   - arc name `"E_s1"` vs `"E_S2"`
   - BMD + default wait indices from `assets/GZ2E01/res/Object/E_s1.h`
   - `anm_init(resNo)` goes through `mapAnm(resNo)` → gen-2 index or fallback

**Do not** fork a second actor proc unless joint-col forces it (prefer remap first).

#### Anm map (logical → E_s1 index)

| Logical (current `#define`) | E_S2 idx | E_s1 idx | Notes |
|---|---|---|---|
| ATTACK | 5 | 5 | OK |
| ATTACK_02 | 6 | 6 | OK |
| DAMAGED | 7 | 9 | **shifted** |
| DASH_01/02 | 8/9 | 0xA/0xB | shifted |
| DEAD_02/03/04 | 0xA–0xC | 0xD–0xF | shifted |
| DEADWAIT_02/03/04 | 0xD–0xF | 0x10–0x12 | shifted |
| DEADWAKE_* | 0x10–0x12 | — | **missing → fallback DEADWAIT or DOWN** |
| DOWN | 0x13 | 0x14 | shifted |
| HANGED / HANG_* | 0x14–0x18 | — | **missing → stub action** (skip hang; go WAIT/DAMAGE) |
| SHOUT | 0x19 | — | **missing → WAIT_02** |
| SHRINK / SHRINK_DOWN / STICK | 0x1A–0x1C | 0x17–0x19 | shifted |
| WAIT_01/02 / WALK | 0x1D–0x1F | 0x1A–0x1C | shifted |

Unused E_s1 clips (hook/pull/defend/…) stay unused unless a later slice wires them.

#### Joint / collision

`jc_data[]` joint indices are **S2-centric**. Slice B: rebuild `jc_data` (or parallel
table) from `S1_JNT_*` for gen-2 profile; verify `mJntCol.init(..., 9)` still valid
joint count. Fail create loudly if init fails.

#### Cut Actors UI

- Keep existing **E_S1 — shipping (E_S2)** preset.
- Add **E_s1 — gen-2 proto (live AI)** preset with sentinel that selects profile.
- Notes: hang/wolf-bite paths degraded; proto mesh.

#### Execute slices

| Slice | Work | Acceptance |
|---|---|---|
| **A** | Res profile + arc string + `mapAnm` + heap BMD/wait indices; shipping path byte-identical when flag off | Field spawn shipping E_S1 unchanged; gen-2 flag loads `E_s1`/`s1.bmd` without crash |
| **B** | S1 `jc_data` + create/heap OK | `mJntCol.init` success; no AV on spawn |
| **C** | Stub hang/shout/deadwake call sites when profile=gen-2 (fallback anims / early-out actions) | Wolf-bite / hang paths don’t AV; beast returns to wait/walk |
| **D** | Cut Actors second preset + labels; despawn cycle | Spawn→fight-ish→despawn OK; Demo viewer E_s1 cycle still works |
| **E** (parallel small) | E_dt floor snap after ACT_WAIT | Toad rests on ground |

#### Risks

- Joint mismatch → silent wrong hitboxes or AV in `jntCol`.
- Frame-number checks in AI tuned to S2 clip lengths → odd combat timing (acceptable v1).
- Audio names may assume S2 — log-only failures OK for v1.
- Must not change Twilight Palace retail beasts (gate + default profile = E_S2).

#### Out of scope (this plan)

- Wiring hook/pull/defend into new actions.
- Boko-flame bridge.
- Changing Demo viewer (already cycles BCKs).

#### Sign-off

| Role | Sign-off | Notes | When |
|------|----------|-------|------|
| Cursor | APPROVED | Plan author | 2026-07-18 |
| History | — | Critique when Moblin H2 yields; joint/audio risks | |
| User | — | **Pass required before Phase D impl** | |

**Implementation gate:** user **pass** required; History APPROVED preferred before Slice A.

**History — MOBLIN HUNT CONCLUDED (2026-07-17): defensible NULL result.**

Method per H-4/C-2: fingerprint built from WW `Mo2.arc` (56 distinctive animation
names, 67 distinctive model strings after subtracting the Bokoblin `Bk`/`Boko`
contrast set), then all 1,397 TP Object arcs swept for ≥2-element hits.

| Evidence lane | Result |
|---|---|
| Mo2/Bk/Boko arcs on TP disc | **absent** (name + SHA sweeps, prior passes) |
| Mo2 animation names in TP | **zero occurrences** in any arc |
| Distinctive Mo2 model strings in TP | zero after noise filtering — all raw "hits" were padding (`ZZZZ…`), section aligners (`aligSHP1`), or the generic Japanese joint vocabulary (`hara_j/kubi_j/ude*`) shared by every humanoid rig |
| Second orc actor in code | absent (one `E_OC` proc; `E_OC2` = mesh variant, folk label demoted) |
| Symbol maps | complete set; no moblin-adjacent module |
| External record | 2004 trailer axe enemy documented (TCRF); possibly became King Bulblin — a design lineage, not a data remnant |

**Conclusion: no Wind Waker Moblin remnant exists on retail TP (GZ2E01).** The
trailer-era axe enemy was fully purged before mastering; the only "Moblin-like"
artifacts are E_OC2's alternate bokoblin mesh and the external trailer footage.
Unswept residual lane (low prior, noted for completeness): a full orphaned-audio-id
audit of `Z2SE_EN_*` families — an audio scar alone cannot constitute a model
remnant, so it cannot change this conclusion, only annotate it. Brief Goal 4 is
answered. (Null results are findings — per the signed plan.)

**History — playtest №4 response (2026-07-17): horse correction + Moblin hunt REOPENED, first traces landed.**

1. **CORRECTION (History error, user visual falsifies my label): `Sample.arc/file2.bmd` is NOT the
   GDC 2005 beast — it is an EARLY EPONA.** User saw an untextured horse with saddle in-game.
   Rig re-census against shipped Epona (`Horse.arc/hs.bmd`) settles it: **identical joint list
   name-for-name** — `center/backbone1-2/F_&B_leg chains/neck1-2/hair_L,R,F/head/ear_L,R/mouth1-2/`
   **`kura1` (鞍 saddle), `belt_L/R` + `abumi_L/R` (鐙 stirrups)**`/waist/tail1-3`. I had quoted only
   the leg/neck subset and pattern-matched "quadruped = beast" — the saddle joints were in my own
   census output. file2.bmd = 59,648 B vs shipped hs.bmd 131,264 B ⇒ **lower-poly prototype of the
   ridable horse**, sitting in the engine *sample* archive — consistent with WW2's founding pitch
   (Takizawa: horseback riding was the reason the project existed). Sample.arc is plausibly the
   original horseback-prototype sample. **TCRF's "early Shadow Beast inside Sample.arc" claim is
   the falsified one.** The data-level beta Shadow Beast remains **E_s1 (gen-2 prototype)** — user
   confirms this matches what they had seen. → Cursor: relabel viewer entry
   `Sample/file2.bmd` → `early Epona prototype (rig-match vs Horse.arc) (probable)`; alias table
   row updated below (folk-label demotion, unilateral per H-5).
2. **Moblin hunt REOPENED (user challenge accepted — and it is methodologically fair).** The NULL
   above only killed hypothesis H1 (*WW-data remnant*: Mo2 fingerprint). It never tested H2
   (*ground-up TP Moblin* — zero Mo2 fingerprint by construction) or non-model trace classes.
   New sweeps this session:
   - **H-4b Orphan placement sweep (NEW TOOL: `tools/demo_cut_content/dzr_orphans.py`)** — proper
     layer-aware DZR/DZS chunk parser (ACTR/ACT0-e, SCOB/SCO0-e, TGOB/TGSC, TRES, Door families)
     across all 645 room/stage members; every placed name diffed against `l_objectName`
     (916 entries, d_stage.cpp). Result: ACTR-family parsing is byte-clean and yields **exactly one
     ghost placement name on the whole disc: `_partic`** — ×3 (F_SP122/R08 layers d+e,
     F_SP123/R13 base layer), `params=FFFFFFFF`, rot 0, enemyNo unset; neighbors include `Coach`
     (Telma's wagon) and `GWolf` ⇒ authored along the **wagon-escort route**, then the actor was
     removed from the roster. `_partic` matches no symbol in TP or WW maps (only generic
     `d_particle.o` engine symbols) — a designer-placed particle-emitter helper deleted before
     ship. Not Moblin, but proof the method finds real deletions. (Door/TRES raw hits were
     misparse noise — TP `Door` stride is 36, not 48; attributed and discarded.)
   - **H-4c Symbol-content grep (all TP .map contents, not just module names):**
     `moblin|moriblin|mobrin|daMo2|boko` → total ONE hit, and it is a find:
     **`getBokoFlamePos__9daPy_py_cFP4cXyz`** — the SAME mangled base-player virtual exists in WW
     maps (WW Link carries flaming **Boko sticks** — Bokoblin/Moblin weapon culture). TP kept the
     interface (stub returns FALSE, `d_a_player.h:778`, no daAlink override) **and shipped a live
     caller**: `d_a_obj_brg.cpp:978` (rope bridge; also inherits WW wind via
     `dKyw_get_AllWind_vec`). Decoded mechanic: flame within 50 units of a plank's rope anchor →
     30-frame burn timer + fire emitter → rope bit cleared per side (`field_0x708`), plank dangles
     (one side) or drops collision `y -= 10000` (both). **A complete per-plank burnable rope
     bridge shipped dormant in TP**, placed at retail `D_MN05/R01` (Forest Temple) and
     `F_SP104` — unreachable only because the Boko-flame query is hard-FALSE.
   - **Restoration candidate (proposal only — needs dual sign-off + user OK, History does not
     implement):** override `getBokoFlamePos` on daAlink to return the LANTERN flame
     (`mKandelaarFlamePos`, d_a_alink.h:3410/4570) while the lantern is lit ⇒ wakes the burnable
     bridge mechanic with zero new assets. Suggested gate: `DEBUG || TARGET_PC` toggle like
     TestCube.
   - **Still-open Moblin trace lanes (H2, queued):** TP-native big-orc rig taxonomy over ENEMY
     arcs (E_oc/E_oc2 axis — the design descendant), stage-arc member sweep (Object-only gap),
     texture-name sweep, orphaned-audio annex. The NULL stands for H1 only; run doc table above
     now scoped accordingly.

**History — beta-Moblin cut-dungeon lane (2026-07-18, responding to user's trailer screenshot +
"could be in a cut dungeon like the beta fire temple"):** the hypothesis is CONFIRMED as a trace
class — TP ships the *names and links* of a ~15-stage cut dev-stage network, though not the data.
Full write-up in TPHistory **§3.8**; highlights:

- **Twilight-capable stage table** (`d_kankyo_data.cpp`, in the DOL) names 12 UNUSED stages incl.
  **`T_IMAI` "Imai Shape Check (Level 2)"** + `T_IMAI0` (Level 3) — Level 2 = fire-dungeon slot =
  best-named candidate for the trailer's torch-lit "beta fire temple" corridor — plus `T_SP05`
  "E3 2005 Warp Test", `F_SP105` "L1 Forest Field", `F_SP05` "Statue Forest", `T_TOMI`/`T_SASA1`
  (staff test stages), `TEST11`, `R_SP30` "Unknown Room".
- **Live special-case code still ships**: wolf-Link `checkStageName("R_SP30")` gated on event bit
  M_010 (`d_a_alink_wolf.inc:7893`); rain engine scripts R_SP30 room 4; env engine checks
  `T_SASA0` (15th name, code-only).
- **NEW TOOL `tools/demo_cut_content/scls_sweep.py`** — SCLS exit tables are VARIABLE-LENGTH
  (name string + 5 bytes; fixed-stride parses frame-shift — hexdump-verified). Shipped rooms
  still hold exits → `T_SP05` (**from Forest Temple D_MN05/R09**), `T_DEMO` (R_SP01/R04),
  `T_OBJ01` (F_SP121/R06), `T_TOMI` (F_SP109/R00). `T_DEMO`/`T_OBJ01` are in NO binary —
  placement-data-only, likely previously unrecorded. (`OPENING` = code sentinel, d_stage.cpp:44;
  `NPC_GND` hit = junk-row tier, that's the Ganondorf NPC actor token.)
- **Room-number gap scan**: 13 stages have holes; Forest Temple D_MN05 missing 10 rooms,
  F_SP122 missing 7 — heavy late cuts inside shipped stages corroborated.
- **Moblin verdict update**: if the trailer axe-wielder existed as data, its home was this
  dev-stage network — purged with it. Consistent with the H1 fingerprint NULL; the surviving
  traces are the network's names/exits/scripting, now documented. Remaining H2 lanes unchanged
  (big-orc rig taxonomy, stage-arc member sweep, texture sweep, audio annex).

**History — playtest №5 response (2026-07-18): E_GS CONFIRMED on camera; E_dt no-show ROOT-CAUSED.**

1. **E_GS lane CLOSED**: user screenshot shows the ghost soldier in Wolf Sense — translucent,
   glowing eyes, Hylian-crest tabard, reaching animation. Wolf-Sense opacity gating was the whole
   story (matches `gho_gt` resources + the opacity=1 probe). Alias promotion candidate:
   `E_GS — ghost soldier (Wolf-Sense-gated)? (probable→confirmed)` — needs Cursor countersign
   per H-5 to hit confirmed tier.
2. **Early Epona now partially textured** in the viewer (real head/leg/bridle textures + garbage
   patches + green untextured materials) — user-confirmed it reads as a horse. Whatever loader
   change Cursor made improved TEX1 handling; remaining garbage = stripped/partial texture data
   in the 2002-era BMD, likely not fully recoverable.
3. **E_dt "does not spawn" FALSIFIED — it spawns, lives, and hides (source-diagnosed):** log
   shows `phase=COMPLEATE`; `daE_DT_c::create()` passes all gates (switch param&0xff clear, not
   `isStageMiddleBoss`, heap 0xA840 OK, resLoads E_DT+Dalways+E_OT). Then `executeOpening()`
   case 1/2 **hard-positions the toad at ABSOLUTE world (0, 4000, 0)**, inverted
   (`shape_angle.x=-0x4000`), spawns **20 E_OT tadpoles**, and waits on `pointInSight`/shutter.
   The skip path (`cDmr_SkipInfo`) likewise hard-sets `(0, 0, -500)` + starts DEKUTOAD BGM. In
   the Lakebed arena those are room-center coordinates; anywhere else = a point in the sky at
   map origin. **Every Cut Actors spawn worked — it's dangling at world origin.**
   **Proposal (Cursor, small, needs sign-off):** post-create override in the spawner for E_dt —
   set `current.pos` near the player and `setActionMode(ACT_WAIT)` (post-drop state) so it lands
   fighting; note it needs floor room for its 1400-unit AABB and will still spawn tadpoles.
4. **E_s1 animations (user request):** the gen-2 beast arc carries 24 BCKs
   (`S1_HOOK/PULL/DEFEND/SHRINK/STICK/…`). Cursor: wire an anim-cycle control into the leftover
   viewer for out-of-Demo entries (same pattern as existing viewer anim support) so the beta
   beast moves. Needs no new policy — display-side only.

### Phase E — Drive results (if in scope)

| Dive / label | Result | Notes |
|--------------|--------|-------|
| | | |

---

## Appendix — History evidence tables (Slice 0, posted 2026-07-17)

### A1. E_ms create-path precondition table (source-verified, d_a_e_ms.cpp)

| Param bits | Field | Create-time effect | Line |
|---|---|---|---|
| 0–7 | `field_0x5b6` | behavior flag (runtime; `==1` path at L102) — semantics TBD | 1022 |
| 8–15 | `field_0x5b7` | behavior flag (runtime) — semantics TBD | 1023 |
| 16–23 | (local) | **KILL-switch**: `!=0xff && isSwitch(bits,room)` → `cPhs_ERROR` (dead-persistence) | 1016–1019 |
| 24–31 | `field_0x5b8` | **ENABLE-switch**: `!=0xff && !isSwitch` → spawns DORMANT (`mAction=10`, `0xba5=1`); polled at runtime L588 | 1054–1058 |

Order of checks: `resLoad("E_MS")` phase → kill-switch → `entrySolidHeap 0x1860` → init → enable-switch.
**Predictions for Cursor's slices:** params=0 in a room with switch 0 SET → ERROR at kill check; switch 0 UNSET → *successful create but dormant/invisible* (the log must not read either as mystery). `0x00FF0000` → create succeeds, still dormant unless switch 0 set. `0xFFFF0000` → fully active immediately. Res indices 20/16 verified against retail index (E_ms.h); runtime resLoad line still wanted in the log per C-1.

### A2. E_gs preconditions (partial — source read)

Reads bits 0–7 (`arg0`) + 8–15 (`arg1`) (L196–197); `OS_REPORT`s its params at create (L195 — free logging already!); **no create-time switch-kill observed**; `entrySolidHeap 0x4B000` (L200) directly after resLoad. Priors: placement/environment first, heap second, params last.

### A3. DZR placement sweep (byte-scan of `name `-padded ACTR names, all 79 stages; layer-blind — counts are pattern hits, not confirmed-active-layer placements)

| Actor | Hits | Stages |
|---|---|---|
| `E_ms` | **72** | D_SB01 (×47 across R01/R10/R15/R21/R46), D_MN10 (Snowpeak Ruins ×4), D_SB02/05/06/10, R_SP107, R_SP116 |
| `E_gs` | **22** | D_MN09 only (R03 ×8, R09 ×8, R12 ×6) |
| `E_dt` | **1** | D_MN01B R51 |
| `E_oc` (control) | 126 | field/dungeon spread as expected — method validated |

### A4. RECLASSIFICATION (major — corrects run context snapshot + TPHistory)

1. **E_ms is NOT a cut actor.** 72 placements across 8 retail stages. Combined with the
   skeleton (quadruped, 4-segment tail, mouth), anims (swim/bite/gnaw/kyorokyoro), audio
   (wet footsteps, swim, cries), and the placement biome pattern (sewer-type R_SP rooms,
   Snowpeak Ruins, cave floors): **E_ms is with high confidence the shipped RAT family**
   ("ms" = mouse — folk-tier naming hypothesis until playtest confirm). This also explains
   why no TCRF unused-page ever listed it. The Cut Actors preset label "cut water beast
   (TP-original)" must be demoted; TPHistory §3.3's "cut" classification is corrected
   (its skeleton/audio evidence stands; the cut claim falls — evidence-first working as
   intended, on our own prior conclusion).
2. **E_dt is with high confidence the SHIPPED Deku Toad midboss** ("DT" = Deku Toad):
   exactly one placement in a boss-arena-class stage room, 2,592-line actor, `OP_DEMO*`
   intro-cutscene anims = its midboss entrance. E_ot (**otama** = tadpole) = its spawned
   minions, not "the shipped toad". TPHistory §3.4 corrected accordingly.
3. **E_gs is placed but single-stage** (D_MN09 ×22, three rooms) — a shipped, location-
   locked enemy. Spawn-diagnosis control room per C-1 Slice 3: a D_MN09 placement room.
   Its arbitrary-field spawn failure is now EXPECTED to be environmental first.
4. **Consequence for the run:** the "cut-actor spawn diagnosis" lane is really a
   "spawn shipped enemies out-of-habitat" lane for E_ms/E_gs. The diagnosis slices stay
   valid and valuable (the switch/dormancy semantics explain the observed failures
   precisely), but playtest confirmation is now trivial too: visit the retail rooms.

### A5. Alias appendix — day-one demotions (display-only; Cursor applies in Slice 1)

| Current label | Action | Basis |
|---|---|---|
| `E_OC2 — "Moblin mesh"` | → folk: `E_OC2 (alt mesh)? (unverified)` | no evidence bundle; prior research says bokoblin skeleton + nata SFX |
| `E_ms — "cut water beast (TP-original)"` | → evidence alias: `E_ms — rat family (shipped)? (probable)` | A3/A4 №1 — promote to confirmed after playtest sighting in a retail room |
| `E_dt — "cut frog (large actor)"` | → evidence alias: `E_dt — Deku Toad (shipped midboss)? (probable)` | A4 №2 |
| `E_IS — "Armos/Iron Titan (unreleased)"` | verify next | placement sweep pending |
| Demo09 `ym` as "Link" | → folk `(unverified)` | rig table pending (H-3) |
| `Sample/file2.bmd — "GDC 2005 Twilight Beast"` | → **demoted** (user visual + rig re-census): `early Epona prototype (rig-match vs Horse.arc)? (probable)` | joint list identical to `Horse.arc/hs.bmd` incl. `kura1` saddle + `abumi_L/R` stirrups; TCRF Sample-beast claim falsified; Cursor applies relabel |

### A6. Mo2 fingerprint (H-4.1) — status

Kickoff next History heartbeat: joint/anim/texture name extraction from WW `Mo2.arc` +
`Bk.arc`/`Boko.arc` contrast set. Hit criterion per C-2: ≥2 independent elements.

## Paste openers

**Cursor (implementer — this chat):**

> Interconnected Run: **Cut Actors / Demo Restore — Cursor ↔ History**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, this run doc, `docs/state/run-control.md`, and `docs/state/cut-actors-demo-restore.md`. Obey run-control (no work unless `running`). Follow deliberation phases — do not implement until both sign-offs are APPROVED. Drive only if this run’s brief says so.

**History (archaeology + critique):**

> Interconnected Run: **Cut Actors / Demo Restore — Cursor ↔ History**. Read `docs/Interconnected Chats/INTERCONNECTED-RUNS.md`, `docs/Interconnected Chats/Cut-Actors-Demo-Restore-Cursor-History.md`, `docs/state/run-control.md`, and `docs/state/cut-actors-demo-restore.md`. Your job is disc archaeology, naming/alias policy, Moblin remnant evidence, and critical review. Critique seriously; sign off only when the unified plan is sound. No source edits unless the brief says otherwise. Obey pause/stop. Write your **Phase A — History proposed plan** into the run doc (do not copy Cursor’s plan). Then wait for ferry to Phase B.
