# BRIEFING — P1 is viable. Roadmap to a patch-free (or near-patch-free) delivery.

> **ROUTED VERBATIM TO ALL LANES on the user's order, 2026-08-17.**
> Foundry is the router only. **Everything below the first rule and above the
> last is the user's text, copied without edit** — no lane should read Foundry's
> framing into it. Foundry's own retirement call is a SEPARATE row.
>
> **DECODER IS EXEMPT FROM THE RETIREMENT ORDER THAT ACCOMPANIES THIS BRIEFING.**
> Decoder does not retire, is not addressed by that order, and continues.

---

## What changed

Measurements behind the current architecture were taken against a May-2026 vanilla dusklight. Our branch and our fork's main are both 2,861 commits behind origin (TwilitRealm/dusklight), diverged 2026-05-08. Since then origin has shipped:

SaveService — mod-owned persistent named blobs (set_blob/get_blob/peek_blob, plus save lifecycle events). StageService — add_actor/patch_actor/delete_actor into existing stages by (stage, room, layer). ItemService, WindowService, mods::hook::uninstall, the embedded symbol database, and a Game ABI/headers refactoring.

And open PR #2270 "Mod SDK: Actor service" (reviewed favorably, awaiting final approval) allocates profile IDs above the fixed enum (fpcNm_MAX_NUM + index) and extends both dStage_searchName and dStage_getName at the lookup site.

Consequence: two of the three things that killed P1 are gone. The void* defect class dissolves because docs/modding.md documents "include game headers, call directly into any public function" — header-ful is the intended model. The 829-entry/13-free profile ceiling dissolves via #2270. One question remains: the ABSENT-unhookable seam (dExtWwSave_registerWwStage / dBootStage_add).

## Phase 0 — BASELINE, before anything is touched · Integrator + Foundry

The merge is the risky step and it must be reversible and attributable.

Capture, on the current tree: field FPS (the 220–250 band), the 8/8 assert battery, golden traces, the exe hash and build ID, and the current seam census numbers. Do the merge in a worktree or branch; the current branch stays intact as the reference.

Standing rule for this whole phase: anything that moves after the merge is a merge artifact until proven otherwise. Do not debug it as a WW bug first. The alarm bands apply — ~100–127 is documented as soft-poison/bad-link, and a 2,861-commit merge is exactly the event that produces one.

## Phase 1 — THE MERGE · Integrator, with Housing sweep

Fetch origin (the 2,861 figure is as of a July fetch — get the real number first), then merge into a working branch.

Expect conflicts precisely where the seam work already predicted them: the eight leg-carriers (d_door, d_stage, d_particle, d_event_data, d_a_swhit0, f_op_msg_mng, f_pc_profile_lst, JASChannel), the tree-side WW TUs, and the ALBW surfaces. The ownership map is the conflict map — this is what it was for.

Note GameService went to major 2 on Aug 12, an ABI epoch bump: game-visible struct/vtable layouts changed. Expect real breakage, not just textual conflicts.

Housing runs the covenant sweep on the merged tree before anything else is trusted.

## Phase 2 — RE-MEASURE · Foundry

Re-run against current vanilla: the 122-seam census, binding_plan / 19a, and the doorway classification. Specifically determine how many of the 87 ABSENT-hookable are now EXISTS or covered by a published service, and whether the 1 ABSENT-unhookable survives.

Everything downstream branches on this number. It is also the honest answer to "is the patch needed at all."

## Phase 3 — ADOPT THE OFFICIAL SERVICES · Engine + Bridge

Replace bespoke mechanisms with published ones. This is itself a mod-compatibility win — a mod that hooks dStage_searchName may conflict with #2270's hook; a mod that uses the actor service composes under the host's arbitration.

SaveService replaces any receiver-save-struct requirement — WW flag space becomes a mod-owned blob. StageService replaces DZR row baking for placement. The actor service (#2270) replaces the above-enum hack and the hook trio, and closes R1's unfalsified escape (reverse dStage_getName, direct table walks).

## Phase 4 — THE LAST BLOCKER · three routes, in priority order

4a — RE-SITE IT FIRST (Foundry, cheapest, no external dependency). ABSENT-unhookable is a property of the symbol, not the effect. The fork invented a registry; the effect is that stage load behaves differently, and vanilla has a stage-load path. Investigate hooking the consumer rather than reproducing the producer. If this works, the patch count goes to zero. Nobody has looked.

4b — PR TO ORIGIN (user rules; slow, external, best outcome). If re-siting fails, propose the facility upstream. Note: custom stage registration has no facility, no issue, and no PR — so this would be new ground, not a nudge.

4c — TINY BRIDGE PATCH (Engine; interim only). So work is never blocked on an external timeline. Build it disposable by design: one seam, symbol-resolved, no logic — only a redirect — and its removal condition written down the day it is created (e.g. "delete when origin ships stage registration, or when 4a lands"). A patch without a written expiry becomes permanent by default.

## Phase 5 — DROP HEADER-FREE · Engine

The constraint was self-imposed and contradicts the SDK's documented model. Going header-ful kills the R2–R6 defect class — the four-out-of-four authored-constant failures (dzb index, vtable invariant, TEV_TYPE_BG0, UseSingleDL) were all things a compiler checks.

The layout guard already exists: GameService is an ABI epoch marker, and the loader rejects incompatible mods with a clear error rather than corrupting memory. Build no parallel mechanism.

Rebuild plugin-side actors as typed. Then measure — lwood tree-side vs plugin-side, same route — to settle the per-frame boundary cost. If it's non-trivial, the answer is a hybrid split, not a retreat: hot per-frame subsystems (ja1 audio first, since sequence timing jitter is audible) stay tree-side; actors go plugin-side.

## Phase 6 — CONTRIBUTE UPSTREAM · user + Engine

TakaRikka's review on #2270 asks for a sample mod. We have a working demonstration of exactly that path — lwood, akabe and TagSo spawning, registering collision, and drawing on unmodified dusklight, with fire receipts. The contribution contains no WW content. It helps the PR land, and the PR landing is what unblocks us.

This is the fourth time a WW-motivated discovery has turned out to be a clean, WW-free upstream contribution.

## Running alongside — DOCTRINE SUPERSESSION · Librarian

SPEC-patcher-requirements.md, WHY-NOT-ALL-PLUGIN-SIDE.md, the seam-tracker proposal, and R1–R6 were correct for their target and are now measured against a stale vanilla.

Stamp them, do not delete them. Each gets a header noting the measurement date and that it predates the actor/stage/save/item services and the Game ABI refactoring. Their findings remain the record of what was true at that binary.

## Three planning facts to carry

#2270 is not merged. Reviewed favorably, awaiting encounter. Phase 3's actor-service step is gated on it — or on vendoring its mechanism locally as a bridge.

No released dusklight contains the mod SDK. v1.4.1 shipped Jun 16; sdk/ landed Jul 7. Delivery today means users on a main build. The API is explicitly "not yet finalized" — which is a cost for shipping and an opportunity for influencing it.

Update effort is per ABI epoch, not per release. GameService has bumped once (major 2, Aug 12) since the SDK landed. Header-ful mods are invalidated on epoch bumps, not weekly — materially cheaper than the per-release treadmill a patch model would carry.

---

*End of verbatim briefing.*
