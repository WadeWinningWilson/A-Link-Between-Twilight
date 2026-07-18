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
- **Covers:** arcs (raw OR compressed) and any whole disc file **except audio** —
  one hook, no edits to the vendored JAudio2 library. Compression is handled by the
  stock loader because arcs mount through the normal entrynum path against the
  overlay (this is exactly what retiring the old custom mount unlocked).
- **`Audiores/` is deliberately EXCLUDED** from this overlay (as of 2026-07-06). It
  worked for *boot-scoped* audio, but wave samples are boot-bound so it could never
  hot-swap. Audio now has its own runtime-toggleable layer — vanilla stays the
  resident base, the mod is redirected in at the mixer. See **[§6](#6-runtime-toggleable-custom-audio-shadow-wave-redirect--per-wave-wsys-remap)**.
- **Status:** arc/model redirect working in-game; audio moved to §6.

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

#### Layer-B crash-avoidance (learned the hard way — WW iron boots, 2026-07-08)
Two crashes that will hit **any** Layer-B consumer whose caller runs during a
clothes/actor rebuild (not just the boots). Both fixed in `try_load`:
- **GameHeap pin (the important one).** `loaderBasicBmd` allocates the model's
  derived arrays — notably `J3DMaterialTable::mMaterialNodePointer` — on
  `JKRHeap::sCurrentHeap`, *outside* our retained raw buffer. When the caller is
  e.g. `daAlink_c::changeLink`, the current heap is Link's **transient clothes
  arc heap**, torn down every rebuild via `mpArcHeap->freeAll()`. Our
  `J3DModelData` is cached for the session, so the array dangles after the next
  rebuild → `getMaterialNodePointer(0)` reads `NULL[0]` → `EXCEPTION_ACCESS_VIOLATION`
  at `0x0` (inside `mDoExt_J3DModel__create`). **Fix:** pin `mDoExt_getGameHeap()`
  (the persistent parent, 20× on PC) around the `loaderBasicBmd` call, restore
  after. Armogohma dodged this only because it loads on its actor *solid* heap.
  Do **not** pin root/system heap — they hold almost no free space of their own
  (carved into children) and the alloc aborts.
- **Material-node validation guard.** After `loaderBasicBmd`, reject any model
  with `getMaterialNum()==0` or `getMaterialNodePointer(0)==nullptr` (→ `nullptr`,
  caller uses the arc). A SuperBMD BMD built **without its `-m` material JSON** has
  a degenerate MAT3 that loads "successfully" yet NULL-crashes on first `initModel`.
- **Asset-side corollary:** always build the loose BMD with SuperBMD `-m <mat>.json
  -x <texhdr>.json`; without `-m` the material table is degenerate. See
  [`Blender-WW-Items.md`](Blender-WW-Items.md) ▶ NEW CHAT block.
- **Rigged model + original animations?** Editing a rigged BMD in Blender re-derives
  every bone's local frame, so the game's shared animations tear the mesh (invisible
  at rest; limbs shatter when animated). Blender/SuperBMD cannot avoid it — fix the
  exported BMD with the reskin tool: [`BMD-Reskin-Tool.md`](BMD-Reskin-Tool.md).

### Retired — standalone arc mount (old Phase 2b)
- The old `mDoDvdThd`-level custom mount (`dusk_loadLooseArc` +
  `mOverridePath` + `find_override`) has been **removed**. Arcs now redirect via
  Layer A. This deleted the only `find_override` caller and unlocked
  compressed-arc support (the old mount was raw-RARC only). `m_Do_dvd_thread.cpp`
  is back to a stock mount path plus a one-line note.

### Mods window — load order UI  (`src/dusk/ui/mods.cpp`, title screen → Mods)

Full design + playtest notes: **[Mod-Load-Order-Design.md §18](Mod-Load-Order-Design.md#18-collection-ui-subgroups--mods-right-pane-scroll-2026-07-17)**.

- **Load-ordered since 2026-07-11** ([Mod-Load-Order-Design.md](Mod-Load-Order-Design.md) §10):
  `game.customModelsOrder` is the authoritative setting — `|`-delimited folder names
  in **priority order** (top = wins conflicts, decision D1), `-` prefix = disabled.
  One rule at every conflict site (Layer-A `s_map`, audio `s_audioIndex`/`s_modWaves`,
  icons, Layer-B subfolder search): iterate mods top-first, first provider of an asset
  claims it.
- `game.customModelsDisabled` is LEGACY: dual-read + mirrored on every write, for one
  release; then retire.
- **Left pane:** one position-bound row per **mod group** — plain mods use the folder
  name; collections collapse to one row (`HD Tunics (17 variants) — ON — Kokiri Green`).
  Labels refresh each frame via `order_view()` + `group_keys_in_priority_order()`.
  Grab-and-place reorder calls **`move_mod_group()`** / **`move_mod_group_to()`**
  (whole variant block moves together).
- **Right pane:** plain mod = **Enabled** toggle; collection = **Variants** picker
  (`select_collection_variant()`). modinfo screenshot/description + conflict/core info.
  Tall panels scroll via **`Pane::scroll_by()`** on Uncontrolled panes (wheel + pad
  Up/Down after focusable children — see §10 below).
- **One toggle gates both layers.** `toggle_folder()` / `move_folder()` /
  `select_collection_variant()` / `move_mod_group*()` update the setting, then
  `scan()` + `install_overlays()` so the change lands on the **next asset load**
  (reload-scoped — see §4). Layer B checks the toggle at load time.
- **Disk format unchanged:** collections still store every variant as
  `Collection/Variant` in `customModelsOrder`; only UI presentation + group-move
  helpers were added.
- **Status:** Mods window shipped in tree; collection subgroups + right-pane scroll
  added 2026-07-17 (§10).

> **Editor note.** The old ALBW-tab Custom Models picker in `editor.cpp` is superseded
> by the standalone Mods window for load-order management.

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

### Audio: now live too (2026-07-06)
Originally reboot-scoped — `.aw`/`.baa` wave banks are session-resident (JAudio2),
so voice didn't revert on a mid-session toggle. **This is now solved** by a
different mechanism than the model composite (audio can't use private mounts or a
re-init): a shadow-wave redirect in the software mixer. See **[§6](#6-runtime-toggleable-custom-audio-shadow-wave-redirect--per-wave-wsys-remap)**.

---

## 6. Runtime-toggleable custom audio (shadow-wave redirect + per-wave WSYS remap)

**Status (2026-07-06): working, committed `6389807a6a`.** Custom voice/BGM now
tracks the Custom Models toggle **mid-session, instantly** — including a mod that
was **disabled at boot** (the case a downloaded mod always hits, since a download
can't be enabled before first launch). Kill switch: `D_ALBW_AUDIO_SHADOW`
(in `DuskDsp.hpp`) → 0 compiles it out to stock.

### Why audio needed a different approach than models
Models hot-swap via private-heap build-then-swap (§5). Audio can't:
- Wave **samples are read into ARAM once at boot** and never re-read; redirecting
  the file path after boot changes nothing (the consumer already cached).
- The JAudio2 subsystem **cannot be re-init'd** — `JAU_JASInitializer::initJASystem`
  hard-guards against a live audio thread (warns + no-ops), and there is **no
  teardown anywhere** for its global mem-pools / section-heap singleton / the audio
  thread. Every prior path (scene-wave reload; the full "re-init gamble") was a
  confirmed dead end.

### The intermediary: shadow-wave redirect (downstream of the boot cache)
Instead of rebuilding vanilla, **vanilla stays the resident base and the mod is
layered on at the sample-fetch point** in our own PC software mixer (`DuskDsp`):
1. The model overlay (Layer A) now **excludes `Audiores/` wholesale**, so vanilla
   audio always loads to ARAM as the untouched base.
2. As each vanilla wave bank loads to ARAM, if a mod ships that `.aw`, its bytes
   are read into a **pooled RAM buffer** (kept for the run so an in-flight mixer
   read can't dangle) and registered against the bank's ARAM range.
3. In `ReadChannelSamplesChunk` (the per-chunk sample fetch), one conditional picks
   the mod buffer vs vanilla ARAM. The **toggle just flips an active flag** — both
   sets are always resident, so it's instant. The audio thread is never touched.
   Registry is read lock-free on the audio thread (the hurricane-spin idiom).

### Per-wave WSYS remap (why re-encoded voices decode correctly)
A re-skin's voices are **re-encoded to different lengths**, so their `.aw` is *not*
byte-compatible with vanilla — a raw offset swap decodes garbage (this is why the
mod ships its own `.baa`). Fix, fully generic:
- Parse the **mod's own `.baa`** (WSYS — mirrors `JASWSParser`, handles simple &
  basic banks) into per-wave descriptors keyed by **(`.aw` leaf, wave id)**.
- At **`JASBank::noteOn`** the game hands us the wave id + the vanilla ARAM address,
  so we substitute the **mod descriptor** (offset / length / sampleCount / loop /
  format) and repoint at the mod sample. ADPCM coefficients live **per-frame** (fixed
  global tables, read from the data itself), so no coefficient plumbing is needed —
  the mod bytes decode correctly on their own.
- The vanilla `.baa` stays resident, so all non-modded audio is untouched.

### Files
| File | Role |
|------|------|
| `src/dusk/audio/DuskDsp.{cpp,hpp}` | Shadow-wave registry + the redirect in `ReadChannelSamplesChunk`; `D_ALBW_AUDIO_SHADOW` kill switch |
| `src/dusk/custom_assets.{cpp,hpp}` | `Audiores/` overlay exclusion; twin-buffer pool; active flag (follows toggle, enable-independent load); mod-`.baa` WSYS parse → per-wave descriptors; `remap_voice()` |
| `libs/JSystem/src/JAudio2/JASWaveArcLoader.cpp` | Hooks: note `entrynum→name`, acquire twin on ARAM load, release on erase |
| `libs/JSystem/src/JAudio2/JASBank.cpp` | `noteOn` per-wave remap hook |

### Known follow-up
The DSP redirect is **bank-range based**, which is exact only when *every* wave in
a shadowed bank has a mod twin (a "complete" audio mod — true for Linkle). A
**partial-bank** mod (only some waves replaced) wants **per-wave exact keying** (key
the redirect on the exact vanilla wave address instead of the bank range). Clean
generalization; not needed for complete re-skins.

---

## 7. Positioning for a merge with the official Dusklight team

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

## 8. Current file state

| File | Role | Status |
|------|------|--------|
| `src/dusk/custom_assets.cpp` / `.hpp` | Layer A (scan + overlay), Layer B (loose BMD), folder toggle, **collection grouping API** (§10), `overlay_generation()`, **custom-audio shadow layer** (§6) | Working; models + audio live in-game |
| `src/dusk/ui/mods.cpp` / `.hpp` | Title-screen **Mods** window — collapsed collection rows, variant picker, grab-and-place group reorder | 2026-07-17 (§10) |
| `src/dusk/ui/pane.cpp` / `.hpp` | Uncontrolled pane scroll (`reset_scroll`, `scroll_by`, wheel + pad) | 2026-07-17 (§10) |
| `src/dusk/audio/DuskDsp.{cpp,hpp}`, `libs/JSystem/src/JAudio2/JASWaveArcLoader.cpp`, `.../JASBank.cpp` | Runtime custom audio — shadow-wave redirect + per-wave WSYS remap (§6) | Working; committed `6389807a6a` |
| `src/m_Do/m_Do_main.cpp`               | `scan()` (startup) + `install_overlays()` (after disc open) | Wired |
| `src/m_Do/m_Do_dvd_thread.cpp`         | Stock arc mount (custom 2b mount **removed**) + temp `[ca-diag]` hook | Reverted to stock |
| `src/dusk/ui/editor.cpp`               | ALBW tab (legacy Custom Models picker — superseded by Mods window) | Superseded |
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.customModelsOrder` (authoritative) + legacy `game.customModelsDisabled` (dual-read/mirrored one release) | Order setting uncommitted |
| `src/d/actor/d_a_b_gm.cpp`             | Layer-B consumer (Armogohma `B_gm`) | Working |
| `src/d/d_albw_sumo_test.{cpp,h}`, `d_a_alink_wolf.inc`, `d_albw_outfit.{cpp,h}` | §5 resident-asset live toggle (sumo composite) | Body/face working; caps in test |
| `src/d/d_albw_menu_res.cpp`, `include/d/d_albw_menu_res.h`, hook in `d_s_play.cpp` | §5-class live re-mount of boot-resident menu 2D arcs (itemicon/clctres/dmapres) on overlay change — build-then-swap, menu-closed gate; kill switch `D_ALBW_MENU_RES_REMOUNT` (see Mod-Load-Order-Design.md §10) | Uncommitted, awaiting playtest |
| `extern/aurora/lib/dolphin/dvd/{dvd,fst}.cpp` | Aurora overlay system (upstream, unmodified) | Used as-is |

---

## 9. Open items / limitations

- **First-toggle clean apply (native clothes) — FOR LATER.** The sumo composite
  tracks on the first toggle (private mounts), but **native Link clothes** (`al.bmd`
  body / `al_face` from the pipeline clothes arc `Kmdl`/`Bmdl`/`Zmdl`/`Mmdl`) don't
  re-read on a *same-outfit* toggle: the resource manager is name-cached AND
  `loadModelDVD`'s same-arc re-equip path deliberately rebuilds in place without
  reloading (that skip avoids the clothes-pipeline `freeAll` double-free). So the
  first toggle shows a mix; it resolves on the next outfit switch (a *different* arc
  reloads). Fix options: (a) accept (one quick-swap), (b) auto re-equip on toggle
  (crash-safe, brief flicker), (c) move native clothes onto private mounts like the
  sumo body (first-toggle-clean but a big/risky change in the crash-prone pipeline).
- **Caps final verification** (§5) — color caps just re-routed to the private
  loader; confirm they track on toggle across all Cap Wear modes.
- **Audio live toggle — SOLVED (2026-07-06), see [§6](#6-runtime-toggleable-custom-audio-shadow-wave-redirect--per-wave-wsys-remap).**
  Custom voice/BGM now tracks the toggle mid-session (shadow-wave redirect in the PC
  software mixer + per-wave WSYS remap). Historical dead ends, kept for the record:
  - Scene-level wave reload (`Z2SceneMgr::_load1stWaveInner_1/2` erase+load): the
    reload *ran* but audio never changed — `JASWaveArc::load()` re-activates data
    **bound at boot**, it doesn't re-read the `.aw`.
  - Full audio re-init ("the gamble"): abandoned — `initJASystem` hard-guards against
    a live audio thread and there is no teardown for the singleton-heavy subsystem.
  - The shadow-wave redirect sidesteps both: vanilla stays resident, the mod layers on
    at the sample-fetch point — no reload, no re-init, audio thread untouched.
  - Remaining follow-up: partial-bank mods want per-wave exact keying (§6).
- **Face/cap build-then-swap** — DONE (2-slot per arc, in-game verified). Heap-robust:
  private slots retry on game-heap-full (transient, not permanent-fail), release the
  non-live slot after each rebuild (≈1 heap/arc), and free unused cap arcs when stable
  — fixes the Zora+color-cap+sumo exhaustion that froze the body vanilla.
- **Single-asset granularity:** sub-arc assets still need Layer B (loose BMD) or a
  repacked arc; Layer A is whole-file only.
- **Audio consistency (modder rule):** a mod that replaces `.aw` waves must ship its
  matching `Z2Sound.baa` — the audio layer parses that `.baa` for the mod's wave
  descriptors (§6). (It is **not** overlaid onto vanilla; vanilla's `.baa` stays the
  resident wave table. The mod `.baa` lives at `<mod>/files/Audiores/Z2Sound.baa`.)
  Wave **ids** must line up with vanilla (a re-skin keeps them); differing byte
  sizes/offsets are fine — that's what the remap handles. Linkle ships both.
- **Verify a compressed arc** through Layer A (Linkle's are raw RARC).
- **Strip before push:** `[ca-diag]` (`custom_assets.cpp` + `m_Do_dvd_thread.cpp`),
  `ALBW-CAP` / `ALBW-CAPLOAD` (`d_albw_sumo_test.cpp`, `d_a_alink_wolf.inc`).
  (The audio layer ships only informative `[custom_assets]` logs — no temp probes.)

---

## 10. Mods window — collection UI subgroups + right-pane scroll (2026-07-17)

Implements collapsed **collection subgroups** in the Mods load-order UI and
**Uncontrolled pane scrolling** for tall right-pane mod panels. Full UX tables,
playtest checklist, and build note: **[Mod-Load-Order-Design.md §18](Mod-Load-Order-Design.md#18-collection-ui-subgroups--mods-right-pane-scroll-2026-07-17)**.

**Design constraint:** `game.customModelsOrder` still stores every variant as
`Collection/Variant`. Scan, conflict resolution, and `list_folders()` are unchanged.

### Public API — `include/dusk/custom_assets.hpp`

| Symbol | Line | Role |
|--------|------|------|
| *(comment block)* | **115–119** | Collection UI grouping overview |
| `mod_group_key(folder)` | **122** | `"Coll/Var"` → `"Coll"`; plain mod → folder name |
| `mod_is_collection_variant(folder)` | **125** | True when path contains `/` |
| `select_collection_variant(variant_folder)` | **129** | Enable one variant; disable siblings; `scan()` + `install_overlays()` |
| `move_mod_group(group_key, delta, apply)` | **133** | Move whole group one visible slot (`apply=false` defers rescan) |
| `move_mod_group_to(group_key, slot, apply)` | **136** | Grab-and-place at visible group slot |

### Implementation — `src/dusk/custom_assets.cpp`

| Block | Lines | Notes |
|-------|-------|-------|
| `mod_group_key` | **1108–1115** | |
| `mod_is_collection_variant` | **1117–1119** | |
| `order_as_group_blocks` (static) | **1121–1138** | Partition order into contiguous group blocks |
| `flatten_group_blocks` (static) | **1140–1147** | |
| `group_block_on_disk` (static) | **1149–1158** | |
| `find_group_block_index` (static) | **1160–1171** | |
| `select_collection_variant` | **1173–1197** | |
| `move_mod_group` | **1199–1243** | Swaps visible group blocks |
| `move_mod_group_to` | **1245–1304** | Mirrors `move_folder_to` on group blocks |
| Non-PC stubs | **2391–2395** | No-op returns for `!TARGET_PC` |

**Side effect:** reordering a collection may normalize scattered variant entries into
one contiguous block (equivalent resolver output; list layout may change).

### Mods UI — `src/dusk/ui/mods.cpp`

| Block | Lines | Notes |
|-------|-------|-------|
| Header comment (interaction model) | **1–18** | Collapsed collections + grab-and-place |
| `ModUiGroup` + `build_ui_groups` | **49–72** | Build member lists from `list_folders()` at window open |
| `ModUiGroupMap` + `build_ui_group_map` | **74–82** | |
| `group_keys_in_priority_order` | **84–94** | Per-frame slot order from `order_view()` |
| `group_at_slot` | **96–107** | Row index → group for labels + right panel |
| `group_row_title` / `group_any_enabled` / `group_active_variant_name` / `group_conflicts` | **109–155** | Row label + conflict aggregation |
| `group_panel_folder` | **157–167** | Enabled variant for modinfo, else first member |
| `populate_mod_detail_panel` | **285–377** | Plain: Enabled toggle. Collection: variant buttons |
| Left-pane loop (group rows, grab uses `move_mod_group*`) | **409–507** | Drop uses **`keys[s_grabbedSlot]`** (grabbed group) |

### Right-pane scroll — `src/dusk/ui/pane.{hpp,cpp}`

| Symbol / behavior | Lines | Notes |
|-------------------|-------|-------|
| `Pane::reset_scroll()` | **pane.hpp 37**, **pane.cpp 247–249** | `SetScrollTop(0)` |
| `Pane::scroll_by(delta)` | **pane.hpp 38**, **pane.cpp 253–257** | |
| `Pane::clear()` | **pane.cpp 259–264** | Calls `reset_scroll()` after clearing children |
| Uncontrolled **Mousescroll** | **pane.cpp 75–81** | Wheel scroll; capture phase |
| Uncontrolled **Up/Down scroll** | **pane.cpp 83–115** | After focusable children exhausted, scroll 48px/step |

---

## Note for future chats — a dedicated Custom Audio API doc

This doc is now **Custom *Models*** first, with the runtime-audio system folded in as
§6 because it shares the `dusk::custom_assets` module, the mod-folder layout, and the
same toggle. That's fine at today's size. **If the audio system grows** — partial-bank
exact keying, multi-audio-mod support, streamed-BGM handling, sequence/`.bms`
replacement, an authoring/validation tool for modders, or it graduates into its own
`dusk::custom_audio` module — **split it into its own `Custom-Audio-API-Work.md`** and
leave §6 here as a one-paragraph pointer. The natural seam already exists: the audio
work lives in `DuskDsp` + the `JASWaveArcLoader`/`JASBank` hooks + the audio half of
`custom_assets`, all behind `D_ALBW_AUDIO_SHADOW`, so lifting it out is a clean cut.
