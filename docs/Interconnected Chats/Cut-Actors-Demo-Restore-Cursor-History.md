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
- **User process (2026-07-18):** Cursor must **always** put the **Paste for History** block in the **run doc** (verbatim ferry text under a clear heading). Chat may mirror it; the doc is the source History reads.

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

- **2026-07-18 Phase C′:** user pass after commit `728b5fd71e`. Cursor implementing
  gen-2 live AI: Cut Actors preset `E_s1 — gen-2 proto` (`angle.x=0x0E51`) →
  `mUseEs1Arc`, `mapAnm`, S1 `jc_data`, hang/shout stubs. Shipping E_S1 preset unchanged.
  E_dt COMPLEATE: re-gndCheck + zero vertical speed (no private `mAcch` from spawner).

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
| History | **POSTED** (follow-up) | Moblin leaves + E_s1 scope; EN_NS wire proposal → Cursor corrected (already live) | 2026-07-18 |
| User | **PASS** | “Let's try it” after commit — Slice A–E started | 2026-07-18 |

**Implementation gate:** user **pass** received 2026-07-18. History critique still welcome as follow-up.

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

**History — MOBLIN PATH: ALL LEAVES TURNED (2026-07-18). Full synthesis in TPHistory §3.9.**

Ten lanes run to completion (user directive: "turn over every leaf"). Headlines:

1. **E_OC2 folk label DEAD — it SHIPS**: 39/126 retail E_oc placements select OC2 via placement
   rot.x byte 1 (≠0/FF → `resLoad("E_OC2")`, d_a_e_oc.cpp:2998); homes = ToT R11/13/14, Cave of
   Ordeals, Hyrule Field. Same body as oc.bmd (retexture) + own 2×-size weapon. Debug name
   オーク "Orc". → Alias demotion: `E_OC2 — shipped orc retexture w/ heavy cleaver (confirmed)`.
2. **Orphan-arc sweep**: exactly ONE enemy-prefixed arc no code ever names — `E_wpa` =
   `ef_warphole` unused warp-hole EFFECT (viewer candidate, misfiled E_ prefix). All other
   E_/B_ arcs accounted.
3. **All axes accounted** (member-name sweep, 1,397 arcs): Dynalfos `mf_axe`, King Bulblin
   `rb_ono` (斧), OC2 cleaver — no orphaned axe model anywhere.
4. **Cross-master diff (9 discs)**: only the known rev0 E_base leak. Stage arcs: doors only.
5. **FIVE orphaned enemy-audio families discovered** (no actor/arc/code): **EN_NS = beta Shadow
   Beast audio, high confidence** (22 ids; FALLTREE/arm-swing/L-R hands+feet/HANGED/
   **V_DEMO04_HIPPARU** = pulling in the drag-Link-into-twilight cutscene; ≥8 verb matches vs
   E_s1 anims — the S1/S2 line had audio before the E_md rewrite); EN_GF (eater-pod —
   Like-Like profile); EN_ZZ (snorter); EN_GBA (egg-layer w/ hatch); EN_HG (Demo28/Bullbo-arc
   heavy). Candidate viewer/annex labels — each needs Cursor countersign for display.
6. **Moblin verdict (final)**: purged before all nine masters AND before audio mastering froze
   (five other cut enemies left audio scars; the Moblin left none) — its home was the §3.8
   dev-stage network. Retail descendants are design-lineage only (King Bulblin, OC2). No
   disc-side restoration path exists; beta-Moblin restoration = mod-import class (WW Mo2 port
   through custom-assets or recreation) — same class as the parked stubs, which aligns with the
   user's declared next phase (unpark stubs → external-asset load path).

**History — E_s1 scope refinement (2026-07-18, answering user's "does it still have logic tied
to each of its animations?"):**

- **The AI is 100% alive — `d_a_e_s1.cpp` (2,538 lines) IS the retail Shadow Beast** (proc E_S1
  placed across the twilight overworld; loads the E_S2 retail model; `E_md` = Midna's family,
  not the messenger). Live behaviors: roof-ambush (`e_s1_roof`), patrol paths, fight/run, flinch
  (`bibiri`), damage/down, and the signature pack mechanic (`s_pack_*`/`all_fail`/`all_wakeup` —
  last-beast revive shout). Cursor's 0x0E51 variant already runs the gen-2 model INSIDE this
  live AI with an anim index remap — "fit into Cut Actors" is substantially done.
- **Anim coverage split (the real remaining scope):**
  - Retail logic ↔ gen-2 arc BOTH have: attack/attack02, damaged, dash01/02, dead/deadwait
    families, down, shrink/shrink_down, stick, wait01/02, walk — all drive correctly.
  - Retail logic WANTS but gen-2 arc LACKS: `HANGED/HANG_DAMAGE/HANG_BRUSH/BRUSH2/HANG_WAIT`,
    `SHOUT`, `DEADWAKE_02-04` → Cursor's fallback stubs are the right call; revive-shout will
    play a fallback pose.
  - **Gen-2 EXCLUSIVES with NO retail driver: `HOOK`, `PULL`, `DEFEND`, `CHANGE`
    (+ `ATTACK03`)** — vestigial gen-2 behaviors. Design reading (cross-evidence with the
    orphaned EN_NS audio: `HIPPARU` pull + `HANGED/HANGEDBRUSH` ids): the gen-2 beast was a
    GRABBER — it hooked/latched onto (wolf-)Link and was shaken off ("brush"), and dragged prey
    (Demo04 pull). Reviving that moveset = NEW action states (small, but new gameplay — its own
    sign-off if wanted). Without it, gen-2-in-Cut-Actors is complete but plays like a
    reskinned retail beast.
- Standing proposal unchanged: wire EN_NS ids as the gen-2 voice (V_ATTACK/DAMAGE/DEATH/SEARCH/
  WAIT/FN_L/R; HANGED ids if the grab states ever land). Needs Cursor countersign.

**Cursor — countersign / absorb History 2026-07-18 (Moblin leaves + E_s1 scope):**

| History claim | Cursor stance |
|---|---|
| Moblin final NULL / mod-import class | **ACCEPT** — closes hunt for this run |
| E_OC2 ships (rot.x selector) | **ACCEPT** — soft-label demotion in Cut Actors UI |
| `E_wpa` unused warphole effect | **ACCEPT** as viewer candidate (backlog; not Slice now) |
| EN_NS = orphaned beta beast audio → wire for gen-2 | **CORRECTED — already live.** `d_a_e_s1.cpp` already calls ~all `Z2SE_EN_NS_*` (attack/damage/death/search/wait/fn/hand/hanged/shout/…) and `setEnemyName("E_s1")`. Gen-2 `0x0E51` profile inherits that bank unchanged. TPHistory §3.9 “no source reference” is false for the family; sole likely orphaned id in-family is `Z2SE_EN_NS_V_DEMO04_HIPPARU` (no `.cpp` callers) — interesting for a future PULL action, not a rewire |
| Grabber exclusives HOOK/PULL/DEFEND/CHANGE/ATTACK03 | **ACCEPT reading** — out of C′; needs own mini-plan + user pass if we revive gameplay |
| Hang/shout stubs | History affirms — keep |

**No EN_NS rewire work.** Playtest still the gate for C′ acceptance. Optional next (user): E_OC2 label fix (trivial), E_wpa viewer row, or grabber mini-plan.

#### Paste for History (2026-07-18 — Cursor countersign)

```
Cursor countersigned your 2026-07-18 Moblin wrap + E_s1 scope (run doc).

ACCEPT: Moblin closed/mod-import; E_OC2 ships; E_wpa viewer candidate; grabber exclusives = optional later plan; hang/shout stubs stay.

CORRECT EN_NS: not orphaned — d_a_e_s1.cpp already calls the bank; gen-2 0x0E51 inherits it. No rewire. Sole unused id looks like Z2SE_EN_NS_V_DEMO04_HIPPARU (no cpp callers) — save for PULL if we ever do grabber.

E_OC2 Cut Actors soft-label demoted. Waiting user playtest of gen-2 preset.
```

**History — audio retraction + THE `mo2` FOSSIL + unpark plan (2026-07-18):**

1. **RETRACTION (extending Cursor's EN_NS catch to all five):** the "five cut-enemy audio
   families" claim is dead — my token-matching method was flawed. Code-usage grep: EN_NS=`e_s1`
   (Shadow Beast), EN_GF=`e_gb` (**Deku Like**), EN_ZZ=`b_zant_mobile`, EN_GBA=`e_gm`
   (Gohma egg/hatch), EN_HG=`b_gnd` (beast Ganon). **Zero orphaned audio families exist** —
   strengthens the Moblin timing argument (no scar at any layer). TPHistory §3.9 corrected.
2. **THE MOBLIN TRACE EXISTS — user vindicated.** `res/ActorDat/ActorDat.bin` (1,984 B, outside
   all prior sweep scopes): **WW's enemy item-drop database, BYTE-IDENTICAL to the WW disc copy,
   shipped on retail TP, read by no TP code — and its enemy roster includes the literal string
   `mo2` (Moblin)** alongside nezumi/keeth/chuchus/Stal/amos/Fmaster/Rdead/wiz/Oship. The only
   Moblin bytes on any TP master: a fossil of WW's drop table. Also swept to closure: all ~130
   STB cutscene scripts (no cut-stage/enemy tokens; Demo90/98 unremarkable).
3. **Unpark plan (proposal for Phase C″ deliberation — needs Cursor plan + dual sign-off):**
   - **Tier 1 — TestCube** (assets ON disc): activate the deferred H-2/C-1 mini-plan —
     `DEBUG || TARGET_PC` method gate + non-NULL profile; smallest, first.
   - **Tier 1b — E_wpa warp-hole effect** (assets on disc, no proc): leftover-viewer entry
     (display-only) — trivial win.
   - **Tier 2 — Makar (npc_mk) + Medli (npc_p2)**: models DO exist — on the WW extraction
     (D:\XXXXXXX\Ex WW). Path: WW arc → custom-assets import (Beta-Link Kmdl remount precedent)
     → display spawn (viewer category or minimal idle-BCK NPC actor). Their stub rels are
     unrestorable (procSize=1, code gone; symbol maps = names only) — new minimal actor, not
     resurrection. Optional Tier 2+: dialogue/behavior later via DuskScript (code-mods lane).
   - **Tier 3 — obj_damCps** (222 real lines survive): evaluate compile-in under TARGET_PC.
   - **Tier 4 — tag_* minigame family** (arena/escape/instruction/…): logic-only actors, no
     models needed, but code gutted → best served as DuskScript script-actor prototypes
     (dovetails the code-mods END GOAL), guided by surviving symbol-map function names.
   - **Parked indefinitely — KDK/HENNA0** (no models on any disc or extraction we hold).
   Suggested order: 1 → 1b → 2 (Makar first) → 3 → 4.

**History — playtest №6 diagnosis (2026-07-18): gen-2 freeze ROOT-CAUSED from log + source —
it's the E_ms dormancy lesson again: `params=00000000`.**

Log `dusklight-20260718-005235` lines 2060-2141: first gen-2 spawn hit **phase=ERROR**, retry
COMPLEATE — both with `params=00000000`. daE_S1 create-param table (source-verified,
d_a_e_s1.cpp:2376-2431):

| Bits | Field | With 0x00 | With 0xFF (sentinel) |
|---|---|---|---|
| 0-3 | roof-spawn | 0 = ground ✓ | 0xF→0 = ground |
| 4-7 | pack group | group 0 | 0xF→0xFF = solo |
| 8-15 | search range ×100 | **0 = NEVER AGGROS** | 0xFF→20 = 2000 units |
| 16-23 | path number | **path 0 — ERROR if room lacks it; hijacks it if present** | 0xFF = no path |
| 24-31 | kill-switch | switch 0 (room-dependent) | 0xFF = none |

Explains everything observed: ERROR-then-work flakiness (room path 0 existence), "not
interactible" (searchRange 0 — aggro impossible), "half an animation then froze" (ACT_WAIT →
`path_check2` → ACT_PATH onto a foreign room path → patrol stall mid-morf-blend).
**Prescription (Cursor, one-liner tier): both E_S1 presets spawn with `params=0xFFFFFFFF`** —
same fix class as E_ms. Anim-index shim is NOT the fault (BMD 0x1F / WAIT01 0x1A verified
against dRes_INDEX_E_S1_* table; viewer cycling works).

**Scale (user: gen-2 must match final beast size — trailers confirm):** draw path multiplies
`l_HIO.mBaseSize * scale.x` (d_a_e_s1.cpp:2148). Suggestion: on `mUseEs1Arc`, apply a gen-2
size factor there (or via `fopAcM_SetScale` post-create in the spawner) exposed as a live-tune
reg (WREG_F) — user calibrates by spawning retail E_S1 preset beside gen-2 and matching
silhouettes; then freeze the constant. No evidence on the exact authored-unit ratio; empirical
tune is the honest method.

**Cursor — absorb playtest №6 + ship fix (2026-07-18):**

- **ACCEPT** root cause (`params=0` → searchRange 0 + path 0). Anim shim not at fault.
- **Shipped:** both Cut Actors E_S1 presets now `params=0xFFFFFFFF`.
- **Scale:** gen-2 create sets `scale = 1.25 + WREG_F(29)` (draw only; Cc stays on `mBaseSize`). Calibrate vs shipping preset, then we freeze the constant.
- **Unpark C″:** noted — Cursor counterplan after gen-2 playtest confirms aggro/fight; do not start Tier 1 TestCube until that pass + dual sign-off on C″.

#### Paste for History (2026-07-18 — playtest №6 fix)

```
Cursor absorbed playtest №6 diagnosis — ACCEPT root cause (params=0).

Shipped: both E_S1 Cut Actors presets → params=0xFFFFFFFF.
Gen-2 scale: create sets scale=1.25+WREG_F(29) (draw via mBaseSize*scale.x; hitboxes stay HIO).
Waiting user re-playtest (aggro/fight + size vs shipping side-by-side).

Unpark C″: acknowledged; Cursor counterplan after gen-2 confirms — no TestCube yet.
Audio retraction + mo2 fossil: ACCEPT (ActorDat.bin WW drop table). EN_NS correction stands.
```

**History — playtest №7 analysis + PROPOSED PLANS (2026-07-18): wolf-latch failure, negative
HP, anim breadth, and the parked-actor C″ plan.**

**№7 findings (great result overall — params fix + 1.25 scale confirmed working):**

- **The 4 anims seen (follow / hit / attack / death) are CORRECT for a solo beast** — not a
  bug. WAIT/WALK/DASH + DAMAGED + ATTACK + DEAD is the complete solo repertoire. The rest
  need context: BIBIRI (scare) triggers on readied attacks in reaction range; DOWN + SHOUT +
  fail/wakeup are PACK mechanics (≥2 beasts, shared group id); HOOK/PULL/DEFEND/CHANGE are
  gen-2 exclusives with no logic (grabber mini-plan).
- **Wolf lunge: no damage / no reaction — mechanism identified (source):** `damage_check`
  (d_a_e_s1.cpp:490-503): WOLF_ATTACK hits that aren't bite cut-types route to
  `player->onWolfEnemyHangBite()` → on FALSE the function does a bare `return` — no damage
  processing, no state change, no death check. `onWolfEnemyBiteAll`
  (d_a_alink_wolf.inc:7354) returns FALSE unless (a) `mCutType == CUT_TYPE_WOLF_JUMP`
  exactly, AND (b) the latch keep `field_0x281c` is EMPTY. Two prime suspects, both
  fork-specific: **(1) ALBW wolf-arts combat mods may present a different cutType for the
  modded lunge**; **(2) STALE LATCH KEEP — despawning a latched enemy via Cut Actors never
  runs `resetWolfEnemyBiteAll` → every later latch on ANY enemy fails silently.** (2) is new
  territory created by our own tooling.
- **Negative HP:** with the early-return, the actor never evaluates `health <= 0` (that
  happens only inside the processed-damage branch, line 573) — yet health still fell ⇒ a
  fork-side hit hook applies damage outside the actor's handler. First bite → branch runs →
  death. Negative HP is an actor-blind decrement, not an engine overflow.

**PLAN W — wolf-latch + HP integrity (Cursor slice; History-proposed, needs countersign):**
1. **W1 instrument first** (one build): log on the early-return path — `mCutType`,
   `field_0x281c` occupancy, `health` — and grep/flag the fork-side site that decrements
   `health` without the actor's damage branch (suspect: ALBW damage/multiplier hook).
2. **W2 fix per log evidence:** (a) stale keep → call `resetWolfEnemyBiteAll()` from the
   Cut Actors despawn path (and on enemy delete while latched — general robustness win);
   (b) cutType divergence → under TARGET_PC accept the modded lunge cutType(s) as
   latch-eligible in `onWolfEnemyBiteAll` (or normalize the art-lunge to WOLF_JUMP);
   (c) **HP guard (do regardless):** in daE_S1 execute, `health <= 0 && mAction not in
   {FAIL*, WOLFBITE}` → route to fail path; clamp health floor at 0. Actor-local.
3. **W3 gen-2 hang anchor:** once latch fires, Link hangs from a chest joint matrix —
   verify the gen-2 rig's anchor joint index so the hang lands on the chest.
   Acceptance: lunge latches on both presets, mash-bite works, HP floor 0, retail beast
   unaffected, despawn-while-latched leaves player clean.

**PLAN P — gen-2 completeness (small, after W):** pack preset ("gen-2 ×3", shared group in
params bits 4-7) to unlock DOWN/SHOUT/fail/wakeup; optional portal-entrance preset via
`ACT_WARP_APPEAR` (=20); grabber states (HOOK/PULL/DEFEND/CHANGE) stay a separate
sign-off item.

**PLAN C″ — parked actors (formalizing the earlier tier proposal; needs Cursor counterplan +
dual sign-off before any code):**
| Tier | Actor(s) | Assets | Path | Acceptance |
|---|---|---|---|---|
| 1 | **TestCube** | on disc (K_cube00/01, K_cyli00) | `DEBUG \|\| TARGET_PC` method gate + non-NULL profile (deferred H-2/C-1 mini-plan) | spawns + draws + collides on RelWithDebInfo; no crash on non-PC |
| 1b | **E_wpa warp-hole** | on disc | leftover-viewer entry (display-only) | visible in viewer |
| 2 | **WW crew (Makar npc_mk, Medli npc_p2) — PHASE 1: TP BYTES ONLY (user directive 2026-07-18).** Any WW-sourced restoration is RELEGATED to a future SEPARATE mod in its own folder — not in the game. | TP disc only | **TP-only inventory (History, verified):** spawn table still carries them first-class — `Mk` + **THREE Medli subtypes `P2a`/`P2b`/`P2c` (args 0x00-0x02)** — and both sit in the debug spawn roster (`NPC_MK`/`NPC_P2`, f_pc_debug_sv.cpp:622ff). NO models on any TP master (SHA-proven), NO placements (the R_SP116 "Mk" byte-hit = false positive, mid-entry bytes), NO msg-bank text (decompressed Msgus sweep: 0 hits EN+JP), maps = 8-byte return-0 shells (no behavior vocabulary — "full symbol maps" memory claim corrected). Roster fix: **npc_kdk is NOT WW crew** (no WW counterpart) — TP-original cut NPC with **ONE real retail ghost placement: Castle Sewers R_SP107/R03 layer-0, params 000FAFFF, pos (19676, 4500, 2633)**; its map is 14 lines, zero named symbols. **Phase-1 deliverables:** (a) **safe-stub hardening** — make stub procs no-op like retail (create → cPhs_ERROR + log) so table names are placeable/spawnable without the procSize=1 crash; (b) Cut Actors parked entries become **evidence panels** ("TP knows: table name(s), subtypes, debug roster, placement if any"); (c) **Kdk ghost-placement visit (USER-REQUESTED)**: teleport/marker to `R_SP107` (Castle Sewers stage) room 3 — **Zelda's tower-top room** — pos **(19676, 4500, 2633)**, which is 660 units from `zdoor` (Zelda's chamber door) at the same floor height; suggest a Cut Actors "Visit Kdk's spot" button (warp to stage/room/pos) + a small in-world marker (editor gizmo or viewer beacon) at the exact coordinates; (d) dossier lands in TPHistory (§3.8b, done). Kdk facts for the panel: rel = 276 B, NO methods (NULL-profile = TestCube crash class — safe-stub hardening covers it), spawn-table `Kdk`, debug roster `NPC_KDK`, params 000FAFFF undecodable, identity unknown (folk: tower guard / jailer / scene escort). | stubs safe on RelWithDebInfo; evidence panels accurate; Kdk spot visitable; zero WW bytes in game |
| 3 | **obj_damCps** | 222 real lines survive | TARGET_PC compile-in eval | builds + parks safely if create fails |
| 4 | **tag_* minigame family** | logic-only, no models | DuskScript script-actor prototypes (code-mods lane), symbol-map names as behavior guide | deferred until DuskScript M0/M1 |
| — | KDK / HENNA0 | none anywhere we hold | parked indefinitely | n/a |
Sequencing: W → P → C″T1 → C″T1b → C″T2 → C″T3; T4 rides the DuskScript milestones.

