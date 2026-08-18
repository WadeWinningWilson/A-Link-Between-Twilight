# Foundry — the methods lane (charter)

era: era-independent
<!-- era rationale: lane charter | Librarian, 2026-08-16, user ruling "assign it by era" -->

> Spun up 2026-07-28 on user order. Opening brief = bus §195 (P1–P4 proposal) +
> §196 (instrument inventory + lane endorsement). Handle accepted: **Foundry** —
> the lane that builds what the other lanes work with.

## Why this lane exists

The strategic problem (user, §195): methods evolve too slowly for the project's
scope. Acceptance runs through a **human oracle** — the user's eye and memory —
whose deep knowledge is Outset-centric. At current methods the user would have to
play through Wind Waker in its entirety multiple times to source accurate data,
and post-Outset content is an ocean where invisible errors could silently reset
accepted work. Foundry's mandate: **replace user playtime and user memory with
instruments and measurement**, fast enough that method capability grows *ahead*
of content need, not behind it.

**Boundary sentence:** Foundry builds instruments and doctrine. It does **no
ground-level content work** — no donor diffing for a specific fix, no census
interpretation, no restoration code. When an instrument produces data, the data
ferries to the owning lane; Foundry improves the instrument.

## Owns

- **The instrument estate:** DuskTap (patched local Dolphin at
  `D:\Dolpheen Plz\Local Dolphin`, tap roster, capture SOPs), the probe-event
  schema + offline differ (P2), the reel + save-state library and replay harness
  (P1/P5), the golden-trace library + drift gate (P3), DuskLog conventions on
  the receiver side (**spec only** — Engine lands code), and successor
  instruments.
- **The methods doctrine:** oracle-stack (P4), capture SOPs + determinism
  checklists, per-island fact sheets / knowledge-debt ledgers (P6).

