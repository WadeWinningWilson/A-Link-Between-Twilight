#pragma once

// ============================================================================
// WW ACTOR REGISTRY — the above-enum profile vehicle, merged into this plugin.
//
// WHY IT LIVES HERE NOW (Integrator assignment, CALLS row 570): it shipped as
// a separate `ww_registry.dusk` while three plugins each answered a different
// question — does donor data serve, do by-name hooks fire, does above-enum
// dispatch route. All three are answered, so the split has outlived its
// purpose and the user asked for ONE plugin rather than a family.
//
// The registry keeps its own translation unit and its own service imports.
// NOTE — this used to add "(`IMPORT_SERVICE` emits a *static* pointer, so each
// TU holds its own handles and its own modmeta import record)". That stopped
// being true at the Game ABI epoch-2 bump: `service.hpp:49` dropped `static`,
// so the pointer now has EXTERNAL linkage and a service may be imported in
// only one file. See the epoch-2 amendment at the top of `registry.cpp` for
// the discriminator and both arms. Only the entry points changed:
// what were `mod_initialize` / `mod_shutdown` are now these two functions,
// called by the merged plugin's single pair.
//
// CONSEQUENCE OF THE MERGE, stated rather than absorbed: this plugin now
// requires `FEATURES game` because the registry hooks do. That gives the disc
// reader a BUILD-ORDER dependency on the exe's import library — the coupling
// `ww_donor_disc` was originally written to avoid. It is build-time only:
// `dumpbin /imports` on the same shape showed no import from dusklight.exe
// (H10, run 180329), so the emitted plugin stays version-independent.
// ============================================================================

#include <mods/api.h>

ModResult wwRegistry_initialize();
ModResult wwRegistry_shutdown();

// ============================================================================
// WARP ROUTES — the destination tree, DERIVED FROM THE USER'S OWN DISC.
//
// WHY THIS CROSSES THE TU BOUNDARY AT ALL. The menu lives in registry.cpp; the
// disc lives in main.cpp. Hardcoding the tree in the menu was the b2-era
// shortcut and it has a live defect: `sea` was pinned to room 44, so the other
// ~49 rooms the plugin already serves were unreachable from the UI. The disc
// knows which rooms exist — it is enumerated at mount time anyway to build the
// arc-name aliases — so the menu should read what was served, not a list
// someone typed.
//
// CONTRACT:
//   · `wwRegistry_clearRoutes()` FIRST on every (re)build — a config change
//     re-runs the serve, and stale routes would offer stages no longer served.
//   · `wwRegistry_addRoute()` once per (stage, room) actually mounted.
//   · Storage is STATIC and owned by the registry. The UI's SELECT holds
//     `const char* const*` across frames, so a caller's buffer would dangle —
//     the same lifetime rule that governs the object-name table.
//   · Saturation is LOGGED, never silent. A truncated destination list that
//     says nothing is the "0 means not measured" failure in another costume.
// ============================================================================
void wwRegistry_clearRoutes();
void wwRegistry_addRoute(const char* stage, int room);


// Toggle the file token alongside the canon name, mirroring dusklight's own
// two-name model (`MapEntry { mapName; mapFile; }`). Relabels in place.
// CAVEAT carried here because it is the live trap: `3295 + 0` resolves to
// the UI sentence "You have no maps.", NOT an island, so the island block
// starts at room 1 and room 0 must never be auto-named from the formula.
void wwRegistry_setShowFileNames(bool on);

// Diagnostics master switch (default OFF). When off, the HOT-PATH probes are
// never ATTACHED - not merely silenced - so their trampolines cost nothing.
// Must be set BEFORE wwRegistry_initialize().
void wwRegistry_setDiagProbes(bool on);

// Upgrade a route's menu label with an established display name. The donor
// token is NOT replaced — the label becomes `Name  [stage room]`, so "where am
// I going" and "which file is that" are both answerable, which is what the
// user asked for. No-op if that (stage, room) never mounted; a miss is normal
// (names are supplied for all 49 sea rooms whether or not each one is served).
//
// PROVENANCE OF THE NAMES, since this is the one surface where a wrong string
// is invisible: island names are read from the DONOR'S OWN table at
// `messageID = 3295 + room` (anchored on room 44 = "Outset Island"); interior
// names are AUTHORED on the user's explicit ruling, against actor identities
// taken from the donor's source rather than from recall.
void wwRegistry_setRouteName(const char* stage, int room, const char* name);

// ---- GROUPING PASS (user assignment 2026-08-16) ----------------------------
// Donor-derived sort inputs. `type` is `dStageType_*` (STAG +0x0C,
// `(mStageTypeAndSchbit >> 16) & 7`); `parentSeaRoom` is the island whose
// `SCLS` exit table names this stage. Both come off the disc during serve, at
// no extra I/O - nothing here is a typed table.
void wwRegistry_setStageType(const char* stage, int type);
void wwRegistry_setStageParent(const char* stage, int parentSeaRoom);

// ---- SLOT-1 OWNED MOUNT (ownership-contract fix) ---------------------------
// main.cpp hands over its session-retained raw bmgres.arc bytes; the vanilla
// msg-group seam mem-mounts them per WW scene via the receiver's exported
// JKRArchive::mount(void*, JKRHeap*, dir) factory (break-flag 0: the receiver
// never frees this buffer; removeResourceAll is a MOUNT_MEM no-op).
void wwRegistry_setBmgArcBytes(const void* buf, unsigned int size);

// Sort the menu into display order and relabel: islands ascending by donor
// room number, each immediately followed by its own interiors (indented), then
// stages no island claims, bucketed by stage type. CALL ONCE after every route
// is added and named - it permutes the route arrays in place.
//
// WHY IT MATTERS: a flat 81-row list cost a diagnostic run - the user could
// not find Outset, warped elsewhere, and the probe never reached `R44_00`.
void wwRegistry_finalizeRoutes();

// True while the receiver start-stage name is a served WW stage.
bool wwRegistry_isWwHostStage();
