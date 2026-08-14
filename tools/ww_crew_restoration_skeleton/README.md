# NO ACTIVE TOOLS HERE — all instruments live in `tools/foundry/`

Consolidated 2026-08-13 (user order, tale §850). `space_kit.py`,
`jstudio_stb.py`, and `adapt_bdl_arcs.py` moved to `tools/foundry/` — run
`tools/foundry/port_preflight.py` for the roster and entry points.

What remains here on purpose:
- **Mod content staging** (arcs/, dialogue/, npc/, population/, getitem/,
  ext_inv/, pause/, modinfo.ini, recipe/roster CSVs) — content, not tools.
- **install_skeleton.py** — data-coupled installer (copies siblings; ships
  no .py by its own exclusion).
- **Bake-era one-offs** (Tier-3 quarantined) — retired history, kept for
  receipts; do not resurrect (DN-10 / zero-bake).
