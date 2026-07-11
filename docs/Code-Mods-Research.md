# Code Mods — Research & Design Direction

**Status (2026-07-11): RESEARCH ONLY — nothing built.** The declared end goal of the
mod system: user-installable mods that add *behavior*, not just assets. This doc maps
the engine's real extension points, compares delivery models, and proposes a phased
design. Cross-references [Mod-Load-Order-Design.md](Mod-Load-Order-Design.md) (the
asset/load-order foundation, §1–17) and [level-editor-phase1.md](level-editor-phase1.md)
(the placement tooling this synthesizes with).

---

## 1. What "code mod" means (a spectrum, not a switch)

| Tier | Example | Turing-complete? | Risk |
|---|---|---|---|
| **Data mods** | New shop rows, tweaked item stats, new stage layouts | No | Low |
| **Script mods** | New enemy behavior, quest logic, HUD widgets | Yes (sandboxed VM) | Medium |
| **Native plugins** | New rendering passes, engine surgery | Yes (arbitrary machine code) | Maximum |

Most of what modders actually want lives in the first two tiers. Every successful
ecosystem confirms this: Factorio and RE-Framework run Lua; Minecraft's power comes
from a managed VM; Skyrim's SKSE native plugins exist but 95% of mods are data +
Papyrus script. **Recommendation: build tiers 1–2; treat tier 3 as "maybe never."**

---

## 2. What Dusklight already has (the foundation audit)

The load-order system built this month covers the *delivery* half of code mods
completely:

- **Mod identity + ordering + UI** — `dusk::custom_assets` sources, the Mods window,
  grab-and-place, conflict badges, `modinfo.ini` (with `Requirement` / `AddonFor` /
  `NameAsBundle` already reserved).
- **Asset overrides** — any disc file (Layer A), loose models (Layer B), icons,
  textures, audio. A code mod's *assets* need zero new work.
- **Level editor (Phase 1a/1b)** — `dusk::leveledit` can already enumerate every
  placed actor in a room from the stage chunk buffers
  ([enumerate.hpp](../include/dusk/leveledit/enumerate.hpp): `PlacedActor` with chunk
  tag, 8-char name, proc id, params, transform, layer, room), select in-world,
  fly-cam. It is read-only today — but its data model is exactly the unit a "level
  mod" edits.

What the engine offers as behavioral seams (verified in source):

1. **The actor name table** — `l_objectName[]` in [d_stage.cpp:508](../src/d/d_stage.cpp:508)
   maps every 8-char stage-placeable name to a proc id + subtype, resolved through
   ONE function, `dStage_searchName()` ([d_stage.cpp:1526](../src/d/d_stage.cpp:1526)).
   A mod-registered side table checked before (or after) the static array = new
   placeable actor names, ~10 lines of hook.
2. **The process profile list** — `g_fpcPfLst_ProfileList[]` indexed by proc id via
   `fpcPf_Get()` ([f_pc_profile.cpp](../src/f_pc/f_pc_profile.cpp)). Profiles are
   plain structs of lifecycle function pointers (create/delete/execute/draw) + actor
   size + priority. Reserving a dynamic id range above the static list = runtime
   actor types.
3. **Per-frame hook precedent** — `dScnPly_Draw` already dispatches Dusklight systems
   (`dAlbwMenuRes_drive`, wolf-stun hooks) each play-scene frame; a script event pump
   slots in identically.
4. **Game-state surface the ALBW work already wraps** — save event bits
   (`dComIfGs_isEventBit`/`onEventBit`, e.g. the outfit-worn bit 700), items
   (`dComIfGs_getItem`, get-item flows), shop tables (`dALBWVisibleEntry` with
   `customIconName` — a data-mod hook that exists TODAY), dialogue
   ([workflow doc](../docs)), SE/BGM cues, HUD via J2D + named icon slots, settings
   ConfigVars.
