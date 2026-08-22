# PORTING ROADMAP — E1 through E6 (outside eye, routed 2026-08-21)

era: plugin-delivery
<!-- era rationale: forward plan for the plugin-delivery era | routed verbatim by Foundry on user order; analysis appended -->

**Provenance:** delivered by the outside eye via the user, 2026-08-21, with the
instruction *"copy everything between the rules verbatim and route to all
lanes."* The text between the rules below is **VERBATIM — do not edit it**.
Foundry's critical analysis (also user-ordered: *"needs to be critically
analyzed against the actual project"*) follows after the closing rule, clearly
separated. **The verbatim text is the outside eye's claim; the analysis is what
was measured. Where they disagree, the measurement carries.**

---

Instruction for the lane: copy everything between the rules verbatim and route to all lanes.

PORTING ROADMAP — E1 through E6
Context the lanes need first
The world-presentation ring has landed plugin-side. mods-src/ww_donor_disc is now 34 files — sky, vrbox, vrkumo, shore, wave, kankyo, kankyo_wind, three camera TUs, the message trio — with a lineage census of 17 native-port / 2 donor-port / 5 host-plumbing. That ring is done; the ladder below is what comes after it.

The aurora work-set is the patch's entire remaining justification. 13 files, ~1,008 added lines, of which ~920 are functional GX capability — display-list ownership and execution (GXAuroraSetDlOwner, GXCallDisplayList), vertex descriptors (GXSetVtxDesc, GX_POS_MTX_ARRAY), and TEV compare operations (GX_TEV_COMP_*) — and only ~74 are probes. aurora sits below the plugin boundary (it implements GX; the plugin calls it), and it compiles into dusklight.exe. So it cannot move plugin-side and it does require a binary patch.

That patch is disposable by design. Write its removal condition the day it is built: delete when upstream lands the aurora work. One justification, one expiry. Everything else is plugin-side.

Upstream offer is gated on E1 — per user ruling, lwood must be proven working before the aurora contribution goes out. In the interim the patch ships so users get it.

E1 — ONE ACTOR PROVEN END-TO-END · gates everything below
Owner: Integrator + Engine.

lwood through bind → create → collision → DRAW, on vanilla, with a receipt at each stage.

Current state, stated plainly: the chain has receipts for the first three — lwood_ready bound=10 missing=0 tev=1, lwood_created n=1..4, cbgw_set ret=0, dbgs_regist ret=0 — and no draw receipt. R1's "and draws a full frame" has propagated into several documents as demonstrated fact; it is not. The lwood draw crash is recorded UNROOTED and its four-mode bisect was deleted with the plugin revert.

Until one actor completes the chain, every method below is unvalidated. E1 also becomes the reference implementation every subsequent port measures against.

Gate: a draw receipt on vanilla.

Likely resolution note: the aurora work-set targets exactly the surface WW needs and TP never exercised — MDL3 precompiled display lists and TEV compare stages, consistent with "no bdl4 in 350 sampled TP archives." If E1 now passes, that is probably where the draw failure lived, and it should be recorded as the root rather than left open.

E2 — TYPED PORTING · blocked on nothing, start in parallel
Owner: Engine.

Drop the header-free constraint. docs/modding.md documents that mods get "full access to the original game's code: include game headers, call directly into any public function" — header-ful is the intended model, and header-free was self-imposed for a version independence the patch already spends.

This retires the R2–R6 defect class outright. The four authored-constant failures — dzb index, the vtable invariant, TEV_TYPE_BG0, UseSingleDL — were all things a compiler checks. Every port after E2 is structurally more reliable than every port before it.

Do not build a layout guard. GameService is an ABI epoch marker and the loader already rejects incompatible mods with a clear error rather than corrupting memory. It bumped to major 2 on 2026-08-12. Header-ful mods are invalidated per epoch, not per release.

Gate: plugin actors compile typed; no hand-authored offsets remain.

E3 — KIT LEARNING LOOP · cheap, and it decides whether porting compounds
Owner: Foundry (instrument) + History (port).

actor_kit has an apply tier, and its feedback loop ran one cycle — 4 AUTO + 1 REVIEW rules harvested from the Otble port. What is missing is the instrument that says whether it is working.

