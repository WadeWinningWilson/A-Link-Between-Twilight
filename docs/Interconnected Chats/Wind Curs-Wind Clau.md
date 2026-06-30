# Wind Curs ↔ Wind Clau

**Interconnected chat doc** — WW `itemmdl` Track A (get-item bow pilot) ↔ Cursor implementation ↔ Claude review.

| | |
|---|---|
| **Cursor (Wind Curs)** | In-repo edits, build/launch loop, surgical wiring, handoff doc updates |
| **Claude (Wind Clau)** | Review roadmap, graphics strategy, Plan A vs Plan B, second opinion before big changes |
| **Canonical detail** | [`docs/wind-waker-item-work.md`](../wind-waker-item-work.md) — mesh indices, TWW rows, cut-enemy notes |
| **Status (2026-06-27)** | **Track A disconnected** from build (WW sources exist locally; hooks/cmake/settings removed during launch bisect). Mesh-on-screen baseline was achieved before disconnect. **Material pass not done.** |

---

## 🧭 MASTER IMPLEMENTATION ROADMAP — READ THIS FIRST

> **Goal:** Retail TP `itemmdl.arc` → `vbow.bdl` in the bow get-item spin, **WW-faithful cel look** (not TP-material shortcut).  
> **Strategy:** **Plan A** — retail `bdl3` + code-side TEV/draw trial-and-error. **Plan B** (modder BMD swap) is a documented escape hatch only.  
> **No SS decomp** — trial-and-error + optional TWW Rosetta for struct-0 vs struct-14 only.  
> **Tool split:** Cursor executes phases below; Claude reviews diffs/screenshots/logs and revises this doc’s “Open questions” / experiment order.  
> **Phase numbering:** **This doc’s Phases 0–6 are canonical.** `wind-waker-item-work.md` “Implementation plan” uses a different 0–3 scale — ignore it for execution. Session openers must say **“roadmap Phase N”**.

### Visual target (what “done” looks like)

| Stage | On screen |
|-------|-----------|
| **Baseline (already hit once)** | WW bow **shape** spins in get-item; **dark/muddy** fill; possible **white edge shards** on outline pass |
| **Plan A success** | Same WW mesh/textures; **readable cel colors**; tuned **`SC_Vbow_v`** ink outline |
| **Plan B (alt)** | Same WW **silhouette**; shading reads **TP get-item glossy** — faster to stabilize, less WW-authentic |

Our path is **still feasible**: geometry + load path worked; remaining work is **lighting/TEV**, not “can we show the model.”

---

## Phase 0 — Boot hygiene (gate; no WW code)

**Owner:** User + either agent  
**Exit:** Title → opening demo green; no Aurora `unhandled tcg src 21`.

| Step | Action |
|------|--------|
| 0.1 | Build `RelWithDebInfo` via `build_run.bat` — no `DUSK_DRIVE*` for manual playtests |
| 0.2 | If logo/demo fatal: delete **both** `%AppData%\TwilitRealm\Dusklight\dawn_cache.db*` **and** `pipeline_cache.db*` ([build-fps-guidelines](../build-fps-guidelines.md)) |
| 0.3 | Confirm WW track still **out** of `files.cmake` — bisect proved boot crash was cache, not WW |

**Do not:** `git checkout`/`restore` whole files; `git stash`; PowerShell `git show HEAD:file > file`.

---

## Phase 1 — Re-link shell only (build, no item heap yet)

**Owner:** Cursor  
**Exit:** Project compiles and **launches** with WW object files linked; **`d_s_play` hook present but inert when toggle off** — no item-pipeline / heap behavior change.

| Step | Files / work |
|------|----------------|
| 1.1 | Add to `files.cmake`: `src/d/d_ww_itemmdl_pc.cpp`, `src/d/d_ww_itemmdl_test.cpp` (+ headers already present) |
| 1.2 | Restore settings: `game.wwItemmdlGetItem` in `include/dusk/settings.h`, `src/dusk/settings.cpp` |
| 1.3 | Restore Editor UI: toggle + replay button in `src/dusk/ui/editor.cpp` (Advanced → ALBW) |
| 1.4 | Restore play tick hook in `src/d/d_s_play.cpp` for replay helper |
| 1.5 | **Do not** touch `d_a_itembase_static.cpp`, `d_a_demo_item.cpp`, or `d_a_alink_demo.inc` |

