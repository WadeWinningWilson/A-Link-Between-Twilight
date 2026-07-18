# Level Editor — live state

| Field | Value |
|-------|--------|
| **status** | **Gate 11b (Path A) — model-tight pick + per-joint highlight BUILT (uncommitted), awaiting USER playtest.** Playtest of 11a exposed root cause: cull box ≫ mesh (giant amber box), so ray-vs-cull-box wasn't pinpoint & highlight was a box. Fix: pick is now broad-phase cull reject → **narrow-phase per-joint OBB** (`model->getModelData()` joints, `getMin/getMax` + `getAnmMtx(i)`), so the hit volume hugs the mesh; highlight = **one translucent box per joint** (`draw_actor_volume_highlight`) instead of the oversized cull box (fallback = cull box for model-less/jointless actors). Builds clean, cache wiped. **HONEST CAVEAT (told user):** true per-pixel mesh *glow* isn't a cheap one-call tint here — it needs render work ≈ Path B silhouette; per-joint boxes are the tight interim. |
| **owner_impl** | Cursor (Auto — conservative) |
| **owner_review** | Claude |
| **interconnected_run** | [Level-Editor-Cursor-Claude.md](../Interconnected%20Chats/Level-Editor-Cursor-Claude.md) |
| **next** | Re-playtest Gate 10 after 10b hotfix (Select Mode off-center + sticky; no FATAL). No Gate 11 until fresh dual APPROVED. |
| **perf_bar** | ~250 / 270–300 fps; editor gated by `g_levelEditorSession` |
| **do_not** | 1c-drag / terrain / 11b mesh-tint before 11a signs; Phase 2 RARC |
| **drive_in_scope** | no |
| **updated** | 2026-07-18 |
| **detail** | [level-editor-phase1.md](../level-editor-phase1.md) |

---

## Milestone tip

| # | Milestone | State |
|---|-----------|--------|
| 1x / 1x.1 | Launch + deny Save | **Done** |
| 1a | Stage Inspector | **Done** (death-clear deferred) |
| 1b | Click-to-select in world | **Gate 11a ray pick — BUILT, user playtest** |
| 1c / 1d | Gizmo + project.json; place/delete | **Later** — session drag (`home.pos`) after 11a/11b sign |

## Gate 10 (pick accuracy) — impl landed

- **Fix:** min-`distSq` winner (not min-depth); 15px depth tie-break; hysteresis; radius 50/100; same hover+click
- **Diag:** `Pick crumb` + screen `+` pick-dot; ImGui source only on evidence
- **Bar (user):** off-center actor highlights; no flip-flop; pick-dot under cursor

## Terrain approach — evaluated vs north-star (2026-07-18)

**In-game editor = GMod model:** terrain read-only; place/move actors. Terrain edit = offline bake (Phase 2). See run doc §Terrain.

## Notes

- Deferred: death-clear highlight, Stage pane sync on world pick, flying Kargaroks, silhouette
