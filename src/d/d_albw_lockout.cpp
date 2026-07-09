/**
 * d_albw_lockout.cpp — ALBW meter lockout item perks and session counters.
 */

#if TARGET_PC

#include "d/d_albw_lockout.h"
#include "d/d_albw_shield.h"
#include "d/d_com_inf_game.h"
#include "d/d_focused_arts.h"
#include "d/d_meter2_info.h"
#include "SSystem/SComponent/c_cc_d.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include <algorithm>

namespace {

constexpr u8 kLockoutBowShotsMax           = 3;
constexpr u8 kLockoutBombArrowShotsMax     = 2;
constexpr u8 kLockoutHookshotHitsHeal      = 10;
constexpr u16 kLockoutHeartHealUnits       = 12; // 3 hearts (quarter-heart units)
constexpr int kLockoutSlingshotDebuffFrames = 120; // 4 seconds @ 30fps
constexpr int kLockoutDomRodConfuseFrames   = 300; // 10 seconds @ 30fps
constexpr f32 kLockoutConfuseRetargetRange2 = 2500.0f * 2500.0f;

constexpr int kLockoutTaggedMax = 16;

struct LockoutTaggedEnemy {
    fpc_ProcID mId;
    s16        mFrames;
    bool       mPauseActive;
};

struct DomRodConfuseState {
    fpc_ProcID mHostId;
    fpc_ProcID mTargetId;
    s16        mFrames;
};

u8 sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
u8 sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
u8 sLockoutHookshotHits       = 0;
bool sLockoutDoubleClawUsed   = false;

LockoutTaggedEnemy sTaggedEnemies[kLockoutTaggedMax];
int              sTaggedCount = 0;

DomRodConfuseState sConfuse = {fpcM_ERROR_PROCESS_ID_e, fpcM_ERROR_PROCESS_ID_e, 0};

int lockoutMeterThresholdPct(int i_percent) {
    const int maxVal = dMeter2_getALBWMaxValue();
    if (maxVal <= 0) {
        return 0;
    }
    return (maxVal * i_percent) / 100;
}

LockoutTaggedEnemy* findTaggedEntry(fpc_ProcID i_id) {
    for (int i = 0; i < sTaggedCount; i++) {
        if (sTaggedEnemies[i].mId == i_id) {
            return &sTaggedEnemies[i];
        }
    }
    return NULL;
}

void clearTaggedEntryAt(int i_index) {
    LockoutTaggedEnemy& entry = sTaggedEnemies[i_index];
    if (entry.mPauseActive) {
        base_process_class* proc = fpcM_SearchByID(entry.mId);
        if (proc != NULL) {
            fpcM_PauseDisable(proc, 1);
        }
    }

    sTaggedCount--;
    for (int i = i_index; i < sTaggedCount; i++) {
        sTaggedEnemies[i] = sTaggedEnemies[i + 1];
    }
}

void clearAllTaggedEnemies() {
    while (sTaggedCount > 0) {
        clearTaggedEntryAt(0);
    }
}

void clearConfuse() {
    sConfuse.mHostId   = fpcM_ERROR_PROCESS_ID_e;
    sConfuse.mTargetId = fpcM_ERROR_PROCESS_ID_e;
    sConfuse.mFrames   = 0;
}

bool isConfuseAllowlistName(s16 i_name) {
    return i_name == fpcNm_E_OC_e || i_name == fpcNm_E_DN_e || i_name == fpcNm_E_ST_e;
}

void tagEnemyForLockoutSlingshot(fopAc_ac_c* i_enemy, bool i_nativeStunOnly) {
    if (!dMeter2_isALBWLocked() || i_enemy == NULL ||
        fopAcM_GetGroup(i_enemy) != fopAc_ENEMY_e) {
        return;
    }

    const fpc_ProcID id = i_enemy->id;
    LockoutTaggedEnemy* existing = findTaggedEntry(id);
    if (existing != NULL) {
        existing->mFrames = static_cast<s16>(kLockoutSlingshotDebuffFrames);
        return;
    }

    if (sTaggedCount >= kLockoutTaggedMax) {
        return;
    }

    LockoutTaggedEnemy& entry = sTaggedEnemies[sTaggedCount++];
    entry.mId           = id;
    entry.mFrames       = static_cast<s16>(kLockoutSlingshotDebuffFrames);
    entry.mPauseActive  = false;

    if (!i_nativeStunOnly) {
        fpcM_PauseEnable(i_enemy, 1);
        entry.mPauseActive = true;
    }
}

bool taggedEnemyHasFrames(fopAc_ac_c* i_enemy) {
    if (i_enemy == NULL || !dMeter2_isALBWLocked()) {
        return false;
    }

    const LockoutTaggedEnemy* entry = findTaggedEntry(i_enemy->id);
    return entry != NULL && entry->mFrames > 0;
}

struct NearestEnemySearch {
    fopAc_ac_c* mExclude;
    cXyz const* mPos;
    f32         mBestDist2;
    fopAc_ac_c* mBest;
    bool        mAllowlistOnly;
};

void* judgeNearestEnemy(void* i_actor, void* i_data) {
    fopAc_ac_c* actor = static_cast<fopAc_ac_c*>(i_actor);
    NearestEnemySearch* search = static_cast<NearestEnemySearch*>(i_data);
    if (actor == NULL || actor == search->mExclude ||
        fopAcM_GetGroup(actor) != fopAc_ENEMY_e) {
        return NULL;
    }
    if (search->mAllowlistOnly && !isConfuseAllowlistName(fopAcM_GetName(actor))) {
        return NULL;
    }

    const f32 dist2 = search->mPos->abs2(actor->current.pos);
    if (dist2 < search->mBestDist2) {
        search->mBestDist2 = dist2;
        search->mBest      = actor;
    }
    return NULL;
}

fopAc_ac_c* findNearestEnemy(cXyz const& i_pos, fopAc_ac_c* i_exclude, f32 i_maxDist2,
                             bool i_allowlistOnly) {
    NearestEnemySearch search;
    search.mExclude       = i_exclude;
    search.mPos           = &i_pos;
    search.mBestDist2     = i_maxDist2;
    search.mBest          = NULL;
    search.mAllowlistOnly = i_allowlistOnly;
    fopAcIt_Judge(judgeNearestEnemy, &search);
    return search.mBest;
}

void refreshConfuseTarget() {
    if (sConfuse.mFrames <= 0) {
        return;
    }

    fopAc_ac_c* host = fopAcM_SearchByID(sConfuse.mHostId);
    if (host == NULL || fopAcM_GetGroup(host) != fopAc_ENEMY_e) {
        clearConfuse();
        return;
    }

    fopAc_ac_c* target =
        findNearestEnemy(host->current.pos, host, kLockoutConfuseRetargetRange2, false);
    sConfuse.mTargetId =
        target != NULL ? target->id : fpcM_ERROR_PROCESS_ID_e;
}

} // namespace

