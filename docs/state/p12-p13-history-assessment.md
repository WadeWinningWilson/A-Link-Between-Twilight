# P12 / P13 — History assessment + next-turn decision (§215)

Foundry settled the five-domain parity program (P9–P13). This is History's assessment of the two
newest domains and the resulting next-turn call.

## P12 — Path parity — the [INFERENCE-NEEDED] gate is RESOLVED

**Question (Foundry → History/Bridge):** does STG authoring convert donor `RPAT`/`RPPN` into receiver
`PATH`/`PPNT`?

**Verdict: NO — the donor paths are DROPPED.** Confirmed three ways:
- **Donor Outset is path-rich** — `fact-sheets/sea.md`: Room44 `RPAT`×41 / `RPPN`×446, and every sea
  room carries paths (1,051 sea paths in `donor-path-summary.md`).
- **The receiver HAS a path system** — `d_path.cpp`, `d_spline_path.cpp`, `PATH`/`PPNT` load in
  `d_stage.cpp` — but it is fed by the **host TP stage**, not the donor WW DZR.
- **The WW-restoration pipeline carries no path data** — the mod folder has only actor placements
  (`actor_map.ini`/CSVs); `d_ext_npc_mount.cpp` injects **no** `RPAT`/`RPPN`/`PATH`. So a WW actor that
  follows a donor path in the donor cannot follow it in the port.

**Low current-Outset impact (important):** the visible Outset roster mostly does NOT need paths — the
`Kamome` seagulls use their own flight AI (no `RPAT`), villagers are stationary. The `RPAT`/`RPPN`
consumers are elsewhere (sail routes, patrols, sea traffic). So path-parity is not blocking the current
Outset visual work; it becomes load-bearing when a path-following WW actor is restored.

**Restoration approaches (scoped, deferred):**
- **(A) Bridge STG-conversion** — convert donor `RPAT`/`RPPN` → `PATH`/`PPNT` at authoring time and
  inject into the stage path data the receiver already reads. Cleanest; Bridge-owned.
- **(B) Mod-folder path file + loader** — a `paths.*` data file (donor waypoints, like `actor_map` for
  actors) + a `dExtNpc` loader that feeds the receiver `dPath`. History/Engine-owned; matches the
  existing data-driven pattern.
- **Open sub-question for either:** *which* WW actors actually consume `RPAT` (per-actor `getRailInfo`
  usage) — that decides the priority and which paths to carry first. A decomp pass over the sea actors.

**P12 tap (for the eventual verdict):** one `DuskLog` line at the receiver `dPath` follow-init, same
pattern as the emitter/place taps — arm it once paths are actually carried (nothing to compare until
then).

## P13 — Combat-AI state parity — pipeline READY

`state_map.py` + the worked example `donor-statemap-bk.md` (Bokoblin: master dispatcher `d_a_bk.cpp:3384`,
24 action states, 24 mode sub-machines, 232 line-cited transition writes) give History a **turnkey
enemy-restoration pipeline**: run the extractor on the donor source → implement from the artifact →
verify with the tap pair. The Bokoblin artifact doubles as the **A_mori restoration spec** (A_mori's
"founding quarry" cast = `mo2`/`Bk`/`gmos`). No action needed until the first enemy family is restored.

## Next-turn decision (History)

**Decided + DONE this turn: resolved P12's open sub-question — which WW actors consume `RPAT` paths.**
Grepped the whole Outset donor cast for `getRailInfo`/`dPath`/route use. **Result: exactly ONE consumer
— `d_a_obj_ikada.cpp` (the ship).** Villagers are stationary; `Kamome` seagulls use flight AI (no RPAT);
crabs/pigs/etc. use local AI. The ship follows an **RPAT sail route**: `mpPath =
dPath_GetRoomPath(mPathId, roomNo)` (line 1301) → `dLib_pathMove(&mPathPosTarget, &mCurPathPoint, mpPath,
…)` (line 298) → `cLib_addCalcPosXZ2` moves + steers toward the waypoint (299-301). `mPathId` is in the
params (bits ≥4).

**This unifies P12 and L-5.** The Outset ship isn't a moored raft — it **sails a route** AND bobs. So the
faithful `daObj_Ikada` port (L-5) is *also* the Outset P12 case: it needs (a) its donor RPAT path carried
into the port (P12 restoration approach A or B, scoped above), plus (b) `dPath_GetRoomPath`/`dLib_pathMove`
follow, (c) the wave-bob, (d) flag/rope. **Conclusion: there is no separate "Outset path parity" task —
it lives inside the `daObj_Ikada` port**, which is therefore the single highest-value Outset behavior
port (it closes L-5 *and* the only Outset P12 case at once). The static `vtsp.bdl` placement is the
correct-model stopgap until then.

**Queued as the next MAJOR effort (needs user go — it's a pivot + a large port):** the **first enemy
family via P13 — the Bokoblin** (A_mori). This is what the whole parity program was built to enable; it
is a multi-session actor port, so it starts only on the user's word, not mid-Outset-test.

**Also still open on the Outset thread:** white-flower color (L-6), the `dTree` trees (L-6 deferred),
`daObj_Ikada` full port (L-5 waves/flag), `NpcSo`.
