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

**The instant-on-screen work chose the asset-reload nudge** (not disc fallback):
once an arc/wave bank is in memory, changing the overlay does nothing to the
loaded copy, so the only way to reflect a toggle without an area reload is to make
the specific consumers **re-request** their assets. That's straightforward for
assets the game reloads on a clothes/area change, but hard for **session-resident**
assets that are deliberately never re-requested — which is the whole story of §5.

**Complementary system:** `texture_replacements` is Aurora's *GPU-hash-keyed*
pixel swap. Overlay (file-level) + texture replacements (GPU-level) together
cover raw disc files and in-memory textures.

---

## 5. Resident-asset live toggle — the sumo composite (Option 3b)

The hardest part of the toggle story. Most model arcs (`Kmdl` etc.) reload on a
clothes change, so they track a toggle for free. But some assets are held
**session-resident** by design and never re-requested mid-play — so toggling the
mod while they're on screen leaves them frozen. The ALBW **sumo outfit** is the
worst case: it's a *composite* of several resident pieces, so a mid-session toggle
produced "half-Linkle" hybrids (e.g. Linkle body + vanilla face).

### Diagnosis (the `[ca-diag]` coverage trace)
A temporary diagnostic (`D_ALBW_CA_DIAG` in `custom_assets.cpp`) logged every
overlay `OPEN` (served-from-loose) and every arc `CONVERT` (`overlaid=0/1`). It
proved this was **not** a coverage gap: `alSumou` *was* overlaid and served — it
just loads **once** and is held resident, while `Kmdl` (the face donor) reloads
and tracks. Frozen body vs. live face = the hybrid.

### What DOESN'T work: freeing the resident arc
First attempt: on toggle, `resDelete` the resident `alSumou` so it re-mounts.
**Crashed** — `EXCEPTION_ACCESS_VIOLATION` at `0xffff…`, the documented
dangling-solid-heap double-free (the clothes pipeline's `mpArcHeap->freeAll()`
bypasses the refcount, so a hand `resDelete` on an aliased arc dangles). Recorded
in git `1e5804185f`. **Rule: a resident model arc cannot be hot-freed via the
global resource manager while a model references it.**

### What works: private-heap mounts + build-then-swap (Option 3b)
Adopt the proven **independent-cap pattern** — mount into a *private*
`dRes_info_c` array + `JKRSolidHeap` that the clothes pipeline never indexes or
frees. Because the private heap never aliases `mpArcHeap->freeAll`, it can be
freed safely. A subtlety forces the shape: the resource manager is **name-cached**
(re-`resLoad("alSumou")` returns the *old* content), so you cannot hold two
versions of one arc name in the global manager — hence a **private** mount, and a
**two-slot build-then-swap** to refresh it:
- Load the fresh overlay content into the free slot while the old slot keeps the
  live model valid; repoint `changeLink` to the new slot; release the old slot on
  the *next* swap (so the slot freed is always one a rebuild already moved off).

### The composite: every piece must pull from a toggle-tracking source
`changeLink`'s sumo build reads from several arcs; each had to be moved onto a
private, generation-swapped mount or it stayed frozen:

| Sumo piece | Was | Now |
|---|---|---|
| Body / hand / topknot (`alSumou` 0x31/0x32/0x33) | global resident | private `alSumou` 2-slot build-then-swap |
| Face (`al_face`) | base clothes arc `mArcName` (skip-path frozen) | private `Kmdl` (also base-independent) |
| Color cap G/R/B (`al_head`/`ml_head`/`zl_head`) | global donor `getObjectRes` | private independent cap loader (gen-swap) |

### One coordinator, one rebuild
The body and cap originally had **separate** generation-swaps that each fired
`forceReapply` — so the cap's rebuild ran before the body's build-then-swap
finished, and the topknot (sourced from the body slot) rebuilt from the *stale*
slot. Fixed by a single coordinator (`driveSumoPrivateBody`, called from
`resourcesReady`): on a generation change it starts the body swap **and** frees
the private head arcs, epoch-bumps so the stale head/face isn't drawn, and fires
**exactly one** `forceReapply` — only once the new body slot **and** the head arcs
are all resident (`headArcsReadyForRebuild`). The rebuild uses the shared
metamorphose-reapply hook (`dAlbwOutfit_forceReapply`).

