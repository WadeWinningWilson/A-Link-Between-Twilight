# Mod Load Order — Design (custom_assets evolution)

**Status (2026-07-11): ALL THREE PHASES IMPLEMENTED (uncommitted).** Phase 1 playtested
(menu-icon fix §10, incl. the menu-open gate trap); Phase 2 (Core tier + texture
folding, §11) and Phase 3 (conflict badges, §12) awaiting playtest. Research + design
record for evolving
the Custom Assets system into a load-ordered mod manager, modelled on Mod Organizer 2.
Cross-references [Custom-Model-API-Work.md](Custom-Model-API-Work.md) (the underlying
`dusk::custom_assets` system this builds on).

---

## 1. Goal

Turn the flat "folder of mods, each on/off" system into an **ordered** one, so that:
- when two enabled mods provide the same asset, an **explicit priority** decides the
  winner (today it is silent "last-scanned-wins" = filesystem order = undefined);
- **first-party must-have content** (Boss Refinement models, Wind Waker skins, future
  models/textures) always loads, and user add-on mods layer on top without clobbering it;
- the two folders (`model_replacements/` + `texture_replacements/`) unify under one
  "a mod is an ordered folder of anything" model.

Non-goals (deliberately NOT copied from MO2): plugin/ESP load order (our feature logic is
compiled C++, not reorderable data records), USVFS OS-hooking (we already have a native
FST overlay), profiles, the overwrite folder.

---

## 2. Why MO2 maps so well — and what we already own

| MO2 concept | Our current equivalent | Status |
|---|---|---|
| USVFS virtual merge (no copy) | Aurora DVD overlay (Layer A, `install_overlays`) | **Have it** — natively, cleaner |
| Mods = isolated folders | `model_replacements/<mod>/` | Have it |
| Enable/disable per mod | `game.customModelsDisabled` toggle | Have it |
| **Mod priority / conflict order** | "last-wins", undefined | **The gap** |
| Conflict win/lose indicators | none | nice-to-have |
| Plugin (ESP) load order | N/A (compiled features) | not applicable |
| Profiles / overwrite folder | none | not needed |

We own three of MO2's four pillars. The missing pillar (explicit priority + conflict
resolution) is a `custom_assets` + settings + editor-UI change — **no overlay or engine
surgery**, because conflicts are already decided in `scan()` (`s_map[norm] = path`), not
in Aurora.

---

## 3. The implicit tier that already exists

`resolve_override()` (Layer-B loose BMD) checks, in order:
1. `model_replacements/<fname>` **directly in the root** — loads UNCONDITIONALLY (not
   toggle-gated, highest priority);
2. then each ENABLED subfolder, first match wins (filesystem order).

So today there is already a de-facto two-tier structure: **root-level loose assets =
always-on "core"**, **subfolder mods = toggleable**. The load-order design formalises this
rather than inventing it.

---

## 4. Design

### 4.1 Two tiers: Core (bundled) and User (add-on)

- **Core tier** — first-party must-have content that ships with Dusklight (Boss
  Refinement `B_gm`, Wind Waker `al_bootsH`, future models/textures). Lives in a
  **bundled install location** (next to the exe, e.g. `content/core/`, resolved via
  `SDL_GetBasePath()`), always enabled, read-only from the user's perspective. It is the
  baseline "Dusklight look." (Interim: the existing root-level loose-BMD slot serves the
  same purpose until a bundled dir is wired.)
- **User tier** — add-on mods in `<config>/model_replacements/<mod>/` (AppData). Ordered
  among themselves, toggleable.

Resolution across tiers: **User mods win over Core** (a user mod that explicitly provides
the same asset overrides it — intentional), and any User→Core override is flagged by
conflict detection (§4.5). Core provides everything a User mod does not touch, so
must-haves work out of the box. This is exactly MO2's "base game at the bottom, mods on
top" — Core is our base.

Feature-gating stays orthogonal: whether Boss Refinement even *requests* `B_gm` is decided
by the game's Boss-Refinement setting (C++), not the mod order. Core just guarantees the
asset is *there* when a feature asks.

### 4.2 Data model — one ordered setting

Replace the disabled-set with a single ordered list:
- `game.customModelsOrder` : `ConfigVar<std::string>` — `'|'`-delimited **User** mod names
  in priority order; a `-` prefix marks disabled (e.g. `linkle|-oldboss|sumo`).
- Order is authoritative; enabled state rides along. New folders not yet listed are
  **appended** (stable default) and enabled.
- Core content is not in this list (it is bundled, always-on) — the list orders User mods
  only. Priority: Core (base) < first User entry < … < last User entry, OR the reverse —
  see decision D1.

### 4.3 One resolution rule, applied everywhere

