# In-Game Level Editor — Phase 1 Design & Handoff

**Live tip / Interconnected Run:** [state/level-editor.md](state/level-editor.md) ·
[Interconnected Chats/Level-Editor-Cursor-Claude.md](Interconnected%20Chats/Level-Editor-Cursor-Claude.md)
(protocol: [INTERCONNECTED-RUNS.md](Interconnected%20Chats/INTERCONNECTED-RUNS.md)).
This file remains the **canonical design archive** — keep all sections for revisions.

**Status:** Research / design complete. **§10 (1x) is implemented + playtested
(2026-07-09)** — the older “no code yet” line below is historical for pre-1x
context; trust §10 + live state + git for current code status.
**Scope:** A real, in-game Twilight Princess level editor built on dusklight's own
foundations (native renderer, actor DB, collision, the Aurora file-overlay). Phase 1
= a **read-mostly stage inspector with live, non-persisted edits**, entered as a
separate mode from Dusklight's launch menu. Terrain (DZB) editing is an explicit
**later tier**, kept in view so Phase 1 doesn't paint us into a corner.
**Caveat:** All line numbers are point-in-time and WILL drift — re-verify against
current source before editing.

**Always read first (per [build-fps-guidelines.md](build-fps-guidelines.md)):**
that file + [commit-and-push.md](commit-and-push.md). Build with `build_run.bat`
(RelWithDebInfo) only; wipe GPU caches after each build; don't commit/push unless
the user asks.

**Related:** [Custom-Model-API-Work.md](Custom-Model-API-Work.md) (the Aurora
overlay that makes edits shippable), [heros-shade-secret-boss.md](heros-shade-secret-boss.md)
(the runtime-`fopAcM_create` hack pattern this ultimately supersedes; §3b documents
the old "can't ship stage edits" blocker that the overlay removed),
[TrueALBWWorld.md](TrueALBWWorld.md) (the save-stamp / mode-gating precedent).

---

## 0. Why this is feasible now (one paragraph)

The historical blocker — recorded verbatim in
[heros-shade-secret-boss.md:48](heros-shade-secret-boss.md) — was that "editing a
stage file does NOT reach players" because Dusklight ships as **code** and loads all
data from the player's own disc at runtime. That is **no longer true**: the Custom
Model API's **Aurora virtual-FST overlay** ([Custom-Model-API-Work.md:135](Custom-Model-API-Work.md))
lets a loose file dropped in `model_replacements/<mod>/files/…` transparently replace
any disc file, including whole arcs. So an edited stage arc can ship. Combined with
the fact that dusklight already **is** the parser, renderer, collision engine, and the
authoritative actor-parameter database, an in-game editor is a far stronger foundation
than any external Zelda editor. Phase 1 deliberately stops **before** the arc writer:
edits are live and reversible, needing no new engine subsystem.

---

## 1. Verdict per pillar (from the research pass)

| Pillar | Finding | Difficulty |
|---|---|---|
| **Enumerate placed actors** (read) | Parsed DZS/DZR stays resident; re-walk the buffer with the loader's own tag scan. Name→type via `l_objectName[]` (918 entries). Placement↔live join on `(roomNo, setID)`. | Low, one gap (no `getActor()` accessor → re-walk) |
| **Editor UI + gizmo** | RmlUi component model, warp, ImGui dev menu, **3D debug-draw API**, world→screen projection, and mouse/click are all present and shippable behind `enableAdvancedSettings`. | Substrate done; selection/gizmo **domain logic** is new |
| **Bake / round-trip** (write) | Overlay delivery is free; **no RARC writer or YAZ0 encoder exists** — must be built (offline; can emit *uncompressed* arcs for v1). Unit of replacement = whole `.arc`. | Moderate, isolated — **deferred out of Phase 1** |
| **Terrain (DZB)** | Visual model and DZB collision are **independent assets**. Add-terrain via MoveBG objects is feasible today; full DZB authoring needs a new octree-aware exporter. | Tiered: (a) add easy, (b) in-place moderate, (c) regenerate hard |

Evidence lives in §7 (code map). The rest of this doc specs Phase 1.

---

## 2. Entry point & the security throughline (user-directed)

**Principle:** the editor is a distinct **launch mode**, opt-in, and invisible/inert on
a normal `Play` boot — nothing in the hot path changes unless you deliberately entered
editor mode. This is the "secure throughline that doesn't break anything."

