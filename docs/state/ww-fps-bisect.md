# WW / WIP field FPS bisect (2026-07-19)

**Parent:** [build-fps-guidelines.md](../build-fps-guidelines.md) · [cut-actors-demo-restore.md](cut-actors-demo-restore.md)

Nothing in this bisect is irreversible. Each step is a marked toggle or a mod-folder rename. Restore by reverting the marked hunk or renaming the folder back. **Never commit** `WW-Crew-Restoration` arcs / Nintendo bytes.

---

## Baseline (locked)

| Exe | Menu | Field `F_SP121` |
|-----|------|-----------------|
| **Clean** `dusklight-fps-investigate\build\windows-msvc-relwithdebinfo\dusklight.exe` @ HEAD `5e9669348f` | ~228 | ~200–228 |
| **Dirty** main `build\windows-msvc-relwithdebinfo\dusklight.exe` (full WIP) | 130–150 unstable | **~33 stable** |

Same AppData/mods for both. Clean proves machine + mods can do ~200+. Dirty WIP delta is the regressor.

**Scope note:** Culprit may be **any uncommitted WIP** (ExtNpc/doors/population, henna0/kdk socket wiring, alink leaf weld/diags, cut-actor dyn list, custom_assets, warp UI, …) — not only Outset content. Outset mod folder stays AppData-only by design.

---

## Recipe (step by step)

After each rebuild: wipe `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` and `pipeline_cache.db*`. Measure **menu** + **field `F_SP121`** (FPS overlay on). Do **not** require Outset for these steps.

### A1 — play-scene ExtNpc polls only — **RULED OUT**

Disabled `pollBgWarps` / `Doors_poll` / `onStageReady` in `d_s_play.cpp`.  
**Result (2026-07-19):** field still **~33–35**. Polls restored. Not the tax.

### A2 — henna0 / kdk StubWatch — **RULED OUT**

HEAD refuse-only on henna0/kdk. **Result (2026-07-19):** field still **~33**.  
WIP restored from `docs/state/fps-bisect-restore/*.wip`.

### A3 — disable WW mod folder — **RULED OUT**

Renamed to `WW-Crew-Restoration.DISABLED`. **Result (2026-07-19):** field still **~33**.  
Tax is **other uncommitted WIP**, not the Outset mod folder. Folder left `.DISABLED` during B-steps (rename back anytime).

---

## B-series (non-Outset dirty code)

### B1 — alink leaf / crawl → HEAD — **RULED OUT**

**Result (2026-07-19):** field still ~33. Alink WIP restored from `.wip`.

### B2 — ExtNpc/O2 → HEAD — **RULED OUT**

**Result (2026-07-19):** field still ~33 with ExtNpc at HEAD + WW mod off + alink still WIP.  
**Note:** B2 rebuild **re-ran CMake** on main (FetchContent noise) — same class as June ~32 broken-env incident.  
WIP fully restored from `fps-bisect-restore/*.wip`; WW folder renamed back to active.

### B0 — dirty sources in investigate worktree — **WINNER**

**Result (2026-07-19):** investigate dirty exe — field **~190–250** (combat), Outset **~220–280**.  
Main dirty had been stuck ~33 after CMake re-runs during A/B (June ~32 broken-env class), **not** WW/ExtNpc/alink WIP.

**Fix applied:** copied investigate `dusklight.exe` (+ pdb) → main  
`build/windows-msvc-relwithdebinfo/`. GPU caches wiped.

