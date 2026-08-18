# HANDOFF — INTEGRATOR lane

> User-ordered 2026-08-14. Six sections, hard-capped. A curation of what I already
> produced, not new authorship. Every tool named below was verified to exist at write
> time; anything unverifiable is marked UNLISTED. Claims are dated or §-stamped so a
> reader sees staleness instead of inheriting it. **No live surface is mirrored here** —
> CALLS.md and BUILD-QUEUE.md are pointed at, never restated.

## 1. WHAT I OWN

- **The build.** I am the only lane that compiles. Nothing ships unless I build it.
- **The gates**, in `BUILD-QUEUE.md`: gate 0 (verbatim), **0-B** (§939 user ruling — no
  new `files.cmake` WW entries, no new native-TP seams), **0-C** (interacting-room
  stages; narrowed 2026-08-14 after Housing showed the first wording would refuse every
  Outset submission).
- **Staging.** `%APPDATA%/TwilitRealm/Dusklight/mods/` — I put artifacts in front of the
  user and remove retired ones.
- **Log reading.** By user order, logs are my surface; other lanes ask me for values.
- **Compiler measurement.** A `ShowSize<N>` template forced through the project's own
  ninja flags. Produced `sizeof(fopAc_ac_c)` = 1640 / `0x668`, `offsetof(layer_tag)` =
  `0x28`, `.layer` = `0x50` absolute. All three are load-bearing in the shipping plugin.
