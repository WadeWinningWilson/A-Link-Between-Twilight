/**
 * d_albw_flurry_rush.cpp — ALBW Flurry Rush state module (Phase 1–5).
 */

#if TARGET_PC

#include "d/d_albw_flurry_rush.h"
#include "d/d_focused_arts.h"
#include "d/d_com_inf_game.h"
#include "d/d_attention.h"
#include "d/d_item_data.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_e_oc.h"
#include "d/actor/d_a_player.h"
#include "dusk/settings.h"
#include "dusk/sim_time_scale.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_name.h"

#include <chrono>
#include <cstdio>

namespace {

constexpr float kFlurryStartGateRealSeconds = 2.0f;
constexpr float kFlurrySimTimeScale = 0.1f;
constexpr float kFlurryChainGateWidthFrames = 8.0f;

struct FlurryRushState {
    bool active = false;
    dFlurryRushMode mode = dFlurryRushMode_None;
    fopAc_ac_c* target = nullptr;
    fpc_ProcID targetId = fpcM_ERROR_PROCESS_ID_e;
    dFlurryRushSwordProfile profile = dFlurryRushProfile_Unknown;
    std::chrono::steady_clock::time_point startDeadline{};
    bool startGateArmed = false;
    bool hasStartedAttack = false;
    int hitCount = 0;
    bool pendingPerfectDodge = false;
    dFlurryPerfectDodgeKind pendingDodgeKind = dFlurryPerfectDodge_SideStep;
    char lastEvent[96]{};
    char lastDodgeAttempt[96]{};
};

FlurryRushState s_state;

void logFlurryEvent(const char* i_msg) {
    std::snprintf(s_state.lastEvent, sizeof(s_state.lastEvent), "%s", i_msg);
}

void logDodgeAttempt(const char* i_msg) {
    std::snprintf(s_state.lastDodgeAttempt, sizeof(s_state.lastDodgeAttempt), "%s", i_msg);
}

dFlurryMeleeTelegraphAxis queryActorMeleeTelegraph(fopAc_ac_c* i_actor) {
    if (i_actor == nullptr || fopAcM_GetName(i_actor) != fpcNm_E_OC_e) {
        return dFlurryTelegraph_None;
    }

    const daE_OC_c* oc = static_cast<daE_OC_c*>(i_actor);
    switch (oc->queryFlurryMeleeTelegraph()) {
    case daE_OC_c::FlurryTelegraph_Vertical:
        return dFlurryTelegraph_Vertical;
    case daE_OC_c::FlurryTelegraph_Horizontal:
        return dFlurryTelegraph_Horizontal;
    default:
        return dFlurryTelegraph_None;
    }
}

dFlurryRushSwordProfile swordProfileFromEquip() {
    const u8 sword = dComIfGs_getSelectEquipSword();
    if (sword == dItemNo_WOOD_STICK_e) {
        return dFlurryRushProfile_Wood;
    }
    if (sword == dItemNo_SWORD_e) {
        return dFlurryRushProfile_Ordon;
    }
    if (sword == dItemNo_MASTER_SWORD_e) {
        return dFlurryRushProfile_Master;
    }
    if (sword == dItemNo_LIGHT_SWORD_e) {
        return dFlurryRushProfile_Light;
    }
    return dFlurryRushProfile_Unknown;
}

dFlurryRushProfile profileTable(dFlurryRushSwordProfile i_profile) {
    switch (i_profile) {
    case dFlurryRushProfile_Wood:
        return {0, 0, 10};
    case dFlurryRushProfile_Ordon:
        return {2, 2, 7};
    case dFlurryRushProfile_Master:
    case dFlurryRushProfile_Light:
        return {3, 3, 5};
    default:
        return {0, 0, 0};
    }
}

bool canSpendOnPerfectDodge() {
    const dFlurryRushSwordProfile swordProfile = swordProfileFromEquip();
    if (swordProfile == dFlurryRushProfile_Unknown) {
        return false;
    }

    const dFlurryRushProfile profile = profileTable(swordProfile);
    // Future: wood profile also requires sumo outfit (getSumouMode()); not enforced during FlurryTEST.
    return dFocusedArts_canPerfectDodgeSpend(profile.spendGate, profile.barCost);
}

void refreshStartDeadline() {
    s_state.startDeadline =
        std::chrono::steady_clock::now() +
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(
            std::chrono::duration<float>(kFlurryStartGateRealSeconds));
}

void armStartGate() {
    if (!s_state.active || s_state.hasStartedAttack) {
        return;
    }

    s_state.startGateArmed = true;
    refreshStartDeadline();
}

bool beginInternal(dFlurryRushMode i_mode, fopAc_ac_c* i_target) {
    if (!dFlurryRush_isEnabled() || s_state.active || i_target == nullptr) {
        return false;
    }

    s_state.active = true;
    s_state.mode = i_mode;
    s_state.target = i_target;
    s_state.targetId = fopAcM_GetID(i_target);
    s_state.profile = swordProfileFromEquip();
    s_state.hitCount = 0;
    s_state.hasStartedAttack = false;
    s_state.startGateArmed = false;
    s_state.pendingPerfectDodge = false;

    char buf[96];
    std::snprintf(buf, sizeof(buf), "begin %s profile=%s",
                  i_mode == dFlurryRushMode_AerialBow ? "aerial bow" : "melee",
                  dFlurryRush_getProfileLabel(s_state.profile));
    logFlurryEvent(buf);
    dusk::setSimTimeScale(kFlurrySimTimeScale);
    return true;
}

void refreshTargetPointer() {
    if (s_state.target != nullptr && fopAcM_IsActor(s_state.target)) {
        return;
    }
    s_state.target = fopAcM_SearchByID(s_state.targetId);
}

}  // namespace

