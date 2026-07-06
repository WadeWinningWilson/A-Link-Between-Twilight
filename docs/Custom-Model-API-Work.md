# Custom Model API — Work Log & Design Record

**Goal:** let players install custom-asset mods (models, textures, audio, whole
arcs) by **dropping files into a folder and booting** — no `.arc` repacking, no
ISO rebuilding. Extend that into a **general, repack-free override system** that
can host any current arc-based Dusklight mod.

This doc records everything built so far, how it fits together, and — importantly
— how the **Aurora virtual-FST overlay system** (discovered mid-build) can
streamline the earlier work and align us with the official Dusklight team's
foundation.

---

## 1. The mod folder layout

Everything lives under the config dir:

```
%AppData%/TwilitRealm/Dusklight/model_replacements/
├── Armogohma Custom/            ← "single loose asset" folder
│   └── B_gm_37.bmd
└── Linkle Mod NSTC/             ← "full-mod data tree" folder
    └── files/                   ← mirrors the disc layout 1:1
        ├── Audiores/
        │   ├── Z2Sound.baa
        │   └── Waves/*.aw        (232 wave archives)
        └── res/… (35 arcs)
```

Two shapes are supported:

- **Single loose asset** — a bare `<arc>_<index>.bmd` sitting in the folder
  (or root). Loaded directly, bypassing the arc. (Phase 1.)
- **Full-mod data tree** — a `files/` subtree that mirrors the disc. Every file
  under it can transparently replace its disc counterpart. (Phase 2.)

Each immediate subfolder of `model_replacements/` is one **toggleable custom
model** in the editor list.

---

## 2. What's built — phase by phase

> **History note.** This started as four incremental phases (1: loose-BMD;
> 2a: data-tree scan; 2b: custom arc mount; 2c: Aurora overlay). After the
> overlay proved a superset, the code was **consolidated into two layers in one
> module** (`dusk::custom_assets`) and the standalone arc-mount (old 2b) was
> **retired**. What follows describes the consolidated design; the phase labels
> survive only as a map to the git history.

Everything now lives in one module — **`dusk::custom_assets`**
(`src/dusk/custom_assets.cpp`, `include/dusk/custom_assets.hpp`) — with a shared
scan + toggle and two override layers.

### Layer A — whole-file DVD overlay  (was Phase 2a + 2c; absorbs 2b)
- `scan()` walks each **enabled** `model_replacements/<mod>/files/` tree and
  builds a normalized **game-path → absolute-loose-path** map (case/slash
  insensitive). Logs e.g. `'Linkle Mod NSTC': 268 override file(s)`.
- `install_overlays()` registers every mapped path as an **Aurora DVD overlay
  file** (see §3). Callbacks are plain `fopen/fread/fseek/fclose`; files open
  lazily on first read (no RAM cost for hundreds of entries). Overlay callbacks
  are installed exactly once; the file set is **re-registrable**.
- Called at startup from `src/m_Do/m_Do_main.cpp` right after `aurora_dvd_open`
  (both disc-open paths converge there) and after `scan()`.
- **Covers:** `Z2Sound.baa`, all `.aw` wave archives, BGM streams, **and arcs
  (raw OR compressed)** — one hook, no edits to the vendored JAudio2 library.
  Compression is handled by the stock loader because arcs mount through the
  normal entrynum path against the overlay (this is exactly what retiring the old
  custom mount unlocked).
- **Status:** audio confirmed coming through in-game; arc redirect working.

### Layer B — loose single-BMD injection  (was Phase 1)
- `try_load(arc_name, res_index)` looks for `"<arc>_<index>.bmd"` in the
  `model_replacements` root or any **enabled** immediate subfolder, reads it into
  a 32-byte-aligned buffer, and finishes it through the engine-standard
  `dRes_info_c::loaderBasicBmd('BMDV', …)`. Returns `J3DModelData*` or `nullptr`
  (caller falls back to the arc).
- Exists because a single model *inside* an arc is pulled by resource index, not
  disc path, so Layer A can't reach it at sub-arc granularity.
- Wired into Armogohma at `src/d/actor/d_a_b_gm.cpp` (`useHeapInit`, B_gm res
  0x25) with a hard arc fallback. Working; original (234912 B) and SuperBMD
  re-export (235648 B) both load.