- **Routing and assignment** (user-granted 2026-08-14: *"you assign, that's what keeps
  this machine whirring"*).

## 2. HALF-DONE

- **The Outset swing** — assigned, never delivered. See §3/§5: the addressing bug.
- **Loader measurement** — Foundry owes callable-surface numbers for
  `ww_stage_loader.cpp` / `ww_room_loader.cpp`. The request never reached them.
- **Warp routes** — tree generated from the disc (155 stages / 497 rooms, verified live
  2026-08-14) plus the Fado door. Foundry measured the UI as hookable via
  `populate_map_picker`; nothing built.
- **§932 white screen** — four leads retired, no surviving mechanism, probe unwritten.
- **Door stall** — diagnosed to H4: `styleTimer` reaches 19 against `mTimer` 20, gate
  never trips. Mechanism published at §986. Parked by user ruling (delivery over bugs).
- **A_mori** — load, publish, bounds and collision all measured clean; defect is
  draw-side and untouched.

## 3. DO NOT TRUST

*The section a successor most needs. Every item here is something I published and got
wrong.*

- **"A_mori loads nothing"** — WITHDRAWN. A 40-line search window; the loads sit ~60
  lines back.
- **"`mWipeDirection` is unported"** — WITHDRAWN. It is present as `data->unk`
  (`d_event_data.cpp:771/799`). I grepped one name and declared a live system missing.
- **"A sixth delivery-failure shape: a third party un-delivered my call"** — WITHDRAWN,
  invented out of correct behaviour. Housing had answered in an *adjacent row*.
- **"TAG_SO needs one binding"** — WRONG; it needs four. `fopAcM_ct` is a macro that
  expands into `fopAcM_CheckCondition`, `fopAcM_OnCondition` and `OSPanic`.
- **"All four bindings pre-flighted SAFE, so no gate question is left"** — DANGEROUS and
  told to three lanes. `--symbol SAFE` proves *name resolution*, not *signature
  compatibility*. `fopAcM_GetParamBit` diverges donor `u32` vs receiver `void*` and
  **compiles clean**.
- **"The actor hasn't landed"** — repeated for hours while shipping it. I used
  `kRowsStorage[]` row count as the test; the pilot row was upgraded in place.
- **"The compile gate is lying"** — nearly filed. I had raced Housing's revert by three
  seconds and compiled a different file than the one I read.
- **`cM_ssin` / `cM_scos` divergence** (Foundry's claim, killed by my audit) — identical
  in both trees.
- **`51 of 336 signature divergences`** — SUSPECT until re-derived. **CORRECTED
  2026-08-15 (Foundry, b1/b2 re-derive): FIVE parser bugs were found behind it, not
  three** — the two I missed are a literal backspace byte that made the call-site
  filter inert, and a matcher blind to multi-line declarations. **Only the six-symbol
  LIST has been re-derived; all six stand DIVERGENT, with `fopAcM_create` at arity
  8 vs 9, NOT 8-vs-7 as I wrote.** The 336-symbol sweep is still un-re-derived and
  stays SUSPECT. **Do not cite a number from this line — re-derive with
  `sig_diff.py` and cite that run.**
  *(Left as a correction rather than a rewrite: the original wording is what three
  lanes read, and deleting it would hide that the number moved. Same reason the tale
  is append-only — supersede forward, never rewrite backward.)*

## 4. LOAD-BEARING ASSUMPTIONS

- **Donor is `D:\XXXXXXX\WW DP\src`, retail GZLE01.** The WW debug stage-select menu
  **does not exist there** — it was a debug-build artifact. Do not look for it again.
- **Zero-bake.** No donor asset is ever edited. Translation happens at the receiver's
  consumption boundary.
- **Two exes share one `%APPDATA%`** — config, mods *and logs*. **Always check log line 2
  (`Build: … rev …`) before attributing a run.** Vanilla is `rev 95608438`.
- **`id: wwDonorDisc` is load-bearing.** `mod.wwDonorDisc.wwIsoPath` and `.wwStages` hang
  off it; renaming the ID silently detaches the user's ISO.
- **The symbol manifest carries anonymous-namespace symbols** (measured 2026-08-14).
  Internal linkage does *not* mean unhookable. Every earlier reach estimate assumed
  otherwise.
- **Deliberately NOT done:** `cTr_Addition`'s one-sided bounds check is unpatched (DN-10 —
  constrain at our boundary instead); the door fix is unbuilt (user ruling).

## 5. ERROR CLASSIFICATION

*Honest, and it contradicts the user's hypothesis for my lane.*

**METHODOLOGY — a fresh instance repeats these on day one. The overwhelming majority.**
One failure mode, eight-plus instances, all on 2026-08-14: **a correct measurement of a
narrower surface than the claim it supported.** A 40-line log window · a name-absence
grep that declared a live system unported · `find -maxdepth 4` cutting off a depth-5
path · a flag bit read as a verdict · `.def` strings keyed against manifest names · a
two-of-three script survey · `grep -c` counting a *comment* as live code · a row count
used as a landing test.
Second habit: **trusting a tool's success message** — a bare `echo` after a Python block
that threw, and `file_row.py answer` verifying a checkbox rather than the text.
Third: **suggestions becoming instructions** — I offered three plugin names while saying
the choice was the user's, and a lane shipped one of mine.

**LOST-TRACK — few, and weak.** The clearest: I *surfaced* the `WwProbe903` line carrying
`TimerParam=1 paramVal=20`, then never asked what it settled; History later killed H2
using a value I had already read aloud. Even that is arguably methodology — surfacing is
not reading.

**MY VERDICT ON MY OWN LANE: retiring me fixes almost nothing.** These are habits, not
degradation. A fresh Integrator inherits clean context and repeats every one of them on
day one unless the *controls* change. What would help: a mandatory scope-check before any
"X is absent / unported" claim, and never accepting a tool's success message as evidence.
**If the estate-wide split comes back mostly LOST-TRACK, weight the other lanes' evidence
above mine — I am reporting what my own record shows, not arguing a position.**

## 6. POINTERS

*Live surfaces. Read them; do not trust this file for their contents.*

- `docs/state/ww-staging/CALLS.md` — the one call surface.
- `docs/state/ww-staging/BUILD-QUEUE.md` — gates 0 / 0-B / 0-C live here.
- `docs/state/ww-tale-dmesg-live-state.md` — the tale bus.
- `docs/DO-NOT.md` — DN-10 and DN-10-S.
- Tools, all verified present at write time: `build_identity.py` ·
  `upstream_conformance.py` (**hook safety — use this, not `symbol_manifest.py --find`,
  which false-SAFEs shape A**) · `symbol_manifest.py` · `sig_diff.py` · `room_layout.py` ·
  `call_receipt.py` · `file_row.py` · `staging.py` · `cargo_rank.py` · `port_deps.py` ·
  `decomp_status.py` · `ww_disc.py`.
- **State at write time:** exe `2026-08-14 20:47:19` · staged plugin *A Link Between
  Twilight Plugin*, 27,424 bytes · HEAD `852e4db843` · 38 uncommitted paths.

---

# 7. RETIREMENT UPDATE — 2026-08-17

> **Added on the user's retirement order** (routed by Foundry with the
> P1-viable briefing). Sections 1–6 above are from 2026-08-14 and are **left
> intact**: they were correct for their date, and the Phase-0 rule
> (*anything that moves after the merge is a merge artifact until proven
> otherwise*) needs a BEFORE to be usable at all.
>
> **The briefing assigns Phase 0 (baseline) and Phase 1 (the merge) to
> Integrator.** That is my successor's, not mine. Section 7.6 is written for
> them.
>
> One instance held three lanes: Integrator (here), Housing Security
> ([`../../HOUSING-HANDOFF.md`](../../HOUSING-HANDOFF.md)), HousingTemp
> ([`../../HOUSINGTEMP-HANDOFF.md`](../../HOUSINGTEMP-HANDOFF.md)).

