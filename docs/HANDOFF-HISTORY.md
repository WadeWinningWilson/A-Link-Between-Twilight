# HISTORY — lane handoff

> ## ⚠ THIS LANE HAS THREE HANDOFFS. THEY ARE DIFFERENT KINDS, NOT COPIES.
> *(Cross-reference added 2026-08-16 by History/Bridge, on the Librarian's estate
> audit. Until now none of the three cited any other, and each opened with a
> "read this first" line — so a successor could read one and never learn the
> other two existed. Nothing below was merged or rewritten; only this block was
> added, identically, to all three.)*
>
> | doc | kind | last touched |
> |---|---|---|
> | [`docs/HANDOFF-HISTORY.md`](HANDOFF-HISTORY.md) | **LANE CHARTER** — what History is, its laws, traps that cost rounds, docs that matter. Durable. | 2026-08-11 |
> | [`docs/state/HISTORY-HANDOFF.md`](state/HISTORY-HANDOFF.md) | **CAMPAIGN SNAPSHOT** — the native-dMesg / Grandma-tale attempt. **Its "current attempt" and "immediate next steps" are STALE: that tale closed 2026-08-01.** Read for calibration (§4 "where the previous instance was wrong"), not for state. | 2026-08-01 |
> | [`docs/state/ww-handoff-history-bridge.md`](state/ww-handoff-history-bridge.md) | **CURRENT INSTANCE HANDOFF** — what is owned, half-done, do-not-trust, and the pointer to the live WW message-system decode. **Start here for present state.** | 2026-08-16 |
>
> **If you are picking this lane up: read the CHARTER for the rules, the CURRENT
> handoff for the state, and the SNAPSHOT only for calibration.** Which of the
> three should be canonical — or whether they should merge — is a Librarian/user
> ruling, deliberately not taken here.

Written 2026-08-08. Read this before touching WW-lane work. Everything here is either a
standing responsibility, a live campaign, or something easy to forget that will cost a round.

---

## 0. THE ONE THING TO KNOW FIRST

**CORRECTED 2026-08-10 (History):** the paragraph this replaces claimed the switch was ON and
the hang fixed at A4b. Both claims were stale when written — the stale-state failure class §
STATE-ASSERTION SWEEP names. The measured truth:

- The A4b build (exe 23:16:29) **still hung** at `[Evt] §350c entry() GRANT` (run 232642,
  §468 — reproduced across two builds). §471's C1 falsification then **confirmed A4 owns the
  hang**, and `DUSK_EVT1_NATIVE` was committed at **0** pending History's ratification.
