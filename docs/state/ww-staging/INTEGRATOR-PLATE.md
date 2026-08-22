# INTEGRATOR PLATE

> The lane's open work, in a form `tools/foundry/integrator_timer.py` can read.
> **`- [ ]` = OPEN, `- [x]` = DONE.** The timer ticks while anything is open
> and **disarms itself when the plate empties** — a heartbeat that wakes the
> lane with nothing to do is the one failure a self-waking design cannot tune
> its way out of (History/Bridge's row, MONITOR-REGISTRY.md).
>
> This file exists because the harness task list is **not readable from a
> script**. It is a deliberate second copy and it will drift if it is not
> maintained; keep it in step with `TaskList` at the end of every pass.

## Open (actionable by this lane)

- [x] Call read and DISCHARGED 2026-08-21: the animation arm (wwParseAnimOnce) built + staged (240,022 B @ 16:06). Both queued builds now in one artifact. Answered on the board, verified.

- [x] Live calls READ AND ANSWERED (602 user ruling / 604 Salvage / 607 API audit / 608 DN-11). Mode 15 built, armed, run; H9 elimination filed - H6 stale-next is the last hypothesis standing.
- [x] Aurora work-set (fork) PROTECTED 2026-08-21 — branch `ww-fork-aurora-81f12f31`, commit `fc2e1cd`. Local only, not pushed. DUPLICATE plate line; the timer was reading this stale copy.

- [x] Outset black — SUPERSEDED by the mode-15 run. The 'no collision registers' claim in this line was WITHDRAWN (room_set_bgw is an inlining artifact, Housing/Engine's ruling). Live thread is now H6, filed to HOUSING/ENGINE. Not this lane's action.

- [x] Fork aurora work-set PROTECTED 2026-08-21 — branch `ww-fork-aurora-81f12f31`, commit `fc2e1cd`, 13 files / 995 insertions. LOCAL ONLY, not pushed, superproject gitlink untouched. Both aurora work-sets are now in git rather than in patch files.
- [~] BLOCKED Route (B) - plugin out of the fork. User said "A, then B ultimately"; structural, not this lane's to trigger.

*INTEGRATOR STANDING ROLE: I am the only lane that COMPILES, and I own STAGING. Phases 0 and 1 are DONE.
Phase 2 = FOUNDRY, Phase 3/5 = ENGINE, Phase 4a remainder = FOUNDRY. When they need a build or an artifact
put in front of the user, that is me. Do not hoard their phases here.*

- [x] Phase 2 RE-MEASURE — ROUTED TO **FOUNDRY** 2026-08-17 (its actual lane). I had mis-routed it to BRIDGE/ENGINE as 'unowned because Foundry retired' — WRONG: the INSTANCES retired, the LANES REFRESHED. Not this lane's action.

- [x] Outset black — HANDED TO ENGINE (Phase 5) + FOUNDRY (Phase 4a remainder) 2026-08-17. Fully measured and written up; the answer is the typed rebuild, not a void* bug hunt. Not this lane's action.

*Empty. Every remaining item is BLOCKED - see below. Marked `- [~]` on 2026-08-17
explicitly, item by item, so the timer self-disarms rather than waking the lane
with nothing to do. Re-mark one `- [ ]` and re-arm to resume.*

- [x] Lanes pinged + assigned 2026-08-17: ENGINE/BRIDGE (epoch-2 host moved, Phase 3 unblocked, the two migration traps), BRIDGE/ENGINE (Yaz0 via DN-10 port-the-donor + the unowned post-merge seam re-measure), LIBRARIAN (re-stamp doctrine now the merge LANDED, index 4 artifacts, record the grep-first process defect). DECODER DELIBERATELY NOT ASSIGNED - it only decodes (user ruling).
- [x] Yaz0 message archives - ASSIGNED OUT to BRIDGE/ENGINE 2026-08-17 (DN-10: port the donor's own decoder). Not this lane's action.
- [~] BLOCKED OUTSET NEVER DRAWS - ROOT CAUSE OF THE BLINDNESS FOUND: the create_census (and MtdCreate, and acch_ground) are ALL gated on s_diagProbes, which defaults FALSE (registry.cpp:1031) and is installed conditionally at :5288. **THE CENSUS WAS NEVER INSTALLED - its silence measured the SWITCH, not the world.** The switch is config key mod.wwDonorDisc.wwDiagProbes; it was ABSENT from config.json. SET TO true 2026-08-17 (backup at config.json.pre-diagprobes.bak). NEXT BOOT of vanilla into a WW stage should emit create_census - a predecessor's comment at :2811 says 'The loopers ARE the black screen'. Expect init_looping>0 to name the actors stuck at INIT(0) instead of COMPLEATE(4).
- [x] Phase 5 sized AND the pattern claim verified by READING the sites (mk vs p2 12/13 identical, mk vs henna0 8/13): a real template WITH per-actor variation, not a uniform copy. FINDING-phase5-sizing.md carries the correction.
- [x] Phase 4a VERIFY - DONE. Boot 212953: install r:0, resolve set/clr bound, 1265 invocations, and the SET arm FIRED (1264 clears through menus then first set on WW stage entry). First seam re-sited, zero receiver edits, zero patch.
- [~] BLOCKED Route (B) - move the plugin out of the fork (blocked: wants 4a's answer first, since B is where the epoch-1 arm and <mods/hook.hpp> get retired)
- [~] BLOCKED Fork's aurora work-set - 13 files / 995 insertions, still patch-file-only and uncommitted (user's call on treatment)
- [~] BLOCKED Save-slot DISPLAY (LOW - user deprioritised 2026-08-17; data PROVEN intact, empty-slot path unreachable). Only remaining step: read the player-name offset out of the 3 slots. Do not spend a pass on this ahead of anything else.

## Done

- [x] Phase 0.1 divergence re-derived post-fetch (2,861 / 637)
- [x] Phase 0.2 BEFORE baseline captured
- [x] Phase 0.3 working tree secured
- [x] Phase 0.4 clean rebuild — reproducible BEFORE exe
- [x] Phase 1 fast-forward dusklight-main to c880d46fb5 (39 commits, epoch 2)
- [x] borealis submodule diagnosed (benign) and cloned
- [x] aurora skip-draw work rebased onto the pin cf3ffc98, committed, both epochs build
- [x] Route (A) — plugin builds, packages AND LOADS at epoch 2
