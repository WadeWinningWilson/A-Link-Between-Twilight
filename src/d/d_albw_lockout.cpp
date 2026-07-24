/**
 * d_albw_lockout.cpp — ALBW meter lockout item perks and session counters.
 */

#if TARGET_PC

#include "d/d_albw_lockout.h"
#include "d/d_albw_shield.h"
#include "d/d_albw_wolf_stun.h"
#include "d/d_com_inf_game.h"
#include "d/d_focused_arts.h"
#include "d/d_meter2_info.h"
#include "d/actor/d_a_alink.h"
#include "SSystem/SComponent/c_cc_d.h"
#include "SSystem/SComponent/c_math.h"
#include "f_op/f_op_actor_iter.h"
#include "f_op/f_op_actor_mng.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include <algorithm>

namespace {

constexpr u8 kLockoutBowShotsMax           = 3;
constexpr u8 kLockoutBombArrowShotsMax     = 2;
constexpr u8 kLockoutBomblingUsesMax       = 2;
constexpr int kLockoutSlingshotDebuffFrames = 120; // 4 seconds @ 30fps
constexpr int kLockoutDomRodConfuseFrames   = 300; // 10 seconds @ 30fps
constexpr f32 kLockoutConfuseRetargetRange2 = 2500.0f * 2500.0f;
constexpr int kLockoutProvokedMax           = 8;
constexpr int kLockoutProvokedFrames        = 300; // 10 seconds @ 30fps
constexpr f32 kLockoutBomblingOrbitRadius   = 200.0f; // ~10u at Link body scale
constexpr s16 kLockoutBomblingOrbitStep     = 0x433;  // ~5% faster than prior 0x400

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

struct ProvokedEnemy {
    fpc_ProcID mVictimId;
    fpc_ProcID mAttackerId;
    s16        mFrames;
};

u8 sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
u8 sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
u8 sLockoutBomblingUsesRemaining   = kLockoutBomblingUsesMax;
bool sLockoutDoubleClawUsed       = false;
bool sLockoutDoubleClawFlyActive  = false;
bool sLockoutDoubleClawSlashActive = false;
fpc_ProcID sLockoutDoubleClawTargetId = fpcM_ERROR_PROCESS_ID_e;
fpc_ProcID sLockoutBomblingId = fpcM_ERROR_PROCESS_ID_e;
s16 sLockoutBomblingOrbitAngle = 0;

constexpr u16 kLockoutDoubleClawSlashPower = 30;

void clearDoubleClawSession(bool i_thawTarget) {
    if (i_thawTarget && sLockoutDoubleClawTargetId != fpcM_ERROR_PROCESS_ID_e) {
        fopAc_ac_c* target = fopAcM_SearchByID(sLockoutDoubleClawTargetId);
        if (target != NULL) {
            dAlbwWolfStun_thaw(target);
        }
    }
    sLockoutDoubleClawFlyActive = false;
    sLockoutDoubleClawSlashActive = false;
    sLockoutDoubleClawTargetId = fpcM_ERROR_PROCESS_ID_e;
}

bool isLockoutDoubleClawEquip() {
    daAlink_c* link = daAlink_getAlinkActorClass();
    return link != NULL && link->mEquipItem == dItemNo_W_HOOKSHOT_e;
}

LockoutTaggedEnemy sTaggedEnemies[kLockoutTaggedMax];
int              sTaggedCount = 0;

DomRodConfuseState sConfuse = {fpcM_ERROR_PROCESS_ID_e, fpcM_ERROR_PROCESS_ID_e, 0};

ProvokedEnemy sProvokedEnemies[kLockoutProvokedMax];
int           sProvokedCount = 0;

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

void clearAllProvoked() {
    sProvokedCount = 0;
}

ProvokedEnemy* findProvokedEntry(fpc_ProcID i_victimId) {
    for (int i = 0; i < sProvokedCount; i++) {
        if (sProvokedEnemies[i].mVictimId == i_victimId) {
            return &sProvokedEnemies[i];
        }
    }
    return NULL;
}

void clearProvokedEntryAt(int i_index) {
    sProvokedCount--;
    for (int i = i_index; i < sProvokedCount; i++) {
        sProvokedEnemies[i] = sProvokedEnemies[i + 1];
    }
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

f32 distPointToSegmentXZ2(cXyz const& i_point, cXyz const& i_segA, cXyz const& i_segB) {
    const f32 abx = i_segB.x - i_segA.x;
    const f32 abz = i_segB.z - i_segA.z;
    const f32 apx = i_point.x - i_segA.x;
    const f32 apz = i_point.z - i_segA.z;
    const f32 abLen2 = abx * abx + abz * abz;
    if (abLen2 <= 0.0001f) {
        return apx * apx + apz * apz;
    }

    f32 t = (apx * abx + apz * abz) / abLen2;
    if (t < 0.0f) {
        t = 0.0f;
    } else if (t > 1.0f) {
        t = 1.0f;
    }

    const f32 cx = i_segA.x + abx * t - i_point.x;
    const f32 cz = i_segA.z + abz * t - i_point.z;
    return cx * cx + cz * cz;
}

struct NearestEnemySearch {
    fopAc_ac_c* mExclude;
    cXyz const* mPos;
    cXyz const* mSegA;
    cXyz const* mSegB;
    f32         mBestDist2;
    fopAc_ac_c* mBest;
    bool        mAllowlistOnly;
    bool        mUseSegment;
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

    const f32 dist2 =
        search->mUseSegment
            ? distPointToSegmentXZ2(actor->current.pos, *search->mSegA, *search->mSegB)
            : search->mPos->abs2(actor->current.pos);
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
    search.mSegA          = NULL;
    search.mSegB          = NULL;
    search.mBestDist2     = i_maxDist2;
    search.mBest          = NULL;
    search.mAllowlistOnly = i_allowlistOnly;
    search.mUseSegment    = false;
    fopAcIt_Judge(judgeNearestEnemy, &search);
    return search.mBest;
}

fopAc_ac_c* findNearestEnemyAlongSegment(cXyz const& i_segA, cXyz const& i_segB,
                                       fopAc_ac_c* i_exclude, f32 i_maxDist2,
                                       bool i_allowlistOnly) {
    NearestEnemySearch search;
    search.mExclude       = i_exclude;
    search.mPos           = NULL;
    search.mSegA          = &i_segA;
    search.mSegB          = &i_segB;
    search.mBestDist2     = i_maxDist2;
    search.mBest          = NULL;
    search.mAllowlistOnly = i_allowlistOnly;
    search.mUseSegment    = true;
    fopAcIt_Judge(judgeNearestEnemy, &search);
    return search.mBest;
}

void releaseSlingshotPauseKeepTag(fpc_ProcID i_id) {
    LockoutTaggedEnemy* entry = findTaggedEntry(i_id);
    if (entry == NULL || !entry->mPauseActive) {
        return;
    }

    base_process_class* proc = fpcM_SearchByID(i_id);
    if (proc != NULL) {
        fpcM_PauseDisable(proc, 1);
    }
    entry->mPauseActive = false;
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

    fopAc_ac_c* currentTarget = fopAcM_SearchByID(sConfuse.mTargetId);
    if (currentTarget != NULL && currentTarget != host &&
        fopAcM_GetGroup(currentTarget) == fopAc_ENEMY_e &&
        host->current.pos.abs2(currentTarget->current.pos) <= kLockoutConfuseRetargetRange2)
    {
        return;
    }

    fopAc_ac_c* target =
        findNearestEnemy(host->current.pos, host, kLockoutConfuseRetargetRange2, true);
    if (target == NULL) {
        target = findNearestEnemy(host->current.pos, host, kLockoutConfuseRetargetRange2, false);
    }
    sConfuse.mTargetId =
        target != NULL ? target->id : fpcM_ERROR_PROCESS_ID_e;
}

void tickProvokedEnemies() {
    int writeIdx = 0;
    for (int i = 0; i < sProvokedCount; i++) {
        ProvokedEnemy& entry = sProvokedEnemies[i];
        entry.mFrames--;

        fopAc_ac_c* victim = fopAcM_SearchByID(entry.mVictimId);
        fopAc_ac_c* attacker = fopAcM_SearchByID(entry.mAttackerId);
        if (entry.mFrames > 0 && victim != NULL && attacker != NULL &&
            fopAcM_GetGroup(victim) == fopAc_ENEMY_e &&
            fopAcM_GetGroup(attacker) == fopAc_ENEMY_e &&
            !(sConfuse.mFrames > 0 && entry.mVictimId == sConfuse.mHostId))
        {
            sProvokedEnemies[writeIdx++] = entry;
        }
    }
    sProvokedCount = writeIdx;
}

} // namespace

void dAlbwLockout_onBegin() {
    sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
    sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
    sLockoutBomblingUsesRemaining   = kLockoutBomblingUsesMax;
    sLockoutDoubleClawUsed = false;
    clearDoubleClawSession(true);
    sLockoutBomblingId         = fpcM_ERROR_PROCESS_ID_e;
    sLockoutBomblingOrbitAngle = 0;
    clearAllTaggedEnemies();
    clearConfuse();
    clearAllProvoked();
}

void dAlbwLockout_onEnd() {
    sLockoutBowShotsRemaining       = kLockoutBowShotsMax;
    sLockoutBombArrowShotsRemaining = kLockoutBombArrowShotsMax;
    sLockoutBomblingUsesRemaining   = kLockoutBomblingUsesMax;
    sLockoutDoubleClawUsed = false;
    clearDoubleClawSession(true);
    sLockoutBomblingId         = fpcM_ERROR_PROCESS_ID_e;
    sLockoutBomblingOrbitAngle = 0;
    clearAllTaggedEnemies();
    clearConfuse();
    clearAllProvoked();
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

    if (sProvokedCount > 0) {
        tickProvokedEnemies();
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

    // Lockout single-claw perk: pin meter at 0 (cancels partial recovery) and
    // enable tip contact damage via SetAtAtp while the shot flies (see setAtCollision).
    dMeter2_drainALBWToLockout();
}

void dAlbwLockout_onDoubleHookshotFired() {
    // Finisher consumes on latch, not on fire. No meter restore.
}

bool dAlbwLockout_canFireBow() {
    return dMeter2_isALBWLocked() && sLockoutBowShotsRemaining > 0;
}

bool dAlbwLockout_canFireBombArrow() {
    return dMeter2_isALBWLocked() && sLockoutBombArrowShotsRemaining > 0;
}

bool dAlbwLockout_canUseDoubleHookshot() {
    return dMeter2_isALBWLocked() && !sLockoutDoubleClawUsed;
}

bool dAlbwLockout_shouldForceEnemyStick(fopAc_ac_c* i_actor) {
    return dMeter2_isALBWLocked() && !sLockoutDoubleClawUsed && isLockoutDoubleClawEquip() &&
           i_actor != NULL && fopAcM_GetGroup(i_actor) == fopAc_ENEMY_e;
}

bool dAlbwLockout_shouldPierceHookShield(fopAc_ac_c* i_actor) {
    return dAlbwLockout_shouldForceEnemyStick(i_actor);
}

void dAlbwLockout_onDoubleClawLatch(fopAc_ac_c* i_enemy) {
    if (!dMeter2_isALBWLocked() || sLockoutDoubleClawUsed || i_enemy == NULL ||
        fopAcM_GetGroup(i_enemy) != fopAc_ENEMY_e)
    {
        return;
    }

    sLockoutDoubleClawUsed = true;
    sLockoutDoubleClawTargetId = i_enemy->id;
    sLockoutDoubleClawFlyActive = true;
    sLockoutDoubleClawSlashActive = false;
    dAlbwWolfStun_applyHold(i_enemy);
}

bool dAlbwLockout_isDoubleClawFlyActive() {
    return sLockoutDoubleClawFlyActive;
}

fopAc_ac_c* dAlbwLockout_getDoubleClawTarget() {
    if (sLockoutDoubleClawTargetId == fpcM_ERROR_PROCESS_ID_e) {
        return NULL;
    }
    return fopAcM_SearchByID(sLockoutDoubleClawTargetId);
}

void dAlbwLockout_onDoubleClawFlyEnded() {
    if (!sLockoutDoubleClawFlyActive) {
        return;
    }

    // Interrupted before slash — thaw and clear fly/slash session (use stays spent).
    if (sLockoutDoubleClawTargetId != fpcM_ERROR_PROCESS_ID_e) {
        fopAc_ac_c* target = fopAcM_SearchByID(sLockoutDoubleClawTargetId);
        if (target != NULL) {
            dAlbwWolfStun_thaw(target);
        }
    }
    sLockoutDoubleClawFlyActive = false;
    sLockoutDoubleClawSlashActive = false;
    sLockoutDoubleClawTargetId = fpcM_ERROR_PROCESS_ID_e;
}

void dAlbwLockout_onDoubleClawSlashBegin() {
    sLockoutDoubleClawFlyActive = false;
    sLockoutDoubleClawSlashActive = true;
}

bool dAlbwLockout_isDoubleClawSlashActive() {
    return sLockoutDoubleClawSlashActive;
}

void dAlbwLockout_onDoubleClawSlashEnd() {
    if (!sLockoutDoubleClawSlashActive && sLockoutDoubleClawTargetId == fpcM_ERROR_PROCESS_ID_e) {
        return;
    }

    if (sLockoutDoubleClawTargetId != fpcM_ERROR_PROCESS_ID_e) {
        fopAc_ac_c* target = fopAcM_SearchByID(sLockoutDoubleClawTargetId);
        if (target != NULL) {
            dAlbwWolfStun_thaw(target);
        }
    }
    sLockoutDoubleClawFlyActive = false;
    sLockoutDoubleClawSlashActive = false;
    sLockoutDoubleClawTargetId = fpcM_ERROR_PROCESS_ID_e;
}

u16 dAlbwLockout_getDoubleClawSlashAttackPower() {
    return sLockoutDoubleClawSlashActive ? kLockoutDoubleClawSlashPower : 0;
}

u8 dAlbwLockout_getHookshotContactAtp() {
    // Mirror Hero's Shade secret-boss At construction (npc_kn SetAtAtp): give the
    // claw tip real ATP so cc_at_check deals damage. atp 2 ≈ Ordon light hit after
    // at_power_get for common power types. Double claw keeps grab-only (atp 0).
    if (!dMeter2_isALBWLocked()) {
        return 0;
    }
    return 2;
}

bool dAlbwLockout_canUseBombling() {
    if (!dMeter2_isALBWLocked()) {
        return true;
    }
    // One orbiting bombling at a time; 2 uses per lockout session.
    if (sLockoutBomblingUsesRemaining == 0) {
        return false;
    }
    if (sLockoutBomblingId != fpcM_ERROR_PROCESS_ID_e &&
        fopAcM_SearchByID(sLockoutBomblingId) != NULL)
    {
        return false;
    }
    return true;
}

void dAlbwLockout_onBomblingDeployed(fopAc_ac_c* i_bombling) {
    if (!dMeter2_isALBWLocked() || i_bombling == NULL) {
        return;
    }
    if (sLockoutBomblingUsesRemaining == 0) {
        return;
    }
    sLockoutBomblingUsesRemaining--;
    sLockoutBomblingId = i_bombling->id;
    sLockoutBomblingOrbitAngle = 0;
    // Each lockout use restores 30% of base meter (same fraction as boom refill).
    dMeter2_addALBWBaseFraction(3, 10);
}

void dAlbwLockout_onBomblingDestroyed(fopAc_ac_c* i_bombling) {
    if (i_bombling == NULL || i_bombling->id != sLockoutBomblingId) {
        return;
    }
    sLockoutBomblingId = fpcM_ERROR_PROCESS_ID_e;
}

bool dAlbwLockout_isBomblingActive() {
    if (sLockoutBomblingId == fpcM_ERROR_PROCESS_ID_e || !dMeter2_isALBWLocked()) {
        return false;
    }
    return fopAcM_SearchByID(sLockoutBomblingId) != NULL;
}

bool dAlbwLockout_updateBomblingOrbit(fopAc_ac_c* i_bombling) {
    if (i_bombling == NULL || i_bombling->id != sLockoutBomblingId || !dMeter2_isALBWLocked()) {
        return false;
    }

    daAlink_c* player = daAlink_getAlinkActorClass();
    if (player == NULL) {
        return true;
    }

    sLockoutBomblingOrbitAngle += kLockoutBomblingOrbitStep;
    const f32 radius = kLockoutBomblingOrbitRadius;
    i_bombling->current.pos.x =
        player->current.pos.x + radius * cM_ssin(sLockoutBomblingOrbitAngle);
    i_bombling->current.pos.z =
        player->current.pos.z + radius * cM_scos(sLockoutBomblingOrbitAngle);
    i_bombling->current.pos.y = player->current.pos.y + 40.0f;
    i_bombling->old.pos = i_bombling->current.pos;
    i_bombling->speedF = 0.0f;
    i_bombling->speed = cXyz::Zero;
    i_bombling->gravity = 0.0f;
    i_bombling->shape_angle.y = sLockoutBomblingOrbitAngle + 0x4000;
    i_bombling->current.angle.y = i_bombling->shape_angle.y;
    return true;
}

void dAlbwLockout_onBlockWhileBomblingActive() {
    if (!dAlbwLockout_isBomblingActive()) {
        return;
    }
    dShield_addBashCharge(1);
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

    // Bow / bomb / spinner: +50%. Iron ball: +150%.
    if (i_atType == AT_TYPE_ARROW || i_atType == AT_TYPE_BOMB ||
        i_atType == AT_TYPE_SPINNER)
    {
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

    // Slingshot pause skips execute(); release it so confuse AI can run immediately.
    releaseSlingshotPauseKeepTag(i_enemy->id);

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

fopAc_ac_c* dAlbwLockout_searchDomRodConfuseVictim(cXyz const& i_segA, cXyz const& i_segB,
                                                   f32 i_radius) {
    if (!dMeter2_isALBWLocked()) {
        return NULL;
    }

    const f32 maxDist2 = i_radius * i_radius;
    return findNearestEnemyAlongSegment(i_segA, i_segB, NULL, maxDist2, true);
}

bool dAlbwLockout_isProvoked(fopAc_ac_c* i_victim) {
    return i_victim != NULL && dMeter2_isALBWLocked() &&
           findProvokedEntry(i_victim->id) != NULL;
}

fopAc_ac_c* dAlbwLockout_getProvokeSource(fopAc_ac_c* i_victim) {
    if (!dAlbwLockout_isProvoked(i_victim)) {
        return NULL;
    }

    const ProvokedEnemy* entry = findProvokedEntry(i_victim->id);
    fopAc_ac_c* attacker = fopAcM_SearchByID(entry->mAttackerId);
    if (attacker == NULL || fopAcM_GetGroup(attacker) != fopAc_ENEMY_e) {
        return NULL;
    }
    return attacker;
}

bool dAlbwLockout_hasRivalTarget(fopAc_ac_c* i_actor) {
    return dAlbwLockout_getRivalTarget(i_actor) != NULL;
}

fopAc_ac_c* dAlbwLockout_getRivalTarget(fopAc_ac_c* i_actor) {
    if (i_actor == NULL) {
        return NULL;
    }

    if (dAlbwLockout_isConfused(i_actor)) {
        return dAlbwLockout_getConfuseTarget(i_actor);
    }

    return dAlbwLockout_getProvokeSource(i_actor);
}

s16 dAlbwLockout_getRivalAimAngleY(fopAc_ac_c* i_actor) {
    fopAc_ac_c* target = dAlbwLockout_getRivalTarget(i_actor);
    if (target != NULL) {
        return fopAcM_searchActorAngleY(i_actor, target);
    }
    return fopAcM_searchPlayerAngleY(i_actor);
}

s16 dAlbwLockout_getRivalAimAngleX(fopAc_ac_c* i_actor) {
    fopAc_ac_c* target = dAlbwLockout_getRivalTarget(i_actor);
    if (target != NULL) {
        return fopAcM_searchActorAngleX(i_actor, target);
    }
    return fopAcM_searchPlayerAngleX(i_actor);
}

f32 dAlbwLockout_getRivalAimDistanceXZ(fopAc_ac_c* i_actor) {
    fopAc_ac_c* target = dAlbwLockout_getRivalTarget(i_actor);
    if (target != NULL) {
        return fopAcM_searchActorDistanceXZ(i_actor, target);
    }
    return fopAcM_searchPlayerDistanceXZ(i_actor);
}

f32 dAlbwLockout_getRivalAimDistance(fopAc_ac_c* i_actor) {
    fopAc_ac_c* target = dAlbwLockout_getRivalTarget(i_actor);
    if (target != NULL) {
        return fopAcM_searchActorDistance(i_actor, target);
    }
    return fopAcM_searchPlayerDistance(i_actor);
}

void dAlbwLockout_onConfuseFriendlyFireHit(fopAc_ac_c* i_victim, fopAc_ac_c* i_attacker) {
    if (!dMeter2_isALBWLocked() || i_victim == NULL || i_attacker == NULL ||
        i_victim == i_attacker || fopAcM_GetGroup(i_victim) != fopAc_ENEMY_e ||
        fopAcM_GetGroup(i_attacker) != fopAc_ENEMY_e ||
        !dAlbwLockout_isConfused(i_attacker) ||
        !isConfuseAllowlistName(fopAcM_GetName(i_victim)) ||
        dAlbwLockout_isConfused(i_victim)) {
        return;
    }

    releaseSlingshotPauseKeepTag(i_victim->id);

    ProvokedEnemy* existing = findProvokedEntry(i_victim->id);
    if (existing != NULL) {
        existing->mAttackerId = i_attacker->id;
        existing->mFrames     = static_cast<s16>(kLockoutProvokedFrames);
        return;
    }

    if (sProvokedCount >= kLockoutProvokedMax) {
        return;
    }

    ProvokedEnemy& entry = sProvokedEnemies[sProvokedCount++];
    entry.mVictimId   = i_victim->id;
    entry.mAttackerId = i_attacker->id;
    entry.mFrames     = static_cast<s16>(kLockoutProvokedFrames);
}

#endif // TARGET_PC