Pick **top of the list = highest priority = wins** (decision D1). Implement by iterating
mods in priority order and taking the **first** provider of each asset (skip if already
claimed). Apply the identical rule at every current conflict site:
- Layer-A overlay `s_map`
- audio `s_audioIndex` (+ `s_modWaves`: highest-priority `.baa` wins)
- icons `s_iconIndex` + `s_namedIconIndex`
- Layer-B `resolve_override` (Core/root first, then User subfolders in priority order)
- (Phase 2) texture packs

The overlay is unchanged — it is still handed a deduped set; the dedup is now
priority-ordered instead of accidental.

### 4.4 API + UI

- `list_folders()` returns User mods in priority order; add `move_folder(name, up/down)`
  (or `set_order(list)`); `is_folder_enabled` / `toggle_folder` stay.
- The editor's **Custom Models** section (`editor.cpp:2925`) becomes a **reorderable
  list**: each row gains ▲/▼ plus the existing enable toggle. Rows already exist per
  folder — up/down buttons drop in.

### 4.5 Conflict detection (Phase 3)

During the ordered scan, when a lower-priority mod is skipped for an already-claimed
asset, record `(asset, winner, loser)`. Surface a per-mod conflict count / badge (MO2's
win-lose flags), including User→Core overrides ("overrides a core asset").

### 4.6 Texture-pack folding (Phase 2)

Let each mod carry a `textures/` subtree loaded per-mod in priority order (Aurora's pack
loader already ranks internally; register per-mod groups in order). Keep the global
`texture_replacements/` as a lowest-priority fallback for back-compat. Result: a mod is
fully self-contained (models + audio + icons + textures), and the two-folder confusion
disappears.

### 4.7 Migration (back-compat)

On first run with `customModelsOrder` empty: seed it from today's alphabetical
`list_folders()` and carry `customModelsDisabled` → `-` flags exactly. Dual-read
`customModelsDisabled` for one release so nothing flips. No data loss.

---

## 5. How this can break CURRENT mods (risk analysis)

Overall risk is **low for the common case** (one mod per asset, careful migration), but
concentrated at:

1. **Settings migration.** If the `customModelsDisabled → customModelsOrder` seed is
   sloppy, a currently-enabled mod could disable (or vice-versa). *Mitigation:* seed order
   from `list_folders()` + copy disabled flags verbatim; dual-read the old key one release.
2. **Resolution flip (last-wins → priority-first-wins).** No change when only one mod
   provides an asset. Where two mods CONFLICT, the winner becomes *defined by order*
   instead of accidental filesystem order — so the mod that currently happens to win may
   change. *Mitigation:* default order = alphabetical (close to the current
   `list_folders()` sort); conflict detection makes it visible.
3. **`resolve_override` (Layer-B) order.** Must PRESERVE the root-direct "always-on"
   check (§3) — a must-have currently placed as a root loose BMD must keep loading
   unconditionally. *This is the highest-risk item if overlooked* — reorder only the
   subfolder search, never drop the root/Core-first check.
4. **Audio / icon winner change.** `s_modWaves` (single `.baa`) and the icon indexes move
   from "first found" to "highest priority" — could change which mod wins a rare conflict.
   Low risk.

Bottom line: nothing breaks for a single-mod setup with a correct migration; the real
watch-items are (a) migration fidelity and (b) preserving root/Core-first Layer-B
resolution.

---

## 6. Must-have coexistence (the core question)

The must-haves (WW skins, Boss Refinement models, future content) load today as *optional*
Layer-B overrides that fall back to vanilla if absent. To make them robust in a
load-ordered world:

- Ship them as **Core-tier content** (§4.1) — bundled, always-enabled, base priority.
  They become the guaranteed baseline instead of "hopefully the user has the folder."
- **User mods layer above** and may override a Core asset only by explicitly providing the
  same arc/index — a deliberate act, surfaced by conflict detection, not an accident.
- **Feature toggles stay separate** — Boss Refinement ON/OFF (a game setting) decides
  whether the model is requested; Core guarantees it exists; a user mod can restyle it.

Net: must-haves always work by default, users can still customise, and overrides are
explicit and visible — the MO2 contract.

---

## 7. Phasing

| Phase | Scope | Value |
|---|---|---|
| **1** | Ordered `customModelsOrder` + one-rule resolution (all indexes) + reorder UI + migration | Explicit priority; kills last-wins fragility |
| **2** | Core/bundled tier (install-dir scan) + fold texture packs per-mod | Must-haves robust; unify the two folders |
| **3** | Conflict detection + badges | Ecosystem polish |
| Future | Profiles | Only if the ecosystem grows |

---

