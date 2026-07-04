/**
 * d_albw_wolf_stun.cpp — ALBW Port
 * Wolf combat system — generic pause-based stun module.
 *
 * On a Midna field attack hit against a non-twilight, non-boss enemy:
 *   dAlbwWolfStun_apply() is called from cc_at_check.
 *   fpcM_PauseEnable(enemy, 1) skips the enemy's execute() each frame
 *   while fpcEx_Execute still returns early, but draw() is unaffected —
 *   the enemy freezes mid-animation in its last pose for the stun duration.
 *
 * dAlbwWolfStun_update() is called every frame from daAlink_c::execute()
 * to decrement timers and call fpcM_PauseDisable when each expires.
 *
 * Paused enemies no longer register hurt colliders in execute(). The
 * draw-phase bridge (beforeMove / afterMove) re-registers snapshotted TG
 * shapes so follow-up Link melee (wolf bites or human sword) resolves
 * through cc_at_check().
 */

#if TARGET_PC

#include "d/d_albw_wolf_stun.h"
#include "SSystem/SComponent/c_cc_d.h"
#include "SSystem/SComponent/c_counter.h"
#include "SSystem/SComponent/c_sxyz.h"
#include "d/d_cc_d.h"
#include "d/d_cc_uty.h"
#include "d/d_com_inf_game.h"
#include "d/dolzel.h"
#include "d/actor/d_a_e_oc.h"
#include "d/actor/d_a_e_sh.h"
#include "d/actor/d_a_alink.h"
#include "d/actor/d_a_player.h"
#include "dusk/settings.h"
#include "Z2AudioLib/Z2Creature.h"
#include "f_pc/f_pc_manager.h"
#include "f_pc/f_pc_name.h"
#include "f_op/f_op_actor.h"
#include "f_op/f_op_actor_mng.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