5. **Logging/diagnostics** — DuskLog; the Mods panel for surfacing per-mod status.

Missing entirely: any script VM, any dynamic loading, any stable API boundary.

---

## 3. Delivery models compared

### A. Data-driven extensions (no VM)
Declarative files in the mod folder feeding existing table-driven systems.
- **Already true for:** stage `.arc` replacement (level layouts ship via Layer A
  *today* — a repacked DZR is just a file). Icons, audio, textures, models.
- **Cheap to add:** shop rows (`shop.json` → `dALBWVisibleEntry` list), item stat
  tweaks, spawn-table entries, text/dialogue strings.
- Limits: no new logic; every extension point is bespoke C++ work.
- **Verdict: do continuously — each one is small and immediately useful.**

### B. Embedded scripting VM (the real code-mod tier)
A sandboxed interpreter runs mod scripts against a curated API.
- **Language: Lua 5.4** (plain, not LuaJIT). Rationale: tiny (~300 KB, zero deps,
  compiles into the exe like our other vendored libs), the de-facto game-mod
  standard (modder familiarity), trivially sandboxable (build the environment table
  by hand; never expose `io`/`os`/`require`/`load` of native chunks), built-in
  instruction-count hooks for CPU budgeting (`lua_sethook` + `LUA_MASKCOUNT`).
  LuaJIT is faster but: larger, x64-assembly-heavy (portability + merge posture),
  and its 5.1 dialect splits the ecosystem. Plain 5.4 is fast enough for game-logic
  scripting at our scale; hot loops belong in C++ anyway (see §7 FPS discipline).
  Alternatives rejected: QuickJS (bigger API surface to sandbox, GC pauses),
  AngelScript (smaller community), WASM (great sandbox, terrible authoring UX for
  this audience today).
- **Verdict: the core recommendation.** Details in §4–6.

### C. Native plugin DLLs
`<mod>/plugin.dll` loaded at boot, calling exported engine hooks.
- Maximum power; zero sandbox — a plugin is arbitrary code with the player's user
  account. Also: ABI stability burden on every refactor, Windows-only until proven
  otherwise, antivirus friction, and it forks the community into "requires script
  extender" territory.
- **Verdict: defer indefinitely.** If ever built: separate explicit opt-in per mod
  with a scare-warning (the D4 pattern), signature/hash pinning, and no promise of
  API stability. Nothing in tiers A/B requires it.

---

## 4. Proposed architecture — "DuskScript" (tier B)

One new module, `dusk::script` (mirroring `dusk::custom_assets`'s merge posture:
self-contained, kill-switched, one namespace).

### 4.1 Lifecycle
- A mod opts in by shipping `scripts/main.lua`. The load order's `Source` walk
  already finds mod folders; a `scripts/` flag joins `folder_content()`.
- **One `lua_State` per mod** (not shared): crash/energy isolation, per-mod memory
  accounting via a custom allocator, per-mod instruction budgets, and unload =
  `lua_close`. Cross-mod interference becomes structurally impossible; deliberate
  cross-mod APIs can come later via a brokered channel.
- Scripts load at play-scene entry (not boot — the game state they query must
  exist), in **load order, bottom-to-top, so the TOP mod's hooks register last and
  its writes land last = top wins conflicts**, consistent with the asset rule.
- Toggle OFF (Mods window) = close the state; ON = fresh load. Reload-scoped like
  everything else; a `/reload` debug action for modder iteration.

### 4.2 Event bus (how mods hook the game)
C++ side keeps a small registry: `script::emit(event, args...)` at curated engine
points; Lua side `dusk.on("event", handler)`. Launch set (each is one `emit` call
at an existing seam):