**Playtest:** Launch only. Toggle off → confirm replay hook early-outs (no demo spawn side effects). Toggle may do nothing visible yet — that is OK.

**Claude review prompt:** “Phase 1 diff is shell-only — confirm no item pipeline or demo.inc changes.”

---

## Phase 2 — Wire get-item heap (back to broken mesh baseline)

**Owner:** Cursor  
**Exit:** Toggle **on** → Editor replay or real bow get-item shows **WW mesh spinning** (colors wrong OK).

| Step | Files / work |
|------|----------------|
| 2.1 | Surgical branch in `daItemBase::CreateItemHeap`: bow + toggle → `dWwItemmdl_getVbowModelData("itemmdl")` + `dWwItemmdl_patchModelForPc` |
| 2.2 | Demo item arc swap: bow + toggle → `dComIfG_resLoad` / `DeleteBase` on `"itemmdl"` (`d_a_demo_item.cpp`) |
| 2.3 | PC load: `J3DMLF_DoBdlMaterialCalc`, locked baked-DL draw (**no** unlock/`diff()`) |
| 2.4 | **Stability (2026-06-30 fix):** WW bow skips `setLightTevColorType_MAJI`; loader never attaches arc-resident `J3DModelData*`; **no** draw-time `SC_*` shape hide until Phase 4 |
| 2.5 | Joint null guards in demo item (`demoItemRootHalfHeight`) — WW `vbow` has 2 joints |
| 2.6 | Replay helper in `d_ww_itemmdl_test.cpp` only — **no** `d_a_alink_demo.inc` edits |

**Debug:** `Documents/dusklight/albw_ww_itemmdl_debug.txt`

**Playtest checklist:**

- [ ] Toggle off → vanilla `O_gD_bow` (~6 s replay)
- [ ] Toggle on → WW `vbow` visible (broken colors OK; white edge shards OK for now)
- [ ] Log shows `procCoGetItemInit returned` → `Replay finished (6 s).`
- [ ] Toggle off again → vanilla

**Claude review prompt:** Paste screenshot (broken mesh) + last 20 lines of debug log; ask for smallest Phase 3 experiment order.

---

## Phase 2 stability gate — contingency (if retest still crashes)

**User rule (2026-06-30):** If the Phase 2 retest **still crashes**, Cursor **stops implementation** — no further code experiments in that session. Document findings here, wait for user direction + Wind Clau review, then execute the chosen branch only.

### How to read the log (classify before coding)

| Last log line(s) | Crash window | Meaning |
|------------------|--------------|---------|
| `patchModel: done` but no `demo item ready` | Heap / spawn | `CreateItemHeap` or actor init failed after load |
| `demo item ready` but no `starting procCoGetItemInit` | Pre-proc | Replay timing, joint null, or item deleted before demo start |
| `procCoGetItemInit returned` then silence | **First get-item draw** | Most common — locked `bdl3` + TP draw stack (`DrawBase`, `mDoExt_modelUpdateDL`, tev, shadow) |
| `Replay finished (6 s).` | — | **Phase 2 pass** → proceed to Phase 3 gate |

Paste **last ~20 lines** of `albw_ww_itemmdl_debug.txt` + note whether crash was instant on replay or on toggle-only.

### Strategy (ranked — one branch per approved session)

Do **not** stack these in one diff. Pick one after log classification + Claude sign-off.