namespace {

// RelWithDebInfo strips OS_REPORT (DEBUG=0). Write playtest traces to a file instead.
void wolfStun_debugLog(const char* fmt, ...) {
    if (!dusk::getSettings().game.wolfLinkCombat.getValue()) {
        return;
    }

    static bool sResetDone = false;

    char path[512];
    path[0] = '\0';
    const char* user = getenv("USERPROFILE");
    if (user != NULL && user[0] != '\0') {
        snprintf(path, sizeof(path), "%s/Documents/dusklight/albw_wolf_stun_debug.txt", user);
    } else {
        strncpy(path, "albw_wolf_stun_debug.txt", sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    FILE* fp = fopen(path, sResetDone ? "a" : "w");
    if (fp == NULL) {
        fp = fopen("albw_wolf_stun_debug.txt", sResetDone ? "a" : "w");
    }
    if (fp == NULL) {
        return;
    }

    if (!sResetDone) {
        sResetDone = true;
        fprintf(fp, "--- ALBW Wolf Link stun / collision bridge debug ---\n");
    }

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    fclose(fp);
}

struct WolfStunEntry {
    fpc_ProcID  mId;
    s16         mTimer;
    s8          mColliderCount;
    cCcD_Obj*   mColliders[8];
    // Execute-phase cc_at_check already resolved the opening field-attack hit;
    // skip draw-phase bridge on that frame to avoid a same-frame double deduct.
    int         mSkipBridgeFrame;
};

constexpr int WOLF_STUN_MAX = 16;

static WolfStunEntry sStunList[WOLF_STUN_MAX];
static int           sStunCount = 0;

void wolfStun_debugLogHeartbeat(cCcS* i_ccs) {
    static int sLastHeartbeatFrame = -1000;

    const int frame = g_Counter.mCounter0;
    if (frame - sLastHeartbeatFrame < 60) {
        return;
    }
    sLastHeartbeatFrame = frame;

    for (int i = 0; i < sStunCount; i++) {
        fopAc_ac_c* enemy =
            static_cast<fopAc_ac_c*>(fpcM_SearchByID(sStunList[i].mId));
        const s16 name = enemy != NULL ? fopAcM_GetName(enemy) : -1;
        const s16 hp = enemy != NULL ? enemy->health : -1;
        int tgSetCount = 0;
        for (int c = 0; c < sStunList[i].mColliderCount; c++) {
            cCcD_Obj* obj = sStunList[i].mColliders[c];
            if (obj != NULL && obj->ChkTgSet()) {
                tgSetCount++;
            }
        }

        wolfStun_debugLog(
            "f=%06d evt=heartbeat id=%u name=%d coll=%d tgSet=%d hp=%d ccAt=%d ccTg=%d ccObj=%d\n",
            frame, sStunList[i].mId, name, sStunList[i].mColliderCount, tgSetCount, hp,
            i_ccs != NULL ? i_ccs->mObjAtCount : -1, i_ccs != NULL ? i_ccs->mObjTgCount : -1,
            i_ccs != NULL ? i_ccs->mObjCount : -1);
    }
}

void wolfStun_debugLogNoColliders(fpc_ProcID i_id) {
    static fpc_ProcID sLastId = fpcM_ERROR_PROCESS_ID_e;
    static int sLastFrame = -1000;

    const int frame = g_Counter.mCounter0;
    if (i_id == sLastId && frame - sLastFrame < 30) {
        return;
    }
    sLastId = i_id;
    sLastFrame = frame;

    wolfStun_debugLog("f=%06d evt=no-colliders id=%u mpObjCount=%d\n", frame, i_id,
                      dComIfG_Ccsp()->mObjCount);
}

WolfStunEntry* findEntry(fpc_ProcID i_id) {
    for (int i = 0; i < sStunCount; i++) {
        if (sStunList[i].mId == i_id) {
            return &sStunList[i];
        }
    }
    return NULL;
}

bool sttsBelongsToActor(cCcD_Stts* i_stts, fpc_ProcID i_id, fopAc_ac_c* i_enemy) {
    if (i_stts == NULL) {
        return false;
    }

    if (i_stts->GetApid() == i_id) {
        return true;
    }

    fopAc_ac_c* ac = i_stts->GetAc();
    return ac != NULL && ac == i_enemy;
}

bool colliderRegistered(cCcS* i_ccs, cCcD_Obj* i_obj) {
    for (int i = 0; i < i_ccs->mObjTgCount; i++) {
        if (i_ccs->mpObjTg[i] == i_obj) {
            return true;
        }
    }
    return false;
}

void storeColliders(WolfStunEntry* i_entry, cCcD_Obj* const* i_objs, int i_count) {
    i_entry->mColliderCount = 0;

    const int maxStore = static_cast<int>(ARRAY_SIZEU(i_entry->mColliders));
    for (int i = 0; i < i_count && i_entry->mColliderCount < maxStore; i++) {
        cCcD_Obj* obj = i_objs[i];
        if (obj != NULL) {
            i_entry->mColliders[i_entry->mColliderCount++] = obj;
        }
    }
}

void captureCollidersFromCcS(cCcS* i_ccs, fopAc_ac_c* i_enemy, WolfStunEntry* i_entry) {
    const fpc_ProcID id = fopAcM_GetID(i_enemy);
    cCcD_Obj* found[ARRAY_SIZEU(i_entry->mColliders)] = {};
    int foundCount = 0;

    for (int i = 0; i < i_ccs->mObjCount; i++) {
        cCcD_Obj* obj = i_ccs->mpObj[i];
        if (obj == NULL || !obj->ChkTgSet()) {
            continue;
        }

        if (!sttsBelongsToActor(obj->GetStts(), id, i_enemy)) {
            continue;
        }

        if (foundCount >= static_cast<int>(ARRAY_SIZEU(found))) {
            break;
        }

        found[foundCount++] = obj;
    }

    if (foundCount > 0) {
        storeColliders(i_entry, found, foundCount);
        wolfStun_debugLog("f=%06d evt=capture-mpObj id=%u count=%d mpObj=%d\n",
                          g_Counter.mCounter0, id, foundCount, i_ccs->mObjCount);
    }
}

void removeStunEntry(int i_index);

void restoreStunColliderFlags(WolfStunEntry* i_entry) {
    for (int c = 0; c < i_entry->mColliderCount; c++) {
        cCcD_Obj* obj = i_entry->mColliders[c];
        if (obj == NULL) {
            continue;
        }

        dCcD_GObjInf* gobj = dCcD_GetGObjInf(obj);
        if (gobj != NULL) {
            gobj->OnTgNoConHit();
        }
    }
}

void prepareStunnedEnemyForBridge(fopAc_ac_c* i_enemy, WolfStunEntry* i_entry) {
    if (i_entry->mColliderCount <= 0) {
        return;
    }

    const s16 name = fopAcM_GetName(i_enemy);
    if (name == fpcNm_E_OC_e) {
        static_cast<daE_OC_c*>(i_enemy)->refreshStunHurtColliders();
    } else if (name == fpcNm_E_SH_e) {
        e_sh_refreshStunHurtColliders(reinterpret_cast<e_sh_class*>(i_enemy));
    }

    cCcD_Stts* stts = i_entry->mColliders[0]->GetStts();
    if (stts != NULL) {
        static_cast<dCcD_Stts*>(stts)->Move();
    }

    static fpc_ProcID sLastPrepLogId = fpcM_ERROR_PROCESS_ID_e;
    static int sLastPrepLogFrame = -1000;
    const int frame = g_Counter.mCounter0;
    const bool logPrep = i_entry->mId != sLastPrepLogId || frame - sLastPrepLogFrame >= 60;

    for (int c = 0; c < i_entry->mColliderCount; c++) {
        cCcD_Obj* obj = i_entry->mColliders[c];
        if (obj == NULL) {
            continue;
        }

        dCcD_GObjInf* gobj = dCcD_GetGObjInf(obj);
        if (gobj == NULL) {
            continue;
        }

        if (logPrep && c == 0) {
            wolfStun_debugLog(
                "f=%06d evt=bridge-prep id=%u name=%d hadNoConHit=%d\n", frame, i_entry->mId, name,
                gobj->ChkTgNoConHit() ? 1 : 0);
            sLastPrepLogId = i_entry->mId;
            sLastPrepLogFrame = frame;
        }

        gobj->OffTgNoConHit();
    }
}

void removeStunEntry(int i_index) {
    restoreStunColliderFlags(&sStunList[i_index]);
    base_process_class* proc = fpcM_SearchByID(sStunList[i_index].mId);
    if (proc != NULL) {
        fpcM_PauseDisable(proc, 1);
    }

    for (int j = i_index; j < sStunCount - 1; j++) {
        sStunList[j] = sStunList[j + 1];
    }
    sStunCount--;
}

constexpr u32 BRIDGE_LINK_MELEE_AT_TYPES =
    AT_TYPE_WOLF_ATTACK | AT_TYPE_WOLF_CUT_TURN | AT_TYPE_NORMAL_SWORD |
    AT_TYPE_MASTER_SWORD | AT_TYPE_MIDNA_LOCK;

constexpr u32 BRIDGE_WOLF_BITE_AT_TYPES = AT_TYPE_WOLF_ATTACK | AT_TYPE_WOLF_CUT_TURN;

bool isBridgeLinkMeleeAt(cCcD_Obj* i_atObj) {
    if (i_atObj == NULL) {
        return false;
    }

    if (i_atObj->ChkAtType(BRIDGE_LINK_MELEE_AT_TYPES) == 0) {
        return false;
    }

    fopAc_ac_c* link = daPy_getPlayerActorClass();
    fopAc_ac_c* atAc = i_atObj->GetAc();
    if (atAc == NULL) {
        return link != NULL;
    }

    return atAc == link || fopAcM_GetName(atAc) == fpcNm_ALINK_e;
}

Z2Creature* getBridgeEnemySound(fopAc_ac_c* i_enemy) {
    if (i_enemy == NULL) {
        return NULL;
    }

    const s16 name = fopAcM_GetName(i_enemy);
    switch (name) {
    case fpcNm_E_OC_e:
        return static_cast<daE_OC_c*>(i_enemy)->getStunBridgeSound();
    case fpcNm_E_SH_e:
        return reinterpret_cast<e_sh_class*>(i_enemy)->getStunBridgeSound();
    default:
        return NULL;
    }
}

void prepareBridgeAtInfo(fopAc_ac_c* i_enemy, dCcU_AtInfo* i_atInfo) {
    i_atInfo->mpSound = getBridgeEnemySound(i_enemy);
    i_atInfo->mPowerType = 1;
}

// Paused enemies fail ProcAtTgHitmark's ChkNoneActorPerfTblId() gate during Move(),
// so spawn slash/spark particles here after cc_at_check resolves damage.
void spawnBridgeHitMark(fopAc_ac_c* i_enemy, cCcD_Obj* i_atObj, cCcD_Obj* i_tgObj,
                        dCcU_AtInfo* i_atInfo) {
    if (i_atInfo->mAttackPower == 0 || i_atObj == NULL || i_tgObj == NULL) {
        return;
    }

    if (i_atObj->ChkAtType(BRIDGE_WOLF_BITE_AT_TYPES)) {
        return;
    }

    dCcD_GObjInf* atGObj = dCcD_GetGObjInf(i_atObj);
    dCcD_GObjInf* tgGObj = dCcD_GetGObjInf(i_tgObj);
    if (atGObj == NULL || tgGObj == NULL) {
        return;
    }

    if (atGObj->ChkAtNoHitMark() || tgGObj->ChkTgNoHitMark()) {
        return;
    }

    if ((atGObj->GetAtType() &
         (AT_TYPE_WOLF_ATTACK | AT_TYPE_WOLF_CUT_TURN | AT_TYPE_10000000 | AT_TYPE_MIDNA_LOCK |
          AT_TYPE_HOOKSHOT | AT_TYPE_SHIELD_ATTACK | AT_TYPE_NORMAL_SWORD)) != 0 &&
        tgGObj->GetTgSpl() == dCcG_Tg_Spl_UNK_1)
    {
        return;
    }

    cXyz* hitPosP = tgGObj->GetTgHitPosP();
    if (hitPosP == NULL) {
        return;
    }

    csXyz angle;
    angle.x = 0;
    angle.y = i_atInfo->mHitDirection.y;
    angle.z = 0;

    if (tgGObj->GetTgHitMark() == 5 || tgGObj->GetTgHitMark() == 8) {
        dComIfGp_setHitMark(2, i_enemy, hitPosP, &angle, NULL, atGObj->GetAtType());
        wolfStun_debugLog("f=%06d evt=bridge-vfx id=%u kind=hitmark2\n", g_Counter.mCounter0,
                          i_enemy->id);
        return;
    }

    u16 hitmark = tgGObj->GetTgHitMark() == 3 ? 3 : atGObj->GetAtHitMark();
    if ((hitmark == 0 && tgGObj->GetTgHitMark() != 8) ||
        (hitmark == 4 && tgGObj->GetTgHitMark() == 4))
    {
        return;
    }

    if ((hitmark != 1 && hitmark != 3) || atGObj->GetAtAtp() != 0) {
        dComIfGp_setHitMark(hitmark, i_enemy, hitPosP, &angle, NULL, atGObj->GetAtType());
        wolfStun_debugLog("f=%06d evt=bridge-vfx id=%u kind=hitmark%u\n", g_Counter.mCounter0,
                          i_enemy->id, hitmark);
    }
}

void dispatchBridgeHitVfx(fopAc_ac_c* i_enemy, cCcD_Obj* i_atObj, cCcD_Obj* i_tgObj,
                          dCcU_AtInfo* i_atInfo) {
    if (i_atInfo->mAttackPower == 0) {
        return;
    }

    if (fopAcM_GetGroup(i_enemy) == fopAc_ENEMY_e &&
        i_atObj->ChkAtType(BRIDGE_WOLF_BITE_AT_TYPES))
    {
        daAlink_c* link = static_cast<daAlink_c*>(daPy_getPlayerActorClass());
        if (link != NULL) {
            link->setWolfBiteDamage(static_cast<fopEn_enemy_c*>(i_enemy));
            wolfStun_debugLog("f=%06d evt=bridge-vfx id=%u kind=wolf-bite\n", g_Counter.mCounter0,
                              i_enemy->id);
        }
        return;
    }

    spawnBridgeHitMark(i_enemy, i_atObj, i_tgObj, i_atInfo);
}

void dispatchLethalDeathReaction(fopAc_ac_c* i_enemy) {
    if (i_enemy == NULL || i_enemy->health > 0) {
        return;
    }

    const s16 name = fopAcM_GetName(i_enemy);

    if (name == fpcNm_E_OC_e) {
        daE_OC_c* oc = static_cast<daE_OC_c*>(i_enemy);
        oc->setActionMode(6, 0);
        oc->offTgSph();
        return;
    }

    if (fopAcM_GetGroup(i_enemy) == fopAc_ENEMY_e) {
        static_cast<fopEn_enemy_c*>(i_enemy)->onWolfBiteDamage();
    }
}

void processBridgeHit(fopAc_ac_c* i_enemy, cCcD_Obj* i_atObj, cCcD_Obj* i_tgObj) {
    if (!isBridgeLinkMeleeAt(i_atObj)) {
        wolfStun_debugLog("f=%06d evt=bridge-reject id=%u atType=0x%x ac=%p wolf=%d\n",
                          g_Counter.mCounter0, i_enemy->id,
                          i_atObj != NULL ? i_atObj->GetAtType() : 0,
                          i_atObj != NULL ? i_atObj->GetAc() : NULL,
                          daPy_py_c::checkNowWolf() ? 1 : 0);
        return;
    }

    dCcU_AtInfo atInfo = {};
    atInfo.mpCollider = i_atObj;
    prepareBridgeAtInfo(i_enemy, &atInfo);
    cc_at_check(i_enemy, &atInfo);

    wolfStun_debugLog("f=%06d evt=bridge-hit id=%u dmg=%u hp=%d mpActor=%p sound=%p wolf=%d\n",
                      g_Counter.mCounter0, i_enemy->id, atInfo.mAttackPower, i_enemy->health,
                      atInfo.mpActor, atInfo.mpSound, daPy_py_c::checkNowWolf() ? 1 : 0);

    dispatchBridgeHitVfx(i_enemy, i_atObj, i_tgObj, &atInfo);

    if (i_enemy->health <= 0) {
        dispatchLethalDeathReaction(i_enemy);
    }
}

} // anonymous namespace

// ============================================
// NEW CODE — ALBW Port
// ============================================

bool dAlbwWolfCombat_isEnabled() {
    return dusk::getSettings().game.wolfLinkCombat.getValue();
}

bool dAlbwWolfStun_isTwilightEnemy(s16 i_name) {
    switch (i_name) {
    case fpcNm_E_MD_e:
    case fpcNm_E_YD_e:
    case fpcNm_E_YH_e:
    case fpcNm_E_YD_LEAF_e:
    case fpcNm_E_YMB_e:
    case fpcNm_E_YK_e:
    case fpcNm_E_YR_e:
    case fpcNm_E_YG_e:
        return true;
    default:
        return false;
    }
}

bool dAlbwWolfStun_isStunned(fopAc_ac_c* i_enemy) {
    if (i_enemy == NULL) {
        return false;
    }
    return findEntry(i_enemy->id) != NULL;
}

void dAlbwWolfStun_syncColliders(fopAc_ac_c* i_enemy, cCcD_Obj* const* i_objs, int i_count) {
    if (!dAlbwWolfCombat_isEnabled() || i_enemy == NULL || i_objs == NULL || i_count <= 0) {
        return;
    }

    WolfStunEntry* entry = findEntry(i_enemy->id);
    if (entry == NULL) {
        return;
    }

    storeColliders(entry, i_objs, i_count);
    wolfStun_debugLog("f=%06d evt=sync-colliders id=%u count=%d\n", g_Counter.mCounter0,
                      i_enemy->id, entry->mColliderCount);
}

void dAlbwWolfStun_captureAfterExecute() {
    if (!dAlbwWolfCombat_isEnabled() || sStunCount == 0) {
        return;
    }

    cCcS* ccs = dComIfG_Ccsp();

    for (int i = 0; i < sStunCount; i++) {
        if (sStunList[i].mColliderCount > 0) {
            continue;
        }

        fopAc_ac_c* enemy =
            static_cast<fopAc_ac_c*>(fpcM_SearchByID(sStunList[i].mId));
        if (enemy == NULL) {
            continue;
        }

        const int before = sStunList[i].mColliderCount;
        captureCollidersFromCcS(ccs, enemy, &sStunList[i]);
        if (sStunList[i].mColliderCount > before) {
            wolfStun_debugLog("f=%06d evt=capture-after-exec id=%u count=%d\n",
                              g_Counter.mCounter0, sStunList[i].mId, sStunList[i].mColliderCount);
        }
    }
}

void dAlbwWolfStun_apply(fopAc_ac_c* i_enemy) {
    if (!dAlbwWolfCombat_isEnabled() || i_enemy == NULL) {
        return;
    }

    const fpc_ProcID id = i_enemy->id;

    WolfStunEntry* existing = findEntry(id);
    if (existing != NULL) {
        existing->mTimer = static_cast<s16>(WOLF_STUN_FRAMES);
        if (existing->mColliderCount == 0) {
            captureCollidersFromCcS(dComIfG_Ccsp(), i_enemy, existing);
        }
        wolfStun_debugLog("f=%06d evt=stun-refresh id=%u coll=%d name=%d\n",
                          g_Counter.mCounter0, id, existing->mColliderCount,
                          fopAcM_GetName(i_enemy));
        return;
    }

    if (sStunCount >= WOLF_STUN_MAX) {
        wolfStun_debugLog("f=%06d evt=stun-list-full id=%u\n", g_Counter.mCounter0, id);
        return;
    }

    fpcM_PauseEnable(i_enemy, 1);
    sStunList[sStunCount].mId = id;
    sStunList[sStunCount].mTimer = static_cast<s16>(WOLF_STUN_FRAMES);
    sStunList[sStunCount].mColliderCount = 0;
    sStunList[sStunCount].mSkipBridgeFrame = g_Counter.mCounter0;
    for (int c = 0; c < static_cast<int>(ARRAY_SIZEU(sStunList[sStunCount].mColliders)); c++) {
        sStunList[sStunCount].mColliders[c] = NULL;
    }
    sStunCount++;

    WolfStunEntry* entry = &sStunList[sStunCount - 1];
    captureCollidersFromCcS(dComIfG_Ccsp(), i_enemy, entry);
    wolfStun_debugLog("f=%06d evt=stun-apply id=%u active=%d coll=%d name=%d mpObj=%d skipBridge=%d\n",
                      g_Counter.mCounter0, id, sStunCount, entry->mColliderCount,
                      fopAcM_GetName(i_enemy), dComIfG_Ccsp()->mObjCount, entry->mSkipBridgeFrame);
    if (entry->mColliderCount == 0) {
        wolfStun_debugLog("f=%06d evt=stun-apply-no-colliders id=%u (expect sync on cc_set)\n",
                          g_Counter.mCounter0, id);
    }
}

void dAlbwWolfStun_update() {
    if (!dAlbwWolfCombat_isEnabled()) {
        for (int i = 0; i < sStunCount; i++) {
            restoreStunColliderFlags(&sStunList[i]);
            base_process_class* proc = fpcM_SearchByID(sStunList[i].mId);
            if (proc != NULL) {
                fpcM_PauseDisable(proc, 1);
            }
        }
        sStunCount = 0;
        return;
    }

    int writeIdx = 0;
    for (int i = 0; i < sStunCount; i++) {
        sStunList[i].mTimer--;
        if (sStunList[i].mTimer > 0) {
            sStunList[writeIdx++] = sStunList[i];
        } else {
            restoreStunColliderFlags(&sStunList[i]);
            base_process_class* proc = fpcM_SearchByID(sStunList[i].mId);
            if (proc != NULL) {
                fpcM_PauseDisable(proc, 1);
                wolfStun_debugLog("f=%06d evt=stun-expire id=%u\n", g_Counter.mCounter0,
                                  sStunList[i].mId);
            }
        }
    }
    sStunCount = writeIdx;
}

void dAlbwWolfStun_beforeMove() {
    if (!dAlbwWolfCombat_isEnabled() || sStunCount == 0) {
        return;
    }

    cCcS* ccs = dComIfG_Ccsp();

    for (int i = 0; i < sStunCount; i++) {
        fopAc_ac_c* enemy =
            static_cast<fopAc_ac_c*>(fpcM_SearchByID(sStunList[i].mId));
        if (enemy == NULL) {
            removeStunEntry(i);
            i--;
            continue;
        }

        if (sStunList[i].mColliderCount == 0) {
            captureCollidersFromCcS(ccs, enemy, &sStunList[i]);
            if (sStunList[i].mColliderCount == 0) {
                wolfStun_debugLogNoColliders(sStunList[i].mId);
            }
        }

        prepareStunnedEnemyForBridge(enemy, &sStunList[i]);

        for (int c = 0; c < sStunList[i].mColliderCount; c++) {
            cCcD_Obj* obj = sStunList[i].mColliders[c];
            if (obj == NULL) {
                continue;
            }

            if (!obj->ChkTgSet()) {
                obj->OnTgSetBit();
            }

            if (!colliderRegistered(ccs, obj)) {
                ccs->Set(obj);
            }
        }
    }

    wolfStun_debugLogHeartbeat(ccs);
}

void dAlbwWolfStun_afterMove() {
    if (!dAlbwWolfCombat_isEnabled() || sStunCount == 0) {
        return;
    }

    for (int i = 0; i < sStunCount; i++) {
        fopAc_ac_c* enemy =
            static_cast<fopAc_ac_c*>(fpcM_SearchByID(sStunList[i].mId));
        if (enemy == NULL) {
            removeStunEntry(i);
            i--;
            continue;
        }

        if (sStunList[i].mSkipBridgeFrame == g_Counter.mCounter0) {
            wolfStun_debugLog("f=%06d evt=bridge-skip-same-frame id=%u\n", g_Counter.mCounter0,
                              enemy->id);
            continue;
        }

        for (int c = 0; c < sStunList[i].mColliderCount; c++) {
            cCcD_Obj* tgObj = sStunList[i].mColliders[c];
            if (tgObj == NULL) {
                continue;
            }

            dCcD_GObjInf* tgGObj = dCcD_GetGObjInf(tgObj);
            if (tgGObj == NULL || !tgGObj->ChkTgHit()) {
                continue;
            }

            cCcD_Obj* atObj = tgGObj->GetTgHitObj();
            if (atObj == NULL) {
                wolfStun_debugLog("f=%06d evt=tg-hit-no-at id=%u tgGObj=%p\n",
                                  g_Counter.mCounter0, enemy->id, tgGObj);
                continue;
            }

            processBridgeHit(enemy, atObj, tgObj);

            if (enemy->health <= 0) {
                removeStunEntry(i);
                i--;
                break;
            }
        }
    }
}

// ============================================
// NEW CODE ENDS HERE
// ============================================

#endif // TARGET_PC