| Event | Emit point (exists today) |
|---|---|
| `frame` (play scene, post-pause-check) | `dScnPly_Draw` beside `dAlbwMenuRes_drive` |
| `area_load` / `room_change` | stage load path (`dAlbwBoss_onStageLoad` precedent) |
| `save_loaded` | save-load flow (outfit sync precedent) |
| `item_get` | get-item flow (WW itemmdl hooks precedent) |
| `enemy_killed` | `enemyDeathRupees` hook point |
| `damage_taken` / `damage_dealt` | combat refinement hooks |
| `dialogue_choice` | ALBW dialogue module |
| `mod_toggled` (self) | custom_assets toggle path |

`frame` handlers are the FPS hazard — see §7.

### 4.3 API surface v0 (curated, versioned)
Everything under one `dusk` table; `dusk.api_version = 1`; `modinfo.ini` gains
`apiversion=` checked at load (mismatch → mod stays off + panel badge).

- **Query:** `link.pos()/health()/form()`, `flags.event(bit)`, `save.item_count(id)`,
  `stage.name()/room()`, `time.of_day()`.
- **Act:** `actors.spawn(name, pos, rot, params)` (via `fopAcM_create`),
  `items.give(id)`, `flags.set_event(bit)` (restricted range — see §8),
  `audio.se(id)` / `audio.bgm(id)`, `hud.text(...)` / `hud.icon(named_slot, ...)`
  (reuses the named-icon system), `dialogue.show(...)` (the NPC dialogue workflow).
- **Storage:** `store.get/set(key, value)` — persisted as JSON per (save slot ×
  mod), in AppData next to the save, **never inside the TP save format** (save
  compat is untouchable; a removed mod leaves vanilla saves pristine).
- **Meta:** `dusk.log(...)` (→ DuskLog, tagged per mod), `dusk.defer(fn, frames)`.

### 4.4 Script actors — the level-editor synthesis (the crown jewel)
One generic C++ actor, `d_a_script`, registered once with a reserved proc id. Its
lifecycle methods trampoline into the owning mod's Lua handlers
(`create/execute/draw2D/delete`), and it exposes model binding through Layer-B
loose BMDs / existing arcs. Mods declare placements:

```lua
dusk.actors.define("MKEESE01", {           -- 8-char stage name, mod-namespaced
  on_create = function(self, params) ... end,
  on_execute = function(self) ... end,     -- budgeted like frame handlers
})
```

`dStage_searchName()` gains a mod side-table lookup → "MKEESE01" resolves to the
script-actor profile with the mod+definition packed into the actor's params. **Then
the level editor places it**: the editor's future write path (Phase 2+ of that
project) emits stage actor placements; a placement naming a script actor = modder
content with custom behavior, positioned visually in-world. Editor for WHERE,
scripts for WHAT, the asset system for LOOKS — that's the complete loop, and each
third already exists or is on its roadmap.

Level-editor placements ship as either (a) repacked stage arcs via Layer A (works
today, whole-file = conflict-prone between two level mods touching one stage), or
(b) later, an additive `placements.json` the stage loader merges after DZR parse —
mergeable across mods, ordered by load order. (b) is the better end state and is
ALSO what the level editor should export.

---

## 5. Load-order integration

- `scripts/` becomes recognized content (`folder_content`), so script-only mods
  list, order, toggle, and badge like everything else.
- Hook ordering: bottom-to-top registration (top mod wins — §4.1).
- `modinfo.ini` `Requirement=` finally gets teeth: unmet requirement → mod stays
  disabled with a panel explanation. `AddonFor`/`NameAsBundle` group script addons
  under their master (collection sub-page work).
- Conflict detection extension (later): two mods defining the same actor name or
  hooking the same exclusive seam → badge, top wins.

## 6. Security model

- Sandbox: hand-built environment table; no `io`/`os`/`package`/`debug`; `require`
  resolves ONLY inside the mod's own `scripts/`; no bytecode loading (text chunks
  only — precompiled Lua bypasses type checks and is a classic escape).