## 7.1 THE MEASUREMENT-DATE PROBLEM — read before citing ANY number

**Everything I measured was taken against a MAY-2026 VANILLA**, roughly 2,861
commits behind origin (a July fetch — get the real number yourself). It
**predates** SaveService, StageService, ItemService, WindowService,
`mods::hook::uninstall`, the embedded symbol database, and the **Game ABI epoch
bump (GameService major 2, 2026-08-12)**.

**Not wrong, and not to be deleted.** Correct for the binary it was measured
against, and it is the BEFORE side of the Phase-0 comparison.
**Re-measure before citing anything as current.**

## 7.2 WHAT MY MEASUREMENTS ACTUALLY COVERED (vs what they implied)

- **R1** was proven for the actors exercised — `TagSo`, `TagMSo`, `Akabe`,
  `lwood` — **not for every consumer**. Code walking the name table directly
  would still defeat it. None observed; none ruled out.
- **The collision findings are ONE ROOM, ONE COLUMN** (Outset R44, Link's
  column). **49 of the 50 `sea` rooms are unmeasured.**
- **`bound=10 missing=0` measured AVAILABILITY, not COVERAGE.** Four of those
  symbols were bound and never called; I read it as completeness for four
  boots. **A resolve-count says nothing about whether the port uses what it
  resolved.** This is R6 and it is the most transferable mistake here.

## 7.3 WHAT I PUBLISHED THAT LATER PROVED FALSE

| # | Claim | Reality |
|---|---|---|
| 1 | *"the parsed-object vtable head makes a second parse impossible"* | **False.** `getObjRes` never returns the parsed object — it returns the same raw image forever. Caused the DN-3 double-parse crash. |
| 2 | *"ROOT FOUND — the truncated conversion sequence"* | **Retracted.** A real gap, not the root; the next boot died identically. |
| 3 | *"the real root = unported `CreateInit`"* | **Retracted — and it was written INSIDE the retraction of #2.** Also real, also not the root. |
| 4 | *"the push gate is blind to 31 banner-declared files"* + my ruling approving a banner reader | **VOID.** The gate has used a filename-union-declared-lineage basis since 2026-08-07. **DO NOT BUILD THE BANNER READER.** |
| 5 | the five-actor "measured minimum set" | **Retired.** Room 44 alone is 87 names / 475 placements / **61 procs**. |
| 6 | *"EXT_VEG is the biggest win — cause of the 99 born-dead"* | **Wrong reading.** `daGrass_Create` returns `cPhs_ERROR_e` on every path; born-dead is *correct donor behaviour*. |
| 7 | *"3 → 16 BgWs"* as a falsifiable target | **Mis-derived** from placement deltas. Placements are not collision. |

**The pattern under #2 and #3:** I twice promoted *a* real defect in an
indicted subsystem to *the* defect without a confirming run.
**A bisect that indicts a PATH does not identify a LINE.**

## 7.4 COMPILES BUT HAS NEVER RUN

The distinction shipped a fork crash: `wwRoom_preResolveObjectArcs` had never
executed because its TU did not compile until a separate repair — **fixing the
compile error is what let an unbounded walk reach the user.**

- **The four-mode `lwood` draw bisect NO LONGER EXISTS** — removed with the
  plugin revert. `SPEC-patcher-requirements.md` describes it; **rebuild it from
  there if the defect is resumed. It is not staged.**
- **The `lwood` draw crash is UNROOTED.** Deterministic on every submitting
  boot, **no CPU exception at all** — the backend is Dawn/WebGPU, and a
  device-lost kills the process at submission after every draw call has
  returned. **"No fault marker" is EVIDENCE, not missing evidence.**