### Status & risk
- **Crash-free** through swaps, storage, warps, cutscenes (verified).
- Body + face track the toggle cleanly; caps just re-routed to the private loader
  (in testing).
- **Kill switches:** `D_ALBW_SUMO_PRIVATE_BODY` (body), `D_ALBW_SUMO_PRIVATE_FACECAP`
  (face + caps) — either → `0` restores the prior global path.
- **Open risk:** the body uses crash-proof build-then-swap; the face/cap use
  *in-place* free + epoch-guard (simpler). The epoch guard covers draw+shadow but
  not `calc` — if a toggle-while-sumo-with-cap ever faults, that's the calc window
  and the fix is to give the cap the same build-then-swap.
- `alSumou` is small; the private heaps are game-heap-carved and (so far) survive
  warps, so no stage-transition reload was needed (the doc's "STILL TO TEST" item
  from `Quick-Sumo Work.md` — currently holding).

### Files
`src/d/d_albw_sumo_test.cpp` / `.h` (private loaders + coordinator + accessors),
`src/d/actor/d_a_alink_wolf.inc` (`changeLink` sources body/hand/topknot/face/cap
via the private accessors), `src/d/d_albw_outfit.cpp` / `.h`
(`dAlbwOutfit_forceReapply`), `src/dusk/custom_assets.cpp` / `.hpp`
(`overlay_generation()` — the re-mount signal).

### Still reboot-scoped: audio
The `.aw`/`.baa` wave banks are session-resident (JAudio2), so **voice does not
revert on a mid-session toggle** — that's the separate "audio reapply" follow-up
(model composite was done first). Audio applies cleanly when enabled at boot.

---

## 6. Positioning for a merge with the official Dusklight team

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
- **The sumo composite work (§5) is ALBW-specific**, not part of the general API —
  it's what a resident-asset consumer must do to honor a live toggle. Kept behind
  kill switches so it can be lifted out cleanly.

---

## 7. Current file state

| File | Role | Status |
|------|------|--------|
| `src/dusk/custom_assets.cpp` / `.hpp` | Whole system: Layer A (scan + overlay), Layer B (loose BMD), folder toggle, `overlay_generation()` | Working; audio confirmed in-game |
| `src/m_Do/m_Do_main.cpp`               | `scan()` (startup) + `install_overlays()` (after disc open) | Wired |
| `src/m_Do/m_Do_dvd_thread.cpp`         | Stock arc mount (custom 2b mount **removed**) + temp `[ca-diag]` hook | Reverted to stock |
| `src/dusk/ui/editor.cpp`               | ALBW → Custom Models multi-toggle picker | Working |
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.customModelsDisabled` setting | Working |
| `src/d/actor/d_a_b_gm.cpp`             | Layer-B consumer (Armogohma `B_gm`) | Working |
| `src/d/d_albw_sumo_test.{cpp,h}`, `d_a_alink_wolf.inc`, `d_albw_outfit.{cpp,h}` | §5 resident-asset live toggle (sumo composite) | Body/face working; caps in test |
| `extern/aurora/lib/dolphin/dvd/{dvd,fst}.cpp` | Aurora overlay system (upstream, unmodified) | Used as-is |

---

## 8. Open items / limitations

- **Caps final verification** (§5) — color caps just re-routed to the private
  loader; confirm they track on toggle across all Cap Wear modes.
- **Audio live toggle** — `.aw`/`.baa` still reboot-scoped; the audio-reapply
  follow-up (re-init the resident wave banks on toggle) is not built.
- **Face/cap build-then-swap upgrade** — currently in-place + epoch-guard; upgrade
  to build-then-swap if the calc window ever faults.
- **Single-asset granularity:** sub-arc assets still need Layer B (loose BMD) or a
  repacked arc; Layer A is whole-file only.
- **Audio consistency (modder rule):** a mod that overlays `.aw` waves must also
  overlay the matching `Z2Sound.baa` (WSYS offsets are baked together). Linkle
  ships both.
- **Verify a compressed arc** through Layer A (Linkle's are raw RARC).
- **Strip before push:** `[ca-diag]` (`custom_assets.cpp` + `m_Do_dvd_thread.cpp`),
  `ALBW-CAP` / `ALBW-CAPLOAD` (`d_albw_sumo_test.cpp`, `d_a_alink_wolf.inc`).