bool dFlurryRush_isEnabled() {
    const auto& settings = dusk::getSettings().game;
    return settings.focusedArtsTest.getValue() && settings.flurryRush.getValue();
}

bool dFlurryRush_canOfferPerfectDodgeSpend() {
    if (!dFlurryRush_isEnabled()) {
        return false;
    }

    return canSpendOnPerfectDodge();
}

dFlurryRushProfile dFlurryRush_getProfile(dFlurryRushSwordProfile i_profile) {
    return profileTable(i_profile);
}

dFlurryRushSwordProfile dFlurryRush_getEquippedSwordProfile() {
    return swordProfileFromEquip();
}

const char* dFlurryRush_getProfileLabel(dFlurryRushSwordProfile i_profile) {
    switch (i_profile) {
    case dFlurryRushProfile_Wood:
        return "Wood";
    case dFlurryRushProfile_Ordon:
        return "Ordon";
    case dFlurryRushProfile_Master:
        return "Master";
    case dFlurryRushProfile_Light:
        return "Light";
    default:
        return "Unknown";
    }
}

const char* dFlurryRush_getModeLabel(dFlurryRushMode i_mode) {
    switch (i_mode) {
    case dFlurryRushMode_Melee:
        return "melee";
    case dFlurryRushMode_AerialBow:
        return "aerial bow";
    default:
        return "none";
    }
}

bool dFlurryRush_beginMelee(fopAc_ac_c* i_target) {
    return beginInternal(dFlurryRushMode_Melee, i_target);
}

bool dFlurryRush_beginAerialBow(fopAc_ac_c* i_target) {
    return beginInternal(dFlurryRushMode_AerialBow, i_target);
}