void dAlbwLockout_onBegin() {
    sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
    sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
    sLockoutHookshotHits        = 0;
    sLockoutDoubleClawUsed      = false;
    clearAllTaggedEnemies();
    clearConfuse();
}

void dAlbwLockout_onEnd() {
    sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
    sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
    sLockoutHookshotHits        = 0;
    sLockoutDoubleClawUsed      = false;
    clearAllTaggedEnemies();
    clearConfuse();
}

void dAlbwLockout_update() {
    if (sTaggedCount > 0) {
        int writeIdx = 0;
        for (int i = 0; i < sTaggedCount; i++) {
            sTaggedEnemies[i].mFrames--;
            if (sTaggedEnemies[i].mFrames > 0) {
                sTaggedEnemies[writeIdx++] = sTaggedEnemies[i];
            } else if (sTaggedEnemies[i].mPauseActive) {
                base_process_class* proc = fpcM_SearchByID(sTaggedEnemies[i].mId);
                if (proc != NULL) {
                    fpcM_PauseDisable(proc, 1);
                }
            }
        }
        sTaggedCount = writeIdx;
    }

    if (sConfuse.mFrames > 0) {
        sConfuse.mFrames--;
        if (sConfuse.mFrames <= 0 || !dMeter2_isALBWLocked()) {
            clearConfuse();
        } else {
            refreshConfuseTarget();
        }
    }
}