Track first-compile success rate per batch. Batch 1 was five of six failing. If batch five is two of six, the loop compounds. If it is still five of six, the harvested rules do not generalise — a different problem needing a different fix, and right now nobody would be able to tell which.

And classify every compile failure by cause. Five-of-six is systematic, not idiosyncratic; recurring causes are exactly what codemod rules should absorb.

Gate: a batch-over-batch trend line exists.

E4 — PLACEMENT-WEIGHTED ORDERING · stops intuition choosing what to port
Owner: Foundry emits · History ports.

Outset measures 87.7% portable against 59.5% exercised — 28 points of headroom inside what is already portable. The bottleneck is throughput, not availability.

port_matrix's join — l_objectName → procname → owning TU → decomp status → placement count — names the highest-value next port mechanically.

Weight by placements, never actor count. 49 DZR names resolve to LODBG, 32 to DEMO00, 19 to TBOX, 15 to Obj_Try, 12 to TSUBO. One actor closes dozens of placements, so actor-count percentages actively mislead.

Gate: the next-port queue is generated, not chosen.

E5 — SUBSYSTEM RINGS · separate pipeline, do not queue behind actors
Owner: Engine.

Whole-file subsystem ports are a different pipeline from per-actor work and should run concurrently. The world-presentation ring is done; the next two are what turn a rendered island into a game:

Events (evt1) — quests, cutscenes, staff-driven sequences.
Audio (ja1) — music and sequence playback.

Carry this caution on ja1: audio is the subsystem most exposed to the plugin boundary, because sequence timing jitter is audible where a few FPS is not. If the boundary cost measures non-trivial, ja1 is the first candidate to stay tree-side. That is a per-subsystem call informed by measurement, not an architecture retreat.

Gate: a WW cutscene runs; WW music plays.

E6 — UPSTREAM-FED RATHER THAN RECONSTRUCTED · already running
Owner: Decoder.

The Decoder's end state is drain-by-upstreaming — (A)-clean TUs go to zeldaret/tww. The payoff loops back: when a TU flips to Matching upstream, decomp_watch fires a confirm-or-replace trigger and the port consumes rather than reconstructs.

That is the difference between porting from drafts we authored and porting from source verified byte-identical by someone else. Slow to start, and the only step that raises the ceiling rather than the throughput.

Sequencing
Runs now	Runs in parallel	Waits
E1 — nothing else should start	E2 (blocked on nothing) · E5 (separate pipeline) · E6 (already chartered)	E3, E4 — cheap instruments, but the batch rate only matters once E1/E2 land

The one measurement that settles the rest
lwood tree-side versus plugin-side, same route, same build settings, measured.

That single number decides E5's ja1 question and the general tree-vs-plugin split for every future port, and it is available the moment E1 lands. Per standing doctrine — an FPS measurement without a baseline check is not a measurement — take the baseline before, not after.

---

# FOUNDRY ANALYSIS — measured against the actual project, 2026-08-21

Everything below was **measured today**, not recalled. Verdict shorthand:
✅ holds · ⚠️ holds with correction · ❌ premise wrong as stated.

## Context block — ⚠️ one count off, ❌ one premise materially wrong

- **"34 files"** → measured **37** (16 `.cpp` · 15 `.h` · 1 `.inc` · 1 json ·
  1 md · 2 py). Minor drift, no consequence.
- **❌ "the aurora work-set … compiles into dusklight.exe":** the 13-file /
  ~1,008-line capability set **is NOT in `dusklight-main` today.** Measured:
  `dusklight-main`'s aurora delta over vanilla's pin is **+2 local commits, 4
  files, 53 insertions** (skip-draw, TEV alpha-compare fix, gx logger). The
  roadmap's numbers match the **FORK's aurora submodule worktree dirt: 13
  files, 995 insertions / 27 deletions, UNCOMMITTED** on top of `81f12f3` —
  that is where `GXAuroraSetDlOwner` / `GX_POS_MTX_ARRAY` / `GX_TEV_COMP_*`
  live. Capture verified **byte-identical today** against
  `aurora-patches/fork-aurora-81f12f31.patch` (1,334-line diff).
  **Consequences:** ① E1's "likely resolution note" cannot fire on vanilla
  until the capability set is applied to `dusklight-main`'s aurora — **the
  roadmap is missing that step, and it is E1's real precondition.** ② The
  patch-content enumeration ("one justification, one expiry") must be written
  from the 995-line set, not the 53-line one. ③ Those 995 lines are the most
  loss-exposed bytes in the estate: **submodule worktree dirt is invisible to
  parent-repo commits.** The capture mitigates; the capture is itself in the
  uncommitted pile.

