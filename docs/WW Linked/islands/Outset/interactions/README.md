# TP↔WW actor interaction — recipes

**Why this folder exists:** the grass-cut work (ledger №220–№229) was the FIRST time a WW-restored
actor and TP Link physically interacted — sword reaches grass, grass reacts. Every later
interaction, **combat above all** (TP Link's attacks vs WW enemies, WW enemy attacks vs TP Link),
rides the same engine plumbing. Grass is therefore not a one-off; it is the **reference recipe**.
This folder holds the proven patterns so no future interaction re-derives them from a debug spiral.

| recipe | status | covers |
|---|---|---|
| [mass-interaction-recipe.md](mass-interaction-recipe.md) | ✅ proven (grass) | point-query "did an attack reach here" — cuts, hits, triggers |
| combat (cc AT/TG) | 🔜 not yet written | actor-vs-actor damage with types/materials — port when the first WW enemy takes/deals a hit |

**The one-line distinction that governs everything here:**
- **Mass channel** = cheap point queries against a small per-frame shape list. "Is an attack shape
  at this point?" No damage routing. Grass, pots, dig-spots, cuttable props.
- **cc AT/TG channel** = full actor-vs-actor damage — attack types, material tables, hit callbacks,
  HP. The Hero's-Shade / Armogohma / wolf-arts work already exercises this on the RECEIVER side;
  porting a WW enemy means feeding WW's AT/TG data through it. Its recipe gets written the first
  time a WW enemy lands, and it will lean on the same "receiver-frame-contract" lesson below.
