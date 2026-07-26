# Mod API host promote + local dusk-API merge — live state

| Field | Value |
|-------|--------|
| **status** | **Full mod host** on `integrate/dusk-api-coexist` — G1–G7 + launch-crash fix. **G1 united Mods UI landed** (§7.10): one page carries ALBT Layer-B load order **and** the `.dusk` package half (status, enable/disable/reload, mod panels, recent log). Field FPS **~200–220** (was ~250+) — refine via §7.8.7 A/B before feature-bisect |
| **target** | Public: promote clean ALBT host capabilities into TwilitRealm main; Local: full ALBT+main coexistence tree |
| **owner_impl** | Integration lane (merge); covenant/strip/gate = containment lane |
| **next** | Field-test the united Mods page (§7.10 leftovers: no icons/banner, no full LogsWindow); FPS A/B `DUSK_ENABLE_CODE_MODS=OFF`; chase `albw-meter` post-hook failure (§7.9.5) |
| **do_not** | `git merge origin/main` onto live WIP; push parked WW/ExtNPC/Ext Seq/bridge/audio-shadow; merge onto dirty tree; skip greplist after merge; feature-bisect an FPS drop before A/B-ing `DUSK_ENABLE_CODE_MODS=OFF` (§7.8.7); **throw while holding a lock/RAII guard — this tree has no `/EHsc`** (§7.9.2) |
| **updated** | 2026-07-25 |
| **anchor** | Tag `pre-dusk-api-merge` → commit `bfa264511c` (bus §113). Undo bad merge: `git checkout pre-dusk-api-merge` |
| **related** | [mod-api-port.md](mod-api-port.md) (feature `.dusk` extraction), [Custom-Model-API-Work.md](../Custom-Model-API-Work.md) |

---

## 1. Conflict answers (locked)

| # | Topic | Verdict |
|---|--------|---------|
| **1** | Mods UI | **Built 2026-07-25 (§7.10) — one Mods page, additive merge:** keep ALBT Layer-B **load-order / grab-and-place / collections / core-override**; **also** fold in main’s `.dusk` loader controls (status, enable/disable/reload, failure reason, mod settings panels, logs). Neither system fully replaces the other — same menu, both surfaces. `modinfo.ini` + screenshot helpers retained. *(Earlier coexist pass wrongly kept ALBT-only UI; that was a misread of “load order”, not the locked intent.)* |
| **2** | CMake / `/O2` | Acknowledged — treat post-import rebuild as FPS-untrusted until ninja FLAGS show `/O2` |
| **3** | Pack shape | Public = **`.dusk`**; keeps ini + screenshot helpers |
| **4** | Covenant / push | Hard-gate — never-push strip + greplist before any push (see §4) |
| **5** | GameService ABI | Acknowledged — epoch story when game-facing services land |
| **6** | WIP blast | Mitigated by isolated branch + pre-merge tag (reversible); still do not merge onto live boss/WW branches |

---

## 2. Public push set (clean — order)

1. **Layer B / richer `custom_assets` mount** on main  
2. **Mods UI** discovery + enable/disable (one united page)  
3. **Public pack shape = `.dusk`** carrying Layer-B payload + **`modinfo.ini` + screenshot** helpers  

### Explicit non-push (parked — local only / gitignore with non-clean set)

- Audio shadow  
- ExtNPC (mount/population/NPC receivers)  
- Ext Seq  
- Bridge + `schema_version` handshake  

Local ALBT tree keeps these for lab work. They must not enter a public push set until unparked.

---

## 3. Local vs public

| Surface | Local ALBT tree | Public main |
|---------|-----------------|-------------|
| Layer B + Mods UI + `.dusk` + ini/screenshot | Yes | Yes (promote) |
| ExtNPC / Ext Seq / bridge / audio shadow | Yes (full local merge) | No until unparked |
| Feature gameplay mods (e.g. `albw-meter`) | Parallel | Separate `.dusk` packages — [mod-api-port.md](mod-api-port.md) |

---

## 4. Local merge containment (hard gates)

This is a **coexistence import**, not `git merge origin/main` onto live WIP.

| Gate | Rule |
|------|------|
| Dirty tree | **Done** — checkpoint commit + tag before import |
| Branch | Isolated branch off `pre-dusk-api-merge`; never onto live boss/WW WIP |
| Revert | `git checkout pre-dusk-api-merge` (delete failed branch) |
| Mod folder | Separate local repo — untouchable by receiver merge |
| **#4 covenant** | Documented **never-push strip**: parked surfaces, WW receiver sources, donor/mod folder bytes. Greplist (M6) on exe from merged tree **before any push**. Push stays gated until CLEAN |
| FPS | After import rebuild: confirm `/O2` in RelWithDebInfo ninja FLAGS before feature-bisect |