- File access: none directly; assets go through the existing systems.
- CPU: per-mod instruction budget per frame via `lua_sethook` count hooks; over
  budget → handler suspended for the frame + counted; repeat offenders auto-disabled
  with a Mods-panel badge ("suspended: over frame budget"), never a crash.
- Memory: custom `lua_Alloc` with a per-mod cap (e.g. 16 MB default).
- This is a *courtesy* sandbox against accidents and casual abuse, not a hostile-code
  boundary — Lua-in-C has a long CVE tail. The honest posture, documented for users:
  script mods are far safer than native plugins, not mathematically safe. (Same
  stance every Lua-modded game takes.)

## 7. Performance discipline (the 139–144 FPS mandate)

- Event-driven over polling: most mods should never register `frame` at all.
- `frame` handlers run under the instruction budget; budget sized so the WHOLE
  script tier fits inside ~0.2 ms worst case at 144 fps.
- GC: `lua_gc(LUA_GCSTEP)` with a fixed step per frame per state; no full collects
  mid-play.
- Zero script work when no script mods are enabled (one boolean check in the pump —
  the `dAlbwMenuRes_drive` idle pattern).
- Validation: field `F_SP121` r0 p0 oracle run with a busy-loop dummy mod to prove
  the watchdog holds the floor.

## 8. Save & compat rules

- Mod storage lives OUTSIDE the save file (§4.3). Removing a mod = vanilla save.
- `flags.set_event` restricted to a documented reserved bit range (coordinate with
  the ALBW allocations, e.g. the 700-range precedent) or virtualized entirely into
  mod storage; scripts must not brick progression flags.
- API is semver'd from day one; `apiversion` gate at load.

## 9. Dummy mods for the future test phase

1. **hello-dusk** — `scripts/main.lua`: log on load, HUD text on `save_loaded`.
   Proves: lifecycle, log, HUD, toggle on/off.
2. **rupee-rain** — `frame` handler watching a D-pad combo → `actors.spawn` rupees
   above Link. Proves: input-ish query, spawn API, budget accounting.
3. **patrol-keese** — script actor `MKEESE01` with a patrol brain + Layer-B model,
   placed in Ordon via the level editor. Proves: the §4.4 loop end-to-end.
4. **busy-loop** — deliberately hostile `while true` in `frame`. Proves: watchdog
   suspends it, game holds 144, panel badges it.
5. **shop-extender** (tier A, no VM) — `shop.json` adding a row with a named icon.
   Proves: data-mod path + modinfo + icons together.

## 10. Effort sketch & sequencing

| Step | Scope | Rough size |
|---|---|---|
| A1 | Additive placement merge (`placements.json`) + level-editor export format | medium — pairs with editor Phase 2 |
| A2 | Shop/data extension files | small each |
| B1 | Vendored Lua 5.4 + `dusk::script` states/sandbox/budgets + `frame`/`area_load`/`save_loaded` + log/query API | the big lift |
| B2 | Act APIs (spawn/give/flags/audio/hud) + storage | medium |
| B3 | Script actors + name-table hook | medium |
| B4 | Requirement/apiversion enforcement + panel badges | small |
| C | Native plugins | not planned |

Sequencing note: A1 is valuable *now* (it is also the level editor's save format),
and B1 is where "code mods exist" becomes true.

## 11. Open questions (decide before B1)

- Input API: raw pad reads are a footgun (conflicts with bindings/quick-swap);
  probably expose *named actions* registered through the existing bindings UI.
- Draw API ceiling: HUD text/icons v0; full custom RmlUi documents per mod later?
- Cross-mod calls: none in v1 (isolation first); brokered export tables later.
- Netplay/determinism: if co-op ever lands, `frame` handlers become a sync hazard —
  keep the event surface deterministic-friendly (no wall-clock, seeded RNG API).
- Where does the script pump live relative to pause (menus open) — probably emit
  `frame` only when actors execute, plus a separate `ui_frame` if ever needed.