1. **New backend setting `enableLevelEditor`** — mirror `enableAdvancedSettings`
   ([settings.h:498](../include/dusk/settings.h)); a `ConfigVar<bool>`, default **off**,
   surfaced only inside the Advanced Settings window.
2. **Gated launch-menu button.** The prelaunch menu is a plain `menu-list` of `Button`s
   (`Play` / `Settings` / `Quit`, [prelaunch.cpp:691-732](../src/dusk/ui/prelaunch.cpp)).
   When `enableLevelEditor` is true, push a **`"Level Editor"`** button (between `Play`
   and `Settings`). It launches like `Play` (`IsGameLaunched = true; pop(false)`) but
   also sets the session flag below.
3. **Relaunch as the safety boundary.** The button is evaluated at `Prelaunch`
   construction, so toggling the setting only surfaces it after a fresh boot. The
   existing `is_restart_pending()` → "Apply Options" modal
   ([prelaunch.cpp:655](../src/dusk/ui/prelaunch.cpp), [:775](../src/dusk/ui/prelaunch.cpp))
   can prompt the restart. Result: exactly the "toggle → relaunch → it appears" flow.
   (It *could* be made live by re-checking each frame in `Prelaunch::update()`, but
   restart-gating matches the security intent and reuses existing machinery.)