void dFlurryRush_end(dFlurryRushEndReason i_reason) {
    if (!s_state.active) {
        return;
    }

    const char* reasonText = "ended";
    switch (i_reason) {
    case dFlurryRushEnd_StartGateExpired:
        reasonText = "start gate expired";
        break;
    case dFlurryRushEnd_HitCap:
        reasonText = "hit cap";
        break;
    case dFlurryRushEnd_TargetLost:
        reasonText = "target lost";
        break;
    case dFlurryRushEnd_Interrupt:
        reasonText = "interrupt";
        break;
    case dFlurryRushEnd_EquipChange:
        reasonText = "equip change";
        break;
    case dFlurryRushEnd_Debug:
    default:
        reasonText = "debug end";
        break;
    }

    char buf[96];
    std::snprintf(buf, sizeof(buf), "end (%s) hits=%d", reasonText, s_state.hitCount);
    logFlurryEvent(buf);

    s_state = FlurryRushState{};
    dusk::setSimTimeScale(1.0f);
}

void dFlurryRush_update() {
    if (!s_state.active) {
        return;
    }

    refreshTargetPointer();
    if (s_state.target == nullptr) {
        dFlurryRush_end(dFlurryRushEnd_TargetLost);
        return;
    }

    dAttention_c* attn = dComIfGp_getAttention();
    if (attn != nullptr) {
        attn->keepLock(30);
    }

    daAlink_c* link = daAlink_getAlinkActorClass();
    const u16 linkProc = link != nullptr ? link->mProcID : 0;

    if (!s_state.hasStartedAttack) {
        if (link != nullptr && s_state.mode == dFlurryRushMode_Melee) {
            if (linkProc == daAlink_c::PROC_SIDESTEP || linkProc == daAlink_c::PROC_BACK_JUMP) {
                if (link->swordSwingTrigger() && !link->manualShieldBlocksSwordInput()) {
                    link->onNoResetFlg2(daPy_py_c::FLG2_COMBO_RESERB);
                }
            }
        }

        if (s_state.startGateArmed &&
            std::chrono::steady_clock::now() >= s_state.startDeadline)
        {
            dFlurryRush_end(dFlurryRushEnd_StartGateExpired);
            return;
        }
    }

    if (s_state.hasStartedAttack) {
        const dFlurryRushProfile profile = profileTable(s_state.profile);
        if (profile.maxHits > 0 && s_state.hitCount >= profile.maxHits) {
            dFlurryRush_end(dFlurryRushEnd_HitCap);
            return;
        }
    }

    if (s_state.mode == dFlurryRushMode_Melee && s_state.hasStartedAttack) {
        if (link != nullptr && linkProc != daAlink_c::PROC_FLURRY_RUSH) {
            dFlurryRush_end(dFlurryRushEnd_Interrupt);
        }
    }
}

bool dFlurryRush_isActive() {
    return s_state.active;
}

fopAc_ac_c* dFlurryRush_getTargetActor() {
    if (!s_state.active) {
        return nullptr;
    }

    refreshTargetPointer();
    return s_state.target;
}

bool dFlurryRush_isTargetActor(fopAc_ac_c* i_actor) {
    if (!s_state.active || i_actor == nullptr) {
        return false;
    }

    refreshTargetPointer();
    return s_state.target == i_actor;
}

void dFlurryRush_cancelOnSwordEquipChange() {
    if (!s_state.active) {
        return;
    }

    daAlink_c* link = daAlink_getAlinkActorClass();
    if (link != nullptr && link->mProcID == daAlink_c::PROC_FLURRY_RUSH) {
        link->flurryExitToWait(dFlurryRushEnd_EquipChange);
        return;
    }

    dFlurryRush_end(dFlurryRushEnd_EquipChange);
}

dFlurryRushMode dFlurryRush_getMode() {
    return s_state.mode;
}

f32 dFlurryRush_getTimeScale() {
    return s_state.active ? kFlurrySimTimeScale : 1.0f;
}

bool dFlurryRush_shouldSuppressAlbwSpend() {
    return s_state.active;
}

void dFlurryRush_onMeleeProcEntered() {
    if (!s_state.active || s_state.mode != dFlurryRushMode_Melee || s_state.hasStartedAttack) {
        return;
    }

    logFlurryEvent("melee proc entered (snap pending)");
}

