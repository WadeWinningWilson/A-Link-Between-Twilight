# NEVER-PUSH STRIP SET — covenant hard-gate (§113)

> ⛔ **Housing Security lane. This is the covenant's last line before a public
> push.** A bad merge is reversible (`git checkout pre-dusk-api-merge`); a bad
> **push is un-sendable** — donor bytes or a WW identity literal reaching the
> public `ALBW-Dusklight` remote cannot be recalled. Everything below stays
> **fork-local** and is **never staged for a push to public `main`** until it
> has passed an individual clean-slice promotion review.
>
> Authored by Housing Security on the merged tree (branch
> `integrate/dusk-api-coexist`), 2026-07-25 — the dusk-API coexist merge has
> landed, so §113's gate is now live, not anticipatory.

## Baseline (M6 gate run, this tree)

`strings dusklight.exe | grep -iE '\b(Ivan|Outset|Aryll|Tetra|Sturgeon|Windfall|Makar|Medli|WW-Crew|bmgres|Great Sea)\b'`
→ **0 hits, all patterns. Binary covenant CLEAN.** No WW identity/place literal
reaches the exe. The strip set exists to KEEP it that way across promotions.

---

## Tier 1 — Never-push SOURCE files (the WW receiver layer)

These 34 tracked files are the fork-local WW-restoration receiver layer. They
exist legitimately in the fork; they are **never promoted to public `main` as a
set.** A single file leaves this set only by a deliberate clean-slice review
(§5) — not by riding along in a broad push.

**Includes (18):**
`include/d/d_albw_dialogue.h` · `d_ext_fado_door.h` · `d_ext_mod_flags.h` ·
`d_ext_npc_doors.h` · `d_ext_npc_mount.h` · `d_ext_npc_population.h` ·
`d_ext_quick_equip.h` · `d_ext_save_guard.h` · `d_ext_seq_space.h` ·
`d_ext_status.h` · `d_ww_itemmdl_pc.h` · `d_ww_itemmdl_test.h` ·
`include/d/ext_seq/ja1_{bank,event_dump,parser,seq_ctrl,track}.h` ·
`include/dusk/custom_assets.hpp`

**Sources (16):**
`src/d/actor/d_a_ext_plank_span.cpp` · `d_a_ext_vegetation.cpp` ·
`src/d/d_albw_dialogue.cpp` · `d_ext_mod_flags.cpp` · `d_ext_npc_doors.cpp` ·
`d_ext_npc_mount.cpp` · `d_ext_npc_population.cpp` · `d_ext_seq_space.cpp` ·
`d_ww_itemmdl_pc.cpp` · `d_ww_itemmdl_test.cpp` ·
`src/d/ext_seq/ja1_{bank,event_dump,parser,seq_ctrl,track}.cpp` ·
`src/dusk/custom_assets.cpp`

**Why each cluster is covenant surface:**
- `d_ext_npc_*` / `d_a_ext_*` — the WW actor mount/population/door/vegetation
  system; names and drives WW-restoration content.
- `d_ww_itemmdl_*` — the WW held/get-item model pipeline (adapts WW arcs).
- `ext_seq/*` + `d_ext_seq_space` — the parked **audio-shadow / WW sequence**
  surface (§113 "parked audio-shadow/ext-seq").
- `custom_assets` — runtime WW arc adaptation (BDL4→BMD3 retag, audio redirect).
- `d_albw_dialogue` — **shared** with the ALBW postman, but currently drives WW
  NPC boxes; keep local until the WW usage is cleanly separable.

**NOT in this set (promotable — the public ALBW project, no WW content):** all
other `d_albw_*` (meter, combat, HUD, shop, outfit, wardrobe, wolf, potion,
boss, mail, …), the `src/dusk/mods/**` loader/SDK, `dvd_asset`. These are the
public upstream's own content; they are gated by ordinary readiness, **not** by
this covenant strip set.

## Tier 2 — Modified-TP files carrying WW references (the dangerous class)

These are **TP's own files**, edited for WW work — they look promotable but
carry WW identity/place references in comments or edits. They **cannot be
blanket-stripped** (they hold legitimate TP content). Rule: **scrub every WW
reference before ANY promotion**, and never promote the WW-specific edit blocks.