| Priority | Branch | Hypothesis | Minimal change | Pass signal |
|----------|--------|------------|----------------|-------------|
| **2A** | Draw-path bisect | Fault inside `DrawBase` sub-step | Add **temporary** log lines only: before/after `setTevStr`, `animEntry`, `mDoExt_modelUpdateDL` in `daDitem_c::draw` / `DrawBase` | Last log line names the crashing call |
| **2B** | Private heap copy | Arc/cache alias or `finishLoadedModelData` still shares mutable state | Load `vbow` via `JKRReadIdxResource` → temp buffer → `loadBinaryDisplayList` (same flags); **never** return `dComIfG_getObjectRes` pointer | Load + draw without crash |
| **2C** | Minimal WW draw stub | TP get-item draw stack incompatible with locked WW cel DL | WW bow only: skip `setListDark`, skip shadow, struct **14** tev only (already no MAJI), plain `mDoExt_modelUpdateDL` | Mesh visible, ugly but stable |
| **2D** | Vanilla mesh / WW arc | Isolate loader vs draw — is it the mesh or the pipeline? | Toggle on loads `itemmdl` arc but heap still uses `O_gD_bow` BDL (one session only) | Crash gone ⇒ WW mesh/loader; crash remains ⇒ demo.inc / Link proc |
| **2E** | Defer edge without `hide()` | `SC_Vbow_v` white shards acceptable short-term | No shape mutation; accept dual-pass draw until Phase 4 | Stability over visuals |
| **2F** | Plan B preview | Plan A draw path exhausted | Modder BMD + TP get-item materials (see Plan B section) | Stable TP-styled WW silhouette |

### Explicit do-nots while gate is open

| Do not | Why |
|--------|-----|
| Phase 3 TEV experiments (struct 0, delegate, magenta) | Phase 2 must pass first |
| Re-enable draw-time `SC_*` hide | Correlated with first-draw crash |
| Re-enable MAJI on WW bow | Correlated with first-draw crash on locked `bdl3` |
| Unlock materials / `diff()` on BDL | Documented crash |
| Patch `d_a_alink_demo.inc` | Use replay helper only |
| Whole-file git restore | Encoding / WIP loss |

### Agent workflow on crash

1. User reports crash + log tail.
2. Cursor **updates session log** (below) with classification row — **no source edits** unless user explicitly approves a numbered branch (2A–2F).
3. User pastes log + this section to **Wind Clau** for branch pick.
4. Cursor implements **one** approved branch; user playtests; repeat until Phase 2 checklist passes.

**Deliverable when Phase 2 passes:** screenshot (toggle on, broken colors OK) + log tail → Wind Clau Phase 3 gate (3.0 magenta isolate first).

---

## Phase 2 crash analysis — 2026-06-30 retest (doc only; no code)

**Status:** Phase 2 stability fix **did not pass**. Cursor stops here until user approves one branch below.

### Log evidence

Latest run (fresh process, toggle **on**, Editor replay):

```
frame=1488 heap: getVbowModelData: loaded via name (vbow.bdl) mats=2 tex=3
frame=1488 heap: patchModel: mats=2 joints=2 tex=3
frame=1488 heap: patchModel: mat[0]=SC_Vbow_v edge=1
frame=1488 heap: patchModel: mat[1]=Vbow_v edge=0
frame=1488 heap: patchModel: done
frame=1488 phase=3: demo item ready
frame=1488 phase=3: starting procCoGetItemInit (direct, no DEFAULT_GETITEM event)
frame=1488 phase=5: procCoGetItemInit returned (dev replay; auto-end 6s)
```

**No** `Replay finished (6 s).` — same signature as pre-fix run at frame 3529.

**Control (still good):** All pre–Phase-2 replays (toggle on but **vanilla** `O_gD_bow` heap) show full `procCoGetItemInit returned` → `Replay finished (6 s).` (e.g. frames 2469–3209). Crash is **WW heap-specific**, not replay helper / Link proc init alone.

### Classification

| Field | Value |
|-------|--------|
| **Crash window** | **First get-item draw** — after `procCoGetItemInit returned`, before replay auto-end |
| **Load path** | Raw `vbow.bdl` → `loadBinaryDisplayList` + `finishLoadedModelData` (heap log confirms) |
| **Not boot / cache** | Game reached field + replay spawn; not `tcg src 21` at title |

### Ruled out this session (fixes were in build; crash persists)

| Fix applied | Expected if root cause | Result |
|-------------|------------------------|--------|
| Skip `setLightTevColorType_MAJI` on WW bow | No material light/amb patch crash | **Still crashes** |
| Remove draw-time `SC_*` `shape->hide()` | No shared-arc mutation on draw | **Still crashes** |
| Reject arc-resident `J3DModelData*` in loader | Heap-owned copy only | **Still crashes** — load via raw `bdl3` + `finishLoadedModelData` |
| Joint null guards | No pre-proc fault | **Passed** — reaches `procCoGetItemInit returned` |

