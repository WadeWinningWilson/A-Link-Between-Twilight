# WW wind effects (ALL islands) — the visible wind streaks

**Island-generic** (believed universal until proven otherwise): the white wind streaks that drift
across the Great Sea / islands. Kankyo-driven (`WINDEFF_SET`), FILI-gated per stage. Identification
done (bus §130 HUNT 2); **not yet built — parked behind shore work.** Not currently in our build.

## Mechanism — [verified]
- **Emitter/particle:** `ID_AK_JN_WINDLINE00 = 0x0031` — spawned at `d_kankyo_rain.cpp:369`
  `dComIfGp_particle_set(ID_AK_JN_WINDLINE00, &pos)`.
- **Manager:** `WINDEFF_SET` (`d_kankyo_wether.cpp:335`) — up to 30 emitters
  (`mWindEff[30]`); active count = `mWindlineCount`.
- **GATE — FILI, per stage:** `dStage_FileList_dt_ChkPathWindEffect(fili)` (`d_kankyo_wether.cpp:410`)
  — a stage only runs path wind if its FILI enables it. So this is per-space DATA, not automatic.
- Depends on kankyo wind being live (same wind system as grass sway — see grass-effects.md §1).

## Feasibility — MEDIUM (particle path proven)
The WW JPA particle path is proven in-build (§95b — the grass-cut scatter `0x89D7` already loads via
`Pscene011.jpc`). So wind streaks need:
1. `ID_AK_JN_WINDLINE00 = 0x0031` present in a shipped Pscene bank (offline bank scan, same method
   that confirmed 0x89D7);
2. the host stage's FILI carrying the path-wind-effect flag (or our mount forcing it);
3. kankyo wind live on the host stage.
No new engine system expected — it's a particle-set + FILI flag + kankyo wind, all existing surfaces.

## Status / routing
- LOCATED (§130), not built. Parked behind shore-motion (§128).
- **Bridge (when unparked):** confirm `0x0031` in a shipped Pscene bank (bank scan).
- **Engine:** set the emitter on the host stage (FILI path-wind flag + kankyo wind), like the
  grass-cut particle precedent.
- **Housing:** covenant-neutral. No gate.
Cross-refs: bus §95/§95b/§130 · [grass-effects.md](grass-effects.md) (shared kankyo wind) ·
[shared-libraries.md](shared-libraries.md).