**Going forward (agent builds):** Canonical rules live in
[build-fps-guidelines.md § CMake re-run / ~33 FPS](../build-fps-guidelines.md#addendum-cmake-re-run--33-fps-2026-07-19):

- **`build_run.bat` only** — never routine `cmake --preset` / `reconfigure_build.bat`.
- Ninja **`Re-running CMake...`** ⇒ FPS untrusted; recover via investigate worktree + copy exe.
- Do **not** start another feature FPS bisect when field is ~33 after a reconfigure.
- **Guard (2026-07-19):** `build_run.bat` exits **99** if a dry-run would reconfigure (override `ALLOW_CMAKE_RERUN=1` only on purpose). Cursor rule `.cursor/rules/build-fps-safe.mdc`.

### Post-№36/37 note (2026-07-19)

User measured field **~120** with main exe **byte-identical** to investigate `build_fps_probe` output (not the ~33 poison class). Treat as: recover path worked; remaining gap vs ~200 is either real dirty-WIP tax or measure conditions — A/B with `WW-Crew-Restoration` renamed off before blaming №36/37.

### A1b — polls off on *healthy* binary (2026-07-19) — **RULED OUT**

Menu **~288** / field still **~100–115** with polls off. Exe is healthy (menu proves it); tax is play-scene only.  
Polls restored in `d_s_play.cpp`.

### A3b — WW folder off on *healthy* binary (2026-07-19) — **RULED OUT**

Field still ~100–115 with only `WW-Crew-Restoration` disabled. Folder restored.

### A3c — ALL `model_replacements` off — **SKIPPED / not the delta**

User: overlays were **always on** during good (~200) and bad (~110) measures — not the regressor. Folder restored (brief park reverted).

### User note (2026-07-19) — delta = №36+№37 session

Performance was **fine until №36+№37 work** (same session also hit main CMake re-run). Menu ~288 / in-game ~100–115 is new. Overlays constant across good/bad → not the delta.  
Read-only scan: **no cmake files under `model_replacements`** (left untouched).

### B1b — alink → HEAD on *healthy* binary (2026-07-19) — **RULED OUT**

Still ~100–115. Alink dirty restored in investigate.

### N36r — №36/37 ExtNpc → pre-packet (2026-07-19) — **RULED OUT**

Still ~100–115. ExtNpc №36/37 files alone are not the mid-tier tax.

### B0r — full `fps-bisect-restore` WIP replay (2026-07-19) — **RULED OUT**

Still ~100–115 on the B0 source snapshot that previously hit ~190–250. **Not that WIP code.**

### CLEANr — pristine HEAD `5e9669348f` exe (2026-07-19) — **WINNER**

User: field back to **~190–250**. Same AppData as the ~100–115 runs.

**Root class (forensics, same day):** not №36/37 source (N36r/B0r), not overlays, not polls, not alink.

| Finding | Evidence |
|---------|----------|
| **Hard poison = empty RelWithDebInfo flag cache on main** | `CMakeCache.txt`: `CMAKE_CXX_FLAGS_RELWITHDEBINFO` / linker RelWithDebInfo flags are **empty**. `build.ninja`: **0 / 2135** `FLAGS` lines have `/O2`. Healthy trees have `/O2 /Ob1 /DNDEBUG` on essentially all CXX lines. |
| **Main play exe ≠ main factory** | Good ~08:27 exe (~32.5 MB, `UNKNOWN-VERSION` + `5e96693`) matches CLEANr copy timeline; main objs rebuilt ~07:45 under the no-`/O2` graph. |
| **Investigate soft-poison ≠ missing `/O2` in ninja** | Investigate ninja has `/O2`; objs ≈ clean (~1.00×). Still ~100–115 after B0r. Large **`dusklight.ilk` (~189 MB)** + long incremental-link reuse through reconfigure eras is the leading link-image suspect. |
| **Trigger** | CMake re-run (B2 logged it; `CMAKE_CONFIGURE_DEPENDS` on git HEAD also forces ninja reconfigure) regenerates ninja from whatever is in the cache — empty flags stick forever until rewritten. |

**Recovery applied + confirmed (2026-07-19):** pins + one reconfigure + full rebuild on main. Field **~250 stable** (dips ~220). Factory healthy — no more worktree sprawl for routine builds. Details: [build-fps-guidelines.md § soft-poison root cause](../build-fps-guidelines.md#soft-poison-root-cause-2026-07-19-forensics--empty-relwithdebinfo-flags-in-cmakecache).

**Going forward:**
- **Main** + `build_run.bat` only (exit 98 if no `/O2`, exit 99 if would Re-run CMake).
- Investigate / clean worktrees are optional leftovers — not the build process.
- Project-root CMake files are normal — do not delete.
- **Maintain optimization:** [build-fps-guidelines.md § Maintaining optimization](../build-fps-guidelines.md#maintaining-optimization-2026-07-19-evening--factory--play-scene) — factory pins + ExtNpc hot-path rules. A/B 2026-07-19 eve: dirty Ordon 219–265; Outset WW-off 228–270; CLEANr field 202–242 → ~40–50 swing inside 200–270 is normal, not poison.

---

## Permanence rules

- Bisect toggles are **temporary** until a step wins; then replace with a proper gate (not a permanent `#if 0`).
- Document the winning step + final fix in this file and live state.
- Outset arcs / census CSVs stay out of git forever (`do-not` in live state).
- **CMake:** follow [build-fps-guidelines.md](../build-fps-guidelines.md) — pin RelWithDebInfo flags; incremental `build_run.bat` only; one documented recovery if cache/ninja lose `/O2`.