**Conclusion:** Root cause is **not** MAJI, outline hide, or arc-resident attach alone. Fault is deeper in the **WW model + demo-item draw pipeline** on first visible frame.

### Draw path already in play (narrows bisect)

`daDitem_c::setListStart()` is **empty** — demo items do **not** call `dComIfGd_setListDark()` (unlike field items). Remaining `DrawBase()` steps for WW bow:

1. `settingTevStruct(14)` only (MAJI skipped)
2. `animEntry()` — no-op for bow (no BTK/BCK in row)
3. `settingBeforeDraw()` — only if actor flag 4 (effect mtx)
4. **`mDoExt_modelUpdateDL(mpModel)`** ← top suspect
5. `setListEnd()` → `dComIfGd_setList()`
6. **`setShadow()`** ← second suspect

### Technical hypothesis (for Wind Clau)

**H1 — `mDoExt_modelUpdateDL` vs locked `bdl3` post-process (most likely)**  
`finishLoadedModelData()` runs after `loadBinaryDisplayList` with `DoBdlMaterialCalc`:

- Adds `J3DMaterialAnm` per material
- `newSharedDisplayList` + `makeSharedDL()`

`mDoExt_modelUpdateDL` (`m_Do_ext.cpp`) branches on shared-DL + lock state:

- Unlocked + shared DL → **`diff()`** path (documented crash on WW BDL)
- Locked BDL (`MDL3` sets material table lock) → **`unlock()` → `update()` → `lock()`** path

Either branch may fault on WW cel materials (`SC_Vbow_v` edge + `Vbow_v` fill) under TP get-item struct-14 tevstr, without MAJI. Vanilla `O_gD_bow` is a different asset class (TP BMD, not WW `bdl3`).

**H2 — Dual-material GPU draw (`SC_Vbow_v` + `Vbow_v`)**  
Both materials draw on first frame (outline not hidden). Edge pass (`OpaTexEdge`) + baked WW cel DL + struct-14 env may **GPU-fault** Aurora even when CPU path survives init.

**H3 — `setShadow()` on WW model**  
`DrawBase` always calls `setShadow()`; WW bounding / shadow path untested.

**H4 — `s_cached` static model**  
`dWwItemmdl_getVbowModelData` caches first successful load for process lifetime. Unlikely primary cause (first run crashes too), but **always use a fresh process** per playtest.

### Revised execution plan (one branch per approved session)

**Do not stack.** **Do not start Phase 3** until Phase 2 checklist passes.

| Order | Branch | When | Action | Pass signal |
|-------|--------|------|--------|-------------|
| **1** | **2A — Draw bisect** | **Always first** | Temporary logs only in `DrawBase` / `daDitem_c::draw`: before/after `setTevStr`, `settingBeforeDraw`, `mDoExt_modelUpdateDL`, `setListEnd`, `setShadow` | Last log line names faulting call |
| **2** | **2D — Loader isolate** | After 2A or in parallel if user wants fast fork | Toggle on + `itemmdl` arc, but heap still `O_gD_bow` BDL (one session) | Crash **gone** ⇒ WW mesh/loader; **remains** ⇒ Link / demo proc |
| **3a** | **2G — Slim loader** | 2A pins `modelUpdateDL` | Skip `finishLoadedModelData` post-steps (`makeSharedDL` / `newSharedDisplayList` / extra `MaterialAnm`) — use `loadBinaryDisplayList` output as-is | Draw survives |
| **3b** | **2H — WW draw override** | 2A pins `modelUpdateDL` | WW bow: override draw to `mDoExt_modelEntryDL` or locked shared-DL entry **without** `unlock/update` dance; or body mat only | Mesh visible, ugly OK |
| **4** | **2C — Skip shadow** | 2A pins `setShadow` | WW bow: no-op `setShadow` in demo draw | Stable spin |
| **5** | **2B — Private buffer load** | 2A/load ambiguous | `JKRReadIdxResource` → private buffer → `loadBinaryDisplayList` (deprioritized — already on raw path) | Load + draw |
| **6** | **2F — Plan B** | 2A–2H exhausted | Modder BMD + TP get-item materials | Stable TP-styled silhouette |

**Deprioritized:** **2E** (defer edge without hide) — already accepting dual-pass draw; crash persists without hide.

