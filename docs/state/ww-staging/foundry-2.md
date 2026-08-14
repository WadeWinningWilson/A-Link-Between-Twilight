# foundry-2 — actor_kit + enemy_port_kit retools (DRAFT, audit-specced)

> From the integrator kit audit (user-assigned). Both route through the now-
> landed OUTPUT LAW (tools/foundry/kit_output_law.py).

## actor_kit.py:311-314 — the arcs/ staging defect
Replace the overlay copy (MOD/arcs + arcs_lib) with: (1) VERIFY the donor arc
is disc-served (FST name check via ww_disc) — the normal case needs NO copy at
all; (2) if an overlay is unavoidable for an asset, route through
kit_output_law.lawful_copy with reason + strip trigger — silent staging is now
structurally refused anyway.

## enemy_port_kit.py — two defects
:52  guidance inversion — MODEL path teaches dExtNpcMount_acquireModelData;
     under DN-9/DN-10 step 1 is the donor's own resource load. Rewrite the
     guidance block; mount route becomes the declared-exception, not the rule.
:529 shims_skeleton.h emits dExt<Tag>_ adapters with no DN-10 banner and no
     strip trigger — pre-generating the §723 failure at scale. Emitter gains:
     DN-10 banner + KIT-PLUGIN field + strip trigger per generated adapter.

## Claims
- OUTPUT LAW landed + selftested 3 paths (refusal / declared+ledger / disc-verbatim live)
- audit line numbers verified against current files before each edit (not yet done — first step)