**PLAN A — Anim Audition mode (user request, 2026-07-18: "eventually we'll want to GIVE logic
to all animations present"):** extend Cut Actors with a per-spawned-actor **Anim Override**
control: freeze the actor's AI dispatch (hold current action, zero speed), then cycle/select
any BCK in its loaded arc (the leftover viewer already enumerates + plays BCKs — reuse that
machinery on the live actor's morf), with loop/once + speed control. Purpose: the user
auditions each animation in-world on the REAL actor at the REAL scale and assigns intended
behaviors; History captures the mapping into mini-specs (e.g., HOOK = latch-onto-wolf counter,
PULL = drag toward pack, DEFEND = frontal block, CHANGE = ???); Cursor implements as new
action states, one sign-off per behavior batch. First customer: gen-2 grabber set; second:
Makar/Medli BCK kits once Tier 2 mounts them. Pipeline: **audition → assign → spec → state.**

**Cursor — Plan W countersign + first ship (2026-07-18):**

| Plan | Stance |
|---|---|
| **W** | **APPROVED** — ship W1+W2a+W2c now; W2b cutType expand waits on `S1 BITE HANG FAIL` log line; W3 hang joint after latch works |
| **P** | ACCEPT after W playtest (pack ×3 preset) |
| **C″** | ACCEPT TP-bytes-only Phase 1 framing; counterplan after W confirms (TestCube gate first); Kdk visit = user ask |
| **A** | ACCEPT direction — backlog after W/P; first customer gen-2 grabber |

**Shipped in this build (TARGET_PC):**
1. **W1** — `OS_REPORT("S1 BITE HANG FAIL cut=%d keep=%08x health=%d gen2=%d")` on refuse.
2. **W2 hang mute** — refuse no longer bare-returns; falls through to normal damage.
3. **W2a** — `resetWolfEnemyBiteAll` on Cut Actors despawn (if keep is tracked) + on `daE_S1_Delete` if latched.
4. **W2c** — clamp `health` floor 0; if `health<=0` and not FAIL*/WOLFBITE → `e_s1_enter_fail_wait`.

#### Paste for History (2026-07-18 — Plan W countersign + ship)

```
Cursor countersigned Plans W/P/C″/A (run doc).

W APPROVED — shipped TARGET_PC: W1 FAIL log (cut/keep/health/gen2); hang-refuse falls through to damage (no mute); W2a resetWolfEnemyBiteAll on cut despawn + E_S1 delete; W2c HP floor + fail-route.
W2b cutType expand deferred until FAIL log shows which cutType; W3 hang joint after latch works.
P/C″/A accepted as sequenced — no Tier1 TestCube yet.

Please re-playtest wolf lunge on gen-2 + shipping; if latch still fails, paste the S1 BITE HANG FAIL line (cut=).
```

**History — KDK JAILER EXPERIMENT: implementation-grade spec (user-ordered, 2026-07-18;
Cursor implements — small, self-contained, pairs with safe-stub hardening):**

Goal: determine whether the retail story flow ever attempts to spawn `Kdk` during/after the
Demo07 Zelda-visit cutscene (user hypothesis: Kdk = the unseen jailer).

1. **Stub create-attempt log** — at the framework create site (or per-stub profile create),
   when procName ∈ {NPC_KDK, NPC_MK, NPC_P2, NPC_HENNA0}: log
   `[StubWatch] create-attempt proc=<name> stage=<> room=<> layer=<> params=<%08x>
   pos=(<x,y,z>)` then return `cPhs_ERROR_e` safely (the hardening behavior). Kdk's expected
   fingerprint if the hypothesis is right: stage=R_SP107 room=3 params=000FAFFF
   pos=(19676,4500,2633).
2. **Layer-change log (the decisive instrument)** — at the DZR layer-selection site in
   d_stage.cpp (where the ACT0..ACTe chunk set is chosen per room load/relayer): when
   stage==R_SP107, log `[StubWatch] R_SP107 room=<n> layer=<k>`. Even if create is never
   attempted, this tells us which layers the intro sequence actually activates in R03 —
   layer 0 activation at the jailer beat = the hypothesis confirmed at the data level.
3. **User protocol**: fresh-ish save through the intro → sewers → tower climb → Zelda scene →
   exit; then grep the session log for `StubWatch`. History analyzes the trace.

**Cursor — StubWatch shipped (2026-07-18, user-ordered):**

- `include/d/d_stub_watch.h` — shared refuse + R_SP107 layer log (DuskLog + OS_REPORT).
- **NPC_MK / NPC_P2:** Size=`sizeof(fopAc_ac_c)` on TARGET_PC (was 0x1); Create → StubWatch + `cPhs_ERROR`.
- **NPC_KDK / NPC_HENNA0:** non-NULL method table on TARGET_PC; Create → StubWatch + `cPhs_ERROR`.
- **d_stage `layerActorLoader`:** `[StubWatch] R_SP107 room= layer=` when stage matches.
- Cut Actors UI still parks these presets (refuse spawn); retail DZR create path is the experiment.
- **Plan R** (receiver infra): noted — counterplan after jailer trace; no WW bytes this slice.

#### Paste for History (2026-07-18 — StubWatch shipped)

```
Cursor shipped StubWatch (user-ordered jailer experiment).

TARGET_PC: MK/P2/KDK/HENNA0 create → [StubWatch] create-attempt … then cPhs_ERROR (safe).
MK/P2 size fixed (was 0x1). KDK/HENNA0 got method tables (were NULL).
d_stage layerActorLoader logs [StubWatch] R_SP107 room= layer= on that stage.

User protocol: intro → sewers → Zelda scene → exit; grep session log for StubWatch.
Kdk fingerprint if hit: R_SP107 room=3 params=000FAFFF pos≈(19676,4500,2633).

Plan R (receiver): acknowledged — Cursor counterplan after jailer trace; no WW mount.
```

**History — deliberation answer logged (stub-socket route vs blind custom port):** see
TPHistory §3.8b addendum; summary — the sockets (proc ids `fpcNm_NPC_MK/P2/KDK_e`, dylink
module routes c_dylink.cpp:589/692/694, spawn-table names incl. Medli's 3 subtypes, debug
roster) mean restoration = FILLING existing slots, not grafting new ones; assets + behavior
are equal work either way, but identity, registry, subtype plumbing, and ghost-placement
service (the Kdk experiment itself) come free only on the socket route.

**PLAN R — "The Receiver": TP-side infrastructure for a self-contained WW-crew mod
(History proposal 2026-07-18; needs Cursor counterplan + dual sign-off + user OK. Principle:
TP ships machinery that can RECEIVE the characters but implements none of them — with the mod
folder absent, behavior is byte-for-byte phase-1 vanilla incl. safe stubs).**

**TP-side (base game, wholly TP-purposed, inert without payload):**
| # | Component | What it is | What it is NOT |
|---|---|---|---|
| R1 | **Socket adapter** | Stub creates (NPC_MK/P2/KDK/HENNA0) consult a provider registry: no provider → safe `cPhs_ERROR` (phase-1 behavior); provider present → delegate create/execute/draw to the generic mount | no character logic |
| R2 | **Arc-mount capability** in the existing load-order mod manager | a mod declares `arcs/<Name>.arc` → mounted into the res system under that archive name via the STOCK RARC loader, sourced from the mod folder (never the game tree) | no WW bytes in repo/game; generic — serves every future mod |
| R3 | **NPC manifest schema + parser** (Fluffy-style ini, consistent with modinfo.ini) | fields: arc, model member, idle/talk/greet BCKs, collision cylinder, attention+talk config, display name, neck joint (head-tracking), **subtype table** (P2a/b/c arg 0-2 → per-variant anims/attachment/props), optional dialogue file, optional future DuskScript hook (ignored until code-mods lane lands) | no defaults that conjure content |
| R4 | **Generic external-NPC mount actor** | reads manifest → resLoad mounted arc → J3D model + idle BCK + ground snap (L1) → cc cylinder + attention/Z-target/talk prompt (L2) → native dialogue box from mod text (L3, reuse the ALBW NPC dialogue recipe) → TP NPC head-tracking via manifest joint (L1.5) | knows nothing about Makar/Medli specifically |
| R5 | **Cut Actors evidence-panel tie-in** | parked entries show `socket: payload absent/present (<mod name>)` | — |

**Mod folder (self-contained, in the mod manager's scan dir):**
`modinfo.ini` · `arcs/Mk.arc` + `arcs/P2.arc` (user's own WW extraction, unmodified — never
committed) · `npc/mk.ini` (arc=Mk, model=mk.bdl, idle=mk_wait, talks=mk_talk01/02, name,
neck joint) · `npc/p2.ini` (+ subtype table: P2a/P2b/P2c variant configs, dagger attachment
option) · `dialogue/*.txt` (text we author) · later `scripts/` (DuskScript behaviors).

**Sequencing:** R-phase after C″ Tier 1/1b (safe stubs are R1's foundation). R1+R2+R3+R4-L1
first (visible, idle, head-tracking Makar via mod folder); R4-L2 next (solid, targetable);
R4-L3 (talkable) after. Acceptance at every step: delete the mod folder → vanilla phase-1
behavior, zero WW bytes anywhere in the game tree, all sockets safe.

**SIGN-OFF: USER GO (2026-07-18, "tell Cursor to do it") + History APPROVED.** Added
deliverable: **Cursor creates the ready-to-receive folder** — `WW-Crew-Restoration/` skeleton
in the mod manager's scan dir: filled `modinfo.ini`; `arcs/` + README naming the exact files
the user drops in (`Mk.arc`, `P2.arc`, optional `Md.arc`) with a never-commit warning;
`npc/mk.ini` + `npc/p2.ini` PRE-FILLED from History's census (model members, idle/talk BCK
names, P2a/b/c subtype table, dagger-attachment slot, neck joints for head-tracking);
`dialogue/` sample. Cursor countersign closes the dual requirement.

**Cursor — Plan R first ship (2026-07-18, USER GO):**

| Piece | Status |
|---|---|
| Skeleton | `tools/ww_crew_restoration_skeleton/` + boot seed → `%AppData%/…/model_replacements/WW-Crew-Restoration/` |
| R2 | `arcs/<Name>.arc` → DVD overlay `res/Object/<Name>.arc` (enabled mods) |
| R3 | `npc/*.ini` parser → provider map |
| R1 | NPC_MK/P2 Create → mount if payload else StubWatch ERROR |
| R4-L1 | `d_ext_npc_mount` — resLoad + idle McaMorf + ground snap + draw |
| R4-L2/L3 | not yet (collision/talk) |
| R5 | Cut Actors labels note socket; MK/P2 unpark when payload present |
| Invariant | delete/disable mod or empty arcs/ → StubWatch phase-1 |

#### Paste for History (2026-07-18 — Plan R first ship)

```
Cursor shipped Plan R vertical slice (USER GO).

Seeded WW-Crew-Restoration in model_replacements (modinfo + arcs/README + npc/mk.ini + p2.ini + dialogue/sample). Repo template: tools/ww_crew_restoration_skeleton/.
R2: arcs/*.arc overlay as res/Object/<Name>.arc.
R1+R3+R4-L1: NPC_MK/P2 mount idle when arcs+manifest present; else StubWatch ERROR.
Cut Actors unparks MK/P2 only when payload present.

User: drop Mk.arc / P2.arc into arcs/, enable mod, spawn NPC_MK from Cut Actors (or retail Mk placement).
L2 collision / L3 talk / head-track still TODO. No WW bytes in repo.
```

**History — playtest №8 analysis (2026-07-18): jailer NEGATIVE, Makar crash diagnosed, anims
answer.**

1. **E_s1 anim count:** still correct-as-coded for a solo spawn — the unlock vehicles are
   Plan P (pack ×3 preset) and Plan A (Anim Audition mode), neither shipped yet. Queue after
   Plan R stabilization.
2. **Jailer experiment: RUNTIME NEGATIVE, cleanly.** Full intro trace: R_SP107 room 3 loads
   ONLY layers 14, 8, 10 (other rooms also 7/11) — **layer 0 never activates; no
   `create-attempt proc=NPC_KDK` ever fires.** Verdict: the Kdk placement is a
   **layer-orphaned ghost** — retail flow never even tries to spawn it. The jailer-identity
   hypothesis stays circumstantial (placement+scene fit) and cannot be confirmed at runtime
   by normal play. Optional follow-up (small, sign-off): a debug "force layer" toggle or
   editor layer-view would prove the MECHANISM (engine attempts Kdk on layer 0) but not the
   identity — History's recommendation: record as layer-orphaned, folk-tier identity, move on.
3. **Makar crash: Plan R worked END-TO-END until J3D instantiation.** Log: provider
   registered, R2 arc-mount OK, socket create fired (procSize=2200), **all 14 resources
   enumerated + loaded by name from the mounted WW arc (13 BCKs + mk.bdl 47,008 B +
   mk_maba.btp)** — then `EXCEPTION_ACCESS_VIOLATION`, fault addr `0x20000000200` (garbage
   pointer with big-endian smell) right after the model data load. **Diagnosis: WW's `mk.bdl`
   is BDL4 with an `MDL3` section (precompiled GX display lists); TP's own content is
   near-uniformly BMD3, so the port's J3D loader likely mis-walks the BDL4/MDL3 layout →
   wild pointer.** History correction: my "TP provably loads WW-vintage J3D" premise cited
   the itemmdl BDLs *shipping* on disc — whether retail TP ever *runtime-loads* one was
   untested; this crash says the PC port path doesn't handle it. **Fix lane (Cursor):
   receiver-side BDL4 handling — parse standard sections, skip/ignore MDL3 (community J3D
   tools' standard approach), as an IN-MEMORY adaptation in the mount layer (mod bytes stay
   pristine on disk — consistent with the separation directive).** Quick validation asset:
   P2's `p2.bdl` will hit the same path; a BMD-only test (any TP bmd via the same mount)
   would isolate loader-vs-mount if needed.

**History — LAYER-0 SPAWN spec (user-ordered 2026-07-18, "via the warp menu"; Cursor
implements — small):**

Goal: pursue the Kdk trace by making retail's never-activated layer selectable.

1. **Warp-menu field: `Room layer override` = Auto / 0–14 (default Auto).** Implementation
   point: the SAME d_stage layer-selection site StubWatch already instruments — when override
   set, return it instead of the computed layer (all chunk families: ACTx, and naturally
   Envx/LGTx/PALx variants follow — expected side effect, fine for a debug tool). Persist only
   for the current warp (reset to Auto on next stage load) to avoid story-flow weirdness.
2. **Expected result on warp → R_SP107 room 3, layer 0:** StubWatch logs `layer=0`, then
   **`create-attempt proc=NPC_KDK params=000fafff pos=(19676,4500,2633)`** and safe-ERROR —
   the engine executing Nintendo's authored Kdk placement for the first time. Nothing visible
   spawns (no model exists, no provider registered) — the log line IS the artifact. Layer 0
   in R03 contains ONLY Kdk (census), so no other scene changes expected there; other rooms
   under layer 0 may differ (also interesting).
3. **Acceptance:** no crash (safe stubs), StubWatch shows the attempt, Auto restores normal
   behavior.
4. **Flagged for later (separate sign-off, not now): a Kdk PAYLOAD via Plan R** — the socket
   adapter means a provider could give Kdk a body using TP-ONLY bytes (e.g., an existing TP
   soldier arc as a stand-in) — speculative visual, folk-tier, clearly labeled if ever done.

**Cursor — LAYER-0 + BDL4 ship (2026-07-18, after №8):**

| Piece | Status |
|---|---|
| Room layer override | Warp UI `Room layer override` Auto/0–14; `setRoomLayerOverride` → `getLayerNo` force; clears on *next* stage load |
| BDL4/MDL3 | `J3DModelLoader::loadBinaryDisplayList` skips `MDL3` on TARGET_PC; ExtNpc mount sniffs raw `J3D2bdl4`/`bdl3` (BDLM folder) → in-memory parse + SharedDL (mod bytes pristine; itemmdl raw path untouched) |
| Plan P / Plan A | still queued after playtest |

#### Paste for History (2026-07-18 — LAYER-0 + BDL4)

```
Cursor shipped №8 blockers (USER GO layer-0 + Makar BDL4).

1) Warp menu: "Room layer override" Auto/0-14. Forces dComIfG_play_c::getLayerNo (StubWatch /
   layerActorLoader site). Persists for the warp's stage load; clears on the following stage load.
   UI snaps to Auto after Warp press.
   Expect: warp R_SP107 room 3 @ layer 0 → StubWatch layer=0 then
   create-attempt proc=NPC_KDK params=000fafff pos≈(19676,4500,2633) + safe ERROR.

2) Makar/BDL4: Mk.arc folder type is BDLM (raw J3D2bdl4 + MDL3+MAT3). Mount-layer resolve in
   d_ext_npc_mount: loadBinaryDisplayList + SharedDL finish. TARGET_PC skips MDL3 walk
   (archive bytes read-only). Did NOT global-instantiate BDLM in loadResource (would break
   itemmdl private raw path). P2.bdl hits same path.

Playtest: wipe dawn_cache.db* / pipeline_cache.db*; enable WW-Crew-Restoration; Cut Actors
NPC_MK; warp Jailer @ layer 0. Plan P + Plan A still queued.
```

**History — playtest №9 analysis (2026-07-18): KDK MECHANISM PROVEN; Makar crash moved — next
diagnostic ladder.**

1. **KDK LAYER-0 EXPERIMENT: SUCCESS.** Log 110801 line 6715:
   `[StubWatch] create-attempt proc=NPC_KDK stage=R_SP107 room=3 layer=0 params=000fafff
   pos=(19676,4500,2633)` — the layer override worked and **the engine executed Nintendo's
   authored Kdk placement for the first time**, exact predicted fingerprint, safe-declined.
   Trace closed at mechanism level; identity remains folk-tier (jailer hypothesis,
   circumstantial). TPHistory §3.8b addendum to follow.
2. **Makar crash MOVED (BDL4/MDL3 fix engaged):** fault `0x20000000200` → `0x10` (near-NULL
   deref), rva 0xd6258 → 0xd604a, same neighborhood; all 14 resources still load; crash
   right after `mk_maba.btp`. Reading: MDL3 skip worked; now a later stage returns NULL and
   is dereferenced. **Diagnostic ladder for Cursor (cheap→deep):**
   - **D1 stage-pinpoint logging**: NULL-check + log each mount stage (`J3DModelData` create /
     `J3DModel` create / BTP bind / morf-anim attach) — name the failing call.
   - **D2 A/B: drop `mk_maba.btp` from the manifest** — crash sits right after its load; WW
     `J3D1btp1` binding to WW material name tables is a prime near-NULL suspect. If Makar
     appears without it: blink anim deferred (cosmetic), root found.
   - **D3 A/B: mount a TP-native bmd** through the same path (any TP NPC model) — fences
     mount-machinery vs WW-format tolerance.
   - **D4 systematic: WW bdl4 vs TP bmd3 divergence audit** in the port loader (MAT3 entry
     layout/indirect block, TEX1 name-table, SHP1 attribute lists — the classic normalization
     points community J3D loaders handle).
   - **D5 Plan B (the Beta Link road)**: offline adaptation tool in `tools/` (python, cousin
     of bmd_reskin) rewriting bdl4 → TP-shaped bmd3 into the MOD FOLDER as adapted members —
     precedent: the playable Beta Link pack is an "adapted remount, NOT raw extracts."
     Directive-compliant (mod folder owns its bytes); keeps receiver simple. Choose D4 vs D5
     by D1-D3 findings.

**Cursor — D1–D3 ship (2026-07-18, after №9 ladder):**

| Piece | Status |
|---|---|
| D1 | `[ExtNpcMount:D1]` logs each stage (resolve / finish / heap / McaMorf); null-guards material[0] before McaMorf (AV at +0x10 hot path) |
| BDL flags | loadBinaryDisplayList now uses itemmdl flags `0x59020010\|0x2000` (`DoBdlMaterialCalc` → patched MAT3) — likely root of post-MDL3 near-NULL |
| D2 | manifest `skip_btp=1` (mk.ini default on); `d_resorce` leaves BTP raw for that arc during resLoad |
| D3 | `source=dvd` + `npc/mk_d3_tp.ini.example` (npc_ks / saru) — flip over mk.ini for TP-native A/B |
| D4/D5 | wait on D1–D3 playtest |

#### Paste for History (2026-07-18 — D1–D3 ladder)

```
Cursor shipped №9 Makar diagnostic ladder D1–D3 (+ likely fix).

D1: [ExtNpcMount:D1] stage logs + null-guard material[0] before McaMorf
    (create() deref at +0x10 matches fault 0x10).
BDL flags: now 0x59020010|0x2000 like itemmdl (DoBdlMaterialCalc / patched MAT3).
    Prior mount used plain 0x59020010 → wrong MAT3 path after MDL3 skip.

D2: mk.ini skip_btp=1 → d_resorce leaves mk_maba.btp unparsed (L1 never binds it).
    Flip skip_btp=0 for B side.

D3: source=dvd supported; npc/mk_d3_tp.ini.example → npc_ks/saru.bmd + saru_wait_a.bck
    through same mount (copy over mk.ini to run).

Playtest: wipe caches; enable WW-Crew-Restoration; Cut Actors NPC_MK.
Expect either COMPLEATE + visible Makar, or last [ExtNpcMount:D1] line naming the fail.
Then D3 flip if WW still dies. D4 vs D5 after findings.
```

**History — playtest №11 analysis (2026-07-18): D5 ADAPTATION WORKS (WW actor renders in TP!)
— but MAJOR IDENTITY CORRECTION (History's error, user + WWHD naming caught it):**

1. **The mount + offline adaptation SUCCEEDED** — a Wind Waker character model loaded,
   instantiated, and DREW in TP for the first time. Remaining visual defect: all-black body
   with glowing eyes = the predicted no-light-binding failure (eye materials are unlit →
   render; body channels are lit → receive zero bound lights → black). **Cursor fix: standard
   TP actor lighting in the mount (settingTevStruct / setLightTevColorType + light entry per
   frame) — the §"TP lighting" deliberation's murk risk at maximum.**
2. **IDENTITY CORRECTION (mine, not Cursor's — Cursor copied exactly the arcs History named):**
   WW's `Mk.arc` is NOT Makar — it is **Ivan of the Killer Bees (Windfall kid)**: user
   recognized the silhouette instantly; WWHD names Makar `cb.szs`; and the anims agree
   (`mk_jida*` = jidanda foot-stamping tantrum). Verified on our WW extraction:
   **Makar = `Cb.arc`** (`cb.bdl` + `cb_face.bdl` LEAF FACE + `cb_cello.bdl` + nut/sow
   ceremony + play/fly/bow anims; actor `d_a_npc_cb1`), **Medli = `Md.arc`** (`d_a_npc_md`).
   **And `P2.arc` is NOT Medli: it is the small-pirate SHARED BODY with THREE HEADS**
   (`p2head01/02/03` + book/telescope/daggers = Tetra's small crew) — so TP's
   `P2a`/`P2b`/`P2c` subtypes are almost certainly the THREE PIRATES (head-variant
   selection), not "three Medli configurations." §3.8b will be rewritten: **TP's stub
   roster is WW-branch RESIDUE (a Windfall kid + a pirate body), not a planned sage cameo**
   — less romantic, but stronger dolzel2-branch evidence (WW's town-NPC roster survived in
   TP's tree to mastering). TP has NO cb/md stubs — the true sages were never in TP's roster.
3. **Makar (real one) is READY: adapted `Cb.arc` now in the mod folder.** Manifest note for
   Cursor: Makar's face is a SEPARATE model (`cb_face.bdl`) — the mount manifest needs an
   attachment-model concept (model + joint bind), same machinery the pirate heads and
   Medli's harp will want. Minimal Makar = `cb.bdl` body + `cb_face.bdl` face + `wait01`
   idle. Socket policy decision for user+Cursor: bind NPC_MK socket → Cb (user's sage goal)
   and keep Ivan/pirates as bonus generic entries, or keep sockets historically faithful.
   History recommendation: sockets serve the user's goal (sages); the residue truth lives in
   the docs.

**USER DIRECTIVE (2026-07-18): forward push order — (1) IVAN RENDERS PERFECTLY first, then
(2) present Makar + Medli. TP side stays purely receptive throughout.**

**Slice I — "Ivan, perfectly" (Cursor; acceptance = user screenshot):**
- I1 **Light binding** in the mount's frame path: `settingTevStruct(TEV_TYPE_ACTOR)` /
  `setLightTevColorType` + kankyo light entry each frame (the all-black cause; eyes glow
  because their materials are unlit). Expect full color + scene-lit shading + fog/twilight
  participation.
- I2 **Re-enable the blink**: `skip_btp=0` for mk.ini once stable — `mk_maba.btp` = まばたき
  blink pattern; it was exonerated as crash cause (crash persisted with it skipped). If it
  crashes on bind now, defer (cosmetic) and log.
- I3 Idle sanity: `mk_wait` loops, ground snap, scale sane vs Link (Ivan is a child — eyeball
  vs the screenshot), shadow present.
- Acceptance: colored, lit, blinking(±), idling Ivan in Ordon daylight + one twilight zone
  screenshot (lighting deliberation's murk risk check).

**Slice II — "Present the sages" (after I; receiver stays generic):**
- II1 Manifest **attachment-model support** (model + joint bind, per-subtype): needed for
  Makar's separate `cb_face.bdl` (leaf face), later pirate heads (`p2head01-03`) + Medli's
  harp. Generic mechanism, zero character knowledge in TP code.
- II2 Socket assignment per user decision: **NPC_MK socket → `Cb.arc` (Makar: cb.bdl +
  cb_face attachment + `wait01` idle)**; **NPC_P2 socket → `Md.arc` (Medli: md.bdl +
  `wait` idle; harp optional attachment)**. Ivan + pirates remain in the mod folder as
  future generic entries (one multiplexed slot can host all four later).
- II3 Same acceptance bar as Ivan: colored, lit, idling; screenshots.
- Both slices data+generic-code only; delete mod folder ⇒ vanilla. No character logic in TP.

**Cursor — Slice I ship (2026-07-18, USER DIRECTIVE "Ivan perfectly"):**

| Piece | Status |
|---|---|
| I1 Light | `settingTevStruct(0/4)` daylight/twilight + `setLightTevColorType_MAJI` each draw; `dKy_tevstr_init` on create; **light-mask pass** (loaderBasicBmd equivalent; force `0xFF` if mask collapses to 0 — WW body channels) |
| I2 Blink | `skip_btp=0`, `btp=mk_maba.btp`; `mDoExt_btpAnm` loop bind — fail soft + log if bind dies |
| I3 Idle | `mk_wait` loop; ground snap; `scale=` manifest; simple shadow; Cut Actors label → Ivan |
| Slice II | waiting user screenshot acceptance of I |

#### Paste for History (2026-07-18 — Slice I Ivan)

```
Cursor shipped Slice I "Ivan perfectly" (USER DIRECTIVE).

I1: actor tev 0/4 + MAJI every frame; tevstr_init; light-mask pass (zero→0xFF) so lit body
    channels receive kankyo lights (all-black cause).
I2: skip_btp=0 + btp=mk_maba.btp bound as looping mDoExt_btpAnm (defer+log if bind fails).
I3: idle loop, ground snap, scale manifest, simple shadow. Cut Actors label = Ivan socket.

Acceptance: screenshot Ordon daylight + one twilight — colored, lit, blinking(±), idling.
Slice II (attachments + MK→Cb Makar / P2→Md Medli) waits on that pass.
```

**History — №12 (2026-07-18): Slice I still black; MAT3 healthy ⇒ mount light path.**
Live-state tip: I1-REVISED — copy leftover viewer draw sequence.

**Cursor — I1-REVISED (2026-07-18, after №12):**

Draw path now matches `d_demo_leftover_viewer::draw` verbatim intent:
`setLightTevColorType_MAJI(model, &Link->tevStr)` → `dComIfGd_setList()` → `entryDL`.
(Viewer never lights from a private tevStr — Link's is already `Material_use_fg==124`.)
Also: material `change()` before MAJI so SharedDL amb patches land; private tevStr still
filled via `settingTevStruct(0,…)` for shadow. Shadow from attempt-1 kept.

#### Paste for History (2026-07-18 — I1-REVISED)

```
Cursor I1-REVISED after №12 (still black; History MAT3 clear).

Root read: leftover viewer lights arbitrary models by reusing Link->tevStr + MAJI + setList
+ entryDL — not a private settingTevStruct tevStr. Mount now does that.

Also material->change() before MAJI (SharedDL amb patch). Shadow kept.
Re-test Ivan NPC_MK daylight; if still black next suspect is SharedDL bake / non-MAJI path.
```

**Cursor — boots/leaf litMask absorb (2026-07-18, user ask):**

Honest: I1-REVISED was leftover-viewer tevStr, **not** the WW boots/leaf recipe. History
№12c already fixed data-side (`adapt_bdl_arcs.normalize_litmask` → 0x01; AppData Mk.arc
verified `J3D2bmd3` + enabled masks `[0,1]`). Mount had been forcing litMask `0xFF` on
zero — that fights the boots recipe. **Now clamp ENABLED channels to `0x01` (Kmdl_13 /
boots-proven); leave unlit eyes alone.** Draw path still uses Link tevStr + MAJI.

#### Paste for History (2026-07-18 — boots litMask on mount)

```
Cursor absorb №12c boots/leaf litMask (user check).

Prior I1-REVISED = leftover viewer Link->tevStr (not boots). History already regenerated
arcs with normalize_litmask→0x01. Mount applyActorLightMask now matches boots: enabled
channels → 0x01 (was wrongly 0xFF). Unlit untouched. Retest Ivan.
```

**Cursor — I1 boots/leaf DRAW path (2026-07-18, after №13b USER tip):**

Mount draw now matches `d_a_alink` WW boots/leaf/bow cel branch (Blender-WW-Items.md:41 —
**no MAJI**):
`settingTevStruct(0)` → `dWwItemmdl_setWwBowActorAmbient` →
`dWwItemmdl_applyBowMaterialAmbientOnly` → amb forced onto every material → `entryDL`.
Link-tevStr/MAJI path removed. Flattened-ramp arcs stay for max visibility.

#### Paste for History (2026-07-18 — boots ambient draw)

```
Cursor shipped №13b: mount draw = WW boots/leaf recipe (no MAJI).

settingTevStruct(0) → setWwBowActorAmbient → applyBowMaterialAmbientOnly → entryDL
(+ amb on every material; name==NULL covered). Same helpers as iron boots / deku leaf.
Retest Ivan — expect warm lit body. Then History can A/B authentic toon ramps.
```

**Cursor — №14 bloom lever (2026-07-18): mount ambient tunable**

Mount no longer uses bow `kWwBowFixedAmb*` (105/78/48). Manifest `amb=RRGGBB` (default
`5a5a5a` / 90,90,90 neutral) + live `WREG_F(30/31/32)` R/G/B offsets. Draw still
boots-path (no MAJI). History authentic-ramp regen is data-only retest.

#### Paste for History (2026-07-18 — amb tune)

```
Cursor №14 bloom lever: mount amb owned by NPC, not bow tint.

amb=5a5a5a in mk.ini (90/90/90). Live: WREG_F(30)=R WREG_F(31)=G WREG_F(32)=B offsets.
settingTevStruct(0) → write amb to all mats → entryDL. No MAJI / no setWwBowActorAmbient.
Dial bloom down with negative WREG or darker amb= in ini. Slice II after user OK.
```

#### Paste for History (2026-07-18 — Slice II ship)

```
Cursor shipped Slice II GO (attachment support + manifests; `cmd /c build_run.bat` passed).

Mount parser now reads top-level attach_model[/2] + attach_joint[/2] and [subtype.N] arg,
idle, attach_model, attach_joint, display_name. P2 params 0/1/2 select P2a/P2b/P2c,
override label/idle, and bind the matching head BDL to joint "head". Attachments use the
same BDL resolve/light-mask/ambient draw path as the body; after modelCalc they follow the
resolved joint matrix. Missing model/joint is warn-and-skip, never a create crash; joint
failures log available body names (important for Medli's requested hand_l harp bind).

NPC_HENNA0 is now a guarded mount socket: params 0→NPC_MD (Md.arc/Medli + optional harp),
1→NPC_GND (Gnd.arc/Ganondorf). Cut Actors exposes all three pirates plus Medli/Ganondorf,
and only unparks HENNA0 when either matching manifest is loaded. Exact p2/md/gnd manifests
were written to the repo skeleton and live AppData mod; no WW archive bytes were added.
```

#### Paste for History (2026-07-18 — №17+№18 fix)

```
Cursor shipped №17 wiring + Medli arms + №18 lifetime (`cmd /c build_run.bat` passed).

ROOT CAUSE №17 Ganon→Medli: Cut Actors Entry is (label, actorId, angleX, params, …).
Slice II put subtype 1/2 into angleX and left params=0 — HENNA0 create reads
GetParam()&0xFF so Ganon always selected NPC_MD. Same bug hit P2b/P2c. Fixed entries
to put 0/1/2 in params; HENNA0 logs params→proc at create.

Medli hands: not a prop attach — mdarm.bdl is a second skinned piece with
mdarm_wait01.bck. Mount gained companion_model/companion_idle (synced base TRMtx +
own McaMorf). Harp joint is WW `handL` (was wrong `hand_l`); joint search prefers
companion table so harp binds to the arm model.

№18: killed s_skipBtpArc create-time global — shouldSkipBtp is now provider lookup by
arc. acquireMountedModel GameHeap-pins load/finish and session-caches by "arc/model";
actors keep own J3DModel/McaMorf over shared ModelData; never write actor-heap pointers
into dRes slots. Multi-spawn should no longer cross-dangle.
```

**History — OUTSET ISLAND FEASIBILITY (user-ordered research, 2026-07-18): all three
unknowns GREEN.**

1. **Assets located + tiny**: `Ex WW/res/Stage/sea/Room44.arc` (729KB total): island
   `model.bdl` 400KB + water `model1.bdl` 111KB (+`model1.btk` scroll anim — shared
   format, plays in TP) + `model3.bdl` 3KB + **`room.dzb` collision 124KB** + `room.dzr`
   placements + minimap bits. (`LOD44.arc` distant-LOD optional.)
2. **Collision compatibility: PROVEN BY NINTENDO.** TP ships WW's byte-identical
   `kkiba_00.dzb` in `Obj_Bef.arc` AND retail code loads it as collision:
   `d_a_obj_brakeeff.cpp:176` — `mpDBgW->Set((cBgD_t*)getObjectRes("Obj_Bef", 13))`,
   index 13 = `DZB_KKIBA_00` (Obj_Bef.h:0xD). **A TP actor registering a WW-authored
   dzb through dBgW is shipped retail behavior — the exact diorama mechanism.** Caveat
   (minor): tri attribute semantics (sand/water sound codes) may map differently —
   cosmetic, not structural.
3. **Stage materials = the EASY lighting path**: Outset's materials are mostly UNLIT +
   vertex-colored (chan pool: 1 lit / 2-3 unlit, vtx sources; tevRegs already sane —
   whites + real colors incl. the sea-blue (70,90,150); no 128-gray placeholders
   sampled). The entire Ivan cel saga does not apply to the island — it renders on the
   simplest path. Water = model1 + btk texture scroll; skybox optional for diorama
   (TP host-stage sky works; WW vrbox adaptation later for the full-stage road).
4. **Scale precondition already banked**: Ivan correct at scale 1.0 ⇒ WW and TP agree
   on world units.
**Diorama road engineering remaining (Cursor, post-Slice-III):** mount "BG mode" —
manifest declares model+dzb; actor draws (BG tev type) + `dBgW::Set` + placement mtx.
Full-stage road (stage receiver: tables/kankyo/exits) remains the later tier.

**PHASE O — OUTSET ISLAND WARPABLE (user-ordered 2026-07-18; History data-prep DONE,
Cursor implements; rides the GameHeap cache + Slice III):**

**History-shipped (data, in mod folder):**
- `arcs/Outset.arc` — adapted Room44 (island `model.bdl` 396KB + water `model1.bdl`
  + `model1.btk` scroll + `model3.bdl` + `room.dzb` 4,221 tris + `room.dzr`).
- `npc/outset.ini` — bg-mode manifest (spec-by-example): `type=bg`, models,
  `collision=room.dzb`, **`anchor=-200000,0,315000`** (the island is authored at its
  GREAT SEA coordinates — bbox X[-250k,-150k] Z[250k,350k]; the mount translates by
  `host_pos - anchor`), `spawn_rel=-1651,183,-3011` (village flat, dzb-verified;
  beach alternative `4199,77,5117`).

**Cursor implementation points:**
1. **Mount `type=bg` mode**: no morf/idle — static J3DModels (×3) + btk on model2;
   draw via BG tev type (materials are mostly unlit/vertex-colored — no cel work);
   translate-all matrix = `host_pos - anchor`.
2. **Collision**: `mpDBgW->Set((cBgD_t*)getObjectRes(arc, room.dzb), …)` + same
   translate mtx — the EXACT shipped-retail mechanism (d_a_obj_brakeeff.cpp:176 loads
   WW's byte-identical kkiba dzb). RegistN before first Move. ~4.2k tris = modest.
3. **Warp menu entry** (`src/dusk/ui/warp.cpp`): "Outset Island (WW)" → host stage +
   `host_pos + spawn_rel` as Link's spawn. Gate on `dExtNpcMount_hasPayload` so the
   entry only appears with the mod enabled.
4. **Host stage suggestion**: an open-sky flat expanse far from native geometry —
   Lake Hylia region (island offshore in TP water = free ocean!) or Gerudo flats;
   pick `host_pos` in unused airspace (e.g. high +Y offset island) — Cursor's call
   with user. Sky/fog come from the host stage for the diorama tier.
5. Acceptance: warp lands Link on Outset village flat, collision walkable end-to-end,
   water plane animating, no despawn crash on leaving (BG mount despawns with stage).

**SLICE III DETAIL — full interaction + collision for the crew (History spec, 2026-07-18;
user confirms all five spawn correctly incl. multi-spawn):**

**L2 — solid + targetable (three components, manifest fields already staged):**
1. **Collision cylinder**: `dCcD_Cyl` from `cyl_radius`/`cyl_height` (already in manifests),
   registered per-frame in execute via the mount's CcStts — gives Link bump/push. (NPCs don't
   need dzb-solidity; ground-snap + cylinder is the TP NPC standard.)
2. **Attention**: `attention_info.flags |= TALK` + distances table → Z-target lock + the (A)
   Talk prompt appearing at range. `display_name` feeds the lock-on label where supported.
3. **Head-tracking polish**: `neck_joint` already in manifests; per-frame look-at within a
   yaw/pitch clamp, ease in/out (TP NPC standard behavior).

**L3 — dialogue (reuse the ALBW native-dialogue recipe — two-screen BLO layout, font sizing,
input cooldown — from the fork's existing NPC work), text from `dialogue/*.txt` per NPC.**

**L3 CRASH GUARD (user-mandated, design-in-from-start — fail CLOSED, never softlock):**
- **G1 pre-open validation**: at talk-trigger, verify (a) dialogue file present + parsed,
  (b) message-window archive resident (the known archive-clearance crash trap from the ALBW
  dialogue work), (c) no other dialogue open. ANY failure → log `[ExtNpcMount] talk refused:
  <reason>` + play the NPC's talk anim WITHOUT opening a box — never a half-open state.
- **G2 softlock escape**: while a mount dialogue is open — B closes unconditionally; hard
  timeout (~30s no-input advance) force-closes and returns control; on close always restore
  player state even if the box died mid-open (RAII-style cleanup, not sequential teardown).
- **G3 re-entry guard**: one mount dialogue globally; talk re-trigger during close-cooldown
  ignored (input cooldown from the recipe).
- **G4 despawn safety**: despawn-while-talking (Cut Actors despawn or room change) →
  force-close the box FIRST, then despawn (the wolf-latch lesson applied to dialogue).
Acceptance: talk works on all five; deleting a dialogue file yields refusal-not-crash;
B always exits; despawn mid-talk is clean.

**PHASE M — THE REHOMING (History plan, 2026-07-18; user-ordered): purge all WW-specific
content from vanilla code into the mod folders. TP stays purely receptive. Executes AFTER
Phase O (Outset) is confirmed. Cursor layers implementation detail onto this plan, then
dual sign-off + user OK.**

**M0 — CURSOR DISCLOSURE LEDGER (first step, run this immediately after Outset ships):**
Cursor enumerates EVERYTHING they know to be WW-specific that lives in game source — from
the Outset work or any earlier slice. Known from History's audit (starting checklist, not
exhaustive — Cursor extends):
- `d_cut_actor_spawn.cpp:56-70` — hardcoded entry labels (Ivan/Pirate A-C/Medli/Ganondorf
  + arc names in strings).
- `custom_assets.cpp:110-135` — **embedded manifest templates + dialogue text** (the big
  leak: character identities and words in the binary as folder-seeding strings).
- Suspected (Cursor confirms where these live — code defaults vs manifest): Medli's
  `companion_model=mdarm` + `mdarm_wait01.bck` pairing, `handL` harp joint preference,
  pirate-head `attach_joint=head` defaults, `proc=OUTSET` warp wiring + host-stage/anchor
  constants, any character-named comments that describe BEHAVIOR rather than history.
- Anything else: greps for `Ivan|Makar|Medli|Ganondorf|pirate|Killer|Windfall|Outset|
  mdarm|handL|Mk\.arc|P2\.arc|Gnd\.arc` over src/ tell the rest.

**M1 — Classify each ledger item:** {bytes | name/label | words/dialogue | behavior-default},
current home, target home (which mod-folder file+section), and runtime role
(seed-only / fallback / live-read). Seed-only items delete cleanly; fallbacks convert to
G1-style refusal; live-reads mean the schema is missing a field → M2.

**M2 — Schema completion (Cursor designs):** the manifest must be able to express
everything currently hardcoded: per-subtype display names (P2a/b/c), companion models +
their idle anims, attachment joint tables/preferences, dialogue section keys, bg-mount
warp entries (label + host stage + host_pos + spawn_rel), ambient defaults. Rule: if code
needs a character fact, the manifest says it or the feature refuses.

**M3 — The moves:** delete exe-embedded templates; Cut Actors spawn list + warp menu
ENUMERATE dynamically from discovered payloads (labels = manifest `display_name`; vanilla
shows neutral socket entries "payload: none"); character-specific code defaults become
manifest fields with NEUTRAL code fallbacks.

**M4 — Seeding responsibility moves to the skeleton:** `tools/ww_crew_restoration_skeleton/`
= the mod's source-of-truth templates (no WW bytes there — config text only); install =
folder copy (manual or a tools script beside adapt_bdl_arcs.py). The exe NEVER writes mod
content. (User option, decide at execution: keep skeleton in tools/ vs relocate beside
companion_mods/ for visual absoluteness.)

**M5 — Verification gates (acceptance):**
(a) **binary grep gate**: built exe contains ZERO character strings (Ivan, Makar, Medli,
Ganondorf, Killer, Windfall, pirate-as-character, Outset-as-label);
(b) **folder-removal boot**: mod absent → neutral socket list, no WW names in any UI, no
crashes, byte-for-byte receiver-only behavior;
(c) **folder-present boot**: everything that works today still works identically;
(d) boundary statement: docs/ and tools/ are not shipped game code — exempt (they hold no
WW bytes; never-commit rule for arcs stands).

**M6 — THE RECEIVER COVENANT (standing rule going forward):** every future feature —
starting with the WW Link skin — must pass M5's gates at introduction: labels from
manifests, zero character knowledge in code, exe-grep clean. Add the grep gate to the
push-hygiene checklist.

**PHASE GS SKETCH — THE GREAT SEA + SAILING (History deliberation, 2026-07-18; design
only, sequenced after Phase O + M):**
- **Key mapping**: WW's sea = 7×7 grid of 100k-unit rooms (Room44 bbox = exactly one
  cell) ↔ TP's own multi-room field-stage pattern (per-room offsets + proximity
  streaming). GreatSea = stage-receiver stage: 49 adapted sea rooms anchored by grid
  math; PC RAM trivializes streaming (~35MB whole-sea); WW `LODxx.arc` silhouettes =
  horizon vistas (same adapt tool); f32 at 700k proven by WW itself.
- **Boat = the E_s1 lesson applied**: TP's Iza CANOE is a shipped rideable-watercraft
  chassis (mount/steer/camera). KoRL = adapted WW ship arc worn by that chassis;
  sail speed modulated by TP's own `dKyw` wind vector (the rope-bridge wind system).
  Sea surface v1 = diorama water (proven); wavier custom sea actor later.
- **Phases**: GS-1 multi-island diorama + LOD horizon → GS-2 GreatSea stage (grid
  manifest, warp-to-island) → GS-3 boat (KoRL on canoe chassis, docks per manifest)
  → GS-4 waves/weather/sea-enemies/charts (DuskScript). Receiver-clean throughout.

**PHASE O ACCEPTED (2026-07-18): OUTSET ISLAND TRAVERSED END-TO-END IN TP.** Split-transform
convention (№22: models cell-local, dzb world-baked) + ti through-fix closed it. The complete
WW-import formula now covers models, characters, lighting, AND rooms+collision.

**USER DIRECTIVE — SEQUENCE LOCKED:** (1) **PHASE M REHOME NOW** (Cursor M0 ledger incoming;
M1-M6 per plan; gates: exe-grep clean + folder-removal boot). (2) **PHASE O2 AFTER RE-ENTRY:
FULL OUTSET POPULATION — all quests, functionalities, actors — ENTIRELY MOD-FOLDER-SIDE**
under the M2-completed schema.

**History parallel lane (during M): OUTSET POPULATION BLUEPRINT** from the authentic
`room.dzr` census (DONE, 480 entities, 31 chunk types incl. 15 story layers):
- **P1 props/foliage** (112 grass, flowers, trees ×21, pots, crates, signs, rocks, 11 bridge
  segments, 10 palms): per-item decision — WW prop mount vs native TP substitute (TP has
  grass/pots with behaviors).
- **P2 ambient critters**: 6 pigs (catchable!), 9 seagulls, 2 crabs, forest keese ×26 +
  chuchus — crew pipeline + simple wander/flee behaviors via Anim Audition.
- **P3 residents**: named NPCs (Ls1/Zl1/Ob1/Ko1/Ko2/Ym1/Ym2/Ah/Aj1/Bm1/P1a…) — identities
  to VERIFY against WW actor maps (no guessing); each gets crew-pipeline manifest + dialogue.
- **P4 structure/quests**: 10 SCLS exits → house interiors (separate WW stages to adapt),
  4 doors, tags/switches, item spawns ×14, salvage points, **story-layer mapping (ACT0-b) →
  quest states** (DuskScript-era scripting; simplified flags first).
- **SHIP ×2** = KoRL moorings → Phase GS-3 hooks.
Full blueprint doc: `docs/outset-population-blueprint.md` (History authors during M).

**PHASE M CLOSED (2026-07-18, History gate audit): REHOME CONFIRMED.** Exe grep = 0 across the
full greplist (incl. dialogue tripwires); bare "Ganondorf" ×18 adjudicated TP-NATIVE (npc_gnd,
achievements, editor — he's a TP principal; greplist correctly uses "Ganondorf (WW)").
Embedded seeds gone; SoT skeleton + greplist.txt + install_skeleton.py in tools/; `EXT_BG0`
neutral proc (A1 applied); user re-entered Outset folder-present. Outstanding (non-blocking):
M5b folder-absent boot smoke; A2 alignment (state says first-wins; amendment = load-order
top-wins).

**PHASE O2 — OUTSET ALIVE (user GO): population + the GAME-START QUEST LINE, all folder-side.**

**Generic receiver capabilities needed first (Cursor designs; zero WW knowledge in code):**
- **R-O2a Mod flag store**: per-mod persistent key-value namespace (save-file-scoped), read/
  written by dialogue actions and spawn conditions. The quest backbone.
- **R-O2b Conditional dialogue**: manifest grammar — `[section]` gains `if_flag=` /
  `unless_flag=` / `set_flag=` / `next=` (sequences); G1 refusal semantics preserved.
- **R-O2c Grant action**: `action=grant:<tp_item_id>` from dialogue (TP inventory hosts the
  telescope-as-item? or flag-only first — Cursor proposes).
- **R-O2d Carryable payloads**: mount flag `carryable=1` hooking TP's carry/throw system
  (pigs; TP already carries cuccos/pots — same interface).
- **R-O2e Spawn conditions**: manifest `spawn_if_flag=` / layer-equivalent selection so
  placements track quest state (mirrors WW's ACT0-b layer encoding from the census).

**Quest line (WW Day-1 Outset, from game start — content 100% folder-side manifests+dialogue):**
- Q1 Aryll & the telescope (watchtower talk → flag + grant).
- Q2 Orca's sword training (dialogue chain → sword flag; optional hit-drill later via
  audition-era behaviors).
- Q3 Grandma & the hero's clothes (house interior — FIRST INTERIOR ADAPTATION: LinkRM stage
  via the same room formula; doors from census TGDR wire SCLS→interior warp).
- Q4 Rose's pigs (R-O2d carry → pen flag → reward).
- Q5 The forest crash (upper Outset: keese/chuchu population from census + crash-site beat,
  simplified rescue flag).
- Q6 Village flavor (Mesa/Sturgeon/Joel/Zill/Sue-Belle dialogues; identities verified against
  WW actor maps before labeling — the Ivan rule).
Sequencing: R-O2a/b first (flags+dialogue = quest spine) → Q1/Q3 (pure dialogue+flag) →
population waves P1 props/P2 critters (visual life) → Q4 (carry) → Q2/Q5 → interiors.

## CANONICAL EXTERNAL REFERENCES (user-supplied 2026-07-19 — consult for ALL WW naming/params work)

1. **The Wind Waker decompilation — https://github.com/zeldaret/tww** — AUTHORITATIVE for:
   actor source (d_a_npc_* head/params semantics — ends head_from_params guessing), arc↔actor
   bindings, message-id wiring, layer/switch semantics, prop actor implementations (grass/rock
   draw paths). Consult BEFORE inferring any WW identity, param layout, or member binding.
2. **GameBanana WW modding actor/arc reference — https://gamebanana.com/tuts/18805** —
   community actor-name/arc lookup table; quick first-pass naming, verified against (1).
Both complement (never replace) the Ivan-rule user identity pass for display labels.

## THE WIND WAKER RESTORATION — MASTER PHASE PLAN (History, 2026-07-18)

*Precondition verified: rehome HOLDS post-O2-wave (full greplist clean on the 17:58 exe; all
O2 content strings = 0 in binary). Standing rules for every phase: (1) receiver covenant —
greplist grows per phase, exe-grep + folder-absent gates at each close; (2) the formula
pipeline (adapt tool passes) is the only asset path; (3) TP-native substitution preferred
where WW behavior is expensive (the crawl lesson: check TP FIRST); (4) identities verified
before labels (the Ivan rule); (5) each phase independently playable at close.*

**W1 — OUTSET COMPLETE (in flight).** O2 quests Q1–Q6 finished, doors all wired, pigs
carryable, population waves full, crawl holes armed (WW attr → TP wall-code in adapt_dzb),
interiors polished (Pjavdou spawn fix), crew ON the island at authentic positions.
Acceptance: play WW's Day 1 start-to-forest-rescue entirely in TP.

**W2 — GREAT SEA SKELETON.** Stage-receiver tier: the 7×7 grid as a real stage (grid-math
anchors from the №22 convention — all pre-solved), LOD horizon arcs, batch-adapt all 49 sea
rooms (one tool run), warp-chart UI (sea chart as the warp menu). Islands are EMPTY except
Outset. Acceptance: warp to any square, see neighbors on the horizon.

**W3 — SAILING.** KoRL adapted (arc: the ship model+sails) riding the TP Iza-canoe chassis;
sail speed from TP's dKyw wind; census SHIP moorings = dock points; sea surface v2 (wavier
plane). Acceptance: sail Outset→neighbor square unbroken.

**W4 — WINDFALL.** The big town: room + interiors batch, ~30 NPCs through the crew pipeline
(spawn-and-identify sessions per the Ivan rule), shops (TP rupees), minigame stubs (auction,
Salvatore) as flag-quests first. Acceptance: town day-loop feels inhabited.

**W5 — DRAGON ROOST.** Island + cavern = FIRST DUNGEON: rooms/collision by formula, keys/
locks via flag store, WW enemy wave 1 (Bokoblins etc.) behind Anim-Audition behavior batches
— TP enemies as interim population; Medli's people (Rito) dialogue content; boss = the first
full audition-pipeline boss build (Gohma — and EN_GBA audio knowledge already banked).
Acceptance: dungeon clearable start-to-boss.

**W6 — FOREST HAVEN.** Deku Tree + Koroks (Makar HOME at last — Cb.arc's nut-ceremony anims
finally in context), Forbidden Woods as dungeon #2, **Deku Leaf mechanic** (glide research
already done in this fork — merge lanes with the existing deku-leaf-glide work).

**W7 — MECHANICS WAVE.** WW-specific verbs as TP items: Wind Waker baton (model ships on TP
disc already; conducting = flag-gated cutscene-lite first), grappling hook, picto box lite.
Each = its own sign-off; TP-native hosting throughout.

**W8 — THE WORLD FILL.** Remaining islands batch (mechanical: census → blueprint → populate
per island), secret caves (WW Cave01-09 stages), submarines/platforms, Tingle. Prioritize by
quest-line touchpoints, not completionism.

**W9 — MAIN QUEST SPINE.** The story adapted: pearls → Tower of the Gods (dungeon #3) →
under-sea Hyrule (the frozen castle = a set-piece diorama) → Triforce hunt SHORTENED (design
mercy) — all via flag store + DuskScript when it lands (quest logic scale point).

**W10 — ENDGAME.** Ganon's Tower, boss rush optional, **WW Ganondorf duel** — Gnd.arc's
85-anim moveset through the audition pipeline as the final behavior build; Puppet Ganon
(Bgn.arc) stretch. Acceptance: credits-worthy finish.

*Cross-phase: WW Link skin (user-roadmapped) slots anywhere post-M; full WW cel lighting
(per-frame C-regs) upgrades everything when it lands; DuskScript arrival (code-mods END
GOAL) is the W9 dependency to watch. Sequencing beyond W1-W3 can reorder by appetite —
each phase is a self-contained mod-folder increment.*

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

#### Paste for History (2026-07-18 — Slice III ship)

> Cursor shipped Plan R Slice III with `mode=running`: external WW crew mounts now register
> manifest-scaled `dCcD_Cyl` collision, expose TALK/SPEAK attention, and use best-effort
> manifest `neck_joint` tracking. Native ALBW dialogue loads `[mk.greet]`, `[p2.greet]`,
> `[md.greet]`, and `[gnd.greet]` from the enabled mod's `dialogue/*.txt`; a single global
> owner plus validation, B-dismiss, 1800-frame timeout, re-entry cooldown, and despawn-first
> hide guards prevent dialogue softlocks/crashes. Both skeleton and live manifests/dialogue were
> updated; Outset/Phase O and WW arc bytes were left untouched. Please review playtest evidence,
> especially real WW joint-name compatibility (`head` defaults) and event-talk behavior.

#### Paste for History (2026-07-18 — Phase O ship)

> Cursor shipped Phase O with `mode=running`: `OUTSET` is a payload-gated `type=bg` provider
> mounted through the existing NPC_KDK socket. It loads three static WW models, binds
> `model1.btk` to model2, registers `room.dzb` as `MOVE_BG`, and translates all background
> data by actor host position minus the authored Outset anchor. The warp menu now offers
> “Outset Island (WW)” only when the provider/arc is present; it enters Lake Hylia `F_SP115`
> room 0, places Link at the configured village-relative spawn, and creates the mount at
> `(-60000, 4000, -90000)`. Both skeleton/live manifests now declare `proc=OUTSET`; no WW
> arc bytes were added to git. Please review playtest evidence: village flat collision, water
> animation, and leaving the host stage without a crash.

#### Paste for History (2026-07-18 — №20 collision-matrix fix)

> Cursor applied History №20: 162251 J3D-load crash already fixed by BG ModelData cache
> (162446 loaded clean). Live hologram bug addressed with the brakeeff/Obj_Fmobj pattern —
> `dBgW::Set` against identity, `Regist`, then copy `host−anchor` into `mBgMtx` and `Move()`
> so the vtx table deltas into diorama space; ground probe waits 2 frames after `mBgReady`.
> Log diagnostics: diorama hit vs `ground STILL at Great Sea`. Please review next Outset
> playtest for village walkability + leave-stage cleanliness.

#### Paste for History (2026-07-18 — №21 settings OK, still nothing; History leads)

> User: “Nothing” after №20. Cursor verified settings/payload — all green — and is standing
> by for History’s next implementation spec (no further Outset code until then).
>
> **Settings / payload checklist (all PASS):**
> - `WW-Crew-Restoration` in `game.customModelsOrder` **enabled** (no `-` prefix); not in
>   `customModelsDisabled`
> - Boot log: mod ON, 6 overlays, `provider OUTSET ← … arc=Outset`
> - Live files: `arcs/Outset.arc` 729408 B; `npc/outset.ini` (`type=bg`,
>   `anchor=-200000,0,315000`, `spawn_rel=-1651,183,-3011`, `collision=room.dzb`)
> - Warp entry available (payload gate true)
>
> **Log `dusklight-20260718-163128.log` (post–№20 brakeeff pattern):**
> - warp → island create → BG models+btk → `BG COMPLEATE … mtx translate=(140000,4000,-405000)`
> - settle 2 frames → **`NO ground under village probe (-61651, 6183, -93011) gs_hit=false`**
> - Important: Great Sea probe also misses ⇒ not “floor left at GS identity”; collision is
>   **not queryable at either location** (Regist/Move/ConvDzb/group flags/draw-only path?)
> - `draw tick models=true/true/true` at host — user still reports nothing visible
> - No ACCESS_VIOLATION this run
>
> History: please diagnose №21 root cause and spec the next Cursor change.

#### Paste for History (2026-07-18 — №21 probes shipped)

> Cursor shipped History’s №21 instrumentation at Outset settle (post–Move): **P0** authored
> village × model[0] baseTRMtx; **P0b** model/mBgMtx translation + pm_base/lock/nocalc/flags;
> **P1** root-group AABB min/max; **P2** direct `mpBgW->GroundCross`; **P3** same with
> `PolyPassChk=NULL` (+ block gnd counts on P3 miss). Build launched. Awaiting user camera-fly
> report + log lines `[Outset:P0]`…`[Outset:P3]` for History’s decision matrix.

#### Paste for History (2026-07-18 — №22 split-transform ship)

> Cursor shipped №22: mount parses `model_space=local`; `updateBgTransform` now sets
> **model baseTRMtx = translate(host)** and keeps **mBgMtx = translate(host−anchor)** for the
> dzb. Skeleton seed + defaults use History’s corrected anchor `(-200000,0,300000)` and
> `spawn_rel=(-1651,183,11989)`. Live AppData `outset.ini` already had History’s data patch
> (incl. ti through-fix in Outset.arc). Please review playtest: island at warp, village
> walkable, P0/P2/P3 green.

---

## PHASE M — Cursor M0 DISCLOSURE LEDGER + M1/M2 layer (2026-07-18)

User ordered rehoming start. Cursor posts M0 (exhaustive), layers M1 classification + M2
schema proposal. **No M3 deletes until History sign-off + user OK.**

### Stay (generic receiver — keep in exe)

- ExtNpcMount: manifest parse, companion/attach/subtype, BG type, `model_space`,
  GameHeap ModelData cache, StubWatch, provider rescan, dialogue G1–G4 guards.
- TP stub proc IDs / profiles (`NPC_MK`/`P2`/`KDK`/`HENNA0`) as empty sockets.
- Tools: `adapt_bdl_arcs.py` (format tool; not character identity). Docs exempt (M5d).

### M0 ledger + M1 classification

| ID | Location | What | Class | Current home | Target home | Runtime role |
|---|---|---|---|---|---|---|
| CA-01..04 | `d_cut_actor_spawn.cpp` ~56–71 | Preset labels Ivan/Pirate A–C/Outset/Medli/Ganondorf + arc notes | name/label | hardcoded `kEntries[]` | dynamic from manifests `display_name` (+ subtype names) | live-read → M3 enum |
| CA-05 | same ~125–138 | `isParkedStub` payload keys `NPC_MK/P2/MD/GND/OUTSET` | wiring | code | keep keys as **proc strings from manifests** (generic hasPayload(proc)); no character names in UI | live-read |
| CA-06 | same ~262 | “Makar crash class” refuse string | name/label | code | neutral “stub refuse” | seed-only delete |
| WP-01 | `dusk/ui/warp.cpp` ~356–387 | Button “Outset Island (WW)” + F_SP115 + help | name/label + wiring | hardcoded | manifest warp entry (`warp_label`, `host_stage`, `host_pos`) | live-read → M2/M3 |
| SP-01 | `d_s_play.cpp` ~788 | `pollOutsetWarp` every frame | wiring | Outset-named API | generic `dExtNpcMount_pollBgWarps()` | live-read |
| MT-05/06/13 | `d_ext_npc_mount.*` | Outset warp state machine + `kOutsetHostPos` | bytes + wiring | code constants | `npc/outset.ini` `[warp]` section | live-read |
| MT-09 | `d_ext_npc_mount.cpp` ~383–392 | Default dialogue keys `mk/md/gnd/p2.greet` by proc | behavior-default | code fallback | **delete** — require `dialogue=` or G1 refuse | fallback → refuse |
| MT-14/15 | same | anchor/spawn_rel numeric fallbacks | bytes | code | manifest-only (no fallback coords) | fallback → refuse |
| AS-01..11 | `custom_assets.cpp` `ensure_ww_crew_skeleton` | Entire embedded mod: modinfo, mk/p2/md/gnd/outset.ini, dialogue lines | words + name + bytes | **exe strings** | `tools/ww_crew_restoration_skeleton/**` only; exe never writes | seed-only → M4 |
| AK/AP/AD/AH | `d_a_npc_{mk,p2,kdk,henna0}.cpp` | Create delegates `"NPC_MK"` / `"OUTSET"` / MD\|GND by params | wiring | proc string literals | keep as **socket↔proc binding table driven by manifests** listing `socket=` / `proc=`; HENNA0 params→proc from subtype or `socket_arg` map in mod | live-read |
| ED-01 | `dusk/ui/editor.cpp` ~3170 | “(Makar/Medli/…)” copy | name/label | code | “external payload sockets” | seed-only delete |
| SK-01..03 | `tools/ww_crew_restoration_skeleton/` | dialogue + README (+ incomplete vs embedded seeds) | words/name | tools (OK) | complete skeleton = SoT; fix P2≠Medli README | exempt from exe grep |
| CM-* | comments mentioning Ivan/Medli/Outset/WW | archaeology crumbs | name/label | comments | neutralize or move to docs | N/A |

### M2 — Schema completion (Cursor proposal)

Add to BG / NPC manifests (text ini; no WW bytes):

```ini
# NPC (already mostly present)
proc=NPC_MD
socket=NPC_HENNA0          # optional: which TP stub hosts this payload
socket_arg=0               # params low byte for shared sockets (MD=0, GND=1)
display_name=Medli
dialogue=md.greet
companion_model=mdarm.bdl
companion_idle=mdarm_wait01.bck
attach_model=md_harp.bdl
attach_joint=handL

# BG + warp (new)
type=bg
model_space=local
warp_label=Outset Island (WW)   # empty ⇒ no warp menu row
host_stage=F_SP115
host_room=0
host_layer=-1                   # Auto
host_pos=-60000,4000,-90000
# anchor= / spawn_rel= already exist
```

Cut Actors + Warp UI: scan providers → one row per payload with `display_name` /
`warp_label`. Vanilla-only boot: sockets show `NPC_MK (no payload)` etc., never Ivan.

### M3–M6 execution order (after dual sign-off)

1. Complete `tools/ww_crew_restoration_skeleton/` with full ini+dialogue (M4).
2. Delete `ensure_ww_crew_skeleton` embedded bodies; optional one-shot copy from tools/ if folder missing (user option: tools/ vs companion_mods/).
3. Neutralize Cut Actors + warp to dynamic enum; generalize Outset poll → BG warp table.
4. Kill MT-09 dialogue defaults + numeric Outset fallbacks.
5. **M5 gates:** exe strings clean; mod absent boot; mod present parity.
6. Add M6 grep to push hygiene.

### Cursor sign-off request

**Cursor: M0 APPROVED (exhaustive as known).** M1/M2 above are Cursor’s layer —
History: critique classifications + schema; APPROVE or amend. User OK before M3.

### History critique + sign-off (2026-07-18)

**M0: APPROVED — exceeds History's audit** (SP-01, MT-14/15, ED-01, CM-*, and the README
P2≠Medli catch were beyond my findings; the ledger-first protocol worked). **M1/M2: APPROVED
WITH AMENDMENTS** (each small; none blocks M3 start once acknowledged):

- **A1 (naming, M5-critical): the literal proc string `"OUTSET"` in exe code IS a WW place
  name in the binary** — it fails our own grep gate. Amend: bg-socket proc literals must be
  neutral (`EXT_BG0`-style, or fully manifest-declared and matched dynamically). Same test
  applies to any future bg payload proc.
- **A2 (socket conflicts)**: two manifests claiming the same `socket=`+`socket_arg` must
  resolve by the EXISTING load-order semantics (top-wins, consistent with the mod manager),
  loser logged — not first-come or crash.
- **A3 (MT-14/15)**: agreed manifest-only; specify the refusal — missing `anchor`/`spawn_rel`
  ⇒ warp row hidden or click-refused with log (G1 style), never raw-host spawn into void.
- **A4 (M3 step 2)**: recommend NO exe auto-copy at all — the covenant is "exe never writes
  mod content." Seeding = manual copy or `tools/install_skeleton` script. If a convenience
  copy is kept, it must be file-copy from tools/ (no embedded strings) and OFF by default.
  User decides at execution.
- **A5 (M5 grep list as artifact)**: maintain the forbidden-string list as a FILE
  (`tools/ww_crew_restoration_skeleton/greplist.txt`: Ivan, Makar, Medli, Ganondorf, Killer,
  Windfall, Outset, Aryll, KoRL, pirate-as-character…) so the M6 push-hygiene gate is
  automatable and future characters extend the list, not the code.
- **A6 (skeleton hygiene)**: delete `mk_d3_tp.ini.example` (D3 diagnostic era over); fix the
  P2≠Medli README as ledgered; code comments cite run-doc IDs (№14, №22), not character names.
- **A7 (warp row gating)**: a warp row requires payload present AND arc file readable —
  manifest-without-arcs yields refusal-with-reason, not a dead button.

**History sign-off: APPROVED (with A1–A7 folded into M3). Awaiting USER OK to execute M3–M5.**

#### Paste for History (2026-07-18 — Phase M0 ledger posted)

> Cursor started Phase M per History plan: posted **M0 disclosure ledger** + **M1
> classification** + **M2 schema proposal** in the run doc (no M3 deletes yet). Biggest
> leaks: embedded `ensure_ww_crew_skeleton` strings, Cut Actors character labels, Outset
> warp constants/UI, dialogue-key proc defaults. Generic ExtNpcMount stays. Please
> critique M1/M2 and sign off (or amend) so Cursor can execute M3–M5 after user OK.

#### Paste for History (2026-07-18 — Phase M3 shipped, user go)

> User ordered M3 execute. Cursor shipped Phase M rehoming into `mode=running`:
> - Manifest schema: `socket`/`socket_arg`/`warp_label`/`host_stage`/`host_pos`/…
> - Cut Actors + Warp UI enumerate from providers (`display_name` / `warp_label`)
> - BG warp generalized (`requestBgWarp`/`pollBgWarps`); no host constants in code
> - Stub creates resolve via `resolveSocket` (no MD/GND/OUTSET hardcodes)
> - `ensure_ww_crew_skeleton` = filesystem copy from `tools/ww_crew_restoration_skeleton` only
> - Dialogue-key proc defaults removed; skeleton SoT filled earlier
> - M5a: built exe grepped clean for Ivan/Medli/Outset Island/dialogue lines
> Please review + playtest folder-present / folder-absent boots.

#### Paste for History (2026-07-19 — №34 + №35 packet)

> History №34/№35 implementation packet (+ user rupee ruling). Cursor shipped (rebuild green):
> - **P1:** unmapped `kusax*`/`flower`/`flwr*` (Kusa/long.bmd retired — holes per №31)
> - **P2:** mailbox `Toripost/vpost` confirmed; rocks `Piwa` + `WwAlways` (krock/koisi); chests `WwDalways` boxa/b/c — **never** arc=Always/Dalways (would clobber TP)
> - **P3:** doors.ini `ww_ry=` from Outset TGDR (stride 0x24): linkrm=19109, ojhous=32708, ojhous2=20158, onobuta=-32768
> - **P4:** `requestBgWarpGuarded` — if BG not ready in 180f, abort + fade + return to outdoor door; log `[Doors] enter … ABORT`
> - **H1:** Yw→`ywhead01` always; Ob→`oba_head`; Ym/Bm `head_from_params`; Ko1/2 name-locked
> - **H2:** Ls `companion_model=lshand.bdl`
> - **H3:** Zl `cloth.bdl` companion; P1a/P1b → `NPC_P1` + p1a/b_head (P2 kept for P2b)
> - **H4:** `[item] proc=TP_RUPEE` → `fopAcM_createItem` with WW low-byte 01–04 (= TP green/blue/yellow/red)
> - **H5:** ACT4–7=`qs.ah_state`; ACT8=`qs.pirates_ashore`; ACT9=`qs.aryll_taken`; ACTa/b=`qs.depart`
> Please critique guard timeout + WwAlways naming + whether ACT2 empty (=day-1 allow) matches tww.

#### User ruling (2026-07-19 — №35 H4 rupees + cross-pollination) — **⚠ PARTLY RESCINDED, SEE BELOW**

**⚠ CORRECTION (History, 2026-07-19 — this block’s rupee half was an INVERSION of the user’s actual ruling and is STRUCK.**
The user stated FIVE times, and it is FINAL: **rupees in WW spaces are WW RUPEES** — `Vlupy.arc/vlupy.bdl`, colour by anim frame per WW item id.
The TP wallet may be CREDITED (economy is a system), but the VISIBLE pickup object is WW’s. There is no TP-pickup carve-out.
Authority: live state №31 LAW + №36-C (implemented; `pickup_rupee` manifest flag, no proc names in code per №55).
The cross-pollination half (point 1) STANDS unchanged.)

~~1. Cross-pollination: discourages any cross-pollination — №31 stands.~~ → **STANDS (not struck): №31 Space-Purity Law applies to actors/props/spaces — no TP natives in WW spaces, ever.**
~~2. Rupees (H4): the 14 WW `item` rows simply add to the TP rupee counter (TP pickup actors).~~ **STRUCK — inverted the user’s ruling.**
~~3. Declined alternative: visual-purity path (WW rupee models granting TP rupees).~~ **STRUCK — this "declined" option IS the ratified law.**

Cursor: rupees are WW visuals granting TP wallet credit; do not invent a separate WW wallet, and do not spawn TP pickup actors in WW spaces.

#### Paste for History (2026-07-19 — №66 cover/place/lift + symmetric room + stranding)

> History №66 absorbed. Cursor shipped (`build_run.bat` green; caches wiped). №67 noted (will not share ungated/failing exes; current relwithdebinfo is the shareable one).
> - **A:** `beginDoorFade` + demo lock at queue (mid open-anim); enter/exit phases wait for `JUTFader::Wait` before load/unload/place; then lift.
> - **B:** enter waits `checkRoomDisp` (daBg 0x10) + mount ready; unload polls until room fully gone; `SetRoomNo`/`setRoomLayer` on place; `clearExitKnobForProc` on room unload so exit respawns.
> - **C:** stranding G-guard — room-lane last + !drawable / no exit prop ≥120f → forced `requestRoomLaneExit` + log.
> **USER VERIFY:** enter→exit→re-enter ×3; `№66` + `roomDisp=1`; no void; exit prop always.
> Please critique: Wait-only cover vs FadeOut; 120f strand threshold; keep room0 vs full exterior unload.

#### Paste for History (2026-07-19 — №68 ordered teardown)

> History №68 absorbed. Cursor shipped (`build_run.bat` green; caches wiped).
> - Exit: `forgetRoomLaneHandles` (disarm doorAttention, clear exit knob, `clearForBg`, delete+forget mount id, latch `s_roomLaneUnloading`) → `ensureRoomLaneUnloaded` → wait gone → place porch.
> - Skip exec/draw/attention/doors while latch set. `onRoomUnload` idempotent if already forgotten.
> - Twin doors = data (History unmapped `[KNOB00]`); no engine twin logic.
> **USER VERIFY:** enter→exit→re-enter→exit ×3, no AV; `№68 forget handles` + `№68 EXIT place`; single-press exit.

#### Paste for History (2026-07-19 — №69 place-before-Release)

> History №69 absorbed. Cursor shipped (`build_run.bat` green; caches wiped). №70–72 noted; pivot after exemplar.
> - Exit: soft-forget (attention/exit/clearForBg + latch) → **place Link at return_pos + `ClrGroundHit`/`CrrPos`** → **`Bgsp().Release(mpBgW)` + null** → delete mount → unload room.
> - Rule: never Release a dBgW the player is standing on.
> **USER VERIFY:** enter→exit→re-enter→exit ×3, no AV; log `№69 place-first` + `Released interior BgW`.

#### Paste for History (2026-07-19 — №73 arc-scoped model cache purge)

> History №73 absorbed. Cursor shipped (`build_run.bat` green; caches wiped).
> - `retainArcModels` on COMPLEATE; `releaseArcModels` before `resDelete` — purge `s_modelDataCache` + `s_pristineJ3dRaw` for that arc at live==0.
> - Shared arcs (Knob) keep cache while any mount still holds a retain.
> - Comment at rescanProviders corrected; sweep notes on other permanent assumptions left in mount.cpp.
> **USER VERIFY:** enter→exit→re-enter ×3; exit logs `№73 purged model cache for arc '…'`; re-entry logs `model-data cache +` (miss→reparse); room draws clean on entry 2+.

#### Paste for History (2026-07-19 — №74 warn-only + stage feasibility)

> History №74 absorbed. Cursor shipped (`build_run.bat` green; caches wiped).
> - Strand guard: **WARN-ONLY** — existence = `DoorDef.exitKnobSpawned` (not player radius); never auto-warp; logs drawable/exitReg/exitNear/player/host.
> **USER:** roll around Grandma's room — must NOT force-return.
>
> **STAGE FEASIBILITY (№71/72/75–78 — Cursor research):**
> - **Verdict: feasible.** Load path is data-driven: `/res/Stage/<≤7-char>/` via `setStageRes` — no compile-time stage registry. Warp UI list is optional compile-time.
> - **R_SP01 blueprint confirmed:** separate interiors stage vs village; rooms = houses (map_loader_definitions + getLayerNo_common).
> - **Reuse:** `onRoomObjectsReady` / registerRoomLane = YES (stage-gated). `ensureRoomLaneLoaded` keep-room-0 dual-load = NO for dedicated interiors stage — enter/exit become `setNextStage` (doors.cpp already calls it when `hostStage` differs on enter; **exit still same-stage place — must gain setNextStage back**).
> - **Save:** interiors/hosts = save-neutral STAG (0 slots). Dungeons = claim free `STAGE_MAX` indices for memBit (chests/keys/boss); never reuse vanilla. **Free-index count still open (№78).**
> - **Exemplar proposal:** new Outset interiors host (e.g. `R_WW01`); LinkRM = room 0 (native entry) or room 2 (match current exemplar); mount-on-ready keeps room-lane spawn path; exterior host later.
> Please critique names/save-neutral STAG values and author the thin stage shell when GO.

#### Paste for History (2026-07-19 — №81 Cursor ship: WwSave guard + R_DL01 exemplar)

> **№81 absorbed and shipped** (`build_run.bat` green; GPU caches wiped).
> - **IMMEDIATE GUARD:** `[WwSave] REFUSED` on native `onTbox` / `onSaveSwitch(stage)` / `onStageSwitch` / dungeon-item / boss stage writers whenever WW content is active (EXT_* lastBg or room-lane / EXT mount). Covers today's F_SP115=LANAYRU exposure and also refuses writes on `R_DL*` until the extension router lands.
> - **MOD-SIDE STAGE SHELLS = YES.** Layer A already overlays `files/res/Stage/…` (same path as thin `F_SP115/R02_00.arc`). Game tree stays untouched.
> - **LOCKED NAME:** `R_DL01` (≤7, neutral fork prefix; not `R_WW*`).
> - **SAVE-INDEX:** not one of the 9 for WW progression. Shell STAG placeholder = **15** (5-bit field needs a value); **progression home = Dusklight extension** (mirror `dSv_memBit_c`, versioned migration). Please design the router next (`R_DL*` → extension indices).
> - **EXEMPLAR LIVE (mod AppData):** `files/res/Stage/R_DL01/{STG_00.arc,R00_00.arc}` via `tools/ww_crew_restoration_skeleton/build_rdl01_shell.py`. LinkRM (`ext_bg1.ini`): `host_stage=R_DL01` `host_room=0` `host_pos=0,0,0`. `doors.ini` `[linkrm]` `lane=room` `host_room=0`.
> - **SYMMETRY:** enter/exit both `setNextStage` when hosts differ (`transport=stage`); same-stage room-lane path kept for bootstrap leftovers. Warp menu stub: region **Outset Interiors** → Link's House (`R_DL01` r0).
> - **History please:** remaining Outset interior room table on `R_DL01`, Nintendo spawn points for the warp menu, and the versioned WW memBit extension design. User verifying door loops + no auto-warp.

#### Paste for History (2026-07-19 — №83 deferred room-lane mount + fail reasons)

> History №83 absorbed. Cursor shipped (`build_run.bat` green; GPU caches wiped).
> - **Hook moved:** `onRoomObjectsReady` now runs **after** `roomInit` / `roomReLoader` (was beside the `BG_e` create request).
> - **Create:** room-lane path no longer requires `getPlayer(0)` (stage enter can ready the room first). Still requires play-scene layer — otherwise **PENDING** and `pollBgWarps` retries (give-up 300f).
> - **Fail log:** `[ExtNpcMount] №83 roomN create FAILED 'PROC' reason=… actorId=… roomNo=… host=(…) stage='…' playScene=0|1`.
> - **Lane re-bind:** `syncRoomLaneForCurrentStage` on stage-ready + WW-host room-ready — manifests with matching `host_stage` re-`registerRoomLane` (covers boot register during F_SP115).
> **USER VERIFY:** door/warp → R_DL01 → `№83 room0 mount 'EXT_BG1'`; mesh+collision; exit `transport=stage`. If FAILED, the `reason=` line names it.

#### Paste for History (2026-07-19 — №84 native stage change for foreign hosts)

> History №84 absorbed. Cursor shipped (`build_run.bat` green; caches wiped).
> - **Enter (host differs):** `cancelTransports` → `registerRoomLane` → `setNextStage(host, point=0, room, layer)` only. **No** `ensureRoomLaneLoaded` / `requestBgWarpGuarded` / room-create on the old play scene.
> - **Exit (from R_DL*):** same — `armStageExitRemount(EXT_BG0, return_pos)` then `setNextStage(exterior)`; remount runs in `onStageReady`/`poll` once play scene + player exist.
> - **Cut Actors warp button:** foreign host → setNextStage only (was requestBgWarp-before-setNextStage).
> - Room-lane path unchanged for **same-stage** rooms.
> **USER VERIFY:** `transport=stage … native setNextStage only` · overlay `Stage: R_DL01` · ALINK · PLYR (-255,0,1125) · `№83 room0 mount` · exit remount EXT_BG0.

#### Paste for History (2026-07-19 — №86 cross-stage wipe-only + playerInit + mount)

> History №85 absorbed. Cursor shipped №86 (`build_run.bat`; wipe `dawn_cache.db*` / `pipeline_cache.db*` after).
> **Log forensics:**
> - **205514 door:** `transport=stage` → OVERLAP0 → **no PLAY_SCENE for R_DL01**; stayed on F_SP115 (double enter). Demo lock + custom fade fought native wipe.
> - **203939 warp:** PLAY_SCENE + `room.dzr` 68B OK; **no ALINK**; mount `reason=no_play_scene` mid-create.
> **Engine:**
> - Cross-stage enter/exit: arm `setNextStage` immediately; `endDoorDemoLock` + `offFade`; **no** `beginDoorDemoLock` / `beginDoorFade`; debounce if next already armed.
> - `dStage_playerInit`: log SKIP (`player!=NULL`, startRoom, stageRoom, point, stage); on WW host if stale player + rooms match → clear ptrs and spawn.
> - Room-lane `createBgMountAtHost`: if `SearchByName(PLAY_SCENE)` misses mid-create, stay on current layer (do not fail `no_play_scene`).
> **USER VERIFY:** ALINK on R_DL01 · overlay `Stage: R_DL01` · PLYR · `№83 room0 mount 'EXT_BG1'` · exit remount EXT_BG0. If no Link: ferry `[dStage] playerInit SKIP` line.

#### Paste for History (2026-07-19 — №86b room-number reconcile)

> History №86 (log 211728) absorbed — mount/COMPLEATE proven; player guard = room-number branch.
> **Values found:** PLYR params `ff00a03f` → room bits **63** (TP interior copy). `playerInit` post-create writes `startRoom = params&0x3F`.
> **Cursor shipped:**
> 1. `roomLoader`: `setRoomNo(param)` **after** `init()`, before PLYR decode.
> 2. WW host: no player + startRoom≠stageRoom → reconcile `setRoomNo(startRoom)` and continue.
> 3. WW host: force PLYR `parameters` room bits to arrival room before create/`set`.
> 4. Mod data: `R00_00.arc` PLYR `ff00a03f`→`ff00a000` (bak `.plyr-room63-bak`).
> 5. Logs: `[dStage] playerInit enter|№86 reconcile|CREATE`.
> **USER VERIFY:** `playerInit CREATE` + ALINK · `Stage: R_DL01` · PLYR (-255,0,1125) · mount · exit remount.

#### Paste for History (2026-07-19 — №87 RARC stale name_hash = playerInit never called)

> History №87 absorbed (log 213054). Cursor shipped.
> **Root cause (not WW name collision):** `R00_00.arc` string = `room.dzr` but **entry `name_hash` still `0xaee2` = `shell.txt`** (expect `0xa015`). Index/`loadResource` still pulled Size 68; `JKRArchive::findNameResource` / `getStageRes` returned NULL → `roomLoader` skipped → no `playerInit` → no ALINK. Mount still ran (`roomReLoader`/ready path tolerates NULL dzr).
> **Fixes:**
> 1. Mod: patch hash `0xaee2`→`0xa015` (bak `R00_00.arc.hash-shell-bak`).
> 2. Engine PC: `JKRArchive::isSameName` falls through to `strcmp` when hash mismatches (class guard for hand-renamed RARC members).
> 3. Log: `[dScnRoom] phase_2 arc='…' roomNo=… roomInfo=0|1 syncRt=… stage='…'`.
> **USER VERIFY:** `roomInfo=1` · `playerInit CREATE` · ALINK · `Stage: R_DL01` · PLYR · mount · exit remount.
> **History note:** when renaming RARC members in-place, always recompute `name_hash` (or repack).

#### Paste for History (2026-07-19 — №89 destination-owned arrival demo + event G-guard)

> History №89 absorbed (EVLY ruled out; freeze = source DEMO/event outliving stage change). Cursor shipped (`build_run.bat`).
> **Engine root:** `doorCheck` set `mMode=DEMO` then `tryNativeWarp`→`setNextStage`; `reset()` only flags end; **`endProc` blocked while `isEnableNextStage`** → hollow event / freeze family.
> **Shipped:**
> 1. **Source:** if native warp armed next stage → clear INDOOR cmds, `mMode=WAIT`, **return 0** (no source DEMO). Log: `№89 doorCheck — stage-change warp; skip source DEMO event`.
> 2. **Dest arrival (enter + exit mirror):** arm on cross-stage; first player frame on dest → force-end residual → 18f `DEMO_DOOR_OPEN` (facing `spawn_ry` / `return_ry`) → **end same context** → 120f event G-guard (`forceEndDoorEvent` if still active).
> 3. **`dExtNpcMount_forceEndDoorEvent`:** cancel demo + `cancelStaff(ALL)` + `setCameraPlay(0)` + `event->remove()`.
> **USER VERIFY:** enter/exit R_DL01 — Link movable; fly mode OK. Logs: `arm arrival` · `BEGIN` · `END` · `G-guard clear` (or force-end). Wipe dawn/pipeline caches after build.

#### Paste for History (2026-07-19 — №90 pop + exit knob + lane-agnostic G-guard + warp point)

> History ★№90 absorbed (door path green; three residuals). Cursor shipped (`build_run.bat`). №91 parked until this verifies.
> **Shipped:**
> 1. **Population + exit knob:** BG COMPLEATE on WW host / room-lane arms `s_interiorBootstrapProc`; poll drains when player exists → `dExtNpcPopulation_spawnForBg` + `dExtNpcDoors_onInteriorBgReady` + `lastBgProc`. Log: `№90 interior bootstrap … population + exit knob`.
> 2. **Doors poll on `R_DL*`:** stage gate is island host **or** `dExtWwSave_isWwHostStage` (was F_SP115-only).
> 3. **Lane-agnostic G-guard:** `dExtNpcDoors_armArrivalGuard` (no demo) from `onStageReady` + warp foreign/region; door-lane full demo still wins if already armed.
> 4. **Warp point:** WW-host `setNextStage` uses/clamps **point 0** (never −1); arms G-guard.
> **USER VERIFY:** door → `src=census:` + `exit prop key=exit:…` + Grandma + exit; warp → `point=0` + G-guard clear. Then №91.
> **Note:** full manifest-driven warp menu (retire compile-time labels) still open as History proposal — exemplar path is live.

#### Paste for History (2026-07-19 — №94 world-gen spawn latches + self-heal)

> History ⚠№94 absorbed (empty world after void-fall/reload). Cursor shipped (`build_run.bat`).
> **Shipped:**
> 1. **`dExtNpcWorld_generation` / `bump(reason)`** — clears door + population latches; logs `№94 bump gen=N (…)`.
> 2. **Bump sites:** every `onStageReady` (play-scene Create) + `dStage_restartRoom` (death/void restart).
> 3. **Keyed latches:** `s_knobsSpawnedGen` / `s_spawnedBgProcGen` — spawn only skipped if same gen.
> 4. **Self-heal:** latched this gen + zero live outdoor knobs / zero `census:` mounts ⇒ clear + respawn + warn log.
> **USER VERIFY:** fall/restart/reload → Outset doors + census return. Then №91.
> **Parked:** №91 door owns `cutEnd`; History dzb wall fall-through research.

#### Paste for History (2026-07-19 — №91 knob owns cutEnd + №33 native talk wrap)

> User clarified queue **№90 / №91 / №33** (not 91–93). №90 already verified (“Outset works!”). Cursor shipped (`build_run.bat`).
> **№91 (TP mechanism + WW content):**
> 1. Shared `DoorK10` load → bind `DEFAULT_KNOB_DOOR_F_OPEN` / `_B_OPEN` (front/back via `frontCheck`).
> 2. `setEventPrm` → `doorCheck` orders real event (no hollow DEMO / no immediate №53 warp when idxs valid).
> 3. Mount owns `SHUTTER_DOOR` staff: open actions play manifest `door_open_bck` on controller/`DoorDummy`; **`cutEnd` on completion**; **no** `Z2SE_OBJ_KNOB_DOOR_*` (№31).
> 4. On `endCheck` → `event_reset` → `tryNativeWarp(..., openAlreadyDone=true)` (skips `playAnimNearest` / demo-lock / 28f hold).
> 5. DoorK10 miss → prior №53 CANDOOR fallback remains.
> **№33:** mount dialogue stays on `dALBWDialogue_c`; TrigA only `orderSpeakEvent`; window opens on `checkCommandTalk`; hold `onGameStatus(2)` while `mTalkEventActive` until dismiss/`event_reset`.
> **USER VERIFY:** door open/step + WW swing → stage change → movable; talk = lock until A dismiss. Logs: `№91 DoorK10` · `knob demo BEGIN/END` · `№91 post-cutEnd OPEN`. (GPU caches wiped by Cursor per build-fps-guidelines.)
> **Follow-up (not this ship):** dest arrival B_OPEN mirror (still №89 18f demo); History warp-menu-from-manifests proposal; dzb fall-through research.

#### Paste for History (2026-07-19 — absorb №98 / №99 / №100)

> History №98–№100 absorbed into live state (no engine ship this note).
> **Queue (ordered):** **№100** Outset arc released while mount live — log release caller; destroy-or-retain before release; refuse purge if live mount refs arc. **Then №99 R1** neutralise compile-time warp labels + **R2** manifest-driven warp enum (before more room MapEntries). **Then №98** register identity R_DL01 mounts `GLOBAL_e` (NULL mtx) — climb/ledge test; Outset same after relocation.
> **Corrected:** keep-cache vs purge “contradiction” is not a bug (final N→0 release is silent then purge).
> **№97** remains shipped (STG×6 + OPEN fix); remaining rooms wait on №100/R2 ordering.

#### Paste for Cursor (2026-07-20 - No.106 COMMIT ORDER, user-approved: do 1-5, rename FIRST)

> **All six interiors are in with actors/NPCs, and M5b PASSED (folder-absent smoke, validly run for the first time). User approves committing. Do these in order:**
> **1. RENAME FIRST - `include/d/d_ext_ww_save_guard.h` -> `d_ext_save_guard.h`, and update its SIX includers (`d_com_inf_game.cpp/.h`, `d_ext_npc_doors.cpp`, `d_ext_npc_mount.cpp`, `d_stage.cpp`, `dusk/ui/warp.cpp`). THIS IS TIME-CRITICAL: the header is still UNTRACKED, so renaming now costs one rename; after the first commit the `ww` filename lives in git history permanently (and history is far harder to purge than a working tree).**
> **2. ADD the untracked engine sources** - `include/d/d_ext_mod_flags.h`, `d_ext_npc_doors.h`, `d_ext_npc_population.h`, the renamed save-guard header, and `src/d/d_ext_mod_flags.cpp`. **Without these the tree will not compile: 54 tracked files already include them.**
> **3. ADD tools + docs**, including `tools/ww_crew_restoration_skeleton/greplist.txt` (auditor recommended committing so the gate travels; History agreed in No.55) and `extract_bmg.py`.
> **4. EXCLUDE the strays**: `.cursor/`, `tmp_r00_names.txt`, `tools/_symcrash.*`, `tools/demo_cut_content/err.txt`, and the scratch `_dbg_dzr.py` / `_list_arc.py` if unwanted. **Also HOLD `tools/ww_crew_restoration_skeleton/dialogue/outset_quests.txt` pending the user's call - it is game quest text with a WW place-name filename, which stretches the config-text exemption.**
> **5. GATE THE BUILT EXE before pushing** (No.67: binaries outlive source fixes - check the artifact, not just the source).
> - Note: source is otherwise gate-clean; the only remaining WW token is `"Arylls"` inside a COMMENT (compiles away; auditor measured 0 in the binary). Comment-class ruling still with the user.
> - **AFTER THE COMMIT: Outset relocation + WW lighting is the next major work.**

#### Paste for Cursor (2026-07-20 - No.105 rooms 1-5 had the WRONG COLLISION ROOM ID; History fixed it in data)

> **Your No.104 fixes landed: no crashes, all six rooms exist with actors/NPCs, and Ordon/Outset keep their inhabitants across warps. Remaining symptom: only room 0 renders mesh + collision; rooms 1-5 are black with no floor (props/NPCs visible).**
> - **ROOT CAUSE (source-verified): TP reads a polygon's room from the COLLISION FILE - `cBgD_Grp_t.m_room_id` (`d_bg_w.h:92`), walked parent->parent in `d_bg_w.cpp:894`. It is data, not a setter. **WW dzb files carry WW's own room numbering:** LinkRM claimed room 0 (= our room 0, which is the ONLY reason room 0 ever worked); **Ojhous2R1 claimed room 1 while mounted in TP room 2; Ojhous2/Omasao/Onobuta/LinkUG all claimed room 0 while mounted in rooms 3/4/5/1.** Mismatch => room-filtered collision rejects the polys => Link falls.
> - **FIXED IN DATA (no rebuild): History re-tagged each interior's dzb group tree to its TP room (LinkUG->1, Ojhous2R1->2, Ojhous2->3, Omasao->4, Onobuta->5; LinkRM already 0). 0xFFFF 'no room' groups untouched; `.roomid-bak` per arc. Rooms 1-5 should gain floors on the next run with no code change.**
> - **IF THEY STILL RENDER BLACK after that, the remaining half is yours: the room MESH is drawn by our BG mount, not by `daBg_c`. Props/NPCs already draw in rooms 1-5, so per-actor room culling works - only the BG model lags. Check the mount's draw is not room-culled and that the room's status/visibility is set the way `daBg_c` does (`onStatusFlag(roomNo, 0x10)` mirror, No.70).**
> - **General rule now recorded: WW room data is authored for WW's own room numbering. Any WW space mounted into a TP room slot needs its collision room ids re-tagged. History will fold this into the adaptation tool once the room map is stable, so it stops being a per-arc patch.**

#### Paste for Cursor (2026-07-20 - No.104 rooms 1-5 are INVISIBLE, not broken; exterior re-entry skips population)

> **Room table fix worked - and Sturgeon's room is NOT broken. The log shows it fully loading: `dScnRoom phase_2 arc='R02_00' roomNo=2 roomInfo=1` -> `room2 mount 'EXT_BG3'` -> `BG COMPLEATE EXT_BG3 host=(0,0,0) anchor=(0,0,0)` -> population spawns correctly (`Aj1` = Sturgeon at (-8.5,-20,106.9), his Plants, his Otana shelves). The content is all there at the right coordinates.**
> - **FAULT 1: rooms != 0 are never ACTIVATED.** Room 0 works only because it is the default active room. Rooms 1-5 mount and populate, but TP never marks them visible/solid - so the user sees black void and falls (`Room: 1`, y=14625, velocity -52.8). **Please set the entered room's active/visible state via TP's room control (room switch / status flags / `dComIfGp_roomControl`) as a native room change would - the mirror of the bookkeeping `daBg_c` clears on teardown (No.70). Acceptance: enter Sturgeon's and SEE the study, with a floor.**
> - **FAULT 2: returning to the exterior skips population.** After `bump gen=8 (enter 'F_SP115')` -> `room0 mount 'EXT_BG0'` -> `BG COMPLEATE EXT_BG0`, **the only spawn is the exit knob (`src=door:exit:ojhous`) - zero census rows.** Generation bumped, so the No.94 check is not covering exterior re-entry. **Make population re-run on generation mismatch at every mount COMPLEATE, and confirm the No.94 self-heal (BG complete + census rows > 0 + zero spawns => re-run + log) is armed on this path.**
> - Both are transition bookkeeping (the No.65/69/73/83/88/89/94/100 family). **Neither is a data problem - History's room arcs, manifests and census all verify correct in this same log.**

#### Paste for History (2026-07-20 — №104 room activate + exterior pop)

> History №104 absorbed. Shipped (`build_run.bat` + GPU cache wipe).
> **(1) Activate:** `activateWwHostRoom` = `offStatusFlag(0x08)` + `onStatusFlag(0x10)` (mirror `daBg_Create`) + `setStayNo`. Called on BG COMPLEATE, bootstrap, room-lane enter/ensure-loaded. WW shells have no retail daBg ⇒ rooms 1–5 never got drawable/solid.
> **(2) Exterior pop:** EXT_BG0 COMPLEATE now arms the same bootstrap path → `spawnForBg` + `spawnKnobs`. №94 self-heal also fires when **unlatched** + live census/doors == 0 (post-bump skip).
> **USER VERIFY:** Sturgeon/rooms 1–5 visible+floor (`№104 activate roomN`); exit Outset restores folk (`№104 exterior bootstrap` / self-heal).

#### Paste for History (2026-07-20 — №105 crawl unusable: research + plan, no code)

> User: crawl unusable after hold-A retirement; suspects roll (notes roll needs movement). **Research only — no engine change this pass.**
> **Root cause:** free-ground hold-A was the working crawl entry. Retirement assumed native `BUTTON_STATUS_ENTER` from crawl holes. **False in this tree:** `setDoStatus(ENTER)` only at wall_code==6 (ladder/swim path). Roll is `UNK_121` when stick > ~0.75 — standing should not roll. `adapt_dzb` does **not** remap WW crawl attrs → TP wall codes (through-clear only). W1 "crawl holes armed" note was aspirational.
> **Plan (wait GO):** P0 probe DoStatus/wall_code at Outset holes → P1 History names attr→wall-code, Cursor adapt_dzb → P2 interim gated hold-A (NONE + stick idle + not door/talk) if P1 lags → P3 accept crawl-in without breaking roll/doors/talk.
> **Ask History:** WW crawl poly attribute → TP wall code that should arm crawl Enter (or the real retail crawl prompt path).

#### Paste for Cursor (2026-07-20 — №102 empty R_DL01: the RTBL pointer array is garbage)

> **Symptom: entering R_DL01 gives an empty world (no room, no Grandma, no doors) — and NO crash.** The log names it by omission: `№94 bump gen=5 (enter 'R_DL01')` → six `room-lane register EXT_BG1..6 → rooms 0..5` (History's data reads fine) → `PLAY_SCENE` + `CAMERA` → **then nothing: no `ROOM_SCENE`, no `dScnRoom phase_2`, no `room.dzr`, no mount, no population.** The last working run (220615) had `ROOM_SCENE` + `dScnRoom phase_2 arc='R00_00' roomInfo=1` right there. **Regression point = the stage growing 1 → 6 rooms.**
> - **ROOT CAUSE (byte-proven): the RTBL pointer array holds invalid offsets.** TP `R_SP01` (RTBL n=8, table@0xb8) points to clean 12-byte records laid out **after** the array: `01 00 00 00 | 00 00 01 18 | 01 00 00 00`, then `…0119…`, `…011a…`. **Ours (n=6, table@0xe8) points at 0x18/0x20/0x28 — backward into the dzs CHUNK-HEADER DIRECTORY — so the "records" literally contain chunk tags: `4d554c54` = "MULT", `5243414d` = "RCAM".** ⇒ the play scene resolves no rooms ⇒ no ROOM_SCENE ⇒ nothing downstream runs.
> - **FIX: rebuild RTBL in TP's layout — `[n × u32 pointer][n × 12-byte record]`, pointers = dzs-relative offsets to records that FOLLOW the pointer array.** TP's reference bytes above; only one field visibly varies per room. **Please add a build-time assert: any RTBL pointer that resolves inside the chunk directory ⇒ refuse + log.** That would have caught this before the playtest.
> - **History aligned the data side this pass:** you fixed room 0's PLYR params `0x3F(63) → 0x00`; my generated rooms still had `0x3F`, so rooms 1-5 are now `…a001`…`…a005` (low 6 bits = own room), backups `.room63-bak`. **Note: TP's own `R_SP01` ships 0x3F, so 63 may be an "inherit" sentinel — we are following your proven value, not the shipped one.**

#### Paste for History (2026-07-20 — №102 RTBL absolute pointers)

> History №102 absorbed (empty R_DL01 = RTBL ptrs into chunk directory). Fixed in `grow_rdl01_stg.py` + regenerated AppData `STG_00.arc` (`.pre102-bak`). No exe rebuild (mod data only).
> **Cause:** grower wrote payload-relative offsets (0x18…); engine adds dzs base (`param_3`) ⇒ pointers file-absolute. Relative values landed in the chunk directory ("MULT"/"RCAM").
> **Fix:** `[N×u32 abs ptr][N×8 roomRead_data][N×u8 load]` with file-absolute ptr/`m_rooms`. Records are **8 B** (struct size); History's "12-byte" dump window included the next record start. Assert: any ptr/`m_rooms` inside chunk directory ⇒ refuse write.
> **Verified bytes:** ptr[0]=0x100 → `01000000 00000130` … ptr[5]=0x128 → `…0135` (matches TP shape).
> **USER VERIFY:** enter R_DL01 → `ROOM_SCENE` + `dScnRoom phase_2` + mount/pop (not empty).

#### Paste for Cursor (2026-07-19 — №100 CRASH: the island arc is released while still mounted)

> **First, a correction that saves you a wrong hunt: the reported "release says keep-cache-shared but purge drops it anyway" is NOT a bug.** `releaseArcModels` (`d_ext_npc_mount.cpp:1623-1641`) logs `live=N (keep cache — shared)` only while the count is > 0; the **final** release (N→0) erases and purges **with no release log line**. So `release 'Ko' live=1` + `purged 'Ko'` = two releases (2→1 logged, 1→0 silent). Same for Ptubo/Vlupy/Knob/Oyashi/Kn/WwAlways. **Purge is refcount-gated, and there are only two call sites, both inside `releaseArcModels`.**
> - **THE REAL BUG IS `Outset`: models cached (`bg:Outset/model.bdl` +2) → `№73 retain arc 'Outset' live=1` → `№73 purged model cache for arc 'Outset' (models=3 pristine=0)`. A purge implies last-release or no-retain-recorded ⇒ SOMETHING RELEASED THE ISLAND'S ARC WHILE THE ISLAND WAS STILL MOUNTED AND DRAWING.** J3D data freed → `PLAY_SCENE` create (name=11) → **AV at a HEAP address (not null) = dangling pointer into freed model data.** Same family as №69/№73/№83/№88/№94: teardown ordering across a transition.
> - **(1) Log the release CALLER** — add call-site/context to `releaseArcModels` so "who released Outset, and when" is answered by the next run rather than inferred.
> - **(2) Never release a mount's arc while the mount lives** — on stage change the exterior must either keep its retain, or be fully destroyed (draw/actor refs gone) **before** the arc is released. Same order rule as №69 (place → release → free).
> - **(3) Safety assert: refuse + log any purge for an arc a live mount still references.** Converts this class from a crash into a log line.
> - Secondary, likely benign: BG arcs purge with `pristine=0` vs NPC `pristine=N` — BGs do not stash pristine raw copies (that path exists for the №50-C BMT re-parse). Explained, not a defect.
> - **SCHEDULING (auditor's forward flag, ratified): №97 wires five more rooms. If each lands as a labelled `MapEntry`, that is up to six WW labels in the exe, most invisible to a flat greplist exactly like "Link's House". ⇒ DO R2 (manifest-driven warp enumeration) BEFORE the remaining rooms get warp entries — cheapest moment, and it holds the exe label count at one.**

#### Paste for Cursor (2026-07-19 — №99 REHOME PLAN: warp labels leaked into the exe; manifest enumeration is the fix)

> **The containment gate is failing again. `include/dusk/map_loader_definitions.h:607` now has `RegionEntry("Outset Interiors", { MapEntry("Link's House (interior)", "R_DL01", …) })` in a TRACKED, MODIFIED file — it commits on the next push.** No blame: it was added to make the warp testable, and `R_DL01` itself is properly neutral. But every future WW interior added this way is another failure.
> - **The auditor's key point, worth internalising: the second label CANNOT BE GATED. "Link's House" is also a real TP location (same file, line 109 — Ordon), so greplisting it would fire forever on TP's own entry. ⇒ the gate proves absence of UNAMBIGUOUS WW tokens; it cannot prove absence of WW labels TP also uses. Detection has a ceiling; only prevention by construction clears it.**
> - **R1 (you, immediate — unblocks committing): neutralise both labels.** Simplest form with no WW semantics: `RegionEntry("Mod Stages", { MapEntry("R_DL01 r0", "R_DL01", {{0,{0}}}) })`. Warp stays testable, gate passes, one edit.
> - **R2 (you, THE ACTUAL FIX — removes L-1 and L-2 permanently): manifest-driven warp enumeration.** You already register providers from manifests at boot; extend that to feed the warp UI — per BG manifest read `host_stage`, `host_room`, `warp_label` (+ points from the authored PLYR ids) and populate a runtime region named from the mod folder. **Labels then live mod-side ⇒ zero WW strings in the exe by construction, ambiguous ones included, and every future island/interior appears automatically as data.**
> - **R3 (you, cosmetic, removes a risk class): rename `include/d/d_ext_ww_save_guard.h` → `d_ext_save_guard.h` (+ includes).** Auditor tested it clean (0 hits ASCII and UTF-16) so it is naming drift, not a breach — but a `ww` token in a shipping-source filename can reach the binary via `__FILE__`/assert macros.
> - **R5 (hygiene, either lane): `tmp_r00_names.txt`, `tools/_symcrash.*`, `.cursor/` are untracked strays that would commit — gitignore or remove.**
> - **R6 (doctrine, now recorded): any PLAYER-VISIBLE STRING describing WW content must come from DATA, never source.** Same move as save-neutral stages and the E1 exact-match resolver: prevent by construction rather than detect after the fact.
> - **R4 is a USER ruling (open since №55): comment-class.** L-4 empirically confirms comments are inert; History's proposal remains comments allowed, string literals gated, with the gate scanning literals rather than raw source.

#### Paste for Cursor (2026-07-19 — ⚠№98 ladders/ledges: WW spaces are registered as MOVING OBJECTS, not world)

> **User: Outset lost WW-level collision interactivity — ladder not climbable, Link no longer catches ledges; interiors suspected too. Source-verified cause:**
> - **TP's own room collision:** `mpBgW->Set(dzb, cBgW::GLOBAL_e, NULL)` (`d_a_bg.cpp:220`) — **world collision, no matrix.**
> - **Our WW mounts:** `Set((cBgD_t*)dzb, cBgW::MOVE_BG_e, &a->mBgMtx)` (`d_ext_npc_mount.cpp:2129`) — and **every other `MOVE_BG_e` user in TP is a moving object** (ice blocks, `obj_kgate`, `lv3WaterB`, `b_zant_sima`, `bg_obj`). **⇒ to the player, Outset and its interiors are one giant MOVING OBJECT, not terrain.**
> - **Why that breaks climb/ledge: `daAlink_c` branches on this class all over** — `ChkMoveBG` / `ChkMoveBG_NoDABg` at `d_a_alink.cpp:3885, 5484, 9289, 19086, 19113, 19663` (ground checks, ride logic, magnetic lines, start modes). Ledge-grab / wall-climb / ladder attach are world-collision behaviours.
> - **Why we did it:** the matrix was needed to offset WW data into a foreign stage's space (№22). `GLOBAL_e` takes a NULL matrix — no offset possible.
> - **⇒ RELOCATION IS THE FIX, and it is now a GAMEPLAY requirement rather than tidiness: a WW space in its own stage sits at `host_pos=0,0,0` / `anchor=0,0,0`, so the matrix is IDENTITY and the space can register `GLOBAL_e` exactly like TP's rooms.**
> - **ONE-LINE TEST AVAILABLE NOW: the R_DL01 interiors are already at identity — register those mounts `GLOBAL_e` (NULL matrix) instead of `MOVE_BG_e`. Prediction: ladders/ledges/climb return inside Grandma's house.** If it holds, Outset gets the same fix on relocation. Keep `MOVE_BG_e` only for genuinely moving WW collision.

#### Paste for History (2026-07-19 — №97 interiors rollout: STG×6 + door OPEN fix)

> History №97 absorbed (six rooms authored; four Cursor items). Shipped (`build_run.bat` + GPU cache wipe).
> **(1) STG grown (AppData):** `grow_rdl01_stg.py` → `R_DL01/STG_00.arc` with `room0..5.dzs` (FILI clone) + **RTBL n=6 / MULT n=6** (rooms 0–5 identity). Backup `STG_00.arc.pre97-bak`. R01–R05 arcs unchanged.
> **(2) Transport:** enter path already `dest.hostStage`/`hostRoom` — no EXT_BG1 hardcode (exit remount still EXT_BG0 island — intentional).
> **(3) Pop + exit knob:** №90 COMPLEATE bootstrap already per interior BG on WW host / room-lane.
> **(4) Door OPEN:** root cause — `knobDoorOpenAnimDone` treated “not started” as done ⇒ `cutEnd` before WW BCK. Fixed; log `№97 door OPEN anim '…'`.
> **USER VERIFY:** each Outset house door → correct room N; opening swing visible; census/exit where authored. Logs: `host='R_DL01' room=N` · `№90 interior bootstrap` · `№97 door OPEN anim`.
> **Parked:** walkthrough A_mori/Cave09/Pjavdou migrate later (no door demo).

#### Paste for History (2026-07-19 — №100 + №99 R1/R2 + №98)

> History queue order honored (№100 → №99 R1/R2 → №98). Shipped (`build_run.bat` + GPU cache wipe).
> **№100:** `releaseArcModels(arc, reason, dying)` logs `caller=`; refuses purge if another live mount still refs the arc (`liveMountRefsArc`); BG delete clears `mpBgW` + `mpBgModels` before release. Call sites: `delete-bg` / `delete-npc`. Keep/purge “contradiction” left as History ruled (final N→0 silent).
> **№99 R1:** removed `RegionEntry("Outset Interiors", …)` from `map_loader_definitions.h`.
> **№99 R2:** Warp UI section from provider manifests (`warp_label=` + modFolder); no compile-time WW labels.
> **№98:** identity WW-host mounts (`host_pos`/`anchor` zero) → `cBgW::GLOBAL_e` + NULL mtx; all `Move()` paths skip when `mBgGlobal`. Log `№98 BG GLOBAL_e (world)`.
> **USER VERIFY:** (1) door↔island stage change — no AV; grep `№100 release` / refuse-purge. (2) climb/ledge on Outset + interiors. (3) Warp menu = manifest labels only. **Outset exterior non-identity relocation** still History (GLOBAL needs world coords).

#### Paste for Cursor (2026-07-19 — №97 INTERIORS ROLLOUT: History data complete, your four items)

> **Door demo confirmed working by the user (Link opens, steps through, door closes behind). Trivial residual: the door's OPENING animation does not play — closing does.**
> - **ALL SIX ROOMS ARE AUTHORED AND VERIFIED. Room map (room = bgN − 1, identity anchor so WW room-local coords ARE stage coords): `R_DL01` room 0 = EXT_BG1 LinkRM (working exemplar) · 1 = EXT_BG2 LinkUG · 2 = EXT_BG3 Ojhous2R1 (Sturgeon) · 3 = EXT_BG4 Ojhous2 (Orca) · 4 = EXT_BG5 Omasao (Mesa) · 5 = EXT_BG6 Onobuta (Abe & Rose).**
> - **Delivered: `R01_00.arc`…`R05_00.arc` built from the proven 68-byte skeleton, each carrying Nintendo's own PLYR spawn** — LinkUG (349,0,−34)@−16384 · Sturgeon (546,−20,327)@−32768 · Orca (1,0,626)@−32768 · Mesa (127.3,0,625)@−32768 · Onobuta (301.7,0,620.3)@−32768 — all re-parsed and verified. **Manifests `ext_bg2..6.ini` bound to `host_stage=R_DL01` + `host_room`, `host_pos=0,0,0`, `anchor=0,0,0`. No further History data needed.**
> - **YOUR FOUR ITEMS: (1) grow `STG_00.arc` — `room1.dzs`…`room5.dzs` FILI entries + `RTBL`/`MULT` covering 6 rooms; TP's own 6-interior stage `R_SP01` uses **RTBL n=8 / MULT n=6** as the reference shape. (2) confirm the native stage-change transport is driven purely by manifest `host_stage`/`host_room` — nothing hardcoded to EXT_BG1 — so all five new rooms work by data alone. (3) per-room population + exit knob (the №90 hook applied per mount). (4) the door OPENING anim.**
> - **WALKTHROUGH DEVIATION (user flagged): A_mori (BG9), Cave09 (BG10), Pjavdou (BG8) do NOT belong in the interiors stage** — outdoor/natural spaces reached by walk-through gates, and TP keeps caves/grottos as their own stages too. **They need the same native stage-change transport but WITHOUT the door demo: plane-cross → fade → change → place → release (№60 walkthrough class). Leave them on the current working lane until the interiors rollout is proven, then migrate.**

#### Paste for Cursor (2026-07-19 — ⚠№94 EMPTY-WORLD BUG: spawn latches survive world reloads)

> **User: fell through the floor near a wall → respawned at the door with NO actors and NO door → walking out fell into void → second attempt returned to Outset with EVERY DOOR AND ACTOR GONE.** Seventh instance of the state-outlives-a-transition family — and the worst, because it strands the player in an empty world.
> - **CAUSE 1 — DOORS: `s_knobsSpawned` (`d_ext_npc_doors.cpp:74`)** is set true after the props spawn and reset **only** inside `ensureLoaded()` when the **mod folder** changes — the function early-returns for the same mod (`if (s_loaded && strcmp(s_loadedMod, island.modFolder)==0) return;`). **Once doors spawn in a session that latch never clears, so any world reload skips door spawning.**
> - **CAUSE 2 — POPULATION: `s_spawnedBgProcs` (`d_ext_npc_population.cpp:39`)** is session-lived and cleared only via `dExtNpcPopulation_clearForBg` from mount-delete paths. **A respawn that does not run those deletes leaves the proc flagged as spawned, so the census never re-runs.**
> - **FIX (three parts): (a) key spawning to a WORLD GENERATION** — one counter bumped on every stage change, room load and player respawn/restart; spawn logic records the generation it ran for and re-runs on mismatch. **(b) explicit clears on the restart path** (death / void-fall / `getRestartRoom*`) for both latches. **(c) SELF-HEAL net** so the class is never fatal again: island BG COMPLEATE + doors.ini non-empty + zero door props alive ⇒ respawn + log; BG COMPLEATE + census rows > 0 + zero census spawns ⇒ re-run + log.
> - **Also from the same pass:** the door not animating open/close on entry is **expected — that is №91, not yet built** (no action). The **fall-through near a wall is History's research item** (dzb attribute-clearing review vs TP Link's larger body).

#### Paste for Cursor (2026-07-19 — №91 full door animation: TP already implements WW's exact pattern)

> **User wants the real thing: Link opens the door, steps through, it closes behind, and the mirror on the interior side. Research result — NOTHING needs porting, because `d_a_door_knob00.cpp` (`daKnob20_c`) is TP's implementation of WW's `d_a_knob00` pattern.**
> - **TP's mechanism:** `attention_info.flags = fopAc_AttnFlag_DOOR_e` (door-class prompt) → `eventInfo.setArchiveName(getEvArcName())` + `setEventId()` registering **`DEFAULT_KNOB_DOOR_F_OPEN` / `DEFAULT_KNOB_DOOR_B_OPEN`** (+TALK variants) → `setEventPrm()` orders it (`setEventId`, `setMapToolId`, `onCondition(4)`) → `demoProc()` switches on `getDemoAction()`, open actions call `openProc(...)`, and on completion **`dComIfGp_evmng_cutEnd(...)` — the DOOR ends the cut** (+ `event_offHindFlag(1)`), with SFX `Z2SE_OBJ_KNOB_DOOR_CLOSE` / `Z2SE_OBJ_IMPAL_DOOR_OP`.
> - **WW's `d_a_knob00` is the same shape:** front/back check (`mFrontCheck` → `m2C6` = 7 or 8) selects `mEventIdx[...]`, then `eventInfo.setEventId(...)`. **Both games: door orders a named event → event drives Link → door ends the cut. Front/back maps 1:1 with the facing we already compute.**
> - **IMPLEMENTATION (№60 doctrine, TP mechanism + WW content): our WW Knob prop registers `fopAc_AttnFlag_DOOR_e` and orders TP's `DEFAULT_KNOB_DOOR_F_OPEN`/`_B_OPEN`, while WW's `dooropenadoor.bck` plays on `DoorDummy` (already working). Link's open/step-through comes FREE from TP's event; the door calls `cutEnd`, THEN the stage change fires. Arrival = mirror: destination door orders the back-open/step-in cut, WW door closes behind, `cutEnd`, control released.**
> - **STRUCTURAL BONUS worth the refactor: in TP's pattern THE DOOR OWNS `cutEnd`** — begin and end in one actor. That makes the ownership invariant automatic and **retires the ad-hoc `playAnimNearest` + manual demo lock** that produced №88/№89.
> - **PURITY (№31): do NOT play TP's `Z2SE_OBJ_KNOB_DOOR_*` inside WW spaces** — WW door SFX via the audio lane, or silence until adapted. Event/animation machinery is a system (fine); sound is content (not fine).
> - **Sequencing: land №90 first** (population + exit knob + lane-agnostic G-guard + real warp point) so the loop closes; **then this pass, which also deletes code.**

#### Paste for Cursor (2026-07-19 — ★№90 DOOR→INTERIOR WORKS★ + three residuals)

> **№89 confirmed working — Link enters Grandma's house through the Outset door, into our own stage, AND MOVES:** `№89 arrival demo BEGIN … frames=18` → `force-end event (arrival-end)` → `arrival demo END … G-guard 120f` → `event G-guard clear — control free`. Your `dScnRoom` probe also shows `phase_2 arc='R00_00' roomNo=0 roomInfo=1 syncRt=0` — **the №87 NULL is gone.**
> - **RESIDUAL 1 — only the room spawns: no Grandma, no furniture, NO EXIT DOOR (hence no way back).** Evidence: `[Spawn] src=room proc=EXT_BG1` appears, but **zero `src=census:` lines and no `[Doors] exit prop key=…`** anywhere in the R_DL01 section. **Please run `dExtNpcPopulation_spawnForBg` and the exit-knob spawn for a BG mounted in a WW HOST STAGE, keyed off that mount's own COMPLEATE** — they appear to be gated on F_SP115 / the retired room-lane. Data is already correct and stage-agnostic (`population=interior_placements.csv`, `population_stage=LinkRM`).
> - **RESIDUAL 2 — the warp lane requests `point=-1`.** Same log: door path = `playerInit CREATE point=0` (History's authored spawn); warp path = **`point=-1`**, which in `dStage_playerInit` means *use the saved RESTART position*, not a spawn point. Root: R_DL01 has no warp-menu entry supplying a valid point.
> - **RESIDUAL 3 — the event G-guard is door-lane-only.** It runs inside the arrival demo, so a **warp arrival begins no demo and therefore gets no watchdog** — which is exactly why the freeze survives on that lane. **Make the guard lane-agnostic: any arrival into a WW host stage arms it; event active ~120f with no owner ⇒ force-end + release + log.**
> - **HISTORY PROPOSAL — make warp entries DATA, not compile-time.** `map_loader_definitions.h` is a compile-time table: every new WW interior would need an exe edit **and would compile WW-side labels into the binary**. **Better: enumerate WW stages for the warp menu from the mod manifests** — the fields already exist (`host_stage`, `host_room`, `warp_label`, points from the authored PLYR ids). Covenant-clean and it scales to every island automatically. **Exemplar needs only: stage `R_DL01`, room 0, point 0, label from `ext_bg1.ini warp_label`.** History will supply per-stage point lists as we grow.

#### Paste for Cursor (2026-07-19 — №89 destination-owned arrival demo: the freeze is the door lock outliving the stage change)

> **Control test done: the freeze PERSISTS after History neutralised the shell's `EVLY` chunk ⇒ EVLY was not the cause** (my candidate was wrong — saying so plainly). Patch retained as hygiene since TP's own `R_SP01` has no event layer; backup `STG_00.arc.evly-bak`. **⇒ The cause is the remaining candidate: the №53-A door demo lock (`DEMO_DOOR_OPEN`) begun in the OLD stage on the A-press. Its release path dies with that stage, so nothing in R_DL01 ends it — event stays open, Link frozen, editor refuses fly mode.**
> - **USER CALL (ratified): hand the demo to the destination room. This is also beat 6 of the ratified №60 spec (arrival walk-in) — the fix and the polish are the same work.**
> - **THE INVARIANT (this is the SIXTH bug of one family — №65 transport, №69 collision, №73 cache, №83 mount, №88 event): whatever BEGINS a demo/lock/registration MUST be the thing that ENDS it. Never expect state begun in the old context to close in the new one.**
> - **SPEC:** **(1)** for STAGE-CHANGE transports, do **not** begin a door demo lock in the source stage — the fade + load already cover it. **(2)** on the DESTINATION's first frame after transport completes, if the arrival is door-class (the door key is already carried), the destination **begins** a short arrival demo: lock control → Link door-open/step-in at the authored spawn (facing `spawn_ry`, Nintendo's own) → **explicitly END the demo/event and release control**. Begin and end in the same context. **(3) MANDATORY EVENT G-GUARD:** if an event is still active ~120 frames after arrival with no demo owner, **force-end, release control, log** — the player must never be left frozen; a missing animation always beats a soft-lock. **(4)** mirror on EXIT so the porch arrival owns its own demo.
> - **For the record: the pending event is TP's own, not WW's.** No WW event data is involved — we opened an event and nothing closed it.

#### Paste for Cursor (2026-07-19 — ★№88 LINK IS IN R_DL01★ — residual: an event that never ends)

> **IT WORKS. Overlay: `Stage: R_DL01`, `Room: 0`, `Entry: 0`, `Position: -255.0, 0.0, 1125.0`, `Angle: -32768` — exactly the PLYR values History authored — with Grandma's house rendering around him. Neutral mod-side stage → TP room skeleton → Nintendo spawn → WW room mounted → player created. The pivot is proven, not theoretical.**
> - **RESIDUAL: Link is frozen; the editor says "Cannot enter fly mode while event is active".** Log proof: on entry, `frameInterp: onTypeChange 41 -> 40 (event_type_id=40, … entering_event=true)` **with no matching `leaving_event=true`**, frames still running — versus the healthy pair earlier in the same run (enter 4268 → leave 4304).
> - **HISTORY FIX ALREADY APPLIED (data, no rebuild): your `stage.dzs` carried an `EVLY` event-layer chunk that TP's own `R_SP01` does NOT have**, holding a stub entry. A stage declaring an event layer with nothing behind it is a classic never-completing entry event. **Neutralised by retagging `EVLY` → `xVLY` (unknown tags are skipped by `dStage_dt_c_decode`) — minimal and reversible; backup `STG_00.arc.evly-bak`.** Please regenerate the dzs without EVLY when convenient.
> - **IF THE FREEZE SURVIVES THAT, the likely cause is yours and it is a familiar family: the №53-A door demo lock (`DEMO_DOOR_OPEN`) is begun in the OLD stage on the A-press and released on hold-complete/abort — but a NATIVE STAGE CHANGE tears that context down, so nothing ends it in R_DL01.** Fix either way: end/cut the demo on the new stage's first frame after a stage-change transport, **or** do not begin the door demo when the transport is a stage change (the fade + load already cover it; TP's door demo belongs to same-stage doors). **Same family as №65/№69/№73/№83 — state begun in the old context outliving the transition.**
> - **For the multi-room step: TP `R_SP01/stage.dzs` = STAG · RTBL(8) · MULT(6) · RCAM · RARO · SCOB(2) · VRB0(5) · Env0(8) · Col0(7) · PAL0(16)** — no EVLY, single env layer. **RTBL/MULT counts grow with room count** (their 6-interior stage uses RTBL 8 / MULT 6). History will author the per-room `RNN_00.arc` skeletons; those stage-side tables are yours.

#### Paste for Cursor (2026-07-19 — №87 playerInit is NEVER CALLED: room.dzr lookup returns NULL)

> **Your instrumentation disproved my №86 deduction — exactly what it was for.** `[dStage] playerInit enter …` fires for `F_SP102` (point=100), `F_SP122` (room 8), `F_SP115` (room 0), each followed by an `ALINK` create. **It never fires for R_DL01.** The guard is not the issue — **the function is never reached.**
> - **Call chain (source-verified):** `dScnRoom_Create` → `arcName = dComIfG_getRoomArcName(roomNo)` (`d_com_inf_game.cpp:2903` = `"R%02d_00"` ⇒ **`R00_00`**, matches your file) → **`roomInfo = dComIfG_getStageRes(arcName, "room.dzr")` (`d_s_room.cpp:598`)** → *only if non-NULL* → `dStage_dt_c_roomLoader` (`d_stage.cpp:2751`; its table maps **PLYR → dStage_playerInit**). **⇒ `roomInfo` is NULL — that is the blocker.**
> - **History data re-verified against the ENGINE'S decoder (not my parser):** `dStage_dt_c_decode` reads `dStage_fileHeader{m_chunkCount; m_nodes[]}` with `{tag, entryNum, offset}` and `offsetToPtr` rebases each node. **My room.dzr matches byte-for-byte (count=2, FILI@0x1C n=1, PLYR@0x24 n=1). Format is not the fault.**
> - **PRIME SUSPECT — name-keyed collision, same class as №73:** this run loads **two different resources both named `room.dzr`** — ours (**68 B**, the R00_00 stage room) and **WW's own (1088 B, from `LinkRM.arc`, pulled by the mount)**. If `getStageRes` resolves by name rather than strictly per-arc — or the arc is registered under a key other than `R00_00` (overlay lowercasing/pathing) — the room scene's fetch fails or lands on the wrong buffer. **Secondary:** `dComIfG_syncStageRes(arcName)` not complete when `dScnRoom_Create` runs.
> - **ASK — one line at `d_s_room.cpp:598`: log `arcName`, `roomInfo == NULL`, and the sync state.** That separates all three candidates in a single run. **If it is the collision, the fix is arc-scoped resolution — and it is worth auditing the other name-keyed lookups for the same trap (№73 precedent).**

#### Paste for Cursor (2026-07-19 — №86 WW room LIVE in R_DL01; player guard narrowed to ONE value)

> **The pivot's core proposition is PROVEN (log 211728): `№83 room0 mount 'EXT_BG1' id=00000134 stage='R_DL01'` (create now succeeds) → `[Spawn] src=room proc=EXT_BG1` → models load (`model.bdl mats=19`, model1, model3) → `№73 retain arc 'LinkRM' live=1` → **`BG COMPLEATE EXT_BG1 — host=(0,0,0) anchor=(0,0,0)`**. Grandma's house geometry and collision are mounted inside a real TP stage, with play scene, camera, room scene and TP `BG_e` all created and frames running.**
> - **SOLE REMAINING FAILURE: no `fpcNm_ALINK_e` for R_DL01 ⇒ no player ⇒ blank overlay.**
> - **DEDUCTION (saves you a logging round): the guard is `if (dComIfGp_getPlayer(0) != NULL || dComIfGp_getStartStageRoomNo() != i_stage->getRoomNo()) return 1;`. The overlay reports Link's position as `?` ⇒ NO player exists ⇒ `getPlayer(0) == NULL` ⇒ first condition FALSE. **Therefore it fired on the ROOM-NUMBER branch.** (`Failed to find player start point` is still absent, so the point match is never reached — consistent.)
> - **ONE VALUE TO RECONCILE: `dComIfGp_getStartStageRoomNo()` vs the `getRoomNo()` of the room whose dzr is parsed.** You pass `static_cast<s8>(dest.hostRoom)` = 0 to `setNextStage`, so either start-room is not actually landing as 0 (does our change path set it, or does it still hold the previous stage's room?), or **the parsed room is not numbered 0** (stage-side `RTBL`/`MULT`/`roomN.dzs` numbering, or the dzr being attributed to the stage rather than to room 0).
> - **User hypothesis answered: the unwired door animation / entrance demo is NOT the cause** — warping bypasses doors and still yields no player, and player creation is stage-load-time from PLYR data. Door polish can stay parked.
> - **History data unchanged and verified** (room.dzr FILI + PLYR angle.z=0; manifest identity anchor). The WW side of the transition is demonstrably correct now — the room loads and COMPLEATEs.

#### Paste for Cursor (2026-07-19 — №85 play scene OK, PLAYER not created: `dStage_playerInit` guard)

> **№84 landed — confirmed in log 203939: `fpcNm_PLAY_SCENE_e (11)` IS created for R_DL01 (pid 296), then CAMERA, ROOM_SCENE, `room.dzr`, `BG_e`, and `fapGm_Execute frame=600`. Frames run; nothing is hung.** What is missing is **the player**: no `fpcNm_ALINK_e` create for R_DL01 ⇒ overlay `Stage: ?`, no floor.
> - **Engine rule, read from `d_stage.cpp:1651` (`dStage_playerInit`):**
>   - **spawn match is `(u8)player_data->base.angle.z == point`** — the point id lives in PLYR **angle.z**. **History's authored PLYR has angle.z = 0, you request point 0, and TP's own D_SB05 uses the same pattern (0/1) — so the DATA is correct.**
>   - **on no match the engine logs `fatal("Failed to find player start point for next stage! …")` — THAT LINE IS ABSENT**, so the match was never attempted.
>   - **⇒ the early-return fired: `if (dComIfGp_getPlayer(0) != NULL || dComIfGp_getStartStageRoomNo() != i_stage->getRoomNo()) return 1;`**
> - **Two candidates: (1) A PLAYER ALREADY EXISTS — the old Link from F_SP115 was never torn down, so the new stage refuses to create one.** This also explains `playScene=0` at mount-check time and the blank overlay: a half-torn-down old scene. **(2) ROOM-NO MISMATCH** — `getStartStageRoomNo()` ≠ 0 while our room is 0 (check where `setNextStage`'s room argument lands).
> - **ASK: log `getPlayer(0)!=NULL`, `getStartStageRoomNo()`, `getStartStagePoint()`, and the room being parsed at the transition** — same discipline that named №84 in one line. **If (1) holds, the fix is to let the play-scene CHANGE do the teardown** (TP destroys and re-creates Link across a stage change — that is precisely what the play-scene path is for), rather than carrying the old player across.
> - **History status: room.dzr, PLYR angle.z, FILI, and the manifest (host_stage/anchor/spawn) are all verified against TP source or TP data. No data debt on this one.**

#### Paste for Cursor (2026-07-19 — №84 ROOT CAUSE: room-load used where a STAGE CHANGE is required)

> **Your reason-log paid for itself on the first run: `№83 room0 create FAILED 'EXT_BG1' reason=no_play_scene … stage='R_DL01' playScene=0`. THERE IS NO PLAY SCENE — and the log confirms the consequence: no `fpcNm_ALINK_e` create for R_DL01, so no Link, no camera, and the user's all-`?` overlay. A room scene alone is not a playable stage.**
> - **TWO MECHANISMS, and we are using the wrong one:**
>   - **(a) within-stage ROOM LOAD** — `d_stage.cpp:287` `fopScnM_CreateReq(fpcNm_ROOM_SCENE_e, …)`. This is what the room lane calls, and it is correct **only for rooms of the CURRENT stage** — which is exactly why the F_SP115 `R02` exemplar worked.
>   - **(b) real STAGE CHANGE** — the play scene re-requests **itself**: `d_s_play.cpp:608` `fopScnM_ChangeReq(i_this, fpcNm_PLAY_SCENE_e, l_wipeType[wipe], 5)` after `dComIfGp_setNextStage(...)`. **The play scene is what creates Link, the camera and the room scenes.**
> - **Our R_DL01 entry takes path (a), so a foreign stage's room data loads with no play scene around it — a hybrid TP never produces.** Log signature is exact: `fopScnM_CreateReq: procName=18` (ROOM_SCENE) and never a PLAY_SCENE change.
> - **FIX: when `hostStage` differs from the current stage, entry must be a NATIVE STAGE CHANGE** — `dComIfGp_setNextStage(hostStage, point, roomNo, layer)` and let the play scene's own change path run. **Do not `ensureRoomLaneLoaded`/room-create a foreign stage.** The room-lane code stays valid for **same-stage** rooms and becomes the per-room mount path once we are *inside* the interiors stage. **Mirror on exit** (same symmetry invariant as №65/№83).
> - **Verification checklist after the change:** `fpcNm_ALINK_e` created for R_DL01 · overlay shows `Stage: R_DL01` · Link spawns at the authored `PLYR` (-255,0,1125, ry -32768, id 0) · `playScene=1` · mount create succeeds. If the mount still fails with a play scene present, next suspect is create-before-room-ready (№83).
> - **Please keep the reason-logging pattern everywhere** — two earlier failures cost inference passes; this one was named by a single printed line.

#### Paste for Cursor (2026-07-19 — №83 shell LOADS now; only the mount create fails)

> **The screen looks the same to the user, but the failure moved a layer deeper — that is progress worth reading precisely (log 202053):**
> - **WORKING NOW:** `Loading Resource: room.dzr (Size: 68)` (History's authored room), stage.dzs / room0.dzs / pol_* load, **`fpcNm_ROOM_SCENE_e` created OK**, **TP's own `fpcNm_BG_e (732)` created OK**, and `fapGm_Execute` frames run inside R_DL01. **№82's scene-never-came-up blocker is CLOSED.**
> - **SOLE REMAINING FAILURE: `[ExtNpcMount] №62 room0 create FAILED 'EXT_BG1'`** ⇒ empty room (no WW model, no WW collision) ⇒ unchanged visible result.
> - **DATA IS NOT THE CAUSE — verified:** `ext_bg1.ini` already has `host_stage=R_DL01`, `host_room=0`, `host_pos=0,0,0`, `anchor=0,0,0`, `spawn_rel=-255,0,1125`, `spawn_ry=-32768` — identity anchor exactly as recommended. **You set this up correctly; nothing pending on History's side.**
> - **LEADING CAUSE (ordering): the mount create is attempted immediately after `ROOM_SCENE` create and BEFORE TP's `BG_e` room actor exists** — i.e. before the room's actor infrastructure is ready. **Your own feasibility note lists `onRoomObjectsReady` — create the mount from that hook rather than at scene-create time.** Secondary: `№62 room-lane register 'EXT_BG1' → room 0` fires at BOOT (line 801, during the F_SP115 session) — confirm the lane is re-established for R_DL01 rather than carried over stale.
> - **ASK: log the create failure REASON** (fopAcM_create return + actorId / roomNo / layer / proc). Two failures in a row have been diagnosed by inference where one printed value would have named the cause outright.

#### Paste for Cursor (2026-07-19 — №82 stage shell FIXED: R00_00.arc had no room.dzr)

> **Good news first: your naming + placement are right — `R_DL01`, neutral, MOD-SIDE (`files/res/Stage/R_DL01/`). Covenant-clean. `STG_00.arc` is credible too (stage.dzs: STAG/RTBL/MULT/RCAM/RARO/EVLY + Env/Col/PAL; room0.dzs FILI; pol_* tables).**
> - **THE FAULT: `R00_00.arc` contained only `shell.txt` — no `room.dzr`.** Log: `Loading Resource: shell.txt (Size: 28)` → `[ExtNpcMount] №62 room0 create FAILED 'EXT_BG1'`. TP's room scene reads its data from the room arc's `room.dzr`; with none, the scene never comes up — hence the user's **all-`?` debug overlay** and the black-screen door (Link left on F_SP115 at the porch).
> - **HISTORY FIXED IT IN DATA (no rebuild needed — just re-run).** Authored from TP's OWN templates, not invented: read `D_SB05/R00_00.arc` and `R_SP01/R00_00.arc` (Bo's House) out of the TP extract. **Minimal valid `room.dzr` = 68 bytes: 2 chunks — `FILI` (1 entry, TP interior bytes `1fec7f90 00000000` verbatim) + `PLYR` (1 entry): name `Link`, params `ff00a03f`, pos **(-255, 0, 1125)** = Nintendo's LinkRM door spawn in room-local coords, rx 0, **ry -32768** (authored facing), **spawn id 0**, tail `ffff`.** Packed into `R00_00.arc` in place of `shell.txt`; your original kept as `R00_00.arc.shell-bak`. Output re-parsed and verified.
> - **PLYR spawn-id offset re-confirmed on TP data: the u16 at 0x1C** (D_SB05 has consecutive entries with ids 0 and 1) — same decode as №56, so the warp-menu "points" mapping holds for TP-format rooms too.
> - **Next check on your side:** with the room up, verify the R_DL01 mount uses **anchor = identity / host = (0,0,0)** so WW room-local coords ARE the stage coords (that is why the PLYR sits at WW's own (-255,0,1125)); then symmetric `setNextStage` on exit.
> - **When we go multi-room:** each interior needs its own `RNN_00.arc` with this same 68-byte skeleton (**History authors those on GO**) plus a matching `roomN.dzs` FILI entry in `STG_00.arc` and the `RTBL`/`MULT` rows — **those stage-side tables are yours.**
> - **New shared reference: the TP extract is at `D:\XXXXXXX\Ex TP`** (WW is at `D:\XXXXXXX\Ex WW`). Pull TP-format skeletons from there rather than inventing them.

#### Paste for Cursor (2026-07-19 — №81 EXTENSION-FIRST: user call accepted; History reverses №80)

> **User's catch settles it: `F_SP115` declares `dStage_SaveTbl_LANAYRU` (4) — so any native save write from WW content TODAY lands in LANAYRU's chest/switch bits.** Nothing is corrupted only because no WW chest/key/switch is wired yet. **Exposure is live but unexercised.**
> - **HISTORY REVERSES №80's "use the 9 first". EXTENSION-FIRST is correct, because ZERO WW progression is wired — this is the cheapest moment in the project to build the real store (later = migrating real player saves instead of empty ones).**
> - **New allocation: ALL WW progression → the Dusklight extension from day one. The 9 free vanilla indices stay as HEADROOM FOR FORK-SIDE TP FEATURES (ALBW, Shade Refuge, …), not consumed by WW.** Layout-compatibility still stands — mirror `dSv_memBit_c` so native chest/key/boss accessors work unchanged; it is now the PRIMARY home, not a fallback. Please include a versioned migration that never silently resets saves.
> - **IMMEDIATE GUARD (before the pivot lands): WW content must never call native save writers while hosted in a vanilla stage — today that writes into Lanayru. Assert/refuse + log if a WW mount attempts a native save write outside a WW stage.**
> - Still open from №79/№80: **mod-side stage shells?** final neutral stage name + save-index value; **dump STAG values** (now lower priority for WW, but it fixes the fork-headroom map).

#### Paste for Cursor (2026-07-19 — №80 rooms vs slots; use the 9 now, extend by VIRTUAL INDEX later)

> **User asked whether to use the 9 free indices or declare WW-specific ones. History's answer — separate the resources first:**
> - **Stages = unlimited** (your data-driven finding). **Rooms = ~64 per stage** (`dSv_memory2_c mVisitedRoom[2]` = 64 bits; WW `sea` has 50 → fits). **Save slots = the ONLY fixed resource.** So new rooms/stages are always available; only progression slots are budgeted.
> - **A slot is PER-STAGE, not per-room:** one `dSv_memBit_c` = 64 chest bits + 128 switch + 32 item + key count + MAP/COMPASS/BOSS_KEY/BOSS_ENEMY/BOSS_DEMO, shared by all that stage's rooms. **A 21-room dungeon uses ONE slot.** ⇒ 9 slots ≈ 9 dungeons of native progression.
> - **PINCH TO WATCH: Great Sea as one stage = ONE slot = 64 chest bits for all 49 sectors.** If WW's island chest count exceeds it, split the sea into sector-group stages or move it to the extension. History will count WW chest placements per sector when W3 nears.
> - **RECOMMENDATION: (1) use the 9 NOW** — zero engine work, native chests/keys/boss flags work as-is. **(2) Design the extension as a VIRTUAL INDEX SPACE for later: indices ≥32 route to a Dusklight block with the IDENTICAL `dSv_memBit_c` layout**, so `dComIfGs_isStageTbox`/switch/key/boss call sites are unchanged and slot→virtual migration is a COPY. **That layout-compatibility is why starting with the 9 is not a trap; bespoke WW storage WOULD be, because it forks the accessors and loses native behaviour.**
> - **BEFORE ANY DUNGEON CLAIMS A SLOT: dump the actual STAG save-table values across vanilla stages** — the 9 are inferred from unnamed enum entries and should be verified fact, not assumption. (Interiors are unaffected; they stay on the catch-all.)

#### Paste for Cursor (2026-07-19 — №79 save-index COUNT ANSWERED + critique of the stage plan)

> **Your feasibility is accepted — and COUNT #1 is closed from the source, no dump needed (though confirm if cheap).** `dStage_SaveTbl` in `d_stage.h:1325` enumerates vanilla's claims: 0 ORDON · 1 PRISON · 2 FARON · 3 ELDIN · 4 LANAYRU · 6 FIELD · 7 GROVE · 8 SNOWPEAK · 9 CASTLE_TOWN · 10 DESERT · 11 FISHING_POND · 16-24 LV1-LV9 · 25 CAVE1 · 26 CAVE2 · 27 GROTTO = **23 used**. Field is 5 bits (`field_0x09 >> 1 & 0x1f`) ⇒ space 0-31. **FREE: 5, 12, 13, 14, 15, 28, 29, 30, 31 — NINE.**
> - **CRITICAL: "save-neutral" must NOT mean declare 0 — 0 is ORDON.** A WW stage declaring 0 writes into Ordon's chest/switch bits (the free-bit corruption class). **Declare a FREE index instead — propose 15 as the WW interiors/misc catch-all.**
> - **BUDGET: reserve 15 = interiors/misc; reserve 5, 12, 13, 14, 28, 29, 30, 31 (eight) for WW DUNGEONS.** WW has ~7 majors ⇒ sufficient for majors; the long tail (island chests, caves, submarines) is what justifies the №78 save-extension block — **decide that before W3, not before this exemplar.**
> - **NAMING: REJECT `R_WW01`.** Stage folders live in `/res/Stage/` = **the game tree**, and the receiver covenant forbids WW names there. Use TP-native-looking unused numbers (`R_SP90`, `R_SP91`, …) or a neutral fork prefix (`R_DL01`), ≤7 chars. **BETTER: can stage shells live MOD-SIDE via the existing custom-asset redirect (your `Toripost` R2 arc-mount precedent)? Then the game tree stays untouched and the covenant is structurally satisfied, not just name-dodged. Please answer this one.**
> - **ENDORSED:** exit regaining `setNextStage` (**that is the symmetry invariant — enter and exit must BOTH be native stage changes, or we rebuild the asymmetry behind №66/№69/№73**); warn-only strand guard keyed on `exitKnobSpawned`; stage-gated room-lane reuse.
> - **SEQUENCING: interiors are save-neutral ⇒ the pivot is NOT blocked by the dungeon budget. Proceed with the exemplar now.** Use **room 0** for LinkRM in the new interiors stage (fresh native numbering; R02 was an F_SP115 workaround).
> - **On GO, History authors:** the room-assignment table (one room per interior), the warp-menu entries (regions/maps/rooms/points from Nintendo spawn ids), and the per-room manifests. **Cursor authors the thin stage shell** — tell me the final stage name + save-index value and I will match all data to it.

#### Paste for Cursor (2026-07-19 — №78 CORRECTION: WW dungeons DO want save slots; two counts needed before the pivot)

> **User challenged №77's "save-neutral" advice and was right. Correcting: a TP save slot IS the progression vocabulary.** `dSv_memBit_c` (0x20/stage) = `mTbox[2]` **64 chest bits** · `mSwitch[4]` **128 switch bits** · `mItem[1]` 32 item bits · `mKeyNum` **small-key count** · `mDungeonItem` = MAP/COMPASS/BOSS_KEY/**STAGE_BOSS_ENEMY**/STAGE_BOSS_DEMO/STAGE_LIFE/STAGE_BOSS_ENEMY_2. Quest/story = `dSv_event_c mEvent[256]` = **2,048 global event bits**; dungeon runtime = `dSv_danBit_c`.
> - **CORRECTED RULE: WW DUNGEONS CLAIM THEIR OWN SLOTS** (that is how keys/chests/boss-cleared work natively). **The prohibition is only: never declare an index a vanilla stage already uses.** Houses/interiors with no persistent state still declare none.
> - **COUNT #1 (please dump): how many of the 32 `dSv_save_c::STAGE_MAX` indices do vanilla stages actually use?** Assignment is per-stage via STAG (`dStage_stagInfo_GetSaveTbl`). **That free count = the WW dungeon budget, and it should be known before the pivot commits.**
> - **COUNT #2 (design): is 32 enough for the whole WW restoration** (its dungeons + islands/caves with chests)? If not, the honest PC-fork path is a **Dusklight WW save-extension block** — WW-stage entries mirroring `dSv_memBit_c` semantics + a WW event bitfield sized like TP's 2,048 — rather than squeezing vanilla. **Precedent: the fork already extended saves once (mod flags, v1→v2).** Please include a versioned migration that never silently resets player saves.
> - **Clarification for the record: the ~18-entry mod-flag KV in `dSv_reserve_c` is for MOD TOGGLES only — never chests/keys/bosses/quests.**

#### Paste for Cursor (2026-07-19 — №75/№76/№77 PRE-PIVOT DESIGN: stage+room mapping, warp-menu data, save-bit safety)

> **User is going with NEW NEUTRAL STAGES feeding WW entries into the warp menu. History's evidence pass answers the design questions — please fold into the feasibility:**
> - **EXTERIORS/SEA (№76): WW ships `sea` as ONE STAGE WITH 50 ROOMS (sector rooms) — verified in the extract; dungeons likewise (Siren 24, M_Dai 21, kaze 17, Cave09/10/11 21 each). TP room streaming = WW's "islands cull behind you / draw as they approach". And Nintendo already numbered ours: Outset's SCLS says `sea … room=44` ⇒ Outset IS sector room 44.** Hylia was bootstrap only; exteriors move to their own stage with WW coordinates (anchor = identity) and the parking table dies.
> - **INTERIORS (№76): TP's OWN warp table proves the pattern — `R_SP01` holds SIX Ordon interiors (Bo r0, Sera r1, Jaggle r2, Link r4+r7, Rusl r5); `R_SP109` SEVEN Kakariko interiors; `R_SP160` SIX Castle Town interiors — one stage per settlement, ONE ROOM PER INTERIOR.** WW matches (`Ojhous`/`Ojhous2` Room0/Room1, `Obshop` 6 rooms). **Adopt exactly that: interiors stage per settlement, room per interior; dungeon = one stage, its rooms as rooms.**
> - **WARP MENU (№76): it is DATA — `include/dusk/map_loader_definitions.h` (Region → Map(stageFile) → Room → points). Adding WW stages/rooms there yields menu entries for free, and the "points" are exactly the Nintendo PLYR spawn ids History already extracted (№56) — History will author these tables.**
> - **SAVE-BIT SAFETY (№77 — user asked whether new stages compound the old free-bit problems): NO, IF SAVE-NEUTRAL. `dSv_save_c::STAGE_MAX = 32` (+64 `mSave2`), and a stage's save slot is DECLARED by its STAG info (`dStage_stagInfo_GetSaveTbl`, consumed by `dComIfGs_isStageTbox/onStageSwitch/…`). ⇒ **WW host stages must declare NO save table (or a reserved unused index) — then N new stages cost ZERO save bits. Declaring a vanilla index would write into vanilla chest/switch bits (the sumo-bit failure class).** TRAP: TP's native `tbox` chest actor writes to those stage bits — **WW chests/hearts must grant via our `dExtModFlags` store, not tbox.**
> - **FORWARD ITEM (not now, but before W2/W3): our mod-flag KV is ~18 entries in `dSv_reserve_c`'s 80 bytes — fine for Outset, far too small for a full WW restoration (every island's chests/hearts/quests). A hashed store at capacity = silent collisions. Plan a re-size + namespacing + versioned migration that does not silently reset player saves.**
> - Sequencing unchanged: №74 guard fix/demotion first, then this feasibility.

#### Paste for Cursor (2026-07-19 — №74 auto-warp-out = the №66-C guard misfiring; GO for the stage rework)

> **Stability first: user completed 4+ enter/exit loops with NO crash and NO mesh corruption — №69 and №73 both hold.**
> - **The auto-warp names itself in log 182335:** `[Doors] №66-C stranding G-guard — 'EXT_BG1' drawable=1 exit=0 → return_pos`. It fired on entries 3 and 4; entries 1-2 (quick in-and-out near the door) exited normally. The user rolled around the room on the later entries — i.e. **moved away from the door** — and the guard fired.
> - **DIAGNOSIS: the guard's "is there an exit?" predicate is player-proximity based.** `exit=0` even though the same entry logged `exit prop key=exit:linkrm pos=(-60255,2000,-238875)` — the prop existed; Link had simply walked out of its radius. **`drawable=1` means the room was healthy — a stranding guard must never actuate in that state.**
> - **History owns this one: №66-C was my recommendation. Intent right, predicate wrong** (tests where the player stands instead of whether the room HAS an exit).
> - **FIX:** (1) existence test = the room's exit-prop **registry/spawn record**, never distance-to-player; (2) actuate ONLY on genuine no-exit — prop failed to spawn, or Link outside the room AABB with no transport pending — **AND** require ~30+ frames of persistence; (3) log evidence (prop id, player pos, AABB, drawable) so future false positives self-diagnose; **(4) RECOMMENDED NOW: demote to WARN-ONLY (log, do not warp). We are stable, and a guard that misfires is worse than the strand it prevents — re-arm it when the stage lane lands.**
> - **USER ASKED IF IT IS TIME TO REWORK INTERIORS: History says YES.** The crash class (№69) and cache class (№73) are fixed and understood, the exemplar is stable, and the only open item is a guard predicate. **Pivoting now means pivoting from a working, explained baseline (one-exemplar doctrine satisfied), not mid-bug. Please proceed to the №71/№72 stage feasibility as the next major work** — R_SP01 pattern: interiors stage per island with a room per interior; dungeon = one stage, its rooms as rooms; your room-lane code becomes the per-room mount path.

#### Paste for Cursor (2026-07-19 — №73 crash FIXED; re-entry corruption = stale J3D cache across resDelete)

> **№69 WORKS — user did enter→exit ×3 with no crash** (`place-first` → `Released interior BgW` → `forgot mount` → unload, all logged). **New bug on re-entry: garbage geometry, collision FINE — the signature of drawing from freed memory.**
> - **PROOF in log 181136:** the 2nd entry binds models (`BG model[0] 'model.bdl' mats=19 …`) with **NO `model-data cache +` line** ⇒ a **cache hit**, while the unload path had already called `dComIfG_resDelete(arc)` (`d_ext_npc_mount.cpp:4041/4050`). **`s_modelDataCache` (`:70`, keys `bg:<Arc>/<model>.bdl`) is session-lived, and its own comment at `:2053` states the assumption the room lane broke — "Keep s_modelDataCache — session-lived; arcs are stable while the mod folder is."** True in the pinned lane (permanent mounts, arcs never released); false now. **Collision survives because the dzb is re-fetched and a fresh `dBgW` built per entry — hence "everything beyond collision is garbage."**
> - **ROOT LAW (the user connected this to the sumo/BMT crash, and it is 1:1): J3D parsed data is POINTER-FIXED INTO ITS ARCHIVE BUFFER.** Two fatal directions, both now proven in this project: **(a)** re-parsing an already-fixed buffer = the `body_bmt` AV (№50-C; fixed with a pristine-raw stash + parse-from-copy); **(b)** freeing the buffer while parsed data is cached = this bug.
> - **FIX: purge `s_modelDataCache` entries for an arc whenever that arc is released** — arc-scoped key sweep in `dExtNpcMount_delete` / the room-unload path — **and update the now-false comment at `:2053`.** Re-parsing on the next entry is cheap and the fade covers it. **WATCH SHARED ARCS: `Knob.arc` serves 5+ door props plus exit knobs — one owner's release must not orphan another's parsed data (refcount, or pin shared arcs).** Per №70 the `daBg_c` shape puts this purge in the destructor next to the collision release = one symmetric teardown.
> - **PATTERN WORTH NAMING (three in a row): the room lane keeps invalidating pinned-lane invariants — stale door handles (№68), collision released under the player (№69), session-lived model cache (№73). Please SWEEP the remaining session-lived state for "arcs/mounts are permanent" assumptions BEFORE the stage pivot, or they resurface there.**
> - Acceptance: enter → exit → re-enter ×3, room renders correctly on entry 2+.

#### Paste for Cursor (2026-07-19 — №72 CORRECTION: multi-stage hosting IS possible — TP's R_SP01 is the blueprint)

> **User challenged "why is it impossible to house interiors across different stages?" — they are right and History corrects the record.** Two claims got conflated: **(TRUE)** TP's scene loader cannot parse WW stage archives (hence the mount/BG backend); **(FALSE, implied by earlier phrasing)** that we must therefore keep everything in ONE host stage. Nothing forces that.
> - **TP PRECEDENT, in this repo: `R_SP01` = "Ordon Interiors" — a SEPARATE STAGE whose ROOMS are the houses.** See `d_a_npc_seira.cpp:363` ("R_SP01-R01 Ordon Interiors - Sera's Sundries"), plus room 4 (`d_a_alink.cpp:11799`) and room 7 (`d_a_tbox.cpp:539`). **Ordon Village is a different stage entirely (F_SP103). TP separates WORLDS by stage and subdivides each world by ROOM.**
> - **This resolves rooms-vs-stages: it is BOTH.** Rooms = unit within a world; stages = separation between worlds. **Our error was not "using rooms" — it was using rooms OF THE ISLAND'S STAGE for interiors, which is precisely what puts them in the island's world space (the offshore parking table).**
> - **Hard expiry on parking (user's point, and it is decisive): interiors currently sit at coordinates the GREAT SEA will occupy — players would sail into interiors.** Scale it by every island's houses/shops plus dungeons (many rooms each) and the single-stage approach collapses.
> - **PROPOSED MAPPING (mirrors TP and WW; economical on stage entries): island exterior = its own host stage · that island's interiors = ONE interiors host stage with a ROOM PER INTERIOR (the R_SP01 pattern) · each dungeon = one host stage whose rooms are its rooms.** No world-space parking; nothing co-resident that shouldn't be.
> - **Feasibility (sharpens №71):** how host-stage entries get added/registered; scene-create → mount WW BG + population per room; save/`Saved Stage` bookkeeping; return plumbing; **and whether your room-lane code becomes the per-room mount path unchanged (it should — that would make the exemplar work reusable rather than thrown away).**
> - **Sequencing unchanged and user-agreed: №69 crash fix FIRST.**

#### Paste for Cursor (2026-07-19 — №71 USER CALL: interiors should be their own STAGES — feasibility requested)

> **User: "proof we need to make each interior its own stage — am I right?" History: essentially YES, and the tally backs it.** Everything this lane has cost — offshore parking, warm-up hack, leave-shell AABB guards, swim-between-rooms, fade races, stuck fade, twin doors, collision UAF (№69), re-entry draw failure (№66-B) — **not one is a DOOR bug; all are lifetime/ownership bugs, and they exist only because we invented a configuration neither source game has (everything co-resident + player moved by hand). TP/WW make a house door a STAGE CHANGE: exterior unloads, player RE-CREATED at the authored spawn (№70, verified in-source). That makes this whole class inexpressible rather than fixed.**
> - **PRECISION: "interior = its own stage" = a TP HOST STAGE per interior whose scene create mounts the WW BG — NOT TP loading WW's stage format (still impossible; that is why the pinned-warp backend exists).** The mount side is already proven by your room-lane exemplar; the open question is **stage-table entries** vs the room slots you added (F_SP115 R02).
> - **Caveat kept honest:** rooms are not inherently wrong (TP streams dungeon rooms within a stage) — our room lane broke because it diverged from the engine's ownership model (`daBg_c`, №70), not because rooms are the wrong unit. **Deciding factor is scale:** few small interiors ⇒ proper room slots are cheaper; the whole game (many islands, the Great Sea) ⇒ stage-per-area is what both source games do.
> - **SEQUENCING (one-exemplar doctrine): land №69's ordering fix FIRST** — small, unblocks the exemplar, validates the rest of the room lane — **then pivot with your feasibility in hand.** Pivoting mid-crash leaves no working lane and no evidence.
> - **FEASIBILITY REQUESTED (no build yet):** host-stage table entries for N interiors; scene-create → mount WW BG + population; `Saved Stage`/save bookkeeping across the change; exterior reload cost on return (TP/WW both pay it — is it acceptable here?); and whether the existing room-lane code is reusable as the per-stage mount path.

#### Paste for Cursor (2026-07-19 — №70 TP/WW native invariant: why №69 is a restoration, not a hack)

> **User asked how this pairs with TP/WW. Verified in this repo's own TP source — and it hands us the design:**
> - **TP: collision is owned per-room by an actor.** `d_s_room.cpp:281` → `fopAcM_create(fpcNm_BG_e, roomNo, …)`; `daBg_c` registers at `d_a_bg.cpp:599` (`dComIfG_Bgsp().Regist(mpBgW, this)`) and **releases in its DESTRUCTOR** at `:262-265` — `Bgsp().Release(mpBgW)` + `dStage_roomControl_c::setBgW(roomNo, NULL)`, plus `bgp->releaseBg()`, room grass/flower cleanup, `offStatusFlag(roomNo, 0x10)`.
> - **Therefore TP frees collision exactly when the ROOM SCENE dies, and never tears down the room the player occupies** — he has either crossed into the next room (streaming) or is being **re-created** in a new scene (door = stage change). **WW is the same shape** (our №56 extraction: stage + spawn-id + room per exit, player respawned at the authored PLYR). **Neither engine can ever free collision under a persistent player.**
> - **Our exemplar is the one configuration both games avoid** (player persists AND we free his room, then move him) — hence №69. **So the ordering fix (place Link → release collision → free room) reproduces TP's own room-streaming behaviour: a restoration of the native invariant, not a workaround.**
> - **STRUCTURAL SUGGESTION — solves №69 and №66-B together: model the BG mount on `daBg_c`.** One collision-owning actor per room slot: register on create, **release in the destructor**, and mirror the room bookkeeping symmetrically (`setBgW(roomNo, x)` on load → `NULL` on unload; room status flag ON at load → OFF at unload). **Note `daBg_c`'s teardown clears precisely the room state whose absence breaks our re-entry draw (№66-B) — TP already documents the symmetric pair in code.** Route through that shape and the engine's room lifecycle drives load + draw-state + release as one unit.
> - **Ladder:** (1) now = ordering fix (unblocks the exemplar); (2) next = mount-as-room-BG (engine owns lifecycle); (3) endpoint = **stage-per-interior**, literally what TP/WW do for houses (exterior unloads, Link re-created at the spawn) — the №62 "stage-per-area variant", which makes this bug class impossible and is the honest destination once interiors multiply across islands.

#### Paste for Cursor (2026-07-19 — №69 exit crash = COLLISION use-after-free)

> **№68 did NOT fix it, and that is the useful result: the same crash occurred WITH your handle-forgetting in the build** (`№68 forget handles` → `№68 forgot mount` → `№62 unload room2` → `№68 room2 unload (handles already forgotten)` → **AV**). **Identical `EXCEPTION_ACCESS_VIOLATION`, fault addr `0x66c`, crash RVA `0x22f847` in BOTH builds** — a stable RVA across a rebuild means a large unchanged ENGINE function, not the door code you just edited. (History's twin-door data fix is verified working here: **zero** `src=census:KNOB00` spawns.)
> - **DIAGNOSIS (high confidence, from your own source): Link's collision attachment outlives the interior's collision object.** The BG mount registers collision globally at `d_ext_npc_mount.cpp:3886` (`dComIfG_Bgsp().Regist(i_this->mpBgW, i_this)`) and only releases it in `dExtNpcMount_delete` (`:4002-4004`). **On exit we release/free the interior `dBgW` while Link is still standing on it — his ground/attach state still points at the freed `dBgW`, and the next ground update dereferences it → AV at a small member offset (`0x66c`) inside a big engine collision routine.** This is why forgetting *door* handles changed nothing: the dangling pointer is the PLAYER's, not the door system's.
> - **FIX = ORDER, not new logic:** **(1)** place Link on the island first (native return to `return_pos`, so his ground probe re-attaches to EXT_BG0 collision); **(2)** THEN `Bgsp().Release(mpBgW)` + null; **(3)** THEN free the room/heap. If it must be one frame, explicitly **detach/clear Link's cached floor (force re-probe) before the Release**. General rule worth keeping: **never Release a `dBgW` the player is standing on; unregister from global systems BEFORE the heap dies; defer the free to end-of-frame when in doubt.** Applies to the exit-knob actor too.
> - **Symbolization is NOT available on the user's machine** — `llvm-symbolizer` cannot read MSVC PDBs (`??:0:0`; the repo's old `tools/_symcrash.out.txt` shows the same failure) and `cdb.exe` is not installed. **To confirm the frame: link with `/MAP` (RVA→symbol becomes a text lookup) or reproduce once under the VS debugger.** Please do one of those and post the function name — it either confirms the collision path or redirects us in one step.
> - **Acceptance unchanged:** enter → exit → re-enter → exit ×3 with no AV, single-press exit.

#### Paste for Cursor (2026-07-19 — №68 exit crash: teardown order + twin doors)

> **№66-A confirmed landed** (`№66 room-lane EXIT cover→unload` — fade cover is in; entry clean). **Exit then CRASHED: `EXCEPTION_ACCESS_VIOLATION`, fault addr `0x66c`** immediately after `№62 unload room2 keep0 rt=0` → `room2 unload drop 'EXT_BG1'`. A tiny member offset like `0x66c` = **dereference of a freed/NULL object + field** — something still touches room-2 actors after the room dropped them. (Crash PC not symbolizable — exe rebuilt since — but the sequence is unambiguous.)
> - **FIX = ORDER THE TEARDOWN** (same family as №64's FIFO steal: registries/handles outliving their actors). Before dropping the room: **(1)** forget every cached handle into it — DoorDef `exitKnobSpawned`/`hasExitKnobHost`/exit-knob actor id, doorAttention registration, any mount pointer the door poll or `playAnimNearest` walks; **(2)** make poll/draw skip a room that is unloading (one `s_roomUnloading` latch is enough); **(3)** THEN unload; **(4)** THEN place Link; plus `clearForBg` so re-entry re-spawns cleanly. **Acceptance: enter → exit → re-enter → exit ×3, no AV.**
> - **TWIN DOORS — CONFIRMED by this log and ALREADY FIXED IN DATA (no rebuild needed).** The log shows BOTH `[Spawn] src=census:KNOB00@(-255,0,1125) proc=NPC_KNOB` AND `[Doors] exit prop key=exit:linkrm pos=(-60255,2000,-238875)` — the **same world spot**. Every interior had two overlapping knobs: a decorative census one with no door key + the functional exit one. **That is almost certainly the user's "double press to exit" (press 1 lands on the dead twin), and it doubled the teardown surface feeding the crash.** History has unmapped `[KNOB00]`/`[KNOB00D]` in `actor_map.ini` — interior census door rows are holes BY DESIGN now; the exterior CSV has zero knob rows anyway. **Doors come from ONE source: `doors.ini` (exterior) + `exit_door_rel` (interior)** — and `exit_door_rel` was derived from those very census rows, so nothing authored is lost.
> - Expect single-press exit once the crash is fixed; if a double-press survives the twin removal, ferry the `A-press` / attention lines and History will re-diagnose.

#### Paste for Cursor (2026-07-19 — №66 room-lane regressions + №67 binary covenant)

> **History audited the №65 build in code + log 162543. VERDICT: the native transport LANDED** — `№65 room-lane ENTER/EXIT transport … (no BgWarp)`; entry cycles log clean (register room 2 → mount → BG COMPLEATE → population → `Link placed at spawn_rel`). Two regressions remain, both narrow:
> - **№66-A FADE DROPPED — not optional.** The native path lost `beginDoorFade` + the door-demo lock. Beats 2 (instant control lock) + 4 (screen cover) of the **user-RATIFIED №60 seven-beat spec** are what make a transition read as a door instead of a teleport — restore both AROUND the native room change (cover → change room → place → lift). This is the whole “magically moved there” complaint.
> - **№66-B THE VOID IS A DRAW/VISIBILITY BUG ON RE-ENTRY, NOT A LOAD FAILURE.** The failing entry logs IDENTICALLY to good ones (mount id, `BG COMPLEATE`, population, placement) and the user’s overlay reads `Room: 2` at the correct spawn — engine believes Link is in a loaded room while nothing renders. **Suspicion: the unload (`№62 unload room2 keep0 rt=0`) clears room 2’s table/switch/RTBL state and the reload re-creates the MOUNT without restoring the ROOM’s draw registration. Make load/unload symmetric; acceptance = enter → exit → RE-ENTER and the room still draws.**
> - **№66-C STRANDING GUARD (safety).** In the broken state the user had no exit prop (`A-press — no doorAttention prop within 300u` ×12) and no way out but the menu. Extend the leave-shell/G-guard to the room lane: room claimed + not drawable / no exit prop ⇒ forced native return to `return_pos` + log.
> - **Diagnostic honesty:** the exit ledger printed `lane=room` while the transport was pinned (pre-№65). Always label the TRANSPORT, not the room claim — that mislabel is how a half-migration reads as finished.
>
> **№67 — BINARY COVENANT (audit F-3 ratified; affects what you may hand anyone).** History re-ran the gate on **all four** `dusklight.exe` copies: `build/fps-good-backup-20260719-0827` **FAILS** (`"NPC_MK — Ivan socket (Mk.arc; Plan R)"`, `"NPC_P2 — Medli socket (Plan R)"`, `mk.greet`, manifest-template text) — marked in place with `DO-NOT-SHARE-fails-covenant-gate.txt`, **NOT renamed** because `tools/_fps_ab3.bat` hard-codes that path. The other three are clean; **current `windows-msvc-relwithdebinfo` is clean and is the shareable one** — its only `display_name=` hit is the neutral `display_name=RealName` help text, which confirms the №55 covenant edits compiled in. **Standing rule (cookbook §0): binaries outlive source fixes — any preserved exe passes the gate before it leaves the machine. Triage, don’t count: TP’s own vocabulary (Bokoblin/Moblin, Makar/Medli STUB-REL roster labels) is EXEMPT; WW character/arc/dialogue/manifest strings are leaks.**
>
> **Also live from History (data lane, no rebuild needed):** №59 Sturgeon’s real room installed (`Ojhous2R1` from the extract at `D:\XXXXXXX\Ex WW`) + furniture arcs (Otana/Table/Plant/Opaper/Lamp) + №61 Mesa door radius normalised to 240 (it had inherited the forest gate’s 280 and armed on approach — the “Mesa acts like a walkthrough” report).
>
> **Queued behind №66 (do not start until the exemplar passes triple enter/exit/re-enter):** migrate remaining interiors to the room lane (manifests already carry every value); №60 crawlthrough/dropthrough gates (Mesa’s interior hole, LinkUG exterior crawlspace, Cave09 room hop); beat 6 arrival walk-in; twin-KNOB00 dedupe in LinkRM.

#### Paste for History (2026-07-19 — №65 room-lane TRANSPORT)

> History №65 absorbed. Cursor shipped (`build_run.bat` green; caches wiped).
> - **Enter:** `requestRoomLaneEnter` — `ensureRoomLaneLoaded` + wait mount ready + place at `host+spawn_rel`/`spawn_ry` + `ExtNpcPop` + hold/fade. **No `requestBgWarpGuarded`.**
> - **Exit / leave-shell:** `requestRoomLaneExit` — unload claim + place at authored return world + facing. **No `requestBgWarpTo(EXT_BG0)`.**
> - **Ledger:** `transport=room|pinned` (labels the transport, not the claim).
> - Pinned doors unchanged (still BgWarp).
> **USER VERIFY:** enter+exit both `transport=room` / `№65 room-lane`; exit feels like entry (no mid-air teleport feel); other doors pinned.
> Please critique: place-after-load vs true `setNextStage` point spawn; keep room0 vs full exterior unload.

#### Paste for History (2026-07-19 — №64 room-lane FIFO steal + pop bind)

> History №64 absorbed (log 155418). Cursor shipped (`build_run.bat` green; caches wiped).
> - **Placement was already consuming the manifest:** Link at `host+spawn_rel` = (-60255,2050,-238875); `spawn_ry` applied. Mid-room was NOT a lane-local constant.
> - **Root cause:** `createBgMountAtHost` push+bind left an orphan FIFO pending; next HENNA0 (population) stole it → second EXT_BG1 at the door (`host=(-60255,…)`) + Ba1 create ate Lamp's pending (`NPC_BA arc=Lamp`).
> - **Fix:** drain matching FIFO on `bindPendingSpawn`; room-mount create latch (no phase_3/4 double); population `roomNo` = claimed host room; `dExtNpcPopulation_clearForBg` on room unload so re-enter respawns Grandma.
> - Exit/return already used `exit_door_rel` / `return_pos` (same door for LinkRM).
> **USER VERIFY:** one EXT_BG1; enter at door facing in; Ba1 is Grandma not a lamp; exit → porch.
> Please critique: FIFO drain vs stop-pushing when bind follows; LinkRM census only maps ~3 day-1 props (rest unmapped holes — data?).

#### Paste for History (2026-07-19 — №62 Phase D LinkRM room-lane exemplar)

> History №62 + user GO absorbed. Cursor shipped Phase D (`build_run.bat` green; GPU caches wiped).
> - **Exemplar only:** `[linkrm]` → `lane=room` + `host_room=2` (EXT_BG1). All other doors stay `lane=pinned`.
> - **Thin shell:** AppData Aurora `files/res/Stage/F_SP115/R02_00.arc` (tiny RARC, no `room.dzr`) — room slot claim only; WW geometry still from EXT_BG1 mount.
> - **Load path:** forced `loadRoom([0\|0x80, 2\|0x80], true)` + `setNextStayNo(2)` (bypasses RTBL adjacency). Keeps exterior room 0 (field unload deferred).
> - **Lifecycle:** `d_s_room` object-ready → create EXT_BG1 in room 2; status-8 unload → drop mount. Room-lane procs skipped by №58-B warm.
> - **Doors:** `commitEnter` dual backend; exit warps porch then unloads room 2. Ledger: `lane=room host_room=2` / `№62 room2 mount`.
> - **Still deferred:** room-local `host_pos` (still offshore park); full RTBL/STG edit; unload exterior with interior; migrate more doors.
> **USER VERIFY:** mailbox door → LinkRM (room lane logs); exit porch; other doors pinned; no mash/void.
> Please critique: keep-room0 vs kill-exterior; thin shell vs pack LinkRM as full R02; when to move host_pos on-island.

#### Paste for History (2026-07-19 — №62 ROOM-PER-INTERIOR feasibility)

> History №62 absorbed. Cursor feasibility ONLY (no Phase D build). **Verdict: CONDITIONAL-GO.**
>
> **Terminology:** TP *room* = same-stage stream of `Rnn_00` under `/res/Stage/<stage>/` via RTBL + `dStage_roomControl_c::loadRoom` → `d_s_room`. Not Ordon's separate interior *stage* (`R_SP01`), and not today's offshore EXT_BG park.
>
> | Q | Verdict | One line |
> |---|---|---|
> | **1 room slots** | **CONDITIONAL** | Aurora overlay can ship edited stage/room arcs; F_SP115 disc has **only R00+R01** (`D:\XXXXXXX\Ex TP\files\res\Stage\F_SP115`) ⇒ slots **2–63 free**. Need RTBL patch (offline arc or runtime) — no in-repo RARC writer yet. |
> | **2 mount↔room lifecycle** | **YES** | `d_s_room.cpp` `objectSetCheck` (create) + status-8 unload / `dScnRoom_Delete` already host ALBW spawns — bind EXT_BG create/destroy the same way (`roomNo→proc` data map). |
> | **3 door→native room change** | **CONDITIONAL** | Shutter path = `setNextStayNo` + `dStage_RoomCheck`/`loadRoom`; Ordon knobs = `onSceneChangeArea`→SCLS stage change. Today's `doors.ini` only drives pinned warp — needs `host_room=` / `lane=room` + second backend in `commitEnter`. Knob UX (CANDOOR) can stay. |
> | **4 dual-lane migration** | **YES** | Keep `beginBgWarp` default; opt **one** door onto room lane. Failures fall back to green pinned path. |
>
> **Recommended exemplar:** LinkRM / `EXT_BG1` → claim **`R02_00`**, hybrid thin room shell + mount-on-create (reuses proven BG), `[linkrm]` `lane=room`, all other doors stay pinned. Exit → stayNo 0 / porch (not offshore).
>
> **Blockers before Phase D:** (1) RTBL adjacency room0↔2 (+ MEM/MEC if heap tight — 19-block cap); (2) coordinate strategy — room-local origin replaces offshore `host_pos`; (3) field vs dungeon unload semantics on `ST_FIELD` F_SP115; (4) user GO.
>
> **Ordon correction for the ledger:** Ordon houses are separate *stages*, not rooms of `F_SP103`. №62 primary analogy = dungeon shutter same-stage rooms; Ordon = the stage-per-area variant for big spaces (A_mori later).
>
> **Phase D:** do **not** start until user ratifies exemplar. Cursor ready on GO.
> Please critique: hybrid shell+mount vs packing LinkRM as full `R02_00`; shutter `setNextStayNo` vs forced `loadRoom` on field.

#### Paste for History (2026-07-19 — №58-B warm + hold-fade + demo lock)

> History №58-B absorbed (№59 Sturgeon/furniture data already live — no engine work). Cursor shipped (`build_run.bat` green; GPU caches wiped).
> - **Warm:** after EXT_BG0 hold complete, stagger-create all other BG payloads into `s_bgMountIds` (skip shelved EXT_BG7). First door press reuses.
> - **Wait-load:** if cached mount exists but `!mBgReady`, enter WaitIsland — never erase+recreate (that was the mash).
> - **Hold fade:** while resLoad in-flight, keep black; abort only `create_gone` (45f) or hard `no_compleate` (900f). Soft 180f retired.
> - **№53-A demo lock:** `DEMO_DOOR_OPEN` + original-demo through fade; release on hold complete / abort.
> **USER VERIFY:** Sturgeon study + Aj1 spawn; single-press doors; furniture; cave gate.
> Please critique: warm stagger 2f vs hitch risk; DEMO_DOOR_OPEN cutEnd without event staff.

#### Paste for History (2026-07-19 — №56+№57 exit/return/walkthrough/EXT_BG10)

> History №56+№57 absorbed (data already live). Cursor shipped engine in one pass (`build_run.bat` green; №55 covenant edits in this exe).
> - **exit knob:** at manifest `exit_door_rel` (interior KNOB00), not spawn−120.
> - **exit warp:** `return_pos`/`return_ry` via `wwToHost` + local snap + facing override on `requestBgWarpTo` (Sturgeon → UPPER door). Leave-shell G-guard uses the same return.
> - **walkthrough=1:** radius-edge enter on island + exit near `exit_door_rel`/`spawn_rel` in interior (forest + cave, no A).
> - **EXT_BG10:** `fpcNm_EXT_BG10_e` (0x31B) + `g_profile_EXT_BG10` (mount methods); AppData `ext_bg10.ini` `socket=EXT_BG10` + `host_stage=F_SP115` so the cave gate can boot.
> **USER VERIFY:** Grandma at her door + real interior door exit; each house → own porch (Sturgeon upper); Mesa → Mesa; forest + cave walk-through both ways.
> Please critique: return land AT Nintendo PLYR vs +150 step; walkthrough exit zone = spawn_rel when no `exit_door_rel` (A_mori/Cave09).

#### Paste for History (2026-07-19 — №54 local probe + exit-at-wall)

> History №54 absorbed (data fixes already live). Cursor shipped items 4+5 (`build_run.bat` green; GPU caches wiped).
> - **4 return placement:** porch = door + 150u along `ww_ry`; `localGroundSnap` probes from refY+50 and rejects hits outside ±250 (no more sky+2000 rooftop lands). Fail-safe / exit / leave-shell all use porch helper.
> - **5 exit prop at wall:** `host+spawn_rel` MINUS 120u along manifest `spawn_ry`, rotated to `spawn_ry`. Link interior entry facing = `spawn_ry`. OPEN ledger heals empty keys so `prop='exit:<name>'` (not `?`).
> **USER VERIFY:** mailbox door → LinkRM; porch landings; wall exit knob; Orca/Sturgeon swap report if needed.
> Please critique: ±250 clamp vs door_y-only accept; −120 wall offset vs interior TGDR if ever extracted.

#### Paste for History (2026-07-19 — №53 native door system)

> History №53 absorbed (user-ratified). Cursor shipped A–E as ONE package (`build_run.bat` green; GPU caches wiped).
> - **A/B native OPEN:** doorAttention mounts arm `CANDOOR` in range; TP attention shows OPEN → `orderDoorEvent` → `doorCheck` (no stage archive) → `dExtNpcDoors_tryNativeWarp` → existing open-BCK + pinned BG warp + fade-in. TrigA faced-prop kept as fallback.
> - **C exit + sea guard:** interior exit Knob at `host+spawn_rel` (`door:exit:<name>`); leave-shell AABB G-guard teleports to porch if Link swims/walks off the shell.
> - **D anti-scramble:** one doors.ini-order spawn loop; knob params mid-byte = door index; henna0 + position reconcile stamp keys; boot log `[Doors] prop key= pos= idx=`; 1:1 assert when all live.
> - **E parking:** BG1..9 `host_pos` flipped to №53 offshore table (z=-240000, ~30k spaced) in AppData + skeleton + seed — same build as exit warp.
> - **Watchdog:** one-shot latch (no per-frame spam).
> **USER VERIFY:** 5-door sweep — each door OPEN→ITS room→exit to ITS porch; no sea; logs show correct `prop=` / `native OPEN` / reconcile 1:1.
> Please critique: doorCheck hijack vs real WW event archive; exit knob at spawn_rel vs TGDR-interior hinge; AABB sizes.

#### Paste for History (2026-07-19 — №51 faced-door prop bind)

> History №51 absorbed (void class confirmed dead; №40/51 = last door bug). Cursor shipped (`build_run.bat` green; GPU caches wiped).
> - **Root cause of ojhous2 steal:** Knob pending `door:<name>` was FIFO-only and often arrived as empty/`unknown` by Create time, so prop→DoorDef matching could not key off the instance. Also island A-press had no hard refuse when unmatched.
> - **Fix:** stamp `mDoorKey`/`mSpawnSrc` from pending at HENNA0 Create; `bindPendingSpawn(id, …)` after each Knob `fopAcM_create`; A-press uses `facedDoorAttention` (≤90° front, else nearest, ≤300u) then `findDoorByName(mDoorKey)` (knobHost fallback); **no global trigger scan on island**.
> - **Ledger:** `[Doors] req=… door='…' dist=… prop='…'`.
> **USER VERIFY:** Grandma → `door='linkrm' prop='linkrm'`; other four exterior doors each hit their own section. Native prompt/open-anim polish = later on this mount point.
> Please critique: 90° facing cone vs stricter; whether exit path should also be prop-bound.

#### Paste for History (2026-07-19 — №50-A+C+E parent-compose / BMT / two-model door)

> History №50 absorbed (A+C+E only; F = user log ferry, no warp code). Cursor shipped (`build_run.bat` green; GPU caches wiped). **№47-A idle path not touched. Ko identity LOCK stands: Ko1=ZILL / Ko2=JOEL.**
> - **A v5 parent-compose:** `extNpcSlaveJointCB` now `world = body_target × companion local bind` (`J3DGetTranslateRotateMtx` + optional scale → `MTXConcat` → `setAnmMtx` + `j3dSys.mCurrentMtx`). Identity-root heads numerically unchanged; Zill's authored −90/−90 preserved; Quill `bmarm` rides body stubs under the same rule.
> - **C body_bmt:** registry + manifest key `body_bmt`. `ko02.bmt` is **TEX1-only** (no MAT3) — applied as texture-bank replace before finish/shared-DL. Cache key includes BMT (`Ko/ko.bdl+ko02.bmt`) so Joel never poisons Zill's plain `ko.bdl`.
> - **E two-model door:** `door_visual` (default/`npc_knob.ini` = `door_a.bdl`) loaded @ `DoorDummy`; `mAttachSlave=0` → each frame `setBaseTRMtx(controller→getAnmMtx(DoorDummy))`; controller mesh `entryDL` skipped when a non-slave visual exists (kills white triangle). Open BCK still on controller.
> **USER VERIFY:** Zill upright; Quill arms posed with idle; Joel shirt distinct; doors look like doors. Then ferry №50-F `[Doors]`/`BG`/`ground` lines from ONE Grandma attempt.
> Please critique: TEX1-only BMT swap vs full material-table merge; whether DoorDummy visual should also hide controller shapes via `onInvalid` instead of skipping entryDL.

#### Paste for History (2026-07-19 — №49 v4 envelope-stage replace)

> History №49 absorbed. Cursor shipped (`build_run.bat` green; GPU caches wiped):
> - **v4 stage move:** post-calc delta REPLACE retired. Attach + joint_slave companions install `extNpcSlaveJointCB` on ModelData joints; at callback timing=0, REPLACE `setAnmMtx` from body host + `MTXCopy` into `j3dSys.mCurrentMtx` (Nintendo nodeCB pattern) so descendants recurse under the new world mtx and `calcWeightEnvelopeMtx` builds from patched mats.
> - **Quill arms:** actor_map `companion=` → registry → create pull into `mManifest.companionModel` + `companionMode=joint_slave` even when `head_model` also present. `[Bm1]` `bmarm.bdl` + `armLloc:armL,armRloc:armR` pairs resolved at heap.
> **USER VERIFY:** multi-joint soft parts seated (hair/booger/sidehair); Quill armed. Next Cursor queue per №49-C: doors (№40 + menu-warp ground).
> Please critique: shared ModelData callbacks (userArea per instance) vs per-model; whether body neck-look post-calc still needs an envelope rebuild for heads that read the looked neck.

#### Paste for History (2026-07-19 — №48 v3 root-REPLACE)

> History №48 absorbed (name-match superseded). Cursor shipped (`build_run.bat` green; GPU caches wiped). **№47-A idle path not touched.**
> - **Mechanism:** after host/slave `setBaseTRMtx(host base)` + `calc()`, `replaceJointWorld`: `delta = M * inv(current)`; apply `delta` through the joint's descendant subtree (carries snot/hair/sidehair). Heads: REPLACE slave **root (j0)** with body `mAttachJnt` (`head_joint`). No name-equality inject.
> - **slave_map:** actor_map `companion_slave_map` → registry → `mSlaveMap` at create. Companion joint_slave: if map set → `applySlaveMap` (pairs); else root-REPLACE at neck/head. `[Ls1]` `ls_handL:handL,ls_handR:handR` pulled.
> **USER VERIFY:** seated heads through idle+talk; Aryll hands; Quill headed.
> Please critique: delta-through-subtree vs re-`recursiveCalc` after setAnmMtx(root); whether attach props (sickle) should stay root-REPLACE or get their own maps later.

#### Paste for History (2026-07-19 — №47 idle + head seat)

> History №47 absorbed. Cursor shipped A+B (`build_run.bat` green; GPU caches wiped):
> - **A root cause:** N3 `animMatchesModel` refused WW short body idles (`wait.bck` on `ym.bdl` → false) ⇒ McaMorf NULL anm = bind-pose T-pose until talk rebound. **Fix:** `animAllowedOnBody` — allow non-prefixed same-arc anims; still refuse sibling-family (`kohead*` on `ko`). Idle bind in heap + explicit `setMountAnimation(idle)` at create-COMPLEATE (all lanes).
> - **B:** attach `setBaseTRMtx(host->getBaseTRMtx())` (not head-joint); each frame `calc()` then `applyJointSlaveModels(host, attach)` — same inject-by-name as №36-A companion. Shared helper used by companion + heads.
> **USER VERIFY:** idling on warp-in; heads seated through talk; Quill headed. D residuals (menu warp / Grandma door) not this build.
> Please critique: inject-all-matching-names vs inject-only `head_joint` from actor_map; whether `calc()` before inject is required for WW head BDLs with only a root+head joint.

#### Paste for History (2026-07-19 — №45 heads pull + №46 shader guard)

> History №45+№46 absorbed. Cursor shipped (`build_run.bat` green; Dusklight/Dusk `dawn_cache`+`pipeline_cache` wiped):
> - **№45 PULL:** at `rescanProviders`, parse each mod `population/actor_map.ini` → `s_headRegistry["PROC:arg"]` (`head_model` / `head_joint` / `head_from_params`). At create, if no attach yet → `pullHeadFromRegistry` (batch-proof; no transient handoff). Log: `registry pull head '…'`.
> - **№45 FIFO:** population + knobs `pushPendingSpawn` **before** `fopAcM_create` (id-bind-after was too late when Create ran sync). Failed create drains FIFO. Stubs still `takePending` → force attach/src for that Create (P1a/P1b same-arg collisions).
> - **№45 ledger:** `src=` from pending; LS/companion → `head=companion:<model>` (not MISS).
> - **№46 F1:** `tev_alpha_op` COMP_* uses scalar `round(tev_overflow_f32(…) * 255.0)` — no `.r`/`.rg` on f32 (fixes hash `d386602d549677f` class).
> - **№46 F2:** UncapturedErrorCallback: Validation mentioning ShaderModule/WGSL/RenderPipeline → `Log.error` only when initialized; `pipeline.cpp` null-shader guard; `get_pipeline` false on null → skip-draw.
> **USER VERIFY next** (hold was correct — old build was headless): Outset folk heads; grep `[Spawn]` zero MISS for YM/YM2/YW/KO/KO2/OB/LS; Tetra/P1 no FATAL. Then №43 checklist.
> Please critique: registry key uses `socketArg` if ≥0 else params arg — OK vs actor_map `arg=`?; whether companion should also register from actor_map (LS is manifest-only today).

#### Paste for History (2026-07-19 — №44 head race + ExtNpc pick)

> Log `dusklight-20260719-092849` smoking gun: `[ExtNpcMount] NPC_KNOB forced attach 'p1a_head.bdl'` then every folk `[Spawn] … head=MISS:no_head_attach` / `src=unknown`. **Root cause (engine):** single-slot `forceNextAttach`/`forceNextCreateProc`/`forceNextSpawnSrc` overwritten across async `fopAcM_create` (same class as №18a skipBtp). Population set pins; knobs/later rows stole them before Create ran.
> **Fix:** `dExtNpcMount_bindPendingSpawn(id,…)` after create returns; stubs `takePendingSpawn` by `fopAcM_GetID` then sync-apply attach/src for that Create only. Pop + doors converted; BG warp still may use forceNext (single).
> **Pick:** Editor clicks registered but hit TP `Mhint` — ExtNpc setID `0xFFFF` not in DZR buffer join. `pick.cpp` now also considers live ExtNpc mounts (skips EXT_BG cull giants). HUD: `ID #n … proc=… head=…@…`.
> Please critique: pending map lifetime if Create never runs; whether BG warp should also id-bind.

#### Paste for History (2026-07-19 — №43 rebuild clear)

> History №43 absorbed — `ext_bg9` full schema + `[amori] knob=1` confirmed on disk. Cursor: `build_run.bat` green (prior sky-park + №38–40 already in tree); GPU caches wiped. **Handed to USER VERIFY PASS** per №43 checklist (Grandma→LinkRM; forest→Tetra+3Bk no Moblins; `[Spawn]`; heads/hands; Vlupy; facing doors). Ferry play log lines if anything mismatches.

#### Paste for History (2026-07-19 — №42 absorb + sky-park engine)

> History №42 absorbed — **Cursor did NOT re-author** mo2/bk/lwood manifests or actor_map (your values stand; Zl1 global `beat.tetra` removal noted).
> **Engine shipped:** №42 sky-park skip in `d_ext_npc_population` — `pos.y > hostY+20000` → skip + `[ExtNpcPop] skip sky-parked …` (covers A_mori P1a @ y=34707).
> **BLOCKER (History data — Cursor will not invent):** live `npc/ext_bg9.ini` still lacks `population=interior_placements.csv`, `socket=NPC_KDK`/`socket_arg=9`, and `host_stage`+`host_pos` (warp refuse path needs them — see other EXT_BG*). `[amori]` also missing `knob=1`. Until those land, forest door cannot green-transit / cannot pop ACT0 Tetra+Bk. A_mori rows are already in `interior_placements.csv` (172). Please complete ext_bg9+[amori] knob; Cursor rebuilds after.

#### Paste for History (2026-07-19 — №38 E1/E2 + №40 door select)

> History №38–№40. Cursor engine (sources in main; **not yet on clean-HEAD play exe** — FPS factory soft-poison, separate chat):
> - **E1:** `resolveSocket` — exact socket+arg only; **removed** `socket_arg<0` wildcard fallback; REFUSE + `[Spawn] … REFUSED` / E1 warn (no Ganondorf-by-default).
> - **E2:** `[Spawn] src= proc= arg= mod= head=` after head pin (census:/door:/unknown); pop sets `census:Name@(x,y,z)`; knobs `door:<id>`.
> - **E3:** pop warns when folk row lacks head_model; ledger `head=MISS:no_head_attach` when empty on non-static.
> - **№40:** A-press enter uses **nearest doorAttention Knob ≤300u**, then DoorDef by knobHost bind from spawn — not global `doors.ini` radius scan (fixes Grandma→Ojhous2).
> Please critique: knobHost bind vs storing fpc id; whether exit path needs the same; E1 impact on any intentional wildcard manifests.

#### Paste for History (2026-07-19 — №36+№37 ship)

> User “yes get started on all” after №36 decomp + №37 no-grey-area packets. Cursor shipped (investigate incremental rebuild green; main CMake re-ran once → exe recovered via `dusklight-fps-investigate` + copy; GPU caches wiped):
> - **№36 A:** `companion_mode=joint_slave` — after body `modelCalc`, copy body anmMtx → companion joints by **name** (no hand BCK). `NPC_LS` + `lshand.bdl`.
> - **№36 B / №37.2:** census heads fixed — Ym1→`ymhead01`, Ym2→`ymhead02`, Ko1/Ko2/Yw1/Ob1 as table; `disable_pos=` skips Sue-Belle alt `(-203752,481,317449)` + Joel alt `(-203870,495,317220)`.
> - **№36 C / №31:** `[item]→NPC_VLUPY` (`vlupy.bdl`+`vlupy.brk`/`vlupy.btk`); tev frame = item−1 (silver 0x0F→6); near Link → `execItemGet` TP wallet (WW 0x0F→TP 0x07) + delete. Visual-purity path; №35 TP-pickup carve-out for **models** rescinded.
> - **№37.1:** door A-press logs `[Doors] req=/resolved=/anchor=/spawn=`; guarded warp aborts on `no_compleate` / `no_ground` with reason.
> - **№37.3:** Knob = `door.bdl` + `door_open_bck=dooropenadoor.bck`; TGDR pos+rotY (no Y nudge); play open BCK ~28f then warp.
> - **Folder:** live `WW-Crew-Restoration` (Vlupy/Knob present); seed ran AppData+skeleton.
> Please critique joint_slave name-copy vs Nintendo hand callbacks; Vlupy frame map vs tww `mTevFrm`; whether dooropen needs DoorDummy attach for knob variants `door_a..h`.

#### Paste for History (2026-07-19 — №32 B1–B10)

> History №32 consolidated build list. Cursor shipped all B1–B10 (rebuild green; accuracy-first):
> - **B10:** `acquire_audio_shadow` size-mismatch ⇒ REFUSE + log + vanilla bank (№28 invariant)
> - **B1:** real `arcs/Knob.arc` / `door_d.bdl`; `static=1`+`door=1`; walk-through retired; near+A → `startFadeOut` → pinned `requestBgWarp`; log `[Doors] enter <proc> → <proc> (pinned BG warp via door '…')`
> - **B2:** Omori door removed from `doors.ini`; EXT_BG7 label “Forest Haven (shelved W6)”, no warp row
> - **B3:** `population=interior_placements.csv` + `population_stage=<arc>` on LinkRM/Ojhous/Ojhous2/Omasao/Onobuta; CSV stage-col parse; `Ba.arc`/`Ji.arc` restored; `NPC_BA`/`NPC_JI` (+ ba_cloth companion)
> - **B4:** WW-native props only (№31): Kusa/`long.bmd`, Rflw, Yaflw00, Ptubo, Toripost, Oyashi, Kanban — mapped kusax*/flower/pflower/kotubo/ootubo1/Tpost/…
> - **B5:** Ko1→`kohead01.bdl`, Ko2→`kohead02.bdl` (name-locked, not params)
> - **B6:** dialogue honors literal `\n` + real newlines + legacy ` / `
> - **B7:** pigs `carryable=1` ⇒ CARRY attention only (no SPEAK/dialogue)
> - **B8:** Zl1 `spawn_if_flag=beat.tetra`, Dk `beat.helmaroc` (invisible until flags)
> - **B9:** Cycle head / Lock identity path unchanged; Ko heads fixed as above
> Seed: `python tools/ww_crew_restoration_skeleton/seed_o2_content.py`. Overlay count ~42 curated (+Knob/Ji/props). Mesa outdoor TGDR still absent (no invented coords). Please critique door pin log + interior Ba1 + audio refuse.

#### Paste for History (2026-07-19 — №27 N1–N6 + F3)

> History №27 village-alive + next course. Cursor shipped (rebuild green):
> - **N1:** ACTR/SCO* + ACT0 only; `[layers]` gates ACT2+ via mod flags; `unique=1` folk dedupe (herds `unique=0`)
> - **N2:** `head_from_params=` / `head_model=` → `forceNextAttach`; skip `*head*` census names as bodies
> - **N3:** `animMatchesModel` prefix bind; mismatch/missing idle → static NULL anm; Ko body uses `ko_wait01` (not kohead BCKs)
> - **N4:** display = `Census ? (unverified)`; Cut Actors **Cycle head** + **Lock identity** → `population/identity.ini` (no invented names)
> - **N5:** dialogue `ww_ref=N` loads `ww_dialogue_full.txt`; Q1–Q6 + folk + `depart.offer` on pirates
> - **N6:** Kamome orbit; `hide_vrbox` on EXT_BG0; `NPC_KNOB` Kanban stand-ins at doors; Dk scenery + `dk_kamen` attach
> - **F3:** ModelData cache publish-on-success + abort eviction
> Expect: one Ls1/Yw1/Ko1; headed kids; no shatter on Joel; gulls move; no yellow void; departure lines from BMG.

#### Paste for History (2026-07-18 — №25 F1+F2 + actor_map)

> History №25: doors→Ojhous + actors on seafloor.
> Cursor shipped:
> - **F1:** door enter = pinned `requestBgWarp` (same as menu); BG reuse validates `mManifest.proc`; closest-door pick; erase stale cache entries
> - **F2:** populate at Link-place after BG settle; ground-snap sanity (reject probe >500 below authored home; retry ≤90f); skip salvage rows `world_y < host−2000`
> - **actor_map:** day-1 census → folk procs (Ls/Zl/Ob/Ko/Ym/Yw/Ah/Aj/Dk/Kb/Kn/Bb/Kamome + pirates/Mk); labels `? (unverified)`; Oyashi/Kanban deferred (no BCK)
> Rebuild green. Expect: door log `enter 'EXT_BG1' … (pinned BG warp)` + `BG COMPLEATE EXT_BG1`; pop actors stay near authored Y (not y≈-948).

#### Paste for History (2026-07-18 — №24 D1+D2+D3)

> History №24 diagnosed: warp `EXT_BG1` created as `EXT_BG0` (wildcard `socket_arg=-1`).
> Cursor shipped:
> - **D1:** `resolveSocket` exact `socket_arg` first; wildcard last; `forceNextCreateProc` on BG warp + population; stubs consume force; `EXT_BG0` now `socket_arg=0`
> - **D2:** BG create refuses missing `anchor=`; interiors already `anchor=0,0,0`
> - **D3:** population formula unchanged (`host+(ww−anchor)`) + logs first 8 world pos + host/anchor on summary
> Rebuild green. Expect log: `BG COMPLEATE EXT_BG1` (not BG0) on LinkRM; `[ExtNpcPop] spawn #N … world=(near -60k)`.

#### Paste for History (2026-07-18 — Phase O2 full-GO wave)

> User FULL O2 GO. Cursor shipped continuous wave (rebuild green):
> - **R-O2a v2**: 616-bit field in reserve (v1 migrates → flags reset once). `set(false)` clears bit.
> - **R-O2b/c**: `action=grant:0x3E` (Hawk Eye), `clear_flag=`, plus prior if/unless/set/next/else
> - **R-O2d**: `carryable=1` → CARRY attention + LIGHT carry type (pickup polish still open)
> - **R-O2e**: `spawn_if_flag=` / `spawn_unless_flag=` on manifests + actor_map rows
> - **Population**: BG `population=csv` + `population/actor_map.ini`; spawn after BG hold (`host+(csv−anchor)`); day-1 chunks ACTR/ACT0/SCOB/SCO0
> - **Interiors**: EXT_BG1..8 warp rows (LinkRM/UG/Ojhous/…/Pjavdou), socket NPC_KDK args 1–8, anchor=0 local
> - **Quests folder**: `dialogue/outset_quests.txt` Q1–Q6; stand-ins NPC_Q1/Q2/Q3 (Watchtower/Trainer/Caretaker) on HENNA0 args 2–4; grants Hawk Eye / Sword / Kokiri clothes
> Seed: `python tools/ww_crew_restoration_skeleton/seed_o2_content.py`
> Please review actor_map coverage, interior host_pos layout, and v1→v2 reset.

#### Paste for History (2026-07-18 — Phase O2 R-O2a/b shipped)

> User GO on Phase O2. Cursor shipped generic receivers (zero WW names in code):
> - **R-O2a** `d_ext_mod_flags`: per-mod bool KV in `dSv_reserve_c` bytes 1–79 (magic/ver + 18 u32 entries). Save/autosave/slot-copy for free.
> - **R-O2b** dialogue directives under `[section]`: `if_flag=` / `unless_flag=` / `set_flag=` / `next=` / `else=` (else = branch when gate fails). G1 refuse preserved. `set_flag` on successful open; `next` chains on dismiss; empty+next = silent hop.
> Sample `dialogue/sample.txt` demonstrates Ivan first-meet → return line via `mk.met`.
> Rebuild green. Please critique capacity (18 flags) + else= addition before Q1 content / R-O2c grant.

#### Paste for History (2026-07-18 — A1–A7 folded + rebuild)

> User “go for it” + History A1–A7 folded on top of M3:
> - **A1:** proc literal is `EXT_BG0` (manifest `npc/ext_bg0.ini`); no `"OUTSET"` in exe
> - **A2:** socket+arg claim map — first claim wins, conflict logged
> - **A3:** warp refused without `anchor` + `spawn_rel`
> - **A4:** exe never auto-copies mod content; seed via `tools/.../install_skeleton.py`
> - **A5:** `tools/ww_crew_restoration_skeleton/greplist.txt` exists
> - **A6:** example ini removed; skeleton file renamed off Outset place-name
> - **A7:** warp row needs payload + readable arc
> Rebuild green. Full greplist against `dusklight.exe` = clean (incl. Outset/Ivan/Medli/…).
> AppData seeded with installer. Ready for folder-present / folder-absent playtest.