**Irreversible risk is push (#4), not the merge experiment (#6).**

---

## 5. Execution shape

1. Loss-protection — **done** (`pre-dusk-api-merge`).  
2. Integration lane: coexistence import on isolated branch (sdk/loader + ALBT custom_assets/Mods UI coexist).  
3. Containment lane: finalize never-push strip set; greplist on merged exe.  
4. Boundary spec for Layer B + Mods UI + dusk layout (ini/screenshot paths).  
5. Promote clean slices toward main when ready; parked stay local.  
6. Unpark later: ExtNPC / Ext Seq / bridge / audio shadow as separate services after product maturity.

---

## 6. Success

- Main players: install Layer-B `.dusk`, see name/description/screenshot in **one** Mods page, toggle, mounts work without ALBT fork.
- Local tree: full ALBT receivers still run for development.
- Nothing parked ships publicly until explicitly unparked + gate CLEAN.

---

## 7. Coexistence import — session record (2026-07-25)

Branch **`integrate/dusk-api-coexist`**, cut from `pre-dusk-api-merge` (= `bfa264511c`,
which was also `main`'s tip, so the branch cut moved no files). Donor is
`origin/main` @ `95608438c1`, identical to the `dusklight-main` working tree.

### 7.1 Why selective, not `git merge origin/main`

Confirmed blast radius from the merge base `be82e606b2`:

| Surface | base → ALBT | base → main |
|---|---|---|
| `CMakeLists.txt` | +325 | +543 |
| `files.cmake` | +112 | +151 |
| `src/` + `include/` | — | 1560 files, +41k/−11k |

Main also moved the whole `include/dusk/**` tree to `src/dusk/**`. A real merge
would rewrite nearly every ALBT header path. Selective import stands.

### 7.2 Imported (55 files, staged, additive — nothing overwritten)

ALBT had no `sdk/`, no `src/dusk/mods/`, and only `WindowsTargetProcessor.cmake`
under `cmake/`, so every path below landed clean:

- `sdk/**` — 14 files: `CMakeLists.txt`, `include/mods/{api.h,hook.hpp,meta.hpp,service.hpp}`,
  `include/mods/svc/*.h` (11 services), `src/game_feature.cpp`
- `src/dusk/mods/**` — 32 files: `loader/` (bundle disk+zip, context, depgraph,
  loader, native_module, prepatch), `log_buffer`, `manifest`, `svc/` (camera,
  config, game, gfx, hook, host, log, overlay, registry, resource, texture, ui,
  slot_map)
- `src/dusk/mod_loader.hpp`, `src/dusk/gfx.hpp` — host headers the loader needs
- `cmake/ModSDK.cmake`, `cmake/CopyModAssets.cmake` — `add_mod()` / packaging
- `docs/modding.md`

### 7.3 Not wired — four host gaps block compilation *(historical — closed in §7.7)*

`files.cmake` and `CMakeLists.txt` were **intentionally left untouched**. The
imported sources cannot compile against this tree yet, so wiring them would only
buy a broken build plus a CMake re-run:

| # | Gap | Detail |
|---|---|---|
| **G1** | `dusk::ui::ModsWindow` collision | Declared by **both** ALBT `src/dusk/ui/mods.hpp` (load-order manager, `hide()` override) and main `src/dusk/ui/mods_window.hpp` (`update()`, `mods::LoadedMod` snapshots). Same namespace, same name. This *is* conflict answer #1 ("one united Mods page") arriving as a link error. `mods/svc/ui.cpp` + `loader/loader.cpp` also need main's `ui/mod_window.hpp` and `ui/mods_window.hpp`, neither of which exists here. |
| **G2** | Reactive config | `mod_loader.hpp` needs `config::Subscription`; ALBT's `include/dusk/config.hpp` has no subscription API. `ConfigVar<bool>` does exist. |
| **G3** | `miniz` | `loader/loader.hpp` includes `miniz.h`. Absent from the tree and from `extern/aurora`. Needed for `.dusk` zip bundles. |
| **G4** | `funchook` | `mods/svc/hook.cpp` includes `funchook.h`. Requires main's FetchContent block plus `PatchFunchook.cmake` / `PatchCapstone.cmake` — network fetch **and** a reconfigure. |

Minor: `mods/svc/host.cpp` includes `<version.h>` (main's `DetectVersion.cmake`
generates it); ALBT has `include/dusk/version.hpp` instead.

Native mods built with `FEATURES game` additionally need main's export
machinery — `cmake/{SymbolManifest,WindowsExports,GameABIConfig,DetectVersion}.cmake`
and the `symgen` tool — none of which was imported. Host-side loader work does
not depend on it; only mod linking does.

ALBT does already have the RmlUi UI stack (`src/dusk/ui/**`), `src/dusk/data.hpp`,
`dusk/io.hpp`, `aurora/lib/logging.hpp`, nlohmann/json and zstd, so those are
not blockers.

### 7.4 Build status

`cmd /c build_run.bat` → **exit 0**, `dusklight.exe` relinked (25,392,640 bytes).

Ninja *did* print `Re-running CMake...` — not from a hand-edited CMake file, but
because `DetectVersion` re-stamps on git state change (branch cut + new files →
`v1.4.1-155-dirty`). Only 8 targets rebuilt; the factory was not wiped.
Post-build check per [build-fps-safe](../../.cursor/rules/build-fps-safe.mdc):

- `build.ninja` `/O2` occurrences: **2209**
- `CMAKE_CXX_FLAGS_RELWITHDEBINFO` = `/O2 /Ob1 /DNDEBUG` ✅
- `dawn_cache.db*` + `pipeline_cache.db*` wiped ✅

Factory healthy. FPS still wants one field measure since CMake re-ran.

### 7.5 Never-push strip list (branch is local-only)

`integrate/dusk-api-coexist` **must not be pushed**. Before any push of any
slice cut from it, strip:

| Class | Contents |
|---|---|
| Parked WW surfaces | audio shadow; ExtNPC (mount/population/NPC receivers); Ext Seq; bridge + `schema_version` handshake |
| WW receiver sources | every receiver `.cpp`/`.hpp` reached by the parked surfaces above |
| Donor / mod folder bytes | separate local mod repo; no donor asset bytes in any commit |
| Re-imported donor code | `sdk/**`, `src/dusk/mods/**`, `cmake/{ModSDK,CopyModAssets,PatchFunchook,PatchCapstone,SymbolManifest,WindowsExports,DetectVersion,GameABIConfig}.cmake`, `docs/modding.md` are **main's own files** — pushing them back is a no-op at best and a revert at worst. Promote ALBT-side slices only. `cmake/GameABIConfig.cmake` is the one exception: it carries ALBT-local deltas (§7.8.2) that must **not** go to main. |

Gate stays CLOSED until the M6 greplist runs on an exe built from this branch.

### 7.6 Revert

```
git checkout main && git branch -D integrate/dusk-api-coexist
```

Working tree is unaffected — the branch cut moved no files, and `extern/aurora`
was never touched.

---

## 7.7 Host wiring session (2026-07-25, second pass) — G1–G6 closed

The §7.3 gaps are now closed and the imported loader/services **compile and link
into `dusklight.exe`**. One CMake-affecting edit batch; one reconfigure.

### G1 — ModsWindow collision: ADAPTED, not imported

ALBT keeps its own `src/dusk/ui/mods.hpp` `ModsWindow` (load-order manager).
Main's `mods_window.*` was **not** imported — `loader/loader.cpp` now includes
`dusk/ui/mods.hpp` instead. ALBT's `ModsWindow` already satisfies what the loader
asks of it (`Document` subclass with `visible()`).

Main's `ui/mod_window.*` (per-mod config page) **was** copied verbatim; it has no
collision. Supporting UI pieces main's mod stack needs were added additively to
ALBT files rather than replacing them:

| ALBT file | Added |
|---|---|
| `ui/ui.hpp` / `ui.cpp` | `DocumentScope` enum; `register_scoped_styles` / `unregister_scoped_styles` / `apply_scoped_styles`; `game_obscured_below`; `append_text`; `uncover_top_document` |
| `ui/document.hpp` / `.cpp` | `scope()`, `set_document_styles`, `restyle(span<StyleSheetContainer>)`, `cover()` / `uncover()`, `obscures_game()`; base-stylesheet snapshot for rebuild |
| `ui/menu_bar.hpp` / `.cpp` | `MenuBar::rebuild()` (mods registering/removing tabs at runtime); mod tabs appended from `mods::svc::ui_mod_menu_tabs()`; `DocumentScope::MenuBar` |
| `ui/modal.hpp` / `.cpp` | main's missing Modal pieces |
| `ui/string_button.hpp` / `.cpp` | `isDisabled` / `isModified` prop hooks |
| `ui/controls.hpp` | `operator==` on `ControlProps` + `ControlLayout` |

### G2 — Reactive config: PORTED

`include/dusk/config.hpp` + `config_var.hpp` + `src/dusk/config.cpp`:

- `config::Subscription` / `subscribe(name, ChangeCallback)` / typed
  `subscribe(ConfigVar<T>&, cb)` / `unsubscribe(token)`
- 2-arg `Register(ConfigVar<T>&, onChange)` — register + subscribe in one step
- `config::unregister(ConfigVarBase&)` — a mod's CVars can leave at unload; a
  user-set value is stashed so `Save()` keeps writing it and a later re-`Register()`
  back-fills it
- `ConfigVar` mutators (`setValue`, override/speedrun set + clear) notify on
  effective-value change; config **load** and launch args deliberately do not
  notify (they run before subsystems init, `load_value` / `load_override_value`)
- `ConfigVarBase::name` is now `std::string` (mods build names at runtime), and
  `RegisteredConfigVars` keys on `std::string`
- `FinishRegistration()` no longer fatals on late registration — mods register
  long after startup

**Copy-ctor fallout:** `ConfigVar` now deletes its copy constructor (matching
main). Nine ALBT call sites had lambdas whose deduced return type silently
*copied* a `ConfigVar` (`[] { return getSettings().game.speedrunMode; }`); each
got an explicit `-> bool` / `-> int` return type. `src/dusk/ui/settings.cpp` (8)
and `src/dusk/ui/editor.cpp` (1). ALBT's `Save()` / `LoadFromUserPreferences()`
PascalCase names were **not** renamed to main's snake_case.

### G3 — miniz: RESTORED

`FetchContent_Declare(miniz)` (3.0.2 release zip, `EXCLUDE_FROM_ALL`) like main;
`${miniz_SOURCE_DIR}/miniz.c` compiled into `dusklight` and `${miniz_SOURCE_DIR}`
on the target include path. `.dusk` zip bundles work.

### G4 — funchook / prepatch: STUBBED OFF *(historical — closed in §7.8)*

`DUSK_HAS_FUNCHOOK=0`, `DUSK_HAS_PREPATCH=0`, `DUSK_ENABLE_CODE_MODS` option
defaults OFF (so no `DUSK_CODE_MODS=1`). No network fetch of funchook/capstone,
no `PatchFunchook.cmake` / `PatchCapstone.cmake`, no reconfigure churn.
`svc/hook.cpp` and `loader/prepatch.cpp` compile as no-op backends.

**What this means for `albw-meter` and any native mod:** the hook *service ABI*
is present and a mod can call it, but every hook install is a no-op — nothing is
actually patched at runtime. Asset/resource/config/UI/overlay/game/camera
services work; **code mods do not**. Landing real hooks needs the funchook +
capstone FetchContent blocks, and native mods built with `FEATURES game`
additionally need main's export machinery (`SymbolManifest`, `WindowsExports`,
`GameABIConfig`, `DetectVersion` + the `symgen` tool) — still not imported.

### G5 — SDK visibility: DONE

`sdk/include` on `GAME_INCLUDE_DIRS`; `DUSK_BUILDING_GAME=1` in
`GAME_COMPILE_DEFS`. Confirmed on the compile line.

### G6 — Host call sites: DONE

| Site | Change |
|---|---|
| `src/dusk/data.hpp` / `.cpp` | `base_path_relative` promoted out of the anonymous namespace and declared publicly; added `user_home_path`, `normalized_display_path`, `abbreviated_path_string` (mod UI path display) |
| `src/m_Do/m_Do_main.cpp` | new `--mods <dir>` launch arg; search dirs = user dir (`--mods` replaces `ConfigPath/mods`) then `base_path_relative("mods")` with `inPlaceNative`; `ModLoader::init()` before `main01()`; `shutdown()` on the exit path |
| `src/f_ap/f_ap_game.cpp` | `ModLoader::instance().tick()` at the end of `duskExecute()` (inside `#ifdef TARGET_PC`), mirroring main |

ALBT's own data-path behavior is untouched — only additive declarations.

### G7 — NEW gap found: `aurora::gfx` missing (gfx/texture services off) *(historical — closed in §7.8)*

Undocumented in §7.3. This tree's `extern/aurora` submodule has **no
`aurora/gfx.hpp`** and none of `register_draw_type`, `is_offscreen`,
`uses_reversed_z`, `register_virtual_replacement`, `has_replacement`,
`kWildcardTextureHash`. Main's `svc/gfx.cpp` + `svc/texture.cpp` are built on
them, so a new switch gates the whole surface:

- `option(DUSK_MODS_AURORA_GFX ... OFF)` → `DUSK_MODS_AURORA_GFX=0`
- `svc/gfx.cpp` + `svc/texture.cpp` left **out of `files.cmake`**; their
  `g_gfxModule` / `g_textureModule` externs and registry entries are `#if`-guarded
- `svc/camera.cpp` guards its `<aurora/gfx.hpp>` include and falls back to
  `reversedZ = true` (mirrors `aurora::gx::UseReversedZ`, which this tree's aurora
  does not expose publicly)

Closing G7 means an aurora submodule bump — a much larger change than G1–G6 and
deliberately out of scope here.

### 7.7.1 Build + factory check

`cmd /c build_run.bat` → **exit 0**, `dusklight.exe` relinked.

CMake re-ran once (intentional — the single CMake-affecting batch: miniz
FetchContent, mod defines, `sdk/include`, `files.cmake` mod sources). Per
[build-fps-safe](../../.cursor/rules/build-fps-safe.mdc):

- `build.ninja`: **2230** of 2250 `FLAGS =` lines carry `/O2` (the 20 without are
  non-C/C++ rules) ✅
- `CMAKE_CXX_FLAGS_RELWITHDEBINFO` = `/O2 /Ob1 /DNDEBUG`;
  `CMAKE_C_FLAGS_RELWITHDEBINFO` identical ✅
- `dawn_cache.db*` + `pipeline_cache.db*` wiped ✅

Factory healthy. **FPS untrusted until one field measure** — CMake re-ran.

### 7.7.2 Remaining gaps *(G4 / G4b / G7 closed in §7.8)*

| # | Gap | Blocks |
|---|---|---|
| ~~G4~~ | funchook + capstone FetchContent | **done** — §7.8.1 |
| ~~G4b~~ | export machinery (`SymbolManifest` / `WindowsExports` / `GameABIConfig` / `DetectVersion` + `symgen`) | **done** — §7.8.2 |
| ~~G7~~ | `extern/aurora` bump for `aurora::gfx` + texture virtual-replacement API | **done** — §7.8.4 |
| — | `svc/host.cpp` `<version.h>` | resolved by this tree's headers; main generates it via `DetectVersion.cmake` — re-check if host version strings look wrong |
| — | Runtime smoke | nothing has actually loaded a `.dusk` bundle on this branch yet — compile-green only |
| ~~—~~ | One united Mods page (conflict answer #1) | **done** — §7.10 |

---

## 7.8 G4 + G7 session (2026-07-25, third pass) — real funchook and real `aurora::gfx`

`DUSK_ENABLE_CODE_MODS` and `DUSK_MODS_AURORA_GFX` now both **default ON** on this branch
(`CMakeLists.txt`; a public promote can flip the code-mods default back to main's OFF).
The existing cache had them OFF, so they were also set on the command line during the single
intentional reconfigure — `tools/_g4g7_configure.bat` is that exact command, kept for repro.

### 7.8.1 G4 — real funchook: DONE

| Piece | State |
|---|---|
| `cmake/PatchFunchook.cmake`, `cmake/PatchCapstone.cmake` | copied verbatim from main |
| FetchContent | funchook `v1.1.3`, `GIT_SHALLOW`, `PATCH_COMMAND` → `PatchFunchook.cmake`, `EXCLUDE_FROM_ALL`; `FUNCHOOK_BUILD_TESTS/SHARED/INSTALL` all OFF — mirrors main |
| Link | `funchook-static` appended to `GAME_LIBS` when `DUSK_HAS_FUNCHOOK`; `funchook.lib` is on the `dusklight.exe` link statement |
| Defines | `DUSK_CODE_MODS=1`, `DUSK_HAS_FUNCHOOK=1`, `DUSK_HAS_PREPATCH=0` |
| Prepatch | main's Apple-arm64 gate mirrored (`_target_architectures` / single-arch / `arm64`). Windows never takes it, so `loader/prepatch.cpp` stays a no-op backend |
| Hook reliability | `/FUNCTIONPADMIN /OPT:NOICF` on the `dusklight` link (ARM64 variant `/FUNCTIONPADMIN:16` kept). Gated on `DUSK_ENABLE_CODE_MODS` here — main applies them unconditionally |

On x86-64 funchook picks its bundled **distorm** disassembler, so capstone is never cloned on
this host; `PatchCapstone.cmake` is present for the platforms that do use it.

### 7.8.2 G4b — export machinery: DONE (it *is* required)

Reading `svc/hook.cpp` settles the question: `hook_resolve` and every by-name/virtual-member
path go through `manifest::resolve`, which reads the `.symdbh` section `symgen manifest --embed`
appends post-link. Without it, `DEFINE_HOOK_SYMBOL` and virtual-member hooks return
`no symbol manifest for this build`, and `manifest::has_inline_sites` can never warn. On Windows
a `FEATURES game` mod additionally links against the import library `setup_windows_exports`
generates. So G4b is not mod-build-only polish — omitting it leaves the hook service half-blind.

Copied from main: `cmake/SymbolManifest.cmake`, `cmake/WindowsExports.cmake`,
`cmake/GameABIConfig.cmake`, `cmake/DetectVersion.cmake`. `symgen` 1.3.1 is downloaded by
`ensure_symgen` at configure time (pinned GitHub release).

`CMakeLists.txt` now runs, under `DUSK_ENABLE_CODE_MODS`, `setup_windows_exports(dusklight)` then
`setup_symbol_manifest(dusklight)`. Post-link result:

```
INFO 30152 exports (2449 data, 277 forwarded) from 1455 objects
```

- `dusklight_exports.def` — 1,346,453 bytes
- `dusklight_imports.lib` — 9,459,806 bytes (what mods link against)
- `.symdbh` present in `dusklight.exe`
- `dusklight.exe` 25,392,640 → **37,022,720** bytes (export table + manifest + no ICF)

`GameABIConfig.cmake` needed two ALBT-local deltas, both commented in the file:

- `TARGET_PC_NATIVE_UI=1` added to the mod-visible define set — `d_albw_shop.h`,
  `d_albw_rental.h`, `d_albw_ui_text.h`, `d_albw_dialogue.h` are gated entirely on it, so a mod
  compiled without it sees a different set of declarations than the host.
- `${_game_root}/src` and `${_game_root}/libs` added to the ABI include dirs. `src` is **not** on
  main's ABI surface; it is here because this tree still keeps its dusk headers under
  `include/dusk` (main moved them to `src/dusk`) and some of those public headers include
  src-only ones (`include/dusk/settings.h` → `dusk/ui/controls.hpp`). Dropping `src` again means
  relocating those headers first.

`cmake/ModSDK.cmake` is deliberately **not** included by the host `CMakeLists.txt` — there are no
in-tree mods here, and including it would pull `GameABIConfig`'s `dusklight_game_headers` into a
build that already sets `GAME_INCLUDE_DIRS`/`GAME_COMPILE_DEFS` its own way.

### 7.8.3 G4c — `DUSK_GAME_DATA`: partial (new, named gap)

`albw-meter` compiled clean against this tree's headers but failed to link on two *data*
symbols. Windows resolves exported data only through `__declspec(dllimport)`, and this tree had
no `DUSK_GAME_DATA` macro at all — main added it in its own 1560-file annotation sweep (#2214).

Done here: the macro pair (`DUSK_GAME_DATA` / `DUSK_GAME_EXTERN`) copied into `include/global.h`,
and annotations applied only where a mod already needs them —
`g_dComIfG_gameInfo` (`d_com_inf_game.h`) and the five `dMeter_*HIO_c` globals (`d_meter_HIO.h`).
**Every other data declaration in this tree is still unannotated**; the next mod that touches new
game data hits the same `LNK2001` and needs its declaration annotated. Annotating is a no-op for
the host build (the macro is empty when `DUSK_BUILDING_GAME` is defined).

Two ALBT headers on the mod ABI path also included Tracy unconditionally
(`include/dusk/gx_helper.h`, `include/d/d_com_inf_game.h`); both now use main's
`#if defined(DUSK_BUILDING_GAME)` guard with `ZoneScoped`/`ZoneScopedN` no-op fallbacks.

### 7.8.4 G7 — `aurora::gfx`: PORTED, not force-bumped

Histories had diverged, so main's SHA was **not** applied blindly:

| Ref | Meaning |
|---|---|
| `509021d` | merge base ("Enable pipeline thread on OpenGL (ES)") |
| `93a1cda` | old ALBT gitlink — merge base **+1** ALBT-only commit ("Guard unbound TEV texmap sampling", WW itemmdl cel `tex255` FATAL fix) |
| `81f12f3` | main's gitlink — merge base **+14**, including `Public aurora::gfx API`, `Add "virtual" texture replacement API`, `Add API for checking if textures have an available replacement`, `Move depth correction to projection matrix`, `Migrate remaining GX functions to FIFO` |

ALBT was **1 commit ahead**, so the port was a rebase, not a merge or a force-bump:

1. The 5 uncommitted ALBT aurora files (№46 F1 TEV-alpha compare ops / F2 skip-draw instead of
   FATAL) were committed on a scratch branch on top of `93a1cda`.
2. `git rebase --onto 81f12f3 509021d` replayed both commits — **no conflicts**.
3. The result was checked back out as working-tree changes on top of `81f12f3`, matching how both
   this tree and `dusklight-main` already carry aurora patches (gitlink = upstream SHA, tree dirty).

State now: gitlink `81f12f31d23ec822d8bde2031c91e94c470911eb`, working tree modifies
`lib/gfx/pipeline_cache.cpp`, `lib/gx/pipeline.cpp`, `lib/gx/shader.cpp`, `lib/gx/shader_info.cpp`,
`lib/webgpu/gpu.cpp`. Provenance branch `albt/gfx-port` (tip `5c892b0`) keeps the rebased commits.
**No ALBT aurora fix was lost** — the WGSL/TEV/skip-draw set is byte-identical in effect; the only
cosmetic loss is nothing, the `№46` comments came through.

Pre-bump API checks that mattered:

- `GXCallDisplayListLE` / `GXCallDisplayListNative` were **deleted** upstream. Nothing outside
  `extern/aurora` referenced either; plain `GXCallDisplayList` is unchanged.
- Dawn/SDL3/nod pinned versions are **identical** across the two SHAs (`352138a` only moved them
  into `AuroraDependencyVersions.cmake`), so no dependency re-fetch and no Dawn rebuild.
- `AuroraDependencyVersions.cmake` arriving is a bonus: `ModSDK.cmake`'s
  `_mod_add_webgpu_headers` (`FEATURES webgpu`) includes it directly and would have failed before.

Host wiring for the services:

- `files.cmake` appends `svc/gfx.cpp` + `svc/texture.cpp` under `if (DUSK_MODS_AURORA_GFX)`, after
  the sorted list, so `DUSK_MODS_AURORA_GFX=OFF` still builds.
- `svc/registry.{hpp,cpp}` `g_gfxModule` / `g_textureModule` guards now evaluate true;
  `svc/camera.cpp` reads the real `aurora::gfx::uses_reversed_z()` instead of the hardcoded `true`.
- `src/m_Do/m_Do_graphic.cpp` gained main's five `gfx_run_stage` call sites
  (`SCENE_BEGIN`, `SCENE_AFTER_TERRAIN`, `SCENE_AFTER_OPAQUE`, `FRAME_BEFORE_HUD`,
  `FRAME_AFTER_HUD`) plus `#include "dusk/gfx.hpp"`, ported from main's `d9a978f21f`. Guards are
  `#if TARGET_PC && DUSK_MODS_AURORA_GFX` (main's are `#if TARGET_PC`) so the OFF path links.
  **Without these the gfx service registers but never runs a stage** — mods draw nothing.

### 7.8.5 One real conflict from the bump: `OSInitAlloc`

First link failed with `LNK2005: OSInitAlloc already defined in stubs.cpp.obj`
(vs `aurora_os.lib(OSAlloc.cpp.obj)`). Not caused by the bump itself: the duplicate has been
latent, and `/DEF:` is what surfaced it — the generated export list names aurora's `OSInitAlloc`,
so the linker now pulls that object in.

Fix is upstream's own: main deleted the stub in `e9e16a8ac1` "Remove redundant OSInitAlloc". The
same eight lines are gone from `src/dusk/stubs.cpp` here. ALBT's stub was a no-op returning
`arenaStart`; aurora's real implementation initialises the heap-descriptor array that
`JKRHeap.cpp` asks for, which is what main has been running.

### 7.8.6 `albw-meter` now builds a loadable `.dusk` against this tree

Proof that G4/G4b actually work end to end (`tools/_g4g7_sdk_smoke.bat`):

```
cmake -S ...\albw-meter -B %TEMP%\albw-meter-albt-sdk -G Ninja
  -DDUSKLIGHT_DIR=...\dusklight
  -DDUSK_GAME_EXE=...\build\windows-msvc-relwithdebinfo\dusklight_imports.lib
→ configure 0, build 0 → albw_meter.dll + mods\albw_meter.dusk
```

**Caveat, not a blocker:** `albw-meter/CMakeLists.txt` defaults `DUSKLIGHT_DIR` to
`dusklight-main` and says *"Never point at the ALBT fork."* That is the mod's own policy — its
shipping target is stock main. Building it against this tree is a **local host test**, and the
`.dusk` it produces is keyed to this build (`symgen` manifest build id + this exe's export set).
A main-built `albw_meter.dusk` dropped onto this host is *not* interchangeable: import names that
diverge fail to resolve, and shared-struct layout differences would corrupt silently.

### 7.8.7 Build + factory check

`cmd /c build_run.bat` → **exit 0** (full 1447-target rebuild; `GAME_COMPILE_DEFS` and
`include/global.h` both changed). CMake re-ran exactly once, from the intentional reconfigure.

- `build.ninja`: **2246** of 2264 `FLAGS =` lines carry `/O2` (the 18 without are non-C/C++ rules) ✅
- `CMAKE_CXX_FLAGS_RELWITHDEBINFO` = `/O2 /Ob1 /DNDEBUG`; `CMAKE_C_FLAGS_RELWITHDEBINFO` identical ✅
- `DUSK_ENABLE_CODE_MODS=ON`, `DUSK_MODS_AURORA_GFX=ON`, `DUSK_HAS_FUNCHOOK=ON`,
  `DUSK_HAS_PREPATCH=OFF` in the cache ✅
- `dawn_cache.db*` + `pipeline_cache.db*` clean ✅ (`tools/_g4g7_wipecache.bat`)

Factory healthy. **FPS untrusted until one field measure**, and this pass has more reason to
change it than the last two: `/OPT:NOICF` blocks identical-code folding, `/FUNCTIONPADMIN` pads
every function entry, the exe carries 30k exports, and aurora moved 14 commits (including
`Move depth correction to projection matrix` and `Migrate remaining GX functions to FIFO`).
If FPS drops, **A/B `DUSK_ENABLE_CODE_MODS=OFF` first** — that alone removes the link flags, the
export table and the manifest embed — before suspecting the aurora bump or any WIP feature.

### 7.8.8 Helper scripts

`tools/_g4g7_configure.bat` (the one intentional reconfigure), `tools/_g4g7_o2check.bat`
(`/O2` + cache proof), `tools/_g4g7_wipecache.bat`, `tools/_g4g7_sdk_smoke.bat` (out-of-tree mod
build against this host). None are on the normal build path — `build_run.bat` is still it.

## 7.9 First runtime smoke: launch crash at `Initializing mods...` — FIXED

Loading `albw_meter.dusk` killed the process immediately after
`symbol manifest loaded: 280559 symbols`, before any `found '…'` line.
Reason `EXCEPTION (0xe06d7363)` = an unhandled MSVC C++ throw.

### 7.9.1 Symbolized stack

No cdb/LLVM on PATH; `llvm-symbolizer` ships inside VS
(`…\VC\Tools\Llvm\x64\bin`) and reads MSVC PDBs directly:

```
llvm-symbolizer --obj=dusklight.exe --relative-address --inlines <rva>
```

| Frame | RVA | Resolves to |
|-------|-----|-------------|
| #03 | `0xc960c0` | `ModBundleZip::getFileNames` — `bundle_zip.cpp:38` (the `lock_guard`) |
| #04 | `0xcba918` | `locate_native_runtime` — `loader.cpp:152` |
| #05 | `0xcba613` | `ModLoader::load_native_if_present` |
| #06 | `0xcbf663` | `ModLoader::try_load_mod` |
| #07 | `0xcb7cca` | `ModLoader::init` |
| #08 | `0xaefc0`  | `game_main` — `m_Do_main.cpp:862` |

Frames #00–#02 are `RaiseException` ← `_CxxThrowException` ← MSVCP140. Disassembling the call
site pins the exact throw: `mov ecx, 5; call <MSVCP140 thunk>` → `_Throw_Cpp_error(5)` =
`_OPERATION_NOT_PERMITTED`, which `std::mutex::lock()` raises only when `_Mtx_lock` returns
`_Thrd_busy` — i.e. **this thread already holds that mutex**.

### 7.9.2 Root cause: no `/EHsc`, so a throw leaks the bundle lock

`build.ninja` CXX `FLAGS` are `/O2 /Ob1 /DNDEBUG -std:c++20 -MD -Zi /bigobj /MP /FS /W0 /utf-8`.
There is **no `/EHsc` anywhere in the tree** (`CMakeLists.txt` never sets it), so MSVC compiles
without unwind semantics: `throw`/`catch` still work, but **destructors of automatic objects are
not run while an exception unwinds**. `/W0` hides the compiler's own
`warning C4530: C++ exception handler used, but unwind semantics are not enabled`.

`ModBundleZip` guards its miniz handle with a non-recursive `std::mutex`, and two of its methods
used to throw from inside the `lock_guard`'s scope. The sequence:

1. `try_load_mod` → `load_metadata` → `resolve_image_path` → `bundle_has_file("res/icon.png")`
   → `ModBundleZip::getFileSize` takes `m_mutex`, then throws because `albw_meter.dusk`
   contains no `res/` files at all (entries are `mod.json`, `lib/windows-amd64/mod.dll`,
   plus empty `res/`, `overlay/`, `textures/` directory records).
2. No unwind semantics → the `lock_guard` destructor never runs → **`m_mutex` stays locked**
   on the game thread. `bundle_has_file` catches and returns false, so nothing looks wrong.
3. `res/banner.png` repeats it. That second lock already fails, but `std::system_error` derives
   from `std::runtime_error`, so `bundle_has_file`'s `catch` swallows it too — still silent.
4. `load_native_if_present` → `locate_native_runtime` → `getFileNames()` locks the same mutex
   → `_Thrd_busy` → `std::system_error`. Nothing on that path catches it → unhandled → crash.

A mod that happens to ship `res/icon.png` never trips this, and directory-mounted mods use
`ModBundleDisk`, which has no mutex — which is why the bundled in-place mods were fine.

### 7.9.3 Fix

- **`src/dusk/mods/loader/bundle_zip.cpp`** — `readFile` and `getFileSize` now scope the
  `lock_guard` to the miniz calls only and throw *after* releasing `m_mutex`. A comment records
  why this TU may not rely on unwinding.
- **`src/dusk/mods/loader/loader.cpp`** — `try_load_mod` wraps `load_native_if_present` in
  `try`/`catch`, routing a throw to `fail_mod` so a bad mod fails alone. Extraction, `LoadLibrary`
  and the DLL's static initialisers all run under it.
- **`src/m_Do/m_Do_main.cpp`** — added main's missing
  `ModLoader::instance().set_cache_dir(dusk::CachePath / "mod_cache")` before `init()`. Without it
  `init()` fell back to `<mods>/.cache` and `remove_all`-ed a directory inside the user's mods
  folder every launch.

### 7.9.4 Verified

Incremental `cmd /c build_run.bat` → exit 0; 2246 `/O2` lines in `build.ninja`; dawn + pipeline
caches wiped. Both A/B launches reach gameplay:

| Launch | Result |
|--------|--------|
| `--mods <empty dir>` | `no mods found` — clean, no crash |
| default (`albw_meter.dusk` present) | `found 'ALBW Meter' v0.3.0 by ALBT` → `1/1 mod(s) active` → in-game |

### 7.9.5 Open follow-ups

- **`albw-meter` disables itself in gameplay.** After `fpcBs_Create … fpcNm_METER2_e`, three
  `failed: Unknown exception in post hook callback (1)` then `0/1 mod(s) active`. That string is
  **not** in the host or the SDK — it comes from the mod's own DLL, so this is a separate defect
  in `albw-meter` (or its hook contract), not the launch crash. Same no-`/EHsc` trap is a prime
  suspect if the mod is built with the host's flags.
- **`/EHsc` is absent tree-wide.** Every `throw` past an RAII guard anywhere in this tree leaks
  whatever that guard owns. Adding `/EHsc` is the real class-level fix but it changes codegen for
  all ~1450 objects, needs a reconfigure + full rebuild, and puts FPS back in untrusted state —
  **user decision, not an agent's.** Until then: in new code, never throw with a lock or other
  guard live.

---

## 7.10 United Mods page (2026-07-25, fourth pass) — G1 closed additively

Conflict answer #1 is now built. **No new translation unit, no `files.cmake` / CMake edit, no
reconfigure** — main's `mods_window.*` was again not imported; ALBT's own `ModsWindow`
(`src/dusk/ui/mods.{hpp,cpp}`) grew the `.dusk` half instead.

One tab, one left list, one detail pane, three sections top to bottom:

| Section | Half | Contents |
|---|---|---|
| `Load Order — top wins` | ALBT Layer B (unchanged) | position-bound rows, grab-and-place reorder, collections/variants, conflict badges, `modinfo.ini` + `modshot://` screenshot |
| `Packages (.dusk)` | ModLoader (new) | one row per `ModLoader::instance().mods()` entry — `Name vX — Active/Failed/Suspended/Disabled`. Detail pane: status + version/author byline, **Enable** / **Disable** / **Reload** (Reload hidden for `inPlace` mods), failure `Reason`, `Waiting on` providers when suspended, "Disabling or reloading also restarts: …" dependents note, description, `ui_build_mods_panels` mod-owned controls, **Recent log** (last 12 of the mod's `mods::log` lines) + **Copy Full Log**. Empty state: "No .dusk packages in the mods folder." |
| `Options` | ALBT (unchanged) | `Allow Core Override` |

Mechanics worth knowing:

- **Snapshot rebuild.** `ModsWindow::update()` diffs `active` / `loadFailed` / cvar-enabled /
  `suspendedByProvider` / `cacheGeneration` per package (main's pattern) and calls
  `refresh_active_tab()` on a change — enable/disable/reload land on `ModLoader::tick`, a frame or
  more after the button press. The rebuild empties the detail pane, so the row the user was on is
  re-focused, which re-runs its `register_control` callback and repopulates it.
- **`ui_update_mods_panels` is only pumped while the detail pane actually holds that mod's
  panels.** `mSelectedPackage` is cleared by the load-order and Options callbacks (they repurpose
  the same pane), otherwise a mod's update callback would keep writing to element handles whose
  elements were destroyed and log a stale-handle error every frame.
- Rows/buttons are ALBT `ControlledButton` / `Pane::add_button`, so the package half inherits the
  load-order half's look and pad navigation with no new RCSS.

### 7.10.1 Deliberate leftovers

- **No mod icons or banner decorator.** Main's `mod_texture_provider.*` + `res/rml/mods.rcss`
  were not imported; rows are text. (ALBT's own `modshot://` provider stays for `modinfo.ini`
  screenshots on the load-order half.)
- **No `LogsWindow`.** Main's is a separate document built on its `Window::Props` /
  `set_content` API and `res/rml/logs.rcss`, neither of which exists here; it would also need a
  new TU (⇒ `files.cmake` ⇒ reconfigure). The in-pane "Recent log" tail + clipboard copy covers
  the failure-diagnosis case. A full log viewer is a follow-up, and would want the level filter
  and virtualized line window main has.
- Package rows carry no "current" marker beyond focus, and there is no package search/filter.

### 7.10.2 Build + factory check

`cmd /c build_run.bat` → **exit 0**, 4 targets (`mods.cpp`, `menu_bar.cpp`, `loader.cpp`, link).
No `Re-running CMake`.

- `build.ninja`: 2247 `FLAGS =` lines carry `/O2` ✅
- `CMAKE_CXX_FLAGS_RELWITHDEBINFO` = `/O2 /Ob1 /DNDEBUG` ✅
- `dawn_cache.db*` + `pipeline_cache.db*` wiped ✅

Compile-green only — **not yet field-tested with a `.dusk` present.**