### Playtest hygiene (mandatory)

1. **Quit dusklight fully** between attempts (clears `s_cached` + GPU state).
2. Note Windows exit code if shown (`0xC0000409` / `tcg src N` → GPU path).
3. Paste **last ~20 log lines** + which branch user approved before any code resumes.

### Wind Clau prompt (paste on branch pick)

> Phase 2 retest still crashes after MAJI/hide/arc-resident fixes. Log stops at `procCoGetItemInit returned` (frame 1488). Read **Phase 2 crash analysis — 2026-06-30 retest** in `Wind Curs-Wind Clau.md`. Pick **one** branch: 2A first, then 2D / 2G / 2H / 2C / 2F. No Phase 3 until mesh spins 6 s.

---

## Phase 3 — Material / TEV pass (Plan A — trial and error)

**Owner:** Cursor implements; user playtests; Claude prioritizes experiments  
**Exit:** Fill colors **readable**; edge pass **deferred** until fill stable.

### Known root cause (decomp-backed via TWW, not SS)

TP `daDitem_c` uses **`settingTevStruct(14)` + `setLightTevColorType_MAJI`** — built for TP get-item props (`O_gD_bow`), not WW cel BDLs. TWW demo items use **`TEV_TYPE_ACTOR` (0)** + normal `setLightTevColorType`.

### Experiment order (try in sequence; one variable per session)

> **Claude revision (2026-06-30):** run **3.0 isolate first** — confirms TEV vs texture before any struct-0 work.

| # | Experiment | Where | Success signal |
|---|------------|-------|----------------|
| **3.0** | **Isolate:** flat magenta DDS via texture replacement API on vbow embedded tex | AppData `texture_replacements/` | Still dark → TEV/lighting (go 3.1). Magenta wrong/missing → albedo/decode path — **don’t waste time on 3.1–3.3 until fixed** |
| 3.1 | WW bow: **delegate to `daItemBase_c::setTevStr`** (no demo override). Fallback: struct **0** + `setLightTevColorType` | `d_a_demo_item.cpp` | Fill no longer mud-dark |
| 3.2 | Draw list: mask-off vs TP dark list | demo item draw setup | Slight improvement in contrast |
| 3.3 | Keep `d_ww_itemmdl_pc` locked BDL; tune tev block flags only | `d_ww_itemmdl_pc.cpp` | Stable + brighter |
| 3.4 | **Do not** unlock materials for `diff()` until 3.0–3.3 exhausted | — | Avoid post-demo crashes |

**Not in scope:** SS decomp port; `loadFromResourcePointer()` fallback; whole-file git restores.

**Claude review prompt:** “Given screenshot after 3.1, is darkness TEV or texture? What’s the next single change?”

---

## Phase 4 — Edge pass + motion polish (Track A complete)

**Owner:** Cursor  
**Exit:** Track A shippable for bow get-item dev toggle.

| Step | Work |
|------|------|
| 4.1 | Re-enable **`SC_Vbow_v`** draw-time hide/show tuning — cel ink without white shards |
| 4.2 | Enable BTK spin: `dRes_INDEX_ITEMMDL_BTK_VBOW_e` (0x24) in create path after colors stable |
| 4.3 | Stress: load/unload `itemmdl`, repeated Editor replay, real cutscene get-item |
| 4.4 | Update progress table in `wind-waker-item-work.md` |

---

## Phase 5 — Track B held bow (later)

**Owner:** Cursor + optional asset tooling  
**Exit:** `vbow` parented to Link hand; aim/shoot works (stiff mesh OK).

| Step | Work |
|------|------|
| 5.1 | Toggle `game.wwItemmdlHeldBow` + branch `setBowModel()` in `d_a_alink_bow.inc` |
| 5.2 | Default **skip** TP bow BCK on swapped mesh |
| 5.3 | Tune hand matrix in `d_a_alink.cpp` **or** Plan B joint edit in external tool (GCFT) |
| 5.4 | Epona / left-hand path playtest |

---

## Phase 6 — Scale beyond bow (optional)

| Step | Work |
|------|------|
| 6.1 | ImGui itemmdl viewer (21 BDL indices) |
| 6.2 | Extend Phase 3 tev path to other `v*` meshes using TWW arc table in handoff doc |
| 6.3 | Field pickup row for bow |

