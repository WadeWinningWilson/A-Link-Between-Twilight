# WW land/terrain — authoring-technique reference (NON-ACTIONABLE)

**Read this first:** this is a reference stub, NOT a port task. Source: env-artist video
(youtube 9zA2rJmn3Ws, 2026-07-25) reverse-engineering HOW Nintendo AUTHORED WW's terrain. **We PORT
the donor's finished ripped BDLs/textures — we do not re-author them — so authoring technique is
CONTEXT, not work.** Kept only in case terrain RENDERING is ever debugged and understanding the
donor's material economy helps. [recreation] tier (artist inference, not decomp/measurement).

## Observations (context only)
- **Trim sheets + tiling everywhere.** Cliffs/walls/wood = long horizontal-tiling strips off shared
  sheets; ledges get a top grass/rock strip. Islands run ~7–12 materials TOTAL (confirmed when the
  artist opened ripped files: ~7 materials for the starting island).
- **Decals** for grass patches / grunge (slightly z-offset planes to avoid z-fighting).
- **UV mirroring** for symmetric details (rugs, borders) — quarter authored, unfolded/flipped.
- **Contact-point blending** — a consistent grass color + grass decals at seams marry elements.

## Why it's here and not actionable
Our Outset terrain mounts through the donor BDL (`Outset.arc`/room models) and renders via J3D — the
tiling/trim/decal authoring is already baked into those assets. If a terrain material ever renders
wrong, the fix is J3D material fidelity (like the shore-crash §126–§128 arc), NOT re-authoring UVs.
The ONE actionable extract from that video — the shoreline UV-scroll — lives in
[Water effects/shore-crashing.md](Water%20effects/shore-crashing.md), not here.
