# NPC presence — the three-axis system

**Presence** = the full answer to "is this actor here, where, and can you tell?" — three axes,
each driven by WW's own data/systems, never by hand-placed positions or ad-hoc visibility hacks.

| axis | question | WW mechanism |
|---|---|---|
| **1. Existence** | does the actor exist at all right now? | room actor LAYERS + the story-flag ladder |
| **2. Position / state** | where does it stand, in which state? | per-actor **params** placement variants |
| **3. Presentation** | is it drawn / collidable / targetable yet? | actor-state gating (present-but-hidden until its scene readies) |

Recipe: [npc-presence-recipe.md](npc-presence-recipe.md) — evidence, the receiver mirror, and the
implementation contract.

**Why this folder exists (user, 2026-07-22):** placement fixes were heading toward one-off tweaks
(move Grandma upstairs by hand). The end goal is the opposite — mirror WW's own systems so story
progression moves and reveals EVERY NPC from data alone, and no future change ever needs manual
adjustment. Sibling of [../interactions/](../interactions/README.md) (the TP↔WW interaction
recipes); together they cover "being there" and "being touchable."
