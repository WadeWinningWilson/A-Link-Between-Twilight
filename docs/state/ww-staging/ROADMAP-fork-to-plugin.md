# ROADMAP — fork → plugin, with the patcher question settled

> Routed to Foundry as advisory, 2026-08-16. Verbatim as supplied by the user.
> Formatted copy; the same text is carried verbatim in the CALLS row that routes it.

Topology, stated so nothing below misreads it: dusklight = origin, a third-party project we do not own. Our fork = upstream, ours. Anything described as "propose to origin" is a request to people outside this project, on their timeline, with no guarantee of acceptance. No step below depends on origin accepting anything.

## The question that was open, and its answer

Whether to keep building a plugin, or instead build a patcher — build a WW-enabled dusklight and a stock one, diff them, ship the delta, apply it to the user's copy at install. That is TTW's PatchMaker model, and having source is exactly what makes it possible for us where it was impossible for them.

It is structurally premature, and the reason is decisive: a patcher presupposes a finished gold master. TTW built PatchMaker after years of stable gold-master development, because you cannot diff a moving target. The WW fork is under half of Outset with open bugs — a diff regenerated against it would churn continuously.

So this was never plugin-versus-patcher. It is plugin now, because it grows incrementally, and patcher possibly never, because the plugin may reach everything.

## The core tension

Content work happens in the fork. The fork's WW code must end up in the plugin. Migrating a moving target is expensive, and the fork is moving.

The resolution is not to freeze content or rush migration. It is to stop the fork's WW surface from growing, then let migration drain it while content work continues. That converts "we're a long way off" into a countdown with a number attached.

## Phases

| Phase | Steps | Exit gate |
|---|---|---|
| 0 — Stop the growth (now) | Declare the pilot proven (`ww_donor_disc` runs on stock dusklight). New WW work goes plugin-side by default. Fork work permitted only where 19a shows the plugin measurably cannot reach — each instance logged with its reason. Pin the ratchet baseline at the boot-manifest count. | Tree-side WW TU count stops rising and begins falling |
| 1 — Drain the leaves | Migrate Wave 0's six true leaves (`ww_import_gate`, `ww_jpa`, `ww_cam_crawl`, `ww_profile_register`, `d_ext_room_verify`, `d_ext_save_flags`) — P1 is their only blocker. Each validated by the conformance check. Content work continues in parallel, untouched. | Wave 0 migrated, no regressions |
| 2 — Finish Outset on the hybrid | The actual goal: complete Outset, close the open bugs (sea room=0, control-never-returns, A_mori, the arrival-seed root). Migration proceeds opportunistically — any TU touched for a bug moves plugin-side if it can. | EXERCISED approaches the 87.7% ceiling |
| 3 — Drain the interlocked | Waves 1–4 by 19a's ranking. Check whether `d_ext_npc_mount` should be retired rather than migrated — it is Wave 4's heaviest item (378 sites, four stacks) and DN-9 exists to kill it. Migrating a scheduled deletion is waste. | Tree-side WW count → 0, or a declared irreducible residue |
| 4 — Ship | Prebuilt plugin, per the standing distribution ruling. | User |

## On proposing to origin

The `'BMDL'` `#if DEBUG` gate is worth proposing to dusklight — it contains zero WW, it restores decomp-faithful code their own debug builds used, and if accepted it removes a hook we would otherwise carry forever.

But build the hook as though the proposal will never land. It costs nothing to ask and nothing to be declined. It must not appear as a dependency in any phase above.

## On diffing now

Do it — as measurement, not as product. A fork-vs-stock diff today is a migration worklist: exactly what the plugin must eventually replace. It is the same answer 19a produces from the opposite direction, so cross-checking the two validates both, and any disagreement between them is a finding worth chasing.

What it is not is progress toward a distributable patch. Treating it as such is what creates the back-and-forth.

## Escalation to a patcher — bounded

Unlocks only when a specific case is proven unreachable by plugin hook and rejected or unavailable at origin. Concretely: something inlined with no symbol, ICF-folded beyond reach, or requiring a receiver struct layout change. Written proof plus the user's go, same shape as DN-10's ladder.

The current BMDL case does not qualify — it has two cheaper answers (supply the dispatch case via hook; or origin un-gates it).

## The one number that says whether this is working

Tree-side WW TU count — pinned at Phase 0, monotonically decreasing thereafter. Not "percent ported," which measures content. This measures whether the architecture migration is real, and it is the only number that distinguishes "finishing Outset in the fork forever" from "actually leaving."
