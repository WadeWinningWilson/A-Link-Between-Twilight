# Build / FPS factory — live state

| Field | Value |
|-------|--------|
| **status** | **HOLD + checkpoint `94e53cbffc`:** FPS restore docs + `_factory_recover` backup-before-clean committed. Play exe snapshotted at `build/fps-hold-backup-20260823/`. Full dusk-api-coexist WIP remains **uncommitted** (seam gate blocks while dirty `editor.cpp` lacks map entry). No bisect/rebuild until user says go. |
| **owner_impl** | Build-analysis |
| **next** | On go: either adjudicate `editor.cpp` into ownership map then commit coexist WIP, or resume `F_SP121` code bisect with reversible toggles only. |
| **do_not** | Routine `cmake --preset` / `reconfigure_build.bat`; spawn endless git worktrees; blame WIP features while ninja has **zero** `/O2`; treat menu FPS as enough; leave `DUSK_DRIVE*` set |
| **healthy_bar** | Field typical **≥ ~220** (often ~250; Outset was ~260–280 pre-grass). Alarm: **~33 stable** (no `/O2`); **~100–120 stable** (soft factory / bad link); Outset **~100s with pops ~212** = content (grass-class), not factory |
| **updated** | 2026-08-23 — onboarding card for new instances |
| **detail** | [build-fps-guidelines.md](../build-fps-guidelines.md) (§ soft-poison + Maintaining optimization) · [ww-fps-bisect.md](ww-fps-bisect.md) |
| **oracle_144** | [drive-fps.md](drive-fps.md) — capped ~144 drive mean; **not** the RelWithDebInfo health bar |

---

## § Restore path (factory) — do this, in order

Pins already live in `CMakePresets.json` + `CMakeLists.txt` (refuse-empty `/O2`, linker `/INCREMENTAL:NO`). `build_run.bat` exits **98** if ninja lacks `/O2`, **99** if it would `Re-running CMake...`.

### 0) Diagnose (30 seconds)

```bat
findstr /C:"CMAKE_CXX_FLAGS_RELWITHDEBINFO" build\windows-msvc-relwithdebinfo\CMakeCache.txt
findstr /C:"/O2" build\windows-msvc-relwithdebinfo\build.ninja | find /C "/O2"
```

| Result | Meaning |
|--------|---------|
| Cache empty / ninja **0** `/O2` hits | **Hard factory poison** → §1 |
| Cache has `/O2`, ninja many `/O2`, field ~33 or ~100 stable | Suspect stale objs / `.ilk` → §1 full rebuild anyway |
| `/O2` OK, Ordon ~220+, Outset ~100s | **Not factory** → §2 content |
| `build_run.bat` exit **98** / **99** | Follow the bat message; do not ignore |

### 1) One-time factory recovery (main tree — preferred)

```bat
rem Backup current play exe first if you still have a known-good feel:
rem   mkdir build\fps-backup-TIMESTAMP & copy build\windows-msvc-relwithdebinfo\dusklight.exe build\fps-backup-TIMESTAMP\

cmd /c tools\_factory_recover.bat
```

That script: `ALLOW_CMAKE_RERUN=1` → `cmake --preset windows-msvc-relwithdebinfo` → verify `/O2` → `ninja -t clean dusklight` → delete `.ilk` → full `ninja -j8 dusklight`.

Then:

```bat
rem wipe GPU caches (required after rebuild)
del /q "%APPDATA%\TwilitRealm\Dusklight\dawn_cache.db*"
del /q "%APPDATA%\TwilitRealm\Dusklight\pipeline_cache.db*"
```

**Measure field** (`F_SP121` and/or Outset). Menu alone is not enough. Target typical ≥ ~220.

Manual equivalent if the bat is missing:

```bat
call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat"
set ALLOW_CMAKE_RERUN=1
cmake --preset windows-msvc-relwithdebinfo
rem confirm /O2 in build.ninja, then:
ninja -C build\windows-msvc-relwithdebinfo -t clean dusklight
del /q build\windows-msvc-relwithdebinfo\dusklight.ilk 2>nul
ninja -j8 -C build\windows-msvc-relwithdebinfo dusklight
```

### 2) Content / play-scene (factory already healthy)

| Check | Action |
|-------|--------|
| `video.maxFrameRate` too low (e.g. **30**) with interp **Capped** | Overlay cannot show healthy ~220+; set **240** (or uncapped / Off). Common false “120–150 poison.” |
| WW grass / `kusax*` → `NPC_EXTVEG` / `NPC_WWGRASS` | Per-clump / veg draw → Outset ~100s; unmap until packet-centralized ([Outset README](../WW%20Linked/islands/Outset/README.md) №128) |
| ExtNpc mounts / BG `changeFar(1e6)` | Scale with population; A/B WW folder off |
| Ordon OK, Outset bad | Content, not CMake |

Do **not** open a new worktree as the fix. A/B on **main** with folder rename / actor_map unmap.

### 3) Routine builds after recovery

```bat
cmd /c build_run.bat
```

Never routine `cmake --preset`. If exit 99: either recover (§1) or sync carefully — do not “just ALLOW” without field A/B after.

---

## Snapshot (this machine, 2026-08-23)

| Check | Last look |
|-------|-----------|
| Branch | `integrate/dusk-api-coexist` (varies) |
| `CMAKE_CXX_FLAGS_RELWITHDEBINFO` | Was `/O2 /Ob1 /DNDEBUG` |
| Ninja `/O2` hits | Was **2461** (healthy graph) |
| Linker RelWithDebInfo | `/debug /INCREMENTAL:NO` |
| Exe | `build\windows-msvc-relwithdebinfo\dusklight.exe` (~39 MB class; grows with WIP) |

If flags look healthy but the user still reports poison → skip straight to **§0 Ordon vs Outset** and **§2**; do not re-run factory forever.

---

## Related backups / tools

| Path | Role |
|------|------|
| `tools\_factory_recover.bat` | Canonical one-shot factory restore |
| `tools\_fps_ab3.bat` | Ordon / WW-off / CLEANr A/B helper (optional) |
| `build\fps-good-backup-*` / `fps-dirty-backup-*` | Local exe backups if present — do not treat as forever-good |
| `.cursor\rules\build-fps-safe.mdc` | Always-on agent bans |