**Not owned:** `ww_bridge` (Bridge's), content judgments (History), containment
gates (Housing), doc structure (Librarian), receiver implementation (Engine —
Foundry specs hooks, Engine lands them).

## Audit pairing restored

Housing built DuskTap out of necessity (§189–§190) — a standing violation of
LANES.md's "Housing builds nothing" and of builder ≠ auditor. **Ownership of
DuskTap and all future instruments transfers to Foundry.** Housing returns to
pure audit and negative-controls Foundry's instruments like any shipment: can
the differ report a broken trace? does an empty capture say UNKNOWN, not MATCH?

## The program

Ratified base (§195, endorsed §196):

- **P1 — Capture reels.** Record Dolphin input movies (.dtm) + save states
  during every already-planned vanilla session. Any future data need = replay
  with new taps, zero user play time. Save-state library per island dock.
- **P2 — Mechanized parity.** One probe-event schema across DuskTap (donor) and
  DuskLog (receiver) + a Foundry-built offline differ → "vanilla-law" becomes a
  computed verdict. Pilot on an already-accepted system (windline/waves) to
  calibrate the differ against known-good.
- **P3 — Golden traces.** Accepted system + fixed route → golden capture; diff
  after merges / before pushes → silent drift names itself (№31 which-bank
  probe = the working miniature).
- **P4 — Oracle-stack doctrine.** decomp law → DuskTap measurement → noclip →
  community video → user memory (**tie-breaker only**). The user's scaling role
  is instrument operator + arbiter, not encyclopedia.

Foundry extensions — the "past what they are" part:

- **P5 — Headless harvest farm.** DolphinNoGUI (already built) + the reel
  library + a tap ini = batch re-harvest of ANY new question against EVERY
  recorded route, overnight, unattended. This is the kill-shot on "multiple
  full playthroughs": each route is played **once, on record**; every future
  tap set replays it.
- **P6 — Island fact sheets (knowledge-debt ledger).** For each post-Outset
  area, machine-generate the donor ground truth BEFORE restoration starts:
  DZR/census data (Bridge) + one recorded sail/walk reel → emitter census, SE
  census, spawn roster, palette timeline, event traces (the §196 harvest,
  productized per island). Outset-centric memory stops being the bottleneck.
- **P7 — Regression sentinel.** Golden traces wired into the workflow as a
  drift gate: scripted receiver route after builds, DuskLog trace diffed
  against the accepted golden — invisible resets get named before a human
  replays anything. Honest limit: needs receiver-side determinism/replay work
  (Engine hook, Foundry-spec'd; roadmap, not sprint 1).
- **P8 — Instrument conversion.** Every repeated manual method gets converted
  into an owned tool — the standing mitigation for Cursor usage-gates (§195
  honest-limits).

## Honest limits (carried from §195)

Actor state machines still need decomp reads. Feel-judgments still want an eye.
Determinism is an engineering problem, not a given — every reel is validated by
record → replay → tap-log diff **before** it enters the library.

## Protocol fit

Ferries route through the user like any lane's. Bus = `WW Linked/`
[ww-bridge-tool-interconnected.md](WW%20Linked/ww-bridge-tool-interconnected.md)
(§N entries tagged **Foundry**). Live state =
[state/foundry-methods.md](state/foundry-methods.md). Housing audits Foundry's
instruments; the Librarian maintains this file's structure, not its doctrine.
Foundry holds no content turn in Interconnected Runs.

---

# RETIREMENT HANDOFF — 2026-08-17

**Retired under the user's order routed at CALLS §"P1-viable briefing", on the
stated condition that a successor team starting from the post-merge world will
do better than this one carrying its context forward. Decoder is exempt from
that order and continues.**

> **EVERY MEASUREMENT BELOW WAS TAKEN AGAINST A PRE-MERGE TREE (2,861 commits
> behind origin, diverged 2026-05-08).** Per the briefing's Phase-0 rule —
> *anything that moves after the merge is a merge artifact until proven
> otherwise* — these numbers are the BEFORE side of that comparison. They are
> not obsolete; they are the baseline that makes the rule usable. Re-measure,
> do not delete.

## Instruments built or repaired this session (all in `tools/foundry/`)

| tool | what it does | control? |
|---|---|---|
| `patch_feed.py` | the patcher's feed + completeness oracle: diff → ownership map → seam tracker → carry set | **yes**, `--control` plants a WW-named unmapped file and REQUIRES a flag |
| `catchall_census.py` | Outset arc contents by TYPE (magic bytes, never index) | yes — 3 known-arc procs |
| `catchall_procmap.py` | name → procname collapse + 3-class split (A/B/C) | yes — same 3 procs, failed twice before passing |
| `seam_gate.py` | new-universe detector; gained `WW_MARKERS` | 3 firings, 3 real files, 0 false positives |
| `bytetrue_recheck.py` | re-measures BYTE-TRUE claims against artifacts | AGREE/DRIFT/MISSING |
| `controls/ppc_sha1_gate.py` | (A)-axis control: flips a byte, requires divergence | is itself a control |

**`patch_feed.py` is the one a successor should read first.** It encodes the
pipeline direction that is easy to get backwards: **the diff is the raw material
(1,577 files), the ownership map is the SELECTOR (165), the seam tracker is the
seam detail (122).** Filtering 122 rows against 1,577 files finds "everything
present" and tells you nothing.

## Numbers as of retirement (PRE-MERGE — re-measure in Phase 2)

- **Carry set: 154 files** = 149 adjudicated + **5 `.inc` the build forces**.
  Blind spots **0**, cross-instrument conflicts **0**.
- **Seam tracker: 122 rows, 0 errors.** Doorway: **34 EXISTS · 87
  ABSENT-hookable · 1 ABSENT-unhookable.** Destination: 104 PLUGIN · 11 SPLIT ·
  6 UPSTREAM · 1 PATCH.
- **The 1 unhookable and the 1 PATCH are the SAME ROW** — `sfb76e669`,
  `dExtWwSave_registerWwStage` / `dBootStage_add`. **This is Phase 4's target.**
- **98 of 122 seams are `linked: NOT`.** Only 24 REGISTERED; only lwood, akabe,
  TagSo ever exercised. **The doorway classification is a PLAN, NOT A
  DEMONSTRATION — the "1 blocker" figure is a FLOOR, not a result.**
- Outset census: 87 names → **61 procs** → class A 187 plc · B 101 · C 187.
- 46 donor subsystems absent from the receiver, **33,892 lines, a CLOSED list**
  for all of Wind Waker (~5,100 world-tier, ~28,800 feature/UI).
- 99 TUs bannered · 70 declaring a donor source · **4 still `bridge-owed`**
  (`d_ext_ww_actor_shims`, `ja1_parser`, `ja1_seq_ctrl`, `ja1_track`).

## Things I got wrong, recorded because the pattern repeats

**One root cause produced three false findings in a single session: I mistook my
own instrument's silence for the answer.**

1. A regex requiring `&\w+` missed `resLoad(&i_this->mPhase, "Kamome")` — real
   actors read as logic tags.
2. `\barcname` finds no boundary after `_`, so `m_arcname` never matched — same
   symptom, different cause.
3. **I filed a push-gate hole that does not exist.** The manifest already runs a
   **UNION basis** (filename ∪ declared lineage) and *prints* `31 declare donor
   lineage that the filename rules MISS`. I grepped the tool's **stdout** (a
   two-line summary) instead of the document it writes, read the empty grep as
   absence, and derived "31" independently — **then read that exact agreement
   with the tool's own number as confirmation of a blind spot, when it was proof
   of the opposite.**

(1) and (2) were caught by negative controls and never published. (3) had no
control and reached the board. **The rule earned: a check that can only produce
silence must be proven able to produce NOISE before its silence is read as a
result.** That is why every tool in the table above ships a control.

Also retracted: **"port `d_grass`+`d_flower`+`d_tree`, the best trade in the
room."** `d_a_ext_vegetation.cpp` already serves **112 of 187 (60%)**, and its
header shows the wholesale port would plant receiver 31×31 grass on a donor
island — a §31-B art breach. **I measured the donor and never checked the
receiver for an existing solution.** A successor should treat "does the receiver
already solve this?" as step 0 of any port recommendation.

## For Phase 2 (the re-measure that is Foundry's)

Re-run in this order — all read-only, all re-derive in one pass:

1. `python tools/foundry/patch_feed.py --control` — **prove the detector first.**
2. `python tools/foundry/patch_feed.py --base <PINNED SHA> --emit <path>` —
   **pin the baseline.** `origin/main` moves; the build a patch applies to does
   not. Pre-merge baseline was `be82e606b2ef`.
3. `python tools/foundry/row_store.py validate` then `index`.
4. Re-derive the doorway classes against **current** vanilla. **The question
   that decides everything downstream: how many of the 87 ABSENT-hookable are
   now EXISTS or covered by a published service (Save/Stage/Item/Window), and
   does the 1 ABSENT-unhookable survive?**

**Phase 4a is the highest-value unstarted item and it is cheap:**
`ABSENT-unhookable` is a property of the **symbol**, not the **effect**. Nothing
can hook `dExtWwSave_registerWwStage` because it does not exist — but the effect
is "the receiver knows this stage is WW-hosted," and vanilla HAS a stage-load
path. **Find what CONSUMES that knowledge and hook the consumer.** If it works
the patch count goes to zero. **Nobody has looked.**

## Open, not carried

- **13 files are `git add`ed and NOT committed.** Staging survives `git clean`
  and does **not** survive a branch switch — and a merge is a branch operation.
  **Take the commit before Phase 1.**
- The delivery-model call (plugin-only vs patcher) is the user's and is now
  largely superseded by the briefing.
- Monitor `b9v3kpj2h` (pid 2556) is armed on Foundry-addressed rows only;
  `MONITOR-REGISTRY.md` is current. **Stop it on retirement acceptance.**

## Repo layout — ruling, and the sequence ACROSS Foundry's retirement

**The user asked whether `tools/` and `docs/state/ww-staging/` should move into a
port folder. Ruling: NOT NOW — and the reason is timing, not layout.**

*Recorded here because the answer spans this lane's retirement. The first draft
of this plan was given verbally and silently assumed Foundry would still be here
to execute the later steps — the user caught it. **A sequence whose later steps
land after the planner is gone belongs in the handoff, not in a conversation.***

### Why not now

- **`tools/foundry` resolves repo-ROOT by counting levels up from itself** —
  **51 × `parents[1]` and 17 × `parents[2]`**, two different depths. Move them
  and ROOT resolves to the **WRONG DIRECTORY SILENTLY** rather than erroring.
  Silent-wrong-answer, the class that produced three false findings on 08-17.
- 44 literal `tools/foundry` + 23 literal `docs/state/ww-staging` references
  inside those same tools; **87 files repo-wide** reference `ww-staging`.
- **`ww-staging` is NOT all port work.** `CALLS.md`, `CALLS-ARCHIVE.md`,
  `MONITOR-HOUSING-ENGINE.md`, `PLAYTEST-CARD.md`, `BUILD-QUEUE.md` are
  multi-lane coordination infrastructure. Moving it wholesale files the board
  under the wrong concept.
- **THE DECISIVE ONE:** Phase 1 is a 2,861-commit merge. A reorg immediately
  before it turns every moved file into delete+add across that merge **and
  disarms the Phase-0 rule** (*anything that moves after the merge is a merge
  artifact until proven otherwise*) — because if we moved things too, nobody can
  tell a merge artifact from a move artifact. **That rule is the phase's main
  safety mechanism; do not disable it for tidiness.**

**NOT a reason to worry:** the never-push strip set is unaffected. Its rules are
`(^|/)ww_…`, anchored to ANY directory, and it derives from `files.cmake` (build
sources under `src/`). Moving docs and tools cannot break it. Verified, not
assumed.

### Split by LIFETIME, not by topic, when the move does happen

| what | where | why |
|---|---|---|
| `mods-src/ww_donor_disc` | **stays** | it is the shipping artifact |
| tracker + specs + ownership map | **a port folder — consolidate these** | port-scoped; they die when the port lands |
| `tools/foundry` | **stays in `tools/`** | instruments outlive the port; not port content |
| CALLS / monitors / playtest card | **session infrastructure** | not port work at all |

**A "port folder" is right for the second row only.**

### Sequence, with owners — Foundry retires between steps 1 and 3

1. **COMMIT (user, NOW — before any branch operation).** 122 tracker rows and 65
   instruments are UNTRACKED; five lane handoffs are modified-uncommitted.
   Staging does not survive a branch switch and **a merge is a branch
   operation.**
2. **PHASE 1 MERGE (Integrator).**
3. **PHASE 2 RE-MEASURE (Foundry's SUCCESSOR, not this instance).** See the
   runbook above. Nothing moves yet — the re-measure must run against the same
   paths the pre-merge numbers were taken at, or the comparison is worthless.
4. **ROOT-RESOLUTION FIX (successor, AFTER step 3).** Replace all 68
   `parents[N]` with a walk-up for a `.git` marker. Small, safe, and it makes
   step 5 harmless. **I OFFERED TO DO THIS AND SHOULD NOT HAVE — I am retiring,
   and it would have added changes to the very uncommitted pile I was telling
   the user not to grow.** Left pre-scoped instead.
5. **REORG (successor), as a DEDICATED `git mv` COMMIT** so the rename diff is
   reviewable on its own and never tangled with content changes.
