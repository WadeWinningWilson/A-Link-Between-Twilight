#if TARGET_PC

#include "d/d_albw_combat.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_b_tn.h"
#include "dusk/trace_noop.h"
#include "dusk/settings.h"
#include "f_op/f_op_actor.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"

bool dAlbw_isHiddenSkillReworkEnabled() {
    // Core ALBW combat (HS meter costs, JS charge, Helm punish, etc.) — always on; not exposed in UI.
    return true;
}

static bool s_jumpStrikeChargeReady = false;

void dAlbw_resetJumpStrikeChargeReady() {
    s_jumpStrikeChargeReady = false;
    CONAV_LOG("jsready", "reset");
}

void dAlbw_setJumpStrikeChargeReady() {
    if (!s_jumpStrikeChargeReady) {
        s_jumpStrikeChargeReady = true;
        CONAV_LOG("jsready", "set ready=1");
    }
}

bool dAlbw_isJumpStrikeChargeReady() {
    if (!dAlbw_isHiddenSkillReworkEnabled()) {
        return true;
    }

    return s_jumpStrikeChargeReady;
}

bool dAlbw_peekJumpStrikeChargeReady() {
    return s_jumpStrikeChargeReady;
}

bool dAlbw_tryConsumeJumpStrikeChargeReady() {
    if (!dAlbw_isHiddenSkillReworkEnabled()) {
        return true;
    }

    if (!s_jumpStrikeChargeReady) {
        CONAV_LOG("jsready", "consume denied ready=0");
        return false;
    }

    s_jumpStrikeChargeReady = false;
    CONAV_LOG("jsready", "consume ok");
    return true;
}

// ============================================
// Guard-opener classification. Disambiguation is the whole job here:
//  - Hurricane shares CUT_TYPE_LARGE_TURN_* with Great Spin, so the check
//    keys on the PROC_CUT_GS_HURRICANE(_TIRED) proc — Great Spin stays a
//    clank by design.
//  - The Combat Howl AOE rides AT_TYPE_WOLF_CUT_TURN on the ALINK collider,
//    the same AT type as the ordinary wolf spin — mWolfCombatHowlActive
//    disambiguates (Link cannot wolf-spin mid-howl).
//  - The Midna arm is its own actor, so its name is sufficient.
// ============================================
bool dAlbwCombat_isGuardOpenerHit(cCcD_Obj* i_hitObj) {
    if (i_hitObj == NULL) {
        return false;
    }

    fopAc_ac_c* attacker = i_hitObj->GetAc();
    if (attacker == NULL) {
        return false;
    }

    const s16 name = fopAcM_GetName(attacker);
    if (name == fpcNm_ALBW_MIDNA_ARM_e) {
        return true;
    }
    if (name != fpcNm_ALINK_e) {
        return false;
    }

    const daAlink_c* link = daAlink_getAlinkActorClass();
    if (link == NULL) {
        return false;
    }

    if (link->mWolfCombatHowlActive && i_hitObj->ChkAtType(AT_TYPE_WOLF_CUT_TURN)) {
        return true;
    }

    return link->mProcID == daAlink_c::PROC_CUT_GS_HURRICANE ||
           link->mProcID == daAlink_c::PROC_CUT_GS_HURRICANE_TIRED;
}

dAlbwHelmBashTier dAlbwCombat_getHelmBashTier(fopAc_ac_c* i_actor) {
    if (i_actor == NULL || fopAcM_GetGroup(i_actor) != fopAc_ENEMY_e) {
        return dAlbwHelmBash_THRESHOLD;
    }

    if (fopAcM_GetName(i_actor) == fpcNm_B_TN_e) {
        const daB_TN_c* darknut = (const daB_TN_c*)i_actor;
        if (darknut->albwIsUnarmoredPhase()) {
            return dAlbwHelmBash_THRESHOLD;
        }

        return dAlbwHelmBash_MAX;
    }

    return dAlbwHelmBash_THRESHOLD;
}

#endif // TARGET_PC
