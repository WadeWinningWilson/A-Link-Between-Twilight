# WW plugin Outset — live state (Engine / Bridge)

era: plugin-delivery
<!-- era rationale: stock vanilla + ww_donor_disc.dusk + user ISO; no vanilla src edits -->

**Tip.** Product: **dusklight-main** + dusk + GZLE01. Warp sea **44 / 5**. Not the fork exe.

| Field | Value |
|-------|--------|
| **status** | **mode 4 FAIL** (`144315`). Plugin is on **WW-GC BDL path**; working **fork** uses a different PC consume path (below). Kill = ≥~3 UpdateDL. |
| **next** | Align Alwd consume with fork ExtNpc finish **or** prove Pale can keep MDL3. Track UpdateDL/Diff. No private ModelData. |
| **do-not** | Invent private ModelData copies. Sunburst. Fork exe. Mount. |
| **updated** | 2026-08-19 |

## Fork vs plugin (Lwood) — tracked gaps

Fork Lwood: `dExtNpcMount_acquireModelDataByIndex` → `acquireMountedModel` → `finishMountedModelData`. **Not** WW `d_resorce` `0x2020`.

| Step | Fork (works) | Plugin now | Gap? |
|------|--------------|------------|------|
| Acquire | consume-time cache + pristine raw copy | `wwParseModelOnce` on dRes buffer | yes (lifetime/pristine) |
| BDL flags | `0x59022010` (`0x59020010\|0x2000`) | **`0x00002020`** (WW donor) | **yes** |
| MDL3 on PC | **`break` — skipped** (`J3DModelLoader` TARGET_PC) | Pale **reads MDL3** (stock) | **yes — primary** |
| MAT3 | `WwFullMat3Scope` → **force full** `readMaterial` (type 0) | `0x2020` → **patched** MAT3 then MDL3 upgrade | **yes** |
| Finish | `change` + `MaterialAnm` + `newSharedDL` + `simpleCalc` + `makeSharedDL` + lightMask `0x01` | setToonTex; **skip** finish if SharedDL present | **yes** |
| Toon | only demo path gets tev3.a; Lwood finish path does **not** call full WW setToonTex | `wwSetToonTex` (+ texture patch) | plugin-extra |
| Create | `0x80000`, `0x11000022` | same | no |
| Draw | tev + `setListBG` + `modelUpdateDL` + `setList` | mode4: lists+UpdateDL, tev off | bisect only |
| Leaf sway | joint CB on | HELD | held (not kill) |

Fork comment on MDL3 skip: *"WW BDL4 MDL3 … port's material-DL walker cannot safely consume."* Plugin is still consuming MDL3; fork abandoned that on PC and rebuilds DLs from full MAT3.

DN-10 note: fork path is **receiver translation at load** (step 2), not WW-byte identity. Plugin step-1 (WW `0x2020`) loads clean but Diff/UpdateDL dies — consistent with MDL3 being the Pale poison the fork already named.

## UpdateDL / Diff (both trees)

Pale `mDoExt_modelUpdateDL`:

```
SharedDL && !isLocked  →  calc() + modelDiff (calcMaterial/diff/entry)
else                   →  unlock/update/lock
```

WW donor gates Diff on `modelDataType==1 && !isLocked` (same branch for Alwd).

| Build | After load | UpdateDL branch | Result |
|-------|------------|-----------------|--------|
| Plugin `144315` `0x2020` | SharedDL yes, `locked:0`, **Patched+MDL3 DL** | **Diff** | die @ draw n≈3 |
| Plugin `0x1010`+finish | SharedDL yes, `locked:1` | **update/lock** | also died |
| Fork ExtNpc Lwood | SharedDL yes, `locked:0`, **full MAT3 rebuilt DL** | **Diff** | survives |
| Plugin mode5 | one Diff/submit total | Diff once | PASS |

So Diff itself is not forbidden — fork Diffs every frame. The toxic payload is **what** Diff submits (MDL3 / patched SharedDL on Pale), not the branch choice alone.

Create with SharedDL+!locked promotes to **DifferedDLBuffer** + `newDifferedDisplayList(0x11000022)` — fork and plugin both do that for unlocked finished BDLs.

## Investigation (`144315`) — corrected

| Claim (pre) | Fact |
|-------------|------|
| `finish:1` ⇒ TP `newSharedDL` ran | **False.** Zero `model_finish` in `144315`. |
| `locked:0` ⇒ loader failed | **False.** Expected for MAT3→MDL3 else. |
| SharedDL missing | **False.** Finish gate skipped ⇒ present. |

## Log analysis (binding)

| Boot | What it proved |
|------|----------------|
| `133253` mode1 | no submit → PASS |
| `134723` mode5 | **one** lists+UpdateDL → PASS |
| `135122`/`140926` mode4 | multi UpdateDL → FAIL |
| `141831` private ModelData | still FAIL @ n=3 |
| `144315` `0x2020` | WW load OK; still FAIL @ n=3 |

## Holds

Leaf sway · Sunburst · Wave B · NPCs.
