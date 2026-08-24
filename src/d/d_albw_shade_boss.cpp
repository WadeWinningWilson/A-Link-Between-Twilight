#if TARGET_PC

// ============================================
// NEW CODE — ALBW Port — "Hero's Shade Secret Boss"
// Post-game real duel vs the Hero's Shade (d_a_npc_kn) after all Hidden Skills.
// See docs/heros-shade-secret-boss.md for the full design + implementation plan.
// This file starts as just the master toggle; boss/warp/spawn logic lands in
// later milestones.
// ============================================

#include "d/d_albw_shade_boss.h"
#include "d/d_albw_hp_mult.h"  // dAlbwHP_getLockonDisplayHp
#include "dusk/settings.h"
#include "f_op/f_op_actor_mng.h"  // fopAcM_SearchByName / fopAcM_IsActor
#include "f_pc/f_pc_name.h"       // fpcNm_NPC_KN_e
#include "m_Do/m_Do_audio.h"  // mDoAud_bgmStop

// ============================================
// Master toggle (game.heroShadeSecretBoss, default off — WIP)
// ============================================
bool dShadeBoss_isEnabled() {
    return dusk::getSettings().game.heroShadeSecretBoss.getValue();
}

// ============================================
// BGM suppression — Link's House basement (test wolf room)
// Called every frame from dScnRoom_Execute while the player is in R_SP01
// room 7 with the feature on. Instant stop (fade 0) so the shared
// Ordon-interior BGM cuts and stays silent; idempotent no-op once stopped, so
// it is safe to call per frame. Only the main BGM is affected — the wolf's
// panting and other SE keep playing.
// ============================================
void dShadeBoss_suppressHouseBgm() {
    mDoAud_bgmStop(0);
}

// ============================================
// Boss health bar query (combat Shade)
// The combat Shade is the only NPC_KN with HP (mType 7 sets health/field_0x560 =
// 300; lesson Shades never set HP), so `health > 0` uniquely identifies it — no
// mType coupling needed. HP goes through the same dAlbwHP source the lock-on
// debug + Armogohma bar use, so the mod's Boss HP scaler applies uniformly.
// ============================================
bool dShadeBoss_queryHealthBar(int* o_current, int* o_max) {
    if (o_current == NULL || o_max == NULL) {
        return false;
    }
    *o_current = 0;
    *o_max = 0;

    if (!dShadeBoss_isEnabled()) {
        return false;
    }

    fopAc_ac_c* actor = fopAcM_SearchByName(fpcNm_NPC_KN_e);
    if (actor == NULL || !fopAcM_IsActor(actor) || actor->health <= 0) {
        return false;
    }

    const dAlbwHP_LockonDisplay hp = dAlbwHP_getLockonDisplayHp(actor);
    *o_current = hp.current;
    *o_max = hp.max;
    return true;
}

#endif // TARGET_PC