4. **Editor session flag `dusk::g_levelEditorSession`** — a session-scoped bool, **not**
   persisted, **not** in any save. Master gate for every editor behavior. A normal `Play`
   launch leaves it false. This is the entire non-breaking guarantee: all editor code
   early-outs on `!g_levelEditorSession`, so it never touches the FPS-critical path in
   normal play (per [build-fps-guidelines.md:145](build-fps-guidelines.md) "no per-frame
   … ImGui in hot paths without a toggle").

**Navigation is reuse, not new work:** the debug camera exists (ImGui `Debug Camera`,
[ImGuiMenuTools.cpp](../src/dusk/imgui/ImGuiMenuTools.cpp)) with mouse look/zoom via
[mouse.cpp](../src/dusk/mouse.cpp); the menu pointer gives absolute cursor + click
([menu_pointer.cpp:201](../src/dusk/menu_pointer.cpp)). On an editor session we
auto-enable both so "fly around / zoom / click" is free. Warp
([warp.cpp:307](../src/dusk/ui/warp.cpp), `dComIfGp_setNextStage`) is the "jump to the
room I want to edit" primitive.

---

## 3. The storage model — **Level Projects**, not per-save, not a bare setting

A level edit is **authoring output** (like a `model_replacements` mod), **not** gameplay
state. That distinction settles the storage question the user raised:

- **Per-save is wrong** — a save is one playthrough; an edit is a mod you author once,
  preview across any save, share, and ship. Saves are memory-card-shaped.
- **A single "setting" is the wrong container** — it can't hold a variable-size list of
  per-room edits.
- **Recommended: on-disk "Level Projects,"** mirroring the `model_replacements` layout:

  ```
  %AppData%/TwilitRealm/Dusklight/level_projects/<ProjectName>/
      project.json        ← manifest: edit records keyed by (stageFile, roomNo, layer)
      (Phase 2) baked/…   ← repacked .arc output for shipping
  ```

  `project.json` is human-readable and diffable: a list of records
  `{op: move|param|add|delete, stage, room, layer, setID, name, pos, angle, scale, params}`.

**Keep two axes orthogonal** (this is the half-truth in the user's "maybe a setting toggle?"):

| Axis | What it is | Where it lives |
|---|---|---|
| **Storage** | where edits are kept | Level Project folder on disk (never a save) |
| **Application** | whether/which project is *live* right now | a setting `activeLevelProject = <none / name>` |

- **Editor session:** load a project → edits apply live in-memory for authoring → save to
  `project.json`. Non-destructive to disc and saves.
- **Normal session:** if `activeLevelProject` is set, the game **replays** that project's
  edits on room load (Phase 1 mechanism) or loads its **baked overlay arcs** (Phase 2).
  This is how a finished level reaches a player.

So storage = project files; the setting only chooses which project is active. Storage,
preview, and shipping never tangle. Easily revisitable (just a serialization target), but
starting here avoids a painful migration.

**Phase-1 live application without the arc writer:** even before the Phase-2 baker, a
project can be applied at runtime by replaying its records on room load — `move`/`param`
via live actor mutation (`current.pos`/`shape_angle`/params), `add` via `fopAcM_create`
(the Actor Spawner pattern), `delete` by not-spawning / deleting the live actor keyed by
`setID`. That yields a working live preview with no packer.

---

## 4. Milestone 1a — the Stage Inspector (read-only, the concrete first build)

**Goal:** in an editor session, fly through any room and see + inspect every placed object,
with a live in-world highlight on the selected one. **Zero mutation, zero serialization.**
Independently useful: it supersedes guessing actor params for the runtime-spawn hacks.

**New module** `src/dusk/leveledit/` (namespace `dusk::leveledit`), plus one new tab wired
into the existing `EditorWindow` for fast iteration (reuses the Pane/Button plumbing). The
dedicated editor-mode overlay (§2) can wrap this later; prototyping as a tab de-risks the
enumeration logic first.

### 4.1 Core new function — the buffer re-walk (closes the one research gap)

```cpp
// ============================================================================
// Level Editor — placed-actor enumeration (re-walks the resident DZR/DZS buffer,
// because ACTR/SCOB/TGSC have no dStage_dt_c accessor the way PLYR/SCLS/Door do).
// ============================================================================
struct PlacedActor {
    char        chunkTag[4];   // ACTR / SCOB / TGSC / TGDR / PLYR (+ layer variant ACT0..ACTe)
    char        name[8];
    s16         procname;      // via dStage_searchName(name)
    s8          argument;
    u32         params;        // base.parameters
    Vec         pos;
    csXyz       angle;
    Vec         scale;         // TGSC / SCOB only
    u16         setID;
    s8          layer;
    fopAc_ac_c* live;          // joined via (roomNo, setID), or null
};
std::vector<PlacedActor> enumerate_room_actors(int roomNo);
```

Implementation, all from resident data (no engine changes):

1. Get the resident room buffer via `dComIfG_getStageRes(...)` — the same handle the
   loader used ([d_stage.cpp:2403](../src/d/d_stage.cpp)). **Confirm the exact resource
   key early** (`room.dzr` vs `roomNN.dzs`) and that its lifetime spans an editing session.
2. Walk chunk headers with the **same tag scan as `dStage_dt_c_decode`**
   ([d_stage.cpp:2212](../src/d/d_stage.cpp)); for each `ACTR/SCOB/TGSC/TGDR/PLYR` and each
   layer variant `ACT0..ACTe` / `SCO0..SCOe`, cast `((int*)node+1)` to `stage_actor_class*`
   and iterate `m_entries[0..num]` (`stage_actor_data_class` at
   [d_stage.h:208](../include/d/d_stage.h); TGSC adds scale at [:219](../include/d/d_stage.h)).
3. Resolve `name → procname/argument` via `dStage_searchName` (linear over `l_objectName[]`,
   918 entries, [d_stage.cpp:508](../src/d/d_stage.cpp) / [:1525](../src/d/d_stage.cpp)).
4. Build a `(roomNo, setID) → fopAc_ac_c*` map by iterating `g_fopAcTg_Queue` with
   `fopAcIt_Executor` ([f_op_actor_iter.cpp:11](../src/f_op/f_op_actor_iter.cpp)); join on
   `fopAc_ac_c.setID` ([f_op_actor.cpp:481](../src/f_op/f_op_actor.cpp)). Note PLYR spawns
   with `setID = 0xFFFF` — not uniquely keyed; treat spawn points specially.

### 4.2 UI — new "Stage" tab in `EditorWindow` (gated on `g_levelEditorSession`)

Follows the existing two-pane pattern ([editor.cpp:1438](../src/dusk/ui/editor.cpp)):
`add_tab("Stage", [this](content){ auto& left = add_child<Pane>(content, Controlled);
auto& right = add_child<Pane>(content, Uncontrolled); … })`.

- **Left pane (`Controlled`):** scrollable list grouped by chunk type then layer; each row a
  `Button` labelled `"<name> · <ProcName> · #<setID>"`. `on_pressed` sets `g_selectedSetID`.
  Header line: `N actors · M live · room R · layer L`, plus **Refresh** and a **warp-to-room**
  control (reuse `WarpSelectionState`, [warp.cpp:15](../src/dusk/ui/warp.cpp)).
- **Right pane (`Uncontrolled`, via `register_control`):** detail of the selection —
  decoded params (hex + known bitfield break-out), pos/angle/scale, layer, live/placed status.

### 4.3 In-world feedback — no new renderer, all `dDbVw_*`

- Selected actor: translucent box + upright axis marker via `dDbVw_drawCubeXlu` /
  `dDbVw_drawArrowOpa` at `pos` ([d_debug_viewer.h:13](../include/d/d_debug_viewer.h)).
- Optional 2D labels over every placed actor: project with `mDoLib_project`
  ([m_Do_lib.cpp:60](../src/m_Do/m_Do_lib.cpp)) → `dDbVw_Report`.
- Sets up 1b picking: iterate `PlacedActor`s, project each, hit-test the `menu_pointer`
  cursor (no screen→world unproject needed).

### 4.4 Explicitly OUT of 1a (scope fence)

Moving/editing (1c), placing/deleting (1d), any write to `project.json` (arrives with 1c),
the arc baker (Phase 2), DZB/terrain (later tier). 1a proves **enumeration + selection +
in-world draw** against real rooms and nothing more.

---

## 5. Sequenced backlog

| # | Milestone | Builds on | New work | Risk |
|---|---|---|---|---|
| **1a** | Stage Inspector (§4) | resident buffer + `dDbVw_*` | buffer re-walk, list/detail UI, highlight | low |
| **1x** | Launch-menu entry + `g_levelEditorSession` + auto debug-cam | prelaunch + debug cam | button, session flag, mode wiring | low |
| **1b** | Click-to-select in world | 1a + `menu_pointer` + `mDoLib_project` | project-and-hit-test picking | low–med |
| **1c** | Live gizmo (translate/rotate) + **first `project.json` writes** | 1b + `current.pos`/`shape_angle` | gizmo state machine, edit records, live replay | med |
| **1d** | Place (reuse Actor Spawner incl. MoveBG floors) / delete | 1c + `fopAcM_create` | add/delete edit ops | low–med |
| **1e** | DZB collision-poly view (read-only terrain viz) | `poly_draw` ([d_bg_s.cpp:615](../src/d/d_bg_s.cpp)) | wiring only | low |
| — | *boundary — all above is live, reversible, ships behind flags, needs no arc writer* | | | |
| **2** | Offline RARC packer → overlay tree → `activeLevelProject` ships it | custom_assets overlay | packer (offline, uncompressed OK for v1) | med, isolated |

**Ordering rationale:** 1a is useful on its own and de-risks the whole read pillar before
committing to the packer (the only real new subsystem), which is a self-contained offline
tool that can be prototyped independently.

---

## 6. Terrain (DZB) — the later tier, and how it shapes Phase 1 now

Visual model and DZB collision are **fully independent assets**, loaded side-by-side by
`daBg_c::createHeap` ([d_a_bg.cpp:121](../src/d/actor/d_a_bg.cpp)) — `model.bmd`/`.bdl`
(up to 6 parts) vs `room.dzb`. Editing "terrain" is therefore two problems. Walls vs floors
vs edges are decided purely by triangle normal.y ([c_bg_w.cpp:24](../src/SSystem/SComponent/c_bg_w.cpp)):
an invisible wall is a steep collision poly; a fall-off edge is the *absence* of floor polys.

Difficulty tiers:
- **(a) Add terrain via MoveBG objects — feasible today.** `MoveBGCreate(arc, dzb_id,
  setFunc, heap, mtx)` registers a model+small-DZB actor live
  ([d_bg_s_movebg_actor.cpp:88](../src/d/d_bg_s_movebg_actor.cpp)). Adds floors/platforms;
  **cannot** remove or hole-punch baked walls.
- **(b) Edit baked DZB in place — moderate.** Nudging existing vertices is fine (normals
  recompute at load); topology changes invalidate the baked octree/block tables → tier (c).
- **(c) Regenerate DZB from a mesh — hard.** Needs a net-new exporter that builds
  vertex/tri/attribute/block tables **and constructs the octree** (`cBgD_*` structs at
  [d_bg_w.h:98](../include/d/d_bg_w.h)); the format is fully documented but the code doesn't
  exist.

**Two cheap forward-compat moves for Phase 1** (so terrain work later starts from a viewer,
not zero): (1) milestone **1e** — read/visualize DZB via the collision viewer's `poly_draw`;
(2) treat "add a MoveBG floor object" as **just another placeable actor** in 1d, giving a
real (if limited) terrain-add capability nearly for free.

---

## 7. Code map (verified in the research pass; re-verify before editing)

### Entry / mode
- Launch menu: `Prelaunch` ctor pushes `menu-list` buttons
  [prelaunch.cpp:691-732](../src/dusk/ui/prelaunch.cpp); restart model
  `is_restart_pending()` [:655](../src/dusk/ui/prelaunch.cpp), "Apply Options" modal
  [:775](../src/dusk/ui/prelaunch.cpp).
- Advanced-settings gate: `enableAdvancedSettings` [settings.h:498](../include/dusk/settings.h)
  (new `enableLevelEditor` mirrors it).

### Enumeration (read)
- Resident buffer + offset-fixup: `dStage_dt_c_offsetToPtr`
  [d_stage.cpp:2326](../src/d/d_stage.cpp); tag decode `dStage_dt_c_decode`
  [:2212](../src/d/d_stage.cpp); room res `dComIfG_getStageRes` use
  [:2403](../src/d/d_stage.cpp).
- Chunk structs: `stage_actor_data_class` [d_stage.h:208](../include/d/d_stage.h),
  `stage_tgsc_data_class` [:219](../include/d/d_stage.h), SCLS
  `stage_scls_info_class` [:94](../include/d/d_stage.h); resident accessors list
  [:456-549](../include/d/d_stage.h) (note: **no `getActor()`** — the gap).
- Name DB: `l_objectName[]` [d_stage.cpp:508](../src/d/d_stage.cpp) (918 entries),
  `dStage_searchName` [:1525](../src/d/d_stage.cpp), `dStage_getName` [:1539](../src/d/d_stage.cpp);
  entry struct `dStage_objectNameInf` [d_stage.h:1307](../include/d/d_stage.h).
- Layer tags: `layerActorLoader` [d_stage.cpp:2635](../src/d/d_stage.cpp),
  `dStage_setLayerTagName` (patches `ACT0`→`ACTe`).
- Live actors: `fopAcIt_Executor` [f_op_actor_iter.cpp:11](../src/f_op/f_op_actor_iter.cpp);
  `setID` copy at spawn [f_op_actor.cpp:481](../src/f_op/f_op_actor.cpp), field at
  `fopAc_ac_c` 0x494 [f_op_actor.h:286](../include/f_op/f_op_actor.h); dedupe key
  `dComIfGs_isActor(setID, roomNo)` [d_stage.cpp:1962](../src/d/d_stage.cpp).
- Live transform: `current.pos` [f_op_actor.h:297](../include/f_op/f_op_actor.h),
  `shape_angle` [:298](../include/f_op/f_op_actor.h), accessor `fopAcM_GetPosition_p`
  [f_op_actor_mng.h:228](../include/f_op/f_op_actor_mng.h).

### UI / draw / input
- RmlUi editor: `EditorWindow` [editor.hpp:17](../src/dusk/ui/editor.hpp), tab pattern
  [editor.cpp:1438](../src/dusk/ui/editor.cpp), `Pane` API
  [pane.cpp:107-198](../src/dusk/ui/pane.cpp), `register_control` [:107](../src/dusk/ui/pane.cpp);
  menu registration `menu_bar.cpp:57` gated on `enableAdvancedSettings` [:55](../src/dusk/ui/menu_bar.cpp).
- Warp: `WarpWindow` [warp.cpp:149](../src/dusk/ui/warp.cpp), trigger
  `dComIfGp_setNextStage` [:307](../src/dusk/ui/warp.cpp).
- ImGui dev menu: `ImGuiMenuTools::draw` [ImGuiMenuTools.cpp:44](../src/dusk/imgui/ImGuiMenuTools.cpp),
  Actor Spawner (`fopAcM_create` in-world) [ImGuiActorSpawner.cpp:67](../src/dusk/imgui/ImGuiActorSpawner.cpp),
  collision view toggles [ImGuiMenuTools.cpp:92-103](../src/dusk/imgui/ImGuiMenuTools.cpp).
- 3D debug draw: `dDbVw_*` [d_debug_viewer.h:13-123](../include/d/d_debug_viewer.h); collision
  render `poly_draw` [d_bg_s.cpp:615](../src/d/d_bg_s.cpp).
- Projection: `mDoLib_project` [m_Do_lib.cpp:60](../src/m_Do/m_Do_lib.cpp).
- Mouse/pointer: `mouse.cpp` (SDL3), `menu_pointer` absolute cursor + click
  [menu_pointer.cpp:201](../src/dusk/menu_pointer.cpp).

### Bake / overlay (Phase 2)
- Overlay register/reload: `install_overlays()` [custom_assets.cpp:520](../src/dusk/custom_assets.cpp),
  `aurora_dvd_overlay_files` [:569](../src/dusk/custom_assets.cpp), `overlay_generation()`
  [:460](../src/dusk/custom_assets.cpp); FST merge (any size, stable entrynum)
  [fst.cpp:150](../extern/aurora/lib/dolphin/dvd/fst.cpp).
- RARC structs (writer target): `JKRArchive.h:15-110`
  ([JKRArchive.h](../libs/JSystem/include/JSystem/JKernel/JKRArchive.h)); **no writer /
  no YAZ0 encoder exist** (decoder `JKRDecomp::decode`
  [JKRDecomp.cpp:127](../libs/JSystem/src/JKernel/JKRDecomp.cpp)).

### Terrain (later)
- DZB structs: `cBgD_t` [d_bg_w.h:98](../include/d/d_bg_w.h), tri `cBgD_Tri_t` [:59],
  attr `cBgD_Ti_t` [:67], octree `cBgD_Tree_t` [:78]; loader `cBgW::Set`
  [d_bg_w.cpp:411](../src/d/d_bg_w.cpp); classify `cBgW_CheckBGround/Wall/Roof`
  [c_bg_w.cpp:24-45](../src/SSystem/SComponent/c_bg_w.cpp).
- MoveBG: `MoveBGCreate` [d_bg_s_movebg_actor.h:18](../include/d/d_bg_s_movebg_actor.h),
  impl [d_bg_s_movebg_actor.cpp:88](../src/d/d_bg_s_movebg_actor.cpp); example
  `d_a_obj_lv4floor` [d_a_obj_lv4floor.cpp:52](../src/d/actor/d_a_obj_lv4floor.cpp).
- Room model/collision split: `daBg_c::createHeap` [d_a_bg.cpp:121-238](../src/d/actor/d_a_bg.cpp).

---

## 8. Open items to verify before/early in 1a

1. **Exact resident room resource key** (`room.dzr` vs `roomNN.dzs`) via `dComIfG_getStageRes`,
   and that its lifetime spans an editing session (vs a transient heap) — linchpin of the re-walk.
2. **Debug camera drivable independently of Link** in a live field (no cutscene/hijack
   conflicts) for free-fly authoring.
3. **Launch-menu button + `g_levelEditorSession` don't perturb the normal `Play` path** —
   the whole non-breaking premise. Editor code must early-out on `!g_levelEditorSession`.
4. **(Phase 2, verify early because it's the shipping premise):** a *non-vanilla-sized* and a
   *compressed* replacement arc actually mount through Aurora Layer A — the doc's open item
   ([Custom-Model-API-Work.md:418](Custom-Model-API-Work.md); only raw RARC tested so far).

---

## 9. Build & hygiene reminders (for the implementing chat)

- Build: `build_run.bat` → `build/windows-msvc-relwithdebinfo/dusklight.exe`. AI builds by
  default and reports; wipe `dawn_cache.db*` + `pipeline_cache.db*` in
  `%AppData%\TwilitRealm\Dusklight` after each build
  ([build-fps-guidelines.md](build-fps-guidelines.md) §GPU cache addendum).
- **Zero cost in normal play:** every editor code path early-outs on `!g_levelEditorSession`;
  no per-frame file I/O, logging, or ImGui in hot paths without that gate (§3 source rules).
- Comment style: `============` block headers on all new/modified code (project convention).
- Push to **upstream** (`WadeWinningWilson/A-Link-Between-Dusklight`), not origin; don't
  commit/push unless the user explicitly asks ([commit-and-push.md](commit-and-push.md)).

---

## 10. Milestone 1x — locked implementation plan (2026-07-09)

**Decisions locked (user):**
1. **`enableLevelEditor` requires `enableAdvancedSettings`** — the editor toggle is
   disabled in the UI unless advanced settings are on (singular trust boundary; the
   editor leans on debug cam + ImGui tooling that advanced settings already gate).
2. **Auto-enable free-fly cam is in 1a**, not 1x — 1x stays purely mechanical
   (flag + button + restart gate + save block).
3. **Native game saves are blocked in an editor session** (see §10.3). Editor
   persistence (`project.json`) is a *separate* mechanism landing in 1c; a loaded
   save file is **read-only world context**.

### 10.1 Mechanical edit points (all confirmed file:line)

| # | Change | File:line |
|---|---|---|
| A1 | `ConfigVar<bool> enableLevelEditor;` in `backend` struct | [settings.h:498](../include/dusk/settings.h) |
| A2 | `.enableLevelEditor {"backend.enableLevelEditor", false},` | [settings.cpp:219](../src/dusk/settings.cpp) |
| A3 | `Register(g_userSettings.backend.enableLevelEditor);` | [settings.cpp:479](../src/dusk/settings.cpp) |
| A4 | `...enableLevelEditor.setSpeedrunValue(false);` | [speedrun.cpp:43](../src/dusk/speedrun.cpp) |
| B  | `config_bool_select(... enableLevelEditor ...)` after the advanced-settings block; `.isDisabled = [] { return speedrunMode \|\| !enableAdvancedSettings; }` | [settings.cpp:1721](../src/dusk/ui/settings.cpp) |
| C1 | `extern bool g_levelEditorSession;` | [main.h:10](../include/dusk/main.h) |
| C2 | `bool dusk::g_levelEditorSession = false;` | [m_Do_main.cpp:128](../src/m_Do/m_Do_main.cpp) |
| D  | gated `"Level Editor"` button in menu-list; `on_pressed` mirrors Play + sets `g_levelEditorSession = true` | [prelaunch.cpp:720](../src/dusk/ui/prelaunch.cpp) |
| E1 | `bool initialLevelEditor = false;` in `PrelaunchState` | [prelaunch.hpp:57](../src/dusk/ui/prelaunch.hpp) |
| E2 | capture in `ensure_initialized()` | [prelaunch.cpp:631](../src/dusk/ui/prelaunch.cpp) |
| E3 | compare in `is_restart_pending()` | [prelaunch.cpp:655](../src/dusk/ui/prelaunch.cpp) |

### 10.2 Control flow

```
Advanced Settings ▸ Enable Advanced Settings = ON → Enable Level Editor = ON
      └─ is_restart_pending() → "Apply Options" modal → Restart
Boot → Prelaunch ctor reads enableLevelEditor → pushes "Level Editor" button
      └─ click → g_levelEditorSession = true; IsGameLaunched = true; pop()
Everywhere editor code lives:  if (!g_levelEditorSession) return;   // zero cost on normal Play
```

### 10.3 Native-save block — design (the non-obvious part)

Two in-play commit sites persist editor-session progress and must be blocked; the
`d_file_select.cpp` writes are pre-gameplay file management and are left alone.

- **Autosave — clean trigger gate.** `canAutoSave()` ([autosave.cpp:17](../src/dusk/autosave.cpp))
  returns **false** when `g_levelEditorSession`. The autosave proc never starts →
  zero state-machine risk. This is the important one (autosave is the automatic,
  silent corruptor).
- **Manual field save (statue / pause "Save") — proc-safe redirect.** Do **not** skip
  the commit at `dataSave()`: the wait proc polls `SaveSync()`, which returns 0 while
  `mCardState == CARD_STATE_READY_e` ([m_Do_MemCard.cpp:321](../src/m_Do/m_Do_MemCard.cpp)),
  so a naive skip **hangs** the menu. Instead guard `dataWrite()`
  ([d_menu_save.cpp:1352](../src/d/d_menu_save.cpp)): in an editor session, prepare the
  buffer as normal (in-RAM, harmless) but **skip `dataSave()`**, set `mCmdState = 1`
  (success sentinel) and `mMenuProc = PROC_MEMCARD_DATA_SAVE_WAIT2`, so
  `memCardDataSaveWait2()` runs its normal success path **without any card I/O**.
  Net: the menu completes, nothing is written to the card.
- **Known cosmetic caveat (superseded by §10.5):** the redirect still shows vanilla
  "Saved. Continue Playing?" text — misleading in editor mode (nothing persisted, but
  nothing corrupted either). Replaced by the deny-at-selection UX in §10.5.
- **New-file creation** (`PROC_MEMCARD_MAKE_GAME_FILE`, also calls `dataSave()` at
  [d_menu_save.cpp:1284](../src/d/d_menu_save.cpp)) is **not** gated in 1x — editor
  mode loads an existing file as context, not creates one. Revisit if editor entry
  ever supports New Game.

### 10.4 Verification checklist (build-guidelines discipline)

1. Default boot (feature off): menu shows exactly `Play / Settings / Quit`.
2. Toggle on → "Apply Options / Restart" modal appears; after relaunch the
   `Level Editor` button is present and nav reaches it.
3. Click it → `g_levelEditorSession == true`; a normal `Play` boot leaves it false and
   field FPS is unchanged (`F_SP121` r0 p0 per build guidelines).
4. Speedrun mode → the Level Editor toggle is disabled/forced off; button never appears.
5. Editor session: autosave never fires; a manual/statue save completes the menu but
   writes nothing to the card (verify the slot's timestamp is unchanged).

### 10.5 Save-block UX refinement — deny at selection (PLANNED, no code; supersedes §10.3 caveat)

**Playtest outcome (2026-07-09):** all six §10.4 checks pass; FPS unchanged. User
directed a UX improvement: instead of letting the save flow run and silently no-op'ing
the commit (which shows a misleading vanilla "Saved. Continue Playing?"), the vanilla
**Save must be un-selectable in an editor session** — selecting it plays the standard
denied-action buzzer (like the dusk "bash" system) and does nothing.

**Approach:** deny-at-selection is the **UX**; the §10.3 commit guards stay as the
silent **guarantee** (defense in depth). Buzzer = `Z2GetAudioMgr()->seStart(Z2SE_SYS_ERROR,
NULL, 0, 0, 1.0f, 1.0f, -1.0f, -1.0f, 0)` — the generic denied-action SE used in
[d_a_alink_dusk.cpp](../src/d/actor/d_a_alink_dusk.cpp) / [d_menu_fmap.cpp](../src/d/d_menu_fmap.cpp).

**Two save-entry surfaces → two deny hooks** (both `#if TARGET_PC` + `if (g_levelEditorSession)`):

| Surface | Selection point | Deny action |
|---|---|---|
| START → collection menu → Save | `getSubWindowOpenCheck()==1 → SAVE_OPEN`, [d_menu_window.cpp:781](../src/d/d_menu_window.cpp) (`dMw_c::collect_close_proc`); deepest/best-feel hook is the Save-icon confirm inside `dMenu_collect`) | play `Z2SE_SYS_ERROR`; do **not** enter `SAVE_OPEN` — stay in / reopen the collection menu |
| Owl/bird statue prompt | `saveQuestion()` `mYesNoCursor == CURSOR_YES`, [d_menu_save.cpp:825](../src/d/d_menu_save.cpp) | play `Z2SE_SYS_ERROR`; route to the existing No/cancel branch ([:845](../src/d/d_menu_save.cpp)) — no new proc, no `SaveSync` dependency, no hang |

**Keep as silent backstops:** `canAutoSave()` → false (autosave, no menu — unchanged);
the `dataWrite()` guard (§10.3) as a no-write safety net (never reached once selection
is denied, so the "Saved" text no longer appears).

**Open verifications (implementation time):**
1. Do the collection-menu Save and the owl-statue save both funnel through `saveQuestion`
   (one hook) or need the two separate hooks above? Depends on `dMenu_save_c::mUseType`.
2. Exact Save-icon confirm line inside `dMenu_collect` (cursor-stays hook) vs. the simpler
   `collect_close_proc` fallback (momentary close→reopen).
3. Confirm `Z2SE_SYS_ERROR` (generic denied buzzer) is preferred over `Z2SE_SY_FILE_ERROR`.

**Roadmap placement:** small **1x.1** follow-up; land alongside or just before milestone
1a (both touch editor-session UX).
</content>
</invoke>
