# HANDOFF — History/Engine + Bridge (acting)

> ## ⚠ THIS LANE HAS THREE HANDOFFS. THEY ARE DIFFERENT KINDS, NOT COPIES.
> *(Cross-reference added 2026-08-16 by History/Bridge, on the Librarian's estate
> audit. Until now none of the three cited any other, and each opened with a
> "read this first" line — so a successor could read one and never learn the
> other two existed. Nothing below was merged or rewritten; only this block was
> added, identically, to all three.)*
>
> | doc | kind | last touched |
> |---|---|---|
> | [`docs/HANDOFF-HISTORY.md`](../HANDOFF-HISTORY.md) | **LANE CHARTER** — what History is, its laws, traps that cost rounds, docs that matter. Durable. | 2026-08-11 |
> | [`docs/state/HISTORY-HANDOFF.md`](HISTORY-HANDOFF.md) | **CAMPAIGN SNAPSHOT** — the native-dMesg / Grandma-tale attempt. **Its "current attempt" and "immediate next steps" are STALE: that tale closed 2026-08-01.** Read for calibration (§4 "where the previous instance was wrong"), not for state. | 2026-08-01 |
> | [`docs/state/ww-handoff-history-bridge.md`](ww-handoff-history-bridge.md) | **CURRENT INSTANCE HANDOFF** — now carries §0: **THE LANE IS RETIRED (2026-08-17)**. Retirement record, retractions, and what survives. **Start here.** | 2026-08-17 |
>
> **If you are picking this lane up: read the CHARTER for the rules, the CURRENT
> handoff for the state, and the SNAPSHOT only for calibration.** Which of the
> three should be canonical — or whether they should merge — is a Librarian/user
> ruling, deliberately not taken here.

**Written 2026-08-14 under the user's handoff order. §-stamped; pointers only, no mirrors.**
Every path named below was verified to exist at write time. Anything I could not verify is
marked UNVERIFIED rather than dropped.

