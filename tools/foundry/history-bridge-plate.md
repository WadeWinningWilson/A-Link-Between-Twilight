# HISTORY/BRIDGE — THE PLATE

Work this lane has identified for ITSELF. `history_bridge_watch.py` reads this
file every pass and **wakes the lane on a timer while any item is open**, so
self-identified work resumes without the user pushing the button.

RULES, because a self-waking watcher is a noise generator if these slip:
  * One `- [ ]` per item. Tick it `- [x]` the moment it is done.
  * **AN EMPTY PLATE MEANS NO TIMED WAKE** — the watcher then only fires on
    incoming calls. That is the kill switch: clear the plate and the self-waking
    stops. Never leave a done item open "just to keep working"; that is how a
    monitor becomes a thing people mute.
  * An item must name its OWN next action, not a topic. "Drain the tracker" is a
    wish; "row the next 5 TUs off DRAIN-WORKSHEET" is an action.
  * Work ARRIVING from other lanes belongs in CALLS.md, not here. This file is
    only for work nobody else is tracking.

## OPEN

- [x] DRAIN BURN-DOWN — **DONE 2026-08-16: LEGACY-UNROWED 0 · ROWED 96 · floor 6
      == 102.** 93 TUs rowed in four batches via `drain_derive.py` (derives the
      mechanical half, REFUSES to guess `destination`). Adjudicated from each
      TU's own KIT-LINEAGE: native-port/host-plumbing/donor-port -> PLUGIN,
      mixed -> SPLIT, bridge-owed -> PLUGIN carrying an explicit DEBT.

- [x] OWED TO HOUSING — **ANSWERED 2026-08-16.** §308 is the HOST, `WwJMessage::`
      is the ENGINE, and the seam is ONE function of 22: `dExtDmesg_paginate`
      (76 engine lines vs 916 host lines). Today it transcodes; after the port it
      delegates. Additive — nothing in §308 is deleted. Bonus: paginate already
      calls `dComIfGs_getPlayerName()` on stock, which DEMONSTRATES the inline
      reachability answer rather than arguing it.

- [x] BOARD SWEEP PREDATING DELIVERY — **DONE 2026-08-16: 23 -> 8.** Fifteen
      closed, each against a named basis: 3 shipped deliverables (row_store.py,
      seam_gate + build_run wiring, the A3/A4/B1/B2/B3/B5 roadmap set), the
      1,267-line donor message spec, the itemmdl_test.h cell I had already
      flipped in the worksheet, and 9 rows that were other lanes CONFIRMING my
      findings or ruling on questions I raised — informational, nothing owed.
      The remaining 8 all carry REAL ACTION and are listed below, not ticked.

- [x] EIGHT LIVE ASSIGNMENTS SURFACED BY THE SWEEP — **7 of 8 CLOSED; (4) Outset's
      remainder is HOUSING + FOUNDRY work and is tracked in CALLS.md, which is
      where other lanes' work belongs. Off the plate by this file's own rule.** — these are the ones that
      were never work-free, now that the informational rows are out of the way:
      (1) ~~MIN-BATCH-5 carded batch~~ **CLOSED 2026-08-16: do NOT queue
      d_a_obj_wood — donor includes d/d_wood.h and the receiver has NO wood
      system (0 files by symbol). port_deps.py declares 8 systems, `wood` is not
      one, so an undeclared dependency read as CLEAN — the floor rule inverted.
      Card's "two open rows, three short" is stale: the queue carries 16.
      Also repaired a raw NUL in BUILD-QUEUE.md that made grep call it binary.**
      · (2) ~~tag_so ruling~~ **CLOSED: Housing shipped tag_so; my half was
      "spec stands". The Foundry half (cargo_rank observability) was buried in a
      row addressed to HOUSING,HISTORY so their watcher never saw it — RE-FILED
      addressed to FOUNDRY.** · (3) ~~Rider 7~~ **CLOSED: static_asserts still in
      place at registry.cpp:1057/1059; tag_so (its outstanding item) landed.** ·
      (4) ASSIGNMENT 2 — Outset on stock dusklight (the big swing) ·
      (5) ~~OWNERSHIP-WORKSHEET~~ **CLOSED: all 131 of my routed rows ruled
      (103 confirmed, 28 judged, 2 outside vocabulary).** · (6) ~~the three
      per-hunk digs~~ **CLOSED: all three ruled; MAP IS COMPLETE 162/162, 0
      unruled — the fork run's lineage confirmation is DONE, Foundry signalled.**
      · (7) ~~two conflicting handoffs~~ **CLOSED: three (not two), all
      cross-referenced; charter ruled canonical; snapshot §4 calibration folded
      into it. FOUND AN OPEN ORACLE ITEM buried there — the non-donor `susp > 0`
      clamp at d_ext_dmesg.cpp:943, flagged 08-01, still live; donor confirmed
      NOT to clamp (stb.h:71 `_20 += val`). Filed to Housing.** ·
      (8) ~~ERA axis~~ **CLOSED: all 15 cookbook recipes ruled `era: INDEPENDENT`
      — consumption-boundary methods are era-proof by construction. Negative
      result recorded: era CANNOT be keyword-classified (2 passes, both failed
      their control).**
      **REMAINING: (4) Outset — MY half (donor stage/room decode) is DELIVERED;
      Housing + Foundry still owe theirs, so the row stays open.**

