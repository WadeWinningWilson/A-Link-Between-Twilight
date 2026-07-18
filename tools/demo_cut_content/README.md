# Demo cut-content restore toolkit

Reproducible mine → catalog → mount path for leftovers trapped in retail `Demo*.arc`
cutscene archives (the same trail that produced `D:/XXXXXXX/beta link/Kmdl.arc`).

## Why Demo arcs

Retail `Kmdl.arc` is final Hero’s Clothes. Beta / trailer-era Link (and companions)
live inside **cutscene** packs:

| Source | What’s there |
|---|---|
| `Demo01_00.arc` | Beta Link `…bd_original_o` / `…bd_high_o`, beta boar `…wb_…original…`, Hyrule knight `…ctz1…` |
| `Demo04_01.arc` | First-transform faces: `…henkeiface…` (primitive/trailer) vs `…hiface…` (final) + `demo00_link_*_tmp` ancestry |
| Many later demos | Repeated `demo00_*_tmp` Midna/Link scraps — Demo00 era crumbs, not a missing `Demo00_01.arc` on GZ2E01 |

The playable pack at `D:/XXXXXXX/beta link/Kmdl.arc` is an **adapted remount**
(same member names as retail `Kmdl`, different BMD sizes; **zero** SHA matches to
raw demo extracts). Raw demo bodies are not drop-in for gameplay.

## Reproduce (Windows)

```bat
REM 1) Mine all 89 Demo arcs on the TP extract
python tools/demo_cut_content/scan_demo_leftovers.py --extract

REM 2) Stage the proven adapted pack for Custom Models (Layer A)
python tools/demo_cut_content/stage_beta_kmdl_pack.py
```

Defaults:

- Object dir: `D:/XXXXXXX/Ex TP/files/res/Object`
- Beta proof: `D:/XXXXXXX/beta link/Kmdl.arc`
- Reports: `tools/demo_cut_content/out/DEMO_LEFTOVERS.md` (+ CSV/JSON)
- Extracts: `tools/demo_cut_content/out/extract/`
- Pack: `tools/demo_cut_content/out/packs/Beta Link (Demo Restore)/`

Copy the pack folder into `<config>/model_replacements/` and enable **Custom Models**.

## Restore tiers

1. **Mount adapted pack** — `stage_beta_kmdl_pack.py` (playable today via Custom Models).
2. **In-game Demo Leftover Viewer** — Editor → **ALBW** → category filter over all **365** unique Demo BMDs → **Spawn demo model at feet**. Regen catalog: `python tools/demo_cut_content/gen_demo_catalog.py`. Backend: `d_demo_leftover_viewer.cpp`.
3. **Cut Actors** — same ALBW tab: spawn `E_ms` / `E_dt` / titan / stubs via `d_cut_actor_spawn.cpp` (`fopAcM_create`).
4. **Re-adapt keepers** — Blender/J3D into `Kmdl` / real actors as needed.
5. **Level editor place** — later: shared prop spawn + click-to-place.

## Related archaeology

- `docs/TPHistory.md` — WW orphans (`itemmdl`), stub RELs, unused rooms
- Pointed negatives: no retail “beta forest” stage folder; axe trailer Moblin ≠ Ook mesh lineage