## 8. Decisions (locked 2026-07-06)

- **D1 — priority direction: TOP wins.** Top of the list = highest priority = wins
  conflicts. This is the *opposite* of MO2 (which is bottom-wins = "later overrides
  earlier", matching manual-install/plugin-load-order intuition). Top-wins is more
  intuitive for newcomers; the tradeoff is handled by a **visible UI label** ("↑ top wins
  conflicts") so direction is never ambiguous.
- **D2 — one setting.** Single `game.customModelsOrder` encodes order + disabled (`-`
  prefix). `customModelsDisabled` is migrated + dual-read for one release, then retired.
- **D3 — Core location: DEFERRED to Phase 2** (leaning bundled install dir via
  `SDL_GetBasePath()`). Not needed for Phase 1. Today Dusklight has NO bundled content dir
  — all core + user content sits in AppData `model_replacements/`.
- **D4 — Core is overridable, behind a SEPARATE toggle.** A user mod may override a Core
  asset only when an explicit "allow core override" toggle is on (per-mod or global), with
  a clear warning — because overriding core can break boss fights / features. Extra care
  in implementation AND user messaging. Phase 2 (with the Core tier).

---

## 9. Handoff — copy/paste to a new chat

> **Dusklight — continue the load-order mod system (Phase 1).**
>
> Context: I'm building "Dusklight," an A-Link-Between-Worlds-style mod on a Twilight
> Princess PC decomp (`C:\Users\xxxxx\Documents\dusklight`). A prior chat designed a
> Mod-Organizer-2-style **load-order** evolution of the `dusk::custom_assets` system.
> **Read first:** `docs/Mod-Load-Order-Design.md` (the full design + risks + decisions),
> then `docs/Custom-Model-API-Work.md` (the underlying system), then
> `docs/build-fps-guidelines.md` + `docs/commit-and-push.md`.
>
> **Task: build Phase 1** (see §4 + §7): a single ordered `game.customModelsOrder` setting
> (order + `-`disabled), ONE resolution rule ("top wins", labelled) applied at EVERY
> current conflict site — Layer-A `s_map`, audio `s_audioIndex`/`s_modWaves`, icons
> `s_iconIndex`/`s_namedIconIndex`, Layer-B `resolve_override` — plus a reorderable
> Custom-Models editor list (▲/▼ + enable), and a safe migration from
> `customModelsDisabled` (seed alphabetical order, copy disabled flags, dual-read one
> release). Decisions D1/D2 are locked; Core tier (D3/D4) is Phase 2 — DON'T build it yet.
>
> **Critical watch-items (§5):** (a) migration must carry enabled/disabled state verbatim;
> (b) `resolve_override` must KEEP its root-first "always-on" check — only reorder the
> subfolder search, or a must-have loose BMD (boss model / WW boots) could stop loading.
>
> **Repo state (as of 2026-07-06):** custom AUDIO is committed (`6389807a6a`, local only —
> NOT pushed). UNCOMMITTED: custom item/named/shop ICONS + the `game.allowTextureDumps`
> toggle + doc updates (`Custom-Model-API-Work.md`, `Boss-Fights-RefinedGohma.md`,
> `Mod-Load-Order-Design.md`). Don't `git add -A` blindly — see "Never commit" in
> `commit-and-push.md`. Commit only when I ask; **never push** unless I say so.
>
> **Build discipline:** `build_run.bat` (RelWithDebInfo), then wipe
> `dawn_cache.db*`/`pipeline_cache.db*` in `%AppData%\TwilitRealm\Dusklight`. All changes
> stay in `custom_assets` + settings + the editor section — no overlay/engine surgery.

---

## 10. Phase 1 — implementation record (2026-07-11, uncommitted)

Built exactly to §4 + §7 Phase 1; decisions D1 (top wins) + D2 (one setting) as locked.
Core tier (D3/D4) deliberately NOT built.

### What changed

| File | Change |
|------|--------|
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | New `game.customModelsOrder` (`'|'`-delimited priority order, `-` prefix = disabled); `customModelsDisabled` marked LEGACY (dual-read + mirrored, one release) |
| `include/dusk/custom_assets.hpp` | `list_folders()` now returns PRIORITY order; new `move_folder(folder, ±1)` and `order_view(names)` (cheap per-frame view for the editor — setting parse only, no disk) |
| `src/dusk/custom_assets.cpp` | The order model (`parse_order_setting` / `full_order_list` / `persist_order`) + "top wins" applied at EVERY conflict site (see below) |
| `src/dusk/ui/editor.cpp` | Custom Models list is reorderable: priority-numbered toggle rows + `^ swap v` rows between neighbours; pane labelled "load order (top wins)" |

### The one resolution rule, site by site

All sites iterate mods in priority order and take the FIRST provider (insert-if-absent):

- **Layer-A `s_map`** — `emplace` instead of assignment (was last-scanned-wins); the
  per-mod scan log now reports `N shadowed by higher-priority mod(s) [top wins]`.
- **Audio `s_audioIndex`** — `emplace`, walked top-first (still enable-independent so a
  boot-disabled audio mod can be live-toggled).
- **Audio `s_modWaves`** — the highest-priority mod shipping `files/Audiores/Z2Sound.baa`
  wins (was filesystem-order first-found), matching the `.aw` winner.
- **Icons `s_iconIndex` / `s_namedIconIndex`** — `emplace`, enabled mods top-first
  (was last-wins assignment).
- **Layer-B `resolve_override`** — the root-direct "always-on" check is **UNTOUCHED and
  still first/unconditional** (§5.3 watch-item); only the subfolder search now walks
  enabled mods in priority order.

### Migration (§4.7) — as specified

`full_order_list()` = order-setting entries verbatim → disk folders not yet listed
appended **alphabetically with the legacy `customModelsDisabled` flag carried verbatim**
(this single rule is both the first-run seed and the steady-state new-folder append) →
legacy-disabled names not on disk appended as stale disabled entries (state survives a
remove-and-reinstall). `scan()` canonicalizes + persists this on every run (idempotent;
in-memory until the next `config::Save()` — re-derivation is deterministic so nothing can
flip). Every write **mirrors** the disabled set back into `customModelsDisabled` so a
downgrade to a pre-order build keeps identical enabled/disabled state. Stale entries
(folders not on disk) are preserved in the setting so a temporarily-removed mod keeps its
slot + enabled state; they are never shown in the editor or consulted at resolve time.

### UI notes

- The framework's panes are vertical button lists (no in-row sub-buttons), so reorder is
  a **`^ swap v` row between each pair of mods** (swaps the neighbours) instead of the
  sketched per-row ▲/▼ — same reachability (any permutation via adjacent swaps).
- Rows are **position-bound with per-frame labels** (`AlbwOrderRowButton` +
  `order_view()`, a setting-string parse with zero disk I/O), so a swap updates the list
  in place — rebuilding the pane from inside a button's own event handler would destroy
  the pressed button mid-dispatch.
- Toggle behavior (click row = enable/disable, highlighted = enabled) is unchanged.

### Behavior notes / edge cases

- A folder name starting with `-` cannot be represented in the order encoding (the `-`
  is the disabled marker). `|` is impossible in a Windows folder name.
- `toggle_folder` / `move_folder` rescan + re-install overlays (an order change can
  change conflict winners) — reload-scoped, exactly like the old toggle.
- Console/`!TARGET_PC` stubs added for the new API.

### Playtest result (2026-07-11) + the menu-icon fix

First playtest PASSED: migration, toggling, and reordering all work live. One bug: **menu
icons (start-menu outfit icons etc.) did not revert on toggle-off.** Root cause was NOT
the load-order code or the PNG icon system — Linkle overlays whole menu 2D arcs
(`res/Layout/itemicon.arc`, `clctres.arc`, `dmapres.arc`), and those are **ARAM-mounted
once at the logo screen** (`d_s_logo.cpp` `aramMount`) and held for the whole session —
the boot-resident asset class of Custom-Model-API-Work.md §5. A toggle rebuilds the
virtual FST but nothing ever re-reads those arcs, in either direction.

**Fix: `d_albw_menu_res` (new module) — live re-mount with build-then-swap.**
- Per-frame drive from `dScnPly_Draw` (idle cost: one int compare on
  `overlay_generation()`).
- On a generation change, each tracked arc compares its current winning provider
  (`custom_assets::overlay_path_for`, new API) against what its live instance was
  mounted from — only arcs whose provider actually CHANGED re-mount, via the same
  `mDoDvdThd_mountXArchive_c` ARAM mount the logo uses (overlay-aware, so mod-off
  re-reads vanilla disc bytes).
- **Build-then-swap (§5 sumo rule):** the global archive pointer
  (`dComIfGp_set{ItemIcon,CollectRes,DmapRes}Archive`) swaps to the fresh instance only
  while the start menu is CLOSED, so one menu session never mixes two
  instances; the old instance is retired (not freed) and freed on the NEXT swap.
  Consumers copy icon data (`readItemTexture` memcpy) or re-resolve the global per menu
  open, so nothing dangles. Steady state: at most one retired instance per arc.
- **GATE TRAP (found in playtest #2 — toggle-on never applied):** the first gate used
  `dMeter2Info_getMenuWindowClass() != NULL` as "menu open". WRONG — the `dMw_c` PROC is
  created alongside the HUD meter (`dMeter2_Create` → `fopMsgM_Create(MENUWINDOW)`) and
  lives for the entire play session, so the pointer is virtually always non-NULL and the
  swap deferred forever (mounts completed; log showed `re-mounting` with no `swapped
  in`). The real signals, now used by `menu_2d_open()`: **`dMw_c::mMenuProc != NO_MENU`**
  (a menu screen is active) plus **`dComIfGp_isPauseFlag()`** (the menu's screen-capture
  pause). Remember this for any future "is the start menu open" check.
- Kill switch: `D_ALBW_MENU_RES_REMOUNT` → 0 compiles it out (menu arcs revert to
  boot-scoped).
- Files: `src/d/d_albw_menu_res.cpp`, `include/d/d_albw_menu_res.h`, hook in
  `d_s_play.cpp`, `overlay_path_for()` in `custom_assets`, `files.cmake`.
- NOT covered (by design, minor): the LoP item belt's private `clctres` mount
  (`d_albw_lop_item_belt`) keeps its own instance until the HUD rebuilds; scene-scoped
  arcs (`Title.arc`, `fileSel.arc`, `Fshop.arc`, model/demo arcs) already track toggles
  via their normal per-scene mounts.

### Still to do (this phase)

- Playtest the menu-icon fix: toggle Linkle off in-field → open the start menu →
  outfit/item icons must be vanilla (and back on toggle-on). Check the `[menu_res]`
  re-mount/swap lines in the log.
- After one release of dual-read: retire `customModelsDisabled` reads/mirroring.

---

## 11. Phase 2 — implementation record (2026-07-11, uncommitted)

Built per §4.1 (Core tier), §4.6 (texture folding), D3 (bundled install dir) and D4
(core overridable behind a separate warned toggle). Same session as the Phase-1
menu-icon fix (§10).

### Core tier (D3/D4)

- **Location (D3):** `<exe>/content/core/<pack>/` (via `SDL_GetBasePath()`), each pack
  shaped exactly like a user mod folder — `files/` data tree, loose `<arc>_<idx>.bmd`,
  `icons/`, `textures/`. No packs shipped yet — the dir simply not existing is fine.
- **Always enabled, never listed** in the editor order; packs are ordered alphabetically
  among themselves.
- **Resolution (D4):** internal `ordered_sources()` is now THE sequence every conflict
  site walks (Layer-A `s_map`, audio `s_audioIndex` + `s_modWaves`, icons, Layer-B
  subfolder search, texture packs). Default: core packs FIRST (core wins conflicts).
  New `game.customModelsAllowCoreOverride` (default OFF) moves core LAST (user mods
  win, MO2 layering). Flipping it rescans + re-installs overlays.
- **The legacy root-level loose-BMD slot** in `resolve_override` is STILL first and
  unconditional, above core and user alike (§3 / §5.3 watch-item — unchanged again).
- Editor: the Custom Models pane shows "<b>N core pack(s)</b> bundled — always on,
  winning over / overridable by user mods"; a new **Allow Core Override** BoolButton
  sits under the list with an explicit warning ("overriding core assets can break boss
  fights").

### Texture-pack folding (§4.6)

- Every enabled source (core or user, winning order) may carry a `textures/` subtree in
  Aurora's hash-keyed replacement layout. `custom_assets::rebuild_texture_packs()`
  registers each as an `aurora::texture` `ReplacementGroup` with **descending priority
  (> 0)** — Aurora selects the highest priority per texture, so the load-order winner
  also wins texture conflicts.
- The global `<config>/texture_replacements/` dir keeps Aurora priority 0 = the
  lowest-priority fallback (back-compat, unchanged behavior).
- Rebuilt inside every `scan()` (toggle/move/core-flip); since texture replacement
  resolves per-draw by GPU hash, this part of a toggle is **INSTANT** (no reload
  needed). Gated on the same `enableTextureReplacements` master switch as the global
  dir; `texture_replacements::set_enabled` now refreshes mod packs on a flip.

### Files

| File | Change |
|------|--------|
| `include/dusk/settings.h`, `src/dusk/settings.cpp` | `game.customModelsAllowCoreOverride` (bool, default false) |
| `include/dusk/custom_assets.hpp`, `src/dusk/custom_assets.cpp` | `core_root()` / `core_pack_names()` / `Source` / `ordered_sources()`; all conflict sites walk sources; public `list_core_packs()` + `rebuild_texture_packs()` |
| `src/dusk/texture_replacements.cpp` | master-switch flip refreshes per-mod packs |
| `src/dusk/ui/editor.cpp` | core info line + Allow Core Override toggle (warned) |

### Still to do

- Playtest Phase 2: drop a test pack in `<exe>/content/core/` (e.g. move the WW loose
  BMDs into `content/core/wind-waker/`), verify it loads with all user mods disabled,
  verify a user mod copy of the same asset loses by default and wins with Allow Core
  Override ON; drop a `textures/` subtree in a mod and verify instant toggle.
- Phase 3 (conflict detection + badges) — pairs naturally with the planned launcher
  Mod page.

---

## 12. Phase 3 — implementation record (2026-07-11, uncommitted)

Conflict detection + badges per §4.5, built the same session as Phases 1–2.

- **Detection:** during the ordered `scan()`, every emplace-collision (a lower-priority
  source skipped for an already-claimed asset) is attributed via a claimed-by map and
  aggregated per source into `FolderConflicts { wins, losses, overridesCore,
  overriddenByCore }`. Covers Layer-A files AND item/named icon slots; the D4 flags set
  on user-vs-core collisions. Rebuilt by every scan (toggle / move / core flip);
  queried with `custom_assets::folder_conflicts(name)` (cheap map lookup).
- **Badges (editor):** each Custom Models row's per-frame label appends
  `[<w>w/<l>l]` when the mod has any conflicts, plus `CORE!` (this mod overrides core
  content) or `core` (core overrides it). Help text explains the notation. This is the
  same data a future launcher **Mod page** should render with proper iconography.
- **Log:** scan prints one `conflicts '<name>': N win(s), M loss(es)` line per source
  with collisions.
- Full per-asset (asset, winner, loser) tuples are aggregated away for now — if the
  launcher Mod page wants a per-asset conflict list, extend `record_conflict` to retain
  a capped sample list per source.

### Still to do

- Playtest Phase 2 + 3 (and re-test the §10 menu-icon fix after the gate fix): toggle
  Linkle ON in-field → open start menu → Linkle icons; OFF → vanilla. Two overlapping
  mods → badges show `[Nw/Ml]` and swap flips them.
- Launcher Mod page: render order + toggles + conflict badges via the existing
  `custom_assets` API (`list_folders` / `order_view` / `toggle_folder` / `move_folder` /
  `folder_conflicts` / `list_core_packs`).
- After one release: retire `customModelsDisabled` dual-read/mirroring.

---

## 13. Collection mods (2026-07-11, uncommitted) — the "HD Tunic Collection" shape

Stress-test case: a downloaded pack shaped `<wrapper>/<NN Variant>/tex1_*.dds` — 17
mutually-exclusive Dolphin-dump texture variants plus `Default Backup` (vanilla dumps)
and `Gallery` (preview PNGs). Two failure modes without support: dropped as-is the
wrapper has no recognized content (silent no-op); merged into one `textures/` tree,
Aurora's in-group dedup silently picks the alphabetically-first variant — exactly the
"accidental winner" class this system exists to kill.

### Handling (implemented)

- **Content detection** (`folder_content`): a folder "has content" if it holds `files/`,
  `icons/`, `textures/`, loose `*.bmd`, or **Dolphin-named dumps (`tex1_*.dds/.png`) at
  its root** (the shape variant folders ship in — no `textures/` subdir needed).
- **Collection explode** (`disk_entries`): a wrapper with no content of its own but with
  contentful immediate subfolders becomes one source per variant, named
  `"Collection/Variant"` (`/` cannot occur in a folder name → unambiguous in the order
  setting). One nesting level only. `Gallery` (no tex1 names) is skipped naturally;
  `Default Backup` (vanilla dumps) becomes just another variant row — enabling it is a
  harmless "restore vanilla look" option.
- **Variants default DISABLED** on first sight — a collection is a menu of options, not
  a stack (plain new mods still default enabled). The user turns ON the one they want;
  toggling texture packs applies INSTANTLY (per-draw hash resolution, no reload).
- **Texture conflicts now feed the Phase-3 badges**: `rebuild_texture_packs` attributes
  cross-source texture-key collisions (kept in a separate map so a master-switch flip
  can rebuild without touching scan-attributed stats; `folder_conflicts()` merges).
  Enable two variants → both rows badge (winner `[Nw/0l]`, loser `[0w/Nl]`), and Aurora
  still resolves correctly by priority regardless.
- Editor summary getValue throttled to 2 Hz (content detection made per-frame disk
  enumeration heavier).

### Stress-test setup (staged, awaiting playtest)

The collection is copied to `model_replacements/HD Tunic Collection (17 Variants)/`
(2,946 files, 1.1 GB). Expect: 19 new rows (17 variants + Default Backup), all
DISABLED by default, alongside the existing mods. Test: enable one variant → tunic
changes in-field within a second (no reload); enable a second → both rows badge and the
higher one wins; swap them → winner flips instantly; scan log shows per-variant
`texture pack '...'` + `conflicts '...'` lines.

---

## 14. Playtest-2 fixes + the Mods tab (2026-07-11, uncommitted)

### Audio corruption on toggle — FIXED (virtual-address latching)

Playtest: some audio sounded corrupted after toggling Linkle on/off mid-session. Root
cause was in the shadow-wave DESIGN, not the load order: the DSP redirect keyed on bank
ADDRESS RANGES gated by the global active flag at SAMPLE-FETCH time, so a mid-session
toggle re-routed IN-FLIGHT voices to the other data set at now-meaningless offsets (mod
offsets against vanilla bytes and vice versa) — sustained/looping notes decoded garbage
until they died. Fix (`DuskDsp.hpp/.cpp` + `remap_voice`): mod waves now live in a
VIRTUAL address space (`kShadowVirtualBase = 0x40000000`, unreachable by real ARAM ≤
~24 MB). `remap_voice` mints `virtualBase + bankBase + modOffset` at noteOn; the mixer
routes each fetch BY ADDRESS ALONE (virtual → mod buffer, real → vanilla ARAM, no flag
at fetch time). A voice is latched to its source for life; the active flag only gates
NEW noteOns. A stale virtual address (bank erased mid-note) silences + drains the voice
instead of reading out-of-bounds. Also: the mod-`.baa` walker now skips the
modding-tool `'bfca'` extension chunk (1 arg) instead of stopping — Linkle's wave
tables were parsed before it, so that warn was cosmetic, but now the walk completes.

### Custom shop icon drew tiny — FIXED (fill-slot sizing)

The wheel-icon draw size = texture size × `dItem_data::getTexScale(itemNo)` — and a
NAMED custom icon borrows a fallback itemNo (stamina row → Magic Jar) whose scale was
tuned for a different, smaller texture. Custom icons now size to FILL the row's icon
slot box (aspect-fit, `calcWheelIconDrawSize` fillSlot mode, per-row
`mRowItemPicCustom` flag) — matching the visual weight of the icons beside them.
(Separately, the icon encoder was upgraded from 32×32 direct RGB5A3 to **CI8 48×48
with a ≤256-color RGB5A3 palette** after parsing itemicon.arc showed vanilla icons are
CI8 at 48×48/40×64/56×47 — custom icons now match vanilla fidelity in the same 0xC00
buffer.)

### The Mods tab (replaces the ALBW-tab Custom Models section)

Custom Models now has its own editor tab ("Mods", like the level editor's Stage tab):

- **Left pane — Load Order (top wins):** one position-bound row per mod
  (`{n}. {name} - ON/OFF [badges]`), per-frame labels via `order_view`.
  **Grab-and-place reorder:** A/click grabs the focused mod (highlight + `> <`
  markers), Up/Down moves it a slot per press (the pane's natural focus move follows
  the mod because rows are position-bound), A places it; clicking a DIFFERENT row while
  holding drops the mod at that row (`move_folder_to`, new API — arbitrary slot in one
  rescan). Edge presses while grabbed are consumed so the grab can't escape the list.
- **Right pane — per-mod panel:** Enabled toggle button + conflict/core/variant info.
  This is the future home of modder-supplied descriptions (readme) + logos, per the
  planned collection sub-pages.
- "Allow Core Override" (D4) moved here under **Options**.
- The old ALBW-tab section (summary button + toggle rows + `^ swap v`) is gone.

### Still to playtest

- Audio: toggle Linkle on/off repeatedly during BGM + voices — no corruption, at worst
  a note that started before the toggle finishes on its original data set.
- Icon: stamina shop icon now matches neighbours' size (and vanilla sharpness).
- Mods tab: grab/move/place with pad and mouse; Enabled toggle in the right panel;
  badges update after moves.

---

## 15. Playtest-3 fixes (2026-07-11, uncommitted)

- **Garbled custom shop icon — FIXED.** The CI8 encoder wrote format byte `0x8`
  (= `GX_TF_C4`!) instead of `0x9` (`GX_TF_C8`) — the game decoded the 8-bit indices as
  4-bit pairs. Verified against a real vanilla icon BTI (`ari_mesu_00.bti`): fmt=9,
  transparency byte=2, tlutFmt=2, palOff=0x920, imgOff=0x20 — the encoder now matches
  vanilla field-for-field. (Trap for the record: GX C-format enums are C4=0x8, C8=0x9,
  C14X2=0xA.)
- **Mods UI relocated to its own top-level menu entry.** Per direction: not an editor
  tab — `ModsWindow` (`src/dusk/ui/mods.{hpp,cpp}`, one-tab Window like WarpWindow) on
  the Dusklight menu bar right after Editor (present even when advanced settings are
  off — mod management is an end-user feature). The editor's Mods tab and the helper
  class/static moved out of `editor.cpp`; `menu_bar.cpp` registers the entry;
  `files.cmake` gains `mods.cpp`. Grab-and-place interaction unchanged (§14).

---

## 16. modinfo.ini support (2026-07-11, uncommitted)

Fluffy-Manager-compatible mod metadata, per direction (categories/homepage parsed but
not yet surfaced; AddonFor / NameAsBundle / Requirement reserved for the bundle work,
which maps onto our collection sub-pages).

- **File:** `<mod-root>/modinfo.ini` — key=value lines, keys case-insensitive, values
  trimmed. Parsed keys: `name` (display name in the Mods list), `version` + `author`
  (byline in the panel), `description` (panel body; literal `\n` = forced line break),
  `screenshot` (image filename NEXT TO the ini; falls back to screenshot.png/jpg,
  preview.png/jpg), `homepage`, `category` (repeatable). Everything user-supplied is
  RML-escaped.
- **Screenshot preview:** rendered in the Mods panel via a `modshot://` RmlUi texture
  provider (single decoded slot — one panel at a time; `?rev=` busts RmlUi's texture
  cache). **PNG only for now** (aurora ships a PNG decoder; jpg/tga/bmp would need a
  new dependency — document for modders).
- **Display names:** `custom_assets::display_name(folder)` (scan()-cached map, per-frame
  safe) renames list rows; `mod_info(folder)` reads the full ini fresh on panel focus.
  Collection variants inherit the collection root's ini and render as
  "<collection name>: <variant folder>" unless the variant ships its own ini.
- **Placement matters:** the ini + screenshot live at the mod ROOT. (First real-world
  drop put both inside `icons/` — the screenshot then registers as a bogus named icon.
  Root-level PNGs are inert to every scanner: not tex1_-named → not a texture pack,
  not in icons/ → not an icon. So the convention is also the safe spot.)
- Files: `custom_assets.{hpp,cpp}` (`ModInfo`, `mod_info`, `display_name`,
  scan-time name cache), `ui/mods.cpp` (panel rendering + modshot provider).

---

## 17. Playtest-4 fixes + drop-and-play hardening (2026-07-11, uncommitted)

- **Parry-icon / white-static-box knock-on from icon toggles — FIXED.** Aurora clears
  its STATIC TEXTURE CACHE on every single texture-replacement register/unregister, and
  cached HUD/UI textures whose CPU-side source buffers are transient re-upload from
  dead memory after a clear (the recurring garbled-icon / white-box breakage class).
  `rebuild_texture_packs()` ran on EVERY scan — so toggling the (texture-less) Custom
  Icons mod, with any tunic variant enabled, churned ~160 register/unregister cache
  clears and broke the shield-parry icon. Fix: the rebuild fingerprints the DESIRED
  pack set (roots + priorities, master switch folded in) and is a strict NO-OP when
  unchanged — toggling a non-texture mod never touches the texture registry at all.
  (The underlying transient-buffer fragility predates this system; the remaining
  trigger is a REAL texture-pack change, same as flipping the master switch.)
- **Screenshot-in-icons self-heal:** any PNG over 256px found in `icons/` is skipped
  with a log hint ("looks like a screenshot/preview — put it next to modinfo.ini")
  instead of registering as a garbage named icon. Drop-mistake principle: a wrong drop
  produces NOTHING plus a log line, never corruption.
- Mods list: removed the how-to instruction paragraph (header + right-panel context is
  enough).
- Drop-and-play audit (user question): data trees, Dolphin texture packs/collections,
  icons, modinfo, complete audio re-skins = plain drop works. Modder-side authoring
  requirements remain (SuperBMD flags + reskin for rigged BMDs; matching .baa for
  audio; PNG screenshots). CODE MODS are NOT supported and are out of scope for this
  system — no scripting/plugin layer exists; would be its own major design (the load
  order would serve as its delivery mechanism).

---

## 18. Code mods — research complete (2026-07-11)

The end-goal research is written up in **[Code-Mods-Research.md](Code-Mods-Research.md)**:
tiered model (data / script / native), engine seam audit (l_objectName +
fpcPf_Get + the dScnPly hook precedent), recommendation = embedded Lua 5.4
("DuskScript", one sandboxed state per mod, event bus, per-frame instruction
budgets), script actors placed via the LEVEL EDITOR (the §4.4 synthesis), load-order
integration (scripts/ as recognized content, top-runs-last, Requirement enforcement),
dummy test mods for the build phase. Native plugins: deferred indefinitely.
