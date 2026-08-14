# QUEUE ITEM — tale §842 (Housing/Engine) — calm-map/windline fork-letter gates → native truths

**Protocol:** tale §839 (Integrator is the only builder). This item is QUEUED, not landed:
the five hunks below were briefly applied in-tree during the 23:1x pass, then REVERTED so the
tree stays at the last-built state (exe 23:01:10). Apply verbatim at build time; every hunk is
tagged `tale §842` for grep.

**What it fixes (evidence = run dusklight-20260812-230546.log):** the Fado crossing armed
native `sea` room 44 (fork-name retirement verified live) and the FILI wind receipt armed —
but NO `[WwFoam] §101 calm map` line fired. Root: the `stage[0]=='F'` fork-naming idiom
survives at the CALLERS after tale §835 retired it inside the wether loader. Three sites:
`d_ext_npc_mount.cpp:7945` (§97b/§101 arm), `d_kankyo_wether.cpp wwFoamFieldActive`,
`d_kankyo_wether.cpp wwWindlineHostActive`. Native `sea` matches none of them, so the re-keyed
`[sea]` calm map never loads and wind streaks stay dead on the native host.

**The native truths used (no invented names, №99 R2 kept — no WW strings in code):**
- foam/calm sites → "did this stage load a calm-map section" (data-side section presence,
  same law as tale §835's loader change; accessor `dKyw_wave_calm_isLoaded()`).
- windline (exterior tell) → the sky-host flag `dKyWw_isSkyHost()` — the §684 precedent: the
  single truth the stage's own vr_sky flips on declared native stages.

**Scope/DN-10:** modernizes the GATES of the existing §101 bridge only; bridge scope unchanged
and its label stands (native target = donor sea wave_max grid via GetArea, owed). All gates
remain inside `dExtWwSave_isWwHostStage` — mainline TP untouched (№282/№283).

---

## Hunk 1 — include/d/d_kankyo_wether.h (accessor declaration)

OLD:
```cpp
WAVE_INFO* const* dKyw_getWaveInfl();
void dKyw_wave_calm_onStage(const char* stage);
void dKyw_wave_calm_update();
```
NEW:
```cpp
WAVE_INFO* const* dKyw_getWaveInfl();
void dKyw_wave_calm_onStage(const char* stage);
// tale §842: true when the current stage loaded a calm-map section — the
// data-side "is this a foam-bearing host" tell (section presence, no name test).
bool dKyw_wave_calm_isLoaded();
void dKyw_wave_calm_update();
```

## Hunk 2 — src/d/d_kankyo_wether.cpp (accessor definition)

OLD:
```cpp
WAVE_INFO* const* dKyw_getWaveInfl() {
    return s_waveInflPtrs;
}
```
NEW:
```cpp
WAVE_INFO* const* dKyw_getWaveInfl() {
    return s_waveInflPtrs;
}

bool dKyw_wave_calm_isLoaded() {
    return s_calmLoaded;
}
```

## Hunk 3 — src/d/d_kankyo_wether.cpp (foam field gate)

OLD:
```cpp
static bool wwFoamFieldActive() {
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage);
}
```
NEW:
```cpp
static bool wwFoamFieldActive() {
    // ========================================================================
    // tale §842 (row 21 names): was `stage[0]=='F'` — the fork-naming idiom;
    // native `sea` never matched. Both callers gate calm-map behaviors, so the
    // truthful test is "did this stage load a calm map" (data-side, §835).
    // ========================================================================
    return s_calmLoaded;
}
```

## Hunk 4 — src/d/d_kankyo_wether.cpp (windline host gate)

OLD:
```cpp
static bool wwWindlineHostActive() {
    const char* stage = dComIfGp_getStartStageName();
    // Adaptation 2: exteriors only (F_*), same family as foam field gate.
    return stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage);
}
```
NEW:
```cpp
static bool wwWindlineHostActive() {
    // ========================================================================
    // tale §842 (row 21 names): was `stage[0]=='F'` (fork exteriors) — native
    // `sea` never matched, so wind streaks died with the fork. The native
    // exterior tell is the sky-host flag — the §684 precedent: the single
    // truth the stage's own vr_sky flips on declared native stages.
    // ========================================================================
    const char* stage = dComIfGp_getStartStageName();
    return stage != NULL && dExtWwSave_isWwHostStage(stage) && dKyWw_isSkyHost();
}
```

## Hunk 5 — src/d/d_ext_npc_mount.cpp (§97b/§101 arm site, ~line 7942)

OLD:
```cpp
    // §97b/§101: arm shore foam on WW field hosts; load package calm map.
    // flatInter polarity (donor pair): 0=calm/usonami ON, 1=chop/usonami OFF.
    // Ferry A STEP 1: waves bisect skips usonami arming for the run.
    if (stage != NULL && stage[0] == 'F' && dExtWwSave_isWwHostStage(stage) &&
        wwFpsBisectMode() != kWwFpsBisectSkipWaves) {
        dKy_usonami_set(0.0f);
        dKyw_wave_calm_onStage(stage);
        DuskLog.info("[WwFoam] §97b/§101 usonami armed on '{}'", stage);
    } else {
        g_env_light.mWaveChan.mWaveCount = 0;
        g_env_light.mWaveChan.mWaveFlatInter = 1.0f;
        dKyw_wave_calm_onStage(NULL);
        if (wwFpsBisectMode() == kWwFpsBisectSkipWaves) {
            DuskLog.info("[WwFoam] FerryA FPS_BISECT: usonami arm SKIPPED on '{}'",
                         stage != NULL ? stage : "?");
        }
    }
```
NEW:
```cpp
    // §97b/§101: arm shore foam on WW field hosts; load package calm map.
    // flatInter polarity (donor pair): 0=calm/usonami ON, 1=chop/usonami OFF.
    // Ferry A STEP 1: waves bisect skips usonami arming for the run.
    // ========================================================================
    // tale §842 (row 21 names, second site): the `stage[0] == 'F'` field-host
    // test was the SAME fork-naming idiom retired inside the wether loader
    // (tale §835) — it kept the native `sea` host in the else-branch, so the
    // re-keyed calm map never loaded (run 230546: FILI wind armed, no §101
    // line). Which hosts are foam-bearing is DATA now: load for any WW host,
    // then arm usonami only if a calm-map section actually loaded.
    // ========================================================================
    if (stage != NULL && dExtWwSave_isWwHostStage(stage) &&
        wwFpsBisectMode() != kWwFpsBisectSkipWaves) {
        dKyw_wave_calm_onStage(stage);
    } else {
        dKyw_wave_calm_onStage(NULL);
    }
    if (dKyw_wave_calm_isLoaded()) {
        dKy_usonami_set(0.0f);
        DuskLog.info("[WwFoam] §97b/§101 usonami armed on '{}'", stage);
    } else {
        g_env_light.mWaveChan.mWaveCount = 0;
        g_env_light.mWaveChan.mWaveFlatInter = 1.0f;
        if (wwFpsBisectMode() == kWwFpsBisectSkipWaves) {
            DuskLog.info("[WwFoam] FerryA FPS_BISECT: usonami arm SKIPPED on '{}'",
                         stage != NULL ? stage : "?");
        }
    }
```

---

**Residual fork-letter sites, inventoried NOT edited (per-site semantics differ; each needs its
own ruling before a gate swap):**
- `d_ext_npc_mount.cpp:7929` — §266 interior-arrival ClrWallNone, `stage[0]=='R'`. Native
  interiors (LinkRM/Ojhous/…) never match. Deliberately left: №161's CrrPos freeze has not been
  observed on native interior arrivals; if Link ever freezes entering a native interior, this is
  the site.
- `d_ext_npc_mount.cpp:2796` — 'F' field helper inside mount-era machinery (inert for native).
- `d_ext_seq_space.cpp:429` — `stage[0]=='R'` interior test (seq/space kit; needs its own pass).
- `d_a_alink.cpp:9400/9402` — player load-phase letter tests (player lifecycle; highest risk,
  needs its own ruling).
