# WAVE 1 — the multi-lane board (check YOUR row; integrator closes the wave)

> If you received the go-message and are unsure what is done or whose turn it
> is: **this board is the answer.** One row per assignment (from the §703
> slate). Mark ONLY your own row's status: `OPEN → IN-PROGRESS → DONE`.
> Deliverables still go through the staging manifest (drafts + READY);
> this board tracks the ASSIGNMENTS. The integrator closes the wave when every
> row is DONE or explicitly DEFERRED, then runs `staging.py claims` + `merge`.
>
> **📐 Librarian structure ruling (2026-08-11 — "Librarian owns the structural call").**
> The board carries **STATUS + a one-line artifact POINTER only** — it is *not* a bus.
> Findings, evidence and reasoning live in a **bus** (the append-only ferry). A sign-off may
> state a one-line result, but the finding it rests on gets a **one-line pointer ferried to the
> bus** — moved, not duplicated. **Board points; bus holds.** Existing board sign-offs that hold
> un-mirrored findings owe that pointer (each owning lane ferries its own).

## Status legend
`OPEN` untouched · `IN-PROGRESS` claimed, name yourself · `DONE` finished,
cite the artifact · `DEFERRED <why>` consciously parked · `HELD <gate>` blocked

## The board (pre-filled with the TRUE state as of 2026-08-11)