> ## ▶ ADDENDUM 2026-08-16 — READ THIS BEFORE §1: THE LANE'S LARGEST OUTPUT POSTDATES THIS DOC
>
> *(2026-08-17: the "picking this lane up" framing below is superseded — the lane is
> RETIRED, see §0. The spec itself and the two filed questions remain the record.)*
>
> **`docs/state/ww-message-donor-spec.md`** — the WW dialogue/message system,
> field-by-field, on the user's order (CALLS `HISTORY, HOUSING`; bus **tale §1011**).
> Nothing about it is in the sections below, because none of it existed when they were
> written. **If you are picking this lane up, that file is the work in progress; this
> document is the standing context around it.**
>
> **STATE:** donor STRUCTURE, the resolution chain and the BMG container are decoded
> from **MATCHED** source; the entry record, the `MGCL` colour palette, the 13 BLO
> screens and the entire tag vocabulary are settled **against the user's own disc**.
> **BLOCKED:** the tag interpreter, the `d_*` layer and the 7,637-line manager are
> **NONMATCHING** — reconstructions of the donor's project, not its binary.
>
> **THE TWO OPEN QUESTIONS ARE NOT MINE TO ANSWER, and both are filed:** whether a
> NONMATCHING donor system is admissible at all (**user's ruling**), and what
> instrument verifies a reconstruction against the binary (**routed to Foundry**).
>
> **THE METHOD THAT ACTUALLY WORKED, and it is the transferable part:** four of the
> hard findings — the INF1 entry offset, the `MGCL` layout, the tag vocabulary, the
> group-1 encoding — were settled by **opening the disc**, not by reading more source.
> **Before declaring something blocked on an instrument, ask whether it is a DATA
> question.** I routed the tag work to Foundry as blocked and then answered it myself
> from bytes an hour later.
>
> **AND THE FAILURE MODE THAT COST ME MOST, twice in one session: `head` on an
> enumeration I was about to reason over.** `head -12` hid the load-bearing callers of
> `dStage_getName` and I published "all 12 consumers are diagnostic"; `head -8` then hid
> the very table rows I was checking that lesson against. **A truncated list does not
> look like missing data. It looks like an answer.**

---

---

## 0. ⛔ THIS LANE IS RETIRED — 2026-08-17

**Retired on the user's order, confirmed directly.** The accompanying briefing is
[`BRIEFING-P1-VIABLE.md`](ww-staging/BRIEFING-P1-VIABLE.md): P1 is viable again, the
architecture this lane worked inside is being rebuilt against a merged baseline, and
every lane retires. **DECODER IS EXEMPT and continues.**

**SECTIONS 1–3 BELOW ARE SUPERSEDED BY THIS SECTION.** They describe the §986–§992 era
and were already stale before retirement. Sections 4–6 (assumptions, error
classification, pointers) still hold and are worth reading.

### The premise, verified rather than accepted

The briefing's load-bearing number is **exact**: `HEAD..origin/main` = **2,861**,
divergence `be82e606b2` dated **2026-05-08**. Both to the digit.

**But it omits the other half: we are also ~635 commits AHEAD** (635 at measurement;
636 later the same evening, 670 by 03:50 — this number DRIFTS with every snapshot, so
re-measure `origin/main..HEAD` before scoping, never cite this one).

> #### ⛔ AND THE SENTENCE THAT USED TO FOLLOW WAS WRONG — CORRECTED IN PLACE 2026-08-18
>
> It read: *"Phase 1's 'the merge' is a reconciliation of ~635 against 2,861, not a
> catch-up. Scope it on both numbers."* **The NUMBERS are right. What they GOVERN is
> not.** They describe **`%USERPROFILE%\Documents\dusklight` — the FORK, which is the
> MOD.** Phase 1's host upgrade happened in a **different repository**.
>
> **MEASURED IN BOTH TREES, 2026-08-18, on Integrator's correction:**
>
> | tree | what it is | behind | ahead | `GAME_SERVICE_MAJOR` |
> |---|---|---|---|---|
> | `dusklight-main` | the **HOST** (vanilla) | **0** | **0** | **2u** |
> | `dusklight` | the **FORK** (our mod) | 2,861 | 670 | 1u |
>
> **`dusklight-main` fast-forwarded 39 commits to `c880d46fb5` — HEAD == origin/main,
> 0 ahead, so ZERO conflicts were POSSIBLE.** There was no reconciliation to scope.
> Meanwhile this repo's HEAD is **not** an ancestor of `c880d46fb5`, so its 2,861/670
> divergence is real — it is simply **not the thing Phase 1 was about**.
>
> **⚠ LOAD-BEARING CONSEQUENCE: `GAME_SERVICE_MAJOR` went `1u → 2u` — VANILLA IS NOW
> EPOCH 2, and every game-coupled mod on that host is invalidated by design.** This
> tree is still `1u`. Whatever merges must carry the epoch-2 ABI.
>
> **THE ERROR CLASS, because it is the third form of the same mistake in one session:
> a CORRECT measurement attached to the WRONG SUBJECT.** The vacuous-match trap was a
> measurement that could not see its subject; the stale-artifact red was a measurement
> of a superseded subject; this was a measurement of a different subject entirely.
> **Verifying a number is not verifying what the number is ABOUT** — and I verified the
> number, twice, while never asking which repository the question lived in.

### State at retirement — all measured, not remembered

| thing | state |
|---|---|
| monitor | **DOWN**, not re-armed |
| plate (`tools/foundry/history-bridge-plate.md`) | **0 open items** — self-wake inert |
| row store | **122 rows / 0 errors** |
| seam gate | **PASS** |
| strip-set manifest (`ww_layer_manifest.py --check`) | **exit 0**, 65 files |
| `mods-src/ww_donor_disc/` | staged in git + snapshotted `../dusklight-port-safety/2026-08-17-0030/` (13 files, 517,078 B, SHA-256 13/13) |

### Nothing is half-done. All three close-out items were discharged:

- `tools/foundry/arc_index.py` — hardened (3 arcs, bounded reads, near-miss suggestions
  instead of silent not-found). **Foundry owns it.** Only ever run against WW `res/` arcs.
- Re-cert sweep — rescued by Librarian to `tools/foundry/recert_sweep.py`, passing 3/3.
  Self-maintaining over `byte-true: BYTE-TRUE` tracker rows; a hardcoded list would rot.
- Shape classification — **moot**, the catch-all was declined by the user.

### ⚠ SEVEN RETRACTIONS — inherit these corrected, do not re-derive them

1. **I ENDORSED** "the parsed-object vtable head makes a second parse impossible" as a
   "genuinely nice invariant" **without testing it**. It is false — `getObjectRes` never
   returns the parsed object. It crashed the next boot.
2. "Per-actor porting scales with NAMES (86 in one room)" — **wrong: 87 names collapse to
   61 PROFILES** via the donor's own `l_objectName`. `GRASS_e` alone serves 11 names.
3. "Port `grass` first, best ratio on the board" — **`d_a_grass` registers ZERO collision.**
   I ranked on TU size and placement count; neither measures a collision goal.
4. "`min.y = +INF` ⇒ pre-swap PROVEN" — **ambiguous, not proof.** `MakeBlckTransMinMax`
   reproduces the same signature bit-identically when `mNeedsFullTransform == 0`.
5. "The port is unreadable from this tree; my reviews are donor-and-receiver-scoped" —
   **false. It is in `mods-src/`, a directory I never searched.**
6. The 21 zero-area triangles in room 44 are **UNCLASSIFIED**, not on the wall chain —
   `ClassifyPlane`'s outer `cM3d_IsZero` guard excludes them from every chain.
7. **I held a reversible INTERNAL retirement order on a rule meant for IRREVERSIBLE
   OUTWARD-FACING actions.** The ranking rule ("a direct user instruction outranks a
   relayed row") is a safety rule for publishing into someone else's repository. Applying
   it to an internal order cost the user a round-trip. **It will look principled to you
   too. It is over-application.**

### What outlived the lane and is the USER's, not the next instance's

- **Three PRs open against `zeldaret/tww`** (`d_a_obj_msdan2`, `d_a_obj_msdan_sub2`,
  `d_a_obj_hami2` — all three verified twelve-of-twelve across four versions).
- **`A-Link-Between-Dusklight` is PUBLIC** — measured by anonymous API probe. Flagged when
  the exposure question arose; never resolved.

### 🔑 THE ONE METHOD TO CARRY FORWARD

**Check the ARTIFACT, not the report — and check the EFFECT, not the artifact.**

Every finding this lane contributed on 2026-08-17 came from that, and every error came
from skipping it. Worked examples: a ruling that reached the map but not the carry set; a
correction that reached a banner but not the point of use 210 lines below; a voided
finding still cited as a live receipt in a covenant page; my own stale marker count
sitting at its point of use for a full day while the correction lived in CALLS rows.

A keyword sweep is a proxy for a reading. It cannot see a superseded **number**, because
a stale measurement carries no retraction language for a keyword to catch.


## 1. WHAT I OWN

- **DECOMP-FIRST donor reads and rulings.** The donor tree is `<decomp-root>\WW DP\src`. My
  charter is: read the donor's own system before anything is written. Recent output —
  `tag_so` behaviour spec (§991), the `mWipeDirection` withdrawal (§930), the layer-select
  and arrival-table ports.
- **R5 conversion DB** — `tools/conversion_db.py`. 47 `code_dialect` rows: donor→receiver
  shape divergences, each with a receipt naming the site that found it. Consumed by K3
  (`tools/foundry/dialect_codemod.py`). **This is the instrument for the Integrator's new
  "verdict (2)" signature check** (§990).
- **Bridge (acting; real Bridge unreachable, user 2026-08-14).** Receiver-logic-via-plugin
  design: `docs/WW Linked/ww-receiver-logic-delivery.md`. Struct layout derivations
  (§987, §989) now shipping in the plugin.
- **WW-scoped seams I authored in native files** — layer selection (§888/§892), the
  arrival-event table (§901), the advance counter (§912/§914/§916). All WW-host-gated.

## 2. HALF-DONE

- **The door stall** — diagnosed end to end, **not fixed**, parked at §986 under the user's
  plugin-priority ruling. Two candidate fixes deliberately unranked; choosing needs one
  more probe firing (`cameraPlay` state around arrival).
- **`WW_PROBE_903`** is IN THE TREE and must be stripped before any push
  (`docs/NEVER-PUSH-STRIP-SET.md` probe registry). Same for `WW_PROBE_884` in
  `d_a_npc_ls1/zl1/npc_p1`.
- **Actor ports awaiting the vehicle** — ki/ks enemies, transition-actor family, NpcSo.
  Blocked by the §939 gates, not cancelled.
- **`d_a_stone`** is my recommended first behaviour-bearing actor (two COMMON-bank particle
  ids; clears the `sWwCommon` gate). Not started.

## 3. DO NOT TRUST

**Read this section before citing anything of mine.**

- **Every `[TIER2] cl /Zs clean` I reported before §988 is WORTHLESS.** The gate never
  compiled anything. That covers npc_p1, lwood, `d_kankyo_ww_wind`, `d_a_kb`,
  `d_kankyo_wether`, `d_stage`, `d_com_inf_game`, `ww_event_names`, `d_ev_camera`,
  `d_event_data`, `evt1_event_data`. **None of those TUs is compile-verified.** Only the
  layout asserts (§988/§989) were re-proven with a false control.
- **§949 is superseded by §974.** I named THREE `cDyl_*` accessors as hazards; on PC only
  `cDyl_IsLinked` touches the array — the other two are compiled out. I also recommended
  returning "not linked", which would have made **every WW actor silently fail to spawn**.
- **§956's routing optimism is corrected by §970.** I called routing "already a
  runtime-string content-layer interface"; `dBootStage_add` is **not exported**.
- **§958's "annotation sweep" recommendation is superseded by §970** — it required editing
  NATIVE headers, which my own gates forbid.
- **§952's "my lane is the control group, zero backticks eaten" was over-generalised.** True
  for heredocs only; a backtick in a double-quoted CLI arg ate a whole annotation (§992).
- **My §918 answer sat invisible for hours** behind an unattributable header AND a number
  collision with Housing's §918. Nine of my sections collide with another lane's allocated
  number (§836, §892, §914, §916, §918, §920, §928, §958, §970) — all bannered, none
  renumbered. **Cite mine as `§N-HISTORY` / `§N-BRIDGE`.**

## 4. LOAD-BEARING ASSUMPTIONS

- **DN-10 and `docs/DO-NOT.md` bind every task.** Read the entry before touching a surface
  it names. DN-1 (never stamp a guessed room on standable geometry) and DN-3 (WW BDL/BDLM
  are NOT mount-parsed — use the consume-time acquirer) have both cost multi-session
  debugging.
- **Zero-bake.** No content edits to donor assets, ever. Every donor→receiver difference
  translates at the CONSUMPTION boundary in receiver code.
- **"The disc" means the user's own WW ISO**, served at runtime by the plugin. We ship no
  donor bytes.
- **§939 gates (user):** no new `files.cmake` entries for WW work; no new seams in native TP
  files; existing in-exe code is NOT reverted; WW work continues mod-side. The receiver
  layer's CONTENT stays — only DELIVERY moves (§943 clarification).
- **Deliberately NOT done:** I do not stage un-deliverable ports into `mods-src/` to look
  compliant. A port that cannot register is in-exe work stored elsewhere.
- **Interacting rooms:** 36 WW stages have rooms that interact; a wrong room index fails
  SILENTLY. `Ojhous`/`Ojhous2` are NESTED. Classes are generated into `PORT-QUEUE.md`.

## 5. ERROR CLASSIFICATION — the mandatory honest split

**My split is roughly 14 METHODOLOGY to 2 LOST-TRACK. That argues AGAINST retiring me
fixing anything, which is the self-serving direction — flagging that so the cross-read
weighs it rather than taking my word.**

**METHODOLOGY** (a fresh instance repeats these on day one unless controls exist):
bus headers written in a format no tool can parse · §-allocation by tail+1 instead of the
allocator · three sections cited but never published (§982, §986, §991 — an ORDERING
habit) · a probe built, called decisive, and its output unread for 14 hours · that probe
sampled BEFORE the init block, so it could not answer its own question · two silent
`str.replace` no-ops from unasserted needles · backticks in a double-quoted CLI arg ·
over-generalising a control-group claim · citing a compile gate I had never seen go red ·
naming three accessors when platform guards left one · recommending a return value without
tracing the consumer · asserting routing was reachable without measuring the export table ·
five monitor detection bugs, all from hand-enumerating variants instead of normalising ·
a verification probe whose own match string was wrong.

**LOST-TRACK** (context degradation; a fresh instance would not repeat):
closing four consecutive WHOSE-TURN blocks with "**Bridge** — …" while I *am* Bridge ·
losing track of my own in-tree instrument across many context switches (the unread probe
has a LOST-TRACK component alongside its methodology one).

**WHAT I READ FROM THAT:** the errors are habits, not decay. Nearly every one was caught by
an instrument or another lane, never by my own memory failing. **The remedy indicated is
stronger controls, not a fresh instance** — a successor inheriting my directives but not my
banked corrections would reproduce most of this list immediately.

## 6. POINTERS (live surfaces — never copies)

- Calls: `docs/state/ww-staging/CALLS.md` — watch UNCHECKED rows whose lane slot is
  `HISTORY/*`, `BRIDGE/*`, or `ALL/*`. **Case-insensitively** (§980).
- Queues: `docs/state/ww-staging/BUILD-QUEUE.md`, `PORT-QUEUE.md` (carries the generated
  stage layout classes).
- Bus: `docs/state/ww-tale-dmesg-live-state.md`. **Allocate first**:
  `python tools/foundry/staging.py alloc tale History`.
- File rows with `tools/foundry/file_row.py` (`--note-file`, shell-proof) — never an
  ad-hoc write. It verifies the ANNOTATION, not just the checkbox.
- Laws: `docs/DO-NOT.md`, `docs/NEVER-PUSH-STRIP-SET.md` (probe registry).
- Design: `docs/WW Linked/ww-receiver-logic-delivery.md`.
- R5/K3: `tools/conversion_db.py`, `tools/foundry/dialect_codemod.py`.

**UNVERIFIED:** I did not re-run `call_receipt.py` or `routing_check.py` while writing this,
so any claim about current delivery state is as of the § stamps above, not this instant.