---

## Plan B — Asset shortcut (escape hatch)

Use only after **3.0 + 3.1–3.3 exhausted** **and** user accepts non-retail asset path.

| Step | Modder workflow | Dusk integration |
|------|-----------------|------------------|
| B.1 | Unpack WW mesh from `vbow`; repack as **BMD** (not BDL) | Add optional dev arc or replace heap target |
| B.2 | Clone materials from `O_gD_bow` / TP get-item template | May “just work” with struct 14 |
| B.3 | Dummy toon texture for glossy cel | Material slot / TP ramp |
| B.4 | GCFT joint edits for held alignment | Track B primarily |

**Visual:** TP-styled WW shape — not pixel-faithful WW get-item.

---

## Agent pitfalls (mandatory)

| Do not | Why |
|--------|-----|
| `git show HEAD:file.cpp > file.cpp` in PowerShell | UTF-16 BOM corrupts sources |
| `git checkout`/`restore` **whole files** without user naming the file | Wiped WIP (demo.inc incident). **Named-file** `git restore` is OK when user specifies the path |
| Patch `d_a_alink_demo.inc` for message skip | Use `d_ww_itemmdl_test.cpp` |
| Hide **both** `Vbow_v` and `SC_Vbow_v` at create | Post–get-item crash correlation |
| Draw-time / create-time `SC_*` `shape->hide()` on shared arc data | First-draw crash on Aurora (Phase 2 fix removed; Phase 4 only) |
| `setLightTevColorType_MAJI` on WW locked `bdl3` | First-draw crash; WW bow uses struct 14 tev only until Phase 3 |
| Unlock BDL for `diff()` early | Crashes; stay locked + `DoBdlMaterialCalc` |
| Bisect boot without cache wipe first | False positives (`tcg src 21`) |

---

## Key files (quick index)

| Area | Path |
|------|------|
| Handoff / mesh table | `docs/wind-waker-item-work.md` |
| PC loader / materials | `src/d/d_ww_itemmdl_pc.cpp`, `include/d/d_ww_itemmdl_pc.h` |
| Editor replay | `src/d/d_ww_itemmdl_test.cpp` |
| Get-item actor | `src/d/actor/d_a_demo_item.cpp`, `src/d/actor/d_a_itembase.cpp` |
| Item resource table | `src/d/d_a_itembase_static.cpp` |
| Indices | `assets/GZ2E01/res/Object/itemmdl.h` |
| Build / cache | `docs/build-fps-guidelines.md` |
| Git rules | `docs/commit-and-push.md` |

---

## Cursor ↔ Claude workflow

1. **Cursor** runs the phase in this doc; updates “Session log” below when a phase completes or fails.
2. **User** playtests and attaches screenshot + debug log snippet.
3. **Claude** reviews: confirms experiment order, flags scope creep, suggests Plan B only if Plan A stuck.
4. **Neither agent commits** unless user asks ([commit-and-push](../commit-and-push.md)).
5. **Phase 2 crash:** Cursor **does not** continue implementation — classify log, update session log, wait for user + branch approval from **Phase 2 stability gate** (or Wind Clau).

**Cursor session opener (paste):**

> Read `docs/Interconnected Chats/Wind Curs-Wind Clau.md` and `docs/wind-waker-item-work.md`. Execute **roadmap Phase [N]** only. No whole-file git restore. Minimal diff. Do not commit.

**Claude session opener (paste):**

> Read `docs/Interconnected Chats/Wind Curs-Wind Clau.md`. Review Phase [N] plan or attached screenshot/log. Revise experiment order; do not suggest SS decomp.

---

## Session log (append-only)