### Retired — standalone arc mount (old Phase 2b)
- The old `mDoDvdThd`-level custom mount (`dusk_loadLooseArc` +
  `mOverridePath` + `find_override`) has been **removed**. Arcs now redirect via
  Layer A. This deleted the only `find_override` caller and unlocked
  compressed-arc support (the old mount was raw-RARC only). `m_Do_dvd_thread.cpp`
  is back to a stock mount path plus a one-line note.

### Custom Models settings tree  (`src/dusk/ui/editor.cpp`, ALBW tab)
- `game.customModelsDisabled` setting: `|`-delimited list of **disabled** folder
  names (empty = all enabled), persisted in config.
- `custom_assets::list_folders() / is_folder_enabled() / toggle_folder()` drive a
  multi-toggle picker (highlight = enabled, click again to disable — same feel as
  sword/shield equip). Left button summarizes `N/M enabled`.
- **One toggle gates both layers.** `toggle_folder()` updates the setting, then
  `scan()` + `install_overlays()` so the change lands on the **next asset load**
  (reload-scoped — see §4). Layer B checks the toggle at load time.
- **Status:** working.

---

## 3. The Aurora virtual-FST overlay system (the big lever)

Aurora's PC DVD backend (`extern/aurora/lib/dolphin/dvd/dvd.cpp` + `fst.cpp`,
public API in `extern/aurora/include/aurora/dvd.h`) already ships a **first-class
loose-file overlay mechanism** — and nothing in the game was using it.

**How it works:**
- `aurora_dvd_overlay_callbacks(cbs)` — you provide `open/close/read/seek`.
- `aurora_dvd_overlay_files(files, n, out)` — you provide a list of
  `{fileName ("/disc/relative/path"), userData, size}`. Aurora merges them into
  the disc's FST: a file that matches an existing disc entry **replaces** it
  (keeping its entryNum) and is flagged `isOverlay`; new files get a fresh
  Aurora-assigned entryNum.
- After that, `DVDConvertPathToEntrynum` → `DVDFastOpen` sees `isOverlay` and
  routes reads to **your callbacks** instead of the disc partition.

**Why it's a superset of our earlier work:** every audio/arc/stream loader in the
game bottoms out at that same chokepoint. Verified end-to-end:

| Asset            | Load chain (all → overlay-aware `DVDFastOpen`)                                             |
|------------------|--------------------------------------------------------------------------------------------|
| `Z2Sound.baa`    | `mDoDvdThd_toMainRam_c` → `JKRDvdToMainRam(entrynum)` → `DVDFastOpen`                       |
| `.aw` waves      | `JASWaveArc::setPath` → `DVDConvertPathToEntrynum` → `JKRDvdAramRipper::loadToAram` → `JKRDvdFile::open` → `DVDFastOpen` → `DVDReadPrio` |
| BGM streams      | `JASAramStream` → `DVDFastOpen` + `DVDReadPrio`                                             |
| Arcs             | `mDoDvdThd_mountArchive_c` → `my_DVDConvertPathToEntrynum` → (entrynum mount) `DVDFastOpen` |

Because it goes through the **normal** loader, the overlay path also inherits
things our hand-rolled code doesn't: YAZ0/YAY0 decompression, correct alignment,
the async DVD queue, and the game's own error handling.

---

## 4. The toggle model — reload-scoped now, instant-on-screen next (MANDATORY)

Modeled on the texture API (`texture_replacements::set_enabled` →
`unregister_replacements` + `reload()`). Textures flip **instantly** because they
resolve **per-draw** by GPU hash. Files are different: they're **read once at
load**, so the file layer's ceiling with a pure re-register is *reload-scoped*.

**Implemented (reload-scoped):** `toggle_folder()` updates the setting, then
`scan()` + `install_overlays()`. `aurora_dvd_overlay_files` clears and rebuilds
its set each call (the file-layer analog of unregister+register), so the disabled
folder's overlays are dropped and the change takes effect on the **next asset
load** (area transition / next BGM cue). This fixed the earlier regression where
toggling off did nothing until a full reboot. Safety: the FST rebuild is guarded
by `s_fstLock` (shared with `DVDConvertPathToEntrynum`/`DVDFastOpen`), and
disc-replacing files keep stable entrynums, so a mid-session rebuild is safe;
already-open file handles keep their own `FILE*`. The persistent path pool
(`s_pathPool`) means re-registration never dangles a previously-issued
`userData`.

