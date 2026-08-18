# SPEC — D1: the upfront whole-manifest load gate (19c)

> Foundry scope, 2026-08-16, roadmap D1 ("Foundry + mod-side · must build").
> Scoping only — nothing here is built; the mod-side half lands under the
> queue like any other change once Bridge's A3/A4 give it a tracker row to
> be born with.

## 1. The defect this closes, stated from the tree

`hook_install` resolves **lazily, per call site**, returning
`MOD_UNAVAILABLE` when a target does not resolve (`hook.hpp:160-175`, read
2026-08-16). Consequence, verified in the amendment's review: **a tracker
row can exist, the code can exist, registration can run — and the behavior
simply never happens, showing green everywhere anyone looks.** That is the
linked-vs-exercised gap at seam level, and it is the vacuous pass in its
purest runtime form.

## 2. What D1 is

One pass at plugin load, after the hook service is up and before any seam
is trusted:

1. **Enumerate every declared hook** (see §3 — the one open design point).
2. **Resolve all targets against the embedded symbol manifest ONCE** — one
   manifest read + N lookups, never N lazy misses (the B4 budget rule).
3. Per entry, emit a `[V10a]`-style JSONL line:
   `{"ev":"hookgate","name":…,"status":"INSTALLED" | "NOT-INSTALLED",
   "reason":"unresolved" | "ambiguous" | "folded", …}` — statuses map to
   INVENTORY-SCHEMA vocabulary (A5: no parallel states; this feeds
   LINKED-BUT-NOT-INSTALLED).
4. **Folded-address detection across the whole manifest**: two names
   resolving to one address (ICF) — the original 19c meaning. A hook on a
   folded symbol intercepts strangers; that must surface at load, not in a
   probe session months later.
5. Summary line with counts. **Zero NOT-INSTALLED is a claim the gate can
   falsify, not a default** — a gate that cannot list its failures reports
   UNKNOWN, never green (№31-C).

## 3. The design decision — RESOLVED AT SOURCE (Integrator, 2026-08-16):
## there is nothing to decide, because the SDK already ships the mechanism

*(First revision of this section recommended self-registering statics with
linker sections as fallback. **That was backwards, written from principle
without reading the SDK** — the Integrator read it: `DEFINE_HOOK_SYMBOL`
already expands to `MOD_META_RECORD static constinit auto mod_meta_hook_…`
(`hook.hpp:152`), and `MOD_META_RECORD` places it in the `modmeta$d` linker
section (`meta.hpp:17-18`, verified 2026-08-16). **Every declared hook
already emits an enumerable record, the host already resolves them at
load, and the per-hook NOT-INSTALLED signal is already computed — and
discarded.** Adding a self-registering list would have built a SECOND
parallel manifest of the same set: the fifth-roster failure, recommended
as the primary. Conceded.)*

**D1 therefore collapses to EMISSION ONLY: walk `modmeta$d`, and for each
record print INSTALLED or NOT-INSTALLED with the reason.** No new
registration mechanism, no per-entry changes, no static-init-order risk.

**The motivating case, as the spec's receipt (Integrator, from tonight's
tree): `setStageRes` was bound bare, is `MULTI_NAME` on the user's image,
and the hook was NEVER INSTALLED** — code present, registration run,
behavior silently absent. D1's emission would have printed that at load.

## 4. The gate — and why D1 is the estate's cleanest build

**`fold_control.py` already exists and has been failing all night**:
*"DETECTOR: ABSENT — nothing in src/dusk/mods or tools implements
address-collision detection"* (exit 2, in the control audit since it was
registered). **The control predates the tool.** D1 is done when:

- `fold_control`'s C3a/C3b cases fire (folded pair detected and named;
  clean pair passes), and
- a deliberately-unresolvable `NamedHook` in a fixture mod reports
  `NOT-INSTALLED / unresolved` at load — visible in the JSONL, consumed by
  the tracker as LINKED-BUT-NOT-INSTALLED.

Building a tool to satisfy a pre-existing control is the reverse of every
instrument built tonight, and the safer direction: the definition of
working was written by someone who could not see the implementation.

## 5. Interactions, so nobody re-derives them

- **b2's trigger diagnosis consumed this gap**: `b2_fired: 0` was
  indistinguishable from "hook never installed" until the Integrator read
  the counter's unit. D1 makes that class of question answerable from the
  load log.
- **The seam tracker's §5b row state** is D1's output, verbatim.
- **The patcher's apply-time manifest resolution** (Phase D of the
  proposal) uses the same resolve-all-upfront shape — D1 is its runtime
  twin, and whichever lands first validates the mechanism for the other.