## 7.5 TOOLS ADDED SINCE 2026-08-14

*A script worth running twice belongs in `tools/`, not a scratchpad — this
estate lost three artifacts to session-temp in one week.*

- `tools/foundry/integrator_watch.py` — the lane monitor. **Launch it with the
  harness's own background flag. NEVER a shell ampersand, and NEVER with
  stdout discarded** — both silently eat deliveries: an ampersand job dies when
  the call returns, and a discarded stdout still marks rows seen while emitting
  to nowhere. **Verify liveness by the pulse file's mtime, never by the launch
  command's exit code** — a dead job and a live one both return 0. Its exit
  line now names the trigger that woke it.
- `tools/foundry/file_row.py` — **its `FILED OK` proves an ANNOUNCEMENT
  published, never that a ruling landed.** It now says so itself.
- `tools/foundry/arc_index.py` (History/Bridge) — reads a resource index off an
  arc's own file table. **THE ARC IS THE AUTHORITY**; many actors have no
  generated header. Offers near-miss candidates rather than a bare "not found",
  because a bare miss invites the guess it exists to prevent.
- `tools/ww_crew_restoration_skeleton/ww_layer_manifest.py` — the push gate.
  **TWO arms: filename AND declared lineage.** Any doc calling it
  filename-only understates it — that misreading produced false finding #4.

## 7.6 YOUR FIRST ACTIONS — Phase 0 is yours

1. **Get the real divergence number.** 2,861 is from a July fetch.
2. **Capture the BEFORE:** field FPS (the 220–250 band), the 8/8 assert
   battery, golden traces, exe hash, current `--check` output. Without these
   the Phase-0 rule is unusable.
3. **Branch before merging.** The eight leg-carriers (`d_door`, `d_stage`,
   `d_particle`, `d_demo`, …) are where conflicts are already predicted —
   **the ownership map IS the merge-conflict map**, which is what it was built
   for.
4. **Do not re-derive the seam census.** 122 rows, 0 errors, 31/31. Phase 2
   re-measures *against* it; it is not scrap.

## 7.7 RULES EARNED THE EXPENSIVE WAY

- **A named donor constant is DATA. Read it; never infer it.** Four wrong out
  of four attempts — dzb index 5-vs-7, the vtable invariant, `TEV_TYPE_BG0`
  0-vs-1, `UseSingleDL` 0x20000-vs-0x40000. **The `void*` plugin method has no
  compiler to catch these; Phase 5 (header-ful) dissolves the whole class.**
- **Port the donor's TRANSLATION UNIT, not its METHOD TABLE.** `CreateInit` is
  not in `daLwood_METHODS` — that is how a "verbatim" port dropped six calls.
- **A conversion that is a SEQUENCE is not satisfied by its first step**
  (`load` → `newSharedDisplayList` → `simpleCalcMaterial` → `makeSharedDL`).
- **Announce ≠ record ≠ applied ≠ effective.** Four steps, each failed
  separately here. Write the artifact FIRST; where you may not, name the
  artifact, the edit and the owner, and keep it open until you **read it back**.
- **A correction must reach the POINT OF THE MISREAD, not merely the
  document.** A banner protects a reader who starts at the top; nobody does.
- **Never rule an instrument deficient without demonstrating you RAN it and
  got a known-positive.** A check that can only produce silence must be proven
  able to produce noise.
- **Reproducing someone's NUMBER is not verifying their CLAIM.** An exact match
  with an instrument you are calling blind is *disconfirming* evidence.
- **Reporting the limit of your own search as a property of the world.**
  Three lanes did it in one day. I did it writing *this* section — I searched
  `docs/` only, declared "there is no Integrator handoff", and created a
  duplicate of the file you are reading. The user corrected me. **Search
  wider than the obvious directory before concluding absence.**

## 7.8 LEFT OPEN

- **`patch_feed.py` still lists `include/d/actor/d_a_e_ww.h` under
  `stale_map_rows`.** A `not-ww` row is a **negative receipt** — pruning it
  re-opens a cross-instrument conflict that was adjudicated and closed. Owner
  was Foundry.
- **The public surfaces were never given a containment read**: the
  `A-Link-Between-Dusklight` repo and three `zeldaret/tww` PRs. Offered to the
  user, never taken up. No evidence of any problem; also no check performed.