**MANDATORY next phase — truly instant on-screen.** Reload-scoped is the interim
state, **not** the finish line. A toggle must visibly swap the asset without an
area reload. The file layer can't do that with re-register alone (already-loaded
data persists), so instant needs one (or both) of:
- **Callback-level disc fallback:** keep overlays always registered, but have the
  overlay `open`/`read` serve the *original disc bytes* when the folder is
  disabled (capture each replaced file's disc offset+length at install time). The
  entry stays `isOverlay`, so the switch is per-open with no FST rebuild — the
  cleanest analog to the texture per-draw model.
- **Asset-reload nudge:** after a toggle, force the affected assets to re-request
  (re-mount the arc / re-init the audio bank) so the already-loaded copy is
  dropped. The engine already does this on area transition; the work is doing it
  on demand for the specific assets a toggle touches.

Design leans toward the disc-fallback approach (no FST churn, matches how
textures behave). To be scoped and built next.

**Complementary system:** `texture_replacements` is Aurora's *GPU-hash-keyed*
pixel swap. Overlay (file-level) + texture replacements (GPU-level) together
cover raw disc files and in-memory textures.

---

## 5. Positioning for a merge with the official Dusklight team

- **Built on the official, upstream Aurora API**, used exactly as documented
  (`aurora_dvd_overlay_files` / `_callbacks`) — a purpose-built extension point.
  If the official team ships a mod loader it will almost certainly sit on this
  same API, so `install_overlays()` is a thin adapter that should merge trivially.
- **The mod layout (`<mod>/files/` mirroring the disc) is the natural "unpacked
  mod" format** for an overlay loader — a 1:1 loose-path→disc-path map.
- **One module, one namespace.** Everything is under `dusk::custom_assets`
  (`custom_assets.cpp/.hpp`) with no structural coupling, so an upstream rename is
  a single find-replace. The old non-standard arc-mount code is gone, shrinking
  the merge surface to just the Layer-B loose-BMD injector (which is small and
  self-contained).

---

## 6. Current file state

| File | Role | Status |
|------|------|--------|
| `src/dusk/custom_assets.cpp` / `.hpp` | Whole system: Layer A (scan + overlay), Layer B (loose BMD), folder toggle | Working; audio confirmed in-game |
| `src/m_Do/m_Do_main.cpp`               | Calls `scan()` (startup) + `install_overlays()` (after disc open) | Wired |
| `src/m_Do/m_Do_dvd_thread.cpp`         | Stock arc mount (custom 2b mount **removed**) | Reverted to stock + note |
| `src/m_Do/m_Do_main.cpp`               | Calls `scan()` + `install_overlays()` at startup | Wired |
| `src/dusk/ui/editor.cpp`               | ALBW → Custom Models multi-toggle picker | Working |
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.customModelsDisabled` setting | Working |
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.customModelsDisabled` setting | Working |
| `src/dusk/ui/editor.cpp`               | ALBW → Custom Models multi-toggle picker | Working |
| `src/d/actor/d_a_b_gm.cpp`             | Layer-B consumer (Armogohma B_gm) | Working |
| `extern/aurora/lib/dolphin/dvd/{dvd,fst}.cpp` | Aurora overlay system (upstream, unmodified) | Used as-is |

---

## 7. Open items / limitations

- **MANDATORY: truly-instant on-screen toggle** (see §4). Reload-scoped is the
  current interim; instant-without-reload is the required next build, leaning
  toward the callback-level disc-fallback design.
- **Single-asset granularity:** sub-arc assets still need Layer B (loose BMD) or
  a repacked arc; Layer A is whole-file only.
- **Audio consistency (modder rule):** a mod that overlays `.aw` waves must also
  overlay the matching `Z2Sound.baa` (its WSYS offsets are baked against those
  waves). The Linkle mod ships both, so this holds.
- **Verify a compressed arc** through Layer A (Linkle's are raw RARC). Architecture
  says it works (stock loader decompresses the overlaid bytes); wants one test.
