# P1 — export-surface delta: d_ext_ww_actor_shims vs dusklight_exports.def

**Lane: Housing + Engine · 2026-08-11 · read-only run against the built exe
(build/windows-msvc-relwithdebinfo/dusklight_exports.def, 31,645-export surface).**

Method: every callable name declared in `include/d/d_ext_ww_actor_shims.h`
(64 unique — broader than §701's "21 unique" because this parse includes member
functions and WW-layer cross-imports, stated so the counts reconcile rather
than conflict), substring-matched against the .def (catches mangled C++ forms).
№31-C: substring match is INDICATIVE — a hit proves presence, a miss needs
per-symbol confirmation before it is declared a gap (inline/member functions
need no export).

**ON the export surface: 51 / 64.**

**MISSES (13), classified:**

| name | class | verdict |
|---|---|---|
| dComIfGp_evmng_cancelStartDemo | free fn, receiver | REAL export gap candidate |
| dComIfGp_getShipActor | free fn, receiver | REAL export gap candidate |
| dComIfGp_particle_setSimpleLand | free fn, receiver | REAL export gap candidate |
| dComIfGp_setMesgAnimeTagInfo | free fn, receiver | REAL export gap candidate |
| dComIfGs_getpCollect | free fn, receiver | REAL export gap candidate |
| dExtLs1_getScopeWipeFlag | **WW-layer symbol** | NOT an export gap — an (e) INTER-TU import; goes to the wave table as a coupling edge |
| kb_dig | shim-local? | confirm linkage before classifying |
| checkCollect, getBodyMtx, getJointMtx, setRate, onWindOff, offWindOff | member/likely-inline | probably no export needed — confirm per symbol |

**Handoff:** the 5 REAL-candidate free functions are the first concrete input to
P1's export-surface work (they would need exporting — or SDK service coverage
per P2 — before any Wave that carries their callers). The (e) edge corrects one
row of the §702 table. Integrator: this file is a foundry-1 verification input.
