# USER RULINGS 2026-08-12 — plugin source home + mod-folder rehome

## Ruling 1 (Reading A, precise structure)
Plugin source lives in the MOD FOLDER, contained in its own subfolder:

    WW-Crew-Restoration\plugin\   ← source files + eventually the shipped binary

This DELIBERATELY AMENDS the content-only rule: content-only everywhere EXCEPT
the dedicated `plugin\` subfolder. Containment is the point — the old hazard
was tooling LOOSE beside assets, not tooling existing.

## Ruling 2 (the deeper one, user-identified)
The WW-Crew-Restoration folder itself leaves Dusklight-owned locations
entirely. New home: `%USERPROFILE%\Documents\A LBT WW`. Timing delegated to
Foundry.

## Foundry determination: ONE move, at the rehome — LATER
The source is 5 files (donor_disc.cpp/.h, main.cpp, mod.json, CMakeLists) plus
a 3-line CMake seam (dusklight CMakeLists.txt:753-756, EXISTS-guarded
`add_subdirectory`). Moving source into AppData now would be an intermediate
hop — the folder itself moves to A LBT WW later. One coordinated move beats
two. At the rehome slot:

1. mod folder → `%USERPROFILE%\Documents\A LBT WW\WW-Crew-Restoration\`
2. `mods-src\ww_donor_disc\*` → `<newhome>\WW-Crew-Restoration\plugin\`
3. CMake gains a `WW_PLUGIN_SOURCE_DIR` cache var pointing there (survives any
   future move; EXISTS-guard kept)
4. the runtime mod-root gains a config path (generic naming, the row-14
   pattern) so the engine loads content from the new home

Engine executes 3–4 (build + loader wiring, their domain); the moves are
mechanical. **Until then: `mods-src\` in-tree is STAGING WITH THIS NAMED
EXIT** — the default became a decision, which is what the ruling asked for.

Trigger: post-wave, or bundled with Engine's next plugin-build touch —
whichever comes first with the user's go.