void dFlurryRush_onSnapToTargetComplete() {
    if (!s_state.active || s_state.mode != dFlurryRushMode_Melee || s_state.hasStartedAttack) {
        return;
    }

    if (s_state.startGateArmed) {
        return;
    }

    armStartGate();
    logFlurryEvent("snap complete (start gate armed)");
}

void dFlurryRush_onAttackStarted() {
    if (!s_state.active || s_state.hasStartedAttack) {
        return;
    }

    s_state.hasStartedAttack = true;
    logFlurryEvent("attack started (start gate cleared)");
}

void dFlurryRush_onHitLanded() {
    if (!s_state.active || !s_state.hasStartedAttack) {
        return;
    }

    s_state.hitCount++;
    char buf[48];
    std::snprintf(buf, sizeof(buf), "hit %d", s_state.hitCount);
    logFlurryEvent(buf);
}

f32 dFlurryRush_getChainGateWidthFrames() {
    return kFlurryChainGateWidthFrames;
}

void dFlurryRush_onChainGateMissed() {
    logFlurryEvent("chain gate missed");
}

bool dFlurryRush_debugBeginMeleeOnLockTarget() {
    if (!dFlurryRush_isEnabled() || s_state.active) {
        return false;
    }

    dAttention_c* attn = dComIfGp_getAttention();
    if (attn == nullptr) {
        logFlurryEvent("debug begin failed (no attention)");
        return false;
    }

    fopAc_ac_c* target = attn->LockonTarget(0);
    if (target == nullptr) {
        logFlurryEvent("debug begin failed (no lock target)");
        return false;
    }

    if (!dFlurryRush_beginMelee(target)) {
        return false;
    }

    // Debug skips perfect dodge; snap proc still arms gate on arrival (or immediately if in range).
    return true;
}

float dFlurryRush_getStartGateRemainingSeconds() {
    if (!s_state.active || s_state.hasStartedAttack || !s_state.startGateArmed) {
        return 0.0f;
    }

    const auto now = std::chrono::steady_clock::now();
    if (now >= s_state.startDeadline) {
        return 0.0f;
    }

    const auto remaining = std::chrono::duration<float>(s_state.startDeadline - now);
    return remaining.count();
}

int dFlurryRush_getHitCount() {
    return s_state.hitCount;
}

bool dFlurryRush_hasStartedAttack() {
    return s_state.hasStartedAttack;
}

const char* dFlurryRush_getLastEventText() {
    return s_state.lastEvent;
}

const char* dFlurryRush_getTelegraphLabel(dFlurryMeleeTelegraphAxis i_axis) {
    switch (i_axis) {
    case dFlurryTelegraph_Vertical:
        return "vertical";
    case dFlurryTelegraph_Horizontal:
        return "horizontal";
    default:
        return "none";
    }
}

const char* dFlurryRush_getDodgeKindLabel(dFlurryPerfectDodgeKind i_kind) {
    switch (i_kind) {
    case dFlurryPerfectDodge_SideStep:
        return "sidestep";
    case dFlurryPerfectDodge_BackJump:
        return "backflip";
    default:
        return "unknown";
    }
}

dFlurryMeleeTelegraphAxis dFlurryRush_queryLockTargetTelegraph() {
    if (!dFlurryRush_isEnabled()) {
        return dFlurryTelegraph_None;
    }

    dAttention_c* attn = dComIfGp_getAttention();
    if (attn == nullptr) {
        return dFlurryTelegraph_None;
    }

    return queryActorMeleeTelegraph(attn->LockonTarget(0));
}

bool dFlurryRush_hasPendingPerfectDodge() {
    return s_state.pendingPerfectDodge;
}

dFlurryPerfectDodgeKind dFlurryRush_getPendingDodgeKind() {
    return s_state.pendingDodgeKind;
}