- [x] WW TAG VOCABULARY vs §308 — **CLOSED 2026-08-16 by Housing's derived-tier
      read + my verification.** The "52 unclassified pairs" retire as a category:
      the donor cases groups 0-3 and group 0 codes 0/1-29/41, so porting the tier
      handles them by construction. Verified `d_mesg.cpp:427` at donor source
      (mask, code table, six PAL possessive mMsgNo values) rather than relaying.
      ALSO answered their open reachability question: 6 of `do_tag`'s 8 deps are
      live on stock (5 inline over exported `g_dComIfG_gameInfo` + 1 exported),
      2 are bridge-owed stubs. See [[reference_ww_message_reachability]].

- [x] BRIDGE DEBTS — **BOTH PAID 2026-08-16, queued, `COMPILE STATUS: UNVERIFIED`.**
      (1) anime-tag channel: donor one-slot mailbox, 0xFF sentinel,
      consumer-clears, module-static per the offset-stable law.
      (2) `fopMsgM_passwordGet`: donor 8002BE04 ported; verified against the disc
      WITHOUT a build — 6/6 real passwords resolve (Plankton/Treasure/Barnacle/
      Swabbies/Chummily/Schooner) where the stub returned "" for all six.
      Published `dExtWwMsg_textByMsgNo()` — the donor mMsgNo scan — which is also
      what Housing needs for the §324 talk-path defect.

- [x] WIRE `dExtWwShims_resetMesgAnimeTag()` — **DONE 2026-08-16: wired at
      ww_stage_loader.cpp:467 inside DUSK_WW_STAGE_SEAM, beside the §773
      model-cache eviction. Same lifecycle, established precedent, WW-scoped by
      construction. QUEUED, COMPILE STATUS: UNVERIFIED.**
      Closes the window where a module-static outlives a stage change while the
      donor's game-info field (@0x493C) does not — a stale tag firing one wrong
      animation after a transition. I had parked this as "belongs with whoever
      lands the derived tier", reasoning it was shared lifecycle. The parking was
      right; the assumption that no WW-only seam existed was not.

- [x] ORACLE ITEM (6) `d09 item-box mid-tale` — **NOT MINE, AND CHECKED BEFORE
      CLAIMING IT.** Housing/Engine desk-checked it the same day (STB fukiKind by
      index :698 -> ensureBoxBuilt(9); hold contract donor-faithful) and filed a
      BUILD-QUEUE row in the SAME TU as the §324 re-key so one build gates both.
      It stays `⬜` because it is a RUNTIME audit — it needs a tale boot showing
      msg 3095 in the d09 box. Auditing it here would have been the D1-walker
      collision, not diligence. Item (2) is ✅ FIXED, both sites, my receipt cited.

## DONE

- [x] §801 ADMISSIBILITY RULING — ruled admissible under labelling, with a
      falsifiable completeness test (dtk address ranges) and a negative control
      (trips on 2 of 869 TUs). `BYTE-TRUE` barred from this provenance.
- [x] MONITOR DELIVERY — watcher now exits to deliver instead of accumulating.