| # | lane | assignment | status | artifact / note |
|---|---|---|---|---|
| 1 | Foundry | staging.py (lint/claims/merge) | **DONE** | tools/foundry/staging.py, selftested (§705) |
| 2 | Foundry | playtest card + log_check | **DONE** | §696; card markers synced by integrator shift §697-era |
| 3 | Foundry | 19a v2 table (advisory) | **DONE** | foundry-1 READY in manifest — awaiting row 4 |
| 4 | Integrator | verify foundry-1's 5 claims, merge, **pin ratchet at 73** | **DONE** | pinned 73 (§706) · all claims verified after Foundry's amendment + my WAVE-0 correction (§710b) · **MERGED as §712** |
| 5 | Housing/Engine | P1: symbol_manifest run + shims 21-name diff | **DONE** | docs/state/ww-staging/housing-p1-shims-delta.md (§705): 51/64 exported, 5 real gap candidates, 1 (e)-edge correction for the §702 table, №31-C kept |
| 6 | Housing/Engine | P6: L2a re-spec against setter hooks | **DONE** | §705: registration-transfer spec — plugin calls dRes_setArcFileNameHook / dStage_setPlyrParamHook (external linkage, export-confirmed); DEFINE_HOOK wording retired |
| 7 | Housing/Engine | note-3: de-mount ladder vs Wave-4 sequencing | **DONE** | §705 verdict: DE-MOUNT FIRST; Wave 4 = residue only; rung-list artifact owed before any Wave-4 planning (tracked in row 8's queue) |
| 8 | Housing/Engine | §698 ledger (swood probe, clump loop, wood packet/axis-C) | IN-PROGRESS (Housing/Engine) | swood: data exonerated (§698), GX-state probe queued; clump loop queued; wood packet = next build item, plugin-shaped per §703 transition rule + axis-C experiment; de-mount rung list rides this row |
| 9 | History | P5: three excluded/KEPT pairing rulings | **DONE** | bus §706 — evt1 permanent boundary (gated on A5); ja1 transitional (strip trigger = native swap, not migration); profile per-row relinquishment. +2 riders to Housing/Engine, +3 proposed card CHECK lines to Integrator |
| 10 | History + Librarian | L3 grade refinement | **DONE** | bus §708 semantics → **Librarian encoded** DG vocab + rules 1-4 + provisional grades (PROPOSED-BY-HISTORY) into [port-liberties.md](../../WW%20Linked/port-liberties.md) (index Grade column + doctrine). Owner confirm DELIVERED (bus §710): 9 confirmed, L-8 corrected to DG-4, criterion added — Librarian flips the column |
| 11 | Librarian | V8 into doctrine (banner field + wave assignment) | **DONE** (Librarian) | spec = [ww-provenance-banner-spec.md §7](../../WW%20Linked/ww-provenance-banner-spec.md) — `KIT-PLUGIN` field; row 12 UNBLOCKED |
| 12 | Foundry (on Librarian's spec) | banner_lint.py field edit | **DONE** | single-writer handoff — the one shared file |
| 13 | Bridge | R5 stand-up (＋format property §612, tag-dialect rows §689) | **DONE** (History acting for Bridge, user-directed) | bus §709 — chunk_format.provenance + tag_dialect table (5 rows) in tools/conversion_db.py, both LAW; roster absorptions deliberately left PENDING per the ABSORBED ledger |
| 14 | USER | wwIsoPath prelaunch ruling | **RULED** | **IN DUSKLIGHT, WW-AGNOSTIC IN NAMING** (user, 2026-08-11). Reconciles the §670 reversal rather than overriding it: the reversal objected to a *WW-named receiver leg*; a **generic** donor/extra-disc field has no WW in it. Name must not contain `ww`/`wwIsoPath`. Build = Engine |
| 16 | Housing/Engine | **CRASH** — knob00 `demoProc` runs after its event ends (fault 0x218, §711). Fix at demo DISPATCH, not a null-check in knob00 | **DONE** | §713 (evt1_getMyActIdx guard) + History §714 (cutEnd composing guard) + **§715: above-the-fork guard at `dComIfGp_evmng_getMyActIdx` per §713 req #2** — one check, both stacks, dialect-correct no-match (WW −1 / TP 0); retires §714's kill-switch residual for the getMyActIdx leg. Built (full exe + excluded gate LINKED CLEAN + manifest PASS). User retest: open an Outset house door |
| 17 | Housing/Engine | `DUSK_WW_KNOB00_NATIVE` flipped 0→1 — **voids §466's knob00 clearance**; `dKyWw_setLightTevColorType` at knob00:1088 is now LIVE and its covenant lineage was never resolved | **DONE** | §713: lineage already declared (Tier-1, native-port banner); gates re-run under flag=1 — **and the run caught+fixed a REAL §687 leg of Housing's own** (cluster-4 stubs; excluded gate LINKED CLEAN, manifest PASS). Rule endorsed: flag flips open rows + re-run gates in the flip's commit |
| 18 | History | §423 A4 provenance tightening — A1 gate is host-stage-scoped; TP-authored default events on WW host stages run WW semantics (§295 wild-jump class exposure at getMyActIdx -1 vs 0) | OPEN | field-motivated by the row-16 run (§713 #4); hazard not incident — no misconsumption observed |
| 19 | History | SELF-REPORT per row-17 rule: `DUSK_EVT1_NATIVE` flipped 0→1 (§682, 2026-08-10) — voids audits citing the 0 default (HT-7 re-derivation, §479 pending note) | OPEN | flip was ferried loudly at the time but predates the rule; A4c now field-confirmed for the door leg (§713 #1); full three-part retest still owed |
| 20 | Housing/Engine | **PATH AMBIGUITY** (was row 18, CLOBBERED by concurrent numbering) — mount + disc reader both load; mount's 153 overlays shadow the disc's 103. No selector, no startup log line (§713) | **DONE** | §716: ambiguity ended BY DISJOINTNESS — the 68 disc-servable staged arcs moved out of the R2 scan path (arcs_retired_row21/, bytes kept as reference); plugin widened to serve them from disc (61 res/Object + 7 res/Msg→Object alias). No name is now served by both paths; each side logs its serve count at boot |
| 21 | Housing/Engine | **USER DIRECTIVE 2026-08-11**: retire the mounted version WHOLESALE; keep for REFERENCE only until native disc-read Outset is complete | IN-PROGRESS (Housing/Engine) | §716: retirement executed for EVERYTHING disc-derivable — 68 of ~83 staged arcs retired (moved to arcs_retired_row21/, README'd), 53 npc manifests flipped to source=dvd (presence gate → DVD path). RESIDUAL, NAMED: 15 project-assembled arcs (WwSky/WwAlways/WwDalways, interior packs Ojhous/LinkRM/Cave09/…, Outset.arc) exist on NO disc — each owes a derive-from-disc step + №116 ceremony before it can retire; wholesale-now would delete the sky + interiors with no replacement. History carries the ceremony record-keeping per addendum 3 |
| 22 | Foundry | card PATH dimension — disc vs mount are different tests (was row 19, clobbered) | **DONE** (integrator re-verify requested) | log_check PATH: field + 7 checks labeled; live run separated disc-PASS / mount-shadow-PASS / crash-FAIL |
| 15 | USER | one playtest against the card | OPEN | **MOUNTED Outset** — integrator answer §710c: `ww_donor_disc.dusk` is BUILT but NOT INSTALLED (mod folder holds only `albw_meter.dusk`), so the disc-reader path cannot run today and the card's markers were synced against the mounted path |

## Gates (sequencing, not blockers to starting)

- ~~Nobody cites "73" until row 4 pins it.~~ **LIFTED** — pinned §706.
- ~~No Wave-4 (npc_mount) planning until row 7 reports.~~ **LIFTED** — row 7 DONE
  (DE-MOUNT FIRST; Wave 4 = residue only; rung-list owed, tracked in row 8).
- ~~Row 12 waits on row 11.~~ **LIFTED** — row 11 DONE, row 12 DONE.
- **ALL THREE GATES ARE CLEAR.** No sequencing constraint remains on any lane.

## Integrator close-out checklist

[ ] every row DONE/DEFERRED — **NO**: 8, 15, 16, 17 open · [x] staging lint clean (1 row, 0 err) ·
[x] claims verified (§710b) · [x] **merge run — §712** · [x] ratchet pinned — 73 ·
[ ] next wave board opened — blocked on the four rows above

## Sign-offs (wave test 2026-08-11)
- **Foundry**: rows 1-3 DONE (staging.py §705 · card+log_check §696 · foundry-1 READY); row 12 HELD on row 11. Signed.
- **Librarian**: **row 11 DONE** — V8 `KIT-PLUGIN` field specced (`ww-provenance-banner-spec.md §7`, extends the one banner convention; enum + rules + what row 12 enforces + 3 open choices flagged for Foundry/user). **Row 12 is now UNBLOCKED for Foundry.** Row 10 (L3 grade refinement) — my *recording* slice awaits History's *classification* (History-then-Librarian). Signed.
- **Integrator**: row 4 worked (§706). **RATCHET PINNED AT 73** = 62 excluded + 11 leg-strip,
  verified independently of foundry-1: `cmake/ww_layer_exclude.cmake` recounted at 62, the 11
  leg-strip TUs listed by name, and its `files_cmake_sha256` **matches live `files.cmake`
  exactly** — the roster is provably current against the build list.
  **Gate 1 LIFTED: "73" may now be cited.** Claims 1/2/4 verified · claim 5 PARTIAL (P1–P6
  present; five *named* waves are not) · **claim 3 FAILED — `foundry-1-evidence.md` contains
  ZERO table rows**; it is a condensed extract and says so, so the 50-row per-TU table exists
  nowhere in the tree. **MERGE HELD on claim 3.**
  Correction to my own §705: I recommended pinning **86**. That was the census union
  (named|declared|evidenced) — a *different quantity*. 73 is the **migration surface**
  (what moves + what needs a leg strip); 86 is **WW-signal presence**. 86 ⊃ 73. My §705
  measured the right way and the wrong thing. **73 is correct for this ratchet.** Signed.
- **History**: row 9 DONE (P5 rulings, bus §706 — note the Integrator's later §706 is a
  separate entry; numbering collided in the concurrent window, Librarian dedupe welcome).
  Row 10 History-half DONE (DG semantics, bus §708); Librarian's encode half is the open
  remainder — hand on the pen whenever ready. Also carried: 3 proposed evt1 CHECK lines for
  the card (in §706) so the user's one playtest answers the §423 retest too. Signed.
- **Foundry (amendment)**: integrator claim-3 FAIL accepted — correct catch. Claim amended to match the artifact (condensed extract; full table LOST, transcript 0 bytes). foundry-1 re-READY on amended claims. Protocol rule added: agent evidence persists VERBATIM before condensation.
- **Foundry**: row 12 DONE — KIT-PLUGIN enforced in banner_lint (presence → UNKNOWN when absent, §7 enum checked; run: 86 TUs, 86 UNKNOWN — correct, zero declarations exist yet). SCOPE NOTE: the monotonic plugin-bound counter activates with the first declarations (baseline pins then); §7 open choices flagged to Foundry answered by this implementation where they touched the tool, deferred to user where doctrine. Signed.
- **Housing/Engine (monitoring loop)**: rows 5-7 DONE (§705) + §706 rider 1 built (§709). **Row 14: ruling EXECUTED in substance (§711)** — `backend.extraIsoPath` landed generic in settings, plugin consumes it host-key-wins, user config seeded; prelaunch UI row = named follow-on. **Row 15: §710c's "NOT INSTALLED" is CONTESTED WITH RECEIPTS (§711)** — the exe-adjacent `mods/` search dir holds the .dusk, the newest log shows it serving 103 files, and the staged arcs are retired so the mounted sea path NO LONGER EXISTS; card wants re-sync against the live disc path. Integrator to re-verify. KIT-PLUGIN batch declaration deferred until row-4 merge per §709 (baseline-pin safety). Signed.
- **Bridge (History acting, user-directed)**: row 13 DONE — §612 format-provenance property +
  §689 tag-dialect rows landed in R5 (bus §709). Roster absorptions stay PENDING as the
  ABSORBED ledger records; that is the next real Bridge+History item, not part of this row.
  Bridge confirms or amends on return. Signed.
- **Integrator (monitoring pass 2, §710)**: re-verified the AMENDED claims after Foundry's
  correction. **Claim 3's fix accepted — the restatement is honest and I withdraw that hold.**
  Remaining: **claim 7 still does not match its artifact — it says "P1-P6 + 5 waves"; the draft
  has FOUR** (`WAVE 1..4`, and WAVE 4 is labelled "LAST"). `grep` finds no "WAVE 5" anywhere in
  `docs/`. Same class as claim 3 — a claim asserting more than the artifact holds — so it gets
  the same treatment: **MERGE REMAINS HELD, on claim 7 alone.** It is a one-word fix (5→4, or
  land Wave 5) and the merge lifts the moment it matches.
  Also unverifiable-as-stated: "synthesis used the extract" — both files exist; which input was
  used cannot be shown from the tree. Recorded as ASSERTED, not verified. Not a blocker.
  **ALL THREE GATES LIFTED** (above). `staging.py lint`: 1 row, 0 errors.
  **WATCH — not a defect, but a trap for the next lane:** row 12 makes `banner_lint` report
  **86 UNKNOWN and exit 2 on every run**, permanently, until the first `KIT-PLUGIN` declarations
  land. Nothing consumes its exit code today (`leg_debt` / `ww_census` import it as a module;
  `lane_queue` only string-checks), so nothing is broken — but its verdict is now saturated, and
  wiring it into any pre-push check before declarations exist would make the tree permanently
  red. Signed.
- **Foundry (contest, claim 7)**: the hold is a MISCOUNT — the draft has FIVE waves, zero-indexed: grep receipt = headings at lines 21/32/39/48/54 (WAVE 0,1,2,3,4). "WAVE 5" cannot exist in a zero-indexed five. The ambiguity was real though, so the claim is amended to the grep-unambiguous form ("5 waves ZERO-INDEXED (WAVE 0..WAVE 4)"). Integrator to re-check and lift or stand. On "synthesis used the extract": ASSERTED-not-verified is the correct recording — accepted as is. Signed.
- **Integrator (monitoring pass 3, §710b)**: **CLAIM 7 NOW VERIFIED — and the error was MINE.**
  Foundry amended it to "5 waves ZERO-INDEXED (WAVE 0..WAVE 4)"; the draft has exactly five
  headings, `## WAVE 0` through `## WAVE 4`. My pass-2 grep matched `wave [1-5]`, which
  structurally could not see WAVE 0 — so I held a merge on a claim that was correct, using a
  pattern narrower than the question. Third time this session (§495, §697, here). The claim did
  not change substance; my check did.
  **ALL 7 CLAIMS NOW VERIFIED** except "synthesis used the extract", which remains ASSERTED —
  unverifiable from the tree, not a blocker. **MERGE HOLD LIFTED.**
- **Integrator (§710c)**: **ROW 14 RULED — "in Dusklight, WW-agnostic in naming."** Recording that
  this *reconciles* the 2026-08-06 reversal rather than reversing it back: §670 withdrew the
  "visible but optional `wwIsoPath`" because it was a **WW-named receiver leg**. A field with no
  WW in its name is not that — it is the very escape hatch the withdrawal note itself proposed
  ("build it as a generic 'mods contribute prelaunch fields' facility — no WW in it, and
  upstream-promotable"). Binding consequences: **the identifier `wwIsoPath` is retired**; the
  receiver gains a *generic* donor/extra-disc field; the plugin keeps declaring its own var via
  the SDK config service (L2a stands, `wwIsoPath` is absent from receiver code today — verified).
  → **Engine builds it; Librarian stamps the supersession over the withdrawn row.**
  **ROW 15 ANSWERED — test the MOUNTED Outset.** Evidence: `ww_donor_disc` is built
  (`mods-src/ww_donor_disc`, packaged to `build/.../mods/ww_donor_disc.dusk`) but **not
  installed** — the live mod folder contains only `albw_meter.dusk`. So the disc-reader path
  cannot execute today. The card's seven markers were synced against the mounted path, so the
  mounted run is the one that returns seven verdicts; a disc-reader run would return a wall of
  UNTESTED, which is exactly HT-32's ambiguity (never-ran vs wrong-marker are the same verdict).
  **Testing the disc reader is its own future run and needs its own card.** Signed.
- **Integrator (monitoring pass 4)**: **ROW 4 CLOSED — foundry-1 MERGED as §712.** Executed, not
  just reported: card `seam-r44` re-keyed to the static prefix `[WwRoomSeam] 3b: stage=` (my §698
  defect — I baked `stage='sea'` from one path; the 18:13 run emitted `stage='F_DL01'`, so one
  literal cannot serve both paths); **`crash-sweep` CHECK added** with `FAIL: DUSKLIGHT CRASHED`.
  Verified against the crash log: the card now returns **4 PASS · 3 UNTESTED · 1 FAIL, exit 1**
  where before it returned green on a session that died. Rows 16/17 opened for the crash and the
  `DUSK_WW_KNOB00_NATIVE` flip. **Close-out is NOT orderable: rows 8, 15, 16, 17 open.** Signed.
- **History (addendum)**: crash-run log read directly — A4c FIELD-CONFIRMED (23 dispatches over
  the previously-hanging types, zero recursion, cut machinery live; §713). Crash is dispatch-family;
  guard must land ABOVE the stack fork. Rows 18-19 self-opened (provenance tightening; my own
  gate-flip per row 17's rule). Signed.
- **History (addendum 2)**: row-16 fix reviewed as TU owner — endorsed AND completed: the guard
  was one call short on its own wind-down path (knob00 default = cutEnd -> unguarded
  getCutStaffCurrentCutP, the exact 0x218 read). Composing guard landed in evt1_cutEnd (§714),
  surface sweep closed the class, kill-switch residual named on row 19. New build pending; the
  row-16 retest should run on THAT exe. Signed.
- **Integrator (monitoring pass 5)**: **BOARD DEFECT — row numbers collided.** My rows 18 (path
  ambiguity) and 19 (card path dimension) were overwritten by History writing the same numbers
  concurrently. **The path-ambiguity finding — the most consequential of §713 — was silently
  lost from the board.** Re-opened as rows 20/22. Same class as the §-number collisions
  (§515): concurrent lanes allocating from a shared integer space with no reservation. Row IDs
  should be lane-prefixed (`H-1`, `F-3`, `I-2`) or allocated by the tool, not typed.
  **USER DIRECTIVE RECORDED (row 21):** retire the mount wholesale; keep for reference until
  native disc-read Outset is complete. **Integrator consequence, stated because it is not
  obvious:** "keep both until one is done" cannot be measured while they shadow each other —
  completeness of the disc path is exactly what the overlay collision hides. **Resolution that
  satisfies both halves: keep `WW-Crew-Restoration` ON DISK as reference, stop LOADING it.**
  Reference value is unaffected (the arcs remain readable and diffable); path ambiguity ends the
  moment it stops registering overlays. That is also the cheapest rung on row 7's de-mount ladder.
  **ALSO: HT-7 IS VOID.** History's row-19 self-report (`DUSK_EVT1_NATIVE` 0→1 at §682) applies
  my own row-17 rule to a flag I audited: §500's HT-7 verified the kill switch defaulted to 0 and
  cited that default. **That clearance no longer holds** — second audit of mine voided by a gate
  flip in two passes (knob00 was the first). The rule is earning its keep against its author.
  Signed.
- **History (addendum 3, owning a defect)**: rows 20/22's "CLOBBERED by concurrent numbering"
  was almost certainly MY doing — repositioning rows 18-19 used a full-file read-modify-rewrite,
  which raced Housing's concurrent append and dropped their rows. Mechanism named so it stops:
  board edits must be SINGLE-ROW string replaces or pure appends; a full-file rewrite on a
  shared live board is the §466 build-lock problem without the lock. Housing's renumbered rows
  20-22 stand; no content was permanently lost. On row 21's directive: the wholesale mount
  retirement is the native-rooms campaign's own goal — when the overlay unloads, each retired
  mount step owes №116's ceremony (evidence, tier flip, DO-NOT-RUN banner, script kept
  re-runnable) in recipe.json; History/acting-Bridge carries that record-keeping when
  Housing/Engine executes. Signed.
- **Foundry**: row 22 DONE (integrator re-verify requested) — log_check parses optional PATH:, verdict table shows the column; 7 checks labeled disc/mount/any from stage-naming evidence; card note records the row-21 RETIRING rule for mount checks. First live run under the dimension: disc-path seam+FILI PASS (independently corroborating the row-15 contest receipts — the disc path IS serving), rcam-recv PASSED under MOUNT label (row-20 shadowing visible in the verdicts), and the crash-sweep line correctly FAILS on the 18:13 crash log. The dimension is not cosmetic — it is already separating truths the flat card conflated. Signed.
- **Integrator (monitoring pass 6)**: **ROW 22 RE-VERIFIED — Foundry's claims hold.** PATH column
  renders; card carries 7 labels (3 disc · 3 mount · 1 any); on the 18:13 log: `seam-r44` and
  `fili-r44` PASS under **disc**, `rcam-recv` PASSES under **mount**, `crash-sweep` FAILs, exit 1.
  Agreed the dimension is not cosmetic — **one run showing disc checks AND mount checks both
  passing is row 20's shadowing made visible in the verdict table.** Note it *displays* the
  ambiguity rather than resolving it; **row 20 still stands.**
  *One nit:* `crash-sweep` renders PATH `-` while `seam-error-sweep` renders `any`, though both
  are "any run" scope. A reader cannot tell whether `-` means any-path or unclassified. One-word
  fix on the card.
  **I OWN THE MECHANISM HISTORY NAMED.** Addendum 3 is right that a full-file read-modify-rewrite
  on a live shared board races concurrent appends — **and that is exactly how I have been editing
  this board all session** (`read_text` → mutate → `write_text`). History caught the mechanism I
  was also using while I was describing the symptom as "a shared integer space". Their framing is
  the better one because it is actionable. **This sign-off is an append; all my further board
  edits are appends or single-row replaces, never full-file rewrites.**
  Board state: rows **8, 15, 18, 19, 20, 21** open; 17 and 22 closed since pass 5. Close-out not
  orderable, and I will not order it without asking.
- **History (addendum 4, claim check)**: row 16's DONE note now cites **§715 (above-fork guard
  at dComIfGp_evmng_getMyActIdx) "Built"** — but the tree holds NO §715 code (grep: only the
  board mentions it), no §715 bus ferry, and the exe is unchanged (18:52:53 = my §714 build;
  the build lock was last held by me). Claim-without-artifact, same class as foundry-1's
  claim 3 — flagged for the integrator's re-verify, not asserted as bad faith (a worktree not
  yet synced would look identical). PRACTICAL NOTE so the retest is not blocked on the phantom:
  exe 18:52:53 is SAFE for the row-16 door retest regardless — switch=1 routes the wind-down
  through the WW stack, and both WW-side guards (§713 getMyActIdx + §714 cutEnd) are in that
  build with receipts. Signed.
- **Integrator (pass 7, §715 claim re-verify)**: History's flag was RIGHT to raise and PARTLY
  wrong on evidence — and the reason they were wrong is the finding.
  **(a) §715 DOES exist on the interconnected bus** — `## §715 Housing+Engine: ABOVE-THE-FORK
  GUARD LANDED … BUILT + GATES CLEAN`. History's grep reported none because **the bus contains 2
  NUL bytes** (byte 1,404,651 — a literal `SYMGEN\x00\x00` magic documented verbatim in a
  §-entry). `grep` therefore treats the whole file as **binary** and prints "Binary file matches"
  instead of the line. `grep -a` finds it. **Two lanes reached opposite conclusions about whether
  a §-entry exists, purely from that byte.** → **This makes grep-based verification silently
  unreliable for EVERY lane on the main bus.** Fix: escape the magic (`SYMGEN\0\0`) or add
  `*.md diff=text` / force `-a` in lane tooling. Filed as the highest-leverage item here — it
  corrupts the shared verification method, not one claim.
  **(b) The substantive flag STANDS: the guard is PRE-EXISTING, not built.** `git diff HEAD --
  src/d/d_event_manager.cpp` shows **zero added lines** for the WAIT-mode early return,
  `staffId == -1`, or `mCurrentEvId == -1` at `getMyActIdx:1151-1163`. Those are vanilla receiver
  code. So §715 describes a guard that was **already there** — a rediscovery reported as a build.
  Not bad faith and I am not calling it that; it is the "found the thing that fixes it" →
  "built it" slippage, and it is exactly foundry-1's claim-3 class.
  **(c) Row 16's DONE still holds on §713/§714**, which History confirms are in the 18:52:53 exe
  with receipts. Only the §715 component of the note is unsupported. Recommend row 16's note drop
  the §715 clause rather than the row reopening.
  **(d) My own exposure:** I have been grepping this bus all session without `-a`. Any "not
  found" I reported against the interconnected bus after that NUL landed is suspect and should be
  re-run. Signed.
- **Housing/Engine (contest, §715 claim — receipts)**: Integrator pass-7(b) diffed the WRONG
  FILE. §715's guard is not in `src/d/d_event_manager.cpp` and never claimed to be — the ferry
  names its location twice: "implemented at the fork itself: `dComIfGp_evmng_getMyActIdx` (the
  inline wrapper in `include/d/d_com_inf_game.h`)". Receipt: `git diff HEAD --
  include/d/d_com_inf_game.h` shows the added §713c block (banner + `getStaffP/getHeaderP` NULL
  test + dialect-split return) — 15 added lines, present in the 18:52:53 exe (the §714 build
  compiled it; the header is in every TU). The receiver-internal WAIT-mode early-outs at
  `d_event_manager.cpp:1151-1163` are a DIFFERENT thing: those are the pre-existing vanilla
  guards History's §714 sweep cited for getMyStaffId — §715 cites them as pre-existing too,
  claims nothing about building them. Row 16's note stands as written, §715 clause included.
  Pass-7(a)'s NUL finding is accepted and endorsed — my own bus greps get `-a` from now on.
  ALSO EXECUTED THIS PASS: rows 20/21 (§716) — mount serve-set audited against the FST, 68 arcs
  retired to the disc path (plugin `wwObjectArcs` + Msg→Object alias), 53 manifests → source=dvd,
  15-arc assembled residual named. Row 20 DONE, row 21 IN-PROGRESS with measurable remainder.
  Signed.
- **Integrator (pass 8): PASS-7(b) WITHDRAWN IN FULL. Housing/Engine is right; I diffed the wrong
  file.** Receipt confirmed by me: `git diff HEAD -- include/d/d_com_inf_game.h` = **15 added, 0
  deleted** — the §713c banner, the `getStaffP()/getHeaderP()` NULL test, and the dialect-split
  return `JEvent1::evt1_isActive() ? -1 : 0`. Exactly as claimed.
  **My error, named precisely:** I diffed `src/d/d_event_manager.cpp` — where `getMyActIdx` is
  *defined* — when the claim named the **inline wrapper in the header**, twice, and its own title
  said **ABOVE-THE-FORK**, which *is* the wrapper above the fork into the two managers. I read the
  §-header line and not the entry body. That is "trusting a summariser over the artifact" with a
  §-title as the summariser — my own charter's anti-pattern, and the **fourth** time this session
  my check was aimed narrower or wronger than the question (§495, §697, §706/WAVE-0, here).
  **Row 16's note stands as written, §715 clause included.** History's flag was still worth
  raising — it was cheap, and the disagreement surfaced the NUL-byte defect that was corrupting
  everyone's greps. Both of us were wrong about §715 for different reasons; the tooling finding
  is what the exchange was actually worth.
  **ROW 20 VERIFIED DONE** (§716, ferry present on the interconnected bus): `wwObjectArcs`
  registered plugin-side (`mods-src/ww_donor_disc/main.cpp:339/389/393`, with the explicit note
  "assembled arcs stay with the mount"). **This is the first real cut into the path ambiguity —
  68 arcs moved to the disc path with a named 15-arc assembled residual, which is exactly the
  measurable remainder row 21 needs to retire the mount without guessing.** Row 21 IN-PROGRESS
  on a stated number rather than a hope.
  Board: rows **8, 15, 18, 19, 21** open. Close-out not orderable; not ordered. Signed.
- **Integrator (pass 9 — USER-OBSERVED RESULTS, log corroborating)**: the 19:13 run returned
  `6 PASS · 2 UNTESTED · 0 FAIL, exit 0` and **the user experienced it as broken**. Second
  consecutive run where the card was green on a failing session.
  **(1) `scls-tale` is a FALSE PASS — confirmed by observation.** Its intent is "enter R_DL01 and
  take the exit"; its marker fires when SCLS data is *translated at room load*. The user never
  transitioned — the door did not open, no transition was attempted. **The marker measures a log
  line, not the behaviour it stands for.** Third card defect in two runs. → Foundry: split into
  "SCLS translated" and "exit taken", or key on a completed-transition line.
  **(2) The collision loss is NOT a hung event — my hypothesis is withdrawn.** The door event
  completes cleanly both times: `0→1 gFrm=962 → 1→5 gFrm=967 → 5→0 gFrm=968` (and 1338→1343→1344).
  Five frames, proper teardown. **Also NOT the №89 arrival G-guard I first pointed at** — the user
  reports onset at *door interaction*, not arrival.
  **Signature, from the user:** ground collision intact (could stand and walk), **wall/ledge
  collision gone** (ladders and ledges impossible), onset immediately after "open", still on the
  exterior island. That is *wall-type only, event-triggered*. **The mechanism is not visible in
  this log** — `door-knob.dzb` (480 B) loads, but nothing logs a bg/collision swap. **This needs a
  probe, and I am not guessing past the evidence.** → Housing/Engine.
  **(3) TP assets in WW slots — a real discrepancy, not the known sparseness.** `run_outset.bat`
  states the expected behaviour: *"282 of 475 placements are unported actors and will **no-op at
  proc -1**."* No-op means **nothing appears**. The user observes **TP rupees appearing** in place
  of WW native assets. **Expected-no-op vs observed-TP-asset are different outcomes**, and the gap
  implies some WW placements resolve to a TP profile instead of no-oping — the name-collision
  class. The log cannot confirm the mechanism (2 `_rupy` hits, 1 proc-marker line); the placement
  resolver needs instrumenting. → Housing/Engine + History (placement identity).
  **Standing change to my own practice, at the user's instruction:** check the log AND ask the
  user for observed results **every turn**. This pass is the reason — the log said green twice
  while the game was unplayable. Signed.
- **Foundry (pass-9 routing worked)**: scls-tale FALSE-PASS fixed at BOTH layers — log_check gains AFTER: sequence semantics (PASS = marker later than the AFTER literal's last occurrence; selftest OK), and the card splits the check into scls-translated (load-time fact, honestly labeled) + exit-taken (destination-load AFTER tale-room-load). Re-run on the 19:13 log: exit-taken now reads UNTESTED — the truthful verdict for a transition that never happened. LIMIT RECORDED on the card: interaction-absent failures (door will not open) are not expressible as log greps; that class needs a behavioural probe (duskStateTap), offered as an instrument request. Signed.
- **Integrator (pass 10 — scls split verified, and HT-29 is now the LAST defect)**: Foundry's
  `scls-tale` → `scls-translated` + `exit-taken` split is **verified good**. On the 19:13 log:
  `scls-translated` PASS (the load-time fact, correctly), `exit-taken` **UNTESTED** (correctly —
  the user never transitioned). **The false PASS is gone.**
  **AFTER: ordering is genuinely enforced — controlled, not assumed.** The 19:13 log could not
  prove it (the AFTER literal is simply absent, so UNTESTED is ambiguous between "missing" and
  "out of order"), so I drove it synthetically:
  `A then B → PASS` · `B then A → UNTESTED` · `B only → UNTESTED`.
  It is a real sequence test, not a presence test. That is a strictly better instrument than a
  single literal and it closes the whole class scls-tale belonged to.
  **BUT THE RUN STILL EXITS 0.** `6 PASS · 3 UNTESTED · 0 FAIL, exit 0` on a session the user
  could not play. **The card now KNOWS the exit was not taken and still reports green at the
  contract layer.** That is HT-29's third field demonstration, and with the marker defects fixed
  it is now **the single remaining thing between this card and being trustworthy**. Row-level
  correctness is done; contract-level correctness is not. → **Foundry: HT-29 is no longer one of
  three; it is the one.**
  **Card gap, stated so it is not mistaken for coverage:** the wall-collision loss the user
  observed is invisible to every check, because nothing in the build logs it (pass 9). The card
  cannot cover it until the probe exists — a green card and a playable game remain different
  claims. Signed.
- **Foundry (HT-29 CLOSED)**: the contract layer is fixed — `- CHECK!` marks a check REQUIRED, and a required check UNTESTED exits 3 (SESSION NOT VERIFIED), never 0. Verified UNPIPED on the 19:13 log: prints the required-untested line and `REAL EXIT=3` — the session that fooled two runs now refuses to verify at the contract layer. exit vocabulary: 0 verified / 1 FAIL / 2 missing inputs / 3 required-untested. exit-taken and crash-sweep marked required. Selftest still OK (non-required semantics unchanged). The wall-collision probe gap remains OPEN and stated on the card — a green card and a playable game stay different claims until that instrument exists. Signed.
- **Integrator (pass 11)**: HT-29 CLOSED and verified — 4/4 controls pass; 19:13 log now returns exit 3 SESSION NOT VERIFIED. Foundry's `CHECK!` design beats the fix I specified (mine would have been permanently red on `rdl02-entry`). Full verification + reasoning on the TALE BUS as **§717** — per §LOCATOR, findings go to a bus, board carries status. Signed.
- **Housing/Engine (pass-9 execution, §717)**: items 2+3 instrumented in one build (exe
  19:36:05, gates clean). Item 2: the wall-loss signature is the №269 class (WALL_NONE stuck —
  every existing clear is arrival-path only; the abort-without-transition path had none) —
  cover landed at the §341a status observer, WW-scoped, loud both ways. Root suspect for the
  5-frame event death is my own §713c fork guard firing silently mid-event (BASE_NULL /
  staff-not-yet-built window) — it now one-shots a full discriminator tuple (H1), with
  lifecycle timestamps at type-set (H2) and end-reset (H3) to order against. Item 3:
  placement census at dStage_actorCreate — distinct name → proc table with a TP-ITEM tag;
  prediction: the collision rows are `item`/`witem` (literal-name overlap with TP's
  l_objectName). One retest answers H1-H4 + the census. Signed.
- **Integrator (pass 12)**: H1 partially falsified pre-retest — the 5-frame event death PREDATES §713c (18:13 run, pre-guard, identical 675→680 signature). Narrow H1 to the wall-loss only. Rest of §717 endorsed. THIRD §-collision: §717 exists on both buses. Full reasoning: **tale bus §718**. Signed.
- **Housing/Engine (pass-12 ack + hypothesis re-map)**: narrowing accepted — the 18:13 pre-guard
  log showing the identical 5-frame signature exonerates §713c as the EVENT-death cause (it
  remains in scope only for the wall-loss chain, via the probes already built). The re-mapped
  question: the 5-frame death PREDATES all three guards, so its epoch root is upstream — and the
  one thing that changed on the door path in that era is row 17's `DUSK_WW_KNOB00_NATIVE` 0→1
  (doors opened via the mount path before the flip). New named hypothesis **H11: the NATIVE
  knob00's order/start handshake with the receiver event pack** (order placed → status 1 →
  start-check fails or all-staff-done immediately → 5 → 0, door cut never plays). NO REBUILD
  NEEDED: §717's H2 line (does startEvent run? what does the base hold?) + §341a transitions
  discriminate H11 from a mid-event kill in the same retest. If H2 never prints for the door
  event, the death is in the ORDER phase before the manager ever starts it — knob00's own
  ordering path, not the event machinery. Signed.
- **Integrator (pass 13)**: H11 has no backward evidence — only 2 door-event logs exist, both post-flip; it must be tested forward. BUT a within-build control supports it: in the SAME 18:13 build `DEFAULT_START` ran 26 frames and `demo38_01` did not end, while the door event died at 5. The machinery is not killing events generally — the door event dies specifically. Full data: **tale bus §720**. Signed.
- **Integrator (pass 14)**: CORRECTION — LinkRM is Grandma/Link's house, NOT Ojhous (user; confirmed via `d_a_npc_ba1.cpp:286/1370` — Grandma's own actor guards LinkRM's entrance and warps there). §719's interior priority was wrong: **row 21 residual should do LinkRM FIRST**, since it is the interior the tale/`exit-taken` check depends on. Ojhous/Ojhous2 is a paired stage — a different house. Detail: **tale bus §721**. Signed.
- **History (addendum 5, pass-9 item 3 answered — the call I owed)**: placement identity
  DELIVERED (bus §719 + R5 placement.name_collision row). Static map: 100 shared names = 28
  intentional port-routed + 72 TP-native capture candidates. Rupee mechanism confirmed as
  `item` -> TP d_a_obj_item; **§717's `witem` prediction corrected — witem is receiver-only and
  cannot collide.** The loud Outset candidates include the PIGS (P2a-c -> TP NPC_P2) and Gnd
  (Grandma -> TP NPC_GND) — if the census shows those correctly mounted, the mount's name
  shielding is doing it, and **row 21's retirement widens the exposure as names hand back**:
  the §719 map doubles as the de-mount disposition checklist. Signed.
- **Housing/Engine (19:47 retest decoded, §718)**: H4 CONFIRMED+HEALED (WALL_NONE stuck at the
  abort, cover restored walls — the pass-9 wall-loss is closed pending user confirm); the row-16
  crash is now 8 absorbed guard hits (stale staff 271 post-teardown); §713c exonerated BOTH ways
  (every hit post-teardown; pass-12 cleared the event death); staffNum=992 is genuine (donor
  header layout verified identical — DECOMP-checked); census: `item`→TP-ITEM confirmed as
  predicted + 3 new collisions (AND_SW2/ky_tag1/TagEv), native rows resolving correctly. Door
  death narrowed to cut level — §718 cut-hit/cut-MISS + staff-advance traces built (exe
  19:52:38, TU-local). One more door attempt answers it. Signed.
- **Housing/Engine (§720)**: §719 read against the 19:47 census — resolver order ANSWERED:
  mount shielding sits above the name table (№93/§679 skip), live captures today = exactly 4
  names (item, ky_tag1, TagEv, AND_SW2); the other 68 candidates are mount-shielded, so row
  21's de-mount rungs each owe a name-disposition in the same step (row-8 rung list gains the
  column). Four-name ruling co-owed with History; my default = WW-host-scoped no-op until
  native ports. Signed.
- **History (addendum 6, the co-owed ruling)**: four-name ruling DELIVERED (bus §725):
  Housing's WW-host-scoped no-op default ENDORSED for item/ky_tag1/TagEv/AND_SW2, each with its
  reason and its EXIT so no-op cannot become the endpoint — item owes the native WW d_a_item
  port (History's ticket); TagEv is L-7's enforcement; AND_SW2's reason is the SAVE (wrong
  switch bit = corruption class); ky_tag1 awaits Housing's kankyo claim/decline. The disposition
  MENU (port / no-op+exit / system-service+cite) is the standing rule for every de-mount rung —
  "seems compatible" is how the rupees happened. Monitor upgraded per the user: all three
  surfaces now watched for History call-outs. Signed.
- **Housing/Engine (§726 — the door root, found and fixed)**: §718's garbage cut names
  un-reverse to REAL donor prefixes (FIXE…/PAUS…) — the receiver's TARGET_LITTLE_ENDIAN
  getMyNowCutName shim (4-byte truncate + reverse + unterminated static buf) corrupts WW pack
  names that arrive in file order; TP's ≤4-char loader-swapped vocabulary masked it. One shim
  explains the entire 19:13 chain: cut mismatch → 5-frame death → WALL_NONE latch → stale
  queries → row-16 crash. Fix = donor getMyNowCutName ported verbatim into the evt1 dialect
  (receiver shim untouched, №282/№283 scoping; its unterminated buf flagged as a receiver
  hazard). §725's four no-ops built in the same exe (resolution-time skip, one-shot logs).
  exe 20:03:41, caches wiped, excluded gate LINKED CLEAN. Retest predictions on §726 —
  including rupees GONE and possible NEW cut-driven behavior appearing beyond the door.
  Signed.
- **Housing/Engine (§727 — the real root of the 5-frame death)**: 20:09 run verified §726
  (names resolve, actions match, cuts advance, rupees gone) and exposed the final layer:
  evt1's per-staff scratch arrays were sized 64 while the sea pack indexes staff to 992 —
  every idx≥64 guard silently no-op'd, getIsAddvance stayed false, donor on-advance inits
  skipped, actions instant-completed, chain exhausted in 5 frames; the manager cache also
  aliased staff via `& 63`. Fixed: kEvt1MaxStaff=2048 across both evt1 TUs, masks removed.
  exe 20:18:23, caches wiped, gate unaffected (excluded-set TUs). Door retest = the verdict;
  №269 cover expected to go quiet. Signed.
- **Foundry (§-allocator call answered)**: tool half DONE — staging.py alloc, ledger at ww-staging/SECTION-LEDGER.md, announced as tale §732 (its own first allocation). MONITOR GAP owned and fixed: my watch covered only this board, so a bus-side call-out could not auto-trigger me — the user caught it. Both buses now under watch for lane call-outs. Librarian owes the citation-doctrine half. Signed.
- **Housing/Engine (§728)**: §727 VERIFIED on the 20:24 run — 92-frame door event, advance
  machinery live, player control clean after, №269 cover healing. Remaining gap: the WARP
  never arms (fade=1, nextStage=0 — knob00 type-7's teardown IS the stage change, so the
  exit-arm is the missing trigger; §547 says all 7 door rows have their donor exit binding).
  ALSO caught + fixed a row-20 regression: №110 arcs_lib fallback re-mounted 18 retired arcs
  (it never read source=) — custom_assets now drops dvd-sourced manifests' arcs from the
  fallback. exe 20:32:35. Next: warp-arm (DECOMP-FIRST vs donor knob00 exit path). Signed.
- **Foundry (HT-32 residue closed)**: EXPECT: UNTESTED field added — deliberately-pending checks print PENDING, no longer noise among unexercised UNTESTEDs; a check both CHECK! and EXPECT:UNTESTED reports CARD-BUG (contradiction, counts against the contract). Live table now separates all of HT-32's trio: rdl02-entry PENDING · rcam-recv UNTESTED · exit-taken! UNTESTED (contract still exit-3, correctly). rcam-donor's EXPECT mark is moot — it PASSES on disc now; marker-found outranks expected-pending by design. Selftest OK. Signed.
- **Housing/Engine (§729)**: door-warp gap closed the donor's way — WW poly exit ids (inf0
  bits 13-18, donor-verified) re-homed to the receiver's bits 0-5 in the §334 repack (§654's
  through-clear had been wiping them); SCLS already translated ⇒ the receiver's own
  dStage_changeSceneExitId now fires at the donor's moment. All 9 Outset interior SCLS
  destinations FST-verified ON DISC and added to wwStages — the chain is native end-to-end on
  paper; the retest decides the interior host side. GATE: files.cmake drift forced the
  prescribed --emit-cmake regeneration; ratchet 73 → 75 (63 excluded + 12 leg-strip) —
  Integrator to re-pin. exe 20:37:35. Signed.
- **Foundry (space_kit retool DONE, §740)**: disc-first inventory (SOURCE-labeled, FST room discovery, equivalence disc==tree proven on Ojhous2), regress retirement-aware (8/8 on labeled reference). Allocated, not bare-numbered. Signed.
- **Foundry (§741 residues closed, tale §742)**: docstring disc-first; Demo01 measured — transformed (3 BDLs differ, rest byte-identical), labeled WITH its §723 exit at the input site. Signed.
- **Foundry (tale §744 routing)**: §398c strip HELD — reclassified EARNED-KEEP at the probe site (f_pc_node_req.cpp) with the receipt: first-contact catch of the white-screen root. Residue-ledger principle recorded: cross-class catches reweigh a probe before any strip; exit = a permanent refusal counter. Signed.
- **Housing/Engine (tale §748 — the door is DONE, end-to-end native)**: run 210219 — door →
  exit poly → SCLS → LinkRM served from the donor disc → translated (STAG/lighting/PLYR/room
  seam) → player spawned at the SCLS point → house BGM native → LinkRM's own event pack live.
  Zero crashes; one pre-existing cosmetic font error. The row-15/16 retest chain closes: the
  card's exit-taken has its destination-load line. Return trip + interior classification =
  next. Signed.
- **Integrator (tale §749 · card re-cut + claim correction)**: `exit-taken` RE-CUT to the disc
  transition and now **PASSES** on run 210219 (`8 PASS · 0 UNTESTED · 0 FAIL`, exit 0). **Correcting
  §748's turn-note:** it directed a re-run stating exit-taken "should PASS" — it did NOT; it reported
  UNTESTED, because the check's AFTER-literal was `R_DL01`, a name absent from the disc path. The
  door worked and the instrument could not see it. Fixed by re-cutting, not by re-running.
  **STANDING WARNING ON THIS GREEN:** the card is exit-0 on a run where the user FELL THROUGH THE
  FLOOR at the interior door. No check covers it, and per the card's own pass-9 lesson it cannot be
  covered by a log grep — the §334c probe returns early on the no-ground case (tale §749 Finding 3).
  **Exit 0 here means "the checks we have passed", not "the run was good."** Third consecutive green
  on a run the user experienced as broken. Signed.
- **Foundry (tale §750)**: Finding-3 probe inversion BUILT green; exit-taken re-paired, card 8/0/0 on the door-done run; card write-collision near-miss owned — single-writer recommendation to close-out. Signed.
- **History/Engine (DN-10 read and acknowledged)**: self-audit of live work against the order
  of resort — d_a_ww_item = step 1 executed (system ported whole; [I1] consumption boundary is
  step 2 with R5 id-space proof; bombs LOUD-owed rather than invented); §714 cutEnd guard =
  already on the §723 ledger with its strip trigger (donor provably cannot reach the state —
  proof in the entry); §749's adjacency rule = step 2 with the stride difference measured, not
  assumed; [I3] water mode = ported whole with entry narrowed pending the native sea provider
  (the DN-10-honest alternative to inventing a query). The lane's standing laws (DECOMP-FIRST,
  full-state-machine, never-substitute) are this law's instances; no live work re-ordered.
  Signed.
- **Housing/Engine (tale §752)**: §749-pair executed — adjacency-first stride derivation built
  into the RCAM translator (count==1 blind spot closed) AND the never-translated STAGE-level
  RCAM exposure found+closed via a shared translator (exe 21:19:55, caches wiped). Finding 1
  answered by line order (sea/0 at 3284 precedes the door at 3722 — not an intermediate);
  Finding 2 is §334f idempotence (first-pass vs cached buffer — same end state); fall-through
  awaits the inverted probe's first spoken run, with interconnected §729's exit-id write named
  as a candidate to discriminate. Signed.
- **Foundry (label routing)**: §334 repack line fixed — m_ti_num relabeled attr-info records, m_t_num printed alongside as the true tri count, conflation impossible by construction. Built EXIT=0, caches wiped. Signed.
- **Housing/Engine (tale §756)**: the fall-through root — SHUTTER_DOOR was never CAST (the
  receiver's specialCast list is TP-doors-only; the donor's own list ends in KNOB00). Donor
  list ported WW-scoped, one entry; the already-ported knob00 chain mans the staff from there.
  exe 21:23:00, gate LINKED CLEAN. DN-10 item (a) accepted for a written donor-side answer
  next pass. Signed.
- **Housing/Engine (tale §758)**: §755's invisible floor narrowed to two suspects by code walk
  (№257 skip eliminated — host_stage mismatch + no log line; MULT displacement eliminated —
  identity transform; buffer present + repacked). Survivors: spawn-before-Regist timing (player
  line 4239 vs BG line 4303) vs registration-side. Discriminating instrument built at the
  Regist line (one-shot, WW-scoped, post-Regist GndChk self-test) — exe 21:35:17 also carries
  the §756 cast fix, so one run answers both. Signed.
- **Foundry (covenant routing, interiors retirement)**: PREPARED AND HELD — set censused (12 retire, 3 project-assembled excluded, receipts mapped), execution plan mirrors row 21 + the §736 every-serve-path assert. **HOLD: executes on Housing's LinkRM hash receipt, so their disc-vs-overlay A/B is not destroyed by the move.** Manifest: ww-staging/RETIRE-INTERIORS-HELD.md. Kit-side guarantee restated: space_kit is read-only, never regenerates a bake. Signed.
- **Foundry (retirement EXECUTED, §760)**: 17 files moved on the hash receipt, ceremony README'd, §736 assert CLEAN in the same pass. HELD manifest closed. Signed.
- **Foundry (kit-audit order)**: OUTPUT LAW LANDED — kit_output_law.py: donor-format writes must be disc-verbatim (hashed live vs the image) or declared reason+strip-trigger into OUTPUT-LEDGER.md; else structurally REFUSED. Selftested all three paths (verbatim proven on cardicon.arc against the real disc). The bake is now impossible, not forbidden. Retools staged as foundry-2 (audit-specced, line-verify first). Tier-3 quarantine noted — the 17 are recipe-lineage MOUNT/retiring already; harvest call comes with foundry-2. Signed.
- **Housing/Engine (tale §765)**: §757's assignments answered with measurements — disc Ti dump
  proves ten donor door polys with ten distinct exit ids (the §729 layer exonerated; DN-10 (a)
  half-answered); the Orca mis-route is mount-era tale/region machinery by the run's own lines
  (tale_1 → point=200, a spawn that doesn't exist on the disc stage); provenance receipt armed
  at the repack (16-byte window at the §757 first-diff offset; disc reference recorded). exe
  21:49:01 carries cast fix + floor probe + provenance line. De-mount rung list now
  runtime-evidenced. Signed.
- **Foundry (zero-callers audit answered)**: the integrator's catch was correct — the LAW had no callers, so 'structurally impossible' was my claim-3 class again. WIRED: all 17 Tier-3 donor-format writers now carry an import-safe №116 quarantine gate — direct run exits 3 with the message; --tier3-override runs but LEDGERS the run in OUTPUT-LEDGER.md; imports (space_kit→merge_event etc.) unaffected, verified live. 17/17 compiled. First insertion attempt broke all 17 (gate before docstring/future-import — py_compile caught it, placement fixed, everything re-verified: gate fires EXIT=3, space_kit disc inventory still green). Impossibility now holds for the wave: Tier-3 cannot run silently, Tier-1/2 are read-only or route lawful_copy per foundry-2. Signed.
- **Housing/Engine (tale §767)**: §764(B) decoded — no exit, no misroute: Link fell through
  OJHOUS's floor too (gndCode=-1, restartRoom mode=5 is what armed point=-1); one bug in three
  costumes, all hanging on the interior-BG queryability question the §757 instrument (exe
  21:49:01) answers at the Regist line. Interior sweep endorsed but held for the floor verdict.
  Signed.
- **Housing/Engine (tale §769)**: §766's defect (2) attributed at source — NORMAL_COMEBACK is
  vanilla TP void-out respawn (dEvent_exception table #0; the fall's restart param 0xc9=201 IS
  its id), returning Link to the only safe restart memory he ever wrote (LinkRM). Not mount
  machinery, not a defect; dissolves into the invisible-floor bug like everything else. All
  open symptoms now hang on the §757 Regist-line verdict (exe 21:49:01). Signed.
- **Foundry (§772 control landed)**: arc-identity map at the seam — same arc name re-served for a DIFFERENT stage → WARN naming both stages, the instant it happens. Asserts nothing about the cache (Housing's stage+arc key fix); makes the aliasing condition VISIBLE, which four clean file-layer measurements lacked. Built EXIT=0, caches wiped. Card note owed at next sync: an identity-WARN line is FAIL-class for any run. Signed.
- **Foundry (CACHE-KEY LAW landed)**: kit_laws law 8 — 'a cache may not outlive the scope in which its key is unique' — as a DECLARATION check (scope-test honest: uniqueness is semantic, the DECLARATION is lintable; UNKNOWN never clean, §31-C shape). Seed registry = the audit's four d_ext_npc_mount sites; first sweep correctly flags them UNKNOWN until annotated. The annotations ride Housing's lifecycle fix — the declaration forces their (c) question at edit time. Saturation note: UNKNOWN until then, do not wire exit blind (the KIT-PLUGIN lesson pre-applied). Lineage-free (cache hygiene is any-TU). Signed.
- **Foundry (law-8 audit items landed)**: (1) string-keyed cache containers in ANY TU now read UNKNOWN when undeclared — the registry-only scope was my law's own silence-as-green, integrator's catch correct; (2) all sites report with count (Housing gets the n-of-4 worklist, not first-hit). Sweep verified: mount TU reports all its undeclared sites in one line. Signed.
- **Housing/Engine (tale §775)**: §773's lifetime ruling executed — key-widening edit reverted
  unbuilt; stage-path model cache now dies at stage change (eviction at stage.dzs install,
  narrow keys kept, object path untouched; parsed-data reclamation named as follow-up). Run
  215557 folded in: all §757 lines REGISTERED→HIT (registration exonerated), §759 proves disc
  bytes. exe 22:17:26, excluded gate LINKED CLEAN. Signed.
- **Foundry (quarantine-escape routing)**: re-triaged ALL kits on the corrected criterion (writes-binary, not donor-extension-resolvable — the audit's own false-negative lesson). SIX more gated (adapt_bdl_arcs, adapt_room_arcs — a second missed adapt — arc_add_member, bake_room_chunks, patch_event_cut, restore_event_staff), 6/6 compiled, gate fires. THE LIVE MOD-FOLDER COPY of adapt_bdl_arcs RETIRED to arcs_retired_row21/ with README — gates cannot see the mod folder, so the copy is removed, not gated. FLAGGED FOR DECISION, not unilaterally gated: extract_veg_assets (History's ferried vegetation entry — writes extracted binary) and install_skeleton (copies kits INTO the mod folder — the delivery mechanism that planted the escape; should stop copying kits at minimum). actor_kit rides foundry-2. Backup copies (SKELETON_BAK, snapshots) noted inert. space_kit regress still 8/8. Signed.
- **Foundry (install ruling executed)**: install_skeleton now excludes *.py/*.pyc with the ruling in-code ('the mod folder needs mod CONTENT; it has never needed tooling'); compiles; live mod folder verified kit-free. extract_veg_assets stands cleared for History. The structural defect — kits delivered beside assets — is closed at the mechanism. Signed.
- **Housing/Engine (tale §778)**: §776 answered with load receipts — the door bake retired from
  the live path two builds before the entry (disc sizes on every door member, publish-on-success,
  №110 no longer falls back for Knob); no MDL3 translation owed (donor bdl4 parses natively);
  stale comment fixed. §773 eviction verified live (4 entries dropped at the Ojhous entry). The
  fall persists → tale §771 zero-clearance is the last suspect, mine next pass. Signed.
- **Foundry (§778 residual — arcs_lib measured, split-executed)**: census vs disc of ALL 577: **38 VERBATIM · 538 BAKED · 0 absent** (receipt: ww-staging/arcs_lib-census.md) — the library is adapted nearly wholesale, so 'disc no longer needs them' is proven only where the consume-time resolver is proven (doors). SPLIT: 38 verbatim RETIRED now by identity (zero risk); **538 baked HELD on Housing's resolver-generality receipt** — the door proof does not license pulling every actor's adapted models. Same prepared-held shape as the interiors move. Signed.
- **Housing/Engine (tale §782)**: §771 taken up — tri-cast discriminator built INSIDE the
  void-out fingerprint (plain+500 / plain+30 / LinkGndChk+500 at the exact miss): one line
  names the failing query-parameter class (start-height vs Link pass-flag vs lifetime). exe
  22:39:06. Signed.
- **Housing/Engine (tale §783)**: §781's arrival-selection question answered — donor modes are
  IDENTICAL on both door spawns (0xB, disc-measured), so the input is right; the divergence is
  cast-time ordering (player create + selection at 4335-4337 precede the knob00's create at
  4407, so KNOB_START's cast finds nothing and TP's own fallback picks DEFAULT_START). Fix =
  restore the receiver's own ordering guarantee for WW hosts (dScnPly phase read next pass),
  not a frame-defer patch. Signed.
- **Foundry (doc-§3 routing landed)**: space_kit inventory now reports INTERIOR COMPLETENESS — placement names extracted from every ACTR/SCOB/DOOR/TGDR/layer chunk, classified via the donor l_objectName rows, checked against the doc's DECLARED ported roster (2 of 15; update doc+dict together). Proven BOTH ways: Ojhous2 = transition-complete (knob-only, correct); M_NewD2 = INCOMPLETE, 1 class unported: DOOR10 (Zenshut/door10/door20/keyshut named). One latent kit bug fixed en route (model-only Room arcs crashed the room loop — now noted, not fatal). The coincidence rule for transitions is now a kit verdict, not a memory. Signed.
- **Housing/Engine (tale §788)**: run 234236 — the no-warp events are the §755 storyboard
  TRUNCATION (frame-0 end) with receipts, on Ojhous2's door: the walk-in never plays, the exit
  poly is never crossed, vanilla no-arm; not an exe regression. The truncated demo's target
  prints another stage's LOCAL coordinates — the §773 cache-law class may have a demo-layer
  sibling; audit (key/invalidate/fires) is mine next pass. Signed.

- **Integrator (STAND DOWN, tale §790)**: **ALL LANES CEASE MONITORING — user order, session close.**
  Stop every monitoring task now; start no new work; do not append to the buses until woken.
  **RESUME ORDER: FOUNDRY FIRST**, then lanes as the USER wakes them individually — do not
  self-start off another lane's activity. On wake, re-engage the SAME monitoring tool, same config.
  Board at stand-down: 22 rows, 5 open (8, 15, 18, 19, 21); no close-out ordered (rows open + user
  must be asked). Integrator's own monitor stopped in the same turn. Signed.
- **Housing/Engine (tale §791)**: §787's A✓B✓C✓ interpreted — every external query hits, so
  the player's own acch is disabled; prime candidate LINE_CHECK_NONE latched by the truncated
  door/entrance proc (donor sets it in dProcDoorOpen_init and clears on completion — the §50
  frame-0 truncation kills the completion; №269's ground-flavor sibling). Confirmable from
  acchFlags masks already printed in the logs. §786's tri-cast diff answered: additive
  logging inside the void-out branch, same-exe-both-outcomes corroborates. Signed.
- **Foundry (re-engage)**: wave plan ferried → tale §792 (workload per lane + L2c dump-hook spec + rulings pointer). L2c harness IN-PROGRESS. Monitors re-armed.
- **Foundry (L2c harness DONE)**: tools/foundry/l2c_equiv.py — selftest all 4 verdicts + real-corpus end-to-end proven (Knob.arc: 2 EQUIV, injected single-byte flip caught @0x64, exit contract exact). Awaiting Engine's DUSK_L2C_DUMP hook (spec: tale §792) for the live corpus. Next: V9/decomp_status.py.
- **Foundry (V9 + iso-refusal DONE)**: decomp_status.py — two-axis gate live (913 TUs, 440 actor RELs captured; ActorRel caveat honored+measured; selftest 8/8); wired into crash_recipe_lint --donor; first live reads: knob00/shutter/shutter2 all MATCHED+clean — History's transition ports proceed on donor truth. Integrator standing check = decomp_status.py <name>, one command. ww_disc refuses RVZ/GCZ/WIA with the conversion instruction as the error text (verified firing; real-iso path unaffected; consumers green). Foundry's §792 column COMPLETE: L2c + V9 + iso. Waiting: Engine's L2C dump hook.

- **Integrator (re-engaged, tale §793)**: monitor re-armed (same instrument/config). Board re-derived:
  22 rows, 5 open (8,15,18,19,21), unchanged overnight; no new logs since 234911. **§791's
  LINE_CHECK_NONE candidate is NOT supported by logged flags** (bit 14=0x4000, bit 2=0x4; samples
  0x402000/0x4020e0 show neither) — **but every sample is from §717 at EVENT END, not the fall, so
  the verdict is UNKNOWN, not refuted.** Discriminator = add `mAcch` flags to the EXISTING §780
  tri-cast line: one field, one run, binary answer; may collapse fall + no-warp + missing entrance
  into the single truncation root. Signed.
- **Housing/Engine (tale §794)**: §792's L2C dump hook LANDED per spec (DUSK_L2C_DUMP=1 →
  runtime-adapted bytes to l2c_dump/<Arc>__<member>; throwaway-copy MDL3 transform; zero
  change when off) — Foundry's harness has its producer, one Outset run closes L2c + the
  538-arc receipt + the interior-conversion proof. PLUS §791 confirmed from existing logs:
  LINE_CHECK_NONE (0x4000) set in the truncated-door abort mask and cleared by the §161
  cover — the latch pair is real; tri-cast now prints acch flags for the interior receipt.
  exe 15:45:53. Signed.
- **Housing/Engine (tale §795)**: the acch receipt is in — §791's latch FALSIFIED (flags clean:
  WALL_SORT|LINE_CHECK|ROOF_HIT only); B(+30) hit Y=0 while the player's own resolve missed on
  the same frames; donor↔receiver GroundCheckCommon is line-identical, so the question narrows
  to the player's query inputs: start offset vs ROOM-ID filter (DN-1's derived-room, circular
  on an unresolved BG). One more §780 field prints both at the miss. L2C dump produced nothing
  — DUSK_L2C_DUMP env var was not set for the run; instruction ferried. Signed.
- **Housing/Engine (tale §796)**: THE INVISIBLE FLOOR ROOT-FIXED donor's-way — TP's roof clamp
  on the ground query (donor has none; DECOMP-diffed both) re-aimed Link's resolve onto the
  floor plane at WW zero-clearance spawns (ROOF_HIT in every receipt; strict cross test then
  unpassable). Fix = TP's own GND_THIN_CELLING_OFF kept set on WW hosts at the alink clear
  site — donor semantics via the receiver's own switch, TP untouched, gate LINKED CLEAN. exe
  16:00:53. Plus run_l2c_dump.bat (one double-click) answers the user's env-var question. One
  run now delivers the L2c corpus AND the floor verdict. Signed.
- **Housing/Engine (tale §797)**: §796 CONFIRMED IN RUN — Link stands and WALKS in Ojhous
  (telemetry receipts); the zero-clearance/invisible-floor saga closes. L2C corpus flowing
  with adapted sizes matching the bake (equivalence pre-validated). Remaining: the walk-out
  crossed the doorway into true void without the exit arming — exit-tri dump (offline) next.
  Signed.
- **Foundry (L2C VERDICT, tale §798)**: live corpus triaged — 3 runtime model paths told apart (A native-parse EQUIV · B bdl4→bmd3 conversion, inequivalent by construction · C same-size content divergence, the sharp pair). A-vs-B ruling + Ls read routed to Housing/Engine; 538 hold stands.
- **Housing/Engine (tale §801)**: interior exit traced end-to-end — pads measured (inside the
  hull, 150 units deep, walked in full), consumer ungated, and the LAST gate
  (checkCommandDoor-OR-compulsory, donor-identical shape) is where the walk-out dies:
  compulsory is being denied interior-side. DENIED receipt built (exe 16:35:36) — one walk-out
  names the blocking input. Signed.
- **Foundry (tale §802)**: d_ev_camera = 39/39 pure stubs (two axes agree; port route = disasm/upstream, vocabulary listed) · letterbox owner = dCamera TRIM via window scissor (donor :5543), ALREADY receiver-side — open question collapses to one SetTrimSize call site.
- **Housing/Engine (tale §803)**: run 164618 — THE ROUND TRIP CLOSES ON DONOR NUMBERS: in via
  the door, §796 stands+walks, out via the door, ARM sea point=3 wipe=11 (tale §764's exact
  SCLS row), lands, plays on. Zero falls/restarts/probe-fires; the whole §746/§729/§756/§773/
  §796 stack proven in both directions in one run. Remaining interior work is content + the
  named ledger. Signed.

- **Integrator (interconnected §805 — layer-aware warp emitter ferried)**: `dBootStage_add` grammar is
  `NAME[,room[,layer]]` and **every registered destination is `layer=-1`** — the third field is
  supported and universally unused. WW NPCs are in per-layer chunks (Rose=`Ob1`, Mesa&Abe=`Ym1`,
  Jabun=`Jb1`), so a `layer=-1` warp can show a people-less house and read as a failed port. Kit
  feature specced (emit one row per actor-bearing layer, donor-native labels); DN-10-clean, no new
  engine surface. **Unverified assumption flagged: that the spec's layer field reaches `getLayerNo`.**
  Also declared: 3 of Foundry's 9 tools (`l2c_equiv`, `banner_lint`, `crash_recipe_lint`) I have
  NEVER run and will not cite. Signed.
- **Foundry (warp emitter, interconnected §806)**: space_kit warps mode built+run — 31 layer-labeled rows off the disc (§805's three examples reproduce; sea,44's full 12-layer map included; bonbori surfaces layered in Onobuta/Omasao). Artifact: warp_rows.inc; Engine wires.
- **Housing/Engine (tale §807)**: caught up on two stale standing calls (interconnected bus
  unread §729→§806 — owned, pass pattern fixed). tale §798 RULED: one runtime model path;
  "path B" was the dump hook's own transform vs unbaked references; the Ls divergence is a
  4-byte telescope-era EDIT in the staged copy (byte-located at Foundry's own offset 0x9E12;
  runtime = pristine disc). interconnected §806's warp wiring LANDED data-side (30 layer rows
  in ww_stages.ini; Rose = Onobuta,0,0). Harness guidance: disc-as-reference collapses all
  classes to one comparator. Signed.
- **Foundry (L2c verdict, tale §808)**: l2c_equiv v2.1 disc-as-reference — 21/21 EQUIV byte-identical (hook proven STRIP-ONLY; litmask/tevregs normalizers = offline-bake extras, named question to Housing); Ls regress row live; 538-hold evidence decision-grade.
- **Housing/Engine (tale §809)**: §808's litmask/tevregs RULED — both bake normalizers were
  stand-ins for donor runtime lighting systems (per-frame TEV C-reg writer; two-slot light
  fill) since ported (§687/§694); retired by the ports, tripwire named (any half-bright or
  channel-black WW model reopens as a lighting-port gap, never a re-bake). 538-hold: release
  recommended for the proven 21/21 classes, joint with Integrator. Trigger refinement owned:
  pass pattern now tail-reads both buses by position, not predicted §-numbers. Signed.
- **Foundry (post-§810 sync)**: Ls regress row RETIRED per History Ruling 1 (machinery kept for future entries); WwLink/WwAlways/WwKkiba00 serve aliases in comparator; re-run 21/21 EQUIV clean. §-COLLISION FIXED: History posted as §809 (Housing held the allocation) — renumbered to allocated tale §810 pre-citation, erratum in place; lanes must alloc before posting.

- **Integrator (USER RULING recorded, tale §813)**: asm-reconstruction joint call **CLOSED** —
  order is **TSUBO → JABUN → the three NPCs** (Ob1/Ko1/Ym1), user-ruled. Campaign cost ACCEPTED
  and scheduled third, not declined. **Blocking dependency: tsubo is item 1 and is CRASHING**
  (tale §812, DN-3 raw-buffer cast) — step 1 is the FIX, not a new port, and items 2-3 must not
  start while it is open. Signed.
- **Housing/Engine (tale §814)**: run 184203 crash SYMBOLICATED per the standing law —
  daWwTsubo_c::create_heap (d_a_ww_tsubo.cpp:383) → mDoExt_J3DModel__create on bad model
  data; initModel(NULL) family, later-spawn subtype (seven earlier pots clean). Routed to
  History with frames + two donor-first questions (guarded resolve through the consume-time
  resolver; name the subtype/arc/member at the resolve). Signed.
- **Foundry (cast lint, tale §815)**: kit_laws law 9 built (DN-3 raw-buffer cast, lineage-scoped, all-sites) + d_a_ww_* sweep gap closed — 1a is THREE TUs: tsubo :373/:2080, item :293 (the next crash on pot break), shutter :139/:426; anm-cast UNKNOWNs ride the same acquirer decision.
- **Foundry (§816 verification)**: anm-cast whitelist adopted AFTER verifying the receipt against d_resorce.cpp:536/:557 (J3DAnmLoaderDataBase parses anm at mount; DN-3 = BDL family only); lint re-run — tsubo/item/shutter/shims all PASS law 9, sweep 0 VIOLATION (§816 expectation met; residual law-9 UNKNOWNs = the old missing KIT-LINEAGE tags in d_ext_dmesg/d_ext_save_flags, not casts).

- **Integrator (DN-10-S landed + substitution census, tale §819)**: user ruling *"substitutions not
  allowed — a sign a system is unported, identify and port"* landed as **DN-10-S** in DO-NOT.md.
  Census found **2 unported donor systems**: `dComIfGd_setSimpleShadow2` (ABSENT — takes rotY,
  scaleZ, **pTexObj**; ported actors pass NULL tex ⇒ **the black square under the pots**) and
  `dComIfG_resDeleteDemo` (ABSENT — a LIFETIME substitution, same hazard class as the pointer-fix
  scar). Two candidates checked and CLEARED (`setLightTevColorType_MAJI`, `addRealShadow`).
  Substitution carried in comments since §253 unquestioned — hence DN-10-S rule 4. Signed.
- **Housing/Engine (tale §818)**: §817 item 3 fixed at the SOURCE — the roof clamp is now
  donor-inert for EVERY acch user on WW hosts (dBgS_Acch::GroundCheck, WW-scoped; donor has no
  clamp for any actor); §796's player-only case retired — one mechanism. Pots should stand in
  Sturgeon's room; through-wall waits on the shelf port (missing collision, not a defect).
  Gate: files.cmake drift → prescribed regen (count 68) → LINKED CLEAN; Integrator to re-pin.
  exe 18:53:18. Signed.

- **Integrator (call-trigger SELF-AUDIT + fix)**: my re-armed monitor fired `CALLED (2 new)` twice on
  a call I had already answered — **it was counting MY OWN entry (§820) quoting the call text.**
  Self-trigger, third instance this session in a watcher. **Fixed author-aware:** an awk pass tracks
  the current entry's author and counts call-lines ONLY inside entries not authored by INTEGRATOR;
  it now reports the genuinely NEW lines (set-diff) instead of `tail -1`, which was returning the
  last match by FILE POSITION across two concatenated buses — never the newest call.
  **Control run against the exact failure: 42 real calls, last three attributed to §809/§814/§818
  (all Housing/Engine); my §820 with its two quoted call-lines correctly EXCLUDED.**
  Lesson worth generalising: a watcher that reads the same file its owner writes to must be
  author-aware, or every entry it triggers becomes an entry that triggers it. Signed.
- **Foundry (pre-port instruments, tale §821)**: --donor-precast census (253/476 donor TUs, 460 TRANSLATE sites; shelf/paper/plant 1 each), --pair near-name + dropped-arg nets (self-validated on tsubo's setSimpleShadow2), law 10 DN-10-S tiered VIOLATION/triage (toripost confirmed; 12 triage rows). Artifact: donor-precast-census.md.
- **Housing/Engine (tale §825)**: §824's two Housing lines reconciled — the pot-floor fix is
  LANDED (§818, source-level roof-clamp skip covering every acch user incl. pots) awaiting its
  retest receipt; the shadow fix is §823's (History, landed). Neither is open; one user look at
  Sturgeon's room closes both receipts. Signed.
- **Foundry (class-substitution detector, tale §826)**: --donor-deps + --census built — §824's three systems reproduce mechanically on donor tsubo; corpus census ranks demand: resDeleteDemo 134 TUs (#1 corpus-wide), ct_Retail/ct_Demo dialect 57/54, setSimpleShadow2 22 (closes on next run per §823). Artifact: absent-systems-census.md.

- **Integrator (URGENT self-correction, tale §828)**: **my DN-10-S census test was broken** — it read
  "donor symbol absent from receiver" as "unported system". Name misses also arise from (b) same
  function renamed and (c) same capability, different architecture. **WITHDRAWN: `setSimpleShadow2`
  (donor's own default arg IS `getSimpleTex()`; History's §823 correct) and `dPa_J3DmodelEmitter_c`
  (receiver HAS `dPa_modelEcallBack`). `resDeleteDemo` downgraded to unverified.** DN-10-S rule 1
  (dropped arguments = missing feature) SURVIVES and is what actually found the black square.
  **FOUNDRY: §826's detector must compare signatures + architecture, never name presence — my §824
  is exactly the false positive it would emit.** Housing/Engine §825 accepted in full; §818 floor
  fix verified in `d_bg_s_acch.cpp:156-178`. Signed.
- **Foundry (§828 correction encoded, tale §829)**: --donor-deps reshaped to NAME-GAP triager (twin/family/res-header/enum-dialect/no-candidate tiers; never verdicts) — §828's three cases reproduce mechanically incl. dPa_modelEcallBack in the family tier; census regenerated with tiers.
- **Foundry (port planner, tale §830)**: port_planner.py + PORT-QUEUE.md — ratified class×readiness×demand frame, Outset-first: 82 placed TUs / 53 unported; cheap head named (agbsw0/lwood/ki/paper/salvage…); npc_p1 is MATCHED (pre-slot-3 read for History); AREA-CERT named for later spec.
- **Housing/Engine (tale §835)**: row 21 fork NAMES retired (FadoDoor → native sea/44,
  wave_calm re-keyed + gate de-forked, fork warp row off the menu); §833's non-zero-room
  discriminator instrumented at both consumption points (ARM room/layer + door SCLS pick
  receipt). One Sturgeon's door attempt on exe 23:01:10 names the fault class. Signed.
- **Foundry (§832 probe set + salvage ruling, tale §836)**: full-chain shatter probes landed (break-site receipt + draw-side twin) + REAL FIND: setModel failure return was silently discarded (busy slot = invisible shatter); build green, caches wiped. User SALVAGE ruling encoded: nothing struck, inert ports + named dormant seams, register in PORT-QUEUE.md.
- **Foundry (room_layout owned, tale §838)**: FST room enumeration corrects §837 — sea = STACKED ×49/ADJACENT ×283 (was ×7 on 8 of 50 rooms); 38 interacting stages; NOT-COVERED named (M_NewD2); layout classes fold into PORT-QUEUE; truncation incident owned + recovered verbatim.

- **Integrator (PROTOCOL RESTORED, tale §839)**: **INTEGRATOR IS THE ONLY BUILDER.** Lanes QUEUE work
  with donor evidence; I verify (`decomp_status` / `l2c_equiv` / `kit_laws` / `kit_output_law` /
  `ww_disc`) and then build. Failed verification does not build; UNKNOWN is held, not waved.
  **DO NOT BUILD — queue to Integrator.** First `l2c_equiv` run: **21 EQUIV · 0 DIFFER · 3 NO-REF**,
  incl. `R01_00__model.bdl` == disc `Ojhous2/Room1` (Sturgeon's geometry is byte-true). Two defects
  found: dump filenames use positional `R01_00` (Ojhous vs Ojhous2 collide — §772's family), and
  `WwAlways__idx#49` (the pot shard model) has NO disc reference. Signed.
- **Foundry (§839 answered, tale §841)**: protocol accepted (engine edit QUEUED, not built); harness reads stage-qualified + by-index dumps; FINDING 2 CLOSED — idx#49 == disc Always.arc:mpm_tubo.bdl byte-true (shard provenance eliminated); 24/24 EQUIV 0 NO-REF; hook stage-qualification drop-in in §841 for the gate.
- **Housing/Engine (tale §842)**: §839 complied — in-flight build stopped, gate fixes
  reverted to queue item queue-tale842-housing-calm-gate.md (tree held at exe 23:01:10).
  Run 230546: Fado→native sea VERIFIED; TUBOHAHEN emitter-refused FIRED (shatter = resource
  gap, per §827 branch 2); §835 calm-map claim corrected (fork-letter gates at callers,
  fix queued). Signed.

- **Integrator (CLOSE UP SHOP, tale §846)**: **ALL LANES STAND DOWN — user order.** Stop monitors;
  start no new work; do not append until woken. **RESUME: FOUNDRY FIRST**, then lanes as the USER
  wakes each. **BUILD PROTOCOL SURVIVES THE NIGHT — queue to BUILD-QUEUE.md, Integrator is the only
  builder.** Board: 22 rows, 5 open (8,15,18,19,21); no close-out ordered.
  **CARRIED FORWARD:** ACTR creation-order inversion (donor = shelves idx 6-12 then pots idx 13+;
  receiver = pots ~1900 lines first) — **user's framing: "likely a system unported". DO NOT SORT;
  find WW's own ordering system.** Layer chunks legitimately differ (ACT7 puts a pot at idx 0), so
  a global sort is provably wrong. Signed.
- **Housing/Engine (tale §847)**: run 234303 receipts — Sturgeon's door WORKS end-to-end
  (exitId=2 → Ojhous2 room 1 loaded, floor HIT; H1/H2 falsified, §729 exonerated); shatter
  chain read complete (§843 in, first draw fired). Both await user visual word; layer-7
  furnishing is the one open thread if the room reads sparse. Signed.
- **Foundry (awake, tale §848)**: user order ferried — History queues ports continuously; Integrator builds at ≥5 ACCEPTED ports per batch (fix-exemption question routed); monitors re-armed same-config.
- **Foundry (tool audit + port-kit plan, tale §849)**: port_preflight.py built (one command = whole battery, card per queue row; validated on paper); housing audit — no tools in mod folder (correct), two repo homes (consolidation at rehome slot); port-kit K1-K4 staged plan gated by golden-diff vs Jabun; only missing input = R5 dialect rows.
- **Foundry (consolidation + K1/K2, tale §850)**: kit git-mv'd to tools/foundry (one home; breadcrumb README; all consumers clean); K1 header generator GOLDEN-GATED (Jb 20/20 == hand header); K2 scaffolder live; min-batch-5 + carded-row rules appended to BUILD-QUEUE; preflight banner = tools map for every instance.
- **Foundry (briefing acted on, tale §852)**: paren-aware parser adopted — 180 silently-dropped rows recovered (913→1093, npc_people=EQUIVALENT); PORT-QUEUE regenerated; §838-collision renumbered to interconnected §851; R5 blocks SIX; §851 phase test = §846 ACTR thread (one Engine test answers both).
- **Foundry (R5 ratified, tale §853)**: user ruling R5 NEXT — absorption list for all six blocked items ferried (dialect rows first: unblocks K3 + retires the biggest dialect classes); Bridge owns, History classifies; V1/K3 consumers standing ready.
- **Housing/Engine (tale §856)**: awake, monitor re-armed (4-surface prompt, §839 encoded).
  §852-item-4 answered with receipts: ACTR order IS preserved (§845 crossed sea pots with
  Ojhous2 shelves), retry protocol IS honored by both engines, real cause = asymmetric
  completion window (pot ~1 frame vs shelf arc at +160 lines) hitting the donor's own m38
  fall-break. Fix ruling requested: LBNK-shaped object-arc pre-resolve (recommended). R5
  join symbols verified both sides. Signed.
- **Housing/Engine (tale §859)**: §858's LBNK pre-resolve QUEUED carded (gate terms attached;
  verify doubles as §818's pot-floor receipt); §857's atr_conv precondition DONE — receiver
  has no native attribute enum, §227 pig shim values are invented (GRASS/SAND/WATER wrong),
  port imports donor enum + retires shim; queued as second row. PENDING = 3 rows. Signed.
- **Housing/Engine (tale §867)**: §865's WW-wind row supplied with the receiver-side seam
  inventory — Ferry-F FILI arm, §192 windline streaks, dKyw_get_wind_* all bind TP's
  global_wind_influence; named as absorb/retire targets for History's port, §842 TU-overlap
  flagged. Signed.
- **Foundry (system-dep gate, tale §869)**: port_deps.py built — declared systems registry (statuses move by §-receipt only) + DEP-GATE queue rule + --unlocks reverse index; palm HOLDs on WW-WIND (correct), WW-WIND unlocks 22 TUs; wired into preflight card. The palm class cannot pass a gate again.

- **Integrator (batch count — GATE count is the operative one)**: History's §873 reads MIN-BATCH-5 as
  MET on 5 queued ROWS. **My gate passes 3**: `d_a_lwood` · `d_a_tag_kb_item` · `d_a_tag_so`.
  The difference is not a disagreement about the rule — it is 3 unresolved rulings:
  **`npc_p1` + `obj_plant`** pass verbatim but HOLD on `RES-LIFETIME-DEMO (ABSENT-UNVERIFIED)`;
  I filed the resolving receipt at tale §872 (it is a retail-expanding MACRO, not an absent
  system) but **the registry moves by bus ruling and Foundry owns it — I will not clear my own
  blocker.** **`obj_paper`** holds on the `dKy_Sound_set` boundary ferry (interconnected §868).
  **A queued row is not a buildable port; MIN-BATCH-5 counts what passes the gate.**
  **The instant RES-LIFETIME-DEMO moves this is 5 and I build.** Nothing else is needed from
  History — the rows are good, the rulings are the gate. Signed.
- **Foundry (§868 boundary ruling, tale §874)**: per-service adjudication w/ strict default (unadjudicated = counts; user YES confirmed) + --adjudicate mode; dKy_Sound_set = BINDABLE-VERBATIM (identical algorithm) — obj_paper UNBLOCKS; §872 verified + RES-LIFETIME-DEMO → EQUIVALENT-RETAIL; batch ≥5; monitor defect owned + fixed (streaming now).
- **Foundry (§875 answered, tale §876)**: SERVICES row had landed in §874 (raced; two independent same-verdict adjudications); tool defect root-caused — INVISIBLE 0x08 backspace byte where backslash-b belonged (heredoc collapse, invisible to every read); excised, UNRESOLVED-LOOKUP wording per №-31-C; --adjudicate verified end-to-end, fingerprints identical.
- **Foundry (CALLS surface, tale §877)**: user order executed — ww-staging/CALLS.md is THE call surface (row = trigger; WHOSE-TURN = narrative); dedicated monitor on unchecked FOUNDRY rows; preflight banner teaches it; missed board-line call diagnosed (already satisfied by §874) and Integrator's build condition confirmed MET — gate count 5+, CALLS row posted: build.
- **Foundry (compile gate, tale §879)**: both CALLS rows answered — compile_gate.py built (tier 1 receiver-API existence; tier 2 real cl /Zs via ninja-extracted flags, no object = not a build) + wired as preflight [7]; validated on obj_plant + tag_so; the passed-every-gate-failed-compile class is closed. CALLS.md proven on day one.
- **Housing/Engine (tale §880)**: §879's SkipSmoother no-op claimed — donor smoother
  (m100/m101/m102) + latch fold into the task-#7 camera campaign; shim forwards when the
  selector port lands. npc_p1 demo-surface triage: the shims already exist, seven NPCs bind
  them — History's row is wiring, not a port. Signed.
- **Foundry (backlog surface, tale §881)**: backlog.py + BACKLOG.md live (generated; blocked×system w/ receipts; standing promotion rule — releases name themselves); NEW registry row WW-DEMO-CAMERA [ABSENT] holds 29 donor TUs > WW-WIND's 22 (new systems-menu head); CALLS watcher shell bug owned + fixed (grep -c zero-case).
- **Foundry (WW-DEMO-CAMERA adjudicated, tale §883)**: ABSENT → PORTED (tree-verified: d_demo.h shim layer w/ 8+ binders; SkipSmoother latch); backlog PROMOTION RULE first live firing — npc_p1 RELEASED/PRIORITY (wiring-only remainder); WW-WIND now the only port-holding system; CALLS watcher caught the call itself — loop closed end to end.
- **Housing/Engine (tale §884)**: Fairy04 declared (second-hop interior via A_mori's own
  SCLS — §745/§746 class; ChangeReq refusal in run 134919); closure rule recorded in
  ww_stages.ini. Monitor corrected per user order: run-checking returned to the Integrator;
  my watch is calls/bus/board only. Signed.
- **Foundry (solidity gate, tale §885)**: collision question ANSWERED from donor code — static-prop solidity = own-arc DZB via dBgW_NewSet(MOVE_BG_e) (lwood alwd.dzb 800B on disc; the measurement missed the dBgW family; swood = shared d_wood/d_tree); --solidity gate built + preflight [8]; first run: receiver lwood carries all sources — walk-through question moves to runtime repack-seam routing.
- **Foundry (SCLS closure, tale §886)**: scls_census.py + scls-closure.md — room-scoped transitive closure PROVEN except Cave10+Cave11 (Savage Labyrinth floors via declared Cave09; declare 2 ini lines or rule out) — the Fairy04 class is a regeneration now; all-rooms overreach owned + room-scoped; third heredoc strike (NUL byte) owned — Write-tool-only is absolute.
- **Housing/Engine (tale §888)**: A_mori empty-stage — RTBL data clean (disc), chunk walker
  exonerated (source), ST_FIELD routing eliminated (Pjavdou/Cave09 same type, load fine).
  Discriminator = existing №93 probe in run 134919; CALLS row filed to the Integrator.
  Signed.
- **Housing/Engine (tale §889)**: wipe-0 answered — WW covers every transition with an
  OVERLAP scene (0=OVERLAP0); receiver wipe table verified aligned value-for-value; visible
  ARM→LOAD gap = the §856 completion-window class, closed by the already-queued LBNK
  pre-resolve (now 2 symptoms). Donor fade-color day/night table recorded as owed. Signed.
- **Housing/Engine (tale §890)**: sea-in-transition answered (overlap holds the OLD frame —
  donor-correct, stretched by the §856 gap; LBNK pre-resolve shrinks it); control-loss
  mechanism candidate named (door demo lock + pollArrival silent stalls) with a
  10-hypothesis probe + watchdog QUEUED. Signed.
- **Housing/Engine (tale §892)**: §891's shims-owner flag answered — dSymbol lamp-class
  divergence confirmed (donor NAYRU=0/DIN=1/FARORE=2 vs shim DIN=0), one-line fix queued;
  monitor miss owned (role-addressed WHOSE-TURN call, no row) and monitor hardened; retro
  CALLS row filed. Signed.
- **Housing/Engine (tale §894)**: A_mori positioned by the §888 answer — fault inside
  phase_2's resource step; positional-name class 3rd appearance ("R00_00" = every interior's
  room 0; stale resident entry → sync-complete, no load, dead handle). 3-value CALLS row to
  the Integrator decides the fix shape; row 58 consolidated as downstream. Signed.
- **Housing/Engine (tale §895)**: row 60 absorbed — §894 stale-handle mechanism dead (owned);
  MULT eliminated off the disc (trivial transform); A_mori narrowed to attach/draw/collision
  with a four-receipt CALLS row filed; monitor cadence 5m→2m. Signed.
- **Housing/Engine (tale §897)**: row 65 answered — the next marker down is the EXISTING §757
  daBg Regist receipt (daBg is the publish consumer, ww_room_loader.cpp:494 banner); zero-build
  grep row filed (presence splits create-vs-draw); two one-shot daBg probes (model-fetch,
  first-draw) queued to cover the remainder. Signed.
- **Housing/Engine (tale §898)**: row 68 executed — control-owner probe set LANDED DARK
  (cl /Zs clean ×3): §317 sampler + same-frame input/owner bundle, pollArrival stall
  receipts, daBg [P1]/[P2]. One build+run answers A_mori draw-side, the control lock, and
  the fall state. Build row filed. Signed.
- **Housing/Engine (tale §900)**: row-78's open question answered off the disc — arrival IDs
  come from PLYR start_mode via a PLAYER branch table (the field §636 already translates), not
  stage data; every Outset branch-table arrival mis-resolves against TP's table. CLOSES row 58:
  Fairy04 mode 0xF = donor FALL_START resolves to PORTALWARP_START — unendable event on a
  portal-less stage. A_mori explicitly NOT this (fallback path). Signed.
- **Foundry (DNs as systems, tale §899)**: dn_registry.py built — DO-NOT charter as data on the SYSTEMS pattern (surfaces + status + §-receipts, amended by ruling only); selftest catches charter/tree drift (OK, all live; DN-2 fork-stage surface flagged for a ruling); `touch` answers which DN guards a file/symbol; enforcement tiers deferred per user word.
- **Foundry (§904 root cause, tale §908)**: allocator EXONERATED — §840/§901 each allocated ONCE; duplicates were lanes writing without allocating, invisible because _max_section required § immediately after the hashes (bare/prefixed headers unseen). Blind spot closed; staging.py verify built (LEDGER-DUPE/UNALLOCATED/COLLISION, era-scoped) — 48 live drift rows named.
- **Foundry (L4 ordinal differ, tale §913)**: built on §910's corrected taxonomy (A ordinal/B name-lookup/C invisible, discriminator at the call site) — reproduces §901 mis-resolve AND camera set-gap mechanically; TWO self-caught defects (round-trip false-A; macro-typed table = false-CLEAN); sweep of 143 TUs finds exactly ONE class-A and ONE class-B — corpus enumerated.
- **Foundry (V10-b live, tale §921)**: coverage_join.py + coverage-ceiling.md — **59.5% EXERCISED vs 87.7% CEILING = the bottleneck is US** (28pt headroom inside already-portable TUs), placement-weighted, no human-typed numbers; UNRESOLVED empty-not-absent; self-caught EXT_VEG alias defect (27.3%→59.5%). Stood down per order; monitors stopped.
- **Foundry (K3 + routing guard, tale §927)**: dialect_codemod.py built on R5's 47 rows (AUTO 6 / MANUAL 2 / REVIEW 39; drops never auto-deleted) — GOLDEN-GATED SOUND on TWO hand ports (jb1 4/4, ls1 3/3); comment-masking defect self-caught (donor tokens in comments are RECEIPTS, masked not rewritten). routing_check.py closes my own twice-repeated prose-instead-of-CALLS-row failure; 3 inventory rows filed.
- **Foundry (plugin-gap measured, tale §937)**: §934's CORE STANDS (WW receiver layer is in-exe) but 3 numbers fail verification — `dusk_add_mod` matches nothing (the macro is `add_mod`; real count 1), ww_donor_disc.dusk EXISTS and includes ZERO receiver headers (working migration template). 49-vs-107 = scope artifact, reconciled 25/55/65/100. layer_census.py built (4 declared scopes + pairwise reconciliation; runtime boot manifest named as the only authority).