void dAlbwLockout_onArrowFired() {
    if (!dMeter2_isALBWLocked() || sLockoutBowShotsRemaining == 0) {
        return;
    }

    sLockoutBowShotsRemaining--;
}

void dAlbwLockout_onBombArrowFired() {
    if (!dMeter2_isALBWLocked() || sLockoutBombArrowShotsRemaining == 0) {
        return;
    }

    sLockoutBombArrowShotsRemaining--;
}

void dAlbwLockout_onHookshotFired() {
    if (!dMeter2_isALBWLocked()) {
        return;
    }

    sLockoutHookshotHits++;
    if (sLockoutHookshotHits < kLockoutHookshotHitsHeal) {
        return;
    }

    sLockoutHookshotHits = 0;
    const u16 life = dComIfGs_getLife();
    const u16 maxLife = dComIfGs_getMaxLife();
    dComIfGs_setLife((u16)std::min<u32>(maxLife, life + kLockoutHeartHealUnits));
}

void dAlbwLockout_onDoubleHookshotFired() {
    if (!dMeter2_isALBWLocked() || sLockoutDoubleClawUsed) {
        return;
    }

    sLockoutDoubleClawUsed = true;
    dMeter2_restoreALBWMeterToFull();
}

bool dAlbwLockout_canFireBow() {
    return dMeter2_isALBWLocked() && sLockoutBowShotsRemaining > 0;
}

bool dAlbwLockout_canFireBombArrow() {
    return dMeter2_isALBWLocked() && sLockoutBombArrowShotsRemaining > 0;
}

bool dAlbwLockout_canUseDoubleHookshot() {
    return dMeter2_isALBWLocked() && !sLockoutDoubleClawUsed &&
           dMeter2_getALBWMeterValue() >= lockoutMeterThresholdPct(50);
}

void dAlbwLockout_onSlingshotHit(fopAc_ac_c* i_enemy) {
    tagEnemyForLockoutSlingshot(i_enemy, false);
}

void dAlbwLockout_onSlingshotHitNative(fopAc_ac_c* i_enemy) {
    tagEnemyForLockoutSlingshot(i_enemy, true);
}

bool dAlbwLockout_isRangedOpened(fopAc_ac_c* i_enemy) {
    return taggedEnemyHasFrames(i_enemy);
}

bool dAlbwLockout_isSlingshotStunActive(fopAc_ac_c* i_enemy) {
    return taggedEnemyHasFrames(i_enemy);
}

int dAlbwLockout_getSlingshotStunFrames(int i_baseFrames) {
    if (!dMeter2_isALBWLocked() || i_baseFrames <= 0) {
        return i_baseFrames;
    }

    if (i_baseFrames >= kLockoutSlingshotDebuffFrames) {
        return i_baseFrames;
    }

    return kLockoutSlingshotDebuffFrames;
}

void dAlbwLockout_applyAttackPowerBoost(u16& io_attackPower, u32 i_atType) {
    if (io_attackPower == 0 || !dMeter2_isALBWLocked()) {
        return;
    }

    if (dFocusedArts_isJsFinisherLockoutActive()) {
        io_attackPower = static_cast<u16>(
            std::min<u32>(static_cast<u32>(io_attackPower) * 4u, 0xFFFFu));
        return;
    }

    if (i_atType == AT_TYPE_ARROW || i_atType == AT_TYPE_BOMB) {
        io_attackPower = (io_attackPower * 3) / 2;
    } else if (i_atType == AT_TYPE_IRON_BALL) {
        io_attackPower = (io_attackPower * 5) / 2;
    }
}