- **§423 A4c (2026-08-10) found the hang's mechanism:** `evt1_specialProc`'s default leg
  called the hooked `dEvDtStaff_c::specialProc()` — unconditional mutual recursion for every
  staff type without a named case (CAMERA/DEFAULT/SHUTTER_ = 26 of the tale's 39 staffs);
  under /O2 tail calls it spins instead of overflowing — a hang, not a fault, exactly §468's
  signature. Fixed in `evt1_event_data.cpp` (receiver per-type procs called directly);
  probes P-A4c.1–4 armed; switch restored to **1** for the retest.
- **Test (unchanged):** ① the tale, full run including get-item and teardown · ② doors both
  directions · ③ **a mainline TP cutscene as the control** — it must be unchanged.
- **If anything misbehaves:** set `DUSK_EVT1_NATIVE` to `0`. That restores pre-A4 behaviour
  *exactly* — the promise holds by construction through A5 (see §423b).

---

## 1. WHAT HISTORY IS

The decomp-faithful WW→TP restoration lane. The standing covenant, in the order it gets tested:

1. **DECOMP-FIRST.** `D:/XXXXXXX/WW DP/src` is the spec. Read how the donor's own code does it
   *before* writing a fix. Skipping this has cost multiple rounds; it is the single highest-value
   habit in the lane.
2. **Native, not mounted.** Port the vanilla system itself; adapt only at the lowest boundary.
   Bridges are labelled and owed, never endpoints.
3. **Additive.** Never delete receiver code to make room for donor code. Keep both, label the seam.
4. **Scoped.** A WW-motivated change to a shared TP path must be runtime-gated or logging-only.
5. **The user rules.** Lanes recommend; only the user decides. Never self-approve an exception.
6. Every reply ends with a **WHOSE-TURN** block naming the lane that owns each next action.

**Sibling lanes:** Housing Security (J3D, lighting, sky, BDL) · Foundry (instruments, bakes,
space kit) · Bridge (pipeline, R1/R5) · Engine · Librarian · Decode.

---

## 2. LIVE CAMPAIGNS HISTORY OWNS

### §423 — parallel WW event stack (tier 2)
Port WW's event system alongside TP's so TP's files stop serving two games.
- **A1–A4b DONE** (boundary+gate, event data, manager, dispatch, hang fix). **A4b UNPLAYED.**
- **A5** = revert TP's event files to vanilla **but KEEP the scope gates** until A6 ratifies
  (Housing's §423b catch: the kill switch stops being a rollback the moment the gates go).
- **A6** = ratify on a green battery, *then* delete the gates. From that commit the switch is a
  build-time choice, not a rollback — say so in the record.
- Files: `src/d/ext_evt/*`, `include/d/ext_evt/evt1_boundary.h`.

### L4 — TP↔WW interaction mappings (data in R5, never inline)
`tools/conversion_db.py` → `INTERACTION_MAP`, `TP_INTO_WW`, `INTERACTION_MAP_OWED`.
- **Measured:** health (identical layout, **same quarter-heart unit** — damage transfers without
  scaling) · weapon AT_TYPE bits (**only BOMB agrees; five bits collide** — see §4).
- **Owed:** damage tables · **wolf.damage** rows · items beyond id 47.
- **Boundary with L-10:** L4 holds *values that translate*; L-10 (BY-DESIGN) holds *who performs*.
  If a proposed row's donor side describes WW **Link** rather than WW **data**, it is BY-DESIGN.

### Native rooms (current campaign, user-ruled)
Replace mount-era host-stage construction with true native room loading.
- Step 3 done: `Ojhous2/Room{0,1}.arc` → `R_DL02/R{00,01}_00.arc`, **byte-identical** (sha256
  verified). No bake, no chunk surgery — translation at load.
- **Flagged before the run:** neither arc carries RCAM, so Foundry's expected first test of the
  RCAM 0x14 branch **cannot fire on this data**. "0x14 never fired" = UNTESTED, not PASS.

### Still queued
`tsubo` port (49 placements) · Ji1/Aj1 · interior BGM + WW fire SFX + JA_SE id mapping ·
WW-demo-driven Link door animation · KB-1/KB-2 (below).

---

## 3. OPEN BUGS AND THEIR STATE

- **KB-1** (intermittent mid-cutscene kill) — **ROOT-CAUSED**: the №89 arrival G-guard was
  force-ending live events; its kill leg is **deleted** (§399). Probes remain armed pending a
  clean confirming run. `docs/KNOWN-BUGS.md`.
- **KB-2** (presentation gap at the STB→reload seam) — mechanism confirmed, fader-hold fix queued.
- **№89 remnants** — the arrival-end leg and №170 survive *only* until KNOB_START ends natively
  via its own staffs. Deleting them before that strands every arrival.
- **§398 probes** — still armed in the scene/overlap path; strip with the family.

---

## 4. TRAPS THAT HAVE ALREADY COST ROUNDS (the port-class registry)

Check every new donor TU against all five:

1. **GC pointer size** — donor `n * 4` over 8-byte pointer arrays (§372b: half-zeroed table → crash).
2. **Endian/bitfield overlays** — GC is big-endian *and* MSB-first bitfields; both must mirror
   (§375: the whole-stack audio silence).
3. **Name-collision enums** — shared names, different values. Receiver `ID_AK_JN_TORCH` = 0x41,
   donor = 0x01EA (§396: the flameless candle). **Always take the donor's number.**
4. **Name-mapping** (Housing's find) — the Ba1 crash was this, not a stale cache.
5. **AT_TYPE bit collisions** (new, 2026-08-08) — only `BOMB` agrees between games. Bits 6/9/10/
   **14**/**16** carry *different weapons* in each. Passing a donor attack word through unchanged
   turns a WW **arrow** into a TP **hookshot** and a WW **skull hammer** into a **boomerang**.
   Translate per bit; never pass the word.
6. **Donor data SCALE vs receiver-shaped capacity** (new, 2026-08-11, §727) — donor packs index
   far beyond receiver habits (sea event pack: staffNum=**992**, door staffs 271-273; the
   port-side scratch assumed TP-scale 64). Any mirror/scratch/cache KEYED BY DONOR INDICES must
   be sized from the DONOR'S data maximum — and a silent range-guard on an undersized table is
   WORSE than a crash: it converts the scale mismatch into invisible no-ops (`getIsAddvance`
   stayed FALSE, every on-advance init skipped, the 5-frame door death — five hypotheses spent
   before the bound was found). Masking (`idx & 63`) is the same trap plus cross-index
   aliasing. Size from data; never mask; if a bail-guard must exist, it LOGS.

---

## 5. LANE LAWS EARNED (mostly the hard way)

<!-- ============================================================
     FOLDED IN 2026-08-16 from docs/state/HISTORY-HANDOFF.md §4.
     That file is a CLOSED CAMPAIGN SNAPSHOT (Grandma tale, ended
     2026-08-01) but §4 was never campaign state — it is lane-level
     calibration that outlives any tale. It was living in a document
     whose title made the whole thing read as history, which is how an
     OPEN item in its §5 went unread for fifteen days. Durable content
     belongs in the CHARTER; the snapshot keeps only its own campaign.
     ============================================================ -->
- **Overclaimed certainty repeatedly** — said *"this is the fix"* on §319, §319b, §320 before
  verifying, and the user called it out. **Treat every fix as a hypothesis until the log or a
  playtest confirms it.**
- **Widening the textbox was a COVENANT LAPSE.** To hide a mid-word wrap a previous instance
  widened the box's wrap bounds — an invented layout. REVERTED. The real cause was a VALUE:
  retail font is **23**, not 25 (retail is the SECOND `DEMO_SELECT` arg; 25 is the kiosk-demo
  value). **A port/donor mismatch is a wiring or value bug to FIND, never to paper over.**
- **Verify sub-agent claims against runtime.** A spawn map claimed Link was not bound to the
  demo; the log proved he binds (`§48 JSGFindObject actor='Link' → FOUND`).
- **A comment is not a scope.** №93 and №62 sat twelve lines apart, both *reading* WW-specific;
  only one wrote the check. Tool: `tools/ww_scope_check.py`.
- **A safety promise has a scope too** — name the phase where it expires (Housing, §423b).
- **Derive, don't mirror.** State that must track shared state should be *derived from it*; a
  mirror needs a reset hook, and a missed hook is invisible until it hangs (§423 A4b).
- **Share what agrees, own what doesn't, guess at neither.**
- **Port on evidence, not inventory.** A census closed A2b's 605 speculative lines: no WW event
  uses the six staff procs it would have ported.
- **The shipped game is the spec.** `drawVrkumokage` exists only in debug maps — no port (§422).
- **An under-counting gap report fails toward looking busy** — the direction nobody audits (Foundry).
- **Stale artifacts want a detector; an ambiguous ferry wants a citation** (Housing). "B2b" cost a
  blocked turn; "B2b (§170)" would have cost nothing.
- **A scratch mirror is sized by the donor's DATA, not the receiver's habits** (§727) — and a
  probe that reads a value by a DIFFERENT path than the consumer is not evidence about the
  consumer (§730). Both from the same night: the 64-bound ate the advances silently while the
  direct-read probe swore the cut layer was healthy.

---

## 6. DOCS THAT MATTER

| doc | what it is |
|---|---|
| `docs/state/ww-tale-dmesg-live-state.md` | **History's live thread.** §N entries, newest at the end. The single most useful file. |
| `docs/WW Linked/ww-bridge-tool-interconnected.md` | the cross-lane bus (§N ferries) |
| `docs/WW Linked/port-liberties.md` | faithfulness-debt ledger. Statuses: OPEN · VERIFYING · RATIFIED · RECONCILED · **BY-DESIGN** |
| `docs/KNOWN-BUGS.md` | KB-1 / KB-2 with their armed probes |
| `docs/WW Linked/ttw-methods-review.md` | the R+V plan (R1/R5, L1–L7) |
| `docs/WW Linked/layer-precedence-spec.md` | Housing's L1 layering spec |
| `docs/DO-NOT.md` | hard-stop registry — read before touching a surface it names |
| `tools/conversion_db.py` | **R5**: the one consulted table. Every row carries a TIER. |
| `tools/ww_crew_restoration_skeleton/convert_all.py` + `recipe.json` | **R1** runner, 27/27 placed |
| `tools/ww_scope_check.py` | flags WW-tagged blocks in shared TUs with no runtime gate |

**External:** donor source `D:/XXXXXXX/WW DP/src` · donor assets `D:/XXXXXXX/Ex WW` ·
debug maps `D:/XXXXXXX/WW Debug maps` · **Winditor** `D:/XXXXXXX/Winditor` (law for authored
data) · noclip (reference-tier only, never law).

---

## 7. THINGS EASY TO FORGET

- **§334's acceptance was given** (§362, amendment closed §363). It is **not** owed. Its heading
  said "IN PROGRESS" for five days and cost three lanes a turn.
- **The mod folder is committed now** (Housing). The unrecoverable-single-copy risk is closed —
  which is why R1's urgency argument no longer applies.
- **R1 is folded into the native campaign**, not a side track: every mount step retired flips its
  recipe row; every native step added gets one. The recipe then can't document a stale pipeline.
- **The lineage burn-down is the campaign's scoreboard.** `recipe.json` `path` field:
  MOUNT + COMPENSATOR falling = the transition is real. 16 steps are UNCLASSIFIED and must be
  classified by their owners, **not inferred**.
- **№116 set the retirement ceremony**: evidence, tier change, DO-NOT-RUN banner, script kept
  re-runnable. A step deleted without it is indistinguishable from one lost.
- **Epona sits in two places** — `TP_INTO_WW` *and* AT_TYPE bit 2 (`HORSE`). Nobody had connected
  those until the bit-level read.
- **Additions ≠ deviations.** Epona and wolf-in-WW belong in *neither* current ledger:
  port-liberties tracks deviations from vanilla, and an addition changed nothing away from the
  donor. **User ruling owed** on where they live.
- Build only via `build_run.bat`; wipe `dawn_cache.db*`/`pipeline_cache.db*` after every build.
- Push to upstream **ALBW-Dusklight**, not origin. Don't commit unless asked.