- [x] **Yaz0 wiring in `mods-src/ww_donor_disc/ww_message.cpp`** — WIRED + BUILDS 2026-08-18 (donor `decodeSZS` ported, never-evict cache, premise corrected in place). **RUNTIME UNOBSERVED — needs a boot before anyone calls it working.**
  ~~superseded description follows~~ — the donor
  `JKRDecomp::decodeSZS` transcription is PROVEN 12/12 against real archives;
  what remains is the OWNERSHIP design, not the algorithm.
  `rarcFindMember` returns a NON-OWNING pointer and `wwMessage_open` caches
  views into it (`s_base`/`s_inf1`/`s_dat1`), so a decompress into a local
  dangles. Needs a module-owned decompressed cache that outlives the open
  reader, freed only where `wwMessage_close()` already drops its pointers.
  Replace the `:104` refusal with decompress-then-parse and CORRECT the false
  premise comment ("the donor message archives are measured uncompressed" —
  12 of 35 in `res/Msg` are Yaz0; 640 of 1321 disc-wide).
  **MUST BUILD BEFORE ANY STATUS CLAIM** — this lane's DO-NOT-TRUST section
  records that its pre-§988 clean-compile reports were worthless.
- [x] **Job ② — SPLIT tracker row `sfb76e669`** — DONE 2026-08-18: split into `sfb76e669` (dBootStage_add, PATCH) + new `s815faa6d` (dExtWwSave_registerWwStage, FORK). 123 rows, 0 errors, indexed. My "missing schema term" blocker was FALSE — `FORK` was declared all along with zero users.
  ~~old description~~ (measured 2026-08-18; scoping
  DONE, see the filed row). Population is ONE row, not twelve. It mixes two
  symbols under one label: `dBootStage_add` keeps `ABSENT-unhookable`
  (re-verified against the epoch-2 StageService, which is actor-level only and
  does NOT expose stage registration); `dExtWwSave_registerWwStage` is a
  WRONG-PLACE/fork-side verdict. **BLOCKED: `_schema.json` has no WRONG-PLACE
  term — that vocabulary call is Integrator's or the user's, not mine.**
- [x] **CHECK `s49f0caba` AGAINST SaveService** — CLOSED 2026-08-18, answer is
  NO. Housing/Engine measured it and I verified at `save.h`: `set_blob`/`get_blob`
  are CURRENT-SLOT only, `peek_blob(slot,…)` gives cross-slot READ, there is no
  cross-slot WRITE. `eraseSlot`/`copySlot` run from the file-select screen where
  no slot is active (`MOD_UNAVAILABLE`), so the SPLIT is CORRECT AS FILED.
  **Doorway re-measure closes at ZERO rows moved by epoch 2 — a measured null.**
  Residual ask is INTEGRATOR's, not mine: one slot-targeted write mirroring
  `peek_blob`. (superseded item, kept for the record)

  doorway re-measure (CLOSED 2026-08-18: 122 rows → 23 domain → 19 ABSENT →
  16 already PLUGIN → 3 → 1; the 14 `window` candidates were a homonym,
  `WindowService` is host desktop-window management, not J2D). This row is
  `src/d/d_ext_save_flags.cpp`, `SPLIT` / `ABSENT-hookable`. SaveService gives
  `set_blob`/`get_blob`/`peek_blob` + `observe_saves`. **If that covers the WW
  save-flag shim, the SPLIT collapses to a clean PLUGIN route.**

- [x] **PHASE 2 — the doorway re-measure** — SOLVED 2026-08-18: the 21 "flips" are a DONOR-vs-RECEIVER frame mismatch (rows cite `WW DP/configure.py`, validator measures the TP binary). Merge moved ZERO seams; the 1 ABSENT-unhookable survives. Residual is Integrator's design call on which frame `row_doorway.py` should validate.
  ~~old~~ **PHASE 2 — the doorway re-measure** (Integrator's routing, 2026-08-18).
  Raw: 119/123 STALE. NOT yet a finding. Vanilla image VALIDATED (post-merge
  `c880d46fb5`, exe 08-17 20:02). Mangling hypothesis FALSIFIED. Blocked on:
  (1) a CURRENT fork build — ours is 08-17 21:08 vs HEAD 23:00, so `n_own` is
  two hours blind; (2) a ruling on whether planned-port rows are exempt from
  the `n_own == 0` check, since an ABSENT-hookable row cites code that does not
  exist yet BY DESIGN and the check fires on the row's own premise.
  Report per ROW on the "declared ABSENT-* but PRESENT on vanilla" bucket (69
  symbol-level hits) — that is the bucket the briefing actually asks about.