| Date | Phase | Result | Notes |
|------|-------|--------|-------|
| 2026-06-27 | — | Roadmap created | WW track disconnected; baseline mesh achieved pre-bisect. |
| 2026-06-30 | — | Claude first pass | Review merged: canonical numbering, Phase 1 exit tweak, Phase 3.0 isolate first, delegate-first 3.1. Cursor ack below. |
| 2026-06-30 | — | Loop closed | Wind Clau confirm + execution queue agreed. Cursor idle until Phase 0/1 requested. |
| 2026-06-30 | **Phase 0** | **Pass (user)** | Boot/build verified on other work — title runs clean; no WW wiring yet. Cleared for **roadmap Phase 1**. |
| 2026-06-30 | **Phase 1** | **Done (Cursor)** | Re-linked cmake/settings/editor/d_s_play; replay auto-ends at 6 s. User playtest: toggle off inert, launch green. |
| 2026-06-30 | **Phase 2** | **Fix (Cursor)** | Crash on first WW draw: skip MAJI on locked bdl3, remove draw-time SC hide, no arc-resident model attach. User playtest pending. |
| 2026-06-30 | **Phase 2** | **Awaiting user** | Retest queued. If still crashes → Cursor stops coding; use **Phase 2 stability gate** section; Wind Clau picks branch 2A–2F. |
| 2026-06-30 | **Phase 2** | **Fail (user)** | Retest crashed post-`procCoGetItemInit returned`. MAJI/hide/arc-resident fixes insufficient. **Analysis + revised strategy** in doc; **no code** until branch approved. |

---

## Claude review / revisions

<!-- Wind Clau: append review comments, revised experiment order, or Plan A vs B recommendation here. -->

### 2026-06-30 — Phase 2 retest fail (Cursor analysis; awaiting Wind Clau)

**User report:** Second crash after stability fix. **No code changes** this session.

**Findings:** Log classifies as **first get-item draw** window. Load succeeds (`vbow.bdl`, 2 mats, 2 joints). Prior fixes (no MAJI, no outline hide, no arc-resident attach) **did not** restore stability. Vanilla-heap replays with toggle on still complete 6 s — fault is **WW `CreateItemHeap` mesh + `DrawBase`**, not replay shell.

**Recommended next step:** Approve **2A (draw bisect logs only)** → read last log line → pick **2G/2H** if `modelUpdateDL`, **2C** if `setShadow`, or **2D** for loader fork.

**Cursor:** idle until user names branch (e.g. “Execute 2A”).

---

### 2026-06-30 — Wind Clau first pass

Roadmap is sound and the load-bearing facts match the handoff (BDL `0xF`, BTK `0x24`, struct-14-vs-0 root cause, "never hide both materials at create", cache-wipe boot hygiene). Findings, prioritized:

**Must-fix (agent will misread otherwise)**

1. **Phase-number collision.** This doc numbers Phases **0–6**; `wind-waker-item-work.md` "Implementation plan" numbers a *different* Phase **0–3** (0=shared infra, 1=Track A, 2=Track B, 3=optional). The session opener says "Execute Phase [N]" — ambiguous. **This doc's numbering is canonical and supersedes the Implementation-plan numbering in the handoff.** When pasting an opener, say e.g. "roadmap Phase 1".
2. **Stale "Done" table in the handoff.** `wind-waker-item-work.md` Track A table still shows toggle/Editor replay/load as "Done" — but those hooks were disconnected during the launch bisect, which is *why* Phase 1 exists. A 2026-06-27 disconnect note has been added above that table so the two docs stop disagreeing. Trust **this doc's** status: Track A is disconnected; re-link via Phase 1.

**Plan correctness**

3. **Phase 1 "no behavior change" is slightly false.** Step 1.4 restores the `d_s_play.cpp` play-tick hook — a per-frame hook runs even with the toggle off. Revised exit criterion: *hook present but early-outs when toggle off; no item-pipeline/heap change.* Playtest must confirm the off-path is inert, not just that it launches.
4. **Reorder Phase 3 — run the magenta-texture isolate FIRST (promote 3.4 → 3.0).** Struct-14→0 is decomp *inference*, unconfirmed on Aurora. The flat-magenta DDS swap is the experiment that *tests* the hypothesis: still dark ⇒ TEV/lighting (proceed to 3.1); magenta wrong ⇒ it's the texture/albedo decode path and **3.1–3.3 are wasted**. Diagnose before you fix.
5. **In 3.1, try the delegate first.** Prefer delegating to `daItemBase_c::setTevStr` over hand-setting struct 0 + `setLightTevColorType`. TWW's lesson is that demo items *don't* override tev setup, so the delegate is the smaller, more-faithful change; manual struct-0 is the fallback.

**Nits**