| File | Exposure (2026-07-25) |
|---|---|
| `src/d/actor/d_a_demo_item.cpp` | `Ivan`/`Outset` in comments (get-item kit path) |
| `src/d/actor/d_a_grass.cpp` | WW grass-interaction edits + refs |
| `src/d/actor/d_a_knob00.cpp` | WW door-knob edits + refs |
| `src/d/d_camera.cpp` | `Outset` / WW-space camera refs |
| `src/d/d_kankyo_rain.cpp` | `Outset` / WW env refs |

Re-run the locator before any promotion, it drifts:
`git grep -ilnE '\b(Ivan|Outset|Aryll|Tetra|Windfall|Sturgeon|Orca|Makar|Medli)\b' -- 'src/**' 'include/**' | grep -viE 'd_ext_|d_a_ext_|d_ww_itemmdl|custom_assets|ext_seq|d_albw'`

**The `Ivan` note:** `Ivan` is an invented internal WW-cast label ([[donor-export-naming]]
/ Ivan rule — no invented identity labels). It currently appears **only in
comments**, which the compiler strips, so it is exe-exempt per the covenant gate
(cookbook №119) and the M6 baseline is clean. It remains a **source-hygiene**
item: scrub it from any Tier-2 file before that file promotes to public source.

## Tier 3 — Data / arcs / mod folder

- **Repo audit: NO WW data is tracked.** No `.arc` / `.bdl` / `.dzb` / `.bmg` /
  WW dialogue catalog / `WW-Crew-Restoration` path is under git. (The
  `assets/DZDE01/res/Msg*/bmgres.h` files are **TP's own** stage headers, not WW.)
- The live mod folder `%AppData%/TwilitRealm/Dusklight/model_replacements/WW-Crew-Restoration`
  is a **separate local repo** (§108) — untouchable by the receiver merge and
  never part of a receiver push. WW arcs, `ww_dialogue_full.txt`, extracted
  assets, and `tools/ww_crew_restoration_skeleton` outputs live there, not here.
- Confirm before any push: `git ls-files | grep -iE '\.arc$|\.bdl$|\.dzb$|\.bmg$|ww_dialogue|Crew-Restoration'` → must stay **empty**.

---

## The M6 greplist gate (MANDATORY before any push)

Run on the exe **built from the exact tree about to be pushed** (a merge/reconfigure
can change what gets packaged — re-run, never trust a prior CLEAN):

```bash
EXE=build/windows-msvc-relwithdebinfo/dusklight.exe
strings -n 5 "$EXE" | grep -inE '\b(Ivan|Outset|Aryll|Tetra|Zelda-of-WW|Tetra|Sturgeon|Orca|Windfall|Makar|Medli|Valoo|Jabun|Great Sea|WW-Crew|bmgres|zel_00)\b'
```
- **CLEAN = zero hits.** Comments never reach the exe, so a hit means a WW name
  became a **string literal / arc name** — a hard covenant breach. Do not push.
- Shared TP/WW nouns (`Ganondorf`, `Bow`, `Bomb`, `Boomerang`, `Magic Armor`,
  `Link`, `Zelda`) are **exempt** (cookbook №119) — TP ships them natively. They
  are deliberately absent from the pattern above.

## Standing rule + verification procedure

1. **Push stays gated until: M6 CLEAN + Tier-3 empty + user's explicit go.** No
   AI instance self-clears this gate (mirrors [DO-NOT.md](DO-NOT.md) discipline).
2. Never `git add`/stage a Tier-1 file or a WW edit-block toward a public-`main`
   push. Promotion is per-slice (§5 mod-api-host-promote), reviewed, then the
   file leaves Tier 1 explicitly in this doc.
3. **Re-verify the strip set after ANY CMake reconfigure or further merge** —
   packaging can shift silently (§113.6). The M6 baseline is only valid for the
   tree it was run on.
4. Housing owns the gate RUN and this document; Housing **executes no push and
   no merge** (Engine/integration lane). Loss-protection (commit + tag) is the
   only git action in Housing's charter, and only on user request.

## Audit trail

- §113 spec: [WW Linked/ww-bridge-tool-interconnected.md](WW%20Linked/ww-bridge-tool-interconnected.md) §113; push gate: [state/mod-api-host-promote.md](state/mod-api-host-promote.md) §4.
- Pre-merge anchor (full-revert): tag `pre-dusk-api-merge` = `bfa264511c`.
- This strip set supersedes the §113 assessment-era "25 files" estimate with the
  **34** verified post-merge receiver files + the Tier-2 modified-TP class the
  estimate did not enumerate.
