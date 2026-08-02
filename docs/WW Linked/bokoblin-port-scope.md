# Bokoblin (`d_a_bk`) AI port — scope + incremental plan (§221, History via P13)

The first enemy family, started from Foundry's `donor-statemap-bk.md`. This is a **multi-session** port;
this doc is the map + the ordered increments so each lands verified (P13 tap pair) before the next.

## Scale (why it's staged)

- **5,338 lines.** A **24-action master dispatcher** (`switch (dr.mAction)` @L3384, cases 0–31) over
  **23 sub-mode machines** (`switch (dr.mMode)` per action). Full combat enemy — multiple attack types
  (`attack1_info`/`jattack1_info`/`kattack1_info` = standard / jump / special), attack CC kinds
  (`bk_at_kind` = `dCcG_At_Spl`), attack SEs + attack-power tables.

## Dependency status (receiver, clean check)

| Present ✓ | Missing / partial ✗ |
|---|---|
| `dCc_`/`dCcD_`/`dCcG_At_Spl` (CC hit-detection + attack collision) · `dBgS_LinChk` (line collision) · `dBgW` (moveBG) · `mDoExt_McaMorf` (model/anim) · `fpcM_Search`/`fopAcM_searchPlayer` · `cLib_addCalc` · `dPa_control` (particles) · `Bk.arc` extracted | `dDamage_` / `fopDm_` (damage-manager tier) · `daEnemy` (enemy base class) · `dCcMassObj` |

**Read:** movement, search, collision, and *attack hit-detection* all port. **Damage/death** (taking
hits, HP, death throw) needs the damage-manager framework — port or adapt — so it comes AFTER the
non-combat + attack-motion increments.

## Incremental plan (each increment = extract-cited → implement → P13-tap-verify)

1. **Increment A — non-combat locomotion + awareness** (`mAction 0–3`, the `mMode 0/1/2` clusters):
   idle → patrol/wander → player-detect/alert. Deps all present. Turns the static `NPC_KB`-style mount
   into a moving, aware Bokoblin. **First build.**
2. **Increment B — attack motion** (`attack1/jattack1/kattack1`, `dCcG_At_Spl` attack CC — present):
   the swing/lunge with hit-collision active, but no player-damage bookkeeping yet (CC fires, damage
   tier stubbed). Verifies the attack *animation + timing + hitbox* against the statemap.
3. **Increment C — damage/death** (needs `dDamage_`/`fopDm_` + `daEnemy` framework): taking hits, HP,
   flinch, the death sequence. This is the framework-gated part — port the minimal damage tier the
   Bokoblin needs, or adapt the receiver's enemy-damage path.
4. **Increment D — special modes** (the `-10/-9/-20/-19/50–60` sub-modes: knockback, grab, alerted
   variants) as the statemap details them.

**Verification each increment:** `DUSK_*_TAP` on the Bokoblin action stream (mAction/mMode writes) →
`probe_differ seq` vs `donor-statemap-bk.md` — the mechanized-parity loop, applied to combat AI.

**Placement:** A_mori (`Bk`×3, the "founding quarry"), gated behind the A_mori beat; the Bokoblin spawns
static-mount today, so Increment A is the upgrade path.

**Status: STARTED — map + deps + plan locked; Increment A is the next build.**