- Plan B trigger: tie to a concrete gate — *"after 3.0 isolate + 3.1–3.3 exhausted"* — not "multiple sessions".
- git-restore: Phase 0 bans whole-file restore (correct), but a **named-file** `git restore` is allowed (handoff pitfalls) — preserve that nuance so agents don't think all restores are banned.

**Plan A vs B:** stay on Plan A. Geometry + load path already worked; the remaining work is lighting/TEV, and finding 4 will tell you within one session whether the struct hypothesis even holds. Plan B only after 3.0 + 3.1–3.3 are exhausted.

---

### 2026-06-30 — Wind Curs response (ack + merged)

**Received.** Doc-only edits from Wind Clau are accepted; the roadmap body above is updated to match (not just commentary).

| Claude finding | Cursor action |
|----------------|---------------|
| Phase-number collision | **Merged.** Canonical numbering callout at top; session opener now says **roadmap Phase N**. Handoff disconnect note (Claude added) stays the cross-link. |
| Stale “Done” table | **Acknowledged.** Phase 1 re-link is the source of truth; we won’t treat handoff “Done” as wired until Phase 1 playtest passes. |
| Phase 1 hook caveat | **Merged.** Exit criterion + playtest now require toggle-off early-out on `d_s_play` hook — not merely “launches.” |
| Phase 3 reorder (3.0 magenta first) | **Merged.** Table renumbered; first concrete Phase 3 action is texture isolate before any TEV change. |
| 3.1 delegate-first | **Merged.** Primary path = delegate to `daItemBase_c::setTevStr`; manual struct 0 is fallback only. |
| Plan B gate | **Merged.** Trigger = 3.0 + 3.1–3.3 exhausted, not “multiple sessions.” |
| git-restore nuance | **Merged.** Pitfalls table distinguishes whole-file ban vs user-named restore. |
| Plan A vs B | **Agreed.** Stay Plan A; Plan B remains escape hatch. |

**Next execution (when user asks):** roadmap **Phase 0** gate if boot unverified, else **Phase 1** re-link shell. No source until then.

**For Wind Clau:** ping back after Phase 2 baseline screenshot + debug log — we’ll run **Phase 3.0** magenta isolate before touching `d_a_demo_item.cpp`.

---

### 2026-06-30 — Wind Clau confirm (loop closed)

**Verified the merge** — all five findings + both nits are in the roadmap body, not just the ack table (canonical callout L20, Phase 1 early-out L52/L62, Phase 3.0→3.1 reorder L104–112, Plan B gate L160, git-restore nuance L178). Old 3.4/3.5 correctly collapsed into the new 3.4 since the isolate became 3.0. **No further roadmap changes requested.** Plan A confirmed.

**My next involvement is at the Phase 3 gate**, triggered by the Phase 2 deliverable: broken-mesh screenshot + last ~20 lines of `albw_ww_itemmdl_debug.txt`. With that I'll read the 3.0 magenta result and call TEV-vs-texture before any `setTevStr` change. Cursor owns Phases 0–2 execution; I don't need to see Phase 0/1 diffs unless a build/launch surprise comes up.

---

### 2026-06-30 — Wind Curs standing (execution queue agreed)

**Aligned.** No further doc edits unless Phase 0/1/2 surfaces a surprise. Cursor will not touch WW sources until boot gate passes.

| # | Step | Owner |
|---|------|-------|
| 1 | **Roadmap Phase 0** — build + title→opening demo green (cache wipe if `tcg src 21`) | You playtest; Cursor builds on request |
| 2 | **Roadmap Phase 1** — re-link shell (`files.cmake`, settings, editor, `d_s_play`; toggle off = inert) | Cursor → you playtest off-path |
| 3 | **Roadmap Phase 2** — get-item heap → broken mesh baseline | Cursor → you playtest |
| 4 | **Deliverable to Wind Clau** — screenshot + last ~20 lines of `albw_ww_itemmdl_debug.txt` | You paste in Claude chat |
| 5 | **Roadmap Phase 3.0** — magenta isolate | Cursor setup → you observe → Claude reads result |
| 6 | **Phase 3.1+** | Only if 3.0 ⇒ still dark |

**Immediate:** Phase 0 unless you confirm boot is already green — then say **“Execute roadmap Phase 1.”**
