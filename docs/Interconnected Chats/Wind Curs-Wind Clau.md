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
| 2.1 | Surgical branch in `daItemBase::CreateItemHeap` / static table: bow + toggle → arc `"itemmdl"`, BDL index `dRes_INDEX_ITEMMDL_BDL_VBOW_e` (0xF) |
| 2.2 | Ensure PC load uses `d_ww_itemmdl_pc` path: `J3DMLF_DoBdlMaterialCalc`, locked baked-DL draw (**no** unlock/`diff()`) |
| 2.3 | Edge pass: hide **`SC_Vbow_v` at draw time only** — never hide both materials on shared `J3DModelData` at create |
| 2.4 | Replay helper: message timing in `d_ww_itemmdl_test.cpp` only — **no** demo.inc message skip |

**Debug:** `Documents/dusklight/albw_ww_itemmdl_debug.txt`

**Playtest checklist:**

- [ ] Toggle off → vanilla `O_gD_bow`
- [ ] Toggle on → WW `vbow` visible (broken colors OK)
- [ ] No crash after “demo item ready”
- [ ] Toggle off again → vanilla

**Claude review prompt:** Paste screenshot (broken mesh) + last 20 lines of debug log; ask for smallest Phase 3 experiment order.

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
| 2026-06-30 | **Phase 2** | **Done (Cursor)** | CreateItemHeap + demo item arc/draw → itemmdl/vbow when toggle on. User playtest pending. |

---

## Claude review / revisions

<!-- Wind Clau: append review comments, revised experiment order, or Plan A vs B recommendation here. -->

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