bool dFlurryRush_tryEnterProcFromPerfectDodge() {
    if (!s_state.active || s_state.mode != dFlurryRushMode_Melee || !s_state.pendingPerfectDodge) {
        return false;
    }

    s_state.pendingPerfectDodge = false;
    return true;
}

const char* dFlurryRush_getLastDodgeAttemptText() {
    return s_state.lastDodgeAttempt;
}

bool dFlurryRush_tryPerfectDodge(dFlurryPerfectDodgeKind i_kind) {
    if (!dFlurryRush_isEnabled()) {
        return false;
    }

    if (s_state.active) {
        logDodgeAttempt("ignored (rush already active)");
        return false;
    }

    daPy_py_c* player = daPy_getPlayerActorClass();
    if (player == nullptr) {
        logDodgeAttempt("failed (no player)");
        return false;
    }

    if (player->checkWolf()) {
        logDodgeAttempt("failed (wolf form)");
        return false;
    }

    if (!player->checkSwordGet()) {
        logDodgeAttempt("failed (no sword)");
        return false;
    }

    if (!player->checkAttentionLock()) {
        logDodgeAttempt("failed (no Z-lock)");
        return false;
    }

    dAttention_c* attn = dComIfGp_getAttention();
    fopAc_ac_c* target = attn != nullptr ? attn->LockonTarget(0) : nullptr;
    if (target == nullptr) {
        logDodgeAttempt("failed (no lock target)");
        return false;
    }

    const dFlurryMeleeTelegraphAxis telegraph = queryActorMeleeTelegraph(target);
    if (telegraph == dFlurryTelegraph_None) {
        logDodgeAttempt("failed (no telegraph on lock target)");
        return false;
    }

    const bool axisMatch =
        (i_kind == dFlurryPerfectDodge_SideStep && telegraph == dFlurryTelegraph_Vertical) ||
        (i_kind == dFlurryPerfectDodge_BackJump && telegraph == dFlurryTelegraph_Horizontal);
    if (!axisMatch) {
        char buf[96];
        std::snprintf(buf, sizeof(buf), "failed (axis mismatch: dodge=%s telegraph=%s)",
                      dFlurryRush_getDodgeKindLabel(i_kind),
                      dFlurryRush_getTelegraphLabel(telegraph));
        logDodgeAttempt(buf);
        return false;
    }

    const dFlurryRushSwordProfile swordProfile = swordProfileFromEquip();
    if (swordProfile == dFlurryRushProfile_Unknown) {
        logDodgeAttempt("failed (unknown sword profile)");
        return false;
    }

    const dFlurryRushProfile profile = profileTable(swordProfile);
    if (!dFocusedArts_canPerfectDodgeSpend(profile.spendGate, profile.barCost)) {
        char buf[96];
        std::snprintf(buf, sizeof(buf),
                      "failed (FA gate %d cost %d bank %d/%d%s)", profile.spendGate, profile.barCost,
                      dFocusedArts_getBankCount(), dFocusedArts_getMaxBank(),
                      dFocusedArts_isInSpendSequence() ? " HS spend active" : "");
        logDodgeAttempt(buf);
        return false;
    }

    if (!dFlurryRush_beginMelee(target)) {
        logDodgeAttempt("failed (beginMelee rejected)");
        return false;
    }

    if (!dFocusedArts_onPerfectDodgeSpend(profile.spendGate, profile.barCost)) {
        dFlurryRush_end(dFlurryRushEnd_Interrupt);
        logDodgeAttempt("failed (FA spend consume rejected after begin)");
        return false;
    }

    s_state.pendingPerfectDodge = true;
    s_state.pendingDodgeKind = i_kind;

    char buf[96];
    std::snprintf(buf, sizeof(buf), "perfect dodge OK (%s vs %s telegraph)",
                  dFlurryRush_getDodgeKindLabel(i_kind),
                  dFlurryRush_getTelegraphLabel(telegraph));
    logDodgeAttempt(buf);
    logFlurryEvent(buf);
    return true;
}

#endif // TARGET_PC