## E1 — ✅ gates the actor pipeline; two corrections

The no-draw-receipt claim is TRUE and stays true post-milestone (Outset draws
with trees OFF, mode 1). Corrections: **owner is Housing/Engine** (Integrator's
roster row consolidated; the roadmap's "Integrator + Engine" predates it), and
the E1 pass-evidence bar carries the standing caution: **skip-draw is in the
binary, so a pass must show the draw receipt AND `skip-draw path never fired`
AND a user-confirmed frame** — a green log alone is the catalogued vacuous
pass. Add the aurora-capability-patch application as E1 step 0 (see above).

## E2 — ✅ sound; matches the briefing's Phase 6

Version-independence is already spent (user's aurora ruling concurs). One
note: TEV_TYPE_BG0 among the R2–R6 examples no longer exists in the plugin
(deleted in the revert) — the argument stands on the other three.

## E3 — ⚠️ the kit EXISTS but the user's era-concern is CONFIRMED BY READ

`tools/foundry/actor_kit.py` (401 lines). Its CHECKLIST was harvested from
**tree-side, fork-era ports** (lamp §327, toripost §253, Aryll/Grandma), and
of its 8 rules, **three misdirect a plugin-side port**:

| rule | verdict |
|---|---|
| "model via `dExtNpcMount_acquireModelData` — NOT `dComIfG_getObjectRes`" | **MOUNT-ERA, actively inverted for plugin work** — mount is tree-side (57 files), on Engine's do-not list; the plugin's model path IS the receiver `getRes` choke point per the trace method |
| "shims → `d_ext_ww_actor_shims.h` if shared" | **tree-only** — that header is not in `dusklight-main`; plugin equivalent is plugin-local shims |
| "TEV via `settingTevStruct` §47 donor-look on WW host stages" | references tree-side host-stage machinery; needs plugin-side restatement |
| cPhs/assert · particle arity · modelCalc order · staff-claim · anm_prm endianness | **era-independent donor truths — keep** |

Plus two stale hardcoded paths: `MOD` points at `model_replacements/
WW-Crew-Restoration` (model-swap era) and `SCRATCH` at a dead session's
directory. **Retool owned by Foundry, on the plate.** The first-compile-rate
instrument is agreed and cheap; batch 1 = 5/6 failing is the recorded baseline.

## E4 — ❌ as stated: `port_matrix` DOES NOT EXIST

Zero hits for `port_matrix` across `tools/`, `docs/`, `mods-src/`. The join it
describes is real and its inputs exist (census: 87 names → 61 procs; class
A 187/B 101/C 187 placements; ownership map), and the placement-weighting
argument is correct — but the instrument must be **built**, not consulted.
Foundry builds it under E4 or the outside eye names where it lives.

## E5 — ⚠️ "world ring done" overstates by exactly the held-cargo list

Engine's own doc holds leaf sway · NPCs · Wave B · sunburst, and lwood is
open. The ring is done *as scoped*, with that boundary stated. The ja1
caution matches the zone model and the measurement-first ruling. **E5 ports
create FORK-STRIP-LEDGER rows** (evt1/ja1 are Zone-2 tree residents today) —
the roadmap and the strip protocol run on the same items in opposite
directions, deliberately.

## E6 — ✅ matches Decoder's charter as chartered. No corrections.

## The one measurement — ✅ with the standing sequencing note kept

Valid the moment E1 lands (the earlier "measure after typed rebuild" ruling
objected to measuring a draw-nothing path; with draw landed, the objection
dissolves). Baseline-first, per doctrine, and same build settings both sides.

## Sequencing — one amendment

E1 step 0 = apply the fork-aurora capability patch to `dusklight-main`
(measured above). Everything else in the sequencing table stands.