bool dAlbwLockout_isDomRodConfuseAllowlist(fopAc_ac_c* i_enemy) {
    return i_enemy != NULL && fopAcM_GetGroup(i_enemy) == fopAc_ENEMY_e &&
           isConfuseAllowlistName(fopAcM_GetName(i_enemy));
}

void dAlbwLockout_onDomRodConfuseHit(fopAc_ac_c* i_enemy) {
    if (!dMeter2_isALBWLocked() || !dAlbwLockout_isDomRodConfuseAllowlist(i_enemy)) {
        return;
    }

    sConfuse.mHostId = i_enemy->id;
    sConfuse.mFrames = static_cast<s16>(kLockoutDomRodConfuseFrames);
    refreshConfuseTarget();
}

bool dAlbwLockout_isConfused(fopAc_ac_c* i_enemy) {
    return i_enemy != NULL && sConfuse.mFrames > 0 && dMeter2_isALBWLocked() &&
           i_enemy->id == sConfuse.mHostId;
}

fopAc_ac_c* dAlbwLockout_getConfuseTarget(fopAc_ac_c* i_attacker) {
    if (!dAlbwLockout_isConfused(i_attacker)) {
        return NULL;
    }

    fopAc_ac_c* target = fopAcM_SearchByID(sConfuse.mTargetId);
    if (target == NULL || fopAcM_GetGroup(target) != fopAc_ENEMY_e) {
        return NULL;
    }
    return target;
}

s16 dAlbwLockout_getConfuseAimAngleY(fopAc_ac_c* i_attacker) {
    fopAc_ac_c* target = dAlbwLockout_getConfuseTarget(i_attacker);
    if (target != NULL) {
        return fopAcM_searchActorAngleY(i_attacker, target);
    }
    return fopAcM_searchPlayerAngleY(i_attacker);
}

s16 dAlbwLockout_getConfuseAimAngleX(fopAc_ac_c* i_attacker) {
    fopAc_ac_c* target = dAlbwLockout_getConfuseTarget(i_attacker);
    if (target != NULL) {
        return fopAcM_searchActorAngleX(i_attacker, target);
    }
    return fopAcM_searchPlayerAngleX(i_attacker);
}

f32 dAlbwLockout_getConfuseAimDistanceXZ(fopAc_ac_c* i_attacker) {
    fopAc_ac_c* target = dAlbwLockout_getConfuseTarget(i_attacker);
    if (target != NULL) {
        return fopAcM_searchActorDistanceXZ(i_attacker, target);
    }
    return fopAcM_searchPlayerDistanceXZ(i_attacker);
}

f32 dAlbwLockout_getConfuseAimDistance(fopAc_ac_c* i_attacker) {
    fopAc_ac_c* target = dAlbwLockout_getConfuseTarget(i_attacker);
    if (target != NULL) {
        return fopAcM_searchActorDistance(i_attacker, target);
    }
    return fopAcM_searchPlayerDistance(i_attacker);
}

void dAlbwLockout_syncConfuseAtBits(fopAc_ac_c* i_attacker, cCcD_Obj* i_atObj) {
    if (i_atObj == NULL) {
        return;
    }

    if (dAlbwLockout_isConfused(i_attacker)) {
        i_atObj->OffAtVsPlayerBit();
        i_atObj->OnAtVsEnemyBit();
    } else {
        i_atObj->OnAtVsPlayerBit();
        i_atObj->OffAtVsEnemyBit();
    }
}

fopAc_ac_c* dAlbwLockout_searchDomRodConfuseVictim(cXyz const& i_ballPos, f32 i_radius) {
    if (!dMeter2_isALBWLocked()) {
        return NULL;
    }

    return findNearestEnemy(i_ballPos, NULL, i_radius * i_radius, true);
}

#endif // TARGET_PC
